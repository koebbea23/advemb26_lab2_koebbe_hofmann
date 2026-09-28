#include "case_swap.h"

char switch_case(char c)
{
    // ASCII upper and lower case letters differ by 32 ('a' - 'A' == 32).
    if (c <= 'z' && c >= 'a') return c - 32;
    else if (c >= 'A' && c <= 'Z') return c + 32;
    else return c;
}
