# ReadMe

こちらのライブラリを使用しています。動画をバイナリに変換する方法もこちらを参照してください。
https://developer.mbed.org/teams/Renesas/code/RGA_HelloWorld_4_3inch/

上ライブラリを追加することで画像ファイルを扱えるようになります。  

変更するファイルは以下の3つです。

1. main.cpp
2. rga_func.cpp
3. rga_func.h
4. BinaryImage_RZ_A1H.c
5. BinaryImage_RZ_A1H.c

この5つのファイルを差し替えてコンパイルするとmbedに書き込むバイナリファイルが出力されます。

## アニメーションの追加、変更方法
動画を連番jpg or pngに変換します。
https://developer.mbed.org/teams/Renesas/code/RGA_HelloWorld_4_3inch/
を参照して、画像をバイナリデータに変換します。その際
https://github.com/hoshiimo28/thanksignal/tree/master/MakingTextForThankSignal
を使用すると多少楽に作成できます。

出力されたBinaryImage_RZ_A1H.hとBinaryImage_RZ_A1H.hを差し替えて、
rga_func.cppのテーブル graphics_image_t＊ ThankSignal？[？]に動画分の画像を設定します。
