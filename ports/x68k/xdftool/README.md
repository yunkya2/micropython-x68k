# XDF ファイル操作ツール xdftool.py

## 概要

xdftool.py は、XDFファイル (X680x0 エミュレータや X68000Z で用いられるフロッピーディスクイメージファイル) を作成・展開するための Python スクリプトです。

Linux などの Python 実行環境の他に、MicroPython for X680x0 を用いることで X68k 上でも実行できます。

## 実行方法

* Linux 上では、 `xdftool.py` にコマンドラインオプションを付けて直接実行します。
  * `xdftool.py <コマンドライン...>`
* X68k から動かす場合、micropython.x を PATH 環境変数、xdftool.py を MICROPYPATH 環境変数で指定されたディレクトリに置き、以下のように実行します
  * `micropython xdftool.py <コマンドライン...>`

## コマンドラインオプション

### XDF ファイルの表示・展開

* `xdftool.py t <xdf file>`
  * 指定した `<xdf file>` のディスクイメージに含まれるファイル一覧を表示します (list)。
* `xdftool.py x <XDF file> [<files>...]`
  * 指定した `<xdf file>` のディスクイメージ内のファイルを、カレントディレクトリ以下に展開します (extract)。
  * `<files>` を省略した場合はイメージ内のすべてのファイルを、指定した場合は該当ファイルのみを展開します。
  * list, extract とも、ディスクイメージのブートセクタの BPB 領域を参照してディスクフォーマットを判別します。
    * IBM-PC の 2HDフォーマット (1440KB) の読み込みも可能ですが、VFAT の long file name には対応していないためファイル名は 8+3 形式となります。
    * BPB 領域を判別できなかった場合は X68k の標準フォーマットである 2HD (1232KB) として扱います。
  * MicroPython から実行した場合、展開するファイルにはディスクイメージのタイムスタンプ情報が反映されません (MicroPython に `os.utime()` がないため)。

### XDF ファイルの作成

* `xdftool.py c [<format>] <xdf file> [<files>...]`
  * カレントディレクトリ以下にある `<files>` のファイルを収めたディスクイメージ `<xdf file>` を新規作成します (create)。
  * `<format>` には作成するイメージファイルのディスクフォーマットを指定します。
    * (省略) : 2HD (1232KB)
    * `/5` : 2HC (1200KB)
    * `/8` : 2DD (640KB)
    * `/9` : 2DD (720KB)
    * `/4` : 2HQ (1440KB)

## ライセンス

MIT ライセンス
