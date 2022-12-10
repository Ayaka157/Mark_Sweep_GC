#include "__base__.h"
#include "VoidPtrLinkList.h"
#include "Net.h"

void *GC_Alloc(size_t size);
void GC_Init();
void GC_Exit();
void GC_Mark();
void GC_ClearALl(NN *Root);
void GC_ScanMemory();
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
    GC_Mark();

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

void GC_ClearALl(NN *Root, VPLL *Roots)
{
    if (VPLL_Find(Roots, Root) > 0)
        return;
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
    VPLL_Add(Roots, Root);
    VPLL *p = Root->CHead->next;
    while (p != NULL)
    {
        GC_ClearALl((NN *)(p->ptr), Roots);
        p = p->next;
    }
    free(Root->Start);
    Root->Start = NULL;
    Root->End = NULL;
    Root->Size = 0;
    VPLL_Delete(Roots, 1);
    return;
}

void GC_Exit()
{
    // VPLL *ClearRoots = VPLL_Create();
    // GC_ClearALl(GC_Root, ClearRoots);
    GC_Mark();
    VPLL *p = GC_Root->CHead->next;
    while (p)
    {
        free(((NN *)(p->ptr))->Start);
        p = p->next;
    }
    free(GC_Root->Start);
    // VPLL_Free(ClearRoots);
    p = GC_Root->CHead->next;
    while (p)
    {
        NN_Delete((NN *)(p->ptr));
        p = p->next;
    }
    NN_Delete(GC_Root);
}

void GC_SM(NN *FindRes, void *i)
{
    printf("%p, %d, %p, %d", i, (int)i, *(void **)i, *(int *)i);
    printf("  |  %p, %d\n", FindRes, *(int *)(FindRes->Start));
    FindRes->IsDirRef = true;
    FindRes->RefPtrs = VPLL_Add(FindRes->RefPtrs, i);
}

void GC_ScanMemory()
{
    void *EndPtr = (void *)(jmp_buf_V[0]); 
    void *StartPtr = __GC_Stack_Ptr__;
    VPLL_Free(ScanMemory(StartPtr, EndPtr, false, GC_SM));
}

VPLL *ScanMemory(void *StartPtr, void *EndPtr, bool NeedList, void (*Func)(NN *, void *))
{
    if (!StartPtr || !EndPtr)
        return NULL;
    // printf("Start & End : %p, %p | %d \n\n\n", StartPtr, EndPtr, (int)NeedList);
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
    VPLL *ThisNodesPtrs = ScanMemory(Root->End, Root->Start, true, NULL);

    if (!ThisNodesPtrs)
        return NULL;

    VPLL *p = ThisNodesPtrs->next;

    while (p)
    {
        NN *SubNode  = (NN *)(NN_Find(GC_Root, (*(void **)(p->ptr)), true));
        SubNode->IsIndRef = true;
        VPLL_Add(SubNode->RefPtrs, Root);
        // p = SubNode->RefPtrs->next->ptr - SubNode->RefPtrs->next->len - 1
        // SubNode->RefPtrs->next->len = -((*(void **)(p->ptr)) - SubNode->Start) - 1;
        SubNode->RefPtrs->next->len = -((uintptr_t)(p->ptr) - (uintptr_t)(Root->Start)) - 1;
        VPLL_Add(Root->CHead, SubNode);
        Root->CHead->next->len = -((uintptr_t)(*(void **)(p->ptr)) - (uintptr_t)(SubNode->Start)) - 1;
        _Mark(SubNode,Roots);
        p = p->next;
    }

    Roots = VPLL_Delete(Roots,1);
    VPLL_Free(ThisNodesPtrs);
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

void GC_Mark()
{
    GC_ScanMemory();
    VPLL *MarkRoots = VPLL_Create();
    if (GC_Root->CHead->len > 0)
        _Mark((NN *)(GC_Root->CHead->next->ptr), MarkRoots);
    VPLL_Free(MarkRoots);
}

typedef struct Foo
{
    char *x;
    char y;
    void *p;
} Foo;

int main()
{
    __GC_INIT__;

    /* testing code */

    char *x = (char *)GC_Alloc(__CHARSIZE__ * 4);
    memcpy(x, "AB\n\0", sizeof("AB\n\0"));

    Foo *f1 = (Foo *)GC_Alloc(sizeof(Foo));
    f1->x = x;
    f1->y = 'Z';
    f1->p = NULL;
    Foo *f2 = (Foo *)GC_Alloc(sizeof(Foo));
    f2->x = x;
    f2->y = 'Z';
    f2->p = f1;
    f1->p = f2;

    // f->x = (char *)GC_Alloc(__CHARSIZE__ * (strlen(x) + 1));

    // memcpy(f->x, x, __CHARSIZE__ * (strlen(x) + 1));

    // char *y = (char *)GC_Alloc(__CHARSIZE__ * 4);
    // printf("%c\n\n\n\n", *x);
    // printf(x);

    
    // VPLL *GetPtrs = ScanMemory()
    system("PAUSE");
    return 0;
}