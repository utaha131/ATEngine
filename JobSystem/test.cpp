#include <iostream>
#include <vector>
#include <thread>
#include <windows.h>
#include <queue>
#include <mutex>
#include <atomic>
#include <string>

#include <functional>

#define JOB_ENTRY_POINT(FunctionName) void FunctionName(void* parameters)
#define _64_KB_ 65536
#define _512_KB_ 524288 

struct Job {
	std::function<void(void*)> execute;
};

class JobQueue {
public:
	JobQueue() :
		m_mutex(std::mutex()),
		m_queue(std::queue<Job>())//,
		//m_condition(std::condition_variable())
	{

	}
	void PushJob(Job& job) {
		//std::unique_lock<std::mutex> lock(m_mutex);
		std::scoped_lock<std::mutex> lock(m_mutex);
		m_queue.push(job);
		//m_condition.notify_one();
	}

	bool PopJob(Job& job) {
		//std::unique_lock<std::mutex> lock(m_mutex);
		////std::scoped_lock<std::mutex> lock(m_mutex);
		//m_condition.wait(lock, [this]() {  return !m_queue.empty(); });
		//Job job = {};
		//if (!m_queue.empty()) {
		//	job = m_queue.front();
		//	m_queue.pop();
		//}
		//return job;
		std::scoped_lock<std::mutex> lock(m_mutex);
		if (m_queue.empty()) {
			return false;
		}
		job = std::move(m_queue.front());
		m_queue.pop();
		return true;
	}
private:
	std::mutex m_mutex;
	std::queue<Job> m_queue;
	//std::condition_variable m_condition;
};

enum JOB_PRIORITY {
	JOB_PRIORITY_HIGH,
	JOB_PRIORITY_MEDIUM,
	JOB_PRIORITY_LOW,
	JOB_PRIORITY_TYPE_COUNT,
};

std::atomic<bool> Finished = false;
std::atomic<uint64_t> Counter;
JobQueue queues[JOB_PRIORITY_TYPE_COUNT];
std::vector<LPVOID> fiber_pool = std::vector<LPVOID>();

typedef LPVOID* FiberHandle;
struct Fiber {

};

void WINAPI FiberLogic(void* arguments) {
	LPVOID fiber = reinterpret_cast<LPVOID>(arguments);
	uint64_t i = 0;
	while (!Finished) {
		Job job;
		if (queues[JOB_PRIORITY_HIGH].PopJob(job)) {
			job.execute(nullptr);
		}
		
	}
	SwitchToFiber(fiber);
}

void ThreadLogic(void) {
	LPVOID fiber = ConvertThreadToFiber(nullptr);
	
	LPVOID new_fiber = CreateFiber(_64_KB_, FiberLogic, fiber);
	SwitchToFiber(new_fiber);
	ConvertFiberToThread();
	DeleteFiber(new_fiber);
}
#include "C:/Program Files (x86)/Intel/oneAPI/vtune/latest/include/ittnotify.h"
#pragma comment(lib, "C:/Program Files (x86)/Intel/oneAPI/vtune/latest/lib64/libittnotify.lib")

int main() {
	__itt_pause();
	for (uint32_t i = 0; i < 100; ++i) {
		Job job;
		job.execute = [](void* args) {
			for (uint32_t i = 0; i < 1000000; ++i) {
				
			}
		};
		queues[JOB_PRIORITY_HIGH].PushJob(job);
	}
	std::cout << "Done Creating Jobs." << std::endl;
	//Create Fiber Pool.
	/*for (uint32_t i = 0; i < 128; ++i) {
		fiber_pool.push_back(CreateFiber(_64_KB_, FiberLogic, nullptr));
	}*/

	const unsigned int cpu_count = std::thread::hardware_concurrency();
	std::cout << "CPU Processor Count: " << cpu_count << std::endl;
	std::vector<std::thread> threads;// = std::vector<std::thread>(cpu_count - 2);
	std::cout << threads.size() << std::endl;
	for (unsigned int i = 0; i < cpu_count; ++i) {
		threads.emplace_back(std::thread(ThreadLogic));
		DWORD_PTR dword = SetThreadAffinityMask(threads.back().native_handle(), DWORD_PTR(1) << i);
		if (dword == 0) {
			std::cout << "Thread[" << i << "] set Affinity Failed." << GetLastError() << std::endl;
		}
	}

	__itt_resume();
	for (unsigned int i = 0; i < cpu_count; ++i) {
		threads[i].join();
	}

	return 0;
}