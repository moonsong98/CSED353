#include "tcp_connection.hh"

#include <iostream>

// Dummy implementation of a TCP connection

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

size_t TCPConnection::remaining_outbound_capacity() const { return _sender.stream_in().remaining_capacity(); }

size_t TCPConnection::bytes_in_flight() const { return _sender.bytes_in_flight(); }

size_t TCPConnection::unassembled_bytes() const { return _receiver.unassembled_bytes(); }

size_t TCPConnection::time_since_last_segment_received() const { return _time_since_last_segment_received; }

void TCPConnection::segment_received(const TCPSegment &seg) {
    _time_since_last_segment_received = 0;

    if (seg.header().rst) {
        _kill_connection();
        return;
    }

    if (!_sent_SYN && seg.header().ack)
        return;

    _receiver.segment_received(seg);

    if (seg.header().ack)
        _sender.ack_received(seg.header().ackno, seg.header().win);

    if ((seg.header().syn and _sent_SYN) or seg.header().fin or seg.payload().str().length())
        _sender.send_empty_segment();

    _send_segments(false);
}

bool TCPConnection::active() const { return _is_active; }

size_t TCPConnection::write(const string &data) {
    size_t n_bytes_written = _sender.stream_in().write(data);
    _send_segments(false);
    return n_bytes_written;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
    _sender.tick(ms_since_last_tick);

    _time_since_last_segment_received += ms_since_last_tick;

    if (_sender.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS)
        _unclean_shutdown();

    if (_sent_SYN)
        _send_segments(false);
}

void TCPConnection::end_input_stream() {
    _sender.stream_in().end_input();
    _send_segments(false);
}

void TCPConnection::connect() { _send_segments(false); }

void TCPConnection::_send_segments(bool set_rst) {
    _sender.fill_window();
    if (not _sent_SYN)
        _sent_SYN = true;

    while (not _sender.segments_out().empty()) {
        TCPSegment seg_out = _sender.segments_out().front();
        _sender.segments_out().pop();

        if (_receiver.ackno().has_value()) {
            seg_out.header().ack = true;
            seg_out.header().ackno = _receiver.ackno().value();
        }
        seg_out.header().win = _receiver.window_size();

        if (set_rst and _sender.segments_out().empty())
            seg_out.header().rst = true;

        _segments_out.push(seg_out);
    }
    _clean_shutdown();
}

void TCPConnection::_kill_connection() {
    _sender.stream_in().set_error();
    _receiver.stream_out().set_error();
    _is_active = false;
}

void TCPConnection::_unclean_shutdown() {
    _kill_connection();

    if (_sender.segments_out().empty())
        _sender.send_empty_segment();

    _send_segments(true);
}

void TCPConnection::_clean_shutdown() {
    // If the inbound stream ends before the TCPConnection has reached EOF on its outbound stream, this variable needs
    // to be set to false.
    if (_receiver.stream_out().input_ended() and not _sender.stream_in().eof()) {
        _linger_after_streams_finish = false;
    }

    if (_receiver.stream_out().input_ended() and _receiver.unassembled_bytes() == 0  // Prereq #1
        and _sender.stream_in().input_ended()                                        // Prereq #2
        and _sender.bytes_in_flight() == 0                                           // Prereq #3
    ) {
        if (not _linger_after_streams_finish or time_since_last_segment_received() >= 10 * _cfg.rt_timeout) {
            _is_active = false;
        }
    }
}

TCPConnection::~TCPConnection() {
    try {
        if (active()) {
            cerr << "Warning: Unclean shutdown of TCPConnection\n";
            // Your code here: need to send a RST segment to the peer
            _unclean_shutdown();
        }
    } catch (const exception &e) {
        std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
    }
}
