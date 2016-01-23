/*
  ==============================================================================

    RMSMeter.h
    Created: 18 Jan 2016 4:36:57pm
    Author:  yuasa

  ==============================================================================
*/

#ifndef RMSMETER_H_INCLUDED
#define RMSMETER_H_INCLUDED

#include <cassert>
#include <atomic>
#include <boost/circular_buffer.hpp>
#include "./Linear_dB.hpp"
#include "./ILevelMeter.hpp"
#include "./ZeroIterator.hpp"

//! RMS(Root Mean Square)でオーディオ信号のパワーを計測するメーター
struct RMSMeter
:   ILevelMeter
{
    typedef int millisec;
    
private:
    size_t     sampling_rate_;
    double     square_sum_;
    size_t     integration_time_;
    boost::circular_buffer<double>     integration_;

public:
    RMSMeter(size_t sampling_rate, millisec integration_time)
    :	sampling_rate_(sampling_rate)
    ,   square_sum_(0)
    {
        SetIntegrationTime(integration_time);
    }
    
    size_t	GetSamplingRate	() const { return sampling_rate_; }
    
    //! 現在のRMS値を取得する
    /*!
        RMSを計測する時間の範囲は、GetWindowLength()で取得できる
     */
    double	GetRMS			() const
    {
        return sqrt(square_sum_ / (double)integration_.capacity());
    }
    
    //! RMS値を計測するための時間幅
    millisec   GetIntegrationTime () const { return integration_time_; }
    
public:
    void	SetSamplingRate	(size_t sampling_rate)
    {
        sampling_rate_ = sampling_rate;
        integration_.set_capacity(sampling_rate * integration_time_ / 10); // 400ms
        integration_.resize(integration_.capacity());
        std::fill(integration_.begin(), integration_.end(), 0.0);
        square_sum_ = 0;
        
        SetIntegrationTime(integration_time_);
    }
    
    //! 入力したデータを積算する時間を設定する
    //! VUメーター的なデータであれば、300msを設定するのが適切
    void SetIntegrationTime(millisec integration_time)
    {
        integration_time_ = integration_time;
        integration_.set_capacity(sampling_rate_ * integration_time_ / 1000); // 400ms
        integration_.resize(integration_.capacity());
        std::fill(integration_.begin(), integration_.end(), 0.0);
        square_sum_ = 0;
    }
    
public:
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
    
private:
    //! Converterは、イテレータの指す値を[0.0 .. 0.1]にマッピングするために使用する、double(std::iterator_traits<InputIterator>::reference)というシグネチャをもつ
    //! 関数や、関数オブジェクトである。
    //! 呼び出す側がIteratorAdaptorを使用すればよいのだが、実際それは面倒なので、ここでConverterを受けるようにしている。
    template<class InputIterator>
    void	doSetSamples	(InputIterator begin, InputIterator end)
    {
        for(InputIterator it = begin; it != end; ++it) {
            double const oldest = integration_.front();
            double const newest = (*it * *it);

            square_sum_ -= oldest;

            integration_.pop_front();
            integration_.push_back(newest);
            
            square_sum_ += newest;
        }
        
        if(square_sum_ < 0.0) {
            square_sum_ = 0;
        }
    }
};

#endif  // RMSMETER_H_INCLUDED
