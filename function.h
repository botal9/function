//
// Created by vitalya on 19.01.19.
//

#ifndef FUNCTION_FUNCTION_H
#define FUNCTION_FUNCTION_H

#include <utility>

template <typename T>
class function;

template <typename Ret, typename... Args>
class function<Ret (Args...)> {
public:
    function() : holder(nullptr) {}

    ~function() {
        delete holder;
    }

    template <typename Func>
    function(Func f) : holder(new function_holder<Func>(f)) {};

    Ret operator()(Args&&... args) {
        return holder->invoke(std::forward<Args>(args)...);
    }

    function& operator=(function other) {
        std::swap(holder, other.holder);
    }

private:
    class function_holder_base {
    public:
        function_holder_base() {};
        virtual ~function_holder_base() {};
        virtual Ret invoke(Args&&...) = 0;

    };

    template <typename Func>
    class function_holder : public function_holder_base {
    public:
        function_holder(Func f)
                : function_holder_base()
                , inner_function(f)
        {
        }

        Ret invoke(Args&&... args) {
            return inner_function(std::forward<Args>(args)...);
        }

    private:
        Func inner_function;
    };

    function_holder_base* holder = nullptr;
};


#endif //FUNCTION_FUNCTION_H
