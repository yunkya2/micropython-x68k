## X68k版固有機能テスト

### 事前準備

* micropython-x68k/ 以下のファイルがクロス環境 - X68k間で共有されていることを前提とします
* 実行前にクロス環境で ports/x68k と ports/x68k/mpyconv 以下でmakeを行い、micropython.x と mpyconv.x をビルドしておきます
* クロス環境上で run68 を実行できるようにしておきます

### インラインアセンブラのテスト

* クロス環境で testasm.sh を実行します
  * テストコード asmm68k.py をバイトコードに変換し、バイトコード中のインラインアセンブラ出力と、同等のソースコードをm68k-xelf-asでアセンブルした結果が一致することを確認します
  * スクリプトの出力の最後に `### Binary data matches ###` と表示されれば正常です

### サンプルコードの実行テスト

* X68k上で testpy.bat と testmpy.bat を実行します
* sample/ 以下にあるサンプルコードを一通り実行します
* testmpy.bat は同じコードを一旦 mpyconv.x でバイトコードに変換してから実行します

### シフトJISのテスト

* X68k上で tesetsjis.batを実行します。
* クロス環境上で testsjis.sh を実行します。
* `### SJIS test OK ###` と表示されれば正常です

### X-BASIC 外部関数の実行テスト

* Human68kシステムディスクのBASIC2ディレクトリをこのディレクトリにコピーしておきます
* X68k上で testfnc.bat を実行します
* X-BASICの外部関数でマウス座標を取得しながら線を引きます
