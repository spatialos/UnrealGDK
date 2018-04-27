// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SimpleEntitySpawnerBlock.h"

#include "AddComponentOpWrapperBase.h"
#include "CallbackDispatcher.h"
#include "EngineMinimal.h"
#include "EntityRegistry.h"
#include "MetadataAddComponentOp.h"
#include "MetadataComponent.h"
#include "PositionAddComponentOp.h"
#include "PositionComponent.h"
#include "SpatialGDKViewTypes.h"
#include "SpatialGDKWorkerTypes.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "improbable/view.h"

USimpleEntitySpawnerBlock::FSpatialOperation::FSpatialOperation(
	ESpatialOperationType OperationType,
	FComponentIdentifier ComponentIdentifer)
: OperationType(OperationType), ComponentIdentifier(ComponentIdentifer)
{
}

USimpleEntitySpawnerBlock::FSpatialOperation::FSpatialOperation(
	const worker::AddEntityOp& AddEntityOp)
{
	OperationType = ESpatialOperationType::AddEntity;
	ComponentIdentifier =
		FComponentIdentifier{AddEntityOp.EntityId, worker::ComponentId()};
}

USimpleEntitySpawnerBlock::FSpatialOperation::FSpatialOperation(
	const worker::RemoveEntityOp& RemoveEntityOp)
{
	OperationType = ESpatialOperationType::RemoveEntity;
	ComponentIdentifier =
		FComponentIdentifier{RemoveEntityOp.EntityId, worker::ComponentId()};
}

USimpleEntitySpawnerBlock::FSpatialOperation::FSpatialOperation(
	UAddComponentOpWrapperBase* AddComponentOp)
{
	OperationType = ESpatialOperationType::AddComponent;
	ComponentIdentifier = FComponentIdentifier{AddComponentOp->EntityId,
		AddComponentOp->ComponentId};
}

USimpleEntitySpawnerBlock::FSpatialOperation::FSpatialOperation(
	const worker::ComponentId ComponentId,
	const worker::RemoveComponentOp& RemoveComponentOp)
{
	OperationType = ESpatialOperationType::RemoveComponent;
	ComponentIdentifier =
		FComponentIdentifier{RemoveComponentOp.EntityId, ComponentId};
}

bool USimpleEntitySpawnerBlock::FSpatialOperation::
operator==(const FSpatialOperation& Other) const
{
	return (OperationType == Other.OperationType &&
		ComponentIdentifier == Other.ComponentIdentifier);
}

void USimpleEntitySpawnerBlock::Init(UEntityRegistry* Registry)
{
	EntityRegistry = Registry;
}

void USimpleEntitySpawnerBlock::AddEntity(
	const worker::AddEntityOp& AddEntityOp)
{
	QueueOp(FSpatialOperation{AddEntityOp});
}

void USimpleEntitySpawnerBlock::RemoveEntity(
	const worker::RemoveEntityOp& RemoveEntityOp)
{
	QueueOp(FSpatialOperation{RemoveEntityOp});
}

void USimpleEntitySpawnerBlock::AddComponent(
	UAddComponentOpWrapperBase* AddComponentOp)
{
	auto ComponentIdentifier = FComponentIdentifier{AddComponentOp->EntityId,
		AddComponentOp->ComponentId};
	FComponentAddOpQueueWrapper* QueueWrapper =
		ComponentsToAdd.Find(ComponentIdentifier);
	if (!QueueWrapper)
	{
		QueueWrapper = &ComponentsToAdd.Emplace(ComponentIdentifier,
			FComponentAddOpQueueWrapper());
	}
	QueueWrapper->Underlying.Add(AddComponentOp);
	QueueOp(FSpatialOperation{AddComponentOp});
}

void USimpleEntitySpawnerBlock::RemoveComponent(
	const worker::ComponentId ComponentId,
	const worker::RemoveComponentOp& RemoveComponentOp)
{
	QueueOp(FSpatialOperation{ComponentId, RemoveComponentOp});
}

