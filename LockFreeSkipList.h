#pragma once

#include <set>
#include <mutex>
#include <atomic>

#include "marked_ptr.h"

template <class Pheet, typename TT, int MAX_LEVEL=30>
class LockFreeSkipList {

public:
    
    typedef typename Pheet::Mutex Mutex;
    typedef typename Pheet::LockGuard LockGuard;

    struct Node {
        TT key;
        int top_level;
        std::vector<marked_ptr<Node>> next;

        Node(TT key, int height)
            : key(key)
            , top_level(height)
            , next(height+1)
        {
        } 
    };

    
private:

    
    Node* head;
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


    static int random_level();
    static void print_name();
    
};



/// Template method implementations start here


template <class Pheet, typename TT, int MAX_LEVEL>
LockFreeSkipList<Pheet,TT, MAX_LEVEL>::LockFreeSkipList()
    : head(new Node(TT(), MAX_LEVEL))
    , item_count(0)
{
};


template <class Pheet, typename TT, int MAX_LEVEL>
LockFreeSkipList<Pheet,TT, MAX_LEVEL>::~LockFreeSkipList()
{
 
};



template <class Pheet, typename TT, int MAX_LEVEL>
bool LockFreeSkipList<Pheet,TT, MAX_LEVEL>::find(TT key, Node** preds, Node** succs)
{
    int bottom_level = 0;

    bool marked = false;
    bool snip;

    Node* pred = nullptr;
    Node* curr = nullptr;
    Node* succ = nullptr;

 retry:

    pred = head;

    for (int level = MAX_LEVEL; level >= bottom_level; level--) {

        curr = pred->next[level].get_ref();

        while (true) {

            if (curr != nullptr) {
                curr->next[level].get(succ, marked);
            }

            while(curr != nullptr && marked) {
                snip = pred->next[level].compare_and_set(curr, succ, false, false);

                if (!snip) goto retry;

                curr = pred->next[level].get_ref();

                if (curr != nullptr) {
                    curr->next[level].get(succ, marked);
                }
            }

            if (curr != nullptr && curr->key < key) {
                pred = curr;
                curr = succ;
            } else {
                break;
            }            

        }
        
        preds[level] = pred;
        succs[level] = curr;        
    }
    return (curr != nullptr && curr->key == key);
    
};




template <class Pheet, typename TT, int MAX_LEVEL>
bool LockFreeSkipList<Pheet,TT, MAX_LEVEL>::add(TT const& key)
{
    int top_level = random_level();
    int bottom_level = 0;

    Node* preds[MAX_LEVEL + 1];
    Node* succs[MAX_LEVEL + 1];

    while (true) {
        bool found = find(key, preds, succs);

        if (found) {
            return false;
        }

        Node* new_node = new Node(key, top_level);

        for (int level = bottom_level; level <= top_level; level++) {
            Node* succ = succs[level];
            new_node->next[level].set(succ, false);
        }

        Node* pred = preds[bottom_level];
        Node* succ = succs[bottom_level];

        if (!pred->next[bottom_level].compare_and_set(succ, new_node, false, false)) {
            continue;
        }

        for (int level = bottom_level+1; level <= top_level; level++) {
            while (true) {
                pred = preds[level];
                succ = succs[level];
                if (pred->next[level].compare_and_set(succ, new_node, false, false)) {
                    break;
                }
                find(key, preds, succs);
            }
        }
        
        return true;
    }
    
}



template <class Pheet, typename TT, int MAX_LEVEL>
bool LockFreeSkipList<Pheet, TT, MAX_LEVEL>::contains(TT const& key)
{
    return false;
}


template <class Pheet, typename TT, int MAX_LEVEL>
bool LockFreeSkipList<Pheet, TT, MAX_LEVEL>::remove(TT const& key)
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

            node_to_remove->next[level].compare_and_set(succ, succ, false, true);
            node_to_remove->next[level].get(succ, marked);
        }

        bool marked = false;
        node_to_remove->next[bottom_level].get(succ, marked);

        while (true) {
            bool i_marked_it = node_to_remove->next[bottom_level].compare_and_set(succ, succ, false, true);

            succs[bottom_level]->next[bottom_level].get(succ, marked);

            if (i_marked_it) {
                find (key, preds, succs);
                return true;
            } else if (marked) {
                return false;
            }
        }
    }
}


template <class Pheet, typename TT, int MAX_LEVEL>
size_t LockFreeSkipList<Pheet, TT, MAX_LEVEL>::size()
{
    return item_count;
}


template <class Pheet, typename TT, int MAX_LEVEL>
int LockFreeSkipList<Pheet, TT, MAX_LEVEL>::random_level()
{
    int level = 0;

    do {
        ++level;
    } while (level <= MAX_LEVEL && Pheet::rand_int(1) > 0);

    return level-1;
}


template <class Pheet, typename TT, int MAX_LEVEL>
void LockFreeSkipList<Pheet, TT, MAX_LEVEL>::print_name()
{
    std::cout << "LockFreeSkipList";
}
