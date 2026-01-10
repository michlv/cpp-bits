#ifndef _CPPBITS_TWOSTAGEINIT_H_
#define _CPPBITS_TWOSTAGEINIT_H_

#include <type_traits>
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
 
 class B : public A, virtual public cppbits::TwoStageInitVirtualBase {
     B() : A() {
         addTwoStageInitPostConstructCall([this](){ registration_call ;});
         addTwoStageInitPreDestructCall([this](){ de-registration_call ;});
     };
 }
 
 and construction:

 TwoStageInit<C> c;
 
 Once the object is constructed, all the constructors will be executed until the whole object hierarchy is fully initialised.
 Only afterwards, the PostConstruct registered calls, will be executed in the order of the original constructors.
 
 Before destruction, all the registered PreDestruct calls will be called, in the oppsite order of PostConstruct calls,
 before the actual object destructors.

*/

namespace cppbits {

namespace impl {
class TwoStageInitCaller;
};

class TwoStageInitVirtualBase {
public:
    virtual ~TwoStageInitVirtualBase() = default;
    
protected:
    using _TwoStageInit_Call = std::function<void()>;
    
    void addTwoStageInitPostConstructCall(_TwoStageInit_Call call) {_TwoStageInit_postConstructCalls.push_back(std::move(call));};
    void addTwoStageInitPreDestructCall(_TwoStageInit_Call call) {_TwoStageInit_preDestructCalls.push_back(std::move(call));};
    
private:
    friend class impl::TwoStageInitCaller;
    
    void _TwoStageInit_executePostConstructCalls() const { for (auto &c: _TwoStageInit_postConstructCalls) { c(); }; };
    void _TwoStageInit_executePreDestructCalls() const { for (auto &d: boost::adaptors::reverse(_TwoStageInit_preDestructCalls)) { d(); }; };
    
    using _TwoStageInit_Calls = std::vector<_TwoStageInit_Call>;
    _TwoStageInit_Calls _TwoStageInit_postConstructCalls;
    _TwoStageInit_Calls _TwoStageInit_preDestructCalls;
    
    // Make sure that classes inherited from this one, are constructed using the wrapper.
    virtual void _use_TwoStageInit_wrapper_to_construct_() = 0;
};


namespace impl {

class TwoStageInitCaller : public boost::noncopyable {
    const TwoStageInitVirtualBase &_TwoStageInit_base;
    
protected:
    TwoStageInitCaller(const TwoStageInitVirtualBase *b) : _TwoStageInit_base(*b) {
        _TwoStageInit_base._TwoStageInit_executePostConstructCalls();
    }
    ~TwoStageInitCaller() {
        _TwoStageInit_base._TwoStageInit_executePreDestructCalls();
    }
};
} // namespace impl

template <typename T>
class TwoStageInit final : public T, protected impl::TwoStageInitCaller {
    static_assert(std::is_base_of<TwoStageInitVirtualBase, T>::value, "T must inherit from TwoStageInitVirtualBase to use the TwoStageInit wrapper.");
public:
    template <typename... Args>
    TwoStageInit(Args&&... args) : T(std::forward<Args>(args)...), impl::TwoStageInitCaller(this) {};
    
private:
    // Silence compiler warning about undefined method.
    void _use_TwoStageInit_wrapper_to_construct_() override {};
    
};

template <typename T, typename F1, typename F2 = void, typename F3 = void, typename F4 = void>
class TwoStageInitRestricted final : public T, protected impl::TwoStageInitCaller {
    static_assert(std::is_base_of<TwoStageInitVirtualBase, T>::value, "T must inherit from TwoStageInitVirtualBase to use the TwoStageInit wrapper.");
private:
    template <typename... Args>
    TwoStageInitRestricted(Args&&... args) : T(std::forward<Args>(args)...), impl::TwoStageInitCaller(this) {};
    
private:
    // Silence compiler warning about undefined method.
    void _use_TwoStageInit_wrapper_to_construct_() override {};
    
    friend F1;
    friend F2;
    friend F3;
    friend F4;
};

template <typename T, template <typename> class F1>
class TwoStageInitRestrictedT final : public T, protected impl::TwoStageInitCaller {
    static_assert(std::is_base_of<TwoStageInitVirtualBase, T>::value, "T must inherit from TwoStageInitVirtualBase to use the TwoStageInit wrapper.");
private:
    template <typename... Args>
    TwoStageInitRestrictedT(Args&&... args) : T(std::forward<Args>(args)...), impl::TwoStageInitCaller(this) {};
    
private:
    // Silence compiler warning about undefined method.
    void _use_TwoStageInit_wrapper_to_construct_() override {};
    
    friend F1<TwoStageInitRestrictedT<T, F1> >;
};

}


#endif // _CPPBITS_TWOSTAGEINIT_H_