void USimpleEntitySpawnerBlock::ChangeAuthority(
	const worker::ComponentId ComponentId,
	const worker::AuthorityChangeOp& AuthChangeOp)
{
	// Set the latest authority value for this Component on the owning entity
	ComponentAuthorities.Emplace(
		FComponentIdentifier{AuthChangeOp.EntityId, ComponentId}, AuthChangeOp);
}

void USimpleEntitySpawnerBlock::QueueOp(FSpatialOperation SpatialOperation)
{
	if (!QueuedOps.Contains(SpatialOperation.ComponentIdentifier.EntityId))
	{
		QueuedOps.Add(SpatialOperation.ComponentIdentifier.EntityId);
	}
	QueuedOps.Find(SpatialOperation.ComponentIdentifier.EntityId)
		->Add(SpatialOperation);
}

bool USimpleEntitySpawnerBlock::ProcessAddEntityOp(
	UWorld* World, const TWeakPtr<SpatialOSConnection>& InConnection, const FSpatialOperation& SpatialOperation)
{
	TArray<FSpatialOperation>* EntityOpsQueue =
		QueuedOps.Find(SpatialOperation.ComponentIdentifier.EntityId);

	auto PositionIdentifier =
		FComponentIdentifier{SpatialOperation.ComponentIdentifier.EntityId,
			UPositionComponent::ComponentId};
	auto MetadataIdentifier =
		FComponentIdentifier{SpatialOperation.ComponentIdentifier.EntityId,
			UMetadataComponent::ComponentId};

	auto AddPositionComponentOp = FSpatialOperation{
		ESpatialOperationType::AddComponent, PositionIdentifier};
	auto AddMetadataComponentOp = FSpatialOperation{
		ESpatialOperationType::AddComponent, MetadataIdentifier};

	if (EntityOpsQueue->Contains(AddPositionComponentOp) &&
		EntityOpsQueue->Contains(AddMetadataComponentOp))
	{
		// Get pointers to the add component ops queues.
		FComponentAddOpQueueWrapper* PositionBaseComponentQueue =
			ComponentsToAdd.Find(PositionIdentifier);
		FComponentAddOpQueueWrapper* MetadataBaseComponentQueue =
			ComponentsToAdd.Find(MetadataIdentifier);

		if (PositionBaseComponentQueue != nullptr &&
			MetadataBaseComponentQueue != nullptr)
		{
			UAddComponentOpWrapperBase** PositionBaseComponent =
				PositionBaseComponentQueue->Underlying.GetData();
			UAddComponentOpWrapperBase** MetadataBaseComponent =
				MetadataBaseComponentQueue->Underlying.GetData();

			// Only spawn entities for which we have received Position and Metadata
			// components
			if ((PositionBaseComponent &&
					(*PositionBaseComponent)->IsValidLowLevel()) &&
				(MetadataBaseComponent &&
					(*MetadataBaseComponent)->IsValidLowLevel()))
			{
				// Retrieve the EntityType string from the Metadata component
				UMetadataAddComponentOp* MetadataAddComponentOp =
					Cast<UMetadataAddComponentOp>(*MetadataBaseComponent);
				UPositionAddComponentOp* PositionAddComponentOp =
					Cast<UPositionAddComponentOp>(*PositionBaseComponent);

				// Create the actor
				AActor* EntityActor = SpawnNewEntity(MetadataAddComponentOp,
					PositionAddComponentOp,
					World);
				EntityRegistry->AddToRegistry(
					SpatialOperation.ComponentIdentifier.EntityId, EntityActor);
				SetupComponentInterests(EntityActor,
					SpatialOperation.ComponentIdentifier.EntityId,
					InConnection);

				return true;
			}
		}
	}
	return false;
}

