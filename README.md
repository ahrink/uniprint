# uniprint
Adds uniprint, a small C99 utility that converts Unicode code points (e.g. 0041, U+1F512) into UTF-8 bytes. Argument-only, no trailing newline, designed for safe composition in POSIX sh. Includes strict Unicode scalar validation and UTF-8 encoding core.

# uniprint

`uniprint` is a small C99 utility that converts a Unicode code point into its
UTF-8 byte sequence and writes it to standard output.

The program is argument-only (no stdin piping) and produces **no trailing
newline by default**, making it suitable for safe composition in POSIX `sh`
scripts and shell functions.

---

## Synopsis

```sh
uniprint CODEPOINT
uniprint --help

Description

Shells are inconsistent in their handling of escape sequences and Unicode
output. uniprint centralizes Unicode parsing and UTF-8 encoding in C, where
byte-level behavior is explicit and portable.

The tool accepts a textual representation of a Unicode code point, validates
it as a Unicode scalar value, encodes it as UTF-8, and writes the resulting
bytes to stdout.

Code Point Format

The CODEPOINT argument may be specified in any of the following forms:

U+XXXX

XXXX

0xXXXX

Hexadecimal is assumed for:

U+ prefixed values

0x prefixed values

plain hexadecimal tokens of length 4–6

Valid Unicode scalar values are:

U+0000 to U+10FFFF

excluding the surrogate range U+D800–U+DFFF

Examples
uniprint 0041

uniprint U+1F512


POSIX sh usage:

sym=$(uniprint 0041)
printf '%s\n' "$sym"


Shell function wrapper:

uniFN() {
    uni=$1
    art=$(uniprint "$uni")
    printf '%s' "$art"
}

Exit Status

0 success

>0 invalid input or encoding error

Error messages are written to standard error.

Notes

uniprint does not interpret shell escape sequences.

For emitting raw UTF-8 byte sequences from scripts, tools such as awk
may be used.

UTF-8 encoding and Unicode validation are implemented in ahr_utf8.
