// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetBitWriter.h"

#include "UObject/WeakObjectPtr.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"
#include "Utils/EntityPool.h"
#include "Utils/EntityRegistry.h"

FSpatialNetBitWriter::FSpatialNetBitWriter(USpatialNetDriver* InNetDriver, USpatialPackageMapClient* InPackageMap, TSet<TWeakObjectPtr<const UObject>>& InUnresolvedObjects)
	: NetDriver(InNetDriver)
	, FNetBitWriter(InPackageMap, 0)
	, UnresolvedObjects(InUnresolvedObjects)
{}

void FSpatialNetBitWriter::SerializeObjectRef(FUnrealObjectRef& ObjectRef)
{
	int64 EntityId = ObjectRef.Entity;
	*this << EntityId;
	*this << ObjectRef.Offset;

	uint8 HasPath = ObjectRef.Path.IsSet();
	SerializeBits(&HasPath, 1);
	if (HasPath)
	{
		*this << ObjectRef.Path.GetValue();
	}

	uint8 HasOuter = ObjectRef.Outer.IsSet();
	SerializeBits(&HasOuter, 1);
	if (HasOuter)
	{
		SerializeObjectRef(*ObjectRef.Outer);
	}
}

FArchive& FSpatialNetBitWriter::operator<<(UObject*& Value)
{
	FUnrealObjectRef ObjectRef;
	if (Value != nullptr && !Value->IsPendingKill())
	{
		auto PackageMapClient = Cast<USpatialPackageMapClient>(PackageMap);
		// How do we make sure we don't assign entity ids to non-replicated actors?
		// This is one of the places we want to assign an entity id to something that needs it (if we're a server authoritative over that actor)
		FNetworkGUID NetGUID = PackageMapClient->GetNetGUIDFromObject(Value);
		if (!NetGUID.IsValid())
		{
			if (Value->IsFullNameStableForNetworking())
			{
				NetGUID = PackageMapClient->ResolveStablyNamedObject(Value);
			}
			else if (NetDriver->IsServer()) // We want to assign an entity id to this if we're a server authoritative over the actor and it doesn't have an entity id yet.
			{
				UEntityRegistry* EntityRegistry = NetDriver->GetEntityRegistry();

				if (Value->IsA<AActor>())
				{
					// resolve this actor
					AActor* Actor = Cast<AActor>(Value);
					if (Actor->Role == ROLE_Authority && EntityRegistry->GetEntityIdFromActor(Actor) == 0)
					{
						NetDriver->SetupActorEntity(Actor);
					}
				}
				else if (Value->GetOuter()->IsA<AActor>())
				{
					// resolve outer

					AActor* OuterActor = Cast<AActor>(Value->GetOuter());
					if (OuterActor->Role == ROLE_Authority && EntityRegistry->GetEntityIdFromActor(OuterActor) == 0)
					{
						NetDriver->SetupActorEntity(OuterActor);
					}
				}
				// If we are a server authoritative over the actor, assign an entity ID and resolve in package map
			}
		}
		ObjectRef = FUnrealObjectRef(PackageMapClient->GetUnrealObjectRefFromNetGUID(NetGUID));
		if (ObjectRef == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
		{
			UnresolvedObjects.Add(Value);
			ObjectRef = FUnrealObjectRef::NULL_OBJECT_REF;
		}
	}
	else
	{
		ObjectRef = FUnrealObjectRef::NULL_OBJECT_REF;
	}

	SerializeObjectRef(ObjectRef);

	return *this;
}

FArchive& FSpatialNetBitWriter::operator<<(FWeakObjectPtr& Value)
{
	UObject* Object = Value.Get(true);
	*this << Object;

	return *this;
}
