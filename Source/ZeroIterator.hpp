/*
  ==============================================================================

    ZeroIterator.h
    Created: 19 Jan 2016 5:07:54pm
    Author:  yuasa

  ==============================================================================
*/

#ifndef ZEROITERATOR_HPP_INCLUDED
#define ZEROITERATOR_HPP_INCLUDED

//! 常に0を返すイテレータ
struct ZeroIterator
:	std::iterator<std::input_iterator_tag, double>
{
    typedef ZeroIterator this_type;
    ZeroIterator()
    :	begin_(0)
    ,	end_(0)
    {}
    
    explicit
    ZeroIterator(size_t size)
    :	begin_(0)
    ,	end_(static_cast<int>(size))
    {}
    
    double		operator*() const { return 0; }
    
    this_type &	operator++() {
        assert(begin_ < end_);
        begin_++;
        return *this;
    }
    
    this_type operator++(int) {
        this_type tmp(*this);
        ++(*this);
        return tmp;
    }
    
private:
    int begin_;
    int end_;
    
    friend bool operator==(this_type const &lhs, this_type const &rhs)
    {
        return
        (lhs.begin_ == rhs.begin_ && lhs.end_ == rhs.end_) ||
        (lhs.begin_ == lhs.end_ && rhs.begin_ == rhs.end_);
    }
    
    friend bool operator!=(this_type const &lhs, this_type const &rhs)
    {
        return !(lhs == rhs);
    }
};


#endif  // ZEROITERATOR_HPP_INCLUDED
