#pragma once

#include <list>

template <class I>
class ItemPool
{

    std::vector<I*> unused;
    std::vector<I*> retired;
    
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
        unused.pop_back(item);
    }
    
    void retire(I* item)
    {
        retired.pop_back(item);
    }
        
    I* acquire(int height)
    {
        I* item = nullptr;
        
        if (!unused.empty()) {
            I* item = unused.back();
            unused.pop_back();
        } else {
            item = new I(height);
        }
        
        return item;
    }
    
};
