#include "JobSystem.h"    // include our interface

#include <Windows.h>
#include <string>
#include <algorithm>    
#include <atomic> 
#include <thread>
#include <condition_variable>

namespace AT::JobSystem {
    uint32_t numThreads = 0;
    JobQueue<256> jobPool;
    std::condition_variable wakeCondition;
    std::mutex wakeMutex;
    uint64_t currentLabel = 0;
    std::atomic<uint64_t> finishedLabel;
    std::atomic<bool> quit = false;

    void Initialize() {
        finishedLabel.store(0);

        auto numCores = std::thread::hardware_concurrency();
        numThreads = 16u;//(std::max)(1u, numCores);

        for (uint32_t threadID = 0; threadID < numThreads; ++threadID) {
            std::thread worker([=] {
                uint32_t ID = threadID;
                Job job;
                JobCounter* counter;
                while (!quit) {
                    if (jobPool.pop_front(job)) {
                        job.job(ID); // execute job
                        if (job.counter != nullptr) {
                            --(*job.counter);
                        }
                        finishedLabel.fetch_add(1);
                    }
                    else {
                        std::unique_lock<std::mutex> lock(wakeMutex);
                        wakeCondition.wait(lock);
                    }
                }
            });
            worker.detach();
        }
    }

    inline void poll()     {
        wakeCondition.notify_one();
        std::this_thread::yield();
    }

    void Execute(const std::function<void(uint32_t)>& job, JobCounter** counter) {
        currentLabel += 1;

        if (counter != nullptr) {
            *counter = new JobCounter(1);
            while (!jobPool.push_back(Job{ *counter, job })) { poll(); }
        } else {
            while (!jobPool.push_back(Job{ nullptr, job })) { poll(); }
        }

        wakeCondition.notify_one();
    }

    void Execute(std::vector<std::function<void(uint32_t)>>& jobs, JobCounter** counter) {
        if (counter != nullptr) {
            *counter = new JobCounter(jobs.size());
        }
        for (auto job : jobs) {
            currentLabel += 1;
            if (counter != nullptr) {
                while (!jobPool.push_back(Job{ *counter, job })) { poll(); }
            }
            else {
                while (!jobPool.push_back(Job{ nullptr, job })) { poll(); }
            }
            wakeCondition.notify_one();
        }
    }

    bool IsBusy() {
        return finishedLabel.load() < currentLabel;
    }

    void Wait() {
        while (IsBusy()) { poll(); }
    }

    void WaitForCounter(JobCounter* counter, uint64_t value) {
        while (*counter > value) {}
    }

    void Stop() {
        quit = true;
    }

    uint32_t Threads() {
        return numThreads;
    }
}