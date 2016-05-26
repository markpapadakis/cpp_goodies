#pragma once
#include "switch.h"

// Based on std::deque<> http://en.cppreference.com/w/cpp/container/deque
// except that capacity can be adjusted dynamically via resetTo(), and will overwrite values if necessary (i.e if 
// more values than the specified capcity are enqueued); that is, it's a bounded FIFO queue that holds upto specified-capacity values.
namespace Switch
{
        template <typename T>
        class deque
        {
                using reference = T &;
                using reference_const = const T &;

                struct iterator
                {
                        const deque &container;
                        uint32_t idx;

                        iterator &operator++()
                        {
                                idx = container.next(idx);
                                return *this;
                        }

                        inline reference operator*()
                        {
                                return container.A[idx];
                        }

                        auto operator!=(const iterator &o) const
                        {
                                return idx != o.idx;
                        }

			auto operator+(const int32_t n)
			{
				return iterator{container, (idx + n) % container.capacity};
			}

			auto operator-(const int32_t n)
			{
				return iterator{container, (idx + (container.capacity - 1)) % container.capacity};
			}
                };

                uint32_t maxCapacity;

                uint32_t capacity;
                T *A;
                alignas(64) uint32_t backIdx;
                alignas(64) uint32_t frontIdx;
                uint32_t cnt;

                inline uint32_t prev(const uint32_t idx) const
                {
                        return (idx + (capacity - 1)) % capacity;
                }

                inline uint32_t next(const uint32_t idx) const
                {
                        return (idx + 1) % capacity;
                }

		void copyImpl(const deque &o)
		{
                        clear();

                        capacity = o.capacity;
                        if (maxCapacity != o.maxCapacity)
                        {
                                maxCapacity = o.maxCapacity;
                                A = new T[maxCapacity];
                        }
                        frontIdx = o.frontIdx;
                        backIdx = o.backIdx;
                        cnt = o.cnt;

                        const auto upto = frontIdx + cnt;

                        if (upto > capacity)
                        {
                                for (uint32_t i = frontIdx; i != capacity; ++i)
                                        A[i] = o.A[i];

                                const auto n = upto - capacity;

                                for (uint32_t i{0}; i != n; ++i)
                                        A[i] = o.A[i];
                        }
                        else
                        {
                                for (uint32_t i = frontIdx; i != upto; ++i)
                                        A[i] = o.A[i];
                        }
		}

		void moveImpl(deque &&o)
                {
                        clear();
                        delete[] A;

                        maxCapacity = o.maxCapacity;
                        capacity = o.capacity;
                        A = o.A;
                        frontIdx = o.frontIdx;
                        backIdx = o.backIdx;
                        cnt = o.cnt;

                        o.A = nullptr;
                        o.cnt = 0;
                        o.frontIdx = o.backIdx = 0;
                }

              public:
                inline iterator begin() const
                {
                        return {*this, frontIdx};
                }

                inline iterator end() const
                {
                        return {*this, backIdx};
                }

		deque(const std::initializer_list<T> &&in)
			: maxCapacity{0}, cnt{0}, frontIdx{0}, backIdx{0}, A{nullptr}
		{
			resetTo(in.size());
			for (auto &&v : in)
				push_back(v);
		}


                deque(const uint32_t max)
                    : maxCapacity{max}, capacity{maxCapacity}
                {
                        if (maxCapacity)
                                A = new T[maxCapacity];
                        else
                                A = nullptr;

			backIdx = 0;
			frontIdx = 0;
			cnt = 0;
                }

                deque(deque &&o)
			: cnt{0}, frontIdx{0}, backIdx{0}, A{nullptr}
                {
			moveImpl(std::move(o));
                }

                deque(const deque &o)
			: maxCapacity{0}, cnt{0}, frontIdx{0}, backIdx{0}, A{nullptr}
                {
			copyImpl(o);
                }

                auto &operator=(deque &&o)
                {
			moveImpl(std::move(o));
                        return *this;
                }
	
                auto &operator=(const deque &o)
                {
			copyImpl(o);
                        return *this;
                }

                ~deque()
                {
                        clear();
                        delete[] A;
                }

                void clear()
                {
                        if (std::is_destructible<T>::value && !std::is_trivially_destructible<T>::value)
                        {
                                while (cnt)
                                {
                                        --cnt;
                                        A[frontIdx].~T();
                                        new (&A[frontIdx]) T();
                                        frontIdx = next(frontIdx);
                                }
                        }
                        else
                                cnt = 0;

                        frontIdx = backIdx = 0;
                        capacity = maxCapacity;
                }

		inline bool operator==(const deque &o) const
		{
			if (cnt != o.cnt)
				return false;

			const auto otherI = o.frontIdx;
			const auto otherA = o.A;
			
			for (uint32_t i{0}; i != cnt; ++i)
			{
				if (A[(frontIdx + i) % capacity] != otherA[(otherI + i) % capacity])
					return false;
			}

			return true;
		}

                void resetTo(const uint32_t newCapacity)
                {
			if (newCapacity > maxCapacity)
			{
				delete[] A;
				maxCapacity = newCapacity;
				A = new T[maxCapacity];
			}
			else
                        	clear();

                        capacity = newCapacity;
                }

                bool empty() const
                {
                        return !cnt;
                }

                auto max_size() const
                {
                        return capacity;
                }

                inline auto size() const
                {
                        return cnt;
                }

                inline reference front()
                {
                        return A[frontIdx];
                }

                inline reference_const front() const
                {
                        return A[frontIdx];
                }

                inline reference back()
                {
                        return A[prev(backIdx)];
                }

                inline reference_const back() const
                {
                        return A[prev(backIdx)];
                }

		reference at(const size_t idx)
		{
			if (unlikely(idx > cnt))
				throw std::out_of_range("Invalid index");
			else
				return A[(frontIdx + idx) % capacity];
		}

		reference_const at(const size_t idx) const
		{
			if (unlikely(idx > cnt))
				throw std::out_of_range("Invalid index");
			else
				return A[(frontIdx + idx) % capacity];
		}

                reference operator[](const size_t idx)
                {
                        return A[(frontIdx + idx) % capacity];
                }

                reference_const operator[](const size_t idx) const
                {
                        return A[(frontIdx + idx) % capacity];
                }

                void push_back(const T &v)
                {
                        A[backIdx] = v;
                        backIdx = next(backIdx);
                        if (++cnt > capacity)
                                cnt = capacity;
                }

                void push_back(T &&v)
                {
                        A[backIdx] = std::move(v);
                        backIdx = next(backIdx);
                        if (++cnt > capacity)
                                cnt = capacity;
                }

                void push_front(T &&v)
                {
                        frontIdx = prev(frontIdx);
                        A[frontIdx] = std::move(v);
                        if (++cnt > capacity)
                                cnt = capacity;
                }

                void push_front(const T &v)
                {
                        frontIdx = prev(frontIdx);
                        A[frontIdx] = v;
                        if (++cnt > capacity)
                                cnt = capacity;
                }

                void pop_back()
                {
                        backIdx = prev(backIdx);
                        --cnt;
                }

                void pop_front()
                {
                        frontIdx = next(frontIdx);
                        --cnt;
                }
        };
};
