// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "EngineClasses/SpatialShadowActor.h"
#include "EngineClasses/SpatialNetDriverAuthorityDebugger.h"

DEFINE_LOG_CATEGORY(LogSpatialShadowActor);

void USpatialShadowActor::Init(const Worker_EntityId InEntityId, AActor* InActor)
{
	check(InEntityId != SpatialConstants::INVALID_ENTITY_ID);
	check(InActor != nullptr);
	check(IsValid(InActor));

	EntityId = InEntityId;
	ReplicatedPropertyHash = CreateHash(InActor);
	Actor = InActor;
}

void USpatialShadowActor::Update(const Worker_EntityId InEntityId, AActor* InActor)
{
	check(InActor == Actor);

	check(InEntityId != SpatialConstants::INVALID_ENTITY_ID);
	check(InEntityId == EntityId);
	check(InActor != nullptr);

	ReplicatedPropertyHash = CreateHash(InActor);
	Actor = InActor;
}

FString USpatialShadowActor::CreateHash(const AActor* InActor)
{
	FString LatestReplicatedPropertyHash;

	for (TFieldIterator<FProperty> PropIt(InActor->GetClass()); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;

		if (Property->HasAnyPropertyFlags(CPF_Net))
		{
			if (Property->HasAnyPropertyFlags(CPF_HasGetValueTypeHash))
			{
				LatestReplicatedPropertyHash +=
					FString::FromInt(Property->GetValueTypeHash(Property->ContainerPtrToValuePtr<void>(InActor, 0)));
			}
		}
	}

	return LatestReplicatedPropertyHash;
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

	if (ReplicatedPropertyHash.IsEmpty())
	{
		// Have not received the first update yet for this actor
		return;
	}

	FString LocalReplicatedPropertyHash = CreateHash(InActor);

	if (LocalReplicatedPropertyHash != ReplicatedPropertyHash)
	{
		UE_LOG(LogSpatialShadowActor, Error, TEXT("Changed actor without authority! %s %s"), *Actor->GetName(),
			   *InActor->GetClass()->GetName());

		// Store hash to avoid generating a duplicate error message
		ReplicatedPropertyHash = LocalReplicatedPropertyHash;
	}

	return;
}
