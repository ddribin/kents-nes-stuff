/*
    This file is part of img2nes.

    img2nes is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    img2nes is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with img2nes.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#include <QtGui>

static const unsigned char nes_palette[] = {
    0x75, 0x75, 0x75,
    0x27, 0x1B, 0x8F,
    0x00, 0x00, 0xAB,
    0x47, 0x00, 0x9F,
    0x8F, 0x00, 0x77,
    0xAB, 0x00, 0x13,
    0xA7, 0x00, 0x00,
    0x7F, 0x0B, 0x00,
    0x43, 0x2F, 0x00,
    0x00, 0x47, 0x00,
    0x00, 0x51, 0x00,
    0x00, 0x3F, 0x17,
    0x1B, 0x3F, 0x5F,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0xBC, 0xBC, 0xBC,
    0x00, 0x73, 0xEF,
    0x23, 0x3B, 0xEF,
    0x83, 0x00, 0xF3,
    0xBF, 0x00, 0xBF,
    0xE7, 0x00, 0x5B,
    0xDB, 0x2B, 0x00,
    0xCB, 0x4F, 0x0F,
    0x8B, 0x73, 0x00,
    0x00, 0x97, 0x00,
    0x00, 0xAB, 0x00,
    0x00, 0x93, 0x3B,
    0x00, 0x83, 0x8B,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF,
    0x3F, 0xBF, 0xFF,
    0x5F, 0x97, 0xFF,
    0xA7, 0x8B, 0xFD,
    0xF7, 0x7B, 0xFF,
    0xFF, 0x77, 0xB7,
    0xFF, 0x77, 0x63,
    0xFF, 0x9B, 0x3B,
    0xF3, 0xBF, 0x3F,
    0x83, 0xD3, 0x13,
    0x4F, 0xDF, 0x4B,
    0x58, 0xF8, 0x98,
    0x00, 0xEB, 0xDB,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF,
    0xAB, 0xE7, 0xFF,
    0xC7, 0xD7, 0xFF,
    0xD7, 0xCB, 0xFF,
    0xFF, 0xC7, 0xFF,
    0xFF, 0xC7, 0xDB,
    0xFF, 0xBF, 0xB3,
    0xFF, 0xDB, 0xAB,
    0xFF, 0xE7, 0xA3,
    0xE3, 0xFF, 0xA3,
    0xAB, 0xF3, 0xBF,
    0xB3, 0xFF, 0xCF,
    0x9F, 0xFF, 0xF3,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00 };

#define ABS(x) ((x) < 0 ? -(x) : x)
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define NES_R(i) nes_palette[i*3+0]
#define NES_G(i) nes_palette[i*3+1]
#define NES_B(i) nes_palette[i*3+2]

/**
  Returns the index of the NES palette entry that most closely
  matches the given triple \a r, \a g, \a b.
*/
static int nearest_nes_color(int r, int g, int b)
{
    int i;
    int ret = 0;
    int d = INT_MAX;
    for (i = 0; i < 64; ++i) {
        int nr = NES_R(i);
        int ng = NES_G(i);
        int nb = NES_B(i);
        int dd = ABS(nr - r) + ABS(ng - g) + ABS(nb - b);
        if (dd < d) {
	    ret = i;
            d = dd;
        }
    }
    if (ret == 0x0D)
        ret = 0x0F; // by convention, represent black by 0x0F
    return ret;
}

/**
  Comparison function used for qsort.
*/
static int compare_nes_colors(const void *p1, const void *p2)
{
    unsigned char c1 = *(const unsigned char *)p1;
    unsigned char c2 = *(const unsigned char *)p2;
    unsigned int v1 = NES_R(c1) + NES_G(c1) + NES_B(c1);
    unsigned int v2 = NES_R(c2) + NES_G(c2) + NES_B(c2);
    if (v1 == v2)
        return 0;
    else if (v1 < v2)
        return -1;
    return 1;
}

/**
  Returns the difference between colors \a c1 and \a c2.
*/
static int nes_colors_diff(unsigned char c1, unsigned char c2)
{
    return ABS(NES_R(c1) - NES_R(c2)) + ABS(NES_G(c1) - NES_G(c2)) + ABS(NES_B(c1) - NES_B(c2));
}

