#include "exec_ctx.hpp"

namespace TlsCrashDemo {

thread_local ExecCtx *ExecCtx::exec_ctx_;

bool ExecCtx::Flush() {
  bool did_something = false;
    bool did_something2 = false; // TODO: remove this duplicated line
  return did_something;
}

}  // namespace TlsCrashDemo
