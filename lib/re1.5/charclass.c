#include "re1.5.h"

int _re1_5_classmatch(const unichar *pc, const char *sp)
{
    unichar ch = re_get_char(sp);
    // pc points to "cnt" byte after opcode
    int is_positive = (pc[-1] == Class);
    int cnt = *pc++;
    while (cnt--) {
        if (*pc == RE15_CLASS_NAMED_CLASS_INDICATOR) {
            if (_re1_5_namedclassmatch(pc + 1, sp)) {
                return is_positive;
            }
        } else {
            if (ch >= *pc && ch <= pc[1]) {
                return is_positive;
            }
        }
        pc += 2;
    }
    return !is_positive;
}

int _re1_5_namedclassmatch(const unichar *pc, const char *sp)
{
    unichar ch = re_get_char(sp);
    // pc points to name of class
    int off = (*pc >> 5) & 1;
    if ((*pc | 0x20) == 'd') {
        if (!(ch >= '0' && ch <= '9')) {
            off ^= 1;
        }
    } else if ((*pc | 0x20) == 's') {
        if (!(ch == ' ' || (ch >= '\t' && ch <= '\r'))) {
            off ^= 1;
        }
    } else { // w
        if (!((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '_')) {
            off ^= 1;
        }
    }
    return off;
}
