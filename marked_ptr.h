#pragma once

#include <atomic>
#include <assert.h>

template <typename T>
class marked_ptr
{
    
    static const uintptr_t mask = ((1ull << 1) - 1);
    
    std::atomic<uintptr_t> mp;

    
public:

    marked_ptr() : mp(pack(nullptr, false)) {};
    
    bool compare_and_set(T* expected_ref,
                         T* new_ref,
                         bool expected_mark,
                         bool new_mark)
    {
        uintptr_t expected_v = pack(expected_ref, expected_mark);
        uintptr_t new_v = pack(new_ref, new_mark);

        return mp.compare_exchange_strong(expected_v, new_v);
    }

    void get(T*& ptr, bool& mark) const
    {
        uintptr_t v = mp.load();

        mark = mark_unpack(v);
        ptr = (T*)ptr_unpack(v);
    }

    T* get_ref() const
    {
        uintptr_t v = mp.load();

        return (T*)ptr_unpack(v);
    }

    
private:


    static uintptr_t pack(void const volatile* p, bool mark)
    {
        uintptr_t v = (uintptr_t)p;
        assert((v & mask) == 0);

        return v | (mark ? 1 : 0);
    }

    static void* ptr_unpack(uintptr_t mp)
    {
        return (void *) (mp & ~mask);
    }

    static bool mark_unpack(uintptr_t mp)
    {
        return mp & mask != 0;
    }
    
    
};
