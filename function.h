//
// Created by vitalya on 19.01.19.
//

#ifndef FUNCTION_FUNCTION_H
#define FUNCTION_FUNCTION_H

#include <utility>
#include <variant>
#include <memory>

constexpr const size_t MAX_SIZE = 16;

template <typename T>
class function;

template <typename Ret, typename... Args>
class function<Ret (Args...)> {
public:
    function() noexcept : holder(nullptr)
    {
    };

    function(std::nullptr_t) noexcept
        : function()
    {
    };

    ~function() {}

    function(const function& f)
        : holder(new function_holder(f))
    {
    };

    function(function&& f) noexcept
        : holder(nullptr)
    {
        holder.swap(f.holder);
    };

    template <typename Func>
    function(Func f)
         : holder(new function_holder<Func>(f))
    {
    };

    function& operator=(const function& other) {
        function copy(other);
        std::swap(holder, copy.holder);
    }

    function& operator=(function&& other) noexcept {
        std::swap(holder, other.holder);
    }

    explicit operator bool() const noexcept {
        return holder;
    }

    void swap(function& other) noexcept {
        holder.swap(other.holder);
    }

    Ret operator()(Args&&... args) {
        if (!holder) {
            throw std::runtime_error("undefined function");
        }
        return holder->invoke(std::forward<Args>(args)...);
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
        {
            if (sizeof(f) <= MAX_SIZE) {
                inner_function = f;
            } else {
                inner_function = &f;
            }
        }

        Ret invoke(Args&&... args) {
            if (std::holds_alternative<Func>(inner_function)) {
                return std::get<Func>(inner_function)(std::forward<Args>(args)...);
            } else {
                return (*std::get<Func*>(inner_function))(std::forward<Args>(args)...);
            }
        }

    private:
        std::variant<Func, Func*> inner_function;
    };

    std::unique_ptr<function_holder_base> holder;
};


#endif //FUNCTION_FUNCTION_H
