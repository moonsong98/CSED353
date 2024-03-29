#include "tcp_receiver.hh"

// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    // If segment which has syn in a header has not come yet, ignore it
    if (!recievedSYN && !seg.header().syn)
        return;
    // If segment which has syn in a header reached, save isn
    if (seg.header().syn) {
        isn = WrappingInt32(seg.header().seqno);
        recievedSYN = true;
    }
    if (seg.header().fin)
        recievedFIN = true;
    // Calculate absolute sequence number to save in buffer, as in a document, checkpoint is the index of the last
    // reassembled byte
    uint64_t abs_seqno = unwrap(seg.header().seqno, isn, _reassembler.stream_out().bytes_written());
    // Since SYN occupies 1 space in a sequence, remove 1 if it's not a segment which has a syn in a header
    _reassembler.push_substring(seg.payload().copy(), abs_seqno - (seg.header().syn ? 0 : 1), seg.header().fin);
}

optional<WrappingInt32> TCPReceiver::ackno() const {
    // Only return ackno if SYN is recieved
    // If FIN is recieved but some segment in between SYN and FIN is not recieved, do not add 1 since last segment is
    // not assembled yet
    if (recievedSYN)
        return WrappingInt32(isn + _reassembler.stream_out().bytes_written() + recievedSYN +
                             (_reassembler.empty() ? recievedFIN : 0));
    return nullopt;
}

size_t TCPReceiver::window_size() const {
    // Window size is also known as remaining capacity in buffer
    return _reassembler.stream_out().remaining_capacity();
}
