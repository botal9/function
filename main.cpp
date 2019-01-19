#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <functional>
#include <set>
#include <assert.h>

#include "function.h"

static const std::vector<int> vect(100, 100);

int64_t sqr(int32_t x) {
    return x * x;
}

bool greater(const int &a, const int &b) {
    return b < a;
}

bool func(int a) {
    return (bool)a;
}

struct A {
    bool operator()(int a) const {
        return (bool)a;
    }

    std::vector<int> a = vect;
};


struct B {
    char* c;

    B() {
        c = new char[100];
        std::fill(c, c + 100, 'w');
    }

    ~B() noexcept(false) {
        throw std::runtime_error("~B");
    }

    char operator()() const {
        return c[0];
    }
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
    } catch (std::bad_function_call& e) {
        std::cout << "empty function test\n";
    }

    assert(f3(1));
    assert(!f3(0));

    // store a free function
    function<void(int)> f_display = print_num;
    f_display(-9);

    // store a lambda
    function<void()> f_display_42 = []() { print_num(42); };
    f_display_42();

    // store the result of a call to std::bind
    function<void()> f_display_31337 = std::bind(print_num, 31337);
    f_display_31337();

    // store a call to a member function
    /*function<void(const Foo&, int)> f_add_display = &Foo::print_add;
    Foo foo(314159);
    f_add_display(foo, 1);*/

    // copy constructor test
    function<void(int)> f_copy = f_display;
    f_copy(228);
    f_copy = f_display;

    // move constructor test
    function<void(int)> f_move = std::move(f_display);
    f_move(1477);

    try {
        f_display(15);
    } catch (std::bad_function_call& e) {
        std::cout << "move function test\n";
    }

    //function<char (void)> fB = B();


    //swap test
    function<bool(int)> f11 = A();
    function<bool(int)> f12 = func;

    function<bool(int)> f21 = A();
    function<bool(int)> f22 = func;
    f21.swap(f22);
    assert(f11(0) == f22(0));
    assert(f12(1) == f21(1));
}