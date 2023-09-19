#ifndef TLS_CRASH_DEMO_MY_TIME_H_
#define TLS_CRASH_DEMO_MY_TIME_H_

#include <cstdint>
#include <optional>

namespace TlsCrashDemo {

// Timestamp represents a discrete point in time.
class Timestamp {
public:
  explicit constexpr Timestamp(int64_t millis) : millis_(millis) {}

  // Base interface for time providers.
  class Source {
  public:
    // Return the current time.
    virtual Timestamp Now() = 0;
    virtual void InvalidateCache() {}

  protected:
    // We don't delete through this interface, so non-virtual dtor is fine.
    ~Source() = default;
  };

  class ScopedSource : public Source {
  public:
    ScopedSource() : previous_(thread_local_time_source_) {
      thread_local_time_source_ = this;
    }
    ScopedSource(const ScopedSource&) = delete;
    ScopedSource& operator=(const ScopedSource&) = delete;
    void InvalidateCache() override { previous_->InvalidateCache(); }

  protected:
    ~ScopedSource() { thread_local_time_source_ = previous_; }
    Source* previous() const { return previous_; }

  private:
    Source* const previous_;
  };

  constexpr Timestamp() = default;

  static Timestamp Now() { return thread_local_time_source_->Now(); }

  int64_t millis() const {
    return millis_;
  }

private:

  int64_t millis_ = 0;
  static thread_local Timestamp::Source* thread_local_time_source_;
};

class ScopedTimeCache final : public Timestamp::ScopedSource {
public:
  Timestamp Now() override;

  void InvalidateCache() override {
    cached_time_ = std::nullopt;
    Timestamp::ScopedSource::InvalidateCache();
  }
  void TestOnlySetNow(Timestamp now) { cached_time_ = now; }

private:
  std::optional<Timestamp> cached_time_;
};


}  // namespace TlsCrashDemo

#endif /* TLS_CRASH_DEMO_MY_TIME_H_ */
