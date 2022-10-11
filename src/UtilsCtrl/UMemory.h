/***************************
@Author: MirrorYuChen
@Contact: 2458006366@qq.com
@File: UMemory.h
@Time: 2022/10/11 01:43 上午
@Desc:
***************************/

#ifndef CGRAPH_UMEMORY_H
#define CGRAPH_UMEMORY_H

#include <memory>
#include <type_traits>

#include "../CBasic/CBasicInclude.h"

CGRAPH_NAMESPACE_BEGIN

template<bool B, typename T = void>
using enable_if_t = typename std::enable_if<B, T>::type;

template<typename T>
struct MakeUniqueResult {
    using scalar = std::unique_ptr<T>;
};

template<typename T>
struct MakeUniqueResult<T[]> {
    using array = std::unique_ptr<T[]>;
};

template<typename T, size_t N>
struct MakeUniqueResult<T[N]> {
    using invalid = void;
};

template<typename T, typename... Args>
typename MakeUniqueResult<T>::scalar make_unique(
        Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename T>
typename MakeUniqueResult<T>::array make_unique(size_t n) {
    return std::unique_ptr<T>(new typename std::remove_extent<T>[n]());
}

template<typename T, typename... Args>
typename MakeUniqueResult<T>::invalid make_unique(
        Args &&... /* args */) = delete;

CGRAPH_NAMESPACE_END

#endif // CGRAPH_UMEMORY_H
