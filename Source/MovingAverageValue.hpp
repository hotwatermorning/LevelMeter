/*
  ==============================================================================

    MovingAverageValue.h
    Created: 21 Jan 2016 12:44:58pm
    Author:  yuasa

  ==============================================================================
*/

#ifndef MOVINGAVERAGEVALUE_HPP_INCLUDED
#define MOVINGAVERAGEVALUE_HPP_INCLUDED

#include "SimpleRingBuffer.hpp"


//! 移動平均した値を取得するクラス
template<class T>
struct MovingAverageValue
{
    MovingAverageValue(size_t num_history, T initial_value)
    {
        history_.resize(num_history);
        history_.fill(initial_value);
        total_sum_ = initial_value * (int)num_history;
    }
    
    void Push(T const &value)
    {
        auto removed = history_.push_and_pop(value);
        total_sum_ += (-removed + value);        
    }
    
    T GetAverage() const
    {
        return total_sum_ / history_.ssize();
    }
    
private:
    SimpleRingBuffer<T> history_;
    T total_sum_;
};


#endif  // MOVINGAVERAGEVALUE_HPP_INCLUDED
