// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <improbable/worker.h>

#include "CoreMinimal.h"
#include "Net/RepLayout.h"
#include "SpatialTypeBinding.generated.h"

class USpatialUpdateInterop;
class UPackageMap;

enum EReplicatedPropertyGroup
{
	GROUP_SingleClient,
	GROUP_MultiClient
};

inline EReplicatedPropertyGroup GetGroupFromCondition(ELifetimeCondition Condition)
{
	switch (Condition)
	{
	case COND_AutonomousOnly:
	case COND_OwnerOnly:
		return GROUP_SingleClient;
	default:
		return GROUP_MultiClient;
	}
}

//todo: (cc: @david) This code is merged into master, but the UObjects held inside a 
// non-uclass will not be ref counted properly and may be removed under us.
// It will mostly "work" because it's only used under USpatialUpdateInterop's context, but I think we can do something more robust.
UCLASS()
class NUF_API USpatialTypeBinding : public UObject
{
	GENERATED_BODY()
public:
	void Init(USpatialUpdateInterop* UpdateInterop, UPackageMap* PackageMap);

	virtual void BindToView() PURE_VIRTUAL(USpatialTypeBinding::BindToView, );
	virtual void UnbindFromView() PURE_VIRTUAL(USpatialTypeBinding::UnbindFromView, );
	virtual worker::ComponentId GetReplicatedGroupComponentId(EReplicatedPropertyGroup Group) const PURE_VIRTUAL(USpatialTypeBinding::GetReplicatedGroupComponentId, return worker::ComponentId{}; );
	virtual void SendComponentUpdates(const TArray<uint16>& Changed,
									  const uint8* RESTRICT SourceData,
									  const TArray<FRepLayoutCmd>& Cmds,
									  const TArray<FHandleToCmdIndex>& BaseHandleToCmdIndex,
									  const worker::EntityId& EntityId) const PURE_VIRTUAL(USpatialTypeBinding::SendComponentUpdates, );
	virtual worker::Entity CreateActorEntity(const FVector& Position, const FString& Metadata) const PURE_VIRTUAL(USpatialTypeBinding::CreateActorEntity, return worker::Entity{}; );

protected:
	UPROPERTY()
	USpatialUpdateInterop* UpdateInterop;

	UPROPERTY()
	UPackageMap* PackageMap;
};
