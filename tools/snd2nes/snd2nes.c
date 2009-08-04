/*
    This file is part of snd2nes.

    snd2nes is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    snd2nes is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with snd2nes.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <math.h>
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
  Prints \a size bytes of data defined by \a buf to \a out.
*/
static void print_chunk(FILE *out, const char *label,
                        const unsigned char *buf, int size, int cols)
{
    int i, j, m;
    int pos = 0;
    if (label)
        fprintf(out, "%s:\n", label);
    for (i = 0; i < size / cols; ++i) {
        fprintf(out, ".db ");
        for (j = 0; j < cols-1; ++j)
            fprintf(out, "$%.2X,", buf[pos++]);
        fprintf(out, "$%.2X\n", buf[pos++]);
    }
    m = size % cols;
    if (m > 0) {
        fprintf(out, ".db ");
        for (j = 0; j < m-1; ++j)
            fprintf(out, "$%.2X,", buf[pos++]);
        fprintf(out, "$%.2X\n", buf[pos++]);
    }
}

static void convert_to_dmc(SF_INFO *info, short *frames,
                           float samplerate, unsigned char *delta_ctr_load_out,
                           unsigned char **data_out, int *size_out)
{
    unsigned char bit;
    unsigned char encoded;
    short prev, curr;
    float pos, step;
    int bitcount;
    unsigned char *buf;
    int buf_size;
    step = samplerate / info->samplerate;
    encoded = 0;
    bit = 0;
    prev = 0;
    bitcount = 0;
    buf_size = 1024;
    buf = (unsigned char *)malloc(buf_size);
    for (pos = 0 ; pos < info->frames; prev = curr, pos += step, ++bitcount) {
        if (!(bitcount & 7) && bitcount) {
            if ((bitcount / 8) >= buf_size) {
                buf_size += 1024;
                buf = (unsigned char *)realloc(buf, buf_size);
            }
            buf[(bitcount-1) / 8] = encoded;
            encoded = 0;
        }
        curr = frames[(int)pos * info->channels];
        if (curr < prev)
            bit = 0;
        else if (curr > prev)
            bit = 1;
        else
            bit = bit ^ 1;
        encoded |= bit << (bitcount & 7);
    }
    /* pad to 64-byte boundary */
    while (bitcount & 511) {
        if (!(bitcount & 7)) {
            if ((bitcount / 8) >= buf_size) {
                buf_size += 1024;
                buf = (unsigned char *)realloc(buf, buf_size);
            }
            buf[(bitcount-1) / 8] = encoded;
            encoded = 0;
        }
        bit = 0; /*bit ^ 1;*/
        encoded |= bit << (bitcount & 7);
        ++bitcount;
    }
    if ((bitcount / 8) >= buf_size) {
        buf_size += 1024;
        buf = (unsigned char *)realloc(buf, buf_size);
    }
    buf[(bitcount-1) / 8] = encoded;
    *data_out = buf;
    *size_out = bitcount / 8;
    *delta_ctr_load_out = (frames[0] + 0x8000) / 256;
}

static float note_hz(int note)
{
    return pow(2, (note - 57) / 12.0) * 440;
}

static void print_sample_table(const char *table_label, const char *sample_label_prefix,
                               const unsigned char *delta_ctr_loads, FILE *out)
{
    static const unsigned char freqs[] = {
        0,0,
        1,1,
        2,2,
        3,3,
        4,4,
        5,5,
        6,
        7,7,
        8,8,8,
        9,9,
        10,10,
        11,11,11,
        12,12,12,12,
        13,13,13,
        14,14,14,14,14,
        15,15,15,15,15
    };
    static const unsigned char samples[] = {
        4,3,
        4,3,
        4,3,
        4,3,
        4,3,
        4,3,
        4,
        4,3,
        4,3,2,
        4,3,
        4,3,
        4,3,2,
        4,3,2,1,
        4,3,2,
        4,3,2,1,0,
        4,3,2,1,0
    };
    static const char letters[] = "C-C#D-D#E-F-F#G-G#A-A#B-";
    int i;
    if (table_label)
        fprintf(out, "%s:\n", table_label);
    for (i = 0; i < 42; ++i) {
        unsigned char f = freqs[i];
        unsigned char s = samples[i];
        int note = i + 55;
        int j = note % 12;
        fprintf(out, ".db $%.2X,$%.2X,(%s%d-$C000)/64,(%s%d-%s%d)/16-1 ; %.2X=%c%c%d\n",
                f, delta_ctr_loads[s], sample_label_prefix, s, sample_label_prefix,
                s+1, sample_label_prefix, s, i, letters[j*2], letters[j*2+1], note / 12);
    }
}

int snd2nes(const char *input_filename, int note_delta,
            int hz_delta, int multi, FILE *out)
{
    short *frames;
    SNDFILE *sf;
    SF_INFO info;
    sf_count_t count;
    int i;
    unsigned char delta_ctr_loads[5];
    unsigned char *bufs[5];
    int sizes[5];

    info.format = 0;
    sf = sf_open(input_filename, SFM_READ, &info);
    if (!sf)
        return 1;

    frames = (short *)malloc(info.channels * info.frames * sizeof(short));
    count = sf_readf_short(sf, frames, info.frames);
    assert(count == info.frames);

    for (i = 0; i < (multi ? 5 : 1); ++i) {
        static const int base_note = 12*8;
        float base_hz = note_hz(base_note) + hz_delta;
        float hz = note_hz(base_note + note_delta - i);
	convert_to_dmc(&info, frames, 1 * (hz / base_hz) * info.samplerate,
                       &delta_ctr_loads[i], &bufs[i], &sizes[i]);
    }
    free(frames);
    sf_close(sf);

    if (multi)
        fprintf(out, ".public dmc_sample_table\n");

    for (i = 0; i < (multi ? 5 : 1); ++i) {
        char label[32];
        sprintf(label, "sample%d", i);
        print_chunk(out, label, bufs[i], sizes[i], 16);
    }
    if (multi)
        fprintf(out, "sample%d:\n", 5);

    if (multi) {
        print_sample_table("dmc_sample_table", "sample", delta_ctr_loads, out);
    } else {
        fprintf(out, "; values to write to $4010-$4013\n");
        fprintf(out, ".db $0F,$%.2X,(sample0-$C000)/64,$%.2X\n",
                delta_ctr_loads[0], sizes[0]/16-1);
    }

    for (i = 0; i < (multi ? 5 : 1); ++i)
        free(bufs[i]);
    return 0;
}
