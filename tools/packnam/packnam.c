/*
    This file is part of packnam.

    packnam is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    packnam is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with packnam.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static char program_version[] = "packnam 1.0";

/* Prints usage message and exits. */
static void usage()
{
    printf(
        "Usage: packnam [--width=NUM] [--vram-address=NUM]\n"
        "               [--output=FILE] [--zero-terminate]\n"
        "               [--verbose]\n"
        "               [--help] [--usage] [--version]\n"
        "                FILE\n");
    exit(0);
}

/* Prints help message and exits. */
static void help()
{
    printf("Usage: packnam [OPTION...] FILE\n\n"
           "packnam encodes a raw NES nametable.\n\n"
           "Options:\n\n"
           "  --width=NUM                     Use NUM as nametable width (tiles per row) (32)\n"
           "  --vram-address=NUM              Use NUM as VRAM start address (0x2000)\n"
           "  --output=FILE                   Store encoded data in FILE\n"
           "  --zero-terminate                Zero-terminate the output\n"
           "  --verbose                       Print statistics\n"
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

/*
  Packs the given \a nametable and stores the result int \a out and
  its size in \a out_sz.
*/
void pack_nametable(const unsigned char *nametable, int nametable_sz,
                    int width, int vram_address, int zero_terminate,
                    unsigned char **out, int *out_sz)
{
    int out_pos = 0;
    int buf_sz = 0;
    int y = 0;
    int in_pos = 0;
    assert(nametable);
    assert(out);
    assert(out_sz);
    *out = 0;
    while (y * width < nametable_sz) {
	unsigned char rlech;
	int count;
	int count_pos;
	int x = 0;
	int state = 0;
	int start_rle = 0;
	int addr = vram_address + (y * 32);
	while (x < width) {
	    const unsigned char ch = nametable[in_pos];
	    switch (state) {
		case 0:
                    start_rle = (x + 4 <= width)
				&& (nametable[in_pos+1] == ch)
				&& (nametable[in_pos+2] == ch)
				&& (nametable[in_pos+3] == ch);
                    /* fallthrough */
		case 1:
                    if (ch == 0) {
                        /* don't encode 0s */
                        start_rle = 0;
                        ++in_pos;
                        ++x;
                        ++addr;
                        break;
                    }
                    if (out_pos + 2 >= buf_sz) {
                        buf_sz += 64;
                        *out = (unsigned char *)realloc(*out, buf_sz);
                    }
                    (*out)[out_pos++] = (unsigned char)(addr >> 8);
                    (*out)[out_pos++] = (unsigned char)(addr & 255);
                    if (start_rle) {
                        /* start of RLE run */
                        count = 4;
                        rlech = ch;
                        in_pos += 4;
                        x += 4;
                        state = 2;
                    } else {
                        /* start of non-RLE run */
                        count_pos = out_pos;
                        if (out_pos + 2 >= buf_sz) {
                            buf_sz += 64;
                            *out = (unsigned char *)realloc(*out, buf_sz);
                        }
                        (*out)[out_pos++] = 0; /* count backpatched later */
                        count = 1;
                        (*out)[out_pos++] = ch;
                        ++in_pos;
                        ++x;
                        state = 3;
                    }
                    break;

		case 2:
                    if (ch == rlech) {
                        ++count;
                        ++in_pos;
                        ++x;
                    }
                    if ((ch != rlech) || (count == 0x3F)) {
                        /* end RLE run */
                        if (out_pos + 2 >= buf_sz) {
                            buf_sz += 64;
                            *out = (unsigned char *)realloc(*out, buf_sz);
                        }
                        (*out)[out_pos++] = (unsigned char)(0x40 | count);
                        (*out)[out_pos++] = rlech;
                        addr += count;
                        state = 0;
                    }
                    break;

		case 3:
                    start_rle = ((x + 4 <= width)
				 && (nametable[in_pos+1] == ch)
				 && (nametable[in_pos+2] == ch)
				 && (nametable[in_pos+3] == ch));
                    if (start_rle) {
                        /* end non-RLE run */
                        (*out)[count_pos] = count;
                        addr += count;
                        state = 1;
                    } else {
                        if (out_pos + 1 >= buf_sz) {
                            buf_sz += 64;
                            *out = (unsigned char *)realloc(*out, buf_sz);
                        }
                        (*out)[out_pos++] = ch;
                        ++in_pos;
                        ++x;
                        ++count;
                        if (count == 0x3F) {
                            /* end non-RLE run */
                            (*out)[count_pos] = count;
                            addr += count;
                            state = 1;
                        }
                    }
                    break;
	    }
	}
	switch (state) {
	    case 0:
	    case 1:
                break;

	    case 2:
                /* finish RLE run */
                if (out_pos + 2 >= buf_sz) {
                    buf_sz += 64;
                    *out = (unsigned char *)realloc(*out, buf_sz);
                }
                (*out)[out_pos++] = (unsigned char)(0x40 | count);
                (*out)[out_pos++] = rlech;
                break;

	    case 3:
                /* finish non-RLE run */
                (*out)[count_pos] = count;
                break;
	}
	++y;
    }
    if (zero_terminate) {
	if (out_pos + 1 >= buf_sz) {
	    buf_sz += 1;
	    *out = (unsigned char *)realloc(*out, buf_sz);
	}
        (*out)[out_pos++] = 0;
    }
    *out_sz = out_pos;
}

/**
 * Program entrypoint.
 */
int main(int argc, char **argv)
{
    unsigned char nametable[1024];
    int nametable_sz;
    unsigned char *out;
    int out_sz;
    int width = 32;
    int vram_address = 0x2000;
    int verbose = 0;
    int zero_terminate = 0;
    const char *input_filename = 0;
    const char *output_filename = 0;
    /* Process arguments. */
    {
        char *p;
        while ((p = *(++argv))) {
            if (!strncmp("--", p, 2)) {
                const char *opt = &p[2];
                if (!strncmp("width=", opt, 6)) {
                    width = strtol(&opt[6], 0, 0);
                } else if (!strncmp("vram-address=", opt, 13)) {
                    vram_address = strtol(&opt[13], 0, 0);
                } else if (!strncmp("output=", opt, 7)) {
                    output_filename = &opt[7];
                } else if (!strcmp("zero-terminate", opt)) {
                    zero_terminate = 1;
                } else if (!strcmp("verbose", opt)) {
                    verbose = 1;
                } else if (!strcmp("help", opt)) {
                    help();
                } else if (!strcmp("usage", opt)) {
                    usage();
                } else if (!strcmp("version", opt)) {
                    version();
                } else {
                    fprintf(stderr, "packnam: unrecognized option `%s'\n"
			    "Try `packnam --help' or `packnam --usage' for more information.\n", p);
                    return(-1);
                }
            } else {
                input_filename = p;
            }
        }
    }

    if (!input_filename) {
        fprintf(stderr, "packnam: no filename given\n"
                        "Try `packnam --help' or `packnam --usage' for more information.\n");
        return(-1);
    }

    /* Read input file */
    if (verbose)
        fprintf(stdout, "reading `%s'\n", input_filename);
    {
        FILE *fp = fopen(input_filename, "rb");
        if (!fp) {
            fprintf(stderr, "packnam: failed to open `%s' for reading\n", input_filename);
            return(-1);
        }

        nametable_sz = fread(nametable, 1, 1024, fp);
        fclose(fp);
    }

    /* Compress */
    if (verbose)
        fprintf(stdout, "processing\n");
    pack_nametable(nametable, nametable_sz, width, vram_address, zero_terminate, &out, &out_sz);

    /* Write output */
    if (!output_filename)
        output_filename = "packnam.dat";
    if (verbose)
        fprintf(stdout, "writing data to `%s'\n", output_filename);
    {
        FILE *fp = fopen(output_filename, "wb");
        fwrite(out, 1, out_sz, fp);
        fclose(fp);
    }

    if (verbose)
        fprintf(stdout, "compressed size: %d%%\n", (out_sz*100) / nametable_sz);

    free(out);

    /* Success */
    return 0;
}
