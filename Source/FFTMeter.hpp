/*
  ==============================================================================

    FFTMeter.h
    Created: 19 Jan 2016 4:11:43pm
    Author:  yuasa

  ==============================================================================
*/

#ifndef FFTMETER_H_INCLUDED
#define FFTMETER_H_INCLUDED

#include <limits>
#include "./ILevelMeter.hpp"
#include "../JuceLibraryCode/JuceHeader.h"
#include "./ZeroIterator.hpp"
#include "./PeakHoldValue.hpp"
#include "./MovingAverageValue.hpp"
#include "./Linear_dB.hpp"

//! FFTメーターを実現するクラス
struct FFTMeter
:   public ILevelMeter
{
    typedef int millisec;
    typedef double dB_t;
    
    static int const kDefaultReleaseSpeed = -96 / 2;
    static dB_t GetLowestLevel()
    {
        return std::numeric_limits<dB_t>::lowest();
    }
    
    //! コンストラクタ
    /*!
        @param release_speed ピークホールドしたピーク値が、peak_hold_time後に一秒間に下降するスピード
     */
    FFTMeter(int sampling_rate, int order, int moving_average, millisec peak_hold_time, dB_t release_speed = kDefaultReleaseSpeed)
    :   order_(order)
    ,   fft_()
    {
        sampling_rate_ = sampling_rate;
        moving_average_ = moving_average;
        peak_hold_time_ = peak_hold_time;
        release_speed_ = release_speed;
        SetFFTOrder(order);
    }
    
    size_t GetSize() const
    {
        return fft_->getSize();
    }
    
    void SetSamples(float const *samples, size_t num_samples) override
    {
        doSetSample(samples, samples + num_samples);
    }
    
    void SetSamples(double const *samples, size_t num_samples) override
    {
        doSetSample(samples, samples + num_samples);
    }
    
    void Consume(size_t num_samples) override
    {
        doSetSample(ZeroIterator(num_samples), ZeroIterator());
    }
    
    void SetFFTOrder(int order)
    {
        fft_ = std::make_unique<juce::FFT>(order, false);
        
        assert(order >= 6); // 64サンプル以上
 
        fft_work_.clear();
        fft_output_.clear();
        fft_work_.reserve(pow(2, order));
        fft_output_.reserve(pow(2, order));
        
        window_.resize(pow(2, order));
        
        bool use_window_function = true;
    
        if(use_window_function) {
            
            //! hanning window
            for(int i = 0; i < window_.size(); ++i) {
                window_[i] = 0.5 - 0.5 * cos(2 * M_PI * i / (window_.size() - 1));
            }
            
            double amplitude_correction_factor = 0;
            double power_correction_factor = 0;
            
            for(int i = 0; i < window_.size(); ++i) {
                amplitude_correction_factor += window_[i];
                power_correction_factor += (window_[i] * window_[i]);
            }
            amplitude_correction_factor =  amplitude_correction_factor / GetSize();
            power_correction_factor = power_correction_factor / GetSize();
            
            //! 窓関数を掛けたことでFFT後の信号のパワーが変わってしまうのを補正
            for(int i = 0; i < window_.size(); ++i) {
                window_[i] /= amplitude_correction_factor;
            }
            
            enbw_correction_factor_ = power_correction_factor / (amplitude_correction_factor * amplitude_correction_factor);
            
        } else {
            std::fill(window_.begin(), window_.end(), 1.0);
            enbw_correction_factor_ = 1.0;
        }
        
        current_spectrum_.resize(pow(2, order), MovingAverageValue<float>(moving_average_, -640));
        peak_spectrum_.resize(pow(2, order), PeakHoldValue(sampling_rate_, peak_hold_time_, release_speed_));
    }
    
    int GetMovingAverage() const
    {
        return moving_average_;
    }
    
    void SetMovingAverage(int moving_average)
    {
        moving_average_ = moving_average;
        for(auto &a: current_spectrum_) {
            a = MovingAverageValue<float>(moving_average, -640);
        }
    }
    
    void SetPeakHoldTime(millisec peak_hold_time)
    {
        peak_hold_time_ = peak_hold_time;
        for(auto &peak: peak_spectrum_) {
            peak.SetPeakHoldTime(peak_hold_time_);
        }
    }
    
    millisec GetPeakHoldTime() const
    {
        return peak_hold_time_;
    }
    
    dB_t GetReleaseSpeed() const
    {
        return release_speed_;
    }
    
    void SetReleaseSpeed(dB_t release_speed)
    {
        release_speed_ = release_speed;
    }
    
    //! 指定した周波数ビンの移動平均されたパワースペクトルを取得
    dB_t GetSpectrum(int index)
    {
        return current_spectrum_[index].GetAverage();
    }
    
    //! 指定した周波数ビンのパワースペクトルのピークホールド値を取得
    dB_t GetPeakSpectrum(int index)
    {
        return peak_spectrum_[index].GetPeak();
    }
    
private:
    template<class RandomAccessIterator>
    void doSetSample(RandomAccessIterator begin, RandomAccessIterator end)
    {
        for(auto it = begin; it != end; ++it) {
            juce::FFT::Complex c = { (float)(*it), 0.0 };
            fft_work_.push_back(c);
            if(fft_work_.size() == GetSize()) {
                
                double const power_scaling = GetSize() * GetSize();
                double const freq_step = sampling_rate_ / (double)GetSize();
                
                //! 虚数部分のデータには0を埋める
                static juce::FFT::Complex const zero = { 0.0, 0.0 };
                std::fill(fft_output_.begin(), fft_output_.end(), zero);

                //! windowing
                for(int i = 0; i < GetSize(); ++i) {
                    fft_work_[i].r *= window_[i];
                }
                
                //! FFT実行
                fft_->perform(fft_work_.data(), fft_output_.data());
                
                for(int i = 0; i < GetSize() / 2; ++i) {
                    //! スペクトルの絶対値を計算
                    auto point_abs = juce_hypot(fft_output_[i].r, fft_output_[i].i);
                    
                    //! FFTの次数や窓関数によるパワー値のズレを補正
                    float power = point_abs / (power_scaling * freq_step * enbw_correction_factor_);
                    
                    //! 片側パワースペクトルにする
                    if(i != 0 && i != GetSize() / 2 - 1) {
                        power *= 2;
                    }

                    float const spectrum = sqrt(power);
                    current_spectrum_[i].Push(linear_to_dB(spectrum));
                    peak_spectrum_[i].PushPeakValue(linear_to_dB(spectrum));
                }
                
                fft_work_.clear();
                
            } else {
                //! FFTを実行していないときは、ピークホールドの状態のみ更新する
                for(int i = 0; i < GetSize() / 2; ++i) {
                    peak_spectrum_[i].PushPeakValue(GetLowestLevel());
                }
            }
        }
    }

    int sampling_rate_;
    int order_;
    std::unique_ptr<juce::FFT> fft_;
    std::vector<juce::FFT::Complex> fft_work_;
    std::vector<juce::FFT::Complex> fft_output_;
    
    std::vector<float> window_;
    double amplitude_correction_factor_;
    
    // 参考: http://ecd-assist.com/index.php?%E7%AA%93%E9%96%A2%E6%95%B0%E3%81%AB%E3%81%A4%E3%81%84%E3%81%A6
    double enbw_correction_factor_;
    
    std::vector<MovingAverageValue<float>> current_spectrum_;
    std::vector<PeakHoldValue> peak_spectrum_;
    
    dB_t release_speed_;
    millisec peak_hold_time_;
    int moving_average_;
};


#endif  // FFTMETER_H_INCLUDED
