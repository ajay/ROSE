#ifndef __TK_HEAP_H__
#define __TK_HEAP_H__

#include <vector>
#include <stdexcept>

template <class T>
class Heap {
  public:
    Heap(void);
    ~Heap(void);
    void siftup(void);
    void siftdown(void);
    void push(T const &item, int priority);
    T pop(void);
    bool empty(void) const;
    size_t size(void) const;

  private:
    void swap(int const &a, int const &b);
    std::vector<T> queue;
    std::vector<int> priorities;
    int parent(int index) const;
    int lchild(int index) const;
    int rchild(int index) const;
};

// for now, also include heap.cpp
// will fix later

#endif
