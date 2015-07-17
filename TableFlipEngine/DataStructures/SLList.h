/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __SLLIST_H__
#define __SLLIST_H__

//singly linked list
template <typename type>
class SLList
{
private:
    size_t count;
    
    struct Node
    {
        Node *next;
        type item;
        Node(type t) { item = t; next = 0; }
    };
public:
    class iterator
    {
        SLList<type> *list;
        Node *pos;
        bool good;
        iterator();
    public:
        
        iterator(SLList *l) { list = l; reset(); }
        inline void reset() { pos = list->head; good = pos ? true : false; }
        //moves iterator forward one.
        inline void next()
        {
            if (pos->next != 0)
                pos = pos->next;
            else
                good = false;
        }
        inline bool isGood() { return good; }
        
	//XXX BEWARE if size() == 0 will cause crash!
        type get() { return pos->item; }
    };

    //root
    Node *head;

public:
    SLList() { head = 0; count = 0; }
    ~SLList() { while (count) pop_front(); }

    inline size_t size() { return count; }
    inline type front()  { return head ? head->item : 0; }
    inline iterator getIterator() { return iterator(this); }
    
    void push_front(type t)
    {
        Node *n = new Node(t);
        n->next = head;
        head = n;
        ++count;
    }

    void pop_front()
    {
        if (head)
        {
            Node *n = head;
            head = head->next;
            delete n;
            count--;
        }
    }

    //removes the first found, no checks for duplicates
    void remove(type t)
    {
        if (head == 0)
            return;
        
        if (head->item == t)
        {
            pop_front();
            return;
        }

        //not at head, check the whole list.
        Node *prev = head;
        Node *n = head->next;
        while (n)
        {
            if (n->item == t)
            {
                prev->next = n->next;
                delete n;
                count--;
                return;
            }
            prev = n;
            n = n->next;
        }
    }

    //returns true if found in list
    bool has(type t)
    {
        Node *n = head;
        while(n)
        {
            if (n->item == t)
                return true;
            else
                n = n->next;
        }
        return false;
    }
    
};




#endif
