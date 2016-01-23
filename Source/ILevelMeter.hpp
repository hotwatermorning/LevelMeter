/*
  ==============================================================================

    ILevelMeter.h
    Created: 18 Jan 2016 4:37:12pm
    Author:  yuasa

  ==============================================================================
*/

#ifndef ILEVELMETER_H_INCLUDED
#define ILEVELMETER_H_INCLUDED


//
//  PeakMeter.h
//  LevelMeter
//
//  Created by yuasa on 2016/01/18.
//
//

#ifndef LevelMeter_hpp
#define LevelMeter_hpp

#pragma once

//! 音量メーターのインターフェース
struct ILevelMeter
{
protected:
    ILevelMeter()
    {}
    
public:
    virtual ~ILevelMeter()
    {}
    
    //! サンプルデータをメーターにセット(float版)
    virtual void	SetSamples	(float const *samples, size_t num_samples) = 0;
    
    //! サンプルデータをメーターにセット(double版)
    virtual void	SetSamples	(double const *samples, size_t num_samples) = 0;
    
    //! 無音状態の処理。
    /*!
        フレーム処理でなにもデータない状態の時はこの関数を呼び出して
        そのフレームのサンプル数を通知する。
        メーターの各具象クラスは、すべてのデータが0のサンプルが渡されたのと同じようにふるまうこと。
     */
    virtual void    Consume     (size_t num_samples) = 0;
};

#endif /* LevelMeter_hpp */



#endif  // ILEVELMETER_H_INCLUDED
