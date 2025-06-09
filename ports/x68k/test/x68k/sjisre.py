import re

def print_spans(match, str):
    print("----")
    try:
        i = 0
        while True:
            print(match.span(i), match.start(i), match.end(i), str[match.start(i):match.end(i)])
            i += 1
    except IndexError:
        pass

s = "1234日本語5678"
m = re.match(r'([0-9]+)(([^0-9]+)([0-9]+))', s)
print_spans(m, s)

s = "日本語mojiretsu処理"
m = re.match(r'(日本語)?mojiretsu(処理)', s)
print_spans(m, s)

s = "mojiretsu処理"
m = re.match(r'(日本語)?mojiretsu(処理)', s)
print_spans(m, s)
