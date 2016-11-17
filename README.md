# thanksignal
ルネサスコンテスト2016に出したThankSignalの説明とコード類

## 使い方
準備  

1. ThankSignal_PEACHをダウンロードし、コンパイルしたbinファイルをGR-PEACHに書き込みます
2. ThankSignal_Braverodgeをダウンロードし、mbed HRM1017 にコンパイルしたbinファイルを書き込みます
3．iphoneにスマートフォン側アプリをインストール

使い方  

1. 車にデバイスを設置
2. スマホアプリのアイコンをタップするorスマホに向かって「ありがとう」と音声認識させる
3. GR-PEACHのLCD上にアニメーションが表示される

## 回路図の説明
５V電源 - GR-PEACH  
GR-PEACH(D9) - BVMCN5103-BK(P0_9)  
GR-PEACH(P42) - BVMCN5103-BK(GND)  
![回路図](https://github.com/hoshiimo28/thanksignal/blob/master/ThankSignal%E5%9B%9E%E8%B7%AF%E6%A7%8B%E6%88%90.jpg?raw=true "回路図")


## ファイル構成
### GR-PEACH側 ThankSignal_PEACH
*  登録したアニメーションを再生し、画面に表示する

### mbed側（BLE）　ThankSignal_Braveridge
* BLEでスマホと通信し、表示するアニメーションの指定を受けとる
* GR-PEACHにシリアル通信で表示するアニメーションを指定する

### アニメーション作成補助ツール　MakingTextForThankSignal
* xmlを自動作成する

### 外装ファイル
* ThankSignal.ai

### スマートフォン側
下参照  
github：https://github.com/koooootake/ThankSignal-iOS
