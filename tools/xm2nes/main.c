/*
    This file is part of xm2nes.

    xm2nes is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xm2nes is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xm2nes.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "xm2nes.h"

static char program_version[] = "xm2nes 1.0";

/* Prints usage message and exits. */
static void usage()
{
    printf(
        "Usage: xm2nes [--output=FILE] [--channels=CHANNELS]\n"
        "              [--order-start=OFFSET] [--order-end=OFFSET]\n"
        "              [--label-prefix=PREFIX]\n"
        "              [--instruments-map=FILE] [--verbose]\n"
        "              [--help] [--usage] [--version]\n"
        "              FILE\n");
    exit(0);
}

/* Prints help message and exits. */
static void help()
{
    printf("Usage: xm2nes [OPTION...] FILE\n"
           "xm2nes converts Fasttracker ][ eXtended Module (XM) files to Kent's NES music format.\n\n"
           "Options:\n\n"
           "  --output=FILE                   Store output in FILE\n"
           "  --channels=CHANNELS             Process only CHANNELS (0,1,2,3,4)\n"
           "  --order-start=OFFSET            Start offset in pattern order table (0)\n"
           "  --order-end=OFFSET              End offset in pattern order table (song_length-1)\n"
           "  --instruments-map=FILE          Read instrument mapping information from FILE\n"
           "  --label-prefix=PREFIX           Use PREFIX as the prefix of 6502 assembly labels\n"
           "  --verbose                       Print progress information to standard output\n"  
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

#define IS_SPACE(c) ( ((c) == '\t') || ((c) == ' ') )

static void eat_ws(char *s, int *i)
{
    while (IS_SPACE(s[*i])) (*i)++;
}

static int get_ident(char *s, int i)
{
    int len = 0;
    while (isalpha(s[i+len]))
        ++len;
    return len;
}

static int get_value(char *s, int i)
{
    int len = 0;
    if ((s[i+len] == '-') || isalnum(s[i+len])) {
        ++len;
        while (isalnum(s[i+len]))
            ++len;
    }
    return len;
}

static int parse_instruments_map_file(const char *path, struct instr_mapping *map)
{
    int ok;
    int lineno;
    char line[1024];
    FILE *fp = fopen(path, "rt");
    if (!fp) {
        fprintf(stderr, "xm2nes: failed to open `%s' for reading\n", path);
        return 0;
    }
    ok = 1;
    while (ok && fgets(line, 1023, fp) != NULL) {
        int source_instr = -1;
        int target_instr = -1;
        int transpose = 0;
        int pos = 0;
        ++lineno;
        if (line[pos] == '#')
            continue; /* Comment */
        while (line[pos] && (line[pos] != '\n')) {
            int len;
            int attr = -1;
            int val;
            eat_ws(line, &pos);
            len = get_ident(line, pos);
            if (!len) {
                fprintf(stderr, "%s:%d.%d: attribute name expected\n", path, lineno, pos+1);
                ok = 0;
                break;
            }
            if ((len == 6) && !strncmp(&line[pos], "source", 6)) {
                attr = 0;
            } else if ((len == 6) && !strncmp(&line[pos], "target", 6)) {
                attr = 1;
            } else if ((len == 9) && !strncmp(&line[pos], "transpose", 9)) {
                attr = 2;
            } else {
                fprintf(stderr, "%s:%d.%d: unknown attribute\n", path, lineno, pos+1);
                ok = 0;
                break;
            }
            pos += len;
            eat_ws(line, &pos);
            if (!line[pos] || line[pos] != ':') {
                fprintf(stderr, "%s:%d.%d: : expected\n", path, lineno, pos+1);
                ok = 0;
                break;
            }
            ++pos;
            eat_ws(line, &pos);
            len = get_value(line, pos);
            if (!len) {
                fprintf(stderr, "%s:%d.%d: value expected\n", path, lineno, pos+1);
                ok = 0;
                break;
            }
            val = strtol(&line[pos], 0, 0);
            switch (attr) {
                case 0: /* source */
                    source_instr = val;
                    break;
                case 1: /* target */
                    target_instr = val;
                    break;
                case 2: /* transpose */
                    transpose = val;
                    break;
            }
            pos += len;
        }
        if (ok) {
            if (source_instr <= 0) {
                fprintf(stderr, "%s:%d: source attribute not specified\n", path, lineno);
                ok = 0;
                break;
            }
            if (target_instr >= 0)
                map[source_instr-1].target_instr = target_instr;
            if (transpose != 0)
                map[source_instr-1].transpose = transpose;
        }
    }
    fclose(fp);
    return ok;
}

/**
  Program entrypoint.
*/
int main(int argc, char *argv[])
{
    int verbose = 0;
    const char *input_filename = 0;
    const char *output_filename = 0;
    const char *instruments_map_filename = 0;
    const char *label_prefix = 0;
    struct xm2nes_options options;
    struct instr_mapping instr_map[128];
    {
        unsigned char i;
        for (i = 0; i < 128; ++i) {
            instr_map[i].target_instr = i;
            instr_map[i].transpose = 0;
        }
    }
    options.instr_map = instr_map;
    options.channels = 0x1F;
    options.label_prefix = 0;
    options.order_start_offset = 0;
    options.order_end_offset = -1;
    /* Process arguments. */
    {
        char *p;
        while ((p = *(++argv))) {
            if (!strncmp("--", p, 2)) {
                const char *opt = &p[2];
                if (!strncmp("output=", opt, 7)) {
                    output_filename = &opt[7];
                } else if (!strncmp("channels=", opt, 9)) {
                    const char *p = &opt[9];
                    options.channels = 0;
                    if (*p) {
                        options.channels |= 1 << (*p - '0');
                        while (*(++p)) {
                            if (*(p++) != ',')
                                break;
                            if (*p)
                                options.channels |= 1 << (*p - '0');
			}
		    }
                    options.channels &= 0x1F;
                } else if (!strncmp("instruments-map=", opt, 16)) {
                    instruments_map_filename = &opt[16];
                } else if (!strncmp("label-prefix=", opt, 13)) {
                    label_prefix = &opt[13];
                } else if (!strncmp("order-end=", opt, 10)) {
                    options.order_end_offset = strtol(&opt[10], 0, 0);
                } else if (!strncmp("order-start=", opt, 12)) {
                    options.order_start_offset = strtol(&opt[12], 0, 0);
                } else if (!strcmp("verbose", opt)) {
                    verbose = 1;
                } else if (!strcmp("help", opt)) {
                    help();
                } else if (!strcmp("usage", opt)) {
                    usage();
                } else if (!strcmp("version", opt)) {
                    version();
                } else {
                    fprintf(stderr, "xm2nes: unrecognized option `%s'\n"
			    "Try `xm2nes --help' or `xm2nes --usage' for more information.\n", p);
                    return(-1);
                }
            } else {
                input_filename = p;
            }
        }
    }

    if (!input_filename) {
        fprintf(stderr, "xm2nes: no filename given\n"
                        "Try `xm2nes --help' or `xm2nes --usage' for more information.\n");
        return(-1);
    }

    if (!options.channels) {
        fprintf(stderr, "xm2nes: --channels argument needs to include at least one channel\n");
        return(-1);
    }

    if (instruments_map_filename) {
        if (!parse_instruments_map_file(instruments_map_filename, instr_map))
            return(-1);
    }

    {
        struct xm xm;
        FILE *out;
        if (!output_filename)
            out = stdout;
        else {
            out = fopen(output_filename, "wt");
            if (!out) {
                fprintf(stderr, "xm2nes: failed to open `%s' for writing\n", output_filename);
                return(-1);
            }
        }

        {
            FILE *in;
            in = fopen(input_filename, "rb");
            if (!in) {
                fprintf(stderr, "xm2nes: failed to open `%s' for reading\n", input_filename);
                return(-1);
            }
            if (verbose)
                fprintf(stdout, "Reading `%s'...\n", input_filename);
            xm_read(in, &xm);
            if (verbose)
                fprintf(stdout, "OK.\n");
        }

        if (verbose)
            xm_print_header(&xm.header, stdout);

        if (verbose)
            fprintf(stdout, "Converting...\n");

        {
            const char *begin;
            char *prefix;
            int len;
            if (label_prefix) {
                begin = label_prefix;
                len = strlen(begin);
            } else {
                /* Use basename of input filename as prefix */
                char *last_dot;
                begin = strrchr(input_filename, '/');
                if (begin)
                    ++begin;
                else
                    begin = input_filename;
                last_dot = strrchr(begin, '.');
                if (!last_dot)
                    len = strlen(begin);
                else
                    len = last_dot - begin;
            }
            prefix = (char *)malloc(len + 2);
            prefix[len] = '_';
            prefix[len+1] = '\0';
            strncpy(prefix, begin, len);
            options.label_prefix = prefix;

            convert_xm_to_nes(&xm, &options, out);

            free(prefix);
        }
        if (output_filename)
            fclose(out);

        if (verbose)
            fprintf(stdout, "Done.\n");

        xm_destroy(&xm);
    }
    return 0;
}
