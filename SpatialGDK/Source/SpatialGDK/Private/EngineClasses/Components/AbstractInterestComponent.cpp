// NWX_BEGIN - https://improbableio.atlassian.net/browse/NWX-18843 - [IMPROVEMENT] Support for custom Interest components
// NWX_MORE - https://improbableio.atlassian.net/browse/NWX-19238 - [IMPROVEMENT] Move UpdateRequired logic to SpatialActorChannel
// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/Components/AbstractInterestComponent.h"

#include "Engine/World.h"
#include "EngineClasses/SpatialActorChannel.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "GameFramework/Actor.h"
#include "Utils/SpatialStatics.h"

void UAbstractInterestComponent::NotifyChannelUpdateRequired()
{
	AActor* Owner = GetOwner();
	check(Owner != nullptr && Owner->HasAuthority());
	check(USpatialStatics::IsSpatialNetworkingEnabled());

	UWorld* World = GetWorld();
	check(World != nullptr);

	USpatialNetDriver* SpatialNetDriver = Cast<USpatialNetDriver>(World->GetNetDriver());
	check(SpatialNetDriver != nullptr);

	USpatialActorChannel* Channel = SpatialNetDriver->GetOrCreateSpatialActorChannel(Owner);
	check(Channel != nullptr);

	Channel->SetIsInterestUpdateRequired(true);
}
