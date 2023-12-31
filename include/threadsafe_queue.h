/**
 *  Copyright (c) 2015 by Contributors
 *  from
 * :https://github.com/dmlc/ps-lite/blob/master/include/ps/internal/threadsafe_queue.h
 */
#ifndef _THREADSAFE_QUEUE_H_
#define _THREADSAFE_QUEUE_H_
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <utility>

/**
 * \brief thread-safe queue allowing push and waited pop
 */
template <typename T> class ThreadsafeQueue {
  public:
    ThreadsafeQueue() {}
    ~ThreadsafeQueue() {}

    /**
     * \brief push an value into the end. threadsafe.
     * \param new_value the value
     */
    void Push(T new_value) {
        mu_.lock();
        queue_.push(std::move(new_value));
        mu_.unlock();
        cond_.notify_all();
    }

    /**
     * \brief wait until pop an element from the beginning, threadsafe
     * \param value the poped value
     */
    void WaitAndPop(T *value) {
        std::unique_lock<std::mutex> lk(mu_);
        cond_.wait(lk, [this] { return !queue_.empty(); });
        *value = std::move(queue_.front());
        queue_.pop();
    }
    void Clear() {
        std::unique_lock<std::mutex> lk(mu_);
        while (!queue_.empty()) {
            queue_.pop();
        }
    }

  private:
    mutable std::mutex mu_;
    std::queue<T> queue_;
    std::condition_variable cond_;
};

// bool TryPop(T& value) {
//   std::lock_guard<std::mutex> lk(mut);
//   if(data_queue.empty())
//     return false;
//   value=std::move(data_queue.front());
//   data_queue.pop();
//   return true;
// }
#endif