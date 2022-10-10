/*
 * @Author: chenjingyu
 * @Date: 2022-10-11 01:43:58
 * @LastEditTime: 2022-10-11 01:59:50
 * @Description: Memory operator
 * @FilePath: \CThreadPool\src\UtilsCtrl\UMemory.h
 */
#ifndef CGRAPH_UMEMORY_H
#define CGRAPH_UMEMORY_H

#include <memory.h>
#include <type_traits>
#include "../CBasic/CBasicInclude.h"

CGRAPH_NAMESPACE_BEGIN

template <bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template <typename T>
std::unique_ptr<T> WrapUnique(T* ptr) {
  static_assert(!std::is_array<T>::value, "array types are unsupported");
  static_assert(std::is_object<T>::value, "non-object types are unsupported");
  return std::unique_ptr<T>(ptr);
}

namespace memory_internal {

// Traits to select proper overload and return type for `absl::make_unique<>`.
template <typename T>
struct MakeUniqueResult {
  using scalar = std::unique_ptr<T>;
};
template <typename T>
struct MakeUniqueResult<T[]> {
  using array = std::unique_ptr<T[]>;
};
template <typename T, size_t N>
struct MakeUniqueResult<T[N]> {
  using invalid = void;
};

}  // namespace memory_internal

// gcc 4.8 has __cplusplus at 201301 but the libstdc++ shipped with it doesn't
// define make_unique.  Other supported compilers either just define __cplusplus
// as 201103 but have make_unique (msvc), or have make_unique whenever
// __cplusplus > 201103 (clang).
#if (__cplusplus > 201103L || defined(_MSC_VER)) && \
    !(defined(__GLIBCXX__) && !defined(__cpp_lib_make_unique))
using std::make_unique;
#else

template <typename T, typename... Args>
typename memory_internal::MakeUniqueResult<T>::scalar make_unique(
    Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
typename memory_internal::MakeUniqueResult<T>::array make_unique(size_t n) {
  return std::unique_ptr<T>(new typename absl::remove_extent_t<T>[n]());
}

template <typename T, typename... Args>
typename memory_internal::MakeUniqueResult<T>::invalid make_unique(
    Args&&... /* args */) = delete;
#endif

CGRAPH_NAMESPACE_END


#endif // CGRAPH_UMEMORY_H
