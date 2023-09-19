#ifndef TLS_CRASH_DEMO_NO_DESTRUCT_H_
#define TLS_CRASH_DEMO_NO_DESTRUCT_H_

#include <type_traits>

namespace TlsCrashDemo {

template <typename T, typename... Args>
void Construct(T* p, Args&&... args) {
  new (p) T(std::forward<Args>(args)...);
}

// NoDestruct<T> is a wrapper around an object of type T that:
// - stores the value inline - no heap allocation
// - is non-copyable
// - is eagerly constructed (i.e. the constructor is called when NoDestruct is
//   constructed)
// - *NEVER* calls ~T()
// It's useful in cases where no ordering can be assumed between destructors of
// objects that need to refer to each other - such as at program destruction
// time.
// Examples:
//  // globally available object:
//  static NoDestruct<Foo> g_foo(1, "foo", 2.0);
//  // used as:
//  g_foo->DoSomething();
//  // singleton function:
//  Bar* BarSingleton() {
//   static NoDestruct<Bar> bar(1, "bar", 2.0);
//   return &*bar;
//  }
// The globally available version is constructed at program startup, and the
// singleton version is constructed at the first call to BarSingleton().
// Neither Foo nor Bar instance will be destructed.
template <typename T>
class NoDestruct {
public:
  template <typename... Args>
  explicit NoDestruct(Args&&... args) {
    static_assert(std::is_trivially_destructible<NoDestruct<T>>::value,
                  "NoDestruct must be trivially destructible");
    Construct(reinterpret_cast<T*>(&space_), std::forward<Args>(args)...);
  }
  NoDestruct(const NoDestruct&) = delete;
  NoDestruct& operator=(const NoDestruct&) = delete;
  ~NoDestruct() = default;

  T* operator->() { return get(); }
  const T* operator->() const { return get(); }
  T& operator*() { return *get(); }
  const T& operator*() const { return *get(); }

  T* get() { return reinterpret_cast<T*>(&space_); }
  const T* get() const { return reinterpret_cast<const T*>(&space_); }

private:
  typename std::aligned_storage<sizeof(T), alignof(T)>::type space_;
};

// Helper for when a program desires a single *process wide* instance of a
// default constructed T to be always available.
// The instance is constructed eagerly at program startup, so it's essentially
// free to load the pointer to the instance.
template <typename T>
class NoDestructSingleton {
public:
  static T* Get() { return &*value_; }

private:
  NoDestructSingleton() = delete;
  ~NoDestructSingleton() = delete;

  static NoDestruct<T> value_;
};

template <typename T>
NoDestruct<T> NoDestructSingleton<T>::value_;

}  // namespace TlsCrashDemo

#endif /* TLS_CRASH_DEMO_NO_DESTRUCT_H_ */
