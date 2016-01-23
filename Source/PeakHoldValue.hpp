/*
  ==============================================================================

    PeakHoldValue.h
    Created: 21 Jan 2016 12:41:47pm
    Author:  yuasa

  ==============================================================================
*/

#ifndef PEAKHOLDVALUE_HPP_INCLUDED
#define PEAKHOLDVALUE_HPP_INCLUDED

//! ピークホールドを実現するクラス
struct PeakHoldValue
{
    typedef double dB_t;
    typedef int millisec;
    
    PeakHoldValue()
    {}
    
    //! コンストラクタ
    /*!
     @param sampling_rate サンプリングレート
     @param holding_time ピーク値が下降するまでの猶予。この時間が経過するまで値が下降しない。
     @param release_speed ピーク値が一秒間に下降するスピード
     */
    PeakHoldValue(int sampling_rate, millisec peak_hold_time, dB_t release_speed)
    :   sampling_rate_(sampling_rate)
    ,   peak_hold_time_(peak_hold_time)
    ,   peak_hold_sample_(peak_hold_time * sampling_rate / 1000)
    ,   release_speed_(release_speed)
    ,   elapsed_sample_(0)
    ,   releasing_sample_(0)
    ,   peak_value_(-640)
    {}

    dB_t GetReleaseSpeed() const
    {
        return release_speed_;
    }
    
    void SetReleaseSpeed(dB_t release_speed)
    {
        release_speed_ = release_speed;
    }
    
    //! 内部で保持している値よりも新しいピーク値の方が大きい時は、値を更新して、Holdする。
    void PushPeakValue(dB_t new_peak_value)
    {
        if(GetPeak() <= new_peak_value) {
            peak_value_ = new_peak_value;
            elapsed_sample_ = 0;
            releasing_sample_ = 0;
        } else {
            releasing_sample_ = std::max<int>(elapsed_sample_, peak_hold_sample_) - peak_hold_sample_;
            ++elapsed_sample_;
        }
    }
    
    millisec GetPeakHoldTime() const
    {
        return peak_hold_time_;
    }
    
    void SetPeakHoldTime(millisec peak_hold_time)
    {
        peak_hold_time_ = peak_hold_time;
        peak_hold_sample_ = peak_hold_time_ * sampling_rate_ / 1000;
    }
    
    dB_t GetPeak() const
    {
        double const x = releasing_sample_ / (double)sampling_rate_;
        return peak_value_ + (release_speed_ * x * x);
    }
    
private:
    int sampling_rate_;
    dB_t peak_value_;
    millisec peak_hold_time_;
    int peak_hold_sample_;
    dB_t release_speed_;
    int elapsed_sample_;
    int releasing_sample_;
};


#endif  // PEAKHOLDVALUE_HPP_INCLUDED
