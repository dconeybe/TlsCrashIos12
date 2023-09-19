#ifndef TLS_CRASH_DEMO_EXEC_CTX_H_
#define TLS_CRASH_DEMO_EXEC_CTX_H_

#include <limits>
#include <thread>

#include "my_time.hpp"

/* This exec_ctx is ready to return: either pre-populated, or cached as soon as
   the finish_check returns true */
#define GRPC_EXEC_CTX_FLAG_IS_FINISHED 1
/* The exec_ctx's thread is (potentially) owned by a call or channel: care
   should be given to not delete said call/channel from this exec_ctx */
#define GRPC_EXEC_CTX_FLAG_THREAD_RESOURCE_LOOP 2
/* This exec ctx was initialized by an internal thread, and should not
   be counted by fork handlers */
#define GRPC_EXEC_CTX_FLAG_IS_INTERNAL_THREAD 4

#define GRPC_CLOSURE_LIST_INIT { nullptr, nullptr }

namespace TlsCrashDemo {

/* This application callback exec ctx was initialized by an internal thread, and
   should not be counted by fork handlers */
#define GRPC_APP_CALLBACK_EXEC_CTX_FLAG_IS_INTERNAL_THREAD 1

struct grpc_closure;
typedef struct grpc_closure grpc_closure;

typedef struct grpc_closure_list {
  grpc_closure *head;
  grpc_closure *tail;
} grpc_closure_list;

class ExecCtx {
public:
  /** Default Constructor */

  ExecCtx() : flags_(GRPC_EXEC_CTX_FLAG_IS_FINISHED) {
    Set(this);
  }

  /** Parameterised Constructor */
  explicit ExecCtx(uintptr_t fl) : flags_(fl) {
    Set(this);
  }

  /** Destructor */
  virtual ~ExecCtx() {
    flags_ |= GRPC_EXEC_CTX_FLAG_IS_FINISHED;
    Flush();
    Set(last_exec_ctx_);
  }

  /** Disallow copy and assignment operators */
  ExecCtx(const ExecCtx &) = delete;

  ExecCtx &operator=(const ExecCtx &) = delete;

  /** Return pointer to grpc_closure_list */
  grpc_closure_list *closure_list() { return &closure_list_; }

  /** Return flags */
  uintptr_t flags() { return flags_; }

  /** Flush any work that has been enqueued onto this grpc_exec_ctx.
   *  Caller must guarantee that no interfering locks are held.
   *  Returns true if work was performed, false otherwise.
   */
  bool Flush();

  /** Returns true if we'd like to leave this execution context as soon as
   *  possible: useful for deciding whether to do something more or not
   *  depending on outside context.
   */
  bool IsReadyToFinish() {
    if ((flags_ & GRPC_EXEC_CTX_FLAG_IS_FINISHED) == 0) {
      if (CheckReadyToFinish()) {
        flags_ |= GRPC_EXEC_CTX_FLAG_IS_FINISHED;
        return true;
      }
      return false;
    } else {
      return true;
    }
  }

  Timestamp Now() { return Timestamp::Now(); }

  void InvalidateNow() { time_cache_.InvalidateCache(); }

  void TestOnlySetNow(Timestamp now) { time_cache_.TestOnlySetNow(now); }

  /** Gets pointer to current exec_ctx. */
  static ExecCtx *Get() { return exec_ctx_; }

protected:
  /** Check if ready to finish. */
  virtual bool CheckReadyToFinish() { return false; }

  /** Disallow delete on ExecCtx. */
  static void operator delete(void * /* p */) { abort(); }

private:
  /** Set exec_ctx_ to exec_ctx. */
  static void Set(ExecCtx *exec_ctx) { exec_ctx_ = exec_ctx; }

  grpc_closure_list closure_list_ = GRPC_CLOSURE_LIST_INIT;
  uintptr_t flags_;

  unsigned starting_cpu_ = std::numeric_limits<unsigned>::max();

  // (zzyzx) If the call to Get() below is changed to nullptr then the crash
  // goes away. Also, if the time_cache_ variable declaration is moved _above_
  // the exec_ctx_ variable declaration then the crash also goes away.
  static thread_local ExecCtx *exec_ctx_;
  ExecCtx *last_exec_ctx_ = Get();
  ScopedTimeCache time_cache_;
};

}  // namespace TlsCrashDemo

#endif /* TLS_CRASH_DEMO_EXEC_CTX_H_ */

