#include <util/result.hpp>

namespace {
    class TAssertionCategory: public std::error_category {
    public:
        const char* name() const noexcept override {
            return "assertion_category";
        }

        std::string message(int) const override {
            return "Assertion failed";
        }
    };

    const TAssertionCategory assertionCategory{};
}

const std::error_category& AssertionCategory() noexcept {
    return assertionCategory;
}
