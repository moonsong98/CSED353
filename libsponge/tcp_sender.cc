#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <iostream>
#include <random>

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn)
    : _isn(fixed_isn.value_or(WrappingInt32{random_device()()}))
    , _initial_retransmission_timeout{retx_timeout}
    , _stream(capacity) {}

TCPSender::OrderedSegment::OrderedSegment(const size_t abs_seqno, const TCPSegment seg)
    : _abs_seqno(abs_seqno), _seg(seg) {}

uint64_t TCPSender::bytes_in_flight() const { return _bytes_in_flight; }

void TCPSender::fill_window() {
    // If SYN segment hasn't been sent yet, send it
    if (!_syn) {
        TCPSegment new_segment;
        new_segment.header().syn = true;
        _send_segment(new_segment);
        _syn = true;
        return;
    }

    /*
     * Calculate number of bytes which need to be sent considering
     * 1. Receiver's window
     * 2. Gap between next absolute sequence number and recent ackno
     * */
    size_t n_bytes_can_send = _rwnd > 0 ? _rwnd : 1;
    n_bytes_can_send -= _next_seqno - _checkpoint;

    while (n_bytes_can_send > 0 && !_fin) {
        // Buffer is empty, do not create a new segment
        if(!_stream.eof() && _stream.buffer_empty())
            return;

        TCPSegment new_segment;

        // Payload size shouldn't exceed maximum payload size
        size_t payload_size = n_bytes_can_send;
        if(payload_size > TCPConfig::MAX_PAYLOAD_SIZE)
            payload_size = TCPConfig::MAX_PAYLOAD_SIZE;

        // Read from the buffer, and put paylod into the segment
        std::string payload = _stream.read(payload_size);
        new_segment.payload() = Buffer(std::move(payload));

        // If it's a last segment to be sent, add fin tag in a header
        if (_stream.eof() && n_bytes_can_send > new_segment.length_in_sequence_space()) {
            --n_bytes_can_send;
            new_segment.header().fin = true;
            _fin = true;
        }

        _send_segment(new_segment);
        n_bytes_can_send -= payload_size;
    }
}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
    const uint64_t abs_ackno = unwrap(ackno, _isn, _checkpoint);
    _rwnd = window_size;
    /* 
     * Check validity of ackno
     * 1. If it's greater than next_seqno, it hasn't been sent yet.
     * 2. If it's less than checkpoint, it already have been acknowledged.
     * */
    if (abs_ackno > _next_seqno)
        return;
    if (abs_ackno <= _checkpoint)
        return;

    _checkpoint = abs_ackno;
    // Iterate through _segments_in_flight to remove segments no longer needed to be tracked
    while (!_segments_in_flight.empty()) {
        OrderedSegment top_seg = _segments_in_flight.top();

        if (top_seg._abs_seqno + top_seg._seg.length_in_sequence_space() <= abs_ackno) {
            _bytes_in_flight -= top_seg._seg.length_in_sequence_space();
            _segments_in_flight.pop();
        } else
            break;
    }
    /*
     * If ack received,
     * 1. rto := initial rto
     * 2. If outstanding segments left, restart the timer.
     * 3. Set number of consecutive retransmission to 0.
     * */
    _rto = _initial_retransmission_timeout;
    if (!_segments_in_flight.empty()) {
        _time_spent = 0;
        _is_timer_running = true;
    }
    _n_consecutive_retransmissions = 0;
    fill_window();
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
void TCPSender::tick(const size_t ms_since_last_tick) {
    _time_spent += ms_since_last_tick;
    /*
     * If timer is expired,
     * 1. Retransmit the earliest segment that hsn't been fully acknolweged.
     * 1. _n_consecutive_retransmissions := _n_consecutive_retransmissions
     * 2. _rto := _rto * 2
     * 3. Reset the timer
     * */
    if (_time_spent >= _rto && !_segments_in_flight.empty()) {
        _segments_out.push(_segments_in_flight.top()._seg);
        _time_spent = 0;
        if (_rwnd > 0) {
            ++_n_consecutive_retransmissions;
            _rto *= 2;
        }
    }
    if (_segments_in_flight.empty()) {
        _time_spent = 0;
        _is_timer_running = false;
    }
}

unsigned int TCPSender::consecutive_retransmissions() const { return _n_consecutive_retransmissions; }

void TCPSender::send_empty_segment() {
    TCPSegment new_segment;
    new_segment.header().seqno = next_seqno();
    _segments_out.push(new_segment);
}

void TCPSender::_send_segment(TCPSegment new_segment) {
    /*
     * 1. Send a new segment
     * 2. Track a segment in flight
     * 3. Update seqno
     * 4. Track a number of bytes in flight
     * 5. If timer is not running, run it
     * */
    // Set seqno in a header
    new_segment.header().seqno = next_seqno();
    _segments_out.push(new_segment);
    _segments_in_flight.push(OrderedSegment(_next_seqno, new_segment));
    _next_seqno += new_segment.length_in_sequence_space();
    _bytes_in_flight += new_segment.length_in_sequence_space();
    if (!_is_timer_running) {
        _is_timer_running = true;
        _time_spent = 0;
        _rto = _initial_retransmission_timeout;
    }
}
