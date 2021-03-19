// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "SpatialView/ComponentData.h"
#include "SpatialView/ComponentUpdate.h"

#include "CustomPersistenceComponent.generated.h"

UCLASS(ClassGroup = (SpatialGDK), meta = (BlueprintSpawnableComponent), SpatialType = NotPersistent)
class SPATIALGDK_API UCustomPersistenceComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCustomPersistenceComponent();

	virtual Worker_ComponentId GetComponentId() const { return (Worker_ComponentId)0; } // todo use invalid component id constant

	virtual void BeginPlay() override; // TODO remove, only here for some debugging atm
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	virtual void GetAddComponentData(SpatialGDK::ComponentData& Data);
	virtual void GetComponentUpdate(SpatialGDK::ComponentUpdate& Update);

	virtual void OnAuthorityGained() override;

	virtual void OnPersistenceDataAvailable(const SpatialGDK::ComponentData& Data);

private:
	UPROPERTY(Handover)
	bool bHasProvidedPersistenceData;
};
