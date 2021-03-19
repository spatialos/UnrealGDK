// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/Components/CustomPersistenceComponent.h"

#include "EngineClasses/SpatialGameInstance.h"
#include "Interop/Connection/SpatialWorkerConnection.h"
#include "Utils/SpatialStatics.h"

// Sets default values for this component's properties
UCustomPersistenceComponent::UCustomPersistenceComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bWantsInitializeComponent = true;

	bHasProvidedPersistenceData = false;

	// Needed for now, both to replicate the bHasProvidedPersistenceData bool, and to get PreReplication callbacks, during which we pull in
	// the data from the actor. The latter is not a hard requirement (the actor could also be required to push its data to us)
	SetIsReplicatedByDefault(true);
}

void UCustomPersistenceComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("UCustomPersistenceComponent, %s, bHasProvidedPersistenceData during BeginPlay: %d"), *GetName(),
		   bHasProvidedPersistenceData);
}

void UCustomPersistenceComponent::OnAuthorityGained()
{
	UE_LOG(LogTemp, Log, TEXT("UCustomPersistenceComponent, %s, OnAuthorityGained, bHasProvidedPersistenceData %d"), *GetName(),
		   bHasProvidedPersistenceData);

	if (bHasProvidedPersistenceData)
	{
		UE_LOG(LogTemp, Log, TEXT("UCustomPersistenceComponent, OnAuthorityGained, but already provided persistence data previously"));
		return;
	}
	bHasProvidedPersistenceData = true;

	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UCustomPersistenceComponent, didn't have an owner actor during InternalOnPersistenceDataAvailable"));
		return;
	}

	if (!Owner->HasAuthority())
	{
		UE_LOG(LogTemp, Log, TEXT("UCustomPersistenceComponent, owner doesn't have authority"));
		return;
	}

	const uint64 EntityID = USpatialStatics::GetActorEntityId(Owner);
	if (EntityID == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCustomPersistenceComponent, Don't have an entity ID in persistence callback."));
		return;
	}

	const USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
	if (NetDriver == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UCustomPersistenceComponent, Got persistence data callback but can't find a spatial net driver."));
		return;
	}

	SpatialGDK::ViewCoordinator& Coordinator = NetDriver->Connection->GetCoordinator();

	if (!Coordinator.HasEntity(EntityID))
	{
		UE_LOG(LogTemp, Warning,
			   TEXT("UCustomPersistenceComponent, View coordinator doesn't have entity %llu during persistence callback."), EntityID);
		return;
	}

	const SpatialGDK::EntityView& View = Coordinator.GetView();
	const SpatialGDK::EntityViewElement* ViewData = View.Find(EntityID);
	if (ViewData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UCustomPersistenceComponent, Found no persistence data for entity %llu"), EntityID);
		return;
	}

	bool bFoundComponent = false;
	for (const auto& ComponentData : ViewData->Components)
	{
		UE_LOG(LogTemp, Log, TEXT("UCustomPersistenceComponent, saw component data for ID %u"), ComponentData.GetComponentId());
		if (ComponentData.GetComponentId() == GetComponentId())
		{
			UE_LOG(LogTemp, Log, TEXT("UCustomPersistenceComponent, calling user-facing callback. Server: %d, NetDriver: %s"), GIsServer,
				   *NetDriver->GetName());
			OnPersistenceDataAvailable(ComponentData);
			bFoundComponent = true;
			break;
		}
	}

	if (!bFoundComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("UCustomPersistenceComponent, Couldn't find the persistence component for entity %llu"), EntityID);
	}
}

void UCustomPersistenceComponent::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	UpdateDeepCopy();
}

void UCustomPersistenceComponent::UpdateDeepCopy()
{
	// Work with spatial turned off
	if (!USpatialStatics::IsSpatialNetworkingEnabled())
	{
		UE_LOG(LogTemp, Log, TEXT("UCustomPersistenceComponent, not using spatial networking"));
		return;
	}

	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UCustomPersistenceComponent, didn't have an owner actor during PreReplication"));
		return;
	}

	if (!IsValid(Owner))
	{
		UE_LOG(LogTemp, Error, TEXT("UCustomPersistenceComponent, Owner is not yet valid, data loading may fail"));
		return;
	}

	const uint64 EntityID = USpatialStatics::GetActorEntityId(Owner);
	if (EntityID == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCustomPersistenceComponent, didn't have an entity ID in PreReplication."));
		return;
	}

	const USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
	if (NetDriver == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UCustomPersistenceComponent, running with spatial but can't find a spatial net driver."));
		return;
	}

	if (!NetDriver->Connection->GetCoordinator().HasEntity(EntityID))
	{
		UE_LOG(LogTemp, Warning, TEXT("UCustomPersistenceComponent, View coordinator doesn't have entity %llu yet."), EntityID);
		return;
	}

	// Will have to see if this ComponentUpdate type makes sense to be user-facing.
	SpatialGDK::ComponentUpdate Update(GetComponentId());
	GetComponentUpdate(Update);
	NetDriver->Connection->GetCoordinator().SendComponentUpdate(
		EntityID, Update.DeepCopy(), {}); // This deep copy is probably not the right way to do this. Works for now though.
}

void UCustomPersistenceComponent::GetAddComponentData(SpatialGDK::ComponentData& Data) {}

void UCustomPersistenceComponent::GetComponentUpdate(SpatialGDK::ComponentUpdate& Update) {}

void UCustomPersistenceComponent::OnPersistenceDataAvailable(const SpatialGDK::ComponentData& Data) {}
