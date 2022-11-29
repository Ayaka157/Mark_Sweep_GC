// #include "__base__.h"
// #include "VoidPtrLinkList.h"

// int main()
// {
//     VPLL *A = VPLL_Create();
//     VPLL_Free(A);
//     system("PAUSE");
//     return 0;
// }

#include "Net.c"

int main()
{
    // what the fuck ?
    // No leak in this Code...
    // but in that Code it leaked memory !
    int a = 0;
    NetNode *A = NN_Create(&a, &a + __INTSIZE__);
    NetNode *A1 = NN_Create(&a, &a + __INTSIZE__);
    // NetNode *A1 = NN_Create();
    A = NN_AddChild(A, A1);
    NN_FreeAll(A);
    // NN_Delete(A);
    // NN_Delete(A1);
    system("PAUSE");
    return 0;
}