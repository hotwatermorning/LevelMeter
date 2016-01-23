/*
  ==============================================================================

    MovingAverageValue.h
    Created: 21 Jan 2016 12:44:58pm
    Author:  yuasa

  ==============================================================================
*/

#ifndef MOVINGAVERAGEVALUE_HPP_INCLUDED
#define MOVINGAVERAGEVALUE_HPP_INCLUDED

#include <boost/circular_buffer.hpp>

//! 移動平均した値を取得するクラス
template<class T>
struct MovingAverageValue
{
    MovingAverageValue(size_t num_history, T initial_value)
    {
        history_.resize(num_history);
        std::fill(history_.begin(), history_.end(), initial_value);
        total_sum_ = initial_value * (int)num_history;
    }
    
    void Push(T const &value)
    {
        auto removed = history_.front();
        total_sum_ += (-removed + value);
        
        history_.pop_front();
        history_.push_back(value);
    }
    
    T GetAverage() const
    {
        return total_sum_ / history_.size();
    }
    
private:
    boost::circular_buffer<T> history_;
    T total_sum_;
};


#endif  // MOVINGAVERAGEVALUE_HPP_INCLUDED
