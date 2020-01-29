// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogContainerMemoryTracker, Log, All);

class ContainerMemoryTracker
{
public:
	ContainerMemoryTracker(class USpatialNetDriver* InNetDriver);

	void Tick(float DeltaTime);

private:
	void CountMemory();

	template <template<typename...> typename ContainerType, typename... T>
	void CountContainer(const ContainerType<T...>& Container, FString Identifier, bool& bFirstLog);

	USpatialNetDriver* NetDriver;

	double LastCountMemoryTime = 0.0;
	double AccumulatedTime = 0.0;

	const double CountMemoryInterval = 1.0;

	TMap<FString, uint32> KnownAllocations;
};
