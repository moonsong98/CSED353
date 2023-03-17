#include "wrapping_integers.hh"

// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

const uint64_t uint32_bits = 1ul << 32;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint64_t isn_raw = isn.raw_value();
    uint32_t mod_result = ((n % uint32_bits) + (isn_raw % uint32_bits)) % uint32_bits;
    return WrappingInt32{mod_result};
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    // Calculate the offset from checkpoint to n, then add it to checkpoint
    // Exception: Since index of sequence starts with 0, add uint32_bits if checkpoint + diff is less than 0
    int64_t diff = n - wrap(checkpoint, isn);
    return checkpoint + diff + (static_cast<int64_t>(checkpoint) + diff < 0 ? uint32_bits : 0);
}
