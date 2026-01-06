#ifndef _CPP_BITS_SINGLETON_H_
#define _CPP_BITS_SINGLETON_H_

#include <cstdint>
#include <atomic>
#include <new>

/*
This is singleton as it should be implemented using preallocated static storage.

On program shutdown, method __stop__() will be called, to indicate program is shutting down.
The singleton can stop internal processing or switch to shutdown mode, but should never put itself
into invalid state. As this is singleton, it is possible that it gets called even after the __stop__()
call and it needs to at least not crash.

That is reason why the desatructor of the object is never called.

Sample usage:

#include <iostream>

struct ToSingleton {
    ToSingleton() {
        std::cout << "ToSingleton()" << std::endl;
    }
    ~ToSingleton() {
        std::cout << "~ToSingleton()" << std::endl;
    }
    void __stop__() {
        std::cout << "__stop__()" << std::endl;
    }
    void method(){
        std::cout << "method()" << std::endl;
    }
};

int main() {

   using sgt = cppbits::Singleton<ToSingleton>;

   sgt::instance().method();
   ToSingleton& x = sgt::instance();
   x.method();
}

The output will be:

ToSingleton()
method()
method()
__stop__()


*/

namespace cppbits {

template <typename Type>
class Singleton {
    // Disable copy and move(shallow copy)
    Singleton(const Singleton&) = delete;
    Singleton(const Singleton&&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton& operator=(const Singleton&&) = delete;

    class Initialiser {
        Type* ptr_{};

    public:
        Initialiser(Type* aPtr) : ptr_(aPtr) {};
        Initialiser(std::atomic<Type*>& aPtr, uint8_t aMem[]) : Initialiser(aPtr = ::new (aMem) Type) {};
        ~Initialiser() {
            ptr_->__stop__();
        };
        // Disable copy and move(shallow copy)
        Initialiser(const Initialiser&) = delete;
        Initialiser(const Initialiser&&) = delete;
        Initialiser& operator=(const Initialiser&) = delete;
        Initialiser& operator=(const Initialiser&&) = delete;
    };

    static Type& create() {
        static Initialiser init(ptr, storage.mem);
        return *ptr;
    };

    // Following members need to be POD as well, so no destriction takes place.
    struct Storage {
        uint8_t mem[sizeof(Type)] alignas(Type);
    };
    static Storage storage;
    static std::atomic<Type*> ptr;

    static_assert(std::is_trivially_destructible_v<decltype(storage)>, "Singleton::storage must be trivially destructible.");
    static_assert(std::is_trivially_destructible_v<decltype(ptr)>, "Singleton::ptr must be trivially destructible.");

#if __cplusplus >= 202002L
    static_assert(requires(Type t) {t.__stop__();}, "Singleton: Type must implement a 'void __stop__()' method.");
#endif

public:
    static Type& instance() {
        if (ptr.load(std::memory_order_acquire))
            return *ptr;
        return create();
    };
};

template <typename Type> typename Singleton<Type>::Storage Singleton<Type>::storage{};
template <typename Type> std::atomic<Type*> Singleton<Type>::ptr{};

} //namespace cppbits

#endif // _CPP_BITS_SINGLETON_H_
