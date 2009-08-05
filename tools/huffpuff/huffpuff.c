/*
    This file is part of huffpuff.

    huffpuff is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    huffpuff is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with huffpuff.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "huffpuff.h"
#include "charmap.h"

/**
 * Creates a Huffman node.
 * @param symbol The symbol that this node represents, or -1 if it's not a leaf node
 * @param weight The weight of this node
 * @param left The node's left child
 * @param right The node's right child
 * @return The new node
 */
huffman_node_t *huffman_create_node(int symbol, int weight, huffman_node_t *left, huffman_node_t *right)
{
    huffman_node_t *node = (huffman_node_t *)malloc(sizeof(huffman_node_t));
    node->symbol = symbol;
    node->weight = weight;
    node->left = left;
    node->right = right;
    return node;
}

/**
 * Deletes a Huffman node (tree) recursively.
 * @param node The node to delete
 */
void huffman_delete_node(huffman_node_t *node)
{
    if (node == 0)
        return;
    if (node->symbol == -1) {
        huffman_delete_node(node->left);
        huffman_delete_node(node->right);
    }
    free(node);
}

/**
 * Generates codes for a Huffman node (tree) recursively.
 * @param node Node
 * @param length Current length (in bits)
 * @param code Current code
 */
static void huffman_generate_codes(huffman_node_t *node, int length, int code)
{
    if (!node)
        return;
    node->code.length = length;
    node->code.code = code;
    if (node->symbol == -1) {
        length++;
        code <<= 1;
        huffman_generate_codes(node->left, length, code);
        code |= 1;
        huffman_generate_codes(node->right, length, code);
    }
}

/**
 * Builds a Huffman tree from an array of leafnodes with weights set.
 * @param nodes Array of Huffman leafnodes
 * @param nodecount Number of nodes in the array
 * @return Root of the resulting tree
 */
huffman_node_t *huffman_build_tree(huffman_node_t **nodes, int nodecount)
{
    huffman_node_t *n;
    huffman_node_t *n1;
    huffman_node_t *n2;
    huffman_node_t *root;
    int i, j, k;
    if (nodecount == 0)
        return 0;
    for (i=nodecount-1; i>0; i--) {
        /* Sort nodes based on frequency using simple bubblesort */
        for (j=i; j>0; j--) {
            for (k=0; k<j; k++) {
                if (nodes[k]->weight < nodes[k+1]->weight) {
                    n = nodes[k+1];
                    nodes[k+1] = nodes[k];
                    nodes[k] = n;
                }
            }
        }
        /* Combine nodes with two lowest frequencies */
        n1 = nodes[i];
        n2 = nodes[i-1];
        nodes[i-1] = huffman_create_node(/*symbol=*/-1, n1->weight+n2->weight, n1, n2);
    }
    root = nodes[0];
    /* Generate Huffman codes from tree. */
    huffman_generate_codes(root, /*length=*/0, /*code=*/0);
    return root;
}

struct huffman_node_list {
    struct huffman_node_list *next;
    huffman_node_t *node;
};

typedef struct huffman_node_list huffman_node_list_t;

/**
 * Writes codes for nodes in a Huffman tree recursively.
 * @param out File to write to
 * @param root Root node of Huffman tree
 */
static void write_huffman_codes(FILE *out, huffman_node_t *root,
                                const unsigned char *charmap,
                                const char *label_prefix)
{
    huffman_node_list_t *current;
    huffman_node_list_t *tail;
    if (root == 0)
        return;
    current = (huffman_node_list_t*)malloc(sizeof(huffman_node_list_t));
    current->node = root;
    current->next = 0;
    tail = current;
    while (current) {
        huffman_node_list_t *tmp;
        huffman_node_t *node;
        node = current->node;
        /* label */
        if (node != root)
            fprintf(out, "%snode_%d_%d: ", label_prefix,
                    node->code.code, node->code.length);
        if (node->symbol != -1) {
            /* a leaf node */
            fprintf(out, ".db $00, $%.2X\n", charmap[node->symbol]);
        } else {
            /* an interior node -- print pointers to children */
            huffman_node_list_t *succ;
            fprintf(out, ".db %snode_%d_%d-$, %snode_%d_%d-$+1\n",
                    label_prefix, node->code.code << 1, node->code.length+1,
                    label_prefix, (node->code.code << 1) | 1, node->code.length+1);
            /* add child nodes to list */
            succ = (huffman_node_list_t*)malloc(sizeof(huffman_node_list_t));
            succ->node = node->left;
            succ->next = (huffman_node_list_t*)malloc(sizeof(huffman_node_list_t));
            succ->next->node = node->right;
            succ->next->next = 0;
            tail->next = succ;
            tail = succ->next;
        }
        tmp = current->next;
        free(current);
        current = tmp;
    }
}

