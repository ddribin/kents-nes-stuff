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

#include "xm2nes.h"

#define SET_EFFECT_COMMAND_BASE 0xE0
#define SET_INSTRUMENT_COMMAND 0xF0
#define RELEASE_COMMAND 0xF1
#define SET_MASTER_VOLUME_COMMAND 0xF2
#define SET_SPEED_COMMAND 0xF3
#define END_ROW_COMMAND 0xF4

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

/**
  Finds the patterns that are actually used, according to the
  pattern order table.
 */
static void find_used_patterns(int song_length, const unsigned char *order_table,
                               int **used_set)
{
    int i;
    int bits_in_int = sizeof(int) * 8;
    int set_size_in_bytes = ((song_length + bits_in_int-1) / bits_in_int) * sizeof(int);
    *used_set = (int *)malloc(set_size_in_bytes);
    memset(*used_set, 0, set_size_in_bytes);
    for (i = 0; i < song_length; ++i) {
        int j = order_table[i];
        (*used_set)[j / bits_in_int] |= 1 << (j & (bits_in_int-1));
    }
}

/**
  Checks if the given \a pattern is empty.
*/
static int is_pattern_empty_for_channel(const struct xm_pattern *pattern,
					int channel_count, int channel)
{
    int row;
    const struct xm_pattern_slot *slot = &pattern->data[channel];
    for (row = 0; row < pattern->row_count; ++row, slot += channel_count) {
        if ((slot->note != 0) || (slot->instrument != 0) || (slot->volume != 0)
            || (slot->effect_type != 0) || (slot->effect_param != 0)) {
            return 0;
        }
    }
    return 1;
}

/**
 Tests if the patterns \a p1 and \a p2 are equal (contain identical data) 
 for the given \a channel.
 */
static int are_patterns_equal_for_channel(
    const struct xm_pattern *p1,
    const struct xm_pattern *p2,
    int channel_count, int channel)
{
    int row;
    const struct xm_pattern_slot *s1;
    const struct xm_pattern_slot *s2;
    if (p1->row_count != p2->row_count)
        return 0;
    s1 = &p1->data[channel];
    s2 = &p2->data[channel];
    for (row = 0; row < p1->row_count; ++row,
        s1 += channel_count, s2 += channel_count) {
        if ((s1->note != s2->note)
            || (s1->instrument != s2->instrument)
            || (s1->volume != s2->volume)
            || (s1->effect_type != s2->effect_type)
            || (s1->effect_param != s2->effect_param)) {
            return 0;
	}
    }
    return 1;
}

/**
  Finds unique patterns in the given \a xm for the given
  \a channel. Stores the indexes of the unique patterns
  in \a unique_pattern_indexes and the count in
  \a unique_pattern_count.
*/
static void find_unique_patterns_for_channel(
    const struct xm *xm, int channel,
    int *used_patterns_set,
    unsigned char *unique_pattern_indexes,
    int *unique_pattern_count)
{
    int i;
    int bits_in_int = sizeof(int) * 8;
    *unique_pattern_count = 0;
    for (i = 0; i < xm->header.pattern_count; ++i) {
        const struct xm_pattern *pattern;
        int j;
        if (!(used_patterns_set[i / bits_in_int] & (1 << (i & (bits_in_int-1)))))
            continue; /* Whole pattern is unused */
        pattern = &xm->patterns[i];
        for (j = 0; j < *unique_pattern_count; ++j) {
            int k = unique_pattern_indexes[j];
            const struct xm_pattern *other = &xm->patterns[k];
            if (are_patterns_equal_for_channel(pattern, other, xm->header.channel_count, channel)) {
                break;
            }
        }
        if (j == *unique_pattern_count)
            unique_pattern_indexes[(*unique_pattern_count)++] = i;
    }
}

/**
  Calculates the order table of the given \a xm for the given
  \a channel, based on \a unique_pattern_indexes and
  \a unique_pattern_count. Stores the result in \a order_table.
*/
static void calculate_order_table_for_channel(
    const struct xm *xm, int channel,
    int order_start_offset, int order_end_offset,
    unsigned char *unique_pattern_indexes,
    int unique_pattern_count, int pattern_offset,
    unsigned char *order_table, int *order_table_size)
{
    int i;
    int prev = -1;
    int count = 0;
    int pos = 0;
    for (i = order_start_offset; i <= order_end_offset; ++i) {
        int j;
        int k = xm->header.pattern_order_table[i];
        const struct xm_pattern *pattern = &xm->patterns[k];
        for (j = 0; j < unique_pattern_count; ++j) {
            const struct xm_pattern *other = &xm->patterns[unique_pattern_indexes[j]];
            if (are_patterns_equal_for_channel(pattern, other, xm->header.channel_count, channel))
                break;
        }
        assert(j != unique_pattern_count);
        if (count == 0) {
            prev = j;
            ++count;
        } else {
            if (prev == j) {
                ++count;
            } else {
                if (count <= 4) {
                    if (count > 3)
                        order_table[pos++] = prev + pattern_offset;
                    if (count > 2)
                        order_table[pos++] = prev + pattern_offset;
                    if (count > 1)
                        order_table[pos++] = prev + pattern_offset;
                    order_table[pos++] = prev + pattern_offset;
                } else {
                    order_table[pos++] = 0xFB;
                    order_table[pos++] = count;
                    order_table[pos++] = prev + pattern_offset;
                    order_table[pos++] = 0xFC;
                }
                prev = j;
                count = 1;
            }
        }
    }
    if (count != 0) {
        if (count <= 4) {
            if (count > 3)
                order_table[pos++] = prev + pattern_offset;
            if (count > 2)
                order_table[pos++] = prev + pattern_offset;
            if (count > 1)
                order_table[pos++] = prev + pattern_offset;
            order_table[pos++] = prev + pattern_offset;
        } else {
            order_table[pos++] = 0xFB;
            order_table[pos++] = count;
            order_table[pos++] = prev + pattern_offset;
            order_table[pos++] = 0xFC;
        }
    }
    *order_table_size = pos;
}

/**
  Converts the \a channel of the given \a pattern to NES format.
*/
static void convert_xm_pattern_to_nes(const struct xm_pattern *pattern, int channel_count,
				      int channel, const struct instr_mapping *instr_map,
                                      unsigned char **out, int *out_size)
{
    unsigned char lastinstr = 0xFF;
    unsigned char lastefftype = 0x00;
    unsigned char lasteffparam = 0x00;
    const struct xm_pattern_slot *slots = &pattern->data[channel];
    int row;
    int sz = 1024;
    unsigned char *data = (unsigned char *)malloc(sz);
    int pos = 0;
    assert((pattern->row_count % 8) == 0);
    data[pos++] = pattern->row_count;
    /* process channel in 8-row chunks */
    for (row = 0; row < pattern->row_count; row += 8) {
	int i;
        unsigned char copy[3];
        unsigned char flags = 0;
        copy[0] = lastinstr;
        copy[1] = lastefftype;
        copy[2] = lasteffparam;
	/* First pass: calculate active rows byte */
	for (i = 0; i < 8; ++i) {
            int instrument_changed = 0;
            const struct xm_pattern_slot *n = &slots[(row+i)*channel_count];
	    if (n->note != 0) {
		flags |= 1 << i;
	    }
            if ((n->instrument != 0) && (n->instrument != lastinstr)) {
		lastinstr = n->instrument;
		flags |= 1 << i;
                instrument_changed = 1;
	    }
            if (n->volume != 0) {
		if ((n->volume >= 0x10) && (n->volume < 0x50)) {
		    if (channel == 4)
			fprintf(stderr, "volume channel bytes are ignored for channel 4 (DMC)\n");
		    else
			flags |= 1 << i;
		}
	    }
            if ((n->effect_type != lastefftype)
                || ((n->effect_param != lasteffparam)
                    && (n->effect_param != 0))
                || /* NES: setting instrument resets effect */
                (instrument_changed && (n->effect_type != 0))) {
		if ((n->effect_type != 0) && (n->effect_param != 0))
		    lasteffparam = n->effect_param;
		lastefftype = n->effect_type;
		flags |= 1 << i;
	    }
	}
	data[pos++] = flags;

	/* Second pass: the actual note+effect data for these 8 rows */
        /* Note that the conditions for outputting data should exactly
           match those in the first pass! */
        lastinstr = copy[0];
        lastefftype = copy[1];
        lasteffparam = copy[2];
	for (i = 0; i < 8; ++i) {
            int instrument_changed = 0;
	    const struct xm_pattern_slot *n = &slots[(row+i)*channel_count];
	    if (!(flags & (1 << i)))
                continue;
            switch (channel) {
		case 0:
		case 1:
		case 2:
                case 3:
		    if (n->volume != 0) {
			if ((n->volume >= 0x10) && (n->volume < 0x50)) {
			    /* set new channel volume */
			    data[pos++] = SET_MASTER_VOLUME_COMMAND;
			    data[pos++] = ((n->volume - 0x10) >> 2) << 4;
			}
		    }
		    if (n->instrument && (n->instrument != lastinstr)) {
			data[pos++] = SET_INSTRUMENT_COMMAND;
                        data[pos++] = instr_map[n->instrument - 1].target_instr;
                        lastinstr = n->instrument;
                        instrument_changed = 1;
		    }
		    if ((n->effect_type != lastefftype)
			|| ((n->effect_param != lasteffparam)
			    && ((n->effect_param != 0)))
                        || /* NES: setting instrument resets effect */
                        (instrument_changed && (n->effect_type != 0))) {
                        switch (n->effect_type) {
                            case 0x0:
			    case 0x1:
			    case 0x2:
			    case 0x3:
			    case 0x4:
			    case 0x5: /* This is actually arpeggio -- see hack in xm.c */
			    case 0x6:
			    case 0x7:
                            case 0xA: {
                                unsigned char tp = n->effect_type;
                                if (tp == 0xA)
				    tp = 6;
				data[pos++] = SET_EFFECT_COMMAND_BASE | tp;
				if ((n->effect_type != 0) && (n->effect_param != 0))
				    lasteffparam = n->effect_param;
				if (n->effect_type != 0)
				    data[pos++] = lasteffparam;
				break;
			    }
                            case 0xC:
				data[pos++] = SET_MASTER_VOLUME_COMMAND;
				data[pos++] = n->effect_param << 2;
				break;
                            case 0xE:
                                switch ((n->effect_param & 0xF0) >> 4) {
                                    case 0x8: /* pulse modulation */
                                        data[pos++] = SET_EFFECT_COMMAND_BASE | 9;
                                        data[pos++] = n->effect_param & 0x0F;
                                        break;
                                    case 0xC: /* note cut */
                                        data[pos++] = SET_EFFECT_COMMAND_BASE | 8;
                                        data[pos++] = n->effect_param & 0x0F;
                                        break;
                                    default:
                                        fprintf(stderr, "ignoring effect %x%.2x in channel %d, row %d\n",
                                                n->effect_type, n->effect_param, channel, row+i);
                                        break;
                                }
                                break;
			    case 0xF:
				data[pos++] = SET_SPEED_COMMAND;
				data[pos++] = n->effect_param + 1;
				break;
			    default:
				fprintf(stderr, "ignoring effect %x%.2x in channel %d, row %d\n",
					n->effect_type, n->effect_param, channel, row+i);
				break;
			}
			lastefftype = n->effect_type;
		    }
                    if (n->note != 0) {
                        if (n->note == 0x61) {
			    data[pos++] = RELEASE_COMMAND;
			    data[pos++] = END_ROW_COMMAND;
			} else {
                            data[pos++] = n->note + instr_map[lastinstr-1].transpose;
                            if (data[pos-1] >= 0x80)
                                data[pos-1] = 0;
			}
		    } else
                        data[pos++] = END_ROW_COMMAND;
		    break;
		    /* dpcm */
		case 4:
                    if (n->effect_type != 0) {
                        switch (n->effect_type) {
                            case 0xF:
				data[pos++] = SET_SPEED_COMMAND;
				data[pos++] = n->effect_param + 1;
                                break;
                            default:
				fprintf(stderr, "ignoring effect %x%.2x in channel %d, row %d\n",
					n->effect_type, n->effect_param, channel, row+i);
                                ;
                        }               
                    }
                    if (n->note != 0) {
                        unsigned char dmc_sample_index = instr_map[n->instrument - 1].target_instr;
                        if (instr_map[n->instrument - 1].transpose != 0) {
                            /* Transpose is used to indicate that this is a "multi-sample" */
                            /* Ideally there should be a separate attribute for that */
                            dmc_sample_index += n->note + instr_map[n->instrument - 1].transpose;
                        }
                        data[pos++] = dmc_sample_index;
                    } else
                        data[pos++] = END_ROW_COMMAND;
                    break;
	    }
	}
    }

    *out = data;
    *out_size = pos;
}

static void print_pattern_table(int channel_count, int unused_channels,
                                int *unique_pattern_count,
                                const char *label_prefix, FILE *out)
{
    int chn;
    fprintf(out, "%spattern_table:\n", label_prefix);
    for (chn = 0; chn < channel_count; ++chn) {
	int i;
        if (unused_channels & (1 << chn))
            continue;
	for (i = 0; i < unique_pattern_count[chn]; ++i)
	    fprintf(out, ".dw %schn%d_ptn%d\n", label_prefix, chn, i);
    }
}

static void print_song_struct(int channel_count, int unused_channels,
                              int default_tempo, int *order_data_size,
                              unsigned char *order_data, int song_length,
                              const char *label_prefix, FILE *out)
{
    int chn;
    int order_offset = 0;
    fprintf(out, "%ssong:\n", label_prefix);
    for (chn = 0; chn < channel_count; ++chn) {
        if (chn >= 5)
            break;
        if (unused_channels & (1 << chn)) {
            fprintf(out, ".db $FF\n");
        } else {
            fprintf(out, ".db %d,%d\n", order_offset, default_tempo);
            order_offset += order_data_size[chn] + 2;
        }
    }
    fprintf(out, ".dw %sinstrument_table\n", label_prefix);
    fprintf(out, ".dw %spattern_table\n", label_prefix);
    order_offset = 0;
    for (chn = 0; chn < channel_count; ++chn) {
        if (chn >= 5)
            break;
        if (unused_channels & (1 << chn))
            continue;
        print_chunk(out, 0, &order_data[chn * song_length],
                    order_data_size[chn], 16);
        fprintf(out, ".db $FE,%d\n", order_offset); /* loop back to the beginning */
        order_offset += order_data_size[chn] + 2;
    }
}

/**
  Converts the given \a xm to NES format; writes the 6502 assembly
  language representation of the song to \a out.
*/
void convert_xm_to_nes(const struct xm *xm,
                       const struct xm2nes_options *options,
                       FILE *out)
{
    int chn;
    int unused_channels;
    int *used_patterns_set;
    unsigned char **unique_pattern_indexes;
    int *unique_pattern_count;
    unsigned char *order_data;
    int *order_data_size;
    int song_length;
    int order_start_offset;
    int order_end_offset;
    if (xm->header.song_length == 0)
        return;

    order_end_offset = options->order_end_offset;
    if ((order_end_offset == -1) || (options->order_end_offset >= xm->header.song_length))
        order_end_offset = xm->header.song_length - 1;
    order_start_offset = options->order_start_offset;
    if (order_start_offset < 0)
        order_start_offset = 0;
    else if (order_start_offset > order_end_offset)
        order_start_offset = order_end_offset;
    song_length = order_end_offset - order_start_offset + 1;

    unused_channels = 0;
    unique_pattern_indexes = (unsigned char **)malloc(xm->header.channel_count * sizeof(unsigned char *));
    unique_pattern_count = (int *)malloc(xm->header.channel_count * sizeof(int));
    order_data = (unsigned char *)malloc(xm->header.channel_count * song_length * sizeof(unsigned char));
    order_data_size = (int *)malloc(xm->header.channel_count * sizeof(int));

    /* Step 1. Find the patterns that are actually used. */
    find_used_patterns(song_length, xm->header.pattern_order_table + order_start_offset, &used_patterns_set);

    /* Step 2. Find, convert and print unique patterns. */
    for (chn = 0; chn < xm->header.channel_count; ++chn) {
	int i;
        if (!((1 << chn) & options->channels)) {
            unused_channels |= 1 << chn;
            continue;
        }

	unique_pattern_indexes[chn] = (unsigned char *)malloc(xm->header.pattern_count * sizeof(unsigned char));
	find_unique_patterns_for_channel(xm, chn, used_patterns_set,
                                         unique_pattern_indexes[chn], &unique_pattern_count[chn]);

        {
            int j;
            int has_non_empty_pattern = 0;
            for (j = 0; j < unique_pattern_count[chn]; ++j) {
                int pi = unique_pattern_indexes[chn][j];
	        if (!is_pattern_empty_for_channel(&xm->patterns[pi], xm->header.channel_count, chn)) {
                    has_non_empty_pattern = 1;
                    break;
                }
            }
            if (!has_non_empty_pattern) {
                unused_channels |= 1 << chn;
                continue;
            }
        }

	if (chn >= 5) {
            int j;
           fprintf(stderr, "ignoring contents of channel %d; patterns \n", chn);
            for (j = 0; j < unique_pattern_count[chn]; ++j) {
                int pi = unique_pattern_indexes[chn][j];
	        if (!is_pattern_empty_for_channel(&xm->patterns[pi], xm->header.channel_count, chn))
                    fprintf(stderr, " %d", pi);
            }
            fprintf(stderr, "\n");
	    continue;
	}

	for (i = 0; i < unique_pattern_count[chn]; ++i) {
	    unsigned char *data;
	    int data_size;
	    char label[256];
            int pi = unique_pattern_indexes[chn][i];
	    convert_xm_pattern_to_nes(&xm->patterns[pi], xm->header.channel_count,
                                      chn, options->instr_map, &data, &data_size);
	    sprintf(label, "%schn%d_ptn%d", options->label_prefix, chn, i);
	    print_chunk(out, label, data, data_size, 16);
	    free(data);
	}
    }

    /* Step 3. Create order tables. */
    {
	int pattern_offset = 0;
        for (chn = 0; chn < xm->header.channel_count; ++chn) {
            if (unused_channels & (1 << chn))
                continue;
            calculate_order_table_for_channel(xm, chn, order_start_offset,
                                              order_end_offset,
                                              unique_pattern_indexes[chn],
                                              unique_pattern_count[chn], pattern_offset,
                                              &order_data[chn * song_length],
                                              &order_data_size[chn]);
	    pattern_offset += unique_pattern_count[chn];
        }
    }

    /* Step 4. Print the pattern pointer table. */
    print_pattern_table(xm->header.channel_count, unused_channels,
                        unique_pattern_count, options->label_prefix, out);

    /* Step 5. Print song header + order tables. */
    print_song_struct(xm->header.channel_count, unused_channels,
                      xm->header.default_tempo + 1, order_data_size,
                      order_data, song_length,
                      options->label_prefix, out);

    /* Cleanup */
    for (chn = 0; chn < xm->header.channel_count; ++chn)
        free(unique_pattern_indexes[chn]);
    free(unique_pattern_indexes);
    free(unique_pattern_count);
    free(order_data);
    free(order_data_size);
    free(used_patterns_set);
}
