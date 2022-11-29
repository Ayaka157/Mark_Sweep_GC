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
    #define __CHARSIZE__ sizeof(char)
    #define __PTRSIZE__ sizeof(int*)
    #define __INTSIZE__ sizeof(int)
    #define __DOUBLESIZE__ sizeof(double)
    #define __FLOATSIZE__ sizeof(float)
    #define __LONGSIZE__ sizeof(long)

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