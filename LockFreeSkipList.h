#pragma once

#include <set>
#include <mutex>
#include <atomic>

#include "stamped_ptr.h"
#include "infordered.h"


template <class Pheet, typename TT>
class LockFreeSkipList {

    static const int MAX_LEVEL=30;
    
public:
    
    typedef typename Pheet::Mutex Mutex;
    typedef typename Pheet::LockGuard LockGuard;

    struct Node {
        infordered<TT> key;
        int top_level;
        std::vector<stamped_ptr<Node>> next;

        Node(int height)
            : key()
            , top_level(height)
            , next(height+1)
        {
        }
        
        Node(const infordered<TT>& key, int height)
            : key(key)
            , top_level(height)
            , next(height+1)
        {
        }

        void init(const TT& k)
        {
            key = k;

            for (size_t i = 0; i < next.size(); ++i) {
                next[i].set(nullptr, false, 0);
            }
        }
    };

    
private:

    
    Node* head;
    Node* tail;
    std::atomic<size_t> item_count;

    LockFreeSkipList(LockFreeSkipList& other); // private copy constructor


private:


    bool find(TT key, Node** preds, Node** succs);
    
    
public:
    
    LockFreeSkipList();
    ~LockFreeSkipList();

    
    bool add(TT const& item);
    bool remove(TT const& item);
    bool contains(TT const& item);
    
    size_t size(); // Does not have to be exact, but needs to be > 0 if not empty

    int check_marks();

    static int random_level();
    static void print_name();
    
};



/// Template method implementations start here


template <class Pheet, typename TT>
LockFreeSkipList<Pheet,TT>::LockFreeSkipList()
    : head(new Node(infordered<TT>::min(), MAX_LEVEL))
    , tail(new Node(infordered<TT>::max(), MAX_LEVEL))
    , item_count(0)
{
    for (size_t i = 0; i < head->next.size(); i++) {
        head->next[i].set(tail, false, 0);
    }
}


template <class Pheet, typename TT>
LockFreeSkipList<Pheet,TT>::~LockFreeSkipList()
{
 
}



template <class Pheet, typename TT>
bool LockFreeSkipList<Pheet,TT>::find(TT key, Node** preds, Node** succs)
{
    int bottom_level = 0;

    bool marked = false;
    bool snip;

    Node* pred = nullptr;
    Node* curr = nullptr;
    Node* succ = nullptr;

    uint16_t predstamp;
    uint16_t currstamp;
    
 retry:

    pred = head;

    for (int level = MAX_LEVEL; level >= bottom_level; level--) {

        curr = pred->next[level].get_ref();

        while (true) {

            curr->next[level].get(succ, marked, currstamp);

            while(marked) {
                snip = pred->next[level].compare_and_set(curr, false, currstamp, succ, false, currstamp);

                if (!snip) goto retry;

                curr = pred->next[level].get_ref();
                curr->next[level].get(succ, marked, currstamp);
            }

            if (curr->key < key) {
                pred = curr;
                curr = succ;
            } else {
                break;
            }            

        }
        
        preds[level] = pred;
        succs[level] = curr;        
    }
    
    return (curr->key == key);
}




template <class Pheet, typename TT>
bool LockFreeSkipList<Pheet,TT>::add(TT const& key)
{
    int bottom_level = 0;

    Node* preds[MAX_LEVEL + 1];
    Node* succs[MAX_LEVEL + 1];

    while (true) {
        bool found = find(key, preds, succs);

        if (found) {
            return false;
        }

        Node* new_node = new Node(random_level());
        new_node->init(key);


        int top_level = new_node->top_level;
        
        for (int level = bottom_level; level <= top_level; level++) {
            Node* succ = succs[level];
            new_node->next[level].set(succ, false, 0);
        }

        Node* pred = preds[bottom_level];
        Node* succ = succs[bottom_level];

        if (!pred->next[bottom_level].compare_and_set(succ, false, 0, new_node, false, 0)) {
            continue;
        }

        for (int level = bottom_level+1; level <= top_level; level++) {
            while (true) {
                pred = preds[level];
                succ = succs[level];
                if (pred->next[level].compare_and_set(succ, false, 0, new_node, false, 0)) {
                    break;
                }
                find(key, preds, succs);
            }
        }
        
        return true;
    }
    
}



template <class Pheet, typename TT>
bool LockFreeSkipList<Pheet, TT>::contains(TT const& key)
{
    int bottom_level = 0;

    uint16_t currstamp;
    
    bool marked = false;
    bool snip;
    
 retry:
    
    Node* pred = head;
    Node* curr = nullptr;
    Node* succ = nullptr;

    for (int level = MAX_LEVEL; level >= bottom_level; level--) {

        curr = pred->next[level].get_ref();

        while (true) {

            curr->next[level].get(succ, marked, currstamp);

            while (marked) {
                snip = pred->next[level].compare_and_set(curr, false, 0, succ, false, 0);
                if (!snip) goto retry;
                                
                curr = pred->next[level].get_ref();
                curr->next[level].get(succ, marked, currstamp);
            }

            if (curr->key < key) {
                pred = curr;
                curr = succ;
            } else {
                break;
            }
        }
    }
    
    return (curr->key == key);
}


template <class Pheet, typename TT>
bool LockFreeSkipList<Pheet, TT>::remove(TT const& key)
{
    int bottom_level = 0;

    Node* preds[MAX_LEVEL + 1];
    Node* succs[MAX_LEVEL + 1];

    Node* succ;

    while (true) {
        bool found = find(key, preds, succs);

        if (!found) {
            return false;
        }

        Node* node_to_remove = succs[bottom_level];

        for (int level = node_to_remove->top_level; level >= bottom_level+1; level--) {
            bool marked = false;
            uint16_t stamp;
            node_to_remove->next[level].get(succ, marked, stamp);
            
            while (!marked) {
                node_to_remove->next[level].compare_and_set(succ, false, 0, succ, true, 0);
                node_to_remove->next[level].get(succ, marked, stamp);
            }
        }

        bool marked = false;
        uint16_t stamp;
        
        node_to_remove->next[bottom_level].get(succ, marked, stamp);

        while (true) {
            bool i_marked_it = node_to_remove->next[bottom_level].compare_and_set(succ, false, 0, succ, true, 0);

            succs[bottom_level]->next[bottom_level].get(succ, marked, stamp);

            if (i_marked_it) {
                find (key, preds, succs);
                return true;
            } else if (marked) {
                return false;
            }
        }
    }
}


template <class Pheet, typename TT>
size_t LockFreeSkipList<Pheet, TT>::size()
{
    return item_count;
}


template <class Pheet, typename TT>
int LockFreeSkipList<Pheet, TT>::random_level()
{
    int level = 0;

    while (level <= MAX_LEVEL && Pheet::rand_int(1) > 0) {
        ++level;
    }

    return level;
}


template <class Pheet, typename TT>
int LockFreeSkipList<Pheet, TT>::check_marks()
{
    int markcount = 0;
    
    for (int level = MAX_LEVEL; level >= 0; level--) {
        Node* node = head;

        while (node != nullptr) {
            bool mark;

            node->next[level].get(node, mark);

            if (mark) markcount++;
        }
    }

    return markcount;
}


template <class Pheet, typename TT>
void LockFreeSkipList<Pheet, TT>::print_name()
{
    std::cout << "LockFreeSkipList";
}
