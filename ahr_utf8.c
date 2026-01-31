#include "ahr_utf8.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

int ahr_is_valid_unicode(uint32_t cp)
{
    if (cp > 0x10FFFFu) return 0;
    if (cp >= 0xD800u && cp <= 0xDFFFu) return 0; /* surrogates */
    return 1;
}

void ahr_print_uplus(uint32_t cp)
{
    if (cp <= 0xFFFFu)
        printf("U+%04X\n", (unsigned)cp);
    else
        printf("U+%06X\n", (unsigned)cp);
}

int ahr_utf8_encode(uint32_t cp, unsigned char out[4])
{
    if (!out) return -1;
    if (!ahr_is_valid_unicode(cp)) return -1;

    if (cp <= 0x7Fu) {
        out[0] = (unsigned char)cp;
        return 1;
    }
    if (cp <= 0x7FFu) {
        out[0] = (unsigned char)(0xC0u | (cp >> 6));
        out[1] = (unsigned char)(0x80u | (cp & 0x3Fu));
        return 2;
    }
    if (cp <= 0xFFFFu) {
        out[0] = (unsigned char)(0xE0u | (cp >> 12));
        out[1] = (unsigned char)(0x80u | ((cp >> 6) & 0x3Fu));
        out[2] = (unsigned char)(0x80u | (cp & 0x3Fu));
        return 3;
    }

    /* 0x10000..0x10FFFF */
    out[0] = (unsigned char)(0xF0u | (cp >> 18));
    out[1] = (unsigned char)(0x80u | ((cp >> 12) & 0x3Fu));
    out[2] = (unsigned char)(0x80u | ((cp >> 6) & 0x3Fu));
    out[3] = (unsigned char)(0x80u | (cp & 0x3Fu));
    return 4;
}

int ahr_utf8_decode_one(const unsigned char *buf, size_t len, uint32_t *out_cp)
{
    if (!buf || !out_cp) return -1;
    if (len == 0) return 0;

    unsigned char b0 = buf[0];

    /* 1-byte ASCII */
    if (b0 <= 0x7Fu) {
        *out_cp = (uint32_t)b0;
        return 1;
    }

    /* continuation byte cannot start */
    if (b0 >= 0x80u && b0 <= 0xBFu) return -1;

    int need = 0;
    uint32_t cp = 0;

    if (b0 >= 0xC2u && b0 <= 0xDFu) {
        need = 2;
        cp = (uint32_t)(b0 & 0x1Fu);
    } else if (b0 >= 0xE0u && b0 <= 0xEFu) {
        need = 3;
        cp = (uint32_t)(b0 & 0x0Fu);
    } else if (b0 >= 0xF0u && b0 <= 0xF4u) {
        need = 4;
        cp = (uint32_t)(b0 & 0x07u);
    } else {
        return -1; /* invalid lead */
    }

    if (len < (size_t)need) return 0;

    for (int i = 1; i < need; i++) {
        unsigned char bx = buf[i];
        if ((bx & 0xC0u) != 0x80u) return -1;
        cp = (cp << 6) | (uint32_t)(bx & 0x3Fu);
    }

    /* reject overlong */
    if (need == 2 && cp < 0x80u) return -1;
    if (need == 3 && cp < 0x800u) return -1;
    if (need == 4 && cp < 0x10000u) return -1;

    /* enforce boundary rules */
    if (need == 3) {
        unsigned char b1 = buf[1];
        if (b0 == 0xE0u && b1 < 0xA0u) return -1; /* overlong */
        if (b0 == 0xEDu && b1 >= 0xA0u) return -1; /* surrogate */
    }
    if (need == 4) {
        unsigned char b1 = buf[1];
        if (b0 == 0xF0u && b1 < 0x90u) return -1; /* overlong */
        if (b0 == 0xF4u && b1 >= 0x90u) return -1; /* > U+10FFFF */
    }

    if (!ahr_is_valid_unicode(cp)) return -1;

    *out_cp = cp;
    return need;
}

/* ---- parsing helpers ---- */

static const char* skip_ws(const char *s)
{
    while (s && *s && isspace((unsigned char)*s)) s++;
    return s ? s : "";
}

static int is_hex_digit_str(const char *p)
{
    if (!p || !*p) return 0;
    for (; *p; p++) {
        if (!isxdigit((unsigned char)*p)) return 0;
    }
    return 1;
}

static int contains_hex_alpha(const char *p)
{
    for (; p && *p; p++) {
        unsigned char c = (unsigned char)*p;
        if ((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) return 1;
    }
    return 0;
}

/* Policy: plain tokens of length 4–6 that are all hex digits are treated as HEX.
 * This guarantees: "0041" => U+0041 (A), not decimal 41.
 */
int ahr_parse_codepoint(const char *s, uint32_t *out_cp)
{
    if (!s || !out_cp) return -1;

    s = skip_ws(s);

    /* take token until whitespace */
    char tok[128];
    size_t i = 0;
    while (s[i] && !isspace((unsigned char)s[i]) && i + 1 < sizeof(tok)) {
        tok[i] = s[i];
        i++;
    }
    tok[i] = '\0';
    if (i == 0) return -1;

    const char *p = tok;
    int base = 10;

    if ((p[0] == 'U' || p[0] == 'u') && p[1] == '+') {
        p += 2;
        base = 16;
    } else if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        p += 2;
        base = 16;
    } else {
        size_t len = strlen(p);
        if (is_hex_digit_str(p) && (len >= 4 && len <= 6)) {
            base = 16;
        } else if (contains_hex_alpha(p)) {
            base = 16;
        } else {
            base = 10;
        }
    }

    if (*p == '\0') return -1;

    char *end = NULL;
    unsigned long v = strtoul(p, &end, base);
    if (!end || *end != '\0') return -1;
    if (v > 0x10FFFFul) return -1;

    uint32_t cp = (uint32_t)v;
    if (!ahr_is_valid_unicode(cp)) return -1;

    *out_cp = cp;
    return 0;
}
