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

#ifndef HUFFPUFF_H
#define HUFFPUFF_H

/* A Huffman code */
struct huffman_code {
    int code;
    int length;
};

/* A Huffman node */
struct huffman_node {
    int symbol;
    int weight;
    struct huffman_node *left;
    struct huffman_node *right;
    struct huffman_code code;
};

typedef struct huffman_node huffman_node_t;

huffman_node_t *huffman_create_node(int, int, huffman_node_t *, huffman_node_t *);
void huffman_delete_node(huffman_node_t *);
huffman_node_t *huffman_build_tree(huffman_node_t **, int);

#endif /* HUFFPUFF_H */
