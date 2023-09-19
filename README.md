Reproduction App for iOS 12 Crash from Thread-Local Variables

This bug was initially reported here: https://github.com/firebase/firebase-ios-sdk/issues/11509

It appears that in some cases thread-local variable initialization crashes when
the thread-local variable is being initialized from one module that is defined
in a different module.

Steps to Reproduce:
1. Open TlsCrashIos12.xcworkspace in Xcode.
2. Compile the app is _release_ mode.
3. Install and run the app on an iPhone running iOS 12.

Expected Results:
Application completes successfully without crashing (the app has no UI).

Actual Results:
Application crashes with error: Thread 1: EXC_BAD_ACCESS (code=1, address=0x10)

Notes:
- Make sure app is compiled in "release" mode, as the crash does _not_ occur
    when compiled in "debug" mode.
- The crash only occurs in iOS 12; later iOS versions do not suffer from this
    crash.
- The app has no UI; it only has a main method that causes the crash; therefore,
    when you run the app on an iPhone you will not "see" anything.
- The C++ code in TlsCrashIos12Lib/TlsCrashIos12Lib was copied from
    https://github.com/grpc/grpc and adapted to compile, with a lot of unneeded
    code stripped out.
