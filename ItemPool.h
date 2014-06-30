#pragma once

#include <list>
#include <deque>

template <class I>
class ItemPool
{

    std::deque<I*> unused;
    std::deque<I*> retired;
    
public:

    ItemPool()
    {
    };
    
    ~ItemPool()
    {
        for (I* item : unused) {
            delete item;
        }

        for (I* item : retired) {
            delete item;
        }
    }


    void release(I* item)
    {
        unused.push_front(item);
    }
    
    void retire(I* item)
    {
        retired.push_back(item);
    }
        
    I* acquire()
    {
        I* item = nullptr;
        
        if (!unused.empty()) {
            item = unused.back();
            unused.pop_back();
        } else {
            item = new I();
        }
        
        return item;
    }
        
    I* acquire(int height)
    {
        I* item = nullptr;
        
        if (!unused.empty()) {
            item = unused.back();
            unused.pop_back();
        } else {
            item = new I(height);
        }
        
        return item;
    }
    
};
