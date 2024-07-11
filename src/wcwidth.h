// wcwidth.h

// Windows does not have a wcwidth function, so we use compatibilty code from
// http://www.cl.cam.ac.uk/~mgk25/ucs/wcwidth.c by Markus Kuhn

#ifndef MK_WCWIDTH_H
#define MK_WCWIDTH_H


#ifdef _WIN32
#include <stdint.h>
#include <stddef.h>
typedef uint32_t mk_wchar_t; // Windows wchar_t can be 16-bit, we need 32-bit
#else
#include <wchar.h>
typedef wchar_t mk_wchar_t;  // Posix wchar_t is 32-bit so just use that
#endif

int mk_wcwidth(mk_wchar_t ucs);
int mk_wcswidth(const mk_wchar_t *pwcs, size_t n);

#endif // MK_WCWIDTH_H
