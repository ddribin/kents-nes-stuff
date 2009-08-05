/*
    This file is part of bin2asm.

    bin2asm is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    bin2asm is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with bin2asm.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char program_version[] = "bin2asm 1.0";

/* Prints usage message and exits. */
static void usage()
{
    printf(
        "Usage: bin2asm [--offset=NUM] [--length=NUM] [--output=FILE]\n"
        "               [--help] [--usage] [--version]\n"
        "                FILE\n");
    exit(0);
}

/* Prints help message and exits. */
static void help()
{
    printf("Usage: bin2asm [OPTION...] FILE\n\n"
           "  --offset=NUM                    Skip NUM bytes of input\n"
           "  --length=NUM                    Process at most NUM bytes of input\n"
           "  --output=FILE                   Write output to FILE\n"
           "  --help                          Give this help list\n"
           "  --usage                         Give a short usage message\n"
           "  --version                       Print program version\n");
    exit(0);
}

/* Prints version and exits. */
static void version()
{
    printf("%s\n", program_version);
    exit(0);
}

long bin2asm(FILE *in, FILE *out, long length)
{
    long i = 0;
    int ch = fgetc(in);
    while ((ch != -1) && ((length == -1) || (i < length))) {
        int m = (i % 16);
        if (m == 0)
            fwrite(".db ", 1, 4, out);
        else
            fputc(',', out);
        fprintf(out, "$%.2X", ch);
        if (m == 15)
            fputc('\n', out);
        ch = fgetc(in);
        ++i;
    }
    fputc('\n', out);
    return i;
}

/**
 * Program entrypoint.
 */
int main(int argc, char **argv)
{
    long offset = -1;
    long length = -1;
    const char *input_filename = 0;
    const char *output_filename = 0;
    /* Process arguments. */
    {
        char *p;
        while ((p = *(++argv))) {
            if (!strncmp("--", p, 2)) {
                const char *opt = &p[2];
                if (!strncmp("offset=", opt, 7)) {
                    offset = strtol(&opt[7], 0, 0);
                } else if (!strncmp("length=", opt, 7)) {
                    length = strtol(&opt[7], 0, 0);
                } else if (!strncmp("output=", opt, 7)) {
                    output_filename = &opt[7];
                } else if (!strcmp("help", opt)) {
                    help();
                } else if (!strcmp("usage", opt)) {
                    usage();
                } else if (!strcmp("version", opt)) {
                    version();
                } else {
                    fprintf(stderr, "unrecognized option `%s'\n", p);
                    return(-1);
                }
            } else {
                input_filename = p;
            }
        }
    }

    if (!input_filename) {
        fprintf(stderr, "bin2asm: no filename given\n");
        return(-1);
    }

    {
        FILE *out;
        if (!output_filename)
            out = stdout;
        else {
            out = fopen(output_filename, "wt");
            if (!out) {
                fprintf(stderr, "bin2asm: failed to open `%s' for writing\n", output_filename);
                return(-1);
            }
        }
        FILE *in = fopen(input_filename, "rb");
        if (!in) {
            fprintf(stderr, "bin2asm: failed to open `%s' for reading\n", input_filename);
            return(-1);
        }
        if (offset != -1)
            fseek(in, offset, SEEK_CUR);
        bin2asm(in, out, length);
        fclose(in);
        fclose(out);
    }

    return 0;
}
