#include "__base__.h"
#include "VoidPtrLinkList.h"

typedef struct NetNode
{
    // int RootNum;
    void *Start;
    void *End;
    size_t Size;
    bool IsDirRef;
    bool IsIndRef;
    VPLL *CHead;
    VPLL *RefPtrs;
} NetNode;

NetNode *NN_Create(void *Start, void *End);
NetNode *NN_AddChild(NetNode *Root, const NetNode *Child);
void NN_Delete(NetNode *node);
void NN_Free(NetNode *node);

NetNode *NN_Create(void *Start, void *End)
{
    NetNode *node = (NetNode *)malloc(sizeof(NetNode));
    // node->RootNum = 1;
    node->Start = Start;
    node->End = End;
    node->Size = abs((uintptr_t)Start - (uintptr_t)End);
    node->IsDirRef = false;
    node->IsIndRef = false;
    node->CHead = VPLL_Create();
    node->RefPtrs = VPLL_Create();
    return node;
}

// Won't free children
void NN_Free(NetNode *node)
{
    VPLL_Free(node->CHead);
    VPLL_Free(node->RefPtrs);
    free(node);
}

void NN_Delete(NetNode *node)
{
    NN_Free(node);
}

NetNode *NN_AddChild(NetNode *Root, NetNode *Child)
{
    VPLL_Add(Root->CHead, Child);
    return Root;
}

void NN_FreeAll(NetNode *Root)
{
    if (Root->CHead->len == 0)
    {
        NN_Delete(Root);
        return;
    }
    VPLL *p = Root->CHead->next;
    while (Root->CHead->len > 0)
    {
        NN_FreeAll((NetNode *)(p->ptr));
        p = p->next;
        VPLL_Delete(Root->CHead, 1);
    }
}

int main()
{
    int a = 0;
    NetNode *A = NN_Create(&a, &a + __INTSIZE__);
    NetNode *A1 = NN_Create(&a, &a + __INTSIZE__);
    // NetNode *A1 = NN_Create();
    A = NN_AddChild(A, A1);
    NN_FreeAll(A);
    // free(A1);
    // free(A);
    system("PAUSE");
    return 0;
}