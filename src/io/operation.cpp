module;

#include <utility>

#include <cstdint>

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

  template<arg::OpArg... Args>
  struct TOpDescr : Args... {};

  template<typename T>
  struct IsOpDescr : std::false_type {};

  template<typename... Args>
  struct IsOpDescr<TOpDescr<Args...>> : std::true_type {};

  export template<typename T>
  concept OpDescr = IsOpDescr<std::remove_cvref_t<T>>::value;

  export template<typename... Args>
  auto BuildOpDescr(Args&&... args) noexcept {
    return TOpDescr<Args...>{std::forward<Args>(args)...};
  }

  export template<arg::OpArg Arg, OpDescr Op>
  auto GetArg(Op&& op) {
    return static_cast<Arg>(op).v;
  }

  export enum class OpType {
    kRead,
    kWrite,
  };

  export struct OpResult {
    int result;
    uint64_t user_data;
  };

} // namespace async
