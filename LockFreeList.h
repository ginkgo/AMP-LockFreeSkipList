#ifndef LOCKFREELIST_HPP
#define LOCKFREELIST_HPP

#include <atomic>
#include "tagged_ptr.h"

template <typename Pheet, typename T>
class LockFreeList
{
    class Node;
    typedef typename boost::lockfree::detail::tagged_ptr<Node> tagged_node_ptr;

    class Node {
        std::atomic<tagged_node_ptr> _next;

    public:
        T val;

        Node(const T& val)
            : val(val) { }

        void nextStore(tagged_node_ptr& ptr) {
            _next.store(ptr);
        }

        tagged_node_ptr next() {
            tagged_node_ptr ptr = _next.load();
            return ptr;
        }

        tagged_node_ptr next(bool& marked) {
            tagged_node_ptr ptr = next();
            marked = (ptr.get_tag() & 0x8000) == 0x8000;
            return ptr;
        }

        bool markNext(tagged_node_ptr& ptr) {
            auto tag = ptr.get_tag();
            ++tag;
            tag = tag | 0x8000;
            tagged_node_ptr newptr(ptr.get_ptr(), tag);
            return _next.compare_exchange_strong(ptr, newptr);
        }

        bool nextCAS(tagged_node_ptr& expected, tagged_node_ptr& ptr) {
            auto tag = ptr.get_tag();
            if((tag & 0x7FFF) == 0x7FFF)
                tag = tag & 0x8000;
            ++tag;
            ptr.set_tag(tag);
            tagged_node_ptr newptr(ptr.get_ptr(), tag);
            return _next.compare_exchange_strong(expected, newptr);
        }

        static void unMark(tagged_node_ptr& ptr) {
            auto tag = ptr.get_tag();
            tag = tag & 0x7FFF;
            ptr.set_tag(tag);
        }
    };

    tagged_node_ptr head;

    void find(tagged_node_ptr& pred, tagged_node_ptr& curr, T val) {
        bool marked;
        tagged_node_ptr succ;

        retry:
        while(true) {
            pred = head;
            curr = pred->next();
            while(true) {
                if(curr.get_ptr() == nullptr)
                    return;

                succ = curr->next(marked);
                while(marked) {
                    Node::unMark(succ);
                    if(!pred->nextCAS(curr, succ))
                        goto retry;
                    curr = succ;
                    if(succ.get_ptr() == nullptr)
                        return;
                    succ = curr->next(marked);
                }
                if(curr->val >= val)
                    return;
                pred = curr;
                curr = succ;
            }
        }
    }

public:
    LockFreeList() {
        head.set_ptr(new Node(T()));
        tagged_node_ptr ptr(nullptr);
        head->nextStore(ptr);
    }

    bool add(const T& val) {
        tagged_node_ptr pred;
        tagged_node_ptr curr;
        std::unique_ptr<Node> ptr(new Node(val));
        tagged_node_ptr node(ptr.get());

        do {
            find(pred, curr, val);
            if(curr.get_ptr() != nullptr && curr->val == val)
                return false;
            node->nextStore(curr);
        } while(!pred->nextCAS(curr, node));

        ptr.release();
        return true;
    }

    bool remove(const T& val) {
        tagged_node_ptr pred;
        tagged_node_ptr curr;
        tagged_node_ptr succ;

        while(true) {
            find(pred, curr, val);
            if(curr.get_ptr() == nullptr || curr->val != val)
                return false;
            succ = curr->next();
            if(!curr->markNext(succ))
                continue;
            pred->nextCAS(curr, succ);
            return true;
        }
    }

    bool contains(const T& val) {
        bool marked;
        tagged_node_ptr curr;
        tagged_node_ptr succ;

        curr = head->next();
        if(curr.get_ptr() == nullptr)
            return false;

        succ = curr->next(marked);
        while(succ.get_ptr() != nullptr && curr->val < val) {
            curr = succ;
            succ = curr->next(marked);
        }

        return (curr->val == val && !marked);
    }

    size_t size() {
        bool marked;
        size_t s = 0;
        tagged_node_ptr curr;
        tagged_node_ptr succ;

        curr = head->next();
        if(curr.get_ptr() == nullptr)
            return 0;

        do {
            succ = curr->next(marked);
            curr = succ;
            if(!marked)
                ++s;
        } while(curr.get_ptr() != nullptr);
        return s;
    }

    static void print_name() {
        std::cout << "LockFreeList";
    }
};

#endif //LOCKFREELIST_HPP
