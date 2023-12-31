#include "my_time.hpp"

#include <random>

namespace TlsCrashDemo {

namespace {

class GprNowTimeSource final : public Timestamp::Source {
public:
  Timestamp Now() override {
    return Timestamp(std::rand());
  }
};

}

thread_local Timestamp::Source* Timestamp::thread_local_time_source_ = nullptr;

Timestamp ScopedTimeCache::Now() {
  if (!cached_time_.has_value()) {
    previous()->InvalidateCache();
    cached_time_ = previous()->Now();
  }
  return *cached_time_;
}

}  // namespace TlsCrashDemo
