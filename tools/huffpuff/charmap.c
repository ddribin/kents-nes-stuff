/*
 * $Id: charmap.c,v 1.1 2004/06/30 07:55:35 kenth Exp $
 * $Log: charmap.c,v $
 * Revision 1.1  2004/06/30 07:55:35  kenth
 * Initial revision
 *
 */

/**
 *    (C) 2004 Kent Hansen
 *
 *    The XORcyst is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    The XORcyst is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with The XORcyst; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/** This file contains functions for parsing a custom character map.
 * Such a map is used by the assembler to process .char directives.
 * The map is a text file containing a series of lines of the following form:
 *
 * key=value
 * OR: keylo-keyhi=value (specifies a range of keys starting at value)
 *
 * where key is a character or C escape sequence, and value is an integer.
 * # is considered the start of a comment; the rest of the line is ignored.
 *
 * Examples:
 * # map lowercase letters starting at 0xC0, so that a=0xC0, b=0xC1, ...
 * a-z=0xC0
 * # map some punctuation
 * !=0x60
 * ?=0x63
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "charmap.h"

/*---------------------------------------------------------------------------*/

/**
 * Issues a character map error.
 * @param filename File where error occured
 * @param line Line of file
 * @param fmt printf-style format string
 */
static void maperr(const char *filename, int line, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    /* Print error message */
    fprintf(stderr, "error: %s:%d: ", filename, line);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    va_end(ap);
}

#define IS_SPACE(c) ( ((c) == '\t') || ((c) == ' ') )

/**
 * Eats whitespace.
 * @param s String with whitespace (possibly)
 * @param i Start index in string, will be incremented beyond whitespace
 */
static void eat_ws(char *s, int *i)
{
    while (IS_SPACE(s[*i])) (*i)++;
}

/**
 * Parses a key.
 * @param s Pointer to buffer containing key
 * @param i Pointer to index of first character of key in s
 * @param d Where to store the parsed key
 */
static int get_key(char *s, int *i, char *d)
{
    char key;
    /* Read first character */
    key = s[(*i)++];
    /* Make sure we've not hit end of string */
    if ((key == '\0') || (key == '\n')) { return 0; }
    /* Check if escape character */
    if (key == '\\') {
        key = s[(*i)++];
        /* Make sure we've not hit end of string */
        if ((key == '\0') || (key == '\n')) { return 0; }
        /* Convert to C escape char if applicable */
        switch (key) {
            case '0':   key = '\0'; break;
            case 'a':   key = '\a'; break;
            case 'b':   key = '\b'; break;
            case 't':   key = '\t'; break;
            case 'f':   key = '\f'; break;
            case 'n':   key = '\n'; break;
            case 'r':   key = '\r'; break;
        }
    }
    /* Copy to output */
    *d = key;
    /* Success */
    return 1;
}

/**
 * Parses a value.
 * @param s Pointer to first character of value
 */
static unsigned char get_value(char *s)
{
    if (s[0] == '$') {
        return strtol(&s[1], NULL, 16);
    } else if (s[0] == '%') {
        return strtol(&s[1], NULL, 2);
    }
    return strtol(s, NULL, 0);
}

/**
 * Parses a character map from file.
 * @param filename Name of the character map file
 * @param map 256-byte buffer where parsed map shall be stored
 * @return 0 if fail, 1 if OK
 */
int charmap_parse(const char *filename, unsigned char *map)
{
    int lineno;
    FILE *fp;
    char line[1024];
    /* Attempt to open the file */
    fp = fopen(filename, "rt");
    if (fp == NULL) {
        return 0;
    }
    /* Reset line counter */
    lineno = 0;
    /* Read mappings */
    while (fgets(line, 1023, fp) != NULL) {
        unsigned char key;
        unsigned char hkey;
        unsigned char value;
        int i;
        /* Increase line number */
        lineno++;
        /* Comment? */
        if (line[0] == '#')
            continue;
        /* Reset line index */
        i = 0;
        /* Read key */
        if (get_key(line, &i, &key) == 0) {
            maperr(filename, lineno, "key expected");
            continue;
        }
        /* Check if this is a range definition */
        if (line[i] == '-') {
            /* Eat - */
            i++;
            /* Read high key */
            if (get_key(line, &i, &hkey) == 0) {
                maperr(filename, lineno, "high limit key expected");
                continue;
            }
            /* Make sure hkey larger or equal to key */
            if (hkey < key) {
                maperr(filename, lineno, "invalid range");
                continue;
            }
        }
        else {
            hkey = key;
        }
        /* Eat whitespace */
        eat_ws(line, &i);
        /* Verify = */
        if (line[i] != '=') {
            maperr(filename, lineno, "`=' expected");
            continue;
        }
        /* Eat = */
        i++;
        /* Eat whitespace */
        eat_ws(line, &i);
        /* Make sure we've not hit end of string */
        if ((line[i] == '\0') || (line[i] == '\n')) {
            maperr(filename, lineno, "value expected");
            return 0;
        }
        /* Read value */
        value = get_value(&line[i]);
        /* Store mapping(s) */
        for (; key <= hkey; key++) {
            map[key] = value++;
        }
    }
    /* Close the file */
    fclose(fp);
    /* Success */
    return 1;
}
