#pragma once

#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <queue>


class parallel_scheduler {

private:
	int thread_count;
	bool shutdown = false;

	pthread_t* threads;
	pthread_mutex_t queue_lock;
	pthread_cond_t task_available;

	struct Task {
		void (*task_func)(void*);
		void* task_arg;
	};

	std::queue<Task> task_queue;

	static void* thread_worker(void* scheduler_ptr);
	void process_tasks();



public:

	explicit parallel_scheduler(int thread_num);

	void execute(void (*func)(void*), void* arg);

	~parallel_scheduler();	

};

