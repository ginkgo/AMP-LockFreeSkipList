#pragma once

#include <mutex>
#include <atomic>

#include "stamped_ptr.h"
#include "infordered.h"
#include "ItemPool.h"

template <class Pheet, typename TT>
class LockFreeListSet {

public:
    

    struct Node {
        infordered<TT> key;
        stamped_ptr<Node> next;

        Node()
            : key()
        {

        }
        
        Node(infordered<TT> key)
            : key(key)
        {

        }

        uint16_t init(const TT& k)
        {
            key = k;
            
            uint16_t stamp = next.get_stamp();
            
            return stamp;
        }

        
        bool atomic_get_next(Node*& succ, bool& thismarked, uint16_t& thisstamp, infordered<TT>& thiskey) const
        {
            next.get(succ, thismarked, thisstamp);
            thiskey = key;
            
            return thisstamp == next.get_stamp();
        }
    };

    typedef ItemPool<Node> NodePool;

    
private:

    
    Node* head;
    Node* tail;
    
    std::atomic<size_t> item_count;

    LockFreeListSet(LockFreeListSet& other); // private copy constructor


private:
    
    
    bool find(TT const& item, Node*& pred, Node*&succ, Node*& succsucc, uint16_t& predstamp, uint16_t& succstamp);

    
public:
    
    LockFreeListSet();
    ~LockFreeListSet();

    
    bool add(TT const& item);
    bool remove(TT const& item);
    bool contains(TT const& item);
    
    size_t size(); // Does not have to be exact, but needs to be > 0 if not empty

    static void print_name();
    
};


/// Template method implementations start here


template <class Pheet, typename TT>
LockFreeListSet<Pheet,TT>::LockFreeListSet()
    : head(new Node(infordered<TT>::min()))
    , tail(new Node(infordered<TT>::max()))
    , item_count(0)
{
    head->next.set(tail, false, 0);
}

template <class Pheet, typename TT>
LockFreeListSet<Pheet,TT>::~LockFreeListSet()
{
    Node* n = head;
    Node* p;

    while (n) {
        p = n;
        n = n->next.get_ref();
    }
}



template <class Pheet, typename TT>
bool LockFreeListSet<Pheet, TT>::find(TT const& key, Node*& pred, Node*& curr, Node*& succ, uint16_t& predstamp, uint16_t& currstamp)
{
    NodePool& pool = Pheet::template place_singleton<NodePool>();
    
    bool marked = false;
    bool snip;

    infordered<TT> predkey;
    infordered<TT> currkey;
    
 retry:
    
    pred = head;

    pred->atomic_get_next(curr, marked, predstamp, predkey);

    while (true) {

        if (!curr->atomic_get_next(succ, marked, currstamp, currkey)) goto retry;

        assert(predkey < currkey);
        
        while (marked) {
            snip = pred->next.compare_and_set(curr, false, predstamp, succ, false, predstamp);
            if (!snip) goto retry;

            bool success = curr->next.compare_and_set(succ, true, currstamp, succ, true, currstamp+1);
            assert(success);

            if (currstamp < stamped_ptr<Node>::MAX_STAMP) {
                pool.release(curr);
            } else {
                pool.retire(curr);
            }
            
            curr = succ;
            if (!curr->atomic_get_next(succ, marked, currstamp, currkey)) goto retry;
        }
        
        if (currkey < key) {
            pred = curr;
            curr = succ;

            predstamp = currstamp;
            predkey = currkey;
        } else {
            return currkey == key;
        }
    }
}





template <class Pheet, typename TT>
bool LockFreeListSet<Pheet,TT>::add(TT const& key)
{
    NodePool& pool = Pheet::template place_singleton<NodePool>();
    
    Node* node = pool.acquire();
    uint16_t nodestamp = node->init(key);
    
    while (true) {
        Node* pred;
        Node* succ;
        Node* succsucc;
        uint16_t predstamp;
        uint16_t succstamp;

        if (find(key, pred, succ, succsucc, predstamp, succstamp)) {
            pool.release(node);
            return false;
        }
        
        node->next.set(succ, false, nodestamp);
        
        if (pred->next.compare_and_set(succ, false, predstamp, node, false, predstamp)) {
            item_count++;
            return true;
        } 
    }
}



template <class Pheet, typename TT>
bool LockFreeListSet<Pheet, TT>::contains(TT const& key)
{
    Node* pred;
    Node* curr;
    Node* succ;
    uint16_t predstamp;
    uint16_t currstamp;

    return find(key, pred, succ, curr, predstamp, currstamp);
}


template <class Pheet, typename TT>
bool LockFreeListSet<Pheet, TT>::remove(TT const& key)
{
    NodePool& pool = Pheet::template place_singleton<NodePool>();
    
    bool snip;

    while (true) {
        Node* pred;
        Node* curr;
        Node* succ;
        uint16_t predstamp;
        uint16_t currstamp;

        if (!find(key, pred, curr, succ, predstamp, currstamp)) {
            return false;
        }
        
        snip = curr->next.compare_and_set(succ, false, currstamp, succ, true, currstamp);
        if (!snip) {
            continue;
        }

        item_count--;
        
        return true;
    }
        
}


template <class Pheet, typename TT>
size_t LockFreeListSet<Pheet, TT>::size()
{
    return item_count;
}


template <class Pheet, typename TT>
void LockFreeListSet<Pheet, TT>::print_name()
{
    // Should also print some relevant configuration parameters if applicable
    std::cout << "LockFreeListSet";
}
