#ifndef __NET_H__
    #define __NET_H__

    #include "__base__.h"
    #include "VoidPtrLinkList.h"

    #define X_IN_A_TO_B(x, a, b) a <= x &&x <= b

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

    #define __NETNODE_SIZE__ (sizeof(NetNode))

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
        node->Size = (size_t)((uintptr_t)End - (uintptr_t)Start);
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
        if (Root->CHead == NULL || Root->CHead->len == 0)
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
        NN_Delete(Root);
        return;
    }

    NetNode *NN_FindChild(NetNode *Root, void *TPtr)
    {
        VPLL *p = Root->CHead->next;
        while (p != NULL)
        {
            NetNode *CAns = (NetNode *)(p->ptr);
            if (CAns->Start <= TPtr && TPtr < CAns->End)
                return CAns;
        }
        return NULL;
    }

    NetNode *NN_SearchAll(NetNode *Root, void *TPtr, VPLL *Roots)
    {
        if (VPLL_Find(Roots, Root) != -1)
            return NULL;
        if (Root->CHead->len == 0)
        {
            if (Root->Start <= TPtr && TPtr <= Root->End)
                return Root;
            else
                return NULL;
        }
        if (Root->Start <= TPtr && TPtr <= Root->End)
            return Root;
        VPLL_Add(Roots, (void *)Root);
        VPLL *p = Root->CHead->next;
        while (p != NULL)
        {
            NetNode *CAns = NN_SearchAll((NetNode *)(p->ptr), TPtr, Roots);
            if (CAns != NULL)
            {
                return CAns;
            }
            p = p->next;
        }
        VPLL_Delete(Roots, VPLL_Find(Roots, Root));
        return NULL;
    }

    NetNode *NN_Find(NetNode *Root, void *TPtr, bool SearchChildren)
    {
        if (Root->CHead->len == 0)
        {
            if (Root->Start <= TPtr && TPtr <= Root->End)
                return Root;
            else
                return NULL;
        }
        if (!SearchChildren)
            return NN_FindChild(Root, TPtr);
        if (SearchChildren)
        {
            VPLL *Roots = VPLL_Create();
            NetNode *CAns = NN_SearchAll(Root, TPtr, Roots);
            VPLL_Free(Roots);
            return CAns;
        }
    }

    typedef NetNode NN;

#endif