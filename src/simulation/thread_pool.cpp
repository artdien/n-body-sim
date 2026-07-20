#include "simulation/thread_pool.hpp"

#include <algorithm>
#include <ranges>

namespace nbodysim::simulation {

ThreadPool::ThreadPool(usize capacity)
    : running_ {true}, capacity_ {capacity}, active_threads_ {0uz}, barrier_ {static_cast<ptrdiff_t>(capacity)} {
  std::ranges::for_each(std::views::iota(0uz, capacity_),
                        [this](auto _) { threads_.emplace_back([this]() { thread_execution_loop(); }); });
}

ThreadPool::~ThreadPool() {
  {
    const auto lock {std::unique_lock {mutex_}};
    running_ = false;
  }

  task_start_.notify_all();
}

auto ThreadPool::schedule(std::function<void(void)>&& task) -> void {
  {
    const auto lock {std::lock_guard {mutex_}};

    if (!running_) {
      throw std::runtime_error("Trying to schedule a task on a non-running thread pool");
    }

    tasks_.emplace(std::move(task));
  }

  task_start_.notify_one();
}

auto ThreadPool::wait_until_inactive() -> void {
  auto lock {std::unique_lock {mutex_}};
  task_done_.wait(lock, [this] { return active_threads_ == 0uz && tasks_.empty(); });
}

auto ThreadPool::barrier() -> void {
  barrier_.arrive_and_wait();
}

auto ThreadPool::thread_execution_loop() -> void {
  while (true) {
    std::function<void(void)> task;

    {
      auto lock {std::unique_lock {mutex_}};
      task_start_.wait(lock, [this] { return !running_ || !tasks_.empty(); });

      if (!running_ && tasks_.empty()) {
        return;
      }

      task = std::move(tasks_.front());
      tasks_.pop();

      active_threads_++;
    }

    task();

    {
      auto lock {std::lock_guard {mutex_}};
      active_threads_--;
    }

    task_done_.notify_all();
  }
}

} // namespace nbodysim::simulation
