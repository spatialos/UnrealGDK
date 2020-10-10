// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/c_worker.h>
#include <type_traits>

using FEntityId = int64;
using FComponentId = uint32;
using FRequestId = int64;
using FCommandIndex = uint32;
using FSpanId = Trace_SpanId;

// Helpers to ensure that aliases for worker sdk types do not go out of sync.
template <typename T, typename U>
constexpr bool IsEquivalentIntegralType()
{
	// Can not use unreal type traits as some of them don't have specialisations for all integral types.
	return std::is_integral<T>::value == std::is_integral<U>::value && sizeof(T) == sizeof(U)
		   && std::is_signed<T>::value == std::is_signed<U>::value;
}

static_assert(IsEquivalentIntegralType<FEntityId, FEntityId>(), "FEntityId and FEntityId are not equivalent.");
static_assert(IsEquivalentIntegralType<FComponentId, FComponentId>(), "FComponentId and FComponentId are not equivalent.");
static_assert(IsEquivalentIntegralType<FRequestId, FRequestId>(), "FRequestId and FRequestId are not equivalent.");
static_assert(IsEquivalentIntegralType<FCommandIndex, FCommandIndex>(), "FCommandIndex and FCommandIndex are not equivalent.");
