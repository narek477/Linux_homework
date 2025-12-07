#include "parallel_scheduler.h"

void* parallel_scheduler::thread_worker(void* scheduler_ptr) {
	parallel_scheduler* scheduler = static_cast<parallel_scheduler*>(scheduler_ptr);
	scheduler->process_tasks();
	return nullptr;
}


void parallel_scheduler::process_tasks() {
	while(true) {
		pthread_mutex_lock(&queue_lock);

		while(task_queue.empty() && !shutdown) {
			pthread_cond_wait(&task_available, &queue_lock);
		}

		if(shutdown && task_queue.empty()) {
			pthread_mutex_lock(&queue_lock);
			return;
		}

		Task current_task = task_queue.front();
		task_queue.pop();

		pthread_mutex_unlock(&queue_lock);
		
		current_task.task_func(current_task.task_arg);
	
	}
}


parallel_scheduler::parallel_scheduler(int thread_num): thread_count(thread_num) {
	pthread_mutex_init(&queue_lock, nullptr);
	pthread_cond_init(&task_available, nullptr);
	
	threads = new pthread_t[thread_count];

	for(int i = 0; i < thread_count; ++i) {
		pthread_create(&threads[i], nullptr, thread_worker, this);
	}
}


void parallel_scheduler::execute(void (*func)(void*), void* arg) {
	Task new_task;
	new_task.task_func = func;
	new_task.task_arg = arg;

	pthread_mutex_lock(&queue_lock);
	task_queue.push(new_task);
	pthread_mutex_unlock(&queue_lock);

	pthread_cond_signal(&task_available);
}


parallel_scheduler::~parallel_scheduler() {
	pthread_mutex_lock(&queue_lock);
	shutdown = true;
	pthread_mutex_unlock(&queue_lock);

	pthread_cond_broadcast(&task_available);

	for(int i = 0; i < thread_count; i++) {
		pthread_join(threads[i], nullptr);
	}

	delete[] threads;

	pthread_mutex_destroy(&queue_lock);
	pthread_cond_destroy(&task_available);

}
