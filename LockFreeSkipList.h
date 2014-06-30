#pragma once

#include <set>
#include <mutex>
#include <atomic>

#include "stamped_ptr.h"
#include "infordered.h"
#include "ItemPool.h"


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
        std::atomic<bool> add_in_progress;

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

        uint16_t init(const TT& k)
        {
            key = k;

            uint16_t stamp = next[0].get_stamp();
            
            return stamp;
        }

        void atomic_get_next(int level, Node*& succ, bool& thismarked, uint16_t& thisstamp, infordered<TT>& thiskey) const
        {
            do {
                next[level].get(succ, thismarked, thisstamp);
                thiskey = key;
            } while (thisstamp != next[level].get_stamp());
        }
    };

    
    typedef ItemPool<Node> NodePool;

    
private:

    
    Node* head;
    Node* tail;
    std::atomic<size_t> item_count;

    LockFreeSkipList(LockFreeSkipList& other); // private copy constructor


private:


    bool find(TT key, Node** preds, Node** succs, uint16_t* predstamps, uint16_t* succstamps);
    
    
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
    Node* n = head;
    Node* p;

    while (n) {
        p = n;
        n = n->next[0].get_ref();
        delete p;
    }    
}



template <class Pheet, typename TT>
bool LockFreeSkipList<Pheet,TT>::find(TT key, Node** preds, Node** succs, uint16_t* predstamps, uint16_t* succstamps)
{
    NodePool& pool = Pheet::template place_singleton<NodePool>();
    
    int bottom_level = 0;

    bool currmarked = false;
    bool snip;

    uint16_t predstamp;
    uint16_t currstamp;

    infordered<TT> predkey;
    infordered<TT> currkey;
    
 retry:

    Node* pred = head;
    Node* curr = nullptr;
    Node* succ = nullptr;

    predstamp = 0;
    predkey = infordered<TT>::min();

    for (int level = MAX_LEVEL; level >= bottom_level; level--) {

        curr = pred->next[level].get_ref();

        while (true) {

            curr->atomic_get_next(level, succ, currmarked, currstamp, currkey);

            assert(predkey <= currkey);
            
            while(currmarked) {
                snip = pred->next[level].compare_and_set(curr, false, predstamp, succ, false, predstamp);

                if (!snip) {
                    goto retry;
                }

                if (level == 0) {
                    // curr has been completely unlinked
                    // Increment stamp

                    for (int i = curr->top_level; i >= 0; i--) {
                        Node* n = curr->next[i].get_ref();

                        bool success = curr->next[i].compare_and_set(n, true, currstamp, n, true, currstamp+1);

                        assert(success);
                    }
                    
                    if (false && currstamp < stamped_ptr<Node>::MAX_STAMP && !curr->add_in_progress) {
                        pool.release(curr);
                    } else {
                        pool.retire(curr);
                    }
                }

                curr = pred->next[level].get_ref();
                curr->atomic_get_next(level, succ, currmarked, currstamp, currkey);
            }

            if (currkey < key) {
                pred = curr;
                curr = succ;

                predkey = currkey;
                predstamp = currstamp;
            } else {
                break;
            }            

        }
        
        preds[level] = pred;
        succs[level] = curr;

        predstamps[level] = predstamp;
        succstamps[level] = currstamp;
    }
    
    return (currkey == key);
}




template <class Pheet, typename TT>
bool LockFreeSkipList<Pheet,TT>::add(TT const& key)
{
    NodePool& pool = Pheet::template place_singleton<NodePool>();
    int bottom_level = 0;

    Node* preds[MAX_LEVEL + 1];
    Node* succs[MAX_LEVEL + 1];

    uint16_t predstamps[MAX_LEVEL + 1];
    uint16_t succstamps[MAX_LEVEL + 1];

    Node* new_node = pool.acquire(random_level());
    uint16_t nodestamp = new_node->init(key);
    new_node->add_in_progress = true;
        
    while (true) {
        bool found = find(key, preds, succs, predstamps, succstamps);

        if (found) {
            new_node->add_in_progress = false;
            pool.release(new_node);
            return false;
        }

        int top_level = new_node->top_level;
        
        for (int level = bottom_level; level <= top_level; level++) {
            Node* succ = succs[level];
            new_node->next[level].set(succ, false, nodestamp);
        }

        Node* pred = preds[bottom_level];
        Node* succ = succs[bottom_level];

        uint16_t predstamp = predstamps[bottom_level];

        if (!pred->next[bottom_level].compare_and_set(succ, false, predstamp, new_node, false, predstamp)) {
            continue;
        }

        item_count++;
        
        for (int level = bottom_level+1; level <= top_level; level++) {
            while (true) {
                pred = preds[level];
                succ = succs[level];

                predstamp = predstamps[level];

                // What if new_node has been reused at this point?
                // new_node->add_in_progress should forbid reuse while set - Deleted nodes have to be retired
                
                if (pred->next[level].compare_and_set(succ, false, predstamp, new_node, false, predstamp)) {
                    break;
                }
                find(key, preds, succs, predstamps, succstamps);
            }
        }

        new_node->add_in_progress = false;
        
        return true;
    }
    
}





template <class Pheet, typename TT>
bool LockFreeSkipList<Pheet, TT>::remove(TT const& key)
{
    int bottom_level = 0;

    Node* preds[MAX_LEVEL + 1];
    Node* succs[MAX_LEVEL + 1];

    uint16_t predstamps[MAX_LEVEL + 1];
    uint16_t succstamps[MAX_LEVEL + 1];
    
    Node* succ;

    bool found = find(key, preds, succs, predstamps, succstamps);

    if (!found) {
        return false;
    }

    Node* node_to_remove = succs[bottom_level];
    uint16_t nodestamp = succstamps[bottom_level];
        

    for (int level = node_to_remove->top_level; level >= bottom_level+1; level--) {
        bool marked = false;
        uint16_t stamp;
        node_to_remove->next[level].get(succ, marked, stamp);
            
        while (!marked && stamp == nodestamp) {
            node_to_remove->next[level].compare_and_set(succ, false, nodestamp,
                                                        succ, true, nodestamp);
            node_to_remove->next[level].get(succ, marked, stamp);
        }
    }

    bool marked = false;
    uint16_t stamp;
        
    node_to_remove->next[bottom_level].get(succ, marked, stamp);

    while (true) {
        bool i_marked_it =
            node_to_remove->next[bottom_level].compare_and_set(succ, false, nodestamp,
                                                               succ, true, nodestamp);
            
        succs[bottom_level]->next[bottom_level].get(succ, marked, stamp);

        if (i_marked_it) {
            item_count--;
            //find (key, preds, succs, predstamps, succstamps);
            return true;
        } else if (marked || stamp > nodestamp) {
            return false;
        }
    }
}


template <class Pheet, typename TT>
bool LockFreeSkipList<Pheet, TT>::contains(TT const& key)
{
    
    Node* preds[MAX_LEVEL + 1];
    Node* succs[MAX_LEVEL + 1];

    uint16_t predstamps[MAX_LEVEL + 1];
    uint16_t succstamps[MAX_LEVEL + 1];

    return find(key, preds, succs, predstamps, succstamps);
    
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
void LockFreeSkipList<Pheet, TT>::print_name()
{
    std::cout << "LockFreeSkipList";
}
