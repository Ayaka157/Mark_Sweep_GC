#ifndef __VPLL_H__
#define __VPLL_H__
#include "__base__.h"

struct VoidPtrLinkList;
typedef struct VoidPtrLinkList
{
    int len;
    void *ptr;
    VoidPtrLinkList *next;
} VoidPtrLinkList;

void VPLL_Free(VoidPtrLinkList *head);
int VPLL_Find(VoidPtrLinkList *head, void *Ptr);
VoidPtrLinkList *VPLL_Add(VoidPtrLinkList *head, const void *ptr);
VoidPtrLinkList *VPLL_Create_Node();
VoidPtrLinkList *VPLL_Create();

VoidPtrLinkList *VPLL_Create()
{
    VoidPtrLinkList *head = VPLL_Create_Node();
    head->len = 0;
    return head;
}

VoidPtrLinkList *VPLL_Create_Node()
{
    VoidPtrLinkList *head = (VoidPtrLinkList *)malloc(sizeof(VoidPtrLinkList));
    head->len = -1;
    head->ptr = NULL;
    head->next = NULL;
    return head;
}

VoidPtrLinkList *VPLL_Add(VoidPtrLinkList *head, void *ptr)
{
    VoidPtrLinkList *p = head;
    while (p->next != NULL)
        p = p->next;
    p->next = VPLL_Create_Node();
    p->next->ptr = ptr;
    head->len++;
    return head;
}

VoidPtrLinkList *VPLL_Delete(VoidPtrLinkList *head, int index)
{
    if (index <= 0)
        return head;
    if (index > head->len)
        return head;
    VoidPtrLinkList *p = head;
    VoidPtrLinkList *q = NULL;
    int i = 0;
    while (p != NULL)
    {
        if (i == index)
        {
            q->next = p->next;
            free(p);
            head->len--;
            return head;
        }
        q = p;
        p = p->next;
        i++;
    }
    return head;
}

void VPLL_Free(VoidPtrLinkList *head)
{
    while (head->len > 0)
        head = VPLL_Delete(head, 1);
    free(head);
}

int VPLL_Find(VoidPtrLinkList *head, void *Ptr)
{
    VoidPtrLinkList *p = head;
    int i = 0;
    while (p != NULL)
    {
        if (p->ptr == Ptr)
        {
            head->ptr = (void *)p;
            return i;
        }
        p = p->next;
    }
    head->ptr = NULL;
    return -1;
}

typedef VoidPtrLinkList VPLL;

#endif