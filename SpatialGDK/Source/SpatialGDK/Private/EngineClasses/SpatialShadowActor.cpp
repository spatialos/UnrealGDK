// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "EngineClasses/SpatialShadowActor.h"
#include "EngineClasses/SpatialNetDriverAuthorityDebugger.h"

DEFINE_LOG_CATEGORY(LogSpatialShadowActor);

void USpatialShadowActor::Init(AActor& InActor)
{
	NumReplicatedProperties = 0;

	for (TFieldIterator<FProperty> PropIt(InActor.GetClass()); PropIt; ++PropIt)
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

	Actor = &InActor;
	CreateHash();

}

void USpatialShadowActor::Update()
{
	if (!IsValid(Actor) || Actor->IsPendingKillOrUnreachable())
	{
		return;
	}

	CreateHash();
}

void USpatialShadowActor::CreateHash()
{
	int32 i = 0;
	// Store a hashed value for each replicated property
	for (TFieldIterator<FProperty> PropIt(Actor->GetClass()); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;

		if (Property->HasAnyPropertyFlags(CPF_Net) && Property->HasAnyPropertyFlags(CPF_HasGetValueTypeHash))
		{
			ReplicatedPropertyHashes[i] = Property->GetValueTypeHash(Property->ContainerPtrToValuePtr<void>(Actor, 0));
			i++;
		}
	}
}

void USpatialShadowActor::CheckUnauthorisedDataChanges()
{
	if (!IsValid(Actor) || Actor->IsPendingKillOrUnreachable() || !Actor->HasAuthority())
	{
		return;
	}

	if (USpatialNetDriverAuthorityDebugger::IsSuppressedActor(*Actor))
	{
		// We are suppressing warnings about some actor classes to avoid spamming the user
		return;
	}

	// Compare hashed properties
	int32 i = 0;

	for (TFieldIterator<const FProperty> PropIt(Actor->GetClass()); PropIt; ++PropIt) 
	{
		const FProperty* Property = *PropIt;

		if (Property->HasAnyPropertyFlags(CPF_Net) && Property->HasAnyPropertyFlags(CPF_HasGetValueTypeHash))
		{
			const uint32 LatestPropertyHash = Property->GetValueTypeHash(Property->ContainerPtrToValuePtr<void>(Actor, 0));

			if (ReplicatedPropertyHashes[i] != LatestPropertyHash && !USpatialNetDriverAuthorityDebugger::IsSuppressedProperty(*Property))
			{
				UE_LOG(LogSpatialShadowActor, Error,
					   TEXT("Changed actor without authority with name %s of type %s, property changed without authority was %s!"),
					   *Actor->GetName(), *Actor->GetClass()->GetName(), *Property->GetName());

				// Store hash to avoid generating a duplicate error message
				ReplicatedPropertyHashes[i] = LatestPropertyHash;
			}
			i++;
		}
	}
}
