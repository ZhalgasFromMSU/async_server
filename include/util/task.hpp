#pragma once

#include <type_traits>
#include <utility>

namespace NAsync {

    template<typename TFunc>
    concept CVoidToVoid = std::is_same_v<void, std::invoke_result_t<TFunc>>;

    class ITask {
    public:
        virtual ~ITask() {}

        virtual void Execute() = 0;
    };

    template<CVoidToVoid TFunc>
    class TTask: public ITask {
    public:
        TTask(TFunc func)
            : Func_(std::move(func))
        {}

        void Execute() override {
            Func_();
        }

    private:
        TFunc Func_;
    };
} // namespace NAsync
