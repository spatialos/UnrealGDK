// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKSettings.h"

USpatialGDKSettings::USpatialGDKSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EntityPoolInitialReservationCount(3000)
	, EntityPoolRefreshThreshold(1000)
	, EntityPoolRefreshCount(2000)
	, HeartbeatIntervalSeconds(2.0f)
	, HeartbeatTimeoutSeconds(10.0f)
	, PingIntervalSeconds(0.5f)
	, PingTimeoutSeconds(1.5f)
	, ActorReplicationRateLimit(0)
{
}

FString USpatialGDKSettings::ToString()
{
	TArray<FStringFormatArg> Args;
	Args.Add(EntityPoolInitialReservationCount);
	Args.Add(EntityPoolRefreshThreshold);
	Args.Add(EntityPoolRefreshCount);
	Args.Add(HeartbeatIntervalSeconds);
	Args.Add(HeartbeatTimeoutSeconds);
	Args.Add(PingIntervalSeconds);
	Args.Add(PingTimeoutSeconds);
	Args.Add(ActorReplicationRateLimit);

	return FString::Format(TEXT(
		"EntityPoolInitialReservationCount={0}, "
		"EntityPoolRefreshThreshold={1}, "
		"EntityPoolRefreshCount={2}, "
		"HeartbeatIntervalSeconds={3}, "
		"HeartbeatTimeoutSeconds={4}, "
		"PingIntervalSeconds={5}, "
		"PingTimeoutSeconds={6}, "
		"ActorReplicationRateLimit={7}")
		, Args);
}
