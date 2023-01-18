#pragma once

#include <variant>
#include <system_error>

#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define ERROR_INFO(expr_str) __FILE__ ":" STRINGIZE(__LINE__) ": " expr_str

#define VERIFY_SYSCALL(expr) \
    if (!(expr)) { \
        try { \
            throw std::system_error(errno, std::system_category(), ERROR_INFO(#expr)); \
        } catch (...) { \
            std::terminate(); \
        } \
    }

#define VERIFY_EC(error_code) \
    if (error_code) { \
        try { \
            throw std::system_error(error_code, ERROR_INFO(#error_code)); \
        } catch (...) { \
            std::terminate(); \
        } \
    }

#define VERIFY_RESULT(result) VERIFY_EC(result.Error())

const std::error_category& AssertionCategory() noexcept;

#define VERIFY(expr) \
    if (!(expr)) { \
        try { \
            throw std::system_error(0, AssertionCategory(), ERROR_INFO(#expr)); \
        } catch (...) { \
            std::terminate(); \
        } \
    }


namespace NAsync {

    // analogue of rust's std::Result<T, E>
    template<typename T>
        requires (!std::is_same_v<T, void>)
    class TResult: public std::variant<T, std::error_code> {
        using TBase = std::variant<T, std::error_code>;

    public:
        using TBase::TBase;

        operator bool() const {
            return !std::holds_alternative<std::error_code>(*this);
        }

        T& operator*() {
            return std::get<T>(*this);
        }

        const T& operator*() const {
            return std::get<T>(*this);
        }

        T* operator->() {
            return &std::get<T>(*this);
        }

        const T* operator->() const {
            return &std::get<T>(*this);
        }

        std::error_code Error() const {
            if (*this) {
                return std::error_code{};
            }
            return std::get<std::error_code>(*this);
        }
    };

}