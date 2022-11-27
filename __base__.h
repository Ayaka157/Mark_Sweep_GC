#ifndef __BASE_H__
    #define __BASE_H__

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <stdbool.h>
    #include <stdint.h>
    #include <setjmp.h>
    #include <assert.h>

    typedef char Byte;
    #define __CHARSIZE sizeof(char)
    #define __PTRSIZE sizeof(int*)
    #define __INTSIZE sizeof(int)
    #define __DOUBLESIZE sizeof(double)
    #define __FLOATSIZE sizeof(float)
    #define __LONGSIZE sizeof(long)

    #define UNIMPLEMENTED                                                 \
        do                                                                \
        {                                                                 \
            fprintf(stderr, "%s:%zu: TODO: %s has not implemented yet\n", \
                    __FILE__, __LINE__, __func__);                        \
            abort();                                                      \
        } while (0)

    #define max(a, b) a > b ? a : b
    #define min(a, b) a < b ? a : b
    #define abs(x) x > 0 ? x : -x
#endif