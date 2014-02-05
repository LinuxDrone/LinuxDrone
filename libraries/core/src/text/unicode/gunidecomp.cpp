/* decomp.c - Character decomposition.
 *
 *  Copyright (C) 1999, 2000 Tom Tromey
 *  Copyright 2000 Red Hat, Inc.
 *
 * The Gnome Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * The Gnome Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the Gnome Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *   Boston, MA 02111-1307, USA.
 */
/*
 * (c) 2010-2012  Original developer
 */

#include <stdlib.h>
#include "gunicode.h"

#include "gunidecomp.h"

#include "../../memmanager/my_memory.h"


#define CC_PART1(Page, Char) \
((combining_class_table_part1[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
? (combining_class_table_part1[Page] - G_UNICODE_MAX_TABLE_INDEX) \
: (cclass_data[combining_class_table_part1[Page]][Char]))

#define CC_PART2(Page, Char) \
((combining_class_table_part2[Page] >= G_UNICODE_MAX_TABLE_INDEX) \
? (combining_class_table_part2[Page] - G_UNICODE_MAX_TABLE_INDEX) \
: (cclass_data[combining_class_table_part2[Page]][Char]))

#define COMBINING_CLASS(Char) \
(((Char) <= G_UNICODE_LAST_CHAR_PART1) \
? CC_PART1 ((Char) >> 8, (Char) & 0xff) \
: (((Char) >= 0xe0000 && (Char) <= G_UNICODE_LAST_CHAR) \
? CC_PART2 (((Char) - 0xe0000) >> 8, (Char) & 0xff) \
: 0))

/**
 * g_unichar_combining_class:
 * @uc: a Unicode character
 *
 * Determines the canonical combining class of a Unicode character.
 *
 * Return value: the combining class of the character
 *
 * Since: 2.14
 **/
int
g_unichar_combining_class (uint32_t uc)
{
	return COMBINING_CLASS (uc);
}

/* constants for hangul syllable [de]composition */
#define SBase 0xAC00
#define LBase 0x1100
#define VBase 0x1161
#define TBase 0x11A7
#define LCount 19
#define VCount 21
#define TCount 28
#define NCount (VCount * TCount)
#define SCount (LCount * NCount)

/**
 * g_unicode_canonical_ordering:
 * @string: a UCS-4 encoded string.
 * @len: the maximum length of @string to use.
 *
 * Computes the canonical ordering of a string in-place.
 * This rearranges decomposed characters in the string
 * according to their combining classes.  See the Unicode
 * manual for more information.
 **/
void
g_unicode_canonical_ordering (uint32_t *string,
							  size_t     len)
{
	size_t i;
	int swap = 1;
	
	while (swap)
    {
		int last;
		swap = 0;
		last = COMBINING_CLASS (string[0]);
		for (i = 0; i < len - 1; ++i)
		{
			int next = COMBINING_CLASS (string[i + 1]);
			if (next != 0 && last > next)
			{
				size_t j;
				/* Percolate item leftward through string.  */
				for (j = i + 1; j > 0; --j)
				{
					uint32_t t;
					if (COMBINING_CLASS (string[j - 1]) <= next)
						break;
					t = string[j];
					string[j] = string[j - 1];
					string[j - 1] = t;
					swap = 1;
				}
				/* We're re-entering the loop looking at the old
				 character again.  */
				next = last;
			}
			last = next;
		}
    }
}

/* http://www.unicode.org/unicode/reports/tr15/#Hangul
 * r should be null or have sufficient space. Calling with r == NULL will
 * only calculate the result_len; however, a buffer with space for three
 * characters will always be big enough. */
static void
decompose_hangul (uint32_t s,
                  uint32_t *r,
                  size_t *result_len)
{
	int SIndex = s - SBase;
	
	/* not a hangul syllable */
	if (SIndex < 0 || SIndex >= SCount)
    {
		if (r)
			r[0] = s;
		*result_len = 1;
    }
	else
    {
		uint32_t L = LBase + SIndex / NCount;
		uint32_t V = VBase + (SIndex % NCount) / TCount;
		uint32_t T = TBase + SIndex % TCount;
		
		if (r)
        {
			r[0] = L;
			r[1] = V;
        }
		
		if (T != TBase)
        {
			if (r)
				r[2] = T;
			*result_len = 3;
        }
		else
			*result_len = 2;
    }
}

/* returns a pointer to a null-terminated UTF-8 string */
static const char *
find_decomposition (uint32_t ch,
					bool compat)
{
	int start = 0;
	int end = G_N_ELEMENTS (decomp_table);
	
	if (ch >= decomp_table[start].ch &&
		ch <= decomp_table[end - 1].ch)
    {
		while (true)
		{
			int half = (start + end) / 2;
			if (ch == decomp_table[half].ch)
			{
				int offset;
				
				if (compat)
				{
					offset = decomp_table[half].compat_offset;
					if (offset == G_UNICODE_NOT_PRESENT_OFFSET)
						offset = decomp_table[half].canon_offset;
				}
				else
				{
					offset = decomp_table[half].canon_offset;
					if (offset == G_UNICODE_NOT_PRESENT_OFFSET)
						return NULL;
				}
				
				return &(decomp_expansion_string[offset]);
			}
			else if (half == start)
				break;
			else if (ch > decomp_table[half].ch)
				start = half;
			else
				end = half;
		}
    }
	
	return NULL;
}

/**
 * g_unicode_canonical_decomposition:
 * @ch: a Unicode character.
 * @result_len: location to store the length of the return value.
 *
 * Computes the canonical decomposition of a Unicode character.
 *
 * Return value: a newly allocated string of Unicode characters.
 *   @result_len is set to the resulting length of the string.
 **/
uint32_t *
g_unicode_canonical_decomposition (uint32_t ch,
								   size_t   *result_len)
{
	const char *decomp;
	const char *p;
	uint32_t *r;
	
	/* Hangul syllable */
	if (ch >= 0xac00 && ch <= 0xd7a3)
    {
		decompose_hangul (ch, NULL, result_len);
		r = (uint32_t*)my_malloc (*result_len * sizeof (uint32_t));
		decompose_hangul (ch, r, result_len);
    }
	else if ((decomp = find_decomposition (ch, false)) != NULL)
    {
		/* Found it.  */
		int i;
		
		*result_len = g_utf8_strlen (decomp, -1);
		r = (uint32_t*)my_malloc (*result_len * sizeof (uint32_t));
		
		for (p = decomp, i = 0; *p != '\0'; p = g_utf8_next_char (p), i++)
			r[i] = g_utf8_get_char (p);
    }
	else
    {
		/* Not in our table.  */
		r = (uint32_t*)my_malloc (sizeof (uint32_t));
		*r = ch;
		*result_len = 1;
    }
	
	/* Supposedly following the Unicode 2.1.9 table means that the
     decompositions come out in canonical order.  I haven't tested
     this, but we rely on it here.  */
	return r;
}

/* L,V => LV and LV,T => LVT  */
//static bool
//combine_hangul (uint32_t a,
//                uint32_t b,
//                uint32_t *result)
//{
//	int LIndex = a - LBase;
//	int SIndex = a - SBase;
	
//	int VIndex = b - VBase;
//	int TIndex = b - TBase;
	
//	if (0 <= LIndex && LIndex < LCount
//		&& 0 <= VIndex && VIndex < VCount)
//    {
//		*result = SBase + (LIndex * VCount + VIndex) * TCount;
//		return true;
//    }
//	else if (0 <= SIndex && SIndex < SCount && (SIndex % TCount) == 0
//			 && 0 < TIndex && TIndex < TCount)
//    {
//		*result = a + TIndex;
//		return true;
//    }
	
//	return false;
//}

#define __G_UNIDECOMP_C__
