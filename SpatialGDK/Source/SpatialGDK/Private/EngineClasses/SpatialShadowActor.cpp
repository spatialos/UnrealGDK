// Copyright (c) Improbable Worlds Ltd, All Rights Reserved
#include "EngineClasses/SpatialShadowActor.h"
#include "EngineClasses/SpatialNetDriverAuthorityDebugger.h"

DEFINE_LOG_CATEGORY(LogSpatialShadowActor);

void USpatialShadowActor::Init(AActor& InActor)
{
	NumReplicatedProperties = 0;
	bIsValidHash = false;

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
	bIsValidHash = true;
}

void USpatialShadowActor::CheckUnauthorisedDataChanges(const ENetMode NetMode)
{
	if (IsPendingKillOrUnreachable() || !IsValid(Actor) || Actor->IsPendingKillOrUnreachable())
	{
		return;
	}

	if (Actor->HasAuthority())
	{
		// Invalidate the current hash as we are no longer receiving updates whilst we have authority - set back to true after we loose
		// authority and receive first update
		bIsValidHash = false;
		return;
	}

	if (!bIsValidHash)
	{
		return;
	}

	if (USpatialNetDriverAuthorityDebugger::IsSuppressedActor(*Actor))
	{
		// We are suppressing warnings about some actor classes to avoid spamming the user
		return;
	}

	TArray<FLifetimeProperty> LifetimeReplicatedProperties;
	Actor->GetClass()->GetDefaultObject()->GetLifetimeReplicatedProps(LifetimeReplicatedProperties);
	const bool bIsClient = NetMode == NM_Client;

	// Compare hashed properties
	int32 i = -1;

	for (TFieldIterator<const FProperty> PropIt(Actor->GetClass()); PropIt; ++PropIt)
	{
		const FProperty* Property = *PropIt;

		if (Property->HasAnyPropertyFlags(CPF_Net) && Property->HasAnyPropertyFlags(CPF_HasGetValueTypeHash))
		{
			// Use pre-increment to work around multiple exit paths
			++i;

			// Check this is a replicated property we care about
			const FLifetimeProperty* ReplicatedPropertyPtr =
				LifetimeReplicatedProperties.FindByPredicate([Property](const FLifetimeProperty& ReplicatedProperty) {
					return ReplicatedProperty.RepIndex == Property->RepIndex;
				});

			const bool bServerOnly =
				ReplicatedPropertyPtr->Condition == COND_ServerOnly || ReplicatedPropertyPtr->Condition == COND_AuthServerOnly;
			if (bServerOnly && bIsClient)
			{
				continue;
			}

			// If it's a pointer property need to check the object we are pointing to is valid before hashing - otherwise causes problems on
			// shutdown when objects are being deleted
			if (const FObjectProperty* ObjectProperty = CastField<FObjectProperty>(Property))
			{
				const UObject* ObjectPointer = ObjectProperty->GetObjectPropertyValue_InContainer(Actor, 0);

				if (!IsValid(ObjectPointer) || ObjectPointer->IsPendingKillOrUnreachable())
				{
					continue;
				}
			}

			const uint32 LatestPropertyHash = Property->GetValueTypeHash(Property->ContainerPtrToValuePtr<void>(Actor, 0));

			if (ReplicatedPropertyHashes[i] != LatestPropertyHash
				&& !USpatialNetDriverAuthorityDebugger::IsSuppressedProperty(*Property, *Actor))
			{
				UE_LOG(LogSpatialShadowActor, Error,
					   TEXT("Changed actor without authority with name %s of type %s, property changed without authority was %s!"),
					   *Actor->GetName(), *Actor->GetClass()->GetName(), *Property->GetName());

				// Store hash to avoid generating a duplicate error message
				ReplicatedPropertyHashes[i] = LatestPropertyHash;
			}
		}
	}
}
