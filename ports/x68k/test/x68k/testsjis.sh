#!/bin/bash

iconv -f sjis -t utf-8 sjisre.py | python3 > sjisre.exp.log
iconv -f sjis -t utf-8 sjisre.log | tr -d '\r' | diff sjisre.exp.log -
if [ $? -eq 0 ]; then
    diff sjisstr.log sjisstr.exp
else
    false
fi
if [ $? -eq 0 ]; then
    echo "### SJIS test OK ###"
else
    echo "XXX SJIS test NG XXX"
fi
