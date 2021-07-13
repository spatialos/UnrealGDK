// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#pragma optimize("", off)
#include "EngineClasses/SpatialShadowActor.h"

DEFINE_LOG_CATEGORY(LogSpatialShadowActor);

USpatialShadowActor::USpatialShadowActor(const FObjectInitializer& ObjectInitializer /*= FObjectInitializer::Get()*/)
	: Super(ObjectInitializer)
	, EntityId(SpatialConstants::INVALID_ENTITY_ID)
	, ReplicatedPropertyHash("")

{
}

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

FString USpatialShadowActor::CreateHash(AActor* InActor)
{
	FString LatestReplicatedPropertyHash;

	if (InActor->GetName().Contains(TEXT("ReplicatedTestActor")))
	{
		UE_LOG(LogSpatialShadowActor, Display, TEXT("ReplicatedTestActor %s"), *InActor->GetName());
	}

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

void USpatialShadowActor::CheckUnauthorisedDataChanges(const Worker_EntityId InEntityId, AActor* InActor)
{
	check(InActor == Actor);

	check(InEntityId != SpatialConstants::INVALID_ENTITY_ID);
	check(InEntityId == EntityId);
	check(InActor != nullptr);
	check(!InActor->HasAuthority());

	if (ReplicatedPropertyHash.IsEmpty())
	{
		// Have not received the first update yet for this actor
		return;
	}

	if (Actor->IsPendingKillOrUnreachable())
	{
		// Don't need to do anything, because it should have already been logged.
		return;
	}

	FString LocalReplicatedPropertyHash = CreateHash(InActor);

	if (LocalReplicatedPropertyHash != ReplicatedPropertyHash)
	{
		UE_LOG(LogSpatialShadowActor, Error, TEXT("Changed actor without authority! %s"), *Actor->GetName());

		// Store hash to avoid generating a duplicate error message
		ReplicatedPropertyHash = LocalReplicatedPropertyHash;
	}

	return;
}

#pragma optimize("", on)
