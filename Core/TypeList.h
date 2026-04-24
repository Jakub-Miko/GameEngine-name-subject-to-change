#pragma once
#include "TypeList.h"
#include "TypeList.h"

template<typename ... Types>
struct TypeList {};

template<typename List1, typename List2>
struct TypeListConcat {};

template<typename ... List1, typename ... List2>
struct TypeListConcat<TypeList<List1...>, TypeList<List2...>> {
    using type = TypeList<List1..., List2...>;
};

template<typename List>
struct TypeListSelection;

template<typename T>
struct TypeListSelection<TypeList<T>> {
    using first = T;
    using tail = TypeList<>;
};

template<typename T, typename ... Args>
struct TypeListSelection<TypeList<T, Args...>> {
    using first = T;
    using tail = TypeList<Args...>;
};

template<template<typename> typename Predicate, typename List>
struct TypeFilter;

template<template<typename> typename Predicate>
struct TypeFilter<Predicate, TypeList<>> {
    using type = TypeList<>;
};

template<template<typename> typename Predicate, typename ... Args>
struct TypeFilter<Predicate, TypeList<Args...>> {
    using first = typename TypeListSelection<TypeList<Args...>>::first;
    using tail = typename TypeFilter<Predicate, typename TypeListSelection<TypeList<Args...>>::tail>::type;
    using type = std::conditional_t<Predicate<first>::value, typename TypeListConcat<TypeList<first>, tail>::type, tail>;
};

