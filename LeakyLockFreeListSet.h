#pragma once

#include <set>
#include <mutex>
#include <atomic>

#include "marked_ptr.h"

template <class Pheet, typename TT>
class LeakyLockFreeListSet {

public:
    
    typedef typename Pheet::Mutex Mutex;
    typedef typename Pheet::LockGuard LockGuard;

    struct Node {
        TT key;
        marked_ptr<Node> next;

        Node(TT key)
            : key(key)
        {

        } 
    };

    
private:

    
    Node* head;
    std::atomic<size_t> item_count;

    LeakyLockFreeListSet(LeakyLockFreeListSet& other); // private copy constructor


private:
    
    
    struct Window {
        Node* pred;
        Node* curr;
    };
    
    Window find(Node* head, TT const& item);
    
public:
    
    LeakyLockFreeListSet();
    ~LeakyLockFreeListSet() { };

    
    bool add(TT const& item);
    bool remove(TT const& item);
    bool contains(TT const& item);
    
    size_t size(); // Does not have to be exact, but needs to be > 0 if not empty

    static void print_name();
    
};


/// Template method implementations start here


template <class Pheet, typename TT>
LeakyLockFreeListSet<Pheet,TT>::LeakyLockFreeListSet()
    : item_count(0)
{
    head = new Node(TT());
}




template <class Pheet, typename TT>
typename LeakyLockFreeListSet<Pheet,TT>::Window LeakyLockFreeListSet<Pheet, TT>::find(Node* head, TT const& key)
{
    Node* pred = nullptr;
    Node* curr = nullptr;
    Node* succ = nullptr;

    bool marked = false;
    bool snip;

 retry:
    
    pred = head;
    curr = pred->next.get_ref();

    while (curr != nullptr) {
        curr->next.get(succ, marked);
        while (marked) {
            snip = pred->next.compare_and_set(curr, succ, false, false);
            if (!snip) goto retry;
            curr = succ;
            curr->next.get(succ, marked);
        }
        if (curr->key >= key) {
            return {pred, curr};
        }
        pred = curr;
        curr = succ;
    }

    return {pred, curr};        
}





template <class Pheet, typename TT>
bool LeakyLockFreeListSet<Pheet,TT>::add(TT const& key)
{
    while (true) {
        Window window = find(head, key);
        Node* pred = window.pred;
        Node* curr = window.curr;

        if (curr == nullptr || curr->key != key) {
            Node* node = new Node(key);
            node->next.compare_and_set(nullptr, curr, false, false);
            if (pred->next.compare_and_set(curr, node, false, false)) {
                return true;
            }
        } else {
            return false;
        }
    }
}



template <class Pheet, typename TT>
bool LeakyLockFreeListSet<Pheet, TT>::contains(TT const& key)
{    
    bool marked = false;

    Node* curr = head->next.get_ref();

    while (curr != nullptr && curr->key < key) {
        curr = curr->next.get_ref();

        Node* succ;
        if (curr != nullptr) {
            curr->next.get(succ,marked);
        }
    }

    return (curr != nullptr && curr->key == key && !marked);    
}


template <class Pheet, typename TT>
bool LeakyLockFreeListSet<Pheet, TT>::remove(TT const& key)
{
    bool snip;

    while (true) {
        Window window = find(head,key);
        Node* pred = window.pred;
        Node* curr = window.curr;

        if (curr == nullptr || curr->key != key) {
            return false;
        } else {
            Node* succ = curr->next.get_ref();
            snip = curr->next.compare_and_set(succ, succ, false, true);
            if (!snip) {
                continue;
            }

            pred->next.compare_and_set(curr, succ, false, false);
            return true;
        }
    }
        
}


template <class Pheet, typename TT>
size_t LeakyLockFreeListSet<Pheet, TT>::size()
{
    return item_count;
}


template <class Pheet, typename TT>
void LeakyLockFreeListSet<Pheet, TT>::print_name()
{
    // Should also print some relevant configuration parameters if applicable
    std::cout << "LeakyLockFreeListSet";
}
