#define MAX_NUM_LEN 6

#include "common.hh"

uint_t
strtoui(const char* s)
{
    unsigned int r = 0;
    for (int i = 0; i < MAX_NUM_LEN; i++) {
        if (s[i] < '0' || '9' < s[i]) return r;
        r = r * 10 + (s[i] - '0');
    }

    return r;
}
