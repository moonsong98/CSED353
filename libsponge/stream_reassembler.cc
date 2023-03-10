#include "stream_reassembler.hh"

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    cout << "Currently written bites: " << _num_written_bytes << '\n';
    size_t l_index = index, r_index = index + data.length();
    create_segment(l_index, r_index, data, eof);
    write_segment();
    return;
}

void StreamReassembler::write_segment() {
    size_t remaining_capacity = _output.remaining_capacity();
    if (!remaining_capacity)
        return;
    auto it = _unassembled_segments.begin();
    //cout << "before loop: " <<  it->second.data << ' ' << it->second.left_index << ' ' << it->second.isEOF << '\n';
    while (it != _unassembled_segments.end()) {
        if (_num_written_bytes == it->second.left_index) {
            if (remaining_capacity >= it->second.data.length()) {
                _output.write(it->second.data);
                remaining_capacity -= it->second.data.length();
                _num_written_bytes += it->second.data.length();
                _num_unassembled_bytes -= it->second.data.length();
                //cout << it->second.data << ' ' << it->second.left_index << ' ' << it->second.isEOF << '\n';
                if (it->second.isEOF)
                    _output.end_input();
                //cout << "Is input end: " << _output.input_ended() << '\n';
                _unassembled_segments.erase(it++);
                if (remaining_capacity > 0) continue;
                else break;
            }
            _output.write(it->second.data.substr(0, remaining_capacity));
            it->second.left_index += remaining_capacity;
            it->second.data = it->second.data.substr(remaining_capacity);
            _num_written_bytes += remaining_capacity;
            _num_unassembled_bytes -= remaining_capacity;
            break;
        }
        else break;
    }
    return;
}

void StreamReassembler::create_segment(size_t l_index, size_t r_index, string data, bool eof) {
    // Already written data
    if (data == "") {
        if (r_index < _num_written_bytes) return;
    }
    else {
        if (r_index <= _num_written_bytes) return;
    }
    // Prefix has been written, remove prefix
    if (l_index < _num_written_bytes) {
        data = data.substr(_num_written_bytes - l_index);
        l_index = _num_written_bytes;
    }
    cout << l_index << ' ' << r_index << ' ' << eof << ' ' << data << '\n';

    // Find segment
    auto it = _unassembled_segments.lower_bound(l_index);
    while (it != _unassembled_segments.end()) {
        cout << l_index << ' ' << r_index << ' ' << eof << ' ' << data << '\n';
        cout << "block left_index: " << it->second.left_index << '\n';
        // Current segment's data
        size_t b_l_index = it->second.left_index, b_r_index = it->first;
        string b_data = it->second.data;
        bool b_eof = it->second.isEOF;

        if (r_index < b_l_index)
            break;                                              // CASE 1: Cannot merge with block on right
        else if (l_index < b_l_index && r_index < b_r_index) {  // CASE 2: Can merge with block on right
            _num_unassembled_bytes -= b_data.length();
            // merge
            data = data.substr(0, b_l_index - l_index) + b_data;
            // reassign index
            r_index = b_r_index;
            // reassign eof
            eof |= b_eof;
            // move iterator
            _unassembled_segments.erase(it++);
        } else if (b_l_index <= l_index && r_index <= b_r_index)  // CASE 3: Covered by the block
            return;
        else if (l_index <= b_l_index && b_r_index <= r_index) { // CASE 4: Covers the block
            _num_unassembled_bytes-=b_data.length();
            _unassembled_segments.erase(it++);
        } 
        else if (b_l_index < l_index && b_r_index < r_index) {  // CASE 5: Can merge with block on left
            _num_unassembled_bytes -= b_data.length();
            // merge
            data = b_data + data.substr(b_r_index - l_index);
            // reassign index
            l_index = b_l_index;
            // reassign eof
            eof |= b_eof;
            // move iterator
            _unassembled_segments.erase(it++);
        }
    }
    _num_unassembled_bytes += data.length();
    Segment new_segment = {l_index, data, eof};
    _unassembled_segments.insert(make_pair(r_index, new_segment));
}

size_t StreamReassembler::unassembled_bytes() const { return _num_unassembled_bytes; }

bool StreamReassembler::empty() const { return _num_unassembled_bytes == 0; }
