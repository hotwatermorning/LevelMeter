#pragma once

#include <cassert>
#include <vector>

template<class T>
struct SimpleRingBuffer
{
    T push_and_pop(T x)
    {
        assert(buf_.size() > 0);
        auto tmp = buf_[pos_];
        buf_[pos_] = x;
        pos_ += 1;
        if (pos_ >= buf_.size()) {
            pos_ = 0;
        }

        return tmp;
    }

    void resize(int n)
    {
        assert(n > 0);
        buf_.resize(n);
        pos_ = 0;
    }

    void fill(T x)
    {
        std::fill(buf_.begin(), buf_.end(), x);
    }

    int ssize() const {
        return buf_.size();
    }

private:
    std::vector<T> buf_;
    int pos_ = 0;
};
