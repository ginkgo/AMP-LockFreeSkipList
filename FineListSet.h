#pragma once

#include <set>
#include <mutex>

template <class Pheet, typename TT>
class FineListSet {

public:
    
    typedef typename Pheet::Mutex Mutex;
    typedef typename Pheet::LockGuard LockGuard;

    struct Node {
        TT key;
        Node* next;
        Mutex lock;
    };

    
private:
    
    Node *head;
    size_t item_count = 0;

    FineListSet(FineListSet& other); // private copy constructor
    
public:
    
    FineListSet();
    ~FineListSet() { };

    bool add(TT const& item);
    bool contains(TT const& item);
    bool remove(TT const& item);
    
    size_t size(); // Does not have to be exact, but needs to be > 0 if not empty

    static void print_name();
    
};


/// Template method implementations start here


template <class Pheet, typename TT>
FineListSet<Pheet,TT>::FineListSet()
{
    
    head = new Node();
    head->next = nullptr;
};


template <class Pheet, typename TT>
bool FineListSet<Pheet,TT>::add(TT const& key)
{    
    head->lock.lock();
    
    Node *pred = head;
    Node *curr = head->next;

    if (curr != nullptr) curr->lock.lock();

    while (curr != nullptr && curr->key < key) {
        pred->lock.unlock();
        pred = curr;
        curr = curr->next;
        if (curr != nullptr) curr->lock.lock();
    }

    if (curr != nullptr && key == curr->key) {
        curr->lock.unlock();
        pred->lock.unlock();
        return false;
    } else {
        Node* node = new Node();

        node->key = key;
        
        node->next = curr;
        pred->next = node;        

        item_count++;
        
        if (curr != nullptr) curr->lock.unlock();
        pred->lock.unlock();
        return true;
    }
}


template <class Pheet, typename TT>
bool FineListSet<Pheet, TT>::contains(TT const& key)
{    
    head->lock.lock();
    
    Node *pred = head;
    Node *curr = head->next;

    if (curr != nullptr) curr->lock.lock();

    while (curr != nullptr && curr->key < key) {
        pred->lock.unlock();
        pred = curr;
        curr = curr->next;
        if (curr != nullptr) curr->lock.lock();
    }

    if (curr != nullptr && key == curr->key) {
        curr->lock.unlock();
        pred->lock.unlock();
        return true;
    } else {
        if (curr != nullptr) curr->lock.unlock();
        pred->lock.unlock();
        return false;
    }
}


template <class Pheet, typename TT>
bool FineListSet<Pheet, TT>::remove(TT const& key)
{
    head->lock.lock();
    
    Node *pred = head;
    Node *curr = head->next;

    if (curr != nullptr) curr->lock.lock();

    while (curr != nullptr && curr->key < key) {
        pred->lock.unlock();
        pred = curr;
        curr = curr->next;
        if (curr != nullptr) curr->lock.lock();
    }

    if (curr != nullptr && key == curr->key) {
        pred->next = curr->next;

        delete curr;
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


template <class Pheet, typename TT>
size_t FineListSet<Pheet, TT>::size()
{
    return item_count;
}


template <class Pheet, typename TT>
void FineListSet<Pheet, TT>::print_name()
{
    // Should also print some relevant configuration parameters if applicable
    std::cout << "FineListSet<";
    Mutex::print_name();
    std::cout << ">";
}
