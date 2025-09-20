#pragma once
#ifndef WSOCKET__SLIDINGBUFFER_HPP
#define WSOCKET__SLIDINGBUFFER_HPP

#include <cassert>

#include <memory>


namespace wsocket {

struct Buffer {
    uint8_t *buf{};
    size_t   size{};
};

class SlidingBuffer {
public:
    SlidingBuffer() = default;
    explicit SlidingBuffer(size_t len) { Resize(len); }
    ~SlidingBuffer() = default;

    SlidingBuffer(const SlidingBuffer &)            = delete;
    SlidingBuffer &operator=(const SlidingBuffer &) = delete;

    void Resize(size_t len) {
        if(buffer_used_ > len) {
            len = buffer_used_;
        }

        std::unique_ptr<uint8_t[]> tmp = std::make_unique<uint8_t[]>(len);

        if(buffer_ && buffer_used_ > 0) {
            memcpy(tmp.get(), buffer_.get(), buffer_used_);
        }

        std::swap(this->buffer_, tmp);
        buffer_size_ = len;
    }

    Buffer GetData() const { return {buffer_.get(), buffer_used_}; }
    size_t GetDataLen() const { return buffer_used_; }
    size_t GetSize() const { return buffer_size_; }

    Buffer PrepareWrite() const { return Buffer{buffer_.get() + buffer_used_, buffer_size_ - buffer_used_}; }
    void   CommitWrite(size_t len) { buffer_used_ += len; }

    void Feed(const Buffer &buf) {
        if(buf.size + buffer_used_ > buffer_size_) {
            Resize(buf.size + buffer_used_);
        }

        std::memcpy(buffer_.get() + buffer_used_, buf.buf, buf.size);
        buffer_used_ += buf.size;
    }

    void Consume(size_t len) {
        buffer_used_ -= len;

        if(buffer_used_ > 0)
            std::memmove(buffer_.get(), buffer_.get() + len, buffer_used_);
    }
    void Consume(size_t start, size_t len) {
        assert(buffer_used_ >= len);

        int64_t move_len = buffer_used_ - len - start;
        assert(move_len >= 0);

        buffer_used_ -= len;
        if(buffer_used_ > 0 && move_len > 0)
            std::memmove(buffer_.get() + start, buffer_.get() + start + len, move_len);
    }

private:
    std::unique_ptr<uint8_t[]> buffer_;
    size_t                     buffer_size_ = 0;
    size_t                     buffer_used_ = 0;
};

} // namespace wsocket

#endif // WSOCKET__SLIDINGBUFFER_HPP
