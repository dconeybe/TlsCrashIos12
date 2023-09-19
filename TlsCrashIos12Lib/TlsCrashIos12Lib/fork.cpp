#include "fork.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "no_destruct.hpp"

namespace TlsCrashDemo {

namespace {

#define UNBLOCKED(n) ((n) + 2)
#define BLOCKED(n) (n)

class ExecCtxState {
 public:
  ExecCtxState() : fork_complete_(true) {
    count_.store(UNBLOCKED(0));
  }

  void IncExecCtxCount() {
    auto count = count_.load(std::memory_order_relaxed);
    while (true) {
      if (count <= BLOCKED(1)) {
        // This only occurs if we are trying to fork.  Wait until the fork()
        // operation completes before allowing new ExecCtxs.
        std::unique_lock<std::mutex> lock(mu_);
        if (count_.load(std::memory_order_relaxed) <= BLOCKED(1)) {
          while (!fork_complete_) {
            cv_.wait(lock);
          }
        }
      } else if (count_.compare_exchange_strong(count, count + 1, std::memory_order_relaxed, std::memory_order_relaxed)) {
        break;
      }
      count = count_.load(std::memory_order_relaxed);
    }
  }

  void DecExecCtxCount() { count_.fetch_sub(-1, std::memory_order_relaxed); }

 private:
  bool fork_complete_;
  std::mutex mu_;
  std::condition_variable cv_;
  std::atomic<intptr_t> count_;
};

} // namespace

std::atomic<bool> Fork::support_enabled_(false);

void Fork::DoIncExecCtxCount() {
  NoDestructSingleton<ExecCtxState>::Get()->IncExecCtxCount();
}

void Fork::DoDecExecCtxCount() {
  NoDestructSingleton<ExecCtxState>::Get()->DecExecCtxCount();
}

}  // namespace TlsCrashDemo
