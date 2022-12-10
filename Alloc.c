#include "__base__.h"
#include "VoidPtrLinkList.h"
#include "Net.h"

void *GC_Alloc(size_t size);
void GC_Init();
void GC_Exit();
void GC_ClearALl(NN *Root);
VPLL *ScanMemory(void *StartPtr, void *EndPtr, bool NeedList, void (*Func)(NN *, void *));

void *__GC_Stack_Ptr__ = NULL;
jmp_buf jmp_buf_V;

#define __GC_INIT__                            \
    setjmp(jmp_buf_V);                         \
    __GC_Stack_Ptr__ = (void *)(jmp_buf_V[0]); \
    atexit(GC_Exit);                           \
    GC_Init()

NN *GC_Root = NULL;

void *GC_Alloc(size_t size)
{
    void *Ret = malloc(size);
    assert(GC_Root);
    NN *NewNode = NN_Create(Ret, Ret + size);
    GC_Root = NN_AddChild(GC_Root, NewNode);
    return Ret;
}

void GC_Init()
{
    GC_Root = NN_Create(NULL, NULL);
}

void GC_ClearALl(NN *Root)
{
    if (!Root)
        return;
    if (Root->CHead == NULL || Root->CHead->len == 0)
    {
        free(Root->Start);
        Root->Start = NULL;
        Root->End = NULL;
        Root->Size = 0;
        return;
    }
    VPLL *p = Root->CHead->next;
    while (p != NULL)
    {
        GC_ClearALl((NN *)(p->ptr));
        p = p->next;
    }
    free(Root->Start);
    Root->Start = NULL;
    Root->End = NULL;
    Root->Size = 0;
    return;
}

void GC_Exit()
{
    GC_ClearALl(GC_Root);
    NN_FreeAll(GC_Root);
}

void GC_SM(NN *FindRes, void *i)
{
    printf("%p, %d, %p, %d", i, (int)i, *(void **)i, *(int *)i);
    printf("  |  %p, %d\n", FindRes, *(int *)(FindRes->Start));
    FindRes->IsDirRef = true;
    FindRes->RefPtrs = VPLL_Add(FindRes->RefPtrs, i);
}

void GC_ScanMemory(void *StartPtr, void *EndPtr)
{
    if (EndPtr == NULL)
    {
        setjmp(jmp_buf_V);
        EndPtr = (void *)(jmp_buf_V[0]);
    }
    if (StartPtr == NULL)
        StartPtr = __GC_Stack_Ptr__;
    // printf("Start & End : %p, %p | %B \n\n\n", StartPtr, EndPtr, NeedList);
    // VPLL *RetPtrs = NULL;
    // if (NeedList)
    //     RetPtrs = VPLL_Create();
    // for (void *i = StartPtr; i >= EndPtr; i -= 4)
    // {
    //     NN *FindRes = NN_Find(GC_Root, *(void **)i, true);
    //     // if (Heap < *(void **)i && Heap + HEAP_CAP > *(void **)i)
    //     if (FindRes)
    //     // if (FindRes && FindRes != GC_Root)
    //     {
    //         if (FindRes != GC_Root)
    //         {
    //             printf("%p, %d, %p, %d", i, (int)i, *(void **)i, *(int *)i);
    //             printf("  |  %p, %d\n", FindRes, *(int *)(FindRes->Start));
    //             FindRes->IsDirRef = true;
    //             FindRes->RefPtrs = VPLL_Add(FindRes->RefPtrs, i);
    //         }
    //         // i = FindRes->End + 4;
    //     }
    // }
    VPLL_Free(ScanMemory(StartPtr, EndPtr, false, GC_SM));
}

VPLL *ScanMemory(void *StartPtr, void *EndPtr, bool NeedList, void (*Func)(NN *, void *))
{
    if (!StartPtr || !EndPtr)
        return NULL;
    printf("Start & End : %p, %p | %d \n\n\n", StartPtr, EndPtr, (int)NeedList);
    VPLL *RetPtrs = NULL;
    if (NeedList)
        RetPtrs = VPLL_Create();
    for (void *i = StartPtr; i >= EndPtr; i -= 4)
    {
        NN *FindRes = NN_Find(GC_Root, *(void **)i, true);
        if (FindRes)
        {
            if (FindRes != GC_Root)
            {
                if (Func != NULL)
                    Func(FindRes, i);
                if (NeedList)
                    VPLL_Add(RetPtrs, i);
            }
        }
    }
    return RetPtrs;
}

VPLL *_Mark(NN *Root, VPLL *Roots)
{
    if (VPLL_Find(Roots, Root) >= 0)
        return NULL;
    VPLL_Add(Roots, Root);
    VPLL *ThisNodesPtrs = ScanMemory(Root->Start, Root->End, true, NULL);

    VPLL *p = ThisNodesPtrs->next;

    while (p)
    {
        NN *SubNode  = (NN *)(NN_Find(GC_Root, (*(void **)(p->ptr)), true));
        SubNode->IsIndRef = true;
        VPLL_Add(SubNode->RefPtrs, Root);
        // p = SubNode->RefPtrs->next->ptr - SubNode->RefPtrs->next->len - 1
        // SubNode->RefPtrs->next->len = -((*(void **)(p->ptr)) - SubNode->Start) - 1;
        SubNode->RefPtrs->next->len = -(p->ptr - Root->Start) - 1;
        VPLL_Add(Root->CHead, SubNode)
        Root->CHead->next->len = -((*(void **)(p->ptr)) - SubNode->Start) - 1;
        _Mark(SubNode);
        p = p->next;
    }

    VPLL_Free(ThisNodesPtrs);

    // TODO: _Mark() - Mark GC_Net
    return NULL;
}
/*

NetNode 结构及其中变量的计算/用途:

// RefPtrs 的非head节点中，p.len = -(引用指针对应的Node.Start的偏移量)-1
// 引用指针对应的Node.Start的偏移量 = -p.len-1
// 则引用指针 = (对应的Node.Start) + 引用指针对应的Node.Start的偏移量 = Node.Start - p.len - 1

// Chead 的非head节点中，p.len = -(指向对象与其标准对齐的偏移量(即对应的Node.Start的与指向的地址偏移量))-1
// 指向对象与其标准对齐的偏移量 = -p.len-1
// 引用地址 = (对应的Node.Start) + 指向对象与其标准对齐的偏移量 = Node.Start - p.len - 1

// 例： 节点A所对内存：[============]; 节点B所对内存：[===115======]
//                                                    p
//      191          0   1   2   3   810          0   1   2   3
//      191         114 115 116 117  810         514 515 516 517
// 则记录A的Node中的RefPtrs有{.next = ##, .ptr = 810, .len = -2}
// 则记录B的Node中的CHead有{.next = ##, .ptr = 191, .len = -2}
// A节点的引用指针计算： 514 - A.RefPtrs.next.len - 1
//                  = 514 - (-2) - 1
//                  = 514 + 2 - 1
//                  = 515
//               所以 引用A的指针地址515(指针记作p), &p = 515
// 用A节点计算指向它的节点B中的指针真正的指向
// 真正的指向(有偏移) = A.Start - 810.CHead.next.len - 1
//                  = 114 - (810.CHead.next.len = -2) - 1
//                  = 114 - -2 -1
//                  = 114 + 2 - 1
//                  = 115
//               所以 真正其指向的地址是115(指针记作p), *p = 115
*/

typedef struct Foo
{
    char *x;
    char y;
} Foo;

int main()
{
    __GC_INIT__;

    /* testing code */

    // char *x = (char *)GC_Alloc(__CHARSIZE__ * 4);
    // memcpy(x, "AB\n\0", sizeof("AB\n\0"));

    // Foo *f = (Foo *)GC_Alloc(sizeof(Foo));

    // f->x = (char *)GC_Alloc(__CHARSIZE__ * (strlen(x) + 1));

    // memcpy(f->x, x, __CHARSIZE__ * (strlen(x) + 1));
    // // f->x = x;
    // f->y = 'Z';

    // // char *y = (char *)GC_Alloc(__CHARSIZE__ * 4);
    // // printf("%c\n\n\n\n", *x);
    // // printf(x);

    GC_ScanMemory(NULL, NULL);
    // VPLL *GetPtrs = ScanMemory()
    system("PAUSE");
    return 0;
}