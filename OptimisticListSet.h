#pragma once

#include <set>
#include <mutex>

template <class Pheet, typename TT>
class OptimisticListSet {

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

    OptimisticListSet(OptimisticListSet& other); // private copy constructor

    bool validate (Node* pred, Node* curr);
    
public:
    
    OptimisticListSet();
    ~OptimisticListSet() { };

    bool add(TT const& item);
    bool remove(TT const& item);
    bool contains(TT const& item);
    
    size_t size(); // Does not have to be exact, but needs to be > 0 if not empty

    static void print_name();
    
};


/// Template method implementations start here


template <class Pheet, typename TT>
OptimisticListSet<Pheet,TT>::OptimisticListSet()
{
    
    head = new Node();
    head->next = nullptr;
};




template <class Pheet, typename TT>
bool OptimisticListSet<Pheet, TT>::validate(Node* pred, Node* curr)
{
    Node* node = head;

    while (node != nullptr && node->key <= pred->key) {
        if (node == pred) {
            return pred->next == curr;
        }

        node = node->next;
    }

    return false;
}





template <class Pheet, typename TT>
bool OptimisticListSet<Pheet,TT>::add(TT const& key)
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
                pred->lock.unlock();
                curr->lock.unlock();
                return false;
            } else {
                Node* node = new Node();

                node->key = key;
        
                node->next = curr;
                pred->next = node;        

                item_count++;
        
                pred->lock.unlock();
                if (curr != nullptr) curr->lock.unlock();
                return true;
            }
        }

        
        pred->lock.unlock();
        if (curr != nullptr) curr->lock.unlock();
    }
}


template <class Pheet, typename TT>
bool OptimisticListSet<Pheet, TT>::contains(TT const& key)
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
                pred->lock.unlock();
                curr->lock.unlock();
                return true;
            } else {
                pred->lock.unlock();
                if (curr != nullptr) curr->lock.unlock();
                return false;
            }
        }

        
        pred->lock.unlock();
        if (curr != nullptr) curr->lock.unlock();
    }
}


template <class Pheet, typename TT>
bool OptimisticListSet<Pheet, TT>::remove(TT const& key)
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
                pred->next = curr->next;

                //delete curr;
                item_count--;
        
                pred->lock.unlock();
                curr->lock.unlock();
                return true;
            } else {
                pred->lock.unlock();
                if (curr != nullptr) curr->lock.unlock();
                return false;
            }
        }

        
        pred->lock.unlock();
        if (curr != nullptr) curr->lock.unlock();
    }
        
}


template <class Pheet, typename TT>
size_t OptimisticListSet<Pheet, TT>::size()
{
    return item_count;
}


template <class Pheet, typename TT>
void OptimisticListSet<Pheet, TT>::print_name()
{
    // Should also print some relevant configuration parameters if applicable
    std::cout << "OptimisticListSet<";
    Mutex::print_name();
    std::cout << ">";
}
