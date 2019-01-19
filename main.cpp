#include <iostream>

template <typename T>
class function;

template <typename Ret, typename... Args>
class function<Ret (Args...)> {
public:

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
        function_holder(Func func)
            : function_holder_base()
            , inner_function(func)
        {
        }

        Ret invoke(Args&&... args) {
            return inner_function(std::forward(args)...);
        }

    private:
        Func inner_function;
    };

    function_holder_base* holder = nullptr;
};


int main() {
}