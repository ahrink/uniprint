#include "ahr_utf8.h"

#include <stdio.h>
#include <string.h>

static void usage(void)
{
    /* Man-style --help. */
    fputs(
        "NAME\n"
        "    uniprint — print a Unicode code point as UTF-8\n"
        "\n"
        "SYNOPSIS\n"
        "    uniprint CODEPOINT\n"
        "    uniprint --help\n"
        "\n"
        "DESCRIPTION\n"
        "    uniprint converts a Unicode code point into its UTF-8 byte sequence\n"
        "    and writes it to standard output.\n"
        "\n"
        "    The output contains no trailing newline.\n"
        "\n"
        "CODEPOINT FORMAT\n"
        "    CODEPOINT may be specified in one of the following forms:\n"
        "\n"
        "        U+XXXX\n"
        "        XXXX\n"
        "        0xXXXX\n"
        "\n"
        "    Hexadecimal is assumed for:\n"
        "        - U+ prefix\n"
        "        - 0x prefix\n"
        "        - plain hex tokens of length 4–6\n"
        "\n"
        "    Valid Unicode scalar values are:\n"
        "        U+0000 to U+10FFFF, excluding U+D800–U+DFFF.\n"
        "\n"
        "EXAMPLES\n"
        "    uniprint 0041\n"
        "    uniprint U+0041\n"
        "    uniprint 1F512\n"
        "    uniprint U+1F512\n"
        "\n"
        "POSIX SH USAGE\n"
        "    sym=$(uniprint 0041)\n"
        "    printf '%s\\n' \"$sym\"\n"
        "\n"
        "EXIT STATUS\n"
        "    0   success\n"
        "    >0  invalid input or encoding error\n",
        stdout
    );
}

int main(int argc, char **argv)
{
    if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
        usage();
        return 0;
    }

    if (argc != 2) {
        usage();
        return 2;
    }

    uint32_t cp = 0;
    if (ahr_parse_codepoint(argv[1], &cp) != 0) {
        fprintf(stderr, "uniprint: invalid codepoint '%s'\n", argv[1]);
        return 2;
    }

    unsigned char out[4];
    int n = ahr_utf8_encode(cp, out);
    if (n < 0) {
        fprintf(stderr, "uniprint: cannot encode U+%X\n", (unsigned)cp);
        return 2;
    }

    if (fwrite(out, 1, (size_t)n, stdout) != (size_t)n) {
        fprintf(stderr, "uniprint: write error\n");
        return 1;
    }

    return 0; /* no newline by design */
}
