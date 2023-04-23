#include "stream_reassembler.hh"

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity)
    : _output(capacity), _capacity(capacity), _buffer(string(_capacity, ' ')), _written(string(_capacity, 0)) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    const size_t first_unassembled = _output.bytes_written();
    const size_t first_unacceptable = first_unassembled + _capacity - _output.buffer_size();
    const size_t l_index = max(first_unassembled, index);
    const size_t r_index = min(first_unacceptable, index + data.length());

    // Last byte of data comes into capacity
    if (eof && r_index == index + data.length())
        _received_EOF = true;

    create_segment(first_unassembled, index, l_index, r_index, data);
    write_segment();

    // EOF data had been received and all the bytes have been assembled
    if (_received_EOF && _num_unassembled_bytes == 0)
        _output.end_input();

    return;
}

void StreamReassembler::create_segment(size_t first_unassembled,
                                       size_t index,
                                       size_t l_index,
                                       size_t r_index,
                                       const string &data) {
    if (l_index >= r_index)
        return;

    for (size_t it = l_index; it < r_index; ++it) {
        if (_written[it - first_unassembled] == 0) {
            _buffer[it - first_unassembled] = data[it - index];
            _written[it - first_unassembled] = 1;
            ++_num_unassembled_bytes;
        }
    }
}

void StreamReassembler::write_segment() {
    size_t num_assembled_bytes = 0;
    for (size_t it = 0; it < _capacity; ++it) {
        if (_written[it] == 0)
            break;
        ++num_assembled_bytes;
    }
    _output.write(_buffer.substr(0, num_assembled_bytes));
    _buffer.erase(0, num_assembled_bytes);
    _buffer.append(num_assembled_bytes, ' ');
    _written.erase(0, num_assembled_bytes);
    _written.append(num_assembled_bytes, 0);
    _num_unassembled_bytes -= num_assembled_bytes;
}

size_t StreamReassembler::unassembled_bytes() const { return _num_unassembled_bytes; }

bool StreamReassembler::empty() const { return _num_unassembled_bytes == 0; }
