#include <string.h>
#include "encode.h"

enum check_state
{
    DEFAULT,
    UTF8_1_1,
    UTF8_2_1, UTF8_2_2,
    UTF8_3_1, UTF8_3_2, UTF8_3_3,
    UTF8_4_1, UTF8_4_2, UTF8_4_3, UTF8_4_4,
    UTF8_5_1, UTF8_5_2, UTF8_5_3, UTF8_5_4, UTF8_5_5,
    BIG5_1
};

struct check
{
    enum check_state state;
    int good, bad, strange;
    wchar_t current;
};

static inline void utf8_check_next(struct check *ct, unsigned char c)
{
    if (!ct->state) {
        // 80-BF - bad start char
        if ((c & 0xC0) == 0x80) {
            ct->bad++;
            ct->state = DEFAULT;
            return;
        }

        // C0-DF - 2-byte sequence
        if ((c & 0xE0) == 0xC0) {
            ct->current = c & 0x1F;
            ct->state = UTF8_1_1;
            return;
        }

        // E0-EF - 3-byte sequence
        if ((c & 0xF0) == 0xE0) {
            ct->current = c & 0x0F;
            ct->state = UTF8_2_1;
            return;
        }

        // F0-F7 - 4-byte sequence
        if ((c & 0xF8) == 0xF0) {
            ct->current = c & 0x07;
            ct->state = UTF8_3_1;
            return;
        }

        // F8-FB - 5-byte sequence
        if ((c & 0xFC) == 0xF8) {
            ct->current = c & 0x03;
            ct->state = UTF8_4_1;
            return;
        }

        // FC-FD - 6-byte sequence
        if ((c & 0xFE) == 0xFC) {
            ct->current = c & 0x01;
            ct->state = UTF8_5_1;
            return;
        }

        // 00-7F, FE, FF - normal chars
        return;
    }

    // not 80-BF - bad sequence
    if ((c & 0xC0) != 0x80) {
        ct->bad++;
        ct->state = DEFAULT;
        return;
    }

    ct->current = (ct->current << 6) | (c & 0x3F);

    // 0080-07FF
    if (ct->state == UTF8_1_1) {
        if (ct->current < 0x80) {
            ct->strange++;
        }
        else {
            ct->good++;
        }
        ct->state = DEFAULT;
        return;
    }

    // 0800-FFFF
    if (ct->state == UTF8_2_2) {
        if (ct->current < 0x800 || (ct->current >= 0xD800 && ct->current <= 0xDFFF)) {
            ct->strange++;
        }
        else {
            ct->good++;
        }
        ct->state = DEFAULT;
        return;
    }

    // 00010000-001FFFFF
    if (ct->state == UTF8_3_3) {
        if (ct->current < 0x10000 || ct->current > 0x10FFFF) {
            ct->strange++;
        }
        else {
            ct->good++;
        }
        ct->state = DEFAULT;
        return;
    }

    // 00200000 - 7FFFFFFF
    if (ct->state == UTF8_4_4 || ct->state == UTF8_5_5) {
        ct->strange++;
        ct->state = DEFAULT;
        return;
    }

    ct->state++;
}

static inline void big5_check_next(struct check *ct, unsigned char c)
{
    if (!ct->state) {
        if (c >= 0x81 && c <= 0xFE)
            ct->state = BIG5_1;
        return;
    }

    if ((c >= 0x40 && c <= 0x7E) || (c >= 0xA1 && c <= 0xFE)) {
        ct->good++;
        ct->state = DEFAULT;
        return;
    }

    ct->bad++;
    ct->state = DEFAULT;
}

void utf8_check(const char* str, int *good, int *bad, int *strange, int flags)
{
    struct check ct;

    memset(&ct, 0, sizeof(ct));

    while (*str) {
        utf8_check_next(&ct, *(unsigned char*)str);
        if (((flags & ENC_CHECK_BREAK_ON_BAD) && ct.bad) ||
            ((flags & ENC_CHECK_BREAK_ON_STRANGE) && ct.strange))
                break;
        str++;
    }

    // unfinished sequence
    if (ct.state) ct.bad++;

    *good = ct.good;
    *bad = ct.bad;
    *strange = ct.strange;
}

void utf8_check_mem(const void* mem, size_t size, int *good, int *bad, int *strange, int flags)
{
    struct check ct;

    memset(&ct, 0, sizeof(ct));

    while (size--) {
        utf8_check_next(&ct, *(unsigned char*)mem);
        if (((flags & ENC_CHECK_BREAK_ON_BAD) && ct.bad) ||
            ((flags & ENC_CHECK_BREAK_ON_STRANGE) && ct.strange))
                break;
        mem++;
    }

    // unfinished sequence
    if (ct.state) ct.bad++;

    *good = ct.good;
    *bad = ct.bad;
    *strange = ct.strange;
}

void big5_check(const char* str, int *good, int *bad, int *strange, int flags)
{
    struct check ct;

    memset(&ct, 0, sizeof(ct));

    while (*str) {
        big5_check_next(&ct, *(unsigned char*)str);
        if (((flags & ENC_CHECK_BREAK_ON_BAD) && ct.bad))
            break;
        str++;
    }

    // unfinished sequence
    if (ct.state) ct.bad++;

    *good = ct.good;
    *bad = ct.bad;
    *strange = 0;
}

void big5_check_mem(const void* mem, size_t size, int *good, int *bad, int *strange, int flags)
{
    struct check ct;

    memset(&ct, 0, sizeof(ct));

    while (size--) {
        big5_check_next(&ct, *(unsigned char*)mem);
        if (((flags & ENC_CHECK_BREAK_ON_BAD) && ct.bad))
            break;
        mem++;
    }

    // unfinished sequence
    if (ct.state) ct.bad++;

    *good = ct.good;
    *bad = ct.bad;
    *strange = 0;
}
