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