bool USimpleEntitySpawnerBlock::ProcessAddComponentOp(
	const TWeakPtr<SpatialOSView>& InView,
	const TWeakPtr<SpatialOSConnection>& InConnection,
	UCallbackDispatcher* InCallbackDispatcher,
	const FSpatialOperation& SpatialOperation)
{
	AActor* Actor = EntityRegistry->GetActorFromEntityId(
		SpatialOperation.ComponentIdentifier.EntityId);
	if (Actor)
	{
		auto ComponentClass =
			KnownComponents.Find(SpatialOperation.ComponentIdentifier.ComponentId);
		if (!ComponentClass)
		{
			return true;
		}

		USpatialOsComponent* Component =
			Cast<USpatialOsComponent>(Actor->GetComponentByClass(*ComponentClass));
		if (!Component)
		{
			return true;
		}

		FComponentAddOpQueueWrapper* AddComponentOpQueue =
			ComponentsToAdd.Find(SpatialOperation.ComponentIdentifier);
		check(AddComponentOpQueue);
		UAddComponentOpWrapperBase** ComponentBase =
			AddComponentOpQueue->Underlying.GetData();
		Component->ApplyInitialState(**ComponentBase);
		AddComponentOpQueue->Underlying.RemoveAt(0);

		Component->Init(InConnection, InView, SpatialOperation.ComponentIdentifier.EntityId, InCallbackDispatcher);

		auto QueuedAuthChangeOp =
			ComponentAuthorities.Find(SpatialOperation.ComponentIdentifier);
		if (QueuedAuthChangeOp)
		{
			Component->ApplyInitialAuthority(*QueuedAuthChangeOp);
			ComponentAuthorities.Remove(SpatialOperation.ComponentIdentifier);
		}

		EntityRegistry->RegisterComponent(Actor, Component);
	}
	return true;
}

bool USimpleEntitySpawnerBlock::ProcessRemoveComponentOp(
	UCallbackDispatcher* InCallbackDispatcher,
	const FSpatialOperation& SpatialOperation)
{
	AActor* Actor = EntityRegistry->GetActorFromEntityId(
		SpatialOperation.ComponentIdentifier.EntityId);
	auto ComponentClass =
		KnownComponents.Find(SpatialOperation.ComponentIdentifier.ComponentId);

	if (!Actor || !ComponentClass)
	{
		return true;
	}

	USpatialOsComponent* Component =
		Cast<USpatialOsComponent>(Actor->GetComponentByClass(*ComponentClass));

	if (Component)
	{
		Component->Disable(SpatialOperation.ComponentIdentifier.EntityId,
			InCallbackDispatcher);
	}

	EntityRegistry->UnregisterComponent(Actor, Component);

	return true;
}

bool USimpleEntitySpawnerBlock::ProcessRemoveEntityOp(
	UWorld* World, const FSpatialOperation& SpatialOperation)
{
	auto ActorToRemove = EntityRegistry->GetActorFromEntityId(
		SpatialOperation.ComponentIdentifier.EntityId);
	if (ActorToRemove && !ActorToRemove->IsPendingKill() && World)
	{
		EntityRegistry->RemoveFromRegistry(ActorToRemove);
		World->DestroyActor(ActorToRemove);
	}

	return true;
}

void USimpleEntitySpawnerBlock::ProcessOps(
	const TWeakPtr<SpatialOSView>& InView,
	const TWeakPtr<SpatialOSConnection>& InConnection,
	UWorld* World,
	UCallbackDispatcher* InCallbackDispatcher)
{
	for (auto& EntityOps : QueuedOps)
	{
		uint32 NumOpsSuccessfullyCompleted = 0;
		for (FSpatialOperation SpatialOperation : EntityOps.Value)
		{
			if (!ProcessOp(InView, InConnection, World, InCallbackDispatcher, SpatialOperation))
			{
				break;
			}
			++NumOpsSuccessfullyCompleted;
		}

		EntityOps.Value.RemoveAt(0, NumOpsSuccessfullyCompleted);

		if (EntityOps.Value.Num() == 0)
		{
			EmptyOpsQueues.Add(EntityOps.Key);
		}
	}

	for (auto& EntityId : EmptyOpsQueues)
	{
		QueuedOps.Remove(EntityId);
	}
	EmptyOpsQueues.Reset();
}

