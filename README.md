# thanksignal
ルネサスコンテスト2016に出したThankSignalの説明とコード類

## 使い方
** 準備 **
1. ThankSignal_PEACHをダウンロードし、コンパイルしたbinファイルをGR-PEACHに書き込みます
2. ThankSignal_Braverodgeをダウンロードし、mbed HRM1017 にコンパイルしたbinファイルを書き込みます
3．iphoneにスマートフォン側アプリをインストール

** 使い方 **
1. 車にデバイスを設置
2. スマホアプリのアイコンをタップするorスマホに向かって「ありがとう」と音声認識させる
3. GR-PEACHのLCD上にアニメーションが表示される


## ファイル構成
### GR-PEACH側 ThankSignal_PEACH
*  登録したアニメーションを再生し、画面に表示する

### mbed側（BLE）
* BLEでスマホと通信し、表示するアニメーションの指定を受けとる
* GR-PEACHにシリアル通信で表示するアニメーションを指定する

### アニメーション作成補助ツール
* xmlを自動作成する

### スマートフォン側
下参照  
github：https://github.com/koooootake/ThankSignal-iOS
