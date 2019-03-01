// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKSettings.h"

USpatialGDKSettings::USpatialGDKSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EntityPoolInitialReservationCount(1000)
	, EntityPoolRefreshThreshold(100)
	, EntityPoolRefreshCount(500)
{
}

FString USpatialGDKSettings::ToString()
{
	TArray<FStringFormatArg> Args;
	Args.Add(EntityPoolInitialReservationCount);
	Args.Add(EntityPoolRefreshThreshold);
	Args.Add(EntityPoolRefreshCount);

	return FString::Format(TEXT(
		"EntityPoolInitialReservationCount={0}, "
		"EntityPoolRefreshThreshold={1}, "
		"EntityPoolRefreshCount={2}")
		, Args);
}
