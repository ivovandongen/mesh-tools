#pragma once

#include <vector>

namespace meshtools {

template<template<typename...> class Cont = std::vector, class T, class Fn>
Cont<std::reference_wrapper<T>> find(Cont<T>& in, Fn&& fn) {
    Cont<std::reference_wrapper<T>> result;
    for (auto it = in.begin(); it != in.end(); it++) {
        if (fn(*it)) {
            result.emplace_back(*it);
        }
    }
    return result;
}

template<template<typename...> class Cont = std::vector, class T, class Fn>
Cont<std::reference_wrapper<const T>> find(const Cont<T>& in, Fn&& fn) {
    Cont<std::reference_wrapper<const T>> result;
    for (auto it = in.begin(); it != in.end(); it++) {
        if (fn(*it)) {
            result.emplace_back(*it);
        }
    }
    return result;
}

template<template<typename...> class Cont = std::vector, class T, class Fn>
Cont<T> copy(const Cont<T>& in, Fn&& fn) {
    Cont<T> result;
    std::copy_if(in.begin(), in.end(), std::back_inserter(result), fn);
    return result;
}

template<template<typename...> class Cont = std::vector, class T, class Fn>
Cont<size_t> find_indices(const T& in, Fn&& fn) {
    Cont<size_t> result;
    for (auto it = in.begin(); it != in.end(); it++) {
        if (fn(*it)) {
            result.push_back(it - in.begin());
        }
    }
    return result;
}

template<class O, template<typename...> class OutCont = std::vector, class T, class Fn>
OutCont<O> transform(T& in, const Fn& uo) {
    OutCont<O> o{};
    o.reserve(in.size());
    std::transform(in.begin(), in.end(), std::back_inserter(o), uo);
    return o;
}

} // namespace meshtools