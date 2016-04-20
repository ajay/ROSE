#ifndef HEAP_H
#define HEAP_H

#include <stdexcept>
#include <vector>

// for now, also include heap.cpp
// will fix later

template <class T>
class Heap
{
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
#endif