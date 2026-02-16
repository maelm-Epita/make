#ifndef CODES_H
#define CODES_H

#include <stdio.h>

enum CODES
{
    CODE_SUCCESS = 0,
    CODE_FAILURE = 2,
};

#define ERR(msg)                                                               \
    {                                                                          \
        fprintf(stderr, "minimake: *** %s.  Stop.\n", msg);                    \
    }

#endif /* ! CODES_H */
