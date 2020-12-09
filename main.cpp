#include <iostream>
#include "allocator.hpp"

using namespace std;

int main() {
    Allocator allocator(2, 256);
    int *a = (int*)allocator.mem_alloc(64);
    *a = 666;
    allocator.mem_alloc(64);
    allocator.mem_alloc(64);
    allocator.mem_alloc(64);
    int *newA = (int*)allocator.mem_realoc(a, 64);
    cout << *a << endl;
    cout << newA << endl;
    return 0;
}
