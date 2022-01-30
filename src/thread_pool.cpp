#include "thread_pool.hpp"

void ThreadPool::Shutdown() {
  if (threads_.empty()) {
    return;
  }

  {
    std::lock_guard<std::mutex> lock(mutex_);
    shutdown_ = true;
  }

  action_for_worker_.notify_all();

  for (std::thread& thread : threads_) {
    thread.join();
  }

  threads_.clear();
}

void ThreadPool::WorkerThread(std::size_t number) {
  while (true) {
    std::unique_ptr<BaseJob> job = nullptr;
    {
      std::unique_lock<std::mutex> lock(mutex_);

      action_for_worker_.wait(lock, [this]() {
        return !jobs_.empty() || shutdown_;
      });

      if (!jobs_.empty()) {
        job = std::move(jobs_.front());
        jobs_.pop();
        queue_is_not_full_.notify_one();
      }
      else if (shutdown_) {
        return;
      }
    }
    job->execute();
  }
}