bool USimpleEntitySpawnerBlock::ProcessOp(
	const TWeakPtr<SpatialOSView>& InView,
	const TWeakPtr<SpatialOSConnection>& InConnection,
	UWorld* World,
	UCallbackDispatcher* InCallbackDispatcher,
	FSpatialOperation SpatialOperation)
{
	switch (SpatialOperation.OperationType)
	{
		case ESpatialOperationType::AddEntity:
			return ProcessAddEntityOp(World, InConnection, SpatialOperation);
		case ESpatialOperationType::AddComponent:
			return ProcessAddComponentOp(InView, InConnection, InCallbackDispatcher, SpatialOperation);
		case ESpatialOperationType::RemoveComponent:
			return ProcessRemoveComponentOp(InCallbackDispatcher, SpatialOperation);
		case ESpatialOperationType::RemoveEntity:
			return ProcessRemoveEntityOp(World, SpatialOperation);
		default:
			return false;
	}
}

AActor* USimpleEntitySpawnerBlock::SpawnNewEntity(
	UMetadataAddComponentOp* MetadataComponent,
	UPositionAddComponentOp* PositionComponent,
	UWorld* World)
{
	FString EntityTypeString =
		UTF8_TO_TCHAR(MetadataComponent->Data->entity_type().c_str());

	// Attempt to find the class that has been registered to the EntityType string
	UClass** EntityClassTemplate =
		EntityRegistry->GetRegisteredEntityClass(EntityTypeString);

	// EntityClassTemplate is valid iff the EntityRegistry contains an entry
	// corresponding to
	// EntityType
	if (World)
	{
		if (EntityClassTemplate)
		{
			auto Coords = PositionComponent->Data->coords();
			FVector InitialTransform = USpatialOSConversionFunctionLibrary::
				SpatialOsCoordinatesToUnrealCoordinates(
					FVector(Coords.x(), Coords.y(), Coords.z()));

			auto NewActor = World->SpawnActor<AActor>(
				*EntityClassTemplate, InitialTransform, FRotator::ZeroRotator, FActorSpawnParameters());

			TArray<UActorComponent*> SpatialOSComponents =
				NewActor->GetComponentsByClass(USpatialOsComponent::StaticClass());

			for (auto Component : SpatialOSComponents)
			{
				USpatialOsComponent* SpatialOSComponent =
					Cast<USpatialOsComponent>(Component);
				KnownComponents.Emplace(
					SpatialOSComponent->GetComponentId().ToSpatialComponentId(),
					Component->GetClass());
			}

			return NewActor;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("SpawnNewEntity() failed: couldn't find EntityType(`%s`)"), *EntityTypeString);

			return nullptr;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnNewEntity() failed: invalid World context"));

		return nullptr;
	}
}

void USimpleEntitySpawnerBlock::SetupComponentInterests(
	AActor* Actor, const FEntityId& EntityId, const TWeakPtr<SpatialOSConnection>& Connection)
{
	TArray<UActorComponent*> SpatialOSComponents =
		Actor->GetComponentsByClass(USpatialOsComponent::StaticClass());

	worker::Map<worker::ComponentId, worker::InterestOverride>
		ComponentIdsAndInterestOverrides;

	for (auto Component : SpatialOSComponents)
	{
		USpatialOsComponent* SpatialOsComponent =
			Cast<USpatialOsComponent>(Component);
		ComponentIdsAndInterestOverrides.emplace(std::make_pair(
			SpatialOsComponent->GetComponentId().ToSpatialComponentId(),
			worker::InterestOverride{/* IsInterested */ true}));
	}

	auto LockedConnection = Connection.Pin();
	if (LockedConnection.IsValid())
	{
		LockedConnection->SendComponentInterest(EntityId.ToSpatialEntityId(),
			ComponentIdsAndInterestOverrides);
	}
}
