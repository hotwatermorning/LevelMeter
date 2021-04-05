LevelMeter
==========

## このプログラムは

[Event for Diverse Game Engineers](http://edge.connpass.com/event/20910/)
でのLTで使用したプログラムです。

<img width="912" alt="スクリーンショット 2021-04-05 16 03 06" src="https://user-images.githubusercontent.com/359226/113549975-44638680-962d-11eb-9363-705586bc9c10.png">

オーディオ用レベルメーターの実装として、

 * Peak Programme Meter
 * VU Meter(実際にはRMS Meter)
 * Spectrum Analyzer

の実装サンプルが含まれています。

## ビルド方法

このプログラムでは、GUIを実装するために、[JUCEライブラリ](http://www.juce.com/)を使用してします。

リポジトリにjucerファイルが含まれているので、このファイルをIntrojucerで開き、IntrojuerからXcodeのような各プラットフォーム用のIDEを起動して、ビルドを行って下さい。


