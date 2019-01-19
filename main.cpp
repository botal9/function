#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <functional>
#include <set>

#include "function.h"

static const std::vector<int> vect(100, 100);

int64_t sqr(int32_t x) {
    return x * x;
}

bool greater(const int &a, const int &b) {
    return b < a;
}

struct A {
    bool operator()(int a) {
        return (bool)a;
    }

    std::vector<int> a = vect;
};

struct Foo {
    Foo(int num) : num_(num) {};
    void print_add(int i) const { std::cout << num_+i << '\n'; }

    int num_;
};

void print_num(int i)
{
    std::cout << i << '\n';
}

int main() {
    function<int64_t (int32_t)> f1(sqr);
    function<bool (const int&, const int&)> f2(greater);
    function<bool (int)> f3 = A();

    function<void(void)> f;
    try {
        f();
    } catch (...) {
        std::cout << "empty function test\n";
    }
    
    std::cout << f3(1) << " " << f3(0) << std::endl;

    // store a free function
    std::function<void(int)> f_display = print_num;
    f_display(-9);

    // store a lambda
    std::function<void()> f_display_42 = []() { print_num(42); };
    f_display_42();

    // store the result of a call to std::bind
    std::function<void()> f_display_31337 = std::bind(print_num, 31337);
    f_display_31337();

    // store a call to a member function
    std::function<void(const Foo&, int)> f_add_display = &Foo::print_add;
    Foo foo(314159);
    f_add_display(foo, 1);
}