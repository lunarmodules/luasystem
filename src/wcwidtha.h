// wcwidtha.h

#ifndef MK_WCWIDTHA_H
#define MK_WCWIDTHA_H


#include "wcwidth.h"

// Is a character in the list of ambiguous width characters (for east asian display)
int mk_wcwidth_a(mk_wchar_t ucs);

#endif // MK_WCWIDTHA_H
