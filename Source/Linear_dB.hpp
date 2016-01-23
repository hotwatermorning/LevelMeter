/*
  ==============================================================================

    Linear_dB.h
    Created: 18 Jan 2016 4:57:04pm
    Author:  yuasa

  ==============================================================================
*/

#ifndef LINEAR_DB_HPP_INCLUDED
#define LINEAR_DB_HPP_INCLUDED


inline
double linear_to_dB_abs(double linear)
{
    static double const dB_640 = 0.00000000000000000000000000000001;
    if(linear < dB_640) {
        return -640;
    } else {
        return 20.0 * log10(linear);
    }
}

//! 線形な音量値からdBへの変換
//! linearがマイナスの場合はlinearの絶対値が使用される。
//! 4.0 -> +   12dB
//! 2.0 -> +   6dB
//! 1.0 -> +/- 0dB
//! 0.5 -> -   -6dB
//! 0.25-> -   -12dB
//! 0.0 -> -   inf.dB
inline
double linear_to_dB(double linear)
{
    return linear_to_dB_abs(fabs(linear));
}

//! dBから線形な音量値への変換
//! +  12dB -> 4.0
//! +   6dB -> 2.0
//! +/- 0dB -> 1.0
//! -   6dB -> 0.5
//! -  12dB -> 0.25
//! -inf.dB-> 0.0
inline
double dB_to_linear(double dB)
{
    return pow(10.0, dB/20.0);
}

#endif  // LINEAR_DB_HPP_INCLUDED
