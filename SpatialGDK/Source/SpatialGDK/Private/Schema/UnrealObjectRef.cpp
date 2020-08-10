// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/UnrealObjectRef.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "SpatialConstants.h"
#include "Utils/SchemaUtils.h"
#include "Utils/SpatialDebugger.h"
#include "Utils/SpatialMetricsDisplay.h"

#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogUnrealObjectRef, Log, All);

const FUnrealObjectRef FUnrealObjectRef::NULL_OBJECT_REF = FUnrealObjectRef(SpatialConstants::INVALID_ENTITY_ID, 0);
const FUnrealObjectRef FUnrealObjectRef::UNRESOLVED_OBJECT_REF = FUnrealObjectRef(SpatialConstants::INVALID_ENTITY_ID, 1);

UObject* FUnrealObjectRef::ToObjectPtr(const FUnrealObjectRef& ObjectRef, USpatialPackageMapClient* PackageMap, bool& bOutUnresolved)
{
	if (ObjectRef == FUnrealObjectRef::NULL_OBJECT_REF)
	{
		return nullptr;
	}
	else
	{
		if (ObjectRef.bUseClassPathToLoadObject)
		{
			FUnrealObjectRef ClassRef = ObjectRef;
			ClassRef.bUseClassPathToLoadObject = false;

			UObject* Value = PackageMap->GetUniqueActorInstanceByClassRef(ClassRef);
			if (Value == nullptr)
			{
				// This could happen if we no longer spawn all of these Actors before starting replication.
				UE_LOG(LogUnrealObjectRef, Warning, TEXT("Could not load object reference by class path: %s"), **ClassRef.Path);
				bOutUnresolved = true;
			}
			return Value;
		}

		FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
		if (NetGUID.IsValid())
		{
			UObject* Value = PackageMap->GetObjectFromNetGUID(NetGUID, true);
			if (Value == nullptr)
			{
				// Check if the object we are looking for is in a package being loaded.
				if (PackageMap->IsGUIDPending(NetGUID))
				{
					PackageMap->PendingReferences.Add(NetGUID);
					bOutUnresolved = true;
					return nullptr;
				}

				// At this point, we're unable to resolve a stably-named actor by path. This likely means either the actor doesn't exist, or
				// it's part of a streaming level that hasn't been streamed in. Native Unreal networking sets reference to nullptr and
				// continues. So we do the same.
				FString FullPath;
				SpatialGDK::GetFullPathFromUnrealObjectReference(ObjectRef, FullPath);
				UE_LOG(LogUnrealObjectRef, Warning,
					   TEXT("Object ref did not map to valid object. Streaming level not loaded or actor deleted. Will be set to nullptr: "
							"%s %s"),
					   *ObjectRef.ToString(), FullPath.IsEmpty() ? TEXT("[NO PATH]") : *FullPath);
			}

			return Value;
		}
		else
		{
			bOutUnresolved = true;
			return nullptr;
		}
	}
}

