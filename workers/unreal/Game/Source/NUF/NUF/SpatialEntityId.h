// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

// This is required because the clang toolchain defines `std::int64_t` as `long`, but UE4 defines `int64` as `long long`.
// Therefore, `TIsArithmetic<long>::Value` is false, and the template metaprogramming used to determine a key hash function
// breaks if a `std::int64_t` is a key for a TMap or TSet. We need this because `worker::EntityId` is aliased to `std::int64_t`.
//#ifdef PLATFORM_LINUX
template <> struct TIsArithmetic<unsigned long> { enum { Value = true }; };
template <> struct TIsArithmetic<long> { enum { Value = true }; };

inline uint32 GetTypeHash(const unsigned long A)
{
	return ::GetTypeHash((unsigned long long)A);
}

inline uint32 GetTypeHash(const long A)
{
	return ::GetTypeHash((long long)A);
}
//#endif
