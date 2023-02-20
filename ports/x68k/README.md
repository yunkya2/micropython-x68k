# MicroPython for X680x0

MicroPython のシャープ X680x0 向け移植です。

[MicroPython](https://micropython.org/) v1.19.1 をベースにしています。

## ビルド方法

ビルドにはPC環境のクロスツールチェイン [elf2x68k](https://github.com/yunkya2/elf2x68k) が必要です。
ツールチェインにパスが通って m68k-elf-gcc 等が実行できる状態で、以下の手順でビルドします。

```
$ cd ports/x68k
$ make
```

ビルドに成功すると実行ファイル `build/micropython.x` が出来るので、X680x0 の環境上にコピーしてください。

## 実行方法

* X680x0 環境上で micropython.x を実行します。
* オプションを与えずに実行すると対話シェル (REPL - Read, Eval, Print and Loop) が起動します。CTRL+D の入力で終了します。
* コマンドラインオプションにPythonスクリプトのファイル名を与えると、そのファイルを読み込んで起動します。
  * 例: `micropython sample/ledblink.py`

## X680x0固有ライブラリ

MicroPython 自体の使い方は [公式ドキュメント](https://micropython-docs-ja.readthedocs.io/ja/v1.19.1ja/index.html) を参照してください。

X680x0版では、加えて以下のライブラリをサポートしています。

### `machine` --- ハードウェア関連の関数

X680x0版では以下のクラスのみをサポートしています。

#### クラス `Pin` -- I/O ピンの制御(を模したキーボードLEDの制御)

通常の MicroPython では `Pin` はMCUのI/O ピン (GPIO - 汎用入出力)の制御のために使用されますが、X680x0版ではすべて出力ピンとして、ピン#0～#6がキーボードの各LEDに対応します。
MicroPython向けLチカのコードをそのままX680x0版で動かすためだけに存在します。:-)

### `x68k` --- X680x0 固有ハードウェア関連の関数

* `x68k.crtmod(mode [,gon])`
  * 画面の表示モードを設定する IOCS CRTMOD を実行します。mode には設定する画面モードを指定します。
  * gon を `True` にすると、画面モード設定後にグラフィック画面をクリアして表示モードにします(IOCS G_CLR_ON の実行)。
* `x68k.vsync()`
  * CRTCの垂直帰線期間になるまで待ちます。
* `x68k.curon()`
* `x68k.curoff()`
  * カーソル表示をON/OFFします。

* `x68k.gvram()`
* `x68k.tvram()`
  * グラフィックVRAMおよびテキストVRAMのフレームバッファとなる `memoryview` オブジェクトを返します。
  * `framebuf.FrameBuffer` オブジェクトのコンストラクタに与えることで、`framebuffer`クラスによる画面描画が可能になります。
* `x68k.fontrom()`
  * フォントROM領域の `memoryview` オブジェクトを返します。読み出し専用です。

* `x68k.i.<IOCSコール名>`
  * IOCSコール番号を定数で定義しています。
    * 例: `x68k.i.B_PRINT` = 0x21
* `x68k.iocs(d0, d1, d2, d3, d4, d5, a1[w], a2[w], rd, ra)`
  * IOCSコールを実行します。`d0`～`d5`,`a1`,`a2` は呼び出しの際のCPUレジスタ値を指定します。`d0`以外は省略可能です。
    * 例: `x68k.iocs(x68k.i.B_KEYINP)`  # IOCS _B_KEYINPを実行します
  * CPUレジスタ引数には整数値を直接指定する他に、バッファオブジェクトを指定することも可能です。この場合の挙動はレジスタの種類によって異なります。
    * データレジスタの場合は、与えられたオブジェクトの先頭4バイトを整数値として使用します。
      * x68k.iocs()に渡されるPythonの引数は32bit符号付き整数として扱われるため、これに収まらない値(0x80000000のような)は直接引数として与えることができません。バッファオブジェクトを介して `d1=struct.pack('L',0x80000000))` のようにすることで渡せるようになります。
    * アドレスレジスタの場合は、与えられたオブジェクトの先頭アドレスが渡されます。
      * 与えられたバッファに書き込みを行うようなIOCSに対しては、名前を `a1w`, `a2w` にして`bytearray`のような書き換え可能なオブジェクトを与えます。
  * IOCS実行後のd0レジスタの値が関数の戻り値となります。他のレジスタの値が必要な場合は、`rd`, `ra`にそれぞれデータレジスタ、アドレスレジスタの個数を指定することでそれらのレジスタ値を並べたタプルを返します。
    * `rd=2`, `ra=1` を指定すると、関数の戻り値は `(d0,d1,d2,a1)` のタプルとなります。

* `x68k.d.<DOSコール名>`
  * DOSコール番号を定数で定義しています。
    * 例: `x68k.d.PRINT` = 0xff09
* `x68k.dos(callno [,arg])`
  * DOSコールを実行します。`callno`には呼び出すDOSコール番号を指定します。
    * 例: `x68k.dos(x68k.d.GETCHAR)`  # DOS _GETCHAR を実行します
  * DOSコール実行後のd0レジスタの値が関数の戻り値となります。
  * 追加の引数としてバッファオブジェクトを指定することができます。指定されたオブジェクトの内容がスタックに積み上げられてからDOSコールを実行します。オブジェクトのデータ配置、サイズは呼び出すDOSコールと正しく対応している必要があります。
    * 例:
      ```
      import x68k
      import struct
      import ctypes

      buf=bytearray(94)
      x68k.dos(x68k.d.GETDPB,struct.pack('hl',0,ctypes.addressof(buf)))
      ```
    * DOS _GETDPBは「1ワードのドライブ番号」「94バイトのバッファを指す1ロングワードのポインタ」をこの順にスタックに積んで呼び出す仕様なので、`struct.pack`のフォーマット文字列`'hl'` によってこのデータ配置を指定しています。

* `x68k.mpyaddr()`
  * MicroPython本体のメモリ上の開始アドレスを返します。デバッグ用です。

#### クラス `Super` -- スーパーバイザモード制御

* class `x68k.Super()`
  * Super オブジェクトを構築します。以下のように `with` ステートメントで使用することで、ブロック内をスーパーバイザモードで実行します。
  ```
  # ここまではユーザーモード

  with x68k.Super():
    # ここからスーパーバイザモード
    a = machine.mem32[0]    # スーパーバイザモードでアクセス

  # ここからはユーザーモード
  ```

* `x68k.issuper()`
  * スーパーバイザモードであれば `True` を、ユーザーモードであれば `False` を返します。
* `x68k.super([mode])`
  * スーパーバイザモードとユーザモードの間の切り替えを行います。
  * mode を省略または `True` であればスーパーバイザモードに、`False` であればユーザーモードに切り替えます。
  * 関数の戻り値として切り替え前のモードを返します。
  ```
  # ここまではユーザーモード

  s = x68k.super()

  # ここからスーパーバイザモード
  a = machine.mem32[0]    # スーパーバイザモードでアクセス

  x68k.super(s)

  # ここからはユーザーモード
  ```

#### クラス `GVRam` -- グラフィックス描画

* class `x68k.GVRam([page])`
  * GVRam オブジェクトを構築します。複数の描画ページを持つグラフィックスモードの場合は、pageに描画するページ番号を指定します。省略するとページ0を使用します。

以下のメソッドによって描画を行います。

* `GVRam.home(x, y [,all])` -- 表示位置指定
* `GVRam.window(x0, y0, x1, y1)` -- 描画範囲設定
* `GVRam.wipe()` -- 画面クリア
* `GVRam.pset(x, y, c)` -- ポイント描画
* `GVRam.point(x, y)` -- ポイントゲット
* `GVRam.line(x0, y0, x1, y1, c [,style])` -- 直線描画
* `GVRam.box(x0, y0, x1, y1, c [,style])` -- ボックス描画
* `GVRam.fill(x0, y0, x1, y1, c)` -- 塗りつぶしボックス描画
* `GVRam.circle(x, y, r, c [,start, end, ratio])` -- 円描画
* `GVRam.paint(x, y, c [,buf])` -- シードフィル描画
* `GVRam.symbol(x, y, str, xmag, ymag, c [,ftype, angle])` -- 文字列描画
* `GVRam.get(x0, y0, x1, y1, buf)` -- 範囲内のデータ取得
* `GVRam.put(x0, y0, x1, y1, buf)` -- 範囲内へデータ書き込み

* `x68k.vpage(page)`
  * グラフィック画面の表示ページを設定します。`page`のビット0～ビット3が各ページ番号に対応します。

  ```
  import x68k
  x68k.crtmod(16,True)
  x68k.vpage(1)   # ページ0のみ表示
  g = x68k.GVRam(0)   # ページ0のオブジェクト構築
  g.line(0, 0, 767, 511, 11)
  g.fill(100, 100, 200, 200, 9)
  g.symbol(10, 300, 'MicroPython!', 2, 2, 7)
  ```

## インラインアセンブラ

MicroPythonのインラインアセンブラ機能をサポートしています。
詳細は[こちらのドキュメント](README-inlineasm.md)を参照してください。
