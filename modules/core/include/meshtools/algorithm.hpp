#pragma once

#include <vector>

namespace meshtools {

template<template<typename...> class Cont = std::vector, class T, class Fn>
Cont<T> find(const Cont<T>& in, Fn&& fn) {
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