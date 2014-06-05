#pragma once

#include <set>
#include <mutex>


template <class Pheet, typename TT>
class SequentialSet {

    typedef typename Pheet::Mutex Mutex;
    typedef typename Pheet::LockGuard LockGuard;
    
    std::set<TT> internal_set;
    Mutex lock;
    
public:
    SequentialSet() {};
    ~SequentialSet() {};

    bool add(TT const& item);
    bool contains(TT const& item);
    bool remove(TT const& item);
    
    size_t size(); // Does not have to be exact, but needs to be > 0 if not empty

    static void print_name();
};


/// Template method implementations start here

template <class Pheet, typename TT>
bool SequentialSet<Pheet,TT>::add(TT const& item)
{
    LockGuard lg(lock);

    return internal_set.insert(item).second;
}


template <class Pheet, typename TT>
bool SequentialSet<Pheet, TT>::contains(TT const& item)
{    
    LockGuard lg(lock);
    
    return internal_set.count(item) > 0;
}


template <class Pheet, typename TT>
bool SequentialSet<Pheet, TT>::remove(TT const& item)
{
    LockGuard lg(lock);
    
    return internal_set.erase(item) > 0;
}


template <class Pheet, typename TT>
size_t SequentialSet<Pheet, TT>::size()
{
    LockGuard lg(lock);
    
    return internal_set.size();
}


template <class Pheet, typename TT>
void SequentialSet<Pheet, TT>::print_name()
{
    // Should also print some relevant configuration parameters if applicable
    std::cout << "SequentialSet<";
    Mutex::print_name();
    std::cout << ">";
}
