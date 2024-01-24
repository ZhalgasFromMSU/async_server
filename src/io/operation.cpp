module;

#include <utility>

export module async:operation;

namespace async {

  export namespace arg {

    struct fd {
      int v;
    };

    struct buffer {
      void* v;
    };

    struct cbuffer {
      const void* v;
    };

    struct count {
      std::size_t v;
    };

    template<typename T>
    concept OpArg = requires(T t) { t.v; };

  } // namespace arg

  export enum class OpType {
    kRead,
    kWrite,
  };

  export template<OpType type, arg::OpArg... Bases>
  struct Operation : Bases... {
    static constexpr OpType op_type = type;
  };

  export template<OpType type, arg::OpArg... Args>
  auto BuildOp(Args&&... args) noexcept {
    return Operation<type, Args...>{std::forward<Args>(args)...};
  }

  export template<arg::OpArg Arg>
  auto GetArg(auto&& operation) {
    return static_cast<Arg>(operation).v;
  }

} // namespace async