/* A linked list of text strings. */
struct string_list {
    struct string_list *next;
    unsigned char *text;
    unsigned char *huff_data;
    int huff_size;
};

typedef struct string_list string_list_t;

/* The end-of-string token. */
#define STRING_SEPARATOR 0x0A

/**
 * Reads strings from a file and computes the frequencies of the characters.
 * @param in File to read from
 * @param freq Where to store computed frequencies
 * @param total_length If not NULL, the total number of characters is stored here
 * @return The resulting list of strings
 */
string_list_t *read_strings(FILE *in, int ignore_case, int *freq,
                            int *total_length, int *string_count)
{
    unsigned char *buf;
    string_list_t *head;
    string_list_t **nextp;
    int max_len;
    int i;
    if (string_count)
        *string_count = 0;

    /* Zap frequency counts. */
    for (i = 0; i < 256; i++)
        freq[i] = 0;

    /* Read strings and count character frequencies as we go. */
    head = NULL;
    nextp = &head;
    max_len = 64;
    buf = (unsigned char *)malloc(max_len);
    if (total_length)
        *total_length = 0;
    while (!feof(in)) {
        /* Read one string (all chars until STRING_SEPARATOR) into temp buffer */
        int c;
        int in_comment = 0;
        i = 0;
        while (((c = fgetc(in)) != -1) && (c != STRING_SEPARATOR)) {
            if (c == '\\') {
                /* Check for line escape */
                int d;
                d = fgetc(in);
                if (d == STRING_SEPARATOR) {
                    continue;
                } else if (d == '#') {
                    c = '#';
                } else {
                    ungetc(d, in);
                }
            } else if ((i == 0) && (c == '#')) {
                in_comment = 1;
	    }
            if (in_comment)
                continue;
            if (i == max_len) {
                /* Allocate larger buffer */
                max_len += 64;
                buf = (unsigned char *)realloc(buf, max_len);
            }
            if (ignore_case && (c >= 'A') && (c <= 'Z'))
                c += 0x20;
            buf[i++] = (unsigned char)c;
            freq[c]++;
        }

        if (i > 0) {
            /* Add string to list */
            string_list_t *lst = (string_list_t *)malloc(sizeof(string_list_t));
            lst->text = (unsigned char *)malloc(i+1);
            lst->huff_data = 0;
            lst->huff_size = 0;
            memcpy(lst->text, buf, i);
            lst->text[i] = 0;
            lst->next = NULL;
            *nextp = lst;
            nextp = &(lst->next);
            if (total_length)
                *total_length = *total_length + i;
            if (string_count)
                *string_count = *string_count + 1;
        }
    }
    free(buf);
    return head;
}

/**
 * Encodes the given list of strings.
 * @param head Head of list of strings to encode
 * @param codes Mapping from character to Huffman node
 * @return The size of the encoded string data
 */
static int encode_strings(string_list_t *head, huffman_node_t * const *codes,
                          int append_byte)
{
    string_list_t *string;
    unsigned char *buf = 0;
    int maxlen = 0;
    int total_size = 0;
    /* Do all strings. */
    for (string = head; string != NULL; string = string->next) {
        /* Do all characters in string. */
        unsigned char enc = 0;
        int i;
        int len = 0;
        int bitnum = 7;
        const unsigned char *p = string->text;
        int apd = append_byte;
        while (1) {
            const huffman_node_t *node;
            unsigned char c;
            if (*p) {
                c = *(p++);
            } else if (apd != -1) {
                c = (unsigned char)append_byte;
                apd = -1;
            } else {
                break;
            }
            node = codes[c];
            for (i = node->code.length-1; i >= 0; i--) {
                enc |= ((node->code.code >> i) & 1) << bitnum--;
                if (bitnum < 0) {
                    if (len == maxlen) {
                        maxlen += 128;
                        buf = (unsigned char *)realloc(buf, maxlen);
                    }
                    buf[len++] = enc;
                    bitnum = 7;
                    enc = 0;
                }
            }
        }
        if (bitnum != 7) {  /* write last few bits */
            if (len == maxlen) {
                maxlen += 128;
                buf = (unsigned char *)realloc(buf, maxlen);
            }
            buf[len++] = enc;
        }
        /* Store encoded buffer */
        string->huff_data = (unsigned char *)malloc(len);
        memcpy(string->huff_data, buf, len);
        string->huff_size = len;
        total_size += len;
    }
    free(buf);
    return total_size;
}

