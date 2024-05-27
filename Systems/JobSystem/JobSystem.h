#ifndef _AT_JOB_SYSTEM_H_
#define _AT_JOB_SYSTEM_H_

#include <functional>
#include <mutex>
#include <atomic>
namespace AT::JobSystem {
    typedef std::atomic<uint64_t> JobCounter;
    struct Job {
        JobCounter* counter = nullptr;
        std::function<void(uint32_t)> job;
    };
    template <size_t capacity>
    class JobQueue {
    public:
        // Push an item to the end if there is free space
        //  Returns true if succesful
        //  Returns false if there is not enough space
        inline bool push_back(const Job& job) {
            bool result = false;
            std::scoped_lock lock(m_Lock);
            size_t next = (m_Head + 1) % capacity;
            if (next != m_Tail) {
                m_Data[m_Head] = job;
                m_Head = next;
                result = true;
            }
            return result;
        }

        // Get an item if there are any
        //  Returns true if succesful
        //  Returns false if there are no items
        inline bool pop_front(Job& job) {
            bool result = false;
            std::scoped_lock lock(m_Lock);
            if (m_Tail != m_Head) {
                job = m_Data[m_Tail];
                m_Tail = (m_Tail + 1) % capacity;
                result = true;
            }
            return result;
        }

    private:
        Job m_Data[capacity];
        size_t m_Head = 0ull;
        size_t m_Tail = 0ull;
        std::mutex m_Lock;
    };

    class SpinLock {
    public:
        void lock() {
            while (true) {
                while (m_lock) {
                    _mm_pause();
                }
                if (!m_lock.exchange(true))
                    break;
            }
        }

        void unlock() {
            m_lock.store(false);
        }

    private:
        std::atomic<bool> m_lock = false;
    };

    void Initialize();
    void Execute(const std::function<void(uint32_t)>& job, JobCounter** counter);
    void Execute(std::vector<std::function<void(uint32_t)>>& jobs, JobCounter** counter);
    bool IsBusy();
    void Wait();
    void WaitForCounter(JobCounter* counter, uint64_t value);
    void Stop();
    uint32_t Threads();
}
#endif