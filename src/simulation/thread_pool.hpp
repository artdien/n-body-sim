#pragma once

#include <barrier>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

#include "platform/types.hpp"

namespace nbodysim::simulation {

class ThreadPool {
public:
  /// Creates a thread pool with a given capacity.
  ///
  /// @param capacity Number of threads to create in the thread pool.
  ThreadPool(usize capacity = std::max(1u, std::thread::hardware_concurrency() - 1));

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;
  ~ThreadPool();

  /// Schedules a task to run.
  ///
  /// Each task is assigned to an available thread for execution.
  /// If more tasks are scheduled than threads currently available,
  /// those tasks are put into a queue and are assigned when threads become available again.
  ///
  /// @param task Task to be scheduled for execution.
  auto schedule(std::function<void(void)>&& task) -> void;

  /// Waits until all threads are inactive.
  ///
  /// Inactive means that they are not currently executing any tasks and there are no more tasks queued to execute.
  /// In case all threads are already inactive when this method is called this method returns immediately.
  auto wait_until_inactive() -> void;

  /// Clears all tasks in current thread pool.
  ///
  /// Tasks that are currently being executed by threads are not cleared.
  auto clear() -> void;

  /// Gets the current capacity of the thread pool.
  ///
  /// Capacity is the maximum number of threads that are running in the thread pool.
  ///
  /// @return Capacity of thread pool.
  auto capacity() -> usize;

  /// Sets a synchronization barrier.
  ///
  /// This barrier applies to all threads in the pool.
  /// All threads in the pool must arrive at this barrier before they can continue.
  /// If the pool is currently not exhausted and a barrier is set,
  /// all currently active threads will wait at the barrier.
  auto barrier() -> void;

private:
  auto thread_execution_loop() -> void;

  bool running_;
  usize capacity_;
  usize active_threads_;

  std::mutex mutex_;
  std::condition_variable task_start_;
  std::condition_variable task_done_;
  std::barrier<> barrier_;

  std::queue<std::function<void(void)>> tasks_;
  std::vector<std::jthread> threads_;
};

} // namespace nbodysim::simulation
