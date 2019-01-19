#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <functional>
#include <set>


#include "function.h"

int64_t sqr(int32_t x) {
    return x * x;
}

bool greater(const int &a, const int &b) {
    return b < a;
}

struct cmp {
    bool operator()(int a, int b) {
        return b < a;
    }
};

int main() {
    function<int64_t (int32_t)> f1(sqr);
    function<bool (const int&, const int&)> f2(greater);
    function<bool (int, int)> f3(cmp);


    std::vector<int> v{2, 3, 1, 7, -3};
    std::sort(v.begin(), v.end(), greater);

    std::set<int, cmp> s;

    std::cout << f2(7, 1) << std::endl;
    std::cout << f1(10) << std::endl;
    std::for_each(v.begin(), v.end(), [](int x) {std::cout << x << " ";});
}