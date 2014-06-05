#include <iostream>
#include <memory>
#include <random>

using std::cerr;
using std::cout;
using std::endl;

#include <pheet/pheet.h>
typedef pheet::Pheet Pheet;

#define USED_SET 1

#if USED_SET == 1
#include "SequentialSet.h"
typedef SequentialSet<Pheet, int> Set;
#elif USED_SET == 2
#include "CoarseListSet.h"
typedef CoarseListSet<Pheet, int> Set;
#elif USED_SET == 3
#include "FineListSet.h"
typedef FineListSet<Pheet, int> Set;
#elif USED_SET == 4
#include "OptimisticListSet.h"
typedef OptimisticListSet<Pheet, int> Set;
#elif USED_SET == 5
#include "LazyListSet.h"
typedef LazyListSet<Pheet, int> Set;
#endif

void test_set(Set* set, int count, int id, int total)
{
    int *vals = new int[count];

    std::default_random_engine generator;
    std::uniform_int_distribution<int> distribution(0,1<<5);

    for (int j = 0; j < 1; ++j) {
        for (int i = 0; i < count; ++i) {
            vals[i] = distribution(generator) * count * total +  i * total + id; 
            if (!set->add(vals[i])) {
                cout << id << " Adding " << vals[i] << " failed." << endl;
            }
        }

        for (int i = 0; i < count; ++i) {
            if (!set->contains(vals[i])) {
                cout << id << " Set should contain " << vals[i] << ", but doesn't." << endl;
            }
        }
        
        for (int i = 0; i < count; ++i) {
            if (!set->remove(vals[i])) {
                cout << id << " Removing " << vals[i] << " failed." << endl;
            }
        }
        
        for (int i = 0; i < count; ++i) {
            if (set->contains(vals[i])) {
                cout << id << " Set shouldn't contain " << vals[i] << ", but does." << endl;
            }
        }
    }

    cout << "done with " << id << "/" << total << endl;
    delete[] vals;
}

int main ()
{
    
    Set set;

    cout << "Testing ";
    Set::print_name();
    cout << ":" << endl;
    
    {Pheet::Environment p;

        const int N = 12;
        
        for (int i = 0; i < N; ++i) {
            Pheet::spawn(test_set, &set, 10000, i, N);
        }
    }
}
