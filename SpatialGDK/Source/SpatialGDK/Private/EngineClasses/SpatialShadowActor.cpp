// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "EngineClasses/SpatialShadowActor.h"
#include "EngineClasses/SpatialNetDriverAuthorityDebugger.h"

DEFINE_LOG_CATEGORY(LogSpatialShadowActor);

void USpatialShadowActor::Init(const Worker_EntityId InEntityId, AActor* InActor)
{
	check(InEntityId != SpatialConstants::INVALID_ENTITY_ID);
	check(InActor != nullptr);
	check(IsValid(InActor));

	NumReplicatedProperties = 0;

	for (TFieldIterator<FProperty> PropIt(InActor->GetClass()); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;

		if (Property->HasAnyPropertyFlags(CPF_Net) && Property->HasAnyPropertyFlags(CPF_HasGetValueTypeHash))
		{
			NumReplicatedProperties++;
		}
	}

	// Intialise the array
	ReplicatedPropertyHashes.Reserve(NumReplicatedProperties);
	for (int32 i = 0; i < NumReplicatedProperties; i++)
	{
		ReplicatedPropertyHashes.Add(0);
	}

	EntityId = InEntityId;
	CreateHash(InActor);
	Actor = InActor;
}

void USpatialShadowActor::Update(const Worker_EntityId InEntityId, AActor* InActor)
{
	check(InActor == Actor);

	check(InEntityId != SpatialConstants::INVALID_ENTITY_ID);
	check(InEntityId == EntityId);
	check(InActor != nullptr);

	CreateHash(InActor);
	Actor = InActor;
}

void USpatialShadowActor::CreateHash(const AActor* InActor)
{
	int32 i = 0;
	// Store a hashed value for each replicated property
	for (TFieldIterator<FProperty> PropIt(InActor->GetClass()); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;

		if (Property->HasAnyPropertyFlags(CPF_Net) && Property->HasAnyPropertyFlags(CPF_HasGetValueTypeHash))
		{
			ReplicatedPropertyHashes[i] = Property->GetValueTypeHash(Property->ContainerPtrToValuePtr<void>(InActor, 0));
			i++;
		}
	}
}

void USpatialShadowActor::CheckUnauthorisedDataChanges(const Worker_EntityId InEntityId, const AActor* InActor)
{
	check(InActor == Actor);

	check(InEntityId != SpatialConstants::INVALID_ENTITY_ID);
	check(InEntityId == EntityId);
	check(InActor != nullptr);
	check(!InActor->HasAuthority());

	if (USpatialNetDriverAuthorityDebugger::IsSuppressedActor(InActor))
	{
		// We are suppressing warnings about some actor classes to avoid spamming the user
		return;
	}

	// Compare hashed properties
	int32 i = 0;

	for (TFieldIterator<FProperty> PropIt(InActor->GetClass()); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;

		if (Property->HasAnyPropertyFlags(CPF_Net) && Property->HasAnyPropertyFlags(CPF_HasGetValueTypeHash))
		{
			uint32 LatestPropertyHash = Property->GetValueTypeHash(Property->ContainerPtrToValuePtr<void>(InActor, 0));

			if (ReplicatedPropertyHashes[i] != LatestPropertyHash && !USpatialNetDriverAuthorityDebugger::IsSuppressedProperty(Property))
			{
				UE_LOG(LogSpatialShadowActor, Error,
					   TEXT("Changed actor without authority with name %s of type %s, property changed without authority was %s!"),
					   *Actor->GetName(), *InActor->GetClass()->GetName(), *Property->GetName());

				// Store hash to avoid generating a duplicate error message
				ReplicatedPropertyHashes[i] = LatestPropertyHash;
			}

			i++;
		}
	}
}
