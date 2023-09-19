#include <iostream>

#include "exec_ctx.hpp"

int main(int argc, char** argv) {
    std::cout << "TlsCrashDemo started" << std::endl;
    TlsCrashDemo::ExecCtx exec_ctx;
    std::cout << "TlsCrashDemo completed without crashing" << std::endl;
}