/**
 * Decodes a Huffman-encoded string; helpful for debugging.
 * @param root Root node of Huffman tree
 * @param data Encoded data
 * @param len Length of string
 * @param out Where to store decoded string
 */
static void decode_string(huffman_node_t *root, const unsigned char *data,
                          int len, unsigned char *out)
{
    huffman_node_t *n;
    int mask = 0;
    unsigned char bite;
    int i;
    for (i = 0; i < len; ++i) {
        n = root;
        while (1) {
            if (n->symbol != -1) {
                out[i] = (char)n->symbol;
                break;
            }
            if (!mask) {
                bite = *(data++);
                mask = 0x80;
            }
            int isset = (bite & mask) != 0;
            mask >>= 1;
            if (isset)
                n = n->right;
            else
                n = n->left;
        }
    }
    out[len] = 0;
}

/**
 * Verifies that decoding the Huffman data results in the original strings.
 * @param head Strings
 * @param root Root of Huffman tree
 */
static int verify_data_integrity(string_list_t *head, huffman_node_t *root)
{
    string_list_t *str;
    unsigned char *buf = 0;
    int max_len = 0;
    for (str = head; str != NULL; str = str->next) {
        int len = strlen((char *)str->text);
        if (len > max_len) {
            buf = (unsigned char *)realloc(buf, len + 1);
            max_len = len;
        }
        decode_string(root, str->huff_data, len, buf);
        if (strcmp((char *)buf, (char *)str->text)) {
            fprintf(stderr, "*** fatal error: decoded string is not equal to original string\n");
            fprintf(stderr, "    original: %s\n", str->text);
            fprintf(stderr, "    decoded:  %s\n", buf);
            return 0;
        }

    }
    free(buf);
    return 1;
}

/**
 * Writes a chunk of data as assembly .db statements.
 * @param out File to write to
 * @param label Data label
 * @param comment Comment that describes the data
 * @param buf Data
 * @param size Total number of bytes
 * @param cols Number of columns
 */
static void write_chunk(FILE *out, const char *label, const char *comment,
                        const unsigned char *buf, int size, int cols)
{
    int i, j, k, m;
    int has_label = (label && strlen(label)) ? 1 : 0;
    int has_comment = (comment && strlen(comment)) ? 1 : 0;
    if (has_label)
        fprintf(out, "%s:", label);
    if (has_comment) {
        if (has_label)
            fprintf(out, " ");
        fprintf(out, "; %s", comment);
    }
    fprintf(out, "\n");
    k=0;
    for (i=0; i<size/cols; i++) {
        fprintf(out, ".db ");
        for (j=0; j<cols-1; j++) {
            fprintf(out, "$%.2X,", buf[k++]);
        }
        fprintf(out, "$%.2X\n", buf[k++]);
    }
    m = size % cols;
    if (m > 0) {
        fprintf(out, ".db ");
        for (j=0; j<m-1; j++) {
            fprintf(out, "$%.2X,", buf[k++]);
        }
        fprintf(out, "$%.2X\n", buf[k++]);
    }
}

/**
 * Encodes the strings and writes the encoded data to file.
 * @param out File to write to
 * @param head Head of list of strings to encode & write
 * @param label_prefix
 */
static void write_huffman_strings(FILE *out, const string_list_t *head,
                                  const char *label_prefix)
{
    const string_list_t *string;
    int string_id = 0;
    for (string = head; string != NULL; string = string->next) {
        char strlabel[256];
        char strcomment[80];

        sprintf(strlabel, "%sString%d", label_prefix, string_id++);

        strcpy(strcomment, "\"");
        if (strlen((char *)string->text) < 40) {
            strcat(strcomment, (char *)string->text);
        } else {
            strncat(strcomment, (char *)string->text, 37);
            strcat(strcomment, "...");
        }
        strcat(strcomment, "\"");

        /* Write encoded data */
        write_chunk(out, strlabel, strcomment,
                    string->huff_data, string->huff_size, 16);
    }
}

/**
 * Destroys a string list.
 * @param lst The list to destroy
 */
void destroy_string_list(string_list_t *lst)
{
    string_list_t *tmp;
    for ( ; lst != 0; lst = tmp) {
        tmp = lst->next;
        free(lst->text);
        free(lst->huff_data);
        free(lst);
    }
}

