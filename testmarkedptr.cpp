#include "marked_ptr.h"
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;

struct A {};
struct B : public A {char a;};
struct C : public A {short a;};
struct D : public A {int i;};
struct E : public A {int i; int j;};
struct F : public A {int i[20];};

int main ()
{
    for (int i = 0; i < 1000; ++i) {
        A* a = new A();
        B* b = new B();
        C* c = new C();
        D* d = new D();
        E* e = new E();
        F* f = new F();

        // cout << a << " " << sizeof(*a) << endl;
        // cout << b << " " << sizeof(*b) <<  endl;
        // cout << c << " " << sizeof(*c) <<  endl;
        // cout << d << " " << sizeof(*d) <<  endl;
        // cout << e << " " << sizeof(*e) <<  endl;
        // cout << f << " " << sizeof(*f) <<  endl;
    
        marked_ptr<A> p;

        if (!p.compare_and_set(nullptr, a, false, false)) {
            cout << "fail" << endl;
        }

        A* ptr;
        bool mark;

        p.get(ptr, mark);
        assert(ptr == a);
        assert(mark == false);
    
        if (p.compare_and_set(nullptr, a, false, false)) {
            cout << "fail" << endl;
        }

        p.get(ptr, mark);
        assert(ptr == a);
        assert(mark == false);
    
        if (!p.compare_and_set(a, b, false, false)) {
            cout << "fail" << endl;
        }

        p.get(ptr, mark);
        assert(ptr == b);
        assert(mark == false);
    
        if (!p.compare_and_set(b, b, false, true)) {
            cout << "fail" << endl;
        }

        p.get(ptr, mark);
        assert(ptr == b);
        assert(mark == true);
    
        if (!p.compare_and_set(b, c, true, false)) {
            cout << "fail" << endl;
        }

        p.get(ptr, mark);
        assert(ptr == c);
        assert(mark == false);
    }
    // delete a;
    // delete b;
    // delete c;
    // delete d;
    // delete e;
    // delete f;
    
    return 0;
}
