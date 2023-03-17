#include "tcp_receiver.hh"
#include <iostream>

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    seg.payload();
    if(!recievedSYN && !seg.header().syn) return;
    if(seg.header().syn) {
        isn = WrappingInt32(seg.header().seqno);
        recievedSYN = true;
    }
    if(seg.header().fin)
        recievedFIN = true;
    uint64_t abs_seqno = unwrap(seg.header().seqno, isn, _reassembler.num_written_bytes());
    _reassembler.push_substring(seg.payload().copy(), abs_seqno - (seg.header().syn ? 0 : 1), seg.header().fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    if(recievedSYN) return WrappingInt32(isn + _reassembler.num_written_bytes() + recievedSYN + (_reassembler.empty()?recievedFIN:0)); 
    return nullopt;
}

size_t TCPReceiver::window_size() const { 
    return _reassembler.remaining_capacity();
}
