#ifndef AHR_UTF8_H
#define AHR_UTF8_H

#include <stdint.h>
#include <stddef.h>

/* Unicode scalar validation:
 * valid: 0x0000..0x10FFFF excluding surrogate range 0xD800..0xDFFF
 */
int ahr_is_valid_unicode(uint32_t cp);

/* Print as U+XXXX or U+XXXXXX with trailing newline (diagnostic helper). */
void ahr_print_uplus(uint32_t cp);

/* Parse Unicode code point from text forms:
 *   - U+0041 / u+0041
 *   - 0x41
 *   - 0041  (HEX by policy, length 4–6 all-hex digits)
 *   - 1F512 (HEX by policy if len 4–6, or contains A-F)
 *   - decimal (fallback if pure digits and not 4–6 length)
 *
 * Returns 0 on success, -1 on error.
 */
int ahr_parse_codepoint(const char *s, uint32_t *out_cp);

/* Encode Unicode scalar `cp` into UTF-8 bytes.
 * out[4] receives up to 4 bytes.
 * Returns bytes written (1..4), or -1 on invalid cp.
 */
int ahr_utf8_encode(uint32_t cp, unsigned char out[4]);

/* Decode a single UTF-8 sequence from a buffer.
 * Returns:
 *   1..4 : bytes consumed
 *   0    : need more bytes
 *  -1    : invalid sequence
 */
int ahr_utf8_decode_one(const unsigned char *buf, size_t len, uint32_t *out_cp);

#endif /* AHR_UTF8_H */
