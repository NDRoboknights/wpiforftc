//===- llvm/Support/type_traits.h - Simplfied type traits -------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides useful additions to the standard type_traits library.
//
//===----------------------------------------------------------------------===//

#ifndef WPIUTIL_WPI_TYPE_TRAITS_H
#define WPIUTIL_WPI_TYPE_TRAITS_H

#include "wpi/Compiler.h"
#include <type_traits>
#include <utility>

#ifndef __has_feature
#define WPI_DEFINED_HAS_FEATURE
#define __has_feature(x) 0
#endif

namespace wpi {

/// isPodLike - This is a type trait that is used to determine whether a given
/// type can be copied around with memcpy instead of running ctors etc.
    template<typename T>
    struct isPodLike {
        // std::is_trivially_copyable is available in libc++ with clang, libstdc++
        // that comes with GCC 5.
#if (__has_feature(is_trivially_copyable) && defined(_LIBCPP_VERSION)) || \
    (defined(__GNUC__) && __GNUC__ >= 5)
        // If the compiler supports the is_trivially_copyable trait use it, as it
        // matches the definition of isPodLike closely.
        static const bool value = std::is_trivially_copyable<T>::value;
#elif __has_feature(is_trivially_copyable)
        // Use the internal name if the compiler supports is_trivially_copyable but we
        // don't know if the standard library does. This is the case for clang in
        // conjunction with libstdc++ from GCC 4.x.
        static const bool value = __is_trivially_copyable(T);
#else
        // If we don't know anything else, we can (at least) assume that all non-class
        // types are PODs.
        static const bool value = !std::is_class<T>::value;
#endif
    };

// std::pair's are pod-like if their elements are.
    template<typename T, typename U>
    struct isPodLike<std::pair < T, U>> {
    static const bool value = isPodLike<T>::value && isPodLike<U>::value;
};

/// Metafunction that determines whether the given type is either an
/// integral type or an enumeration type, including enum classes.
///
/// Note that this accepts potentially more integral types than is_integral
/// because it is based on being implicitly convertible to an integral type.
/// Also note that enum classes aren't implicitly convertible to integral types,
/// the value may therefore need to be explicitly converted before being used.
template<typename T>
class is_integral_or_enum {
    using UnderlyingT = typename std::remove_reference<T>::type;

public:
    static const bool value =
            !std::is_class<UnderlyingT>::value && // Filter conversion operators.
            !std::is_pointer<UnderlyingT>::value &&
            !std::is_floating_point<UnderlyingT>::value &&
            (std::is_enum<UnderlyingT>::value ||
             std::is_convertible<UnderlyingT, unsigned long long>::value);
};

/// If T is a pointer, just return it. If it is not, return T&.
template<typename T, typename Enable = void>
struct add_lvalue_reference_if_not_pointer {
    using type = T &;
};

template<typename T>
struct add_lvalue_reference_if_not_pointer<
        T, typename std::enable_if<std::is_pointer<T>::value>::type> {
    using type = T;
};

/// If T is a pointer to X, return a pointer to const X. If it is not,
/// return const T.
template<typename T, typename Enable = void>
struct add_const_past_pointer {
    using type = const T;
};

template<typename T>
struct add_const_past_pointer<
        T, typename std::enable_if<std::is_pointer<T>::value>::type> {
    using type = const typename std::remove_pointer<T>::type *;
};

template<typename T, typename Enable = void>
struct const_pointer_or_const_ref {
    using type = const T &;
};
template<typename T>
struct const_pointer_or_const_ref<
        T, typename std::enable_if<std::is_pointer<T>::value>::type> {
    using type = typename add_const_past_pointer<T>::type;
};

} // namespace wpi

// If the compiler supports detecting whether a class is final, define
// an LLVM_IS_FINAL macro. If it cannot be defined properly, this
// macro will be left undefined.
#ifndef LLVM_IS_FINAL
#if __cplusplus >= 201402L || defined(_MSC_VER)
#define LLVM_IS_FINAL(Ty) std::is_final<Ty>()
#elif __has_feature(is_final) || LLVM_GNUC_PREREQ(4, 7, 0)
#define LLVM_IS_FINAL(Ty) __is_final(Ty)
#endif
#endif

#ifdef WPI_DEFINED_HAS_FEATURE
#undef __has_feature
#undef WPI_DEFINED_HAS_FEATURE
#endif

#endif // LLVM_SUPPORT_TYPE_TRAITS_H