static char program_version[] = "huffpuff 1.0.6";

/* Prints usage message and exits. */
static void usage()
{
    printf(
        "Usage: huffpuff [--character-map=FILE]\n"
        "                [--table-output=FILE] [--data-output=FILE]\n"
        "                [--table-label=LABEL] [--node-label-prefix=PREFIX]\n"
        "                [--string-label-prefix=PREFIX]\n"
        "                [--generate-string-table] [--append-byte=VALUE]\n"
        "                [--ignore-case] [--verbose]\n"
        "                [--help] [--usage] [--version]\n"
        "                FILE\n");
    exit(0);
}

/* Prints help message and exits. */
static void help()
{
    printf("Usage: huffpuff [OPTION...] FILE\n"
           "huffpuff encodes strings using Huffman compression.\n\n"
           "Options:\n\n"
           "  --character-map=FILE            Transform characters according to the rules in FILE before encoding\n"
           "  --table-output=FILE             Store Huffman decoder table definition in FILE\n"
           "  --data-output=FILE              Store Huffman string data definition in FILE\n"
           "  --table-label=LABEL             Create symbolic label LABEL for decoder table definition\n"
           "  --node-label-prefix=PREFIX      Prefix symbolic labels in decoder table definition by PREFIX\n"
           "  --string-label-prefix=PREFIX    Prefix symbolic labels in data definition by PREFIX\n"
           "  --generate-string-table         Generate string pointer table\n"
           "  --string-table-label=LABEL      Create symbolic label LABEL for string pointer table definition\n"
           "  --append-byte=VALUE             Append VALUE to every string before encoding\n"
           "  --ignore-case                   Convert characters to lower-case before processing\n"
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

/**
 * Program entrypoint.
 */
int main(int argc, char **argv)
{
    int char_count;
    int string_count;
    int encoded_size;
    unsigned char charmap[256];
    int frequencies[256];
    huffman_node_t *leaf_nodes[256];
    huffman_node_t *code_nodes[256];
    huffman_node_t *root;
    int symbol_count;
    string_list_t *strings;
    FILE *input;
    FILE *table_output;
    FILE *data_output;
    int append_byte = -1;
    int ignore_case = 0;
    const char *input_filename = 0;
    const char *charmap_filename = 0;
    const char *table_output_filename = 0;
    const char *data_output_filename = 0;
    const char *table_label = "";
    const char *node_label_prefix = "";
    const char *string_table_label = "";
    const char *string_label_prefix = "";
    int generate_string_table = 0;
    int verbose = 0;

    /* Process arguments. */
    {
        char *p;
        while ((p = *(++argv))) {
            if (!strncmp("--", p, 2)) {
                const char *opt = &p[2];
                if (!strncmp("character-map=", opt, 14)) {
                    charmap_filename = &opt[14];
                } else if (!strncmp("table-output=", opt, 13)) {
                    table_output_filename = &opt[13];
                } else if (!strncmp("data-output=", opt, 12)) {
                    data_output_filename = &opt[12];
                } else if (!strncmp("table-label=", opt, 12)) {
                    table_label = &opt[12];
                } else if (!strncmp("node-label-prefix=", opt, 18)) {
                    node_label_prefix = &opt[18];
                } else if (!strncmp("string-label-prefix=", opt, 20)) {
                    string_label_prefix = &opt[20];
                    generate_string_table = 1;
                } else if (!strcmp("generate-string-table", opt)) {
                    generate_string_table = 1;
                } else if (!strncmp("string-table-label=", opt, 19)) {
                    string_table_label = &opt[19];
                } else if (!strncmp("append-byte=", opt, 12)) {
                    append_byte = strtol(&opt[12], 0, 0);
                    if ((append_byte < 0) || (append_byte >= 256)) {
                        fprintf(stderr, "huffpuff: --append-byte: value must be in range 0..255\n");
                        return(-1);
                    }
                } else if (!strcmp("ignore-case", opt)) {
                    ignore_case = 1;
                } else if (!strcmp("verbose", opt)) {
                    verbose = 1;
                } else if (!strcmp("help", opt)) {
                    help();
                } else if (!strcmp("usage", opt)) {
                    usage();
                } else if (!strcmp("version", opt)) {
                    version();
                } else {
                    fprintf(stderr, "huffpuff: unrecognized option `%s'\n"
			    "Try `huffpuff --help' or `huffpuff --usage' for more information.\n", p);
                    return(-1);
                }
            } else {
                input_filename = p;
            }
        }
    }

    /* Set default character mapping f(c)=c */
    {
        int i;
        for (i=0; i<256; i++)
            charmap[i] = (unsigned char)i;
    }

    if (charmap_filename) {
        if (verbose)
            fprintf(stdout, "reading character map\n");
        if (!charmap_parse(charmap_filename, charmap)) {
            fprintf(stderr, "error: failed to parse character map `%s'\n",
                    charmap_filename);
            return(-1);
        }
    }

    if (input_filename) {
        input = fopen(input_filename, "rt");
        if (!input) {
            fprintf(stderr, "error: failed to open `%s' for reading\n",
                    input_filename);
            return(-1);
        }
    } else {
        input = stdin;
    }

    /* Read strings to encode. */
    if (verbose)
        fprintf(stdout, "reading strings\n");
    strings = read_strings(input, ignore_case, frequencies, &char_count, &string_count);
    fclose(input);

    /* Create Huffman leaf nodes. */
    if (verbose)
        fprintf(stdout, "creating Huffman leaf nodes\n");
    symbol_count = 0;
    {
        int i;
        if (append_byte != -1)
            frequencies[append_byte] += string_count;
        for (i=0; i<256; i++) {
            if (frequencies[i] > 0) {
                huffman_node_t *node;
                node = huffman_create_node(
                    /*symbol=*/i, /*weight=*/frequencies[i],
                    /*left=*/NULL, /*right=*/NULL);
                leaf_nodes[symbol_count++] = node;
                code_nodes[i] = node;
            } else {
                code_nodes[i] = 0;
            }
        }
    }
    if (verbose)
        fprintf(stdout, "  number of symbols: %d\n", symbol_count);

    /* Build the Huffman tree. */
    if (verbose)
        fprintf(stdout, "Building the Huffman tree\n");
    root = huffman_build_tree(leaf_nodes, symbol_count);

    /* Huffman-encode strings. */
    if (verbose)
        fprintf(stdout, "encoding strings\n");
    encoded_size = encode_strings(strings, code_nodes, append_byte);

    /* Sanity check */
    if (verbose)
        fprintf(stdout, "verifying output integrity\n");
    if (!verify_data_integrity(strings, root)) {
        assert(0);
        /* Cleanup */
        huffman_delete_node(root);
        destroy_string_list(strings);
        return(-1);
    }

    /* Prepare output */
    if (!table_output_filename) {
        table_output_filename = "huffpuff.tab.asm";
    }
    table_output = fopen(table_output_filename, "wt");
    if (!table_output) {
        fprintf(stderr, "error: failed to open `%s' for writing\n",
                table_output_filename);
        /* Cleanup */
        huffman_delete_node(root);
        destroy_string_list(strings);
        return(-1);
    }

    if (!data_output_filename) {
        data_output_filename = "huffpuff.dat.asm";
    }
    data_output = fopen(data_output_filename, "wt");
    if (!data_output) {
        fprintf(stderr, "error: failed to open `%s' for writing\n",
                data_output_filename);
        /* Cleanup */
        huffman_delete_node(root);
        destroy_string_list(strings);
        return(-1);
    }
    fprintf(data_output, "; Huffman-encoded string data automatically generated by huffpuff.\n");

    /* Print the Huffman codes in code length order. */
    if (verbose)
        fprintf(stdout, "writing Huffman decoder table\n");
    fprintf(table_output, "; Huffman decoder table automatically generated by huffpuff.\n");
    if (table_label && strlen(table_label))
        fprintf(table_output, "%s:\n", table_label);
    write_huffman_codes(table_output, root, charmap, node_label_prefix);

    fclose(table_output);

    if (generate_string_table) {
        /* Print string pointer table */
        int i;
        string_list_t *lst;
        if (verbose)
            fprintf(stdout, "writing string pointer table\n");
        if (string_table_label && strlen(string_table_label))
            fprintf(data_output, "%s:\n", string_table_label);
        for (i = 0, lst = strings; lst != 0; lst = lst->next, ++i) {
            fprintf(data_output, ".dw %sString%d\n",
                    string_label_prefix, i);
        }
    }

    /* Write the Huffman-encoded strings. */
    if (verbose)
        fprintf(stdout, "writing encoded string data\n");
    write_huffman_strings(data_output, strings, string_label_prefix);

    fclose(data_output);

    if (verbose)
        fprintf(stdout, "compressed size: %d%%\n", (encoded_size*100) / char_count);

    /* Cleanup */
    huffman_delete_node(root);
    destroy_string_list(strings);

    return 0;
}