/**
  Reads RAW 24-bit RGB image data from the file named by \a filename.
  Stores the data in \a raw_data.
*/
static int read_image(const char *filename, int width, int height,
                      int has_alpha, unsigned char *raw_data)
{
    size_t len = width * height * 3;
    FILE *in = fopen(filename, "rb");
    if (!in) {
	fprintf(stderr, "img2nes: failed to open `%s' for reading\n", filename);
	return(0);
    }
    if (has_alpha) {
        int x, y;
        char dummy;
        int pos = 0;
        for (y = 0; y < height; ++y) {
            for (x = 0; x < width; ++x) {
                if ((fread(&raw_data[pos], 1, 3, in) != 3) || (fread(&dummy, 1, 1, in) != 1)) {
		    fprintf(stderr, "img2nes: failed to read image data\n");
		    return(0);
                }
                pos += 3;
            }
        }
    } else {
        if (fread(raw_data, 1, len, in) != len) {
	    fprintf(stderr, "img2nes: failed to read image data\n");
	    return(0);
        }
    }
    fclose(in);
    return(1);
}

/**
  Converts raw image data to NES palette indices.
  \a in points to \a width * \a height RGB triplets.
  \a out is where to store the resulting data; it must be at least width * height bytes big.
*/
static void convert_to_nes_colors(const unsigned char *in, int width, int height, int has_alpha, unsigned char *out)
{
    int x, y;
    int i = 0;
    for (y = 0; y < height; ++y) {
	for (x = 0; x < width; ++x) {
	    int b = in[i++];
	    int g = in[i++];
	    int r = in[i++];
            if (has_alpha)
                ++i;
	    out[y*width+x] = (unsigned char)nearest_nes_color(r, g, b);
	}
    }
}

/**
  Returns the index of the color \a col in the palette \a pal of the given \a size,
  or -1 if the color is not found.
*/
static int index_of_nes_color(const unsigned char *pal, int size, unsigned char col)
{
    int i;
    for (i = 0; i < size; ++i) {
	if (pal[i] == col)
	    return i;
    }
    return -1;
}

/**
  Counts frequencies of the NES color array at \a buf with the given \a width, \a height.
  Stores the resulting frequencies in \a freqs. \a freqs is not initialized to zeroes.
  \a freqs must contain space for 64 entries.
*/
static void count_nes_color_frequencies(const unsigned char *buf, int width, int height, int stride, int *freqs)
{
    int x, y;
    for (y = 0; y < height; ++y) {
	for (x = 0; x < width; ++x)
	    ++freqs[buf[x]];
        buf += stride;
    }
}

/**
  Creates a NES palette based on the frequency of colors, described by \a freqs.
  Stores the resulting palette in \a pal, and its size in \a num_colors.
  \a pal must be allocated by caller and should be at least 64 bytes in size.
*/
static void nes_palette_from_color_frequencies(const int *freqs, unsigned char *pal, int *num_colors)
{
    int i;
    *num_colors = 1;
    pal[0] = 0x0F; // black is always first
    for (i = 0; i < 64; ++i) {
	if ((freqs[i] != 0) && (i != 0x0F)) {
	    pal[*num_colors] = i;
	    *num_colors = *num_colors + 1;
	}
    }
}

