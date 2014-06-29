#pragma once

#include <atomic>
#include <assert.h>
#include "tagged_ptr.h"

template <typename T>
class stamped_ptr
{
    
    typedef typename boost::lockfree::detail::tagged_ptr<T> tagged_ptr;
    
    std::atomic<tagged_ptr> mp;

    
public:

    stamped_ptr() : mp(pack(nullptr, false, 0)) {};
    
    bool compare_and_set(T* expected_ref,
                         bool expected_mark,
                         uint16_t expected_stamp,
                         T* new_ref,
                         bool new_mark,
                         uint16_t new_stamp)
    {
        tagged_ptr expected_v = pack(expected_ref, expected_mark, expected_stamp);
        tagged_ptr new_v = pack(new_ref, new_mark, new_stamp);

        return mp.compare_exchange_strong(expected_v, new_v);
    }

    void set(T* ptr, bool mark, uint16_t stamp)
    {
        tagged_ptr v = pack(ptr, mark, stamp);

        mp.store(v);
    }
    
    void get(T*& ptr, bool& mark, uint16_t& stamp) const
    {
        tagged_ptr v = mp.load();

        mark = mark_unpack(v);
        ptr = (T*)ptr_unpack(v);
        stamp = stamp_unpack(v);
    }

    T* get_ref() const
    {
        tagged_ptr v = mp.load();

        return (T*)ptr_unpack(v);
    }

    bool get_mark() const
    {
        tagged_ptr v = mp.load();

        return mark_unpack(v);
    }

    bool get_stamp() const
    {
        tagged_ptr v = mp.load();

        return stamp_unpack(v);
    }

    static const uint16_t MAX_STAMP = 0x7fff;
    
private:

    static const uint16_t MARK_MASK = 0x8000;
    static const uint16_t STAMP_MASK = 0x7fff;

    static tagged_ptr pack(void const volatile* p, bool mark, uint16_t stamp)
    {
        assert(stamp <= MAX_STAMP);

        return tagged_ptr(p, stamp | (mark ? MARK_MASK : 0));
    }

    static void* ptr_unpack(tagged_ptr tp)
    {
        return tp.get_ptr();
    }

    static bool mark_unpack(tagged_ptr tp)
    {
        uint16_t tag = tp.get_tag();
        
        return (tag & MARK_MASK) != 0;
    }

    static uint16_t stamp_unpack(tagged_ptr tp)
    {
        uint16_t tag = tp.get_tag();
        
        return (tag & STAMP_MASK);
    }    
    
};
