#include "double-init.hpp"

#include <cstring>
#include <memory>
#include <iostream>

std::string calls;

using namespace cppbits;

template <int a>
struct A : virtual public TwoStageInitVirtualBase {
    std::string n;
    
    A() : n(std::to_string(a)) {
        __addTwoStageInitPostConstructCall__([this](){this->record("PC");});
        __addTwoStageInitPreDestructCall__([this](){this->record("PD");});
        record("C");
    };
    ~A() {
        record("D");
    };
    void record(const std::string& what) const {
        calls += " " + n + what;
    }
};

struct M : public A<0>, public A<1> {
    M() {
        __addTwoStageInitPostConstructCall__([this](){this->record("PC");});
        __addTwoStageInitPreDestructCall__([this](){this->record("PD");});
        record("C");
    }
    ~M() {
        record("D");
    };
    void record(const std::string& what) const {
        calls += " M" + what;
    }
};


struct R;
using RT = TwoStageInitRestricted<A<0>, R>;

struct R {
    R() {
        RT r;
    };
};

void testSingleInheritance() {
    calls = "";
    { TwoStageInit<A<0>> a; };
    std::cout << calls << std::endl;
    std::cout << " 0C 0PC 0PD 0D" << std::endl;
};

void testMultipleInheritance() {
    calls = "";
    { TwoStageInit<M> a; };
    std::cout << calls << std::endl;
    std::cout << " 0C 1C MC 0PC 1PC MPC MPD 1PD 0PD MD 1D 0D" << std::endl;
};

void testSingleInheritanceBasePointer() {
    calls = "";
    { std::shared_ptr< A<0> > a(std::make_shared< TwoStageInit<A<0>> >()); };
    std::cout << calls << std::endl;
    std::cout << " 0C 0PC 0PD 0D" << std::endl;
};

void testSingleInheritanceRestricted() {
    calls = "";
    { R a; };
    std::cout << calls << std::endl;
    std::cout << " 0C 0PC 0PD 0D" << std::endl;
};

int main() {
    testSingleInheritance();
    testMultipleInheritance();
    testSingleInheritanceBasePointer();
    testSingleInheritanceRestricted();
};