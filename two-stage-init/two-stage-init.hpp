#ifndef _CPPBITS_TWOSTAGEINIT_H_
#define _CPPBITS_TWOSTAGEINIT_H_

#include <vector>
#include <boost/range/adaptor/reversed.hpp>
#include <functional>
#include <boost/noncopyable.hpp>


/*
 This set of classes automates two stage init pattern and also allows safe event registration in middle hierarchy classes.
 
 Say we have classes:
 
 class A;
 class B : public A;
 class C : public B;

 Class B can register for events, in construcor and deregister in destructor safely. Like:
 
 class B : public A, public cppbits::TwoStageInitVirtualBase {
     B() : A() {
         __addTwoStageInitPostConstructCall__([this](){ registration_call ;});
         __addTwoStageInitPreDestructCall__([this](){ de-registration_call ;});
     };
 }
 
 and construction:

 TwoStageInit<C> c;
 
 Once the object is constructed, all the constructors will be executed until the whole object hierarchy is fully initialised.
 Only afterwards, the Post registered calls, will be executed in the order of the original constructors.
 
 Before destruction, all the registered pre destruct calls will be called, before the actual object destructors.

*/

namespace cppbits {

namespace impl {
class TwoStageInitCaller;
};

class TwoStageInitVirtualBase {
public:
    virtual ~TwoStageInitVirtualBase() = default;
    
protected:
    using __TwoStageInitCall__ = std::function<void()>;
    
    void __addTwoStageInitPostConstructCall__(__TwoStageInitCall__ call) {__postConstructCalls__.push_back(call);};
    void __addTwoStageInitPreDestructCall__(__TwoStageInitCall__ call) {__preDestructCalls__.push_back(call);};
    
private:
    friend class impl::TwoStageInitCaller;
    
    void __executePostConstructCalls__() const { for (auto &c: __postConstructCalls__) { c(); }; };
    void __executePreDestructCalls__() const { for (auto &d: boost::adaptors::reverse(__preDestructCalls__)) { d(); }; };
    
    using __TwoStageInitCalls__ = std::vector<__TwoStageInitCall__>;
    __TwoStageInitCalls__ __postConstructCalls__;
    __TwoStageInitCalls__ __preDestructCalls__;
    
    // Make sure that classes inherited from this one, are constructed using the wrapper.
    virtual void __use_TwoStageInit_wrapper_to_construct__() = 0;
};


namespace impl {

class TwoStageInitCaller : public boost::noncopyable {
    const TwoStageInitVirtualBase &__b__;
    
protected:
    TwoStageInitCaller(const TwoStageInitVirtualBase *b) : __b__(*b) {
        __b__.__executePostConstructCalls__();
    }
    ~TwoStageInitCaller() {
        __b__.__executePreDestructCalls__();
    }
};
} // namespace impl

template <typename T>
class TwoStageInit final : public T, protected impl::TwoStageInitCaller {
public:
    template <typename... Args>
    TwoStageInit(Args&&... args) : T(std::forward<Args>(args)...), impl::TwoStageInitCaller(this) {};
    
private:
    // Silence compiler warning about undefined method.
    void __use_TwoStageInit_wrapper_to_construct__() {};
    
};

template <typename T, typename F1, typename F2 = void, typename F3 = void, typename F4 = void>
class TwoStageInitRestricted final : public T, protected impl::TwoStageInitCaller {
private:
    template <typename... Args>
    TwoStageInitRestricted(Args&&... args) : T(std::forward<Args>(args)...), impl::TwoStageInitCaller(this) {};
    
private:
    // Silence compiler warning about undefined method.
    void __use_TwoStageInit_wrapper_to_construct__() {};
    
    friend F1;
    friend F2;
    friend F3;
    friend F4;
};

template <typename T, template <typename> class F1>
class TwoStageInitRestrictedT final : public T, protected impl::TwoStageInitCaller {
private:
    template <typename... Args>
    TwoStageInitRestrictedT(Args&&... args) : T(std::forward<Args>(args)...), impl::TwoStageInitCaller(this) {};
    
private:
    // Silence compiler warning about undefined method.
    void __use_TwoStageInit_wrapper_to_construct__() {};
    
    friend F1<TwoStageInitRestrictedT<T, F1> >;
};

}


#endif // _CPPBITS_TWOSTAGEINIT_H_
