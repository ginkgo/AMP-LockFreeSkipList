#pragma once

#include <set>
#include <mutex>

template <class Pheet, typename TT>
class CoarseListSet {

public:
    
    typedef typename Pheet::Mutex Mutex;
    typedef typename Pheet::LockGuard LockGuard;

    struct Node {
        volatile TT key;
        volatile Node* next;        
    };

    
private:
    
    Mutex lock;

    volatile Node *head;
    size_t item_count = 0;

    CoarseListSet(CoarseListSet& other); // private copy constructor
    
public:
    
    CoarseListSet();
    ~CoarseListSet() { };

    bool add(TT const& item);
    bool contains(TT const& item);
    bool remove(TT const& item);
    
    size_t size(); // Does not have to be exact, but needs to be > 0 if not empty

    static void print_name();
    
};


/// Template method implementations start here


template <class Pheet, typename TT>
CoarseListSet<Pheet,TT>::CoarseListSet()
{
    
    head = new Node();
    head->next = nullptr;
}


template <class Pheet, typename TT>
bool CoarseListSet<Pheet,TT>::add(TT const& key)
{    
    LockGuard lg(lock);

    volatile Node *pred = head;
    volatile Node *curr = head->next;

    while (curr != nullptr && curr->key < key) {
        pred = curr;
        curr = curr->next;
    }

    if (curr != nullptr && key == curr->key) {
        return false;
    } else {
        Node* node = new Node();

        node->key = key;
        
        node->next = curr;
        pred->next = node;        

        item_count++;
        
        return true;
    }
}


template <class Pheet, typename TT>
bool CoarseListSet<Pheet, TT>::contains(TT const& key)
{    
    LockGuard lg(lock);

    volatile Node *curr = head->next;

    while (curr != nullptr && curr->key < key) {
        curr = curr->next;
    }

    if (curr != nullptr && key == curr->key) {
        return true;
    } else {
        return false;
    }
}


template <class Pheet, typename TT>
bool CoarseListSet<Pheet, TT>::remove(TT const& key)
{
    LockGuard lg(lock);

    volatile Node *pred = head;
    volatile Node *curr = head->next;

    while (curr != nullptr && curr->key < key) {
        pred = curr;
        curr = curr->next;
    }

    if (curr != nullptr && key == curr->key) {
        pred->next = curr->next;

        delete curr;
        item_count--;
        
        return true;
    } else {
        // if (curr == nullptr) {
        //     cout << "curr == null ";
        // } else {
        //     cout << "key =  "<< curr->key << " ";
        // }
        return false;
    }
}


template <class Pheet, typename TT>
size_t CoarseListSet<Pheet, TT>::size()
{
    return item_count;
}


template <class Pheet, typename TT>
void CoarseListSet<Pheet, TT>::print_name()
{
    // Should also print some relevant configuration parameters if applicable
    std::cout << "CoarseListSet<";
    Mutex::print_name();
    std::cout << ">";
}
