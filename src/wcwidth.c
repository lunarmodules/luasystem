// This file was modified from the original version by Markus Kuhn

/* Original copyrights:
 *
 * Markus Kuhn -- 2007-05-26 (Unicode 5.0)
 *
 * Permission to use, copy, modify, and distribute this software
 * for any purpose and without fee is hereby granted. The author
 * disclaims all warranties with regard to this software.
 *
 * Latest version: http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c
 */

#include "wcwidth.h"

struct interval {
  int first;
  int last;
};

/* auxiliary function for binary search in interval table */
static int bisearch(mk_wchar_t ucs, const struct interval *table, int max) {
  int min = 0;
  int mid;

  if (ucs < table[0].first || ucs > table[max].last)
    return 0;
  while (max >= min) {
    mid = (min + max) / 2;
    if (ucs > table[mid].last)
      min = mid + 1;
    else if (ucs < table[mid].first)
      max = mid - 1;
    else
      return 1;
  }

  return 0;
}


/* The following two functions define the column width of an ISO 10646
 * characters.
 *
 * @param ucs the Unicode code point to check
 * @param ambiguous_width the width to return for ambiguous width characters (1 or 2)
 * @return the width of the character, or -1 if the character is a control character
 */

int mk_wcwidth(mk_wchar_t ucs, int ambiguous_width)
{
  static const struct interval zero_width_ranges[] = {
    #include "wcwidth_zero_width.c"
  };
  static const struct interval ambiguous_width_ranges[] = {
    #include "wcwidth_ambiguous_width.c"
  };
  static const struct interval double_width_ranges[] = {
    #include "wcwidth_double_width.c"
  };

  /* test for 8-bit control characters */
  if (ucs == 0)
    return 0;
  if (ucs < 32 || (ucs >= 0x7f && ucs < 0xa0))
    return -1;

  /* binary search in table of non-spacing characters */
  if (bisearch(ucs, zero_width_ranges,
	       sizeof(zero_width_ranges) / sizeof(struct interval) - 1))
    return 0;

  /* binary search in table of ambiguous width characters */
  if (bisearch(ucs, ambiguous_width_ranges,
	       sizeof(ambiguous_width_ranges) / sizeof(struct interval) - 1))
    return ambiguous_width;

  /* binary search in table of double width characters, default to 1 width */
  return 1 + (bisearch(ucs, double_width_ranges,
    sizeof(double_width_ranges) / sizeof(struct interval) - 1));
}


int mk_wcswidth(const mk_wchar_t *pwcs, size_t n, int ambiguous_width)
{
  int w, width = 0;

  for (;*pwcs && n-- > 0; pwcs++)
    if ((w = mk_wcwidth(*pwcs, ambiguous_width)) < 0)
      return -1;
    else
      width += w;

  return width;
}

