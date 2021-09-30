# 電車でGO!コントローラでTSマスコンをエミュレートしてBVEで遊ぶ。

<img src="https://github.com/7m4mon/dengo_ts_emu/blob/main/dengo_ts_emu_g.jpg" alt="" title="">

Bve trainsim というPCで遊べる電車でGO的な鉄道運転シミュレーターがあるのですが、キーボード操作では風情がありませんよね。  
いうことで、プレイステーション版 電車でGOのコントローラを入手し、BVEで使ってみることにしました。  

電GOコントローラーをArduinoで読み取るサンプルは、KeKe115さんが github で公開されています。  
https://github.com/KeKe115/Densya_de_go  

また、TSマスコンのインターフェースDLLのソースコードは開発元より公開されていて、動作に必要なコマンドはすべて記載されています。  

入力デバイスプラグイン サンプルプログラム  
https://mackoy.exblog.jp/20477692/

この2つを組み合わせて、電GOコントローラーの信号をArduinoで読み取り、TSマスコンの動作をエミュレートする手法を取ることにしました。

レバーサはセレクトボタン押下で「中立→前進→中立→後退」と遷移するようにしてあります。

制作例  
<img src="https://github.com/7m4mon/dengo_ts_emu/blob/main/dengo_ts_1.jpg" alt="" title="">  
<img src="https://github.com/7m4mon/dengo_ts_emu/blob/main/dengo_ts_2.jpg" alt="" title="">  

