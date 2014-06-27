#ifndef LOCKFREELIST_HPP
#define LOCKFREELIST_HPP

#include <atomic>
#include "tagged_ptr.h"

static thread_local void* emptyStack = nullptr;

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
            : val(val)
        {
            tagged_node_ptr temp(nullptr);
            _next.store(temp);
        }

        void nextStore(Node* next_ptr, boost::uint16_t act_tag) {
            tagged_node_ptr ptr(next_ptr, act_tag);
            _next.store(ptr);
        }

        tagged_node_ptr next() {
            tagged_node_ptr ptr = _next.load();
            return ptr;
        }

        tagged_node_ptr next(boost::uint16_t& act_tag) {
            tagged_node_ptr ptr = next();
            act_tag = ptr.get_tag();
            return ptr;
        }

        bool mark(tagged_node_ptr& succ) {
            auto tag = succ.get_tag();
            ++tag;
            tag = tag | 0x8000;
            tagged_node_ptr newptr(succ.get_ptr(), tag);
            return _next.compare_exchange_strong(succ, newptr);
        }

        bool nextCAS(tagged_node_ptr& expected, Node* next_ptr, boost::uint16_t act_tag) {
            tagged_node_ptr newptr(next_ptr, act_tag);
            return _next.compare_exchange_strong(expected, newptr);
        }

        static bool isMarked(boost::uint16_t tag) {
            return (tag & 0x8000) == 0x8000;
        }

        static boost::uint16_t unMark(boost::uint16_t tag) {
            return (tag & 0x7FFF);
        }
    };

    tagged_node_ptr head;

    bool find(tagged_node_ptr& pred, tagged_node_ptr& curr, tagged_node_ptr& succ, T& curr_val, const T& val) {
        boost::uint16_t pred_tag;
        boost::uint16_t curr_tag;

        retry:
        while(true) {
            pred = head;
            curr = pred->next(pred_tag); //init pred_tag, should be always 0
            while(true) {
                if(curr.get_ptr() == nullptr)
                    return false;

                succ = curr->next(curr_tag);
                while(Node::isMarked(curr_tag)) {
                    if(!pred->nextCAS(curr, succ.get_ptr(), pred_tag))
                        goto retry;

                    //curr is removed phisically from the list
                    //pred.next is compared to curr, so curr must be in the same state since we acquired it

                    if(curr_tag == 0xFFFF) {
                        //retiredNode
                    } else {
                        curr->nextStore((Node*)emptyStack, Node::unMark(curr_tag));
                        emptyStack = curr.get_ptr();
                    }

                    curr = succ;
                    if(succ.get_ptr() == nullptr)
                        return false;
                    succ = curr->next(curr_tag);
                }

                curr_val = curr->val;
                if(curr->next().get_tag() != curr_tag)
                    goto retry;

                //atomic curr tag is compared to curr_tag, so curr must be in the same state since we acquired it
                //TODO: compare only timestamp, is this correct?
                //NOTE: using milisec precise timestamp is only enough for ~32 secs, using increment on mark

                if(curr_val >= val)
                    return true;
                pred = curr;
                curr = succ;
                pred_tag = curr_tag;
            }
        }
    }

public:
    LockFreeList() {
        head.set_ptr(new Node(T()));
        head->nextStore(nullptr, 0);
    }

    bool add(const T& val) {
        T curr_val;
        tagged_node_ptr pred;
        tagged_node_ptr curr;
        tagged_node_ptr succ;
        tagged_node_ptr node;
        boost::uint16_t node_tag;
        std::unique_ptr<Node> ptr;

        if(emptyStack != nullptr) { //reuse deleted node
            node = tagged_node_ptr((Node*)emptyStack);
            tagged_node_ptr next = node->next(node_tag);
            emptyStack = next.get_ptr();
            node->val = val;
        } else { //create new node
            ptr.reset(new Node(val));
            node.set_ptr(ptr.get());
        }

        do {
            find(pred, curr, succ, curr_val, val);
            if(curr.get_ptr() != nullptr && curr_val == val)
                return false;
            node->nextStore(curr.get_ptr(), node_tag);
        } while(!pred->nextCAS(curr, node.get_ptr(), curr.get_tag())); //pred_tag

        ptr.release();
        return true;
    }

    bool remove(const T& val) {
        T curr_val;
        tagged_node_ptr pred;
        tagged_node_ptr curr;
        tagged_node_ptr succ;

        while(true) {
            find(pred, curr, succ, curr_val, val);
            if(curr.get_ptr() == nullptr || curr_val != val)
                return false;
            if(!curr->mark(succ))
                continue;
            //pred->nextCAS(curr, succ);
            return true;
        }
    }

    bool contains(const T& val) {
#if 1
        T curr_val;
        tagged_node_ptr curr;
        tagged_node_ptr succ;
        boost::uint16_t curr_tag;

        curr = head->next();
        if(curr.get_ptr() == nullptr)
            return false;

        retry:
        do {
            succ = curr->next(curr_tag);
            curr_val = curr->val;
            if(curr->next().get_tag() != curr_tag)
                goto retry;
            curr = succ;
        } while(succ.get_ptr() != nullptr && curr_val < val);

        return (curr_val == val && !Node::isMarked(curr_tag));
#elif 0
        //use find, solve ABA
        T curr_val;
        tagged_node_ptr pred;
        tagged_node_ptr curr;
        tagged_node_ptr succ;

        if(find(pred, curr, succ, curr_val, val))
            return curr_val == val;
        else
            return false;
#elif 0
        //original from book, no ABA
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
#endif
    }

    size_t size() {
        size_t s = 0;
        tagged_node_ptr curr;
        tagged_node_ptr succ;
        boost::uint16_t curr_tag;

        curr = head->next();
        if(curr.get_ptr() == nullptr)
            return 0;

        do {
            succ = curr->next(curr_tag);
            curr = succ;
            if(!Node::isMarked(curr_tag))
                ++s;
        } while(curr.get_ptr() != nullptr);
        return s;
    }

    static void print_name() {
        std::cout << "LockFreeList";
    }
};

#endif //LOCKFREELIST_HPP
