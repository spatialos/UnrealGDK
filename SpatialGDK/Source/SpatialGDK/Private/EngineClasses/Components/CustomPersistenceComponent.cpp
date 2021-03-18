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

	SetIsReplicatedByDefault(true);
}

void UCustomPersistenceComponent::InitializeComponent()
{
	Super::InitializeComponent();

	USpatialGameInstance* GameInstance = Cast<USpatialGameInstance>(GetWorld()->GetGameInstance());
	if (GameInstance != nullptr)
	{
		GameInstance->OnPersistenceDataAvailable.AddDynamic(this, &UCustomPersistenceComponent::InternalOnPersistenceDataAvailable);
	}
}

void UCustomPersistenceComponent::InternalOnPersistenceDataAvailable()
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UCustomPersistenceComponent, didn't have an owner actor during InternalOnPersistenceDataAvailable"));
		return;
	}

	if (!Owner->HasAuthority())
	{
		return;
	}

	const uint64 EntityID = USpatialStatics::GetActorEntityId(Owner);
	if (EntityID == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Don't have an entity ID in persistence callback."));
		return;
	}

	const USpatialNetDriver* NetDriver = Cast<USpatialNetDriver>(GetWorld()->GetNetDriver());
	if (NetDriver == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Got persistence data callback but can't find a spatial net driver."));
		return;
	}

	SpatialGDK::ViewCoordinator& Coordinator = NetDriver->Connection->GetCoordinator();

	if (!Coordinator.HasEntity(EntityID))
	{
		UE_LOG(LogTemp, Warning, TEXT("View coordinator doesn't have entity %llu during persistence callback."), EntityID);
		return;
	}

	const SpatialGDK::EntityView& View = Coordinator.GetView();
	const SpatialGDK::EntityViewElement* ViewData = View.Find(EntityID);
	if (ViewData == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Found no persistence data for entity %llu"), EntityID);
		return;
	}

	bool bFoundComponent = false;
	for (const auto& ComponentData : ViewData->Components)
	{
		if (ComponentData.GetComponentId() == GetComponentId())
		{
			OnPersistenceDataAvailable(ComponentData);
			bFoundComponent = true;
			break;
		}
	}

	if (!bFoundComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Couldn't find the persistence component for entity %llu"), EntityID);
	}
}

void UCustomPersistenceComponent::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	// Work with spatial turned off
	if (!USpatialStatics::IsSpatialNetworkingEnabled())
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UCustomPersistenceComponent, didn't have an owner actor during PreReplication"));
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
		UE_LOG(LogTemp, Warning, TEXT("View coordinator doesn't have entity %llu yet."), EntityID);
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
