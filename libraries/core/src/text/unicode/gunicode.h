/* gunicode.h - Unicode manipulation functions
 *
 *  Copyright (C) 1999, 2000 Tom Tromey
 *  Copyright 2000, 2005 Red Hat, Inc.
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

#ifndef __G_UNICODE_H__
#define __G_UNICODE_H__

#include <stdint.h>
#ifndef _WIN32
#include <unistd.h>
#endif

typedef int GError;

/* Count the number of elements in an array. The array must be defined
 * as such; using this with a dynamically allocated array will give
 * incorrect results.
 */
#define G_N_ELEMENTS(arr)		(sizeof (arr) / sizeof ((arr)[0]))

/*
 * The G_LIKELY and G_UNLIKELY macros let the programmer give hints to
 * the compiler about the expected result of an expression. Some compilers
 * can use this information for optimizations.
 *
 * The _G_BOOLEAN_EXPR macro is intended to trigger a gcc warning when
 * putting assignments in g_return_if_fail ().
 */
#if defined(__GNUC__) && (__GNUC__ > 2) && defined(__OPTIMIZE__)
#define _G_BOOLEAN_EXPR(expr)                   \
__extension__ ({                               \
int _g_boolean_var_;                         \
if (expr)                                    \
_g_boolean_var_ = 1;                      \
else                                         \
_g_boolean_var_ = 0;                      \
_g_boolean_var_;                             \
})
#define G_LIKELY(expr) (__builtin_expect (_G_BOOLEAN_EXPR(expr), 1))
#define G_UNLIKELY(expr) (__builtin_expect (_G_BOOLEAN_EXPR(expr), 0))
#else
#define G_LIKELY(expr) (expr)
#define G_UNLIKELY(expr) (expr)
#endif

/* These are the possible character classifications.
 * See http://www.unicode.org/Public/UNIDATA/UCD.html#General_Category_Values
 */
typedef enum
{
	G_UNICODE_CONTROL,
	G_UNICODE_FORMAT,
	G_UNICODE_UNASSIGNED,
	G_UNICODE_PRIVATE_USE,
	G_UNICODE_SURROGATE,
	G_UNICODE_LOWERCASE_LETTER,
	G_UNICODE_MODIFIER_LETTER,
	G_UNICODE_OTHER_LETTER,
	G_UNICODE_TITLECASE_LETTER,
	G_UNICODE_UPPERCASE_LETTER,
	G_UNICODE_COMBINING_MARK,
	G_UNICODE_ENCLOSING_MARK,
	G_UNICODE_NON_SPACING_MARK,
	G_UNICODE_DECIMAL_NUMBER,
	G_UNICODE_LETTER_NUMBER,
	G_UNICODE_OTHER_NUMBER,
	G_UNICODE_CONNECT_PUNCTUATION,
	G_UNICODE_DASH_PUNCTUATION,
	G_UNICODE_CLOSE_PUNCTUATION,
	G_UNICODE_FINAL_PUNCTUATION,
	G_UNICODE_INITIAL_PUNCTUATION,
	G_UNICODE_OTHER_PUNCTUATION,
	G_UNICODE_OPEN_PUNCTUATION,
	G_UNICODE_CURRENCY_SYMBOL,
	G_UNICODE_MODIFIER_SYMBOL,
	G_UNICODE_MATH_SYMBOL,
	G_UNICODE_OTHER_SYMBOL,
	G_UNICODE_LINE_SEPARATOR,
	G_UNICODE_PARAGRAPH_SEPARATOR,
	G_UNICODE_SPACE_SEPARATOR
} GUnicodeType;

/* These are the possible line break classifications.
 * Note that new types may be added in the future.
 * Implementations may regard unknown values like G_UNICODE_BREAK_UNKNOWN
 * See http://www.unicode.org/unicode/reports/tr14/
 */