FUnrealObjectRef FUnrealObjectRef::FromObjectPtr(UObject* ObjectValue, USpatialPackageMapClient* PackageMap)
{
	FUnrealObjectRef ObjectRef = FUnrealObjectRef::NULL_OBJECT_REF;

	if (ObjectValue != nullptr && !ObjectValue->IsPendingKill())
	{
		FNetworkGUID NetGUID;
		if (ObjectValue->IsSupportedForNetworking())
		{
			NetGUID = PackageMap->GetNetGUIDFromObject(ObjectValue);

			if (!NetGUID.IsValid())
			{
				if (ObjectValue->IsFullNameStableForNetworking())
				{
					NetGUID = PackageMap->ResolveStablyNamedObject(ObjectValue);
				}
				else
				{
					NetGUID = PackageMap->TryResolveObjectAsEntity(ObjectValue);
				}
			}
		}

		// The secondary part of the check is needed if we couldn't assign an entity id (e.g. ran out of entity ids)
		if (NetGUID.IsValid() || (ObjectValue->IsSupportedForNetworking() && !ObjectValue->IsFullNameStableForNetworking()))
		{
			ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
		}
		else
		{
			ObjectRef = FUnrealObjectRef::NULL_OBJECT_REF;
		}

		if (ObjectRef == FUnrealObjectRef::UNRESOLVED_OBJECT_REF)
		{
			// There are cases where something assigned a NetGUID without going through the FSpatialNetGUID (e.g. FObjectReplicator)
			// Assign an UnrealObjectRef by going through the FSpatialNetGUID flow
			if (ObjectValue->IsFullNameStableForNetworking())
			{
				PackageMap->ResolveStablyNamedObject(ObjectValue);
				ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			}
			else
			{
				// Check if the object is an actor or a subobject of an actor that is torn off or non-replicated.
				if (AActor* Actor = ObjectValue->IsA<AActor>() ? Cast<AActor>(ObjectValue) : ObjectValue->GetTypedOuter<AActor>())
				{
					if (Actor->GetTearOff() || !Actor->GetIsReplicated())
					{
						return FUnrealObjectRef::NULL_OBJECT_REF;
					}
				}

				// If this is an Actor that should exist once per Worker (e.g. GameMode, GameState) and hasn't been resolved yet,
				// send its class path instead.
				if (ShouldLoadObjectFromClassPath(ObjectValue))
				{
					ObjectRef = GetRefFromObjectClassPath(ObjectValue, PackageMap);
					if (ObjectRef.IsValid())
					{
						return ObjectRef;
					}
				}

				// Check if the object is a newly referenced dynamic subobject, in which case we can create the object ref if we have the
				// entity id of the parent actor.
				if (!ObjectValue->IsA<AActor>())
				{
					PackageMap->TryResolveNewDynamicSubobjectAndGetClassInfo(ObjectValue);
					ObjectRef = PackageMap->GetUnrealObjectRefFromObject(
						ObjectValue); // This should now be valid, as we resolve the object in the line before
					if (ObjectRef.IsValid())
					{
						return ObjectRef;
					}
				}

				// Unresolved object.
				UE_LOG(LogUnrealObjectRef, Warning, TEXT("FUnrealObjectRef::FromObjectPtr: ObjectValue is unresolved! %s"),
					   *ObjectValue->GetName());
				ObjectRef = FUnrealObjectRef::NULL_OBJECT_REF;
			}
		}
	}

	return ObjectRef;
}

FUnrealObjectRef FUnrealObjectRef::FromSoftObjectPath(const FSoftObjectPath& ObjectPath)
{
	FUnrealObjectRef PackageRef;

	PackageRef.Path = ObjectPath.GetLongPackageName();

	FUnrealObjectRef ObjectRef;
	ObjectRef.Outer = PackageRef;
	ObjectRef.Path = ObjectPath.GetAssetName();

	return ObjectRef;
}

FSoftObjectPath FUnrealObjectRef::ToSoftObjectPath(const FUnrealObjectRef& ObjectRef)
{
	if (!ObjectRef.Path.IsSet())
	{
		return FSoftObjectPath();
	}

	bool bSubObjectName = true;
	FString FullPackagePath;
	const FUnrealObjectRef* CurRef = &ObjectRef;
	while (CurRef)
	{
		if (CurRef->Path.IsSet())
		{
			FString Path = *CurRef->Path;
			if (!FullPackagePath.IsEmpty())
			{
				Path.Append(bSubObjectName ? TEXT(".") : TEXT("/"));
				Path.Append(FullPackagePath);
				bSubObjectName = false;
			}
			FullPackagePath = MoveTemp(Path);
		}
		CurRef = CurRef->Outer.IsSet() ? &(*CurRef->Outer) : nullptr;
	}

	return FSoftObjectPath(MoveTemp(FullPackagePath));
}

bool FUnrealObjectRef::ShouldLoadObjectFromClassPath(UObject* Object)
{
	// We don't want to add objects to this list which are stably-named. This is because:
	// - stably-named Actors are already handled correctly by the GDK and don't need additional special casing,
	// - stably-named Actors then follow two different logic paths at various points in the GDK which results in
	//   inconsistent package map entries.
	// The ensure statement below is a sanity check that we don't inadvertently add a stably-name Actor to this list.
	return IsUniqueActorClass(Object->GetClass()) && ensure(!Object->IsNameStableForNetworking());
}

bool FUnrealObjectRef::IsUniqueActorClass(UClass* Class)
{
	return Class->IsChildOf<AGameStateBase>() || Class->IsChildOf<AGameModeBase>() || Class->IsChildOf<ASpatialMetricsDisplay>()
		   || Class->IsChildOf<ASpatialDebugger>();
}

FUnrealObjectRef FUnrealObjectRef::GetRefFromObjectClassPath(UObject* Object, USpatialPackageMapClient* PackageMap)
{
	FUnrealObjectRef ClassObjectRef = FromObjectPtr(Object->GetClass(), PackageMap);
	if (ClassObjectRef.IsValid())
	{
		ClassObjectRef.bUseClassPathToLoadObject = true;
	}
	return ClassObjectRef;
}
