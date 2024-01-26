module;

#include "header.hpp"

export module Mod;

export template<typename T>
int foo(T t) {
  return f(t);
}
