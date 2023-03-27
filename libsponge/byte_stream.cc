#include "byte_stream.hh"

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

using namespace std;

ByteStream::ByteStream(const size_t capacity) : _capacity(capacity), buffer(string(capacity, ' ')) {}

size_t ByteStream::write(const string &data) {
    size_t writeSize = min(data.length(), remaining_capacity());
    for (unsigned int i = 0; i < writeSize; ++i)
        buffer[endOfBuffer++] = data[i];
    bytesWritten += writeSize;

    return writeSize;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const { return buffer.substr(0, len); }

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
    for (unsigned int i = len; i < endOfBuffer; ++i)
        buffer[i - len] = buffer[i];
    endOfBuffer -= len;
    bytesRead += len;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    size_t read_len = len < endOfBuffer ? endOfBuffer : endOfBuffer;
    string readStr = peek_output(read_len);
    pop_output(read_len);
    return readStr;
}

void ByteStream::end_input() { isInputEnded = true; }

bool ByteStream::input_ended() const { return isInputEnded; }

size_t ByteStream::buffer_size() const { return endOfBuffer; }

bool ByteStream::buffer_empty() const { return endOfBuffer == 0; }

bool ByteStream::eof() const { return isInputEnded && buffer_empty(); }

size_t ByteStream::bytes_written() const { return bytesWritten; }

size_t ByteStream::bytes_read() const { return bytesRead; }

size_t ByteStream::remaining_capacity() const { return _capacity - endOfBuffer; }
