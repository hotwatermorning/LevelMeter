//
//  PeakMeter.h
//  LevelMeter
//
//  Created by yuasa on 2016/01/18.
//
//

#ifndef PeakMeter_hpp
#define PeakMeter_hpp

#pragma once

#include <cassert>
#include <limits>
#include "./PeakHoldValue.hpp"
#include "./Linear_dB.hpp"
#include "./ILevelMeter.hpp"
#include "./PeakHoldValue.hpp"
#include "./MovingAverageValue.hpp"
#include "./ZeroIterator.hpp"

//! Sample PPMを実現するクラス
struct PeakMeter
:   ILevelMeter
{
    typedef int millisec;
    typedef double dB_t;
    
    size_t      sampling_rate_;
    PeakHoldValue               peak_hold_;
    MovingAverageValue<dB_t>    current_peak_;
    dB_t                        highest_level_;
    
    static int const kDefaultReleaseSpeed = -96 / 2;
    
    static double GetLowestLevel() { return -640; }
    
    PeakMeter(size_t sampling_rate, int moving_average, millisec peak_hold_time, dB_t release_speed = kDefaultReleaseSpeed)
    :	sampling_rate_(sampling_rate)
    ,   current_peak_(1, 0)
    ,	highest_level_(GetLowestLevel())
    {
        peak_hold_ = PeakHoldValue(sampling_rate, peak_hold_time, release_speed);
        SetMovingAverage(moving_average);
    }
    
    size_t	GetSamplingRate	() const { return sampling_rate_; }
    
    //! ピークホールドする時間を返す
    millisec GetPeakHoldTime() const { return peak_hold_.GetPeakHoldTime(); }
    
    //! ピークホールドする時間を設定する。
    void	SetPeakHoldTime		(millisec peak_hold_time)
    {
        peak_hold_.SetPeakHoldTime(peak_hold_time);
    }
    
    //! ReleaseSpeedは、一秒あたりピークレベルがどれだけ変化するかをdBで表す。
    //! -96.0であれば、一秒に-96dBまでメータが下降する
    //! 0.0であればメーターは張り付いたまま下降しない
    dB_t	GetReleaseSpeed	() const { return peak_hold_.GetReleaseSpeed(); }
    
    //! 現在のピーク値を取得する。
    //! ホールドタイムの設定により、
    //! より大きなピーク値が設定されない場合はピーク値が維持される。
    dB_t	GetPeakHoldLevel		() const { return peak_hold_.GetPeak(); }
    
    //! 現在の音量レベル値を取得する。
    //! リリースタイムの設定により、
    //! 無音状態になっても即座に0になるわけではない。
    dB_t	GetCurrentPeakLevel		() const { return current_peak_.GetAverage(); }
    
    //! リセット状態からの最大値を取得する。
    //! この値は明示的にResetHighestLevelを呼ばない限り0.0へ初期化されない。
    dB_t	GetHighestLevel	() const { return highest_level_; }
    
public:
    
    void	SetReleaseSpeed	(dB_t speed)
    {
        peak_hold_.SetReleaseSpeed(speed);
    }
    
    void	SetSamplingRate	(size_t sampling_rate)
    {
        peak_hold_ = PeakHoldValue(sampling_rate, GetPeakHoldTime(), GetReleaseSpeed());
    }
    
    void	ResetHighestLevel()
    {
        highest_level_ = GetLowestLevel();
    }
    
    void    SetMovingAverage(int moving_average)
    {
        current_peak_ = MovingAverageValue<dB_t>(moving_average, GetLowestLevel());
    }

    template<class InputIterator>
    void	doSetSamples	(InputIterator begin, InputIterator end)
    {
         for(InputIterator it = begin; it != end; ++it) {
             auto new_value = linear_to_dB(*it);
             peak_hold_.PushPeakValue(new_value);
             current_peak_.Push(new_value);
             if(peak_hold_.GetPeak() > highest_level_) {
                 highest_level_ = peak_hold_.GetPeak();
             }
        }
    }
    
    void	SetSamples	(float const *samples, size_t num_samples) override
    {
        doSetSamples(samples, samples + num_samples);
    }
    
    void	SetSamples	(double const *samples, size_t num_samples) override
    {
        doSetSamples(samples, samples + num_samples);
    }
    
    void	Consume(size_t n) override
    {
        doSetSamples(ZeroIterator(n), ZeroIterator());
    }
};

#endif /* PeakMeter_hpp */
