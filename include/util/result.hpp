#pragma once

#include <variant>
#include <system_error>

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
            return std::get<std::error_code>(*this);
        }
    };

}