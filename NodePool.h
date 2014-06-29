#pragma once

template <class Pheet, class N>
class NodePool
{

    N* head;
    N* retired;
    
public:

    NodePool();
    ~NodePool();


    void release(N* node);
    void retire(N* node);

    N* acquire(int height);
    
};
