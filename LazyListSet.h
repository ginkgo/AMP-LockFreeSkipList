#pragma once

#include <set>
#include <mutex>

template <class Pheet, typename TT>
class LazyListSet {

public:
    
    typedef typename Pheet::Mutex Mutex;
    typedef typename Pheet::LockGuard LockGuard;

    struct Node {
        TT key;
        Node* next;
        Mutex lock;

        bool marked;

        Node(TT key)
            : key(key)
            , next(nullptr)
            , marked(false)
        {

        } 
    };

    
private:
    
    Node *head;
    size_t item_count = 0;

    LazyListSet(LazyListSet& other); // private copy constructor

    bool validate (Node* pred, Node* curr);
    
public:
    
    LazyListSet();
    ~LazyListSet() { };

    bool add(TT const& item);
    bool remove(TT const& item);
    bool contains(TT const& item);
    
    size_t size(); // Does not have to be exact, but needs to be > 0 if not empty

    static void print_name();
    
};


/// Template method implementations start here


template <class Pheet, typename TT>
LazyListSet<Pheet,TT>::LazyListSet()
{
    TT v;
    head = new Node(v);
};




template <class Pheet, typename TT>
bool LazyListSet<Pheet, TT>::validate(Node* pred, Node* curr)
{
    return !(pred->marked) && (curr == nullptr || !(curr->marked)) && pred->next == curr;
}





template <class Pheet, typename TT>
bool LazyListSet<Pheet,TT>::add(TT const& key)
{
    while (true) {
    
        Node *pred = head;
        Node *curr = head->next;

        while (curr != nullptr && curr->key < key) {
            pred = curr;
            curr = curr->next;
        }

        pred->lock.lock();
        if (curr != nullptr) curr->lock.lock();

        if (validate(pred,curr)) {
            if (curr != nullptr && key == curr->key) {
                curr->lock.unlock();
                pred->lock.unlock();
                return false;
            } else {
                Node* node = new Node(key);        
                node->next = curr;
                pred->next = node;        

                item_count++;
        
                if (curr != nullptr) curr->lock.unlock();
                pred->lock.unlock();
                return true;
            }
        }

        if (curr != nullptr) curr->lock.unlock();        
        pred->lock.unlock();
    }
}


template <class Pheet, typename TT>
bool LazyListSet<Pheet, TT>::contains(TT const& key)
{    
    Node* curr = head->next;

    if (curr == nullptr) return false;

    while (curr->key < key && curr->next != nullptr) {
        curr = curr->next;
    }

    return curr->key == key && !(curr->marked);
            
}


template <class Pheet, typename TT>
bool LazyListSet<Pheet, TT>::remove(TT const& key)
{
    while (true) {
    
        Node *pred = head;
        Node *curr = head->next;

        while (curr != nullptr && curr->key < key) {
            pred = curr;
            curr = curr->next;
        }

        pred->lock.lock();
        if (curr != nullptr) curr->lock.lock();

        if (validate(pred,curr)) {
            if (curr != nullptr && key == curr->key) {
                curr->marked = true;
                pred->next = curr->next;

                //delete curr;
                item_count--;
        
                curr->lock.unlock();
                pred->lock.unlock();
                return true;
            } else {
                if (curr != nullptr) curr->lock.unlock();
                pred->lock.unlock();
                return false;
            }
        }
        
        if (curr != nullptr) curr->lock.unlock();
        pred->lock.unlock();
    }
        
}


template <class Pheet, typename TT>
size_t LazyListSet<Pheet, TT>::size()
{
    return item_count;
}


template <class Pheet, typename TT>
void LazyListSet<Pheet, TT>::print_name()
{
    // Should also print some relevant configuration parameters if applicable
    std::cout << "LazyListSet<";
    Mutex::print_name();
    std::cout << ">";
}
