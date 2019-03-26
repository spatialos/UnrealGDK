// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKSettings.h"

USpatialGDKSettings::USpatialGDKSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, EntityPoolInitialReservationCount(3000)
	, EntityPoolRefreshThreshold(1000)
	, EntityPoolRefreshCount(2000)
	, HeartbeatIntervalSeconds(2.0f)
	, HeartbeatTimeoutSeconds(10.0f)
	, ActorReplicationRateLimit(0)
	, bUsingQBI(false)
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
	Args.Add(ActorReplicationRateLimit);
	Args.Add(bUsingQBI);

	return FString::Format(TEXT(
		"EntityPoolInitialReservationCount={0}, "
		"EntityPoolRefreshThreshold={1}, "
		"EntityPoolRefreshCount={2}, "
		"HeartbeatIntervalSeconds={3}, "
		"HeartbeatTimeoutSeconds={4}, "
		"ActorReplicationRateLimit={5}, "
		"bUsingQBI={6}")
		, Args);
}

#if WITH_EDITOR
// Add a pop-up to warn users to update their config upon changing the using QBI property.
void USpatialGDKSettings::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(USpatialGDKSettings, bUsingQBI))
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::FromString(FString::Printf(TEXT("If you are not using auto-generated launch config, you must make sure to set the value of the \"enable_chunk_interest\" field to \"%s\" in your launch configuration for this to work. (You can check what launch configuration you are using in the SpatialOS GDK for Unreal Editor Settings.)"),
				bUsingQBI ? TEXT("false") : TEXT("true"))));
	}
	Super::PostEditChangeProperty(e);
}
#endif
