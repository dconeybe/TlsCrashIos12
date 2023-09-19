#ifndef TLS_CRASH_DEMO_FORK_H_
#define TLS_CRASH_DEMO_FORK_H_

#include <atomic>

/* Use GPR_LIKELY only in cases where you are sure that a certain outcome is the
 * most likely. Ideally, also collect performance numbers to justify the claim.
 */
#ifdef __GNUC__
#define GPR_LIKELY(x) __builtin_expect((x), 1)
#define GPR_UNLIKELY(x) __builtin_expect((x), 0)
#else /* __GNUC__ */
#define GPR_LIKELY(x) (x)
#define GPR_UNLIKELY(x) (x)
#endif /* __GNUC__ */

namespace TlsCrashDemo {

class Fork {
public:
  typedef void (*child_postfork_func)(void);

  static void GlobalInit();

  // Returns true if fork suppport is enabled, false otherwise
  static bool Enabled();

  // Increment the count of active ExecCtxs.
  // Will block until a pending fork is complete if one is in progress.
  static void IncExecCtxCount() {
    if (GPR_UNLIKELY(support_enabled_.load(std::memory_order_relaxed))) {
      DoIncExecCtxCount();
    }
  }

  // Decrement the count of active ExecCtxs
  static void DecExecCtxCount() {
    if (GPR_UNLIKELY(support_enabled_.load(std::memory_order_relaxed))) {
      DoDecExecCtxCount();
    }
  }

  // Provide a function that will be invoked in the child's postfork handler to
  // reset the polling engine's internal state.
  static void SetResetChildPollingEngineFunc(
      child_postfork_func reset_child_polling_engine);

  static child_postfork_func GetResetChildPollingEngineFunc();

  // Check if there is a single active ExecCtx
  // (the one used to invoke this function).  If there are more,
  // return false.  Otherwise, return true and block creation of
  // more ExecCtx s until AlloWExecCtx() is called
  //
  static bool BlockExecCtx();

  static void AllowExecCtx();

  // Increment the count of active threads.
  static void IncThreadCount();

  // Decrement the count of active threads.
  static void DecThreadCount();

  // Await all core threads to be joined.
  static void AwaitThreads();

  // Test only: overrides environment variables/compile flags
  // Must be called before grpc_init()
  static void Enable(bool enable);

private:
  static void DoIncExecCtxCount();

  static void DoDecExecCtxCount();

  static std::atomic<bool> support_enabled_;
  static bool override_enabled_;
  static child_postfork_func reset_child_polling_engine_;
};

}  // namespace TlsCrashDemo

#endif /* TLS_CRASH_DEMO_FORK_H_ */
