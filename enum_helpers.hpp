#pragma once

#ifndef __ENUM_HELPERS_HPP
#define __ENUM_HELPERS_HPP

#include <type_traits>

template <typename E>
constexpr typename std::underlying_type<E>::type to_underlying(E e) noexcept {
    return static_cast<typename std::underlying_type<E>::type>(e);
}

template <typename E>
constexpr E from_underlying(typename std::underlying_type<E>::type e) noexcept {
    return static_cast<E>(e);
}

#endif