/**
  Reduces the number of colors in the image defined by \a buf, \a width, \a height.
  \a freqs contains initial frequency count of colors, and is updated according to
  the color reduction. \a pal contains the initial palette, and is updated according
  to the color reduction.
  \a buf is updated according to the color reduction.
  Returns the number of colors in the resulting image.
*/
static void reduce_colors(unsigned char *buf, int width, int height, int stride,
	                  int *freqs, unsigned char *pal, int *num_colors)
{
    int nc = *num_colors;
    while (nc > 4) {
	int i, j;
        int x, y;
	/* printf("reducing from %d colors\n", nc); */
	int ci1 = -1;
	int ci2 = -1;
	int diff = INT_MAX;
        /* find the two colors that are closest */
        /* (smallest diff between RGB values, and lowest frequency) */
	for (i = 0; i < nc; ++i) {
	    unsigned char c1 = pal[i];
	    for (j = i + 1; j < nc; ++j) {
		unsigned char c2 = pal[j];
		int d = nes_colors_diff(c1, c2);
		if (d <= diff) {
		    if ((ci1 == -1)
			|| (freqs[c1] < freqs[pal[ci1]]) || (freqs[c2] < freqs[pal[ci2]])
			|| (freqs[c2] < freqs[pal[ci1]]) || (freqs[c1] < freqs[pal[ci2]])) {
			diff = d;
			ci1 = i;
			ci2 = j;
		    }
		}
	    }
	}

	if (freqs[pal[ci1]] > freqs[pal[ci2]]) {
	    int tmp = ci1;
	    ci1 = ci2;
	    ci2 = tmp;
	}
	/* replace ci1 by ci2 */
#if 0
	printf("replace #%d (%.2X) by #%d (%.2X)\n", ci1, pal[ci1], ci2, pal[ci2]);
        printf("freqs before: %d %d\n", freqs[pal[ci1]], freqs[pal[ci2]]);
#endif
        unsigned char *p = buf;
	for (y = 0; y < height; ++y) {
	    for (x = 0; x < width; ++x) {
//                printf("%.2x", p[x]);
		if (p[x] == pal[ci1]) {
		    p[x] = pal[ci2];
                    if (p[x] != 0x0F)
                        ++freqs[pal[ci2]];
		    --freqs[pal[ci1]];
//                    printf("*");
		} else {
//                    printf(" ");
                }
	    }
//            printf("\n");
            p += stride;
	}
//        printf("freqs after: %d %d\n", freqs[pal[ci1]], freqs[pal[ci2]]);
	assert(freqs[pal[ci1]] == 0);

        /* kill the removed color from the palette */
	memmove(&pal[ci1], &pal[ci1+1], (nc-(ci1+1))*sizeof(unsigned char));

	--nc;
    }
    *num_colors = nc;
}

/**
   Returns 1 if \a p2 is subsumed by \a p1; otherwise returns 0.
*/
static int subsumes_palette(const unsigned char *p1, int s1,
                            const unsigned char *p2, int s2)
{
    int i, j;
    if (s2 > s1)
        return 0;
    for (i = 0; i < s2; ++i) {
        unsigned char c2 = p2[i];
        for (j = 0; j < s1; ++j) {
            if (p1[j] == c2)
                break;
        }
        if (j == s1)
            return 0;
    }
    return 1;
}

/**
  Finds the palettes in \a pals that are unique.
  Stores the indexes of those palettes in \a unique_pals, and the number of
  unique palettes in \a unique_pals_count.
*/
static void find_unique_palettes(const unsigned char *pal_data, int pals_count, const int *pal_sizes,
				 int *unique_pals, int *unique_pals_count, int *pal_indexes)
{
    int i;
    *unique_pals_count = 0;
    for (i = 0; i < pals_count; ++i) {
	const unsigned char *p1 = &pal_data[i * 4];
	int s1 = pal_sizes[i];
	int j;
	/* check if this palette is subsumed by an existing palette */
	for (j = 0; j < *unique_pals_count; ++j) {
	    const unsigned char *p2 = &pal_data[unique_pals[j] * 4];
	    int s2 = pal_sizes[unique_pals[j]];
            if (subsumes_palette(p2, s2, p1, s1)) {
		pal_indexes[i] = j;
		break;
	    }
	}
	if (j == *unique_pals_count) {
	    /* check if this palette subsumes an existing palette */
	    for (j = 0; j < *unique_pals_count; ++j) {
		const unsigned char *p2 = &pal_data[unique_pals[j] * 4];
		int s2 = pal_sizes[unique_pals[j]];
		if (subsumes_palette(p1, s1, p2, s2)) {
		    unique_pals[j] = i;
		    pal_indexes[i] = j;
		    break;
		}
	    }
	    if (j == *unique_pals_count) {
		/* add the palette */
		unique_pals[*unique_pals_count] = i;
		pal_indexes[i] = *unique_pals_count;
		*unique_pals_count = *unique_pals_count + 1;
	    }
	}
    }
#if 0
    printf("original palette count: %d unique palettes: %d\n", pals_count, *unique_pals_count);
    for (i = 0; i < pals_count; ++i) {
        for (int j = 0; j < pal_sizes[i]; ++j)
            printf("%.2X ", pal_data[i*4 + j]);
        printf("\n");
    }
    printf("***\n");
    for (i = 0; i < *unique_pals_count; ++i) {
        printf("%d: ", unique_pals[i]);
        for (int j = 0; j < pal_sizes[unique_pals[i]]; ++j)
            printf("%.2X ", pal_data[unique_pals[i]*4 + j]);
        printf("\n");
    }
#endif
}

/**
  Returs the NES color most similar to \a col in the given palette, \a
  pal, of the size \a pal_size.
*/
static unsigned char nearest_color_in_palette(unsigned char col,
                                              const unsigned char *pal,
                                              int pal_size)
{
    unsigned char ret = 0xFF;
    int i;
    int v1 = NES_R(col) + NES_G(col) + NES_B(col);
    int diff = INT_MAX;
    for (i = 0; i < pal_size; ++i) {
        unsigned char c = pal[i];
        int v2 = NES_R(c) + NES_G(c) + NES_B(c);
        if (ABS(v1 - v2) < diff) {
            diff = ABS(v1 - v2);
            ret = c;
        }
    }
    return ret;
}

/**
   Replaces colors in the image defined by \a buf, \a width and \ a
   height, according to the palette defined by \a pal and \a pal_size.
   This means: If a color is found in \a buf that isn't contained in
   the palette, it is replaced by the palette entry that's the best
   match.
*/
static void replace_colors(unsigned char *buf, int width, int height, int stride,
                           const unsigned char *pal, int pal_size)
{
    int x, y;
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            buf[x] = nearest_color_in_palette(buf[x], pal, pal_size);
        }
        buf += stride;
    }
}

/**
   Returns the difference between palettes \a p1 and \a p2.
*/
static int nes_palettes_diff(const unsigned char *p1, int s1,
                             const unsigned char *p2, int s2)
{
    int i;
    int d = 0;
    for (i = 0; i < MIN(s1, s2); ++i)
        d += nes_colors_diff(p1[i], p2[i]);
    return d;
}

/**
  Reduces the number of 4-color palettes in the image described
  by \a buf, \a width and \a height.
  Updates unique_pals to reflect the new palette usage.
  Updates \a pal_indexes to reflect the new palette usage.
  Updates \a buf to reflect the new palette usage.
*/
static void reduce_palettes(unsigned char *buf, int width, int height,
	       	            const unsigned char *pal_data, const int *pal_sizes,
			    int *unique_pals, int *unique_pals_count, int *pal_indexes)
{
    int i, j;
    int upc = *unique_pals_count;
    if (upc <= 4)
	return; /* nothing to do */

    int cw = width / 16;
    int ch = height / 16;
    /* count palette frequencies */
    int *pal_freqs = (int*)calloc(upc, sizeof(int));
    for (i = 0; i < cw * ch; ++i)
	++pal_freqs[pal_indexes[i]];

    while (upc > 4) {
	int pi1 = -1;
	int pi2 = -1;
	int diff = INT_MAX;
#if 0
        printf("reducing from %d palettes\n", upc);
        printf("before:\n");
        for (int v = 0; v < upc; ++v) {
            printf(" %d: %d\t", v, pal_freqs[v]);
            for (int f = 0; f < pal_sizes[unique_pals[v]]; ++f)
                printf(" %.2X", pal_data[unique_pals[v]*4 + f]);
            printf("\n");
        }
        for (int w = 0; w < cw*ch; ++w)
            printf("%d ", pal_indexes[w]);
        printf("\n");
#endif
        /* find the two palettes that are most similar */
	for (i = 0; i < upc; ++i) {
	    const unsigned char *p1 = &pal_data[unique_pals[i] * 4];
	    int s1 = pal_sizes[unique_pals[i]];
	    for (j = i + 1; j < upc; ++j) {
		const unsigned char *p2 = &pal_data[unique_pals[j] * 4];
		int s2 = pal_sizes[unique_pals[j]];
                int d = nes_palettes_diff(p1, s1, p2, s2);
		if (d <= diff) {
		    if ((pi1 == -1)
			|| (pal_freqs[i] < pal_freqs[pi1])
			|| (pal_freqs[j] < pal_freqs[pi2])
			|| (pal_freqs[j] < pal_freqs[pi1])
			|| (pal_freqs[i] < pal_freqs[pi2])) {
			diff = d;
			pi1 = i;
			pi2 = j;
		    }
		}
	    }
	}

	if (pal_freqs[pi1] > pal_freqs[pi2]) {
	    int tmp = pi1;
	    pi1 = pi2;
	    pi2 = tmp;
	}
	/* replace pi1 by pi2 */
//	printf("replacing %d by %d\n", pi1, pi2);
        const unsigned char *dest_pal = &pal_data[unique_pals[pi2]*4];
        int dest_pal_sz = pal_sizes[unique_pals[pi2]];
	for (i = 0; i < ch; ++i) {
            for (j = 0; j < cw; ++j) {
                int k = i*cw + j;
                if (pal_indexes[k] == pi1) {
                    pal_indexes[k] = pi2;
                    if (pi2 > pi1)
                        --pal_indexes[k]; /* because pi1 will be removed from the array */
                    ++pal_freqs[pi2];
                    --pal_freqs[pi1];
                    /* replace colors so that only colors in target palette are used */
                    replace_colors(&buf[(i*width*16)+(j*16)], 16, 16, width,
                                   dest_pal, dest_pal_sz);
                } else if (pal_indexes[k] > pi1) {
                    --pal_indexes[k];
                }
            }
        }
	assert(pal_freqs[pi1] == 0);
        /* kill the unique_pals entry */
	memmove(&unique_pals[pi1], &unique_pals[pi1+1], (upc-(pi1+1))*sizeof(int));
	memmove(&pal_freqs[pi1], &pal_freqs[pi1+1], (upc-(pi1+1))*sizeof(int));
	--upc;
    }
    *unique_pals_count = upc;
#if 0
    printf("after:\n");
    for (int v = 0; v < upc; ++v) {
        printf(" %d: %d\t", v, pal_freqs[v]);
        for (int f = 0; f < pal_sizes[unique_pals[v]]; ++f)
            printf(" %.2X", pal_data[unique_pals[v]*4 + f]);
        printf("\n");
    }
    for (int w = 0; w < cw*ch; ++w)
        printf("%d ", pal_indexes[w]);
    printf("\n");
#endif
    free(pal_freqs);
}

/**
  Encodes the image defined by \a buf, \a width, \a height as tiles.
  Palette data is defined by \a pals, \a unique_pals, \a pal_indexes.
  Resulting tile data is stored in \a tile_data.
*/
static void encode_nes_tiles(const unsigned char *buf, int width, int height,
                             const unsigned char *pal_data, const int *pal_sizes,
                             const int *unique_pals, const int *pal_indexes,
                             unsigned char *tile_data)
{
    int i, j;
    int cw = width / 16;
    int pos = 0;
    for (i = 0; i < height / 8; ++i) {
	for (j = 0; j < width / 8; ++j) {
	    int pi = pal_indexes[(i / 2)*cw + (j / 2)];
	    const unsigned char *pal = &pal_data[unique_pals[pi] * 4];
	    int sz = pal_sizes[unique_pals[pi]];
            int x, y;
	    for (y = 0; y < 8; ++y) {
		unsigned char b0 = 0;
		unsigned char b1 = 0;
		for (x = 0; x < 8; ++x) {
		    unsigned char v = buf[(i*8+y)*width + (j*8+x)];
		    int ci = index_of_nes_color(pal, sz, v);
                    if (ci == -1)
                        printf("did not find match for color %.2X\n", v);
		    assert(ci != -1);
		    assert(ci >= 0 && ci <= 3);
		    b0 |= (ci & 1) << (7-x);
		    b1 |= (ci >> 1) << (7-x);
		}
		tile_data[pos+y] = b0;
		tile_data[pos+y+8] = b1;
	    }
	    pos += 16;
	}
    }
}

/**
  Writes the tiles defined by (\a tile data, \a tile_data_sz) to the file
  named by \a chr_output_filename.
*/
static void write_tiles(const char *chr_output_filename, const unsigned char *tile_data, int tile_data_sz)
{
    FILE *out;
    if (!chr_output_filename)
	chr_output_filename = "img2nes.chr";
    out = fopen(chr_output_filename, "wb");
    if (!out) {
	fprintf(stderr, "img2nes: failed to open `%s' for writing\n", chr_output_filename);
	return;
    }
    fwrite(tile_data, sizeof(unsigned char), tile_data_sz, out);
    fclose(out);
}

/**
  Writes the palette data to the file named by \a pal_output_filename.
*/
static void write_palettes(const char *pal_output_filename, const unsigned char *pal_data,
                           const int *pal_sizes, const int *unique_pals, int unique_pals_count)
{
    FILE *out;
    int i, j;
    if (!pal_output_filename)
	pal_output_filename = "img2nes.pal";
    out = fopen(pal_output_filename, "wb");
    if (!out) {
        fprintf(stderr, "img2nes: failed to open `%s' for writing\n", pal_output_filename);
        return;
    }
    for (i = 0; i < unique_pals_count; ++i) {
	const unsigned char *pal = &pal_data[unique_pals[i] * 4];
	int sz = pal_sizes[unique_pals[i]];
	fwrite(pal, sizeof(unsigned char), sz, out);
	for (j = sz; j < 4; ++j)
	    fputc(0x0F, out);
    }
    fclose(out);
}

/**
  Encodes NES attribute data.
  \a cw is the number of horizontal 16x16-pixel blocks;
  \a ch is the number of vertical 16x16-pixel blocks;
  \a pal_indexes contains the palette indexes for the blocks;
  \a attrib_data is where to store the encoded data.
*/
static void encode_nes_attributes(int cw, int ch, const int *pal_indexes,
                                  unsigned char *attrib_data)
{
    int i, j;
    for (i = 0; i < ch; ++i) {
        for (j = 0; j < cw; ++j) {
            unsigned char datum;
            int quadrant = (i & 1)*2 + (j & 1);
            int shift = quadrant*2;
            int pal_no = pal_indexes[i*cw+j];
            int data_ofs = (i/2)*(cw/2) + (j/2);
            assert(pal_no < 4);
            if (quadrant == 0)
                datum = 0;
            else
                datum = attrib_data[data_ofs];
            datum |= (unsigned char)(pal_no << shift);
            attrib_data[data_ofs] = datum;
        }
    }
}

/**
  Writes attribute data defined by \a attrib_data of size \a attrib_data_sz,
  to the file named by \a attrib_output_filename.
*/
static void write_attributes(const char *attrib_output_filename,
                             unsigned char *attrib_data, int attrib_data_sz)
{
    if (!attrib_output_filename)
        attrib_output_filename = "img2nes.att";
    {
        FILE *out = fopen(attrib_output_filename, "wb");
        if (!out) {
            fprintf(stderr, "img2nes: failed to open `%s' for writing\n", attrib_output_filename);
            return;
        }
	fwrite(attrib_data, sizeof(unsigned char), attrib_data_sz, out);
        fclose(out);
    }
}



static char program_version[] = "img2nes 1.0";

/* Prints usage message and exits. */
static void usage()
{
    printf(
        "Usage: img2nes  [--output=FILE]\n"
        "               [--help] [--usage] [--version]\n"
        "                FILE\n");
    exit(0);
}

/* Prints help message and exits. */
static void help()
{
    printf("Usage: img2nes [OPTION...] FILE\n\n"
           "  --output=FILE                   Store output in FILE\n"
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

/**
  Processes command line arguments.
*/
static int process_args(char **argv, const char **input_filename, const char **chr_output_filename,
                        const char **pal_output_filename, const char **attrib_output_filename,
                        int *width, int *height)
{
    char *p;
    while ((p = *(++argv))) {
	if (!strncmp("--", p, 2)) {
	    const char *opt = &p[2];
	    if (!strncmp("character-output=", opt, 17)) {
		*chr_output_filename = &opt[17];
	    } else if (!strncmp("palette-output=", opt, 15)) {
		*pal_output_filename = &opt[15];
	    } else if (!strncmp("attribute-output=", opt, 17)) {
		*attrib_output_filename = &opt[17];
	    } else if (!strncmp("width=", opt, 6)) {
		*width = strtol(&opt[6], 0, 0);
	    } else if (!strncmp("height=", opt, 7)) {
		*height = strtol(&opt[7], 0, 0);
	    } else if (!strcmp("help", opt)) {
		help();
	    } else if (!strcmp("usage", opt)) {
		usage();
	    } else if (!strcmp("version", opt)) {
		version();
	    } else {
		fprintf(stderr, "unrecognized option `%s'\n", p);
		return(0);
	    }
	} else {
	    *input_filename = p;
	}
    }
    return(1);
}

static QImage to_qimage(const unsigned char *buf, int width, int height)
{
    QImage result(width, height, QImage::Format_Indexed8);
    for (int i = 0; i < 64; ++i)
        result.setColor(i, qRgb(nes_palette[i*3], nes_palette[i*3+1], nes_palette[i*3+2]));
    for (int y = 0; y < height; ++y) {
         memcpy(result.scanLine(y), &buf[y*width], width);
    }
    return result;
}

static QLabel *create_label(const QImage &image)
{
    QLabel *label = new QLabel();
    label->setPixmap(QPixmap::fromImage(image));
    label->setScaledContents(true);
    return label;
}

/**
 * Program entrypoint.
 */
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QWidget window;

    unsigned char *raw_data;
    unsigned char *buf;
    int *color_freqs;
    int *unique_pals;
    int unique_pals_count;
    int *pal_indexes;
    unsigned char *pal_data;
    int *pal_sizes;
    unsigned char *tile_data;
    int tile_data_sz;
    unsigned char *attrib_data;
    int attrib_data_sz;
    const char *input_filename = 0;
    const char *chr_output_filename = 0;
    const char *pal_output_filename = 0;
    const char *attrib_output_filename = 0;
    int width = -1;
    int height = -1;

    /* Process arguments. */
    if (!process_args(argv, &input_filename, &chr_output_filename, &pal_output_filename,
                      &attrib_output_filename, &width, &height)) {
        return(-1);
    }

    /* check arguments */
    if (!input_filename) {
        fprintf(stderr, "img2nes: no filename given\n");
        return(-1);
    }
#if 0
    if (width == -1 || height == -1) {
	fprintf(stderr, "img2nes: please specify width and height of image\n");
        return(-1);
    }
    if (width % 16 || height % 16) {
	fprintf(stderr, "img2nes: width and height must be multiples of 16\n");
        return(-1);
    }
#endif

    QImage originalImage = QImage(input_filename).convertToFormat(QImage::Format_RGB32);
    if (originalImage.isNull()) {
        fprintf(stderr, "img2nes: unable to read image '%s'\n", input_filename);
        return(-1);
    }
    width = originalImage.width();
    height = originalImage.height();

    /* allocate some buffers */
    raw_data = (unsigned char *)malloc(width * height * 3);
    buf = (unsigned char *)malloc(width * height);
    tile_data_sz = (height / 8) * (width / 8) * 16 * sizeof(unsigned char);
    tile_data = (unsigned char *)malloc(tile_data_sz);
    if (!raw_data || !buf || !tile_data) {
	fprintf(stderr, "img2nes: failed to allocate memory for image data\n");
	return(0);
    }

#if 0
    /* read input image */
    if (!read_image(input_filename, width, height, /*has_alpha=*/0, raw_data))
        return(-1);
#endif

    /* convert raw RGB to nes palette indices */
    convert_to_nes_colors(originalImage.bits(), width, height, /*has_alpha=*/1, buf);

    QImage originalNesImage = to_qimage(buf, width, height);

    QImage reducedColorsImage;
    QImage reducedPalettesImage;

    /* that was the easy part */
    {
        int i, j;
	int cw = width / 16;
	int ch = height / 16;
	color_freqs = (int*)calloc(cw * ch,  64 * sizeof(int));
	pal_data = (unsigned char *)malloc(cw * ch * 4 * sizeof(unsigned char));
	pal_sizes = (int*)malloc(cw * ch * sizeof(int));
        /* process all 16x16 blocks */
	for (i = 0; i < ch; ++i) {
	    for (j = 0; j < cw; ++j) {
		int num_colors;
		unsigned char pal[64];
		int *f = &color_freqs[(i*cw+j)*64];
		/* count frequency of colors */
                count_nes_color_frequencies(&buf[i*16*width+(j*16)], 16, 16, width, f);
		f[0x0F] = INT_MAX;

		/* record the colors that are actually used */
                nes_palette_from_color_frequencies(f, pal, &num_colors);

#if 0
                {
                    printf("palette: ");
                    for (int k = 0; k < num_colors; ++k) {
                        printf("%.2X ", pal[k]);
                    }
                    printf("\n");
                }
#endif

		/* reduce number of colors if necessary */
                reduce_colors(&buf[i*16*width+(j*16)], 16, 16, width, f, pal, &num_colors);

		/* sort the palette so that different palettes can be easily compared */
		qsort(pal, num_colors, sizeof(unsigned char), compare_nes_colors);
#if 0
                {
                    printf("sorted palette: ");
                    for (int k = 0; k < num_colors; ++k) {
                        printf("%.2X ", pal[k]);
                    }
                    printf("\n");
                }
#endif

		/* store the final palette for this 16x16 block */
                assert(num_colors <= 4);
		memcpy(&pal_data[(i*cw+j)*4], pal, num_colors * sizeof(unsigned char));
		pal_sizes[i*cw+j] = num_colors;
	    }
	}

        reducedColorsImage = to_qimage(buf, width, height);

	/* now each 16x16 block uses max 4 unique colors
	   the next step is to reduce the number of 4-color palettes, if necessary */
        unique_pals = (int*)malloc(cw * ch * sizeof(int));
        unique_pals_count = 0;
        pal_indexes = (int*)malloc(cw * ch * sizeof(int));
        /* find unique palettes and record their usage */
        find_unique_palettes(pal_data, cw * ch, pal_sizes, unique_pals, &unique_pals_count, pal_indexes);

        /*	    printf("unique palettes: %d\n", unique_pals_count); */
        /* reduce number of palettes used if necessary */
        reduce_palettes(buf, width, height, pal_data, pal_sizes, unique_pals, &unique_pals_count, pal_indexes);

        reducedPalettesImage = to_qimage(buf, width, height);

        /* encode tiles */
        encode_nes_tiles(buf, width, height, pal_data, pal_sizes, unique_pals, pal_indexes, tile_data);

        /* write tiles */
        write_tiles(chr_output_filename, tile_data, tile_data_sz);

        /* write palette data */
        write_palettes(pal_output_filename, pal_data, pal_sizes, unique_pals, unique_pals_count);

        /* encode attribute data */
        attrib_data_sz = (cw/2)*(ch/2);
        attrib_data = (unsigned char *)malloc(attrib_data_sz);
        encode_nes_attributes(cw, ch, pal_indexes, attrib_data);

        /* write attribute data */
        write_attributes(attrib_output_filename, attrib_data, attrib_data_sz);
    }

    QHBoxLayout *box = new QHBoxLayout(&window);
    box->addWidget(create_label(originalImage));
    box->addWidget(create_label(originalNesImage));
    box->addWidget(create_label(reducedColorsImage));
    box->addWidget(create_label(reducedPalettesImage));
    window.show();
    app.exec();

    /* Cleanup */
    free(raw_data);
    free(buf);
    free(tile_data);
    free(pal_data);
    free(pal_sizes);
    free(unique_pals);
    free(pal_indexes);
    free(color_freqs);
    free(attrib_data);

    /* All done. */
    return 0;
}
