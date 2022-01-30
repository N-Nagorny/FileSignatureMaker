#pragma once

#include <functional>
#include <future>
#include <queue>
#include <thread>

// Thread pool with a constant
// number of awaiting threads
// which take a new job as soon
// as it is available.
class ThreadPool {
public:
  explicit ThreadPool()
    : ThreadPool(std::thread::hardware_concurrency(), std::thread::hardware_concurrency())
  {}

  // max_jobs can be larger than num_threads, when light jobs, and equal to, when heavy
  ThreadPool(std::size_t num_threads, std::size_t max_jobs)
    : shutdown_(false)
    , max_jobs_(max_jobs)
  {
    threads_.resize(num_threads);
    for (std::size_t i = 0; i < num_threads; ++i)
    {
      threads_[i] = std::thread(&ThreadPool::WorkerThread, this, i);
    }
  }

  virtual ~ThreadPool() {
    Shutdown();
  };

  // Returns future so it's supposed EnqueueJob() caller catches exceptions
  template <typename Func, typename... Args>
  auto EnqueueJob(Func&& function, Args&&... args) {
    using return_type = std::invoke_result_t<Func, Args...>;
    std::packaged_task<return_type()> job(std::move(std::bind(function, args...)));
    std::future<return_type> future = job.get_future();
    {
      std::unique_lock<std::mutex> lock(mutex_);
      queue_is_not_full_.wait(lock, [this]() {
        return jobs_.size() < max_jobs_;
      });
      jobs_.emplace(std::make_unique<Job<return_type>>(std::move(job)));
    }

    action_for_worker_.notify_one();
    return future;
  }

  // Forces ThreadPool to complete jobs ASAP
  void Shutdown();

private:
  ThreadPool(const ThreadPool&) =
    delete;  // non construction-copyable
  ThreadPool& operator=(const ThreadPool&) =
    delete;  // non copyable

  class BaseJob {
  private:
    std::packaged_task<void()> function_;
  public:
    virtual ~BaseJob() {};
    virtual void execute() = 0;
  };

  template<typename RetType>
  class Job : public BaseJob {
  private:
    std::packaged_task<RetType()> function_;
  public:
    explicit Job(std::packaged_task<RetType()> function)
      : function_(std::move(function))
    {}
    void execute() override {
      function_();
    }
  };

  std::vector<std::thread> threads_;
  std::queue<std::unique_ptr<BaseJob>> jobs_;
  mutable std::mutex mutex_; // for both jobs_ and shutdown_
  mutable std::condition_variable action_for_worker_; // notifies worker about action to do
  mutable std::condition_variable queue_is_not_full_;
  bool shutdown_;
  std::size_t max_jobs_;

  void WorkerThread(std::size_t number);
};
