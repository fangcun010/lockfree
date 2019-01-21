/*
MIT License

Copyright(c) 2019 fangcun

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


			Compiled under the mingw64
********You need to implement your own memory manager********
********Or you will get memory leak					 ********
*/
#ifndef LOCK_FREE_QUEUE_HPP_INCLUDED
#define LOCK_FREE_QUEUE_HPP_INCLUDED

#include <functional>

namespace lock_free {
	template<typename T>
		class LockFreeQueue {
		public:
			struct Node;
			struct Ptr {
				Node *ptr;
				unsigned long long tag;
			}  __attribute__((aligned(16)));
			bool PtrEqual(const Ptr &p1, const Ptr &p2) {
				return p1.ptr == p2.ptr && p1.tag == p2.tag;
			}
			struct Node {
				T value;
				Ptr next;
			};
		private:
			Ptr head_;
			Ptr tail_;
			std::function<Node *()> alloc_;
			std::function<void(Node *)> free_;
		public:
			LockFreeQueue() {
				alloc_ = nullptr;
				free_ = nullptr;
			}
			~LockFreeQueue() {
				if (free_ == nullptr) return;
				if (head_.ptr != nullptr)
					free_(head_.ptr);
			}
			void SetAllocAndFree(std::function<Node *()> alloc,
				std::function<void(Node *)> free) {
				alloc_ = alloc;
				free_ = free;
			}
			void Init() {
				if (alloc_ == nullptr)
					alloc_ = []()->Node * { return new Node() ; };
				if (free_ == nullptr)
					free_ = [](Node * node) { delete node; };
				Node * node = alloc_();
				node->next.ptr = nullptr;
				node->next.tag = 0;
				tail_.tag = head_.tag = 0;
				tail_.ptr = head_.ptr = node;
			}
			void Push(const T &value) {
				Ptr new_ptr;
				Node * node = alloc_();
				node->value = value;
				node->next.ptr = nullptr;
				for (;;) {
					Ptr tail = tail_;
					Ptr next = tail.ptr->next;
					if (PtrEqual(tail, tail_)) {
						if (next.ptr == nullptr) {
							new_ptr = { node, next.tag + 1 };
							if (__sync_bool_compare_and_swap(reinterpret_cast<__uint128_t *>(&tail.ptr->next), 
								*reinterpret_cast<__uint128_t *>(&next),
								*reinterpret_cast<__uint128_t *>(&new_ptr))) {
								new_ptr = { node, tail.tag + 1 };
									__sync_bool_compare_and_swap(reinterpret_cast<__uint128_t *>(&tail_),
										*reinterpret_cast<__uint128_t *>(&tail),
										*reinterpret_cast<__uint128_t *>(&new_ptr));
								return;
							}
						}
						else {
							new_ptr = { next.ptr, tail.tag + 1 };
							__sync_bool_compare_and_swap(reinterpret_cast<__uint128_t *>(&tail_),
								*reinterpret_cast<__uint128_t *>(&tail),
								*reinterpret_cast<__uint128_t *>(&new_ptr));
						}
					}
				}
			}
			bool Pop(T &var) {
				Ptr new_ptr;
				for (;;) {
					Ptr head = head_;
					Ptr tail = tail_;
					Ptr next = head.ptr->next;
					if (PtrEqual(head, head_)) {
						if (head.ptr == tail.ptr) {
							if (next.ptr == nullptr)
								return false;
							new_ptr = { next.ptr, tail.tag + 1 };
							__sync_bool_compare_and_swap(reinterpret_cast<__uint128_t *>(&tail_), 
								*reinterpret_cast<__uint128_t *>(&tail),
								*reinterpret_cast<__uint128_t *>(&new_ptr));
						}
						else {
							var = next.ptr->value;
							new_ptr = { next.ptr, tail.tag + 1 };
							if (__sync_bool_compare_and_swap(reinterpret_cast<__uint128_t *>(&head_),
								*reinterpret_cast<__uint128_t *>(&head),
								*reinterpret_cast<__uint128_t *>(&new_ptr))) {
								free_(head.ptr);
								return true;
							}
						}
					}
				}
			}
			bool Empty() {
				//non-thread-safe
				return PtrEqual(head_, tail_);
			}
		};

}

#endif //LOCK_FREE_QUEUE_HPP_INCLUDED