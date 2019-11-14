// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Schema/UnrealObjectRef.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "Utils/SchemaUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogUnrealObjectRef, Log, All);

const FUnrealObjectRef FUnrealObjectRef::NULL_OBJECT_REF = FUnrealObjectRef(0, 0);
const FUnrealObjectRef FUnrealObjectRef::UNRESOLVED_OBJECT_REF = FUnrealObjectRef(0, 1);

UObject* FUnrealObjectRef::ToObjectPtr(const FUnrealObjectRef& ObjectRef, USpatialPackageMapClient* PackageMap, bool& bOutUnresolved)
{
	if (ObjectRef == FUnrealObjectRef::NULL_OBJECT_REF)
	{
		return nullptr;
	}
	else
	{
		if (ObjectRef.bUseSingletonClassPath)
		{
			// This is a singleton ref, which means it's just the UnrealObjectRef of the singleton class, with this boolean set.
			// Unset it to get the original UnrealObjectRef of its singleton class, and look it up in the PackageMap.
			FUnrealObjectRef SingletonClassRef = ObjectRef;
			SingletonClassRef.bUseSingletonClassPath = false;

			UObject* Value = PackageMap->GetSingletonByClassRef(SingletonClassRef);
			if (Value == nullptr)
			{
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
				// At this point, we're unable to resolve a stably-named actor by path. This likely means either the actor doesn't exist, or
				// it's part of a streaming level that hasn't been streamed in. Native Unreal networking sets reference to nullptr and continues.
				// So we do the same.
				FString FullPath;
				SpatialGDK::GetFullPathFromUnrealObjectReference(ObjectRef, FullPath);
				UE_LOG(LogUnrealObjectRef, Warning, TEXT("Object ref did not map to valid object. Streaming level not loaded or actor deleted. Will be set to nullptr: %s %s"),
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

				// If this is a singleton that hasn't been resolved yet, send its class path instead.
				if (ObjectValue->GetClass()->HasAnySpatialClassFlags(SPATIALCLASS_Singleton))
				{
					ObjectRef = GetSingletonClassRef(ObjectValue, PackageMap);
					if (ObjectRef.IsValid())
					{
						return ObjectRef;
					}
				}

				// Unresolved object. 
				UE_LOG(LogUnrealObjectRef, Verbose, TEXT("FUnrealObjectRef::FromObjectPtr: ObjectValue is unresolved! %s"), *ObjectValue->GetName());
				ObjectRef = FUnrealObjectRef::NULL_OBJECT_REF;
			}
		}
	}

	return ObjectRef;
}

FUnrealObjectRef FUnrealObjectRef::GetSingletonClassRef(UObject* SingletonObject, USpatialPackageMapClient* PackageMap)
{
	FUnrealObjectRef ClassObjectRef = FromObjectPtr(SingletonObject->GetClass(), PackageMap);
	if (ClassObjectRef.IsValid())
	{
		ClassObjectRef.bUseSingletonClassPath = true;
	}
	return ClassObjectRef;
}
