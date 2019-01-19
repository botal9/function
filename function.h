//
// Created by vitalya on 19.01.19.
//

#ifndef FUNCTION_FUNCTION_H
#define FUNCTION_FUNCTION_H

#include <utility>
#include <variant>
#include <memory>
#include <cstring>

constexpr const size_t MAX_SIZE = 16;
constexpr const size_t ALIGN = 16;

namespace {
    template <typename T>
    struct is_small_type {
        static const bool value = sizeof(T) <= MAX_SIZE
                && alignof(T) <= ALIGN
                && std::is_nothrow_move_constructible<T>::value;
    };

    enum OBJECT_TYPE {
        EMPTY = 0,
        SMALL,
        BIG
    };
}


template <typename T>
class function;

template <typename Ret, typename... Args>
class function<Ret (Args...)> {
private:
    using storage_type = typename std::aligned_storage<MAX_SIZE, ALIGN>::type;

    using invoker_type = Ret(*)(storage_type& storage, Args&&...);
    using deleter_type = void(*)(storage_type&);
    using copier_type = void(*)(storage_type& dst, const storage_type& src);
    using mover_type = void(*)(storage_type& dst, storage_type& src);

    struct function_traits {
        invoker_type invoker = nullptr;
        deleter_type deleter = nullptr;
        copier_type copier = nullptr;
        mover_type mover = nullptr;

        function_traits() = default;

        function_traits(invoker_type invoker_, deleter_type deleter_,
                copier_type copier_, mover_type mover_)
            : invoker(invoker_)
            , deleter(deleter_)
            , copier(copier_)
            , mover(mover_)
        {
        };

        function_traits& operator=(const function_traits& other)
        {
            invoker = other.invoker;
            deleter = other.deleter;
            copier = other.copier;
            mover = other.mover;

            return *this;
        };

        /*void swap(function_traits& other) {
            std::swap(invoker, other.invoker);
            std::swap(deleter, other.deleter);
            std::swap(copier, other.copier);
            std::swap(mover, other.mover);
        }*/
    };


    // move
    template <typename T>
    static void move_small_object(storage_type& storage, storage_type& other) {
        T& f = reinterpret_cast<T&>(other);
        new (&storage) T(std::move(f));
    }

    template <typename T>
    static void move_big_object(storage_type& storage, storage_type& other) {
        T*& f = reinterpret_cast<T*&>(other);
        new (&storage) T*(std::move(f));
    }

    static void move_empty_object(storage_type& storage, storage_type& other)
    {
    }


    // copy
    template <typename T>
    static void copy_small_object(storage_type& storage, const storage_type& other) {
        auto f = reinterpret_cast<const T&>(other);
        new (&storage) T(f);
    }

    template <typename T>
    static void copy_big_object(storage_type& storage, const storage_type& other) {
        auto f = reinterpret_cast<T* const&>(other);
        new (&storage) T*(f);
    }

    static void copy_empty_object(storage_type& storage, const storage_type& other)
    {
    }


    //delete
    template <typename T>
    static void delete_small_object(storage_type& storage) {
        reinterpret_cast<T&>(storage).~T();
    }

    template <typename T>
    static void delete_big_object(storage_type& storage) {
        delete reinterpret_cast<T*&>(storage);
    }

    static void delete_empty_object(storage_type& storage)
    {
    }


    //invoke
    template <typename T>
    static Ret invoke_small_object(storage_type& storage, Args&&... args) {
        return (reinterpret_cast<T&>(storage))(args...);
    }

    template <typename T>
    static Ret invoke_big_object(storage_type& storage, Args&&... args) {
        return (*reinterpret_cast<T*&>(storage))(args...);
    }

    static Ret invoke_empty_object(storage_type& storage, Args&&... args) {
        std::__throw_bad_function_call();
    }



    // result traits
    static function_traits empty_object_traits() {
        return function_traits(invoke_empty_object, delete_empty_object, copy_empty_object, move_empty_object);
    }

    template <typename T>
    static function_traits big_object_traits() {
        return function_traits(invoke_big_object<T>, delete_big_object<T>, copy_big_object<T>, move_big_object<T>);
    }

    template <typename T>
    static function_traits small_object_traits() {
        return function_traits(invoke_small_object<T>, delete_small_object<T>, copy_small_object<T>, move_small_object<T>);
    }


    template <typename T>
    static function_traits object_traits(OBJECT_TYPE object_type) {
        switch (object_type) {
            case EMPTY:
                return empty_object_traits();
            case SMALL:
                return small_object_traits<T>();
            case BIG:
                return big_object_traits<T>();
            default:
                std::__throw_bad_function_call();
        }
    }

public:
    function() noexcept
        : type(EMPTY)
        , traits(empty_object_traits())
    {
    }

    function(std::nullptr_t) noexcept
        : function()
    {
    }

    ~function() {}

    function(const function& f)
        : traits(f.traits)
        , type(f.type)
    {
        traits.copier(storage, f.storage);
    }

    function(function&& f) noexcept
        : traits(f.traits)
        , type(f.type)
    {
        f.type = EMPTY;
        traits.mover(storage, f.storage);
    }

    template <typename Func>
    function(Func f)
    {
        if (is_small_type<Func>::value) {
            new (&storage) Func(std::move(f));
            type = SMALL;
        } else {
            new (&storage) Func*(new Func(std::move(f)));
            type = BIG;
        }
        traits = object_traits<Func>(type);
    }

    function& operator=(const function& other) {
        traits.deleter(storage);
        traits = other.traits;
        type = other.type;
        traits.copier(storage, other.storage);

        return *this;
    }

    function& operator=(function&& other) noexcept {
        traits.deleter(storage);
        traits = other.traits;
        type = other.type;
        other.type = EMPTY;
        traits.move(storage, other.storage);

        return *this;
    }

    explicit operator bool() const noexcept {
        return !(type == EMPTY);
    }

    void swap(function& other) noexcept {
        storage_type buff1;
        storage_type buff2;

        traits.mover(buff1, storage);
        traits.deleter(storage);

        other.traits.mover(buff2, other.storage);
        other.traits.deleter(other.storage);

        std::swap(type, other.type);
        std::swap(traits, other.traits);

        other.traits.mover(other.storage, buff1);
        other.traits.deleter(buff1);

        traits.mover(storage, buff2);
        traits.deleter(buff2);
    }

    Ret operator()(Args&&... args) {
        if (type == EMPTY) {
            std::__throw_bad_function_call();
        }
        return traits.invoker(storage, std::forward<Args>(args)...);
    }

private:
    function_traits traits;
    storage_type storage;
    OBJECT_TYPE type;
};


#endif //FUNCTION_FUNCTION_H