typedef enum
{
	G_UNICODE_BREAK_MANDATORY,
	G_UNICODE_BREAK_CARRIAGE_RETURN,
	G_UNICODE_BREAK_LINE_FEED,
	G_UNICODE_BREAK_COMBINING_MARK,
	G_UNICODE_BREAK_SURROGATE,
	G_UNICODE_BREAK_ZERO_WIDTH_SPACE,
	G_UNICODE_BREAK_INSEPARABLE,
	G_UNICODE_BREAK_NON_BREAKING_GLUE,
	G_UNICODE_BREAK_CONTINGENT,
	G_UNICODE_BREAK_SPACE,
	G_UNICODE_BREAK_AFTER,
	G_UNICODE_BREAK_BEFORE,
	G_UNICODE_BREAK_BEFORE_AND_AFTER,
	G_UNICODE_BREAK_HYPHEN,
	G_UNICODE_BREAK_NON_STARTER,
	G_UNICODE_BREAK_OPEN_PUNCTUATION,
	G_UNICODE_BREAK_CLOSE_PUNCTUATION,
	G_UNICODE_BREAK_QUOTATION,
	G_UNICODE_BREAK_EXCLAMATION,
	G_UNICODE_BREAK_IDEOGRAPHIC,
	G_UNICODE_BREAK_NUMERIC,
	G_UNICODE_BREAK_INFIX_SEPARATOR,
	G_UNICODE_BREAK_SYMBOL,
	G_UNICODE_BREAK_ALPHABETIC,
	G_UNICODE_BREAK_PREFIX,
	G_UNICODE_BREAK_POSTFIX,
	G_UNICODE_BREAK_COMPLEX_CONTEXT,
	G_UNICODE_BREAK_AMBIGUOUS,
	G_UNICODE_BREAK_UNKNOWN,
	G_UNICODE_BREAK_NEXT_LINE,
	G_UNICODE_BREAK_WORD_JOINER,
	G_UNICODE_BREAK_HANGUL_L_JAMO,
	G_UNICODE_BREAK_HANGUL_V_JAMO,
	G_UNICODE_BREAK_HANGUL_T_JAMO,
	G_UNICODE_BREAK_HANGUL_LV_SYLLABLE,
	G_UNICODE_BREAK_HANGUL_LVT_SYLLABLE
} GUnicodeBreakType;

typedef enum
{                         /* ISO 15924 code */
	G_UNICODE_SCRIPT_INVALID_CODE = -1,
	G_UNICODE_SCRIPT_COMMON       = 0,   /* Zyyy */
	G_UNICODE_SCRIPT_INHERITED,          /* Qaai */
	G_UNICODE_SCRIPT_ARABIC,             /* Arab */
	G_UNICODE_SCRIPT_ARMENIAN,           /* Armn */
	G_UNICODE_SCRIPT_BENGALI,            /* Beng */
	G_UNICODE_SCRIPT_BOPOMOFO,           /* Bopo */
	G_UNICODE_SCRIPT_CHEROKEE,           /* Cher */
	G_UNICODE_SCRIPT_COPTIC,             /* Qaac */
	G_UNICODE_SCRIPT_CYRILLIC,           /* Cyrl (Cyrs) */
	G_UNICODE_SCRIPT_DESERET,            /* Dsrt */
	G_UNICODE_SCRIPT_DEVANAGARI,         /* Deva */
	G_UNICODE_SCRIPT_ETHIOPIC,           /* Ethi */
	G_UNICODE_SCRIPT_GEORGIAN,           /* Geor (Geon, Geoa) */
	G_UNICODE_SCRIPT_GOTHIC,             /* Goth */
	G_UNICODE_SCRIPT_GREEK,              /* Grek */
	G_UNICODE_SCRIPT_GUJARATI,           /* Gujr */
	G_UNICODE_SCRIPT_GURMUKHI,           /* Guru */
	G_UNICODE_SCRIPT_HAN,                /* Hani */
	G_UNICODE_SCRIPT_HANGUL,             /* Hang */
	G_UNICODE_SCRIPT_HEBREW,             /* Hebr */
	G_UNICODE_SCRIPT_HIRAGANA,           /* Hira */
	G_UNICODE_SCRIPT_KANNADA,            /* Knda */
	G_UNICODE_SCRIPT_KATAKANA,           /* Kana */
	G_UNICODE_SCRIPT_KHMER,              /* Khmr */
	G_UNICODE_SCRIPT_LAO,                /* Laoo */
	G_UNICODE_SCRIPT_LATIN,              /* Latn (Latf, Latg) */
	G_UNICODE_SCRIPT_MALAYALAM,          /* Mlym */
	G_UNICODE_SCRIPT_MONGOLIAN,          /* Mong */
	G_UNICODE_SCRIPT_MYANMAR,            /* Mymr */
	G_UNICODE_SCRIPT_OGHAM,              /* Ogam */
	G_UNICODE_SCRIPT_OLD_ITALIC,         /* Ital */
	G_UNICODE_SCRIPT_ORIYA,              /* Orya */
	G_UNICODE_SCRIPT_RUNIC,              /* Runr */
	G_UNICODE_SCRIPT_SINHALA,            /* Sinh */
	G_UNICODE_SCRIPT_SYRIAC,             /* Syrc (Syrj, Syrn, Syre) */
	G_UNICODE_SCRIPT_TAMIL,              /* Taml */
	G_UNICODE_SCRIPT_TELUGU,             /* Telu */
	G_UNICODE_SCRIPT_THAANA,             /* Thaa */
	G_UNICODE_SCRIPT_THAI,               /* Thai */
	G_UNICODE_SCRIPT_TIBETAN,            /* Tibt */
	G_UNICODE_SCRIPT_CANADIAN_ABORIGINAL, /* Cans */
	G_UNICODE_SCRIPT_YI,                 /* Yiii */
	G_UNICODE_SCRIPT_TAGALOG,            /* Tglg */
	G_UNICODE_SCRIPT_HANUNOO,            /* Hano */
	G_UNICODE_SCRIPT_BUHID,              /* Buhd */
	G_UNICODE_SCRIPT_TAGBANWA,           /* Tagb */
	
	/* Unicode-4.0 additions */
	G_UNICODE_SCRIPT_BRAILLE,            /* Brai */
	G_UNICODE_SCRIPT_CYPRIOT,            /* Cprt */
	G_UNICODE_SCRIPT_LIMBU,              /* Limb */
	G_UNICODE_SCRIPT_OSMANYA,            /* Osma */
	G_UNICODE_SCRIPT_SHAVIAN,            /* Shaw */
	G_UNICODE_SCRIPT_LINEAR_B,           /* Linb */
	G_UNICODE_SCRIPT_TAI_LE,             /* Tale */
	G_UNICODE_SCRIPT_UGARITIC,           /* Ugar */
	
	/* Unicode-4.1 additions */
	G_UNICODE_SCRIPT_NEW_TAI_LUE,        /* Talu */
	G_UNICODE_SCRIPT_BUGINESE,           /* Bugi */
	G_UNICODE_SCRIPT_GLAGOLITIC,         /* Glag */
	G_UNICODE_SCRIPT_TIFINAGH,           /* Tfng */
	G_UNICODE_SCRIPT_SYLOTI_NAGRI,       /* Sylo */
	G_UNICODE_SCRIPT_OLD_PERSIAN,        /* Xpeo */
	G_UNICODE_SCRIPT_KHAROSHTHI,         /* Khar */
	
	/* Unicode-5.0 additions */
	G_UNICODE_SCRIPT_UNKNOWN,            /* Zzzz */
	G_UNICODE_SCRIPT_BALINESE,           /* Bali */
	G_UNICODE_SCRIPT_CUNEIFORM,          /* Xsux */
	G_UNICODE_SCRIPT_PHOENICIAN,         /* Phnx */
	G_UNICODE_SCRIPT_PHAGS_PA,           /* Phag */
	G_UNICODE_SCRIPT_NKO,                /* Nkoo */
	
	/* Unicode-5.1 additions */
	G_UNICODE_SCRIPT_KAYAH_LI,           /* Kali */
	G_UNICODE_SCRIPT_LEPCHA,             /* Lepc */
	G_UNICODE_SCRIPT_REJANG,             /* Rjng */
	G_UNICODE_SCRIPT_SUNDANESE,          /* Sund */
	G_UNICODE_SCRIPT_SAURASHTRA,         /* Saur */
	G_UNICODE_SCRIPT_CHAM,               /* Cham */
	G_UNICODE_SCRIPT_OL_CHIKI,           /* Olck */
	G_UNICODE_SCRIPT_VAI,                /* Vaii */
	G_UNICODE_SCRIPT_CARIAN,             /* Cari */
	G_UNICODE_SCRIPT_LYCIAN,             /* Lyci */
	G_UNICODE_SCRIPT_LYDIAN              /* Lydi */
} GUnicodeScript;

/* Returns TRUE if current locale uses UTF-8 charset.  If CHARSET is
 * not null, sets *CHARSET to the name of the current locale's
 * charset.  This value is statically allocated, and should be copied
 * in case the locale's charset will be changed later using setlocale()
 * or in some other way.
 */
bool g_get_charset (char **charset);

/* These are all analogs of the <ctype.h> functions.
 */
bool g_unichar_isalnum   (uint32_t c);
bool g_unichar_isalpha   (uint32_t c);
bool g_unichar_iscntrl   (uint32_t c);
bool g_unichar_isdigit   (uint32_t c);
bool g_unichar_isgraph   (uint32_t c);
bool g_unichar_islower   (uint32_t c);
bool g_unichar_isprint   (uint32_t c);
bool g_unichar_ispunct   (uint32_t c);
bool g_unichar_isspace   (uint32_t c);
bool g_unichar_isupper   (uint32_t c);
bool g_unichar_isxdigit  (uint32_t c);
bool g_unichar_istitle   (uint32_t c);
bool g_unichar_isdefined (uint32_t c);
bool g_unichar_iswide    (uint32_t c);
bool g_unichar_iswide_cjk(uint32_t c);
bool g_unichar_iszerowidth(uint32_t c);
bool g_unichar_ismark    (uint32_t c);

/* More <ctype.h> functions.  These convert between the three cases.
 * See the Unicode book to understand title case.  */
uint32_t g_unichar_toupper (uint32_t c);
uint32_t g_unichar_tolower (uint32_t c);
uint32_t g_unichar_totitle (uint32_t c);

/* If C is a digit (according to `g_unichar_isdigit'), then return its
 numeric value.  Otherwise return -1.  */
int32_t g_unichar_digit_value (uint32_t c);

int32_t g_unichar_xdigit_value (uint32_t c);

/* Return the Unicode character type of a given character.  */
GUnicodeType g_unichar_type (uint32_t c);

/* Return the line break property for a given character */
GUnicodeBreakType g_unichar_break_type (uint32_t c);

/* Returns the combining class for a given character */
int32_t g_unichar_combining_class (uint32_t uc);


/* Compute canonical ordering of a string in-place.  This rearranges
 decomposed characters in the string according to their combining
 classes.  See the Unicode manual for more information.  */
void g_unicode_canonical_ordering (uint32_t *string,
								   size_t     len);

/* Compute canonical decomposition of a character.  Returns g_malloc()d
 string of Unicode characters.  RESULT_LEN is set to the resulting
 length of the string.  */
uint32_t *g_unicode_canonical_decomposition (uint32_t  ch,
											 size_t    *result_len);

/* Array of skip-bytes-per-initial character.
 */
extern const char * const g_utf8_skip;

#define g_utf8_next_char(p) (char *)((p) + g_utf8_skip[*(const uint8_t *)(p)])

uint32_t g_utf8_get_char           (const char  *p);
uint32_t g_utf8_get_char_validated (const char *p,
									size_t        max_len);

char*   g_utf8_offset_to_pointer (const char *str,
                                   long        offset);
long    g_utf8_pointer_to_offset (const char *str,
								   const char *pos);
char*   g_utf8_prev_char         (const char *p);
char*   g_utf8_find_next_char    (const char *p,
								   const char *end);
char*   g_utf8_find_prev_char    (const char *str,
								   const char *p);

long g_utf8_strlen (const char *p,
					 size_t       max);

/* Copies n characters from src to dest */
char* g_utf8_strncpy (char       *dest,
					   const char *src,
					   size_t        n);

/* Find the UTF-8 character corresponding to ch, in string p. These
 functions are equivalants to strchr and strrchr */
char* g_utf8_strchr  (const char *p,
					   size_t       len,
					   uint32_t     c);
char* g_utf8_strrchr (const char *p,
					   size_t       len,
					   uint32_t     c);
char* g_utf8_strreverse (const char *str,
						  size_t len);

uint16_t *g_utf8_to_utf16     (const char      *str,
								long             len,
								long            *items_read,
								long            *items_written,
								GError          **error);
uint32_t * g_utf8_to_ucs4      (const char      *str,
								long             len,
								long            *items_read,
								long            *items_written,
								GError          **error);
uint32_t * g_utf8_to_ucs4_fast (const char      *str,
								long             len,
								long            *items_written);
uint32_t * g_utf16_to_ucs4     (const uint16_t  *str,
								long             len,
								long            *items_read,
								long            *items_written,
								GError          **error);
char*     g_utf16_to_utf8     (const uint16_t  *str,
								long             len,
								long            *items_read,
								long            *items_written,
								GError          **error);
uint16_t *g_ucs4_to_utf16     (const uint32_t   *str,
								long             len,
								long            *items_read,
								long            *items_written,
								GError          **error);
char*     g_ucs4_to_utf8      (const uint32_t   *str,
								long             len,
								long            *items_read,
								long            *items_written,
								GError          **error);

/* Convert a single character into UTF-8. outbuf must have at
 * least 6 bytes of space. Returns the number of bytes in the
 * result.
 */
int      g_unichar_to_utf8 (uint32_t    c,
							 char      *outbuf);

/* Validate a UTF8 string, return TRUE if valid, put pointer to
 * first invalid char in **end
 */

bool g_utf8_validate (const char  *str,
                          size_t        max_len,
                          const char **end);

/* Validate a Unicode character */
bool g_unichar_validate (uint32_t ch);

char *g_utf8_strup   (const char *str,
					   size_t       len);
char *g_utf8_strdown (const char *str,
					   size_t       len);
char *g_utf8_casefold (const char *str,
						size_t       len);

typedef enum {
	G_NORMALIZE_DEFAULT,
	G_NORMALIZE_NFD = G_NORMALIZE_DEFAULT,
	G_NORMALIZE_DEFAULT_COMPOSE,
	G_NORMALIZE_NFC = G_NORMALIZE_DEFAULT_COMPOSE,
	G_NORMALIZE_ALL,
	G_NORMALIZE_NFKD = G_NORMALIZE_ALL,
	G_NORMALIZE_ALL_COMPOSE,
	G_NORMALIZE_NFKC = G_NORMALIZE_ALL_COMPOSE
} GNormalizeMode;

char *g_utf8_normalize (const char   *str,
						 size_t         len,
						 GNormalizeMode mode);

int   g_utf8_collate     (const char *str1,
						   const char *str2);
char *g_utf8_collate_key (const char *str,
						   size_t       len);
char *g_utf8_collate_key_for_filename (const char *str,
										size_t       len);

bool g_unichar_get_mirror_char (uint32_t ch,
                                    uint32_t *mirrored_ch);

GUnicodeScript g_unichar_get_script (uint32_t ch);


int utf8ByteOffset( char* str, int offset );
int utf8ByteOffset( const char* str, int offset, int maxLen );

#endif /* __G_UNICODE_H__ */
