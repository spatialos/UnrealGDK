// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

// IMPROBABLE: MCS - this whole file is a workaround for UNR-180
#pragma once

#include "CoreTypes.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "SpatialPackageMapClient.h"


class FSpatialMemoryWriter : public FMemoryWriter
{
public:
	FSpatialMemoryWriter(USpatialPackageMapClient* InPackageMap, TArray<uint8>& InBytes, bool bIsPersistent = false, bool bSetOffset = false, const FName InArchiveName = NAME_None )
	: FMemoryWriter(InBytes, bIsPersistent, bSetOffset, InArchiveName)
	, PackageMap(InPackageMap)
	{
	}

	virtual FString GetArchiveName() const override { return TEXT("FSpatialMemoryWriter"); }

	virtual FArchive& operator<<(class UObject*& Res) override
	{
		if (Res != nullptr)
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Res);
			improbable::unreal::UnrealObjectRef ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
			if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
			{
				//Interop->QueueOutgoingObjectRepUpdate_Internal(Value[i], Channel, 28);
				auto NullRef = SpatialConstants::NULL_OBJECT_REF;
				Serialize(&NullRef, sizeof(improbable::unreal::UnrealObjectRef));
			}
			else
			{
				Serialize( &ObjectRef, sizeof(improbable::unreal::UnrealObjectRef) );
			}
		}
		else
		{
			auto NullRef = SpatialConstants::NULL_OBJECT_REF;
			Serialize( &NullRef, sizeof(improbable::unreal::UnrealObjectRef));
		}

		return *this;
	}

protected:
	USpatialPackageMapClient* PackageMap;
};

class FSpatialMemoryReader : public FMemoryReader
{
public:
	FSpatialMemoryReader(USpatialPackageMapClient* InPackageMap, const TArray<uint8>& InBytes, bool bIsPersistent = false)
		: FMemoryReader(InBytes, bIsPersistent)
		, PackageMap(InPackageMap)
	{
	}

	virtual FString GetArchiveName() const override { return TEXT("FSpatialMemoryReader"); }

	virtual FArchive& operator<<(class UObject*& Res) override
	{
		improbable::unreal::UnrealObjectRef ObjectRef;
		Serialize(&ObjectRef, sizeof(improbable::unreal::UnrealObjectRef));
		check(ObjectRef != SpatialConstants::UNRESOLVED_OBJECT_REF);
		if (ObjectRef == SpatialConstants::NULL_OBJECT_REF)
		{
			Res = nullptr;
		}
		else
		{
			FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromUnrealObjectRef(ObjectRef);
			if (NetGUID.IsValid())
			{
				UObject* Object_Raw = PackageMap->GetObjectFromNetGUID(NetGUID, true);
				checkf(Object_Raw, TEXT("An object ref %s should map to a valid object."), *ObjectRefToString(ObjectRef));
				Res = Object_Raw;
				//Value[i] = dynamic_cast<US_ItemStatsPacketObject*>(Object_Raw);
				//checkf(Value[i], TEXT("Object ref %s maps to object %s with the wrong class."), *ObjectRefToString(ObjectRef), *Object_Raw->GetFullName());
			}
			else
			{
				//UE_LOG(LogSpatialOSInterop, Log, TEXT("%s: Received unresolved object property. Value: %s. actor %s (%lld), property %s (handle %d)"),
				//	*Interop->GetSpatialOS()->GetWorkerId(),
				//	*ObjectRefToString(ObjectRef),
				//	*ActorChannel->Actor->GetName(),
				//	ActorChannel->GetEntityId().ToSpatialEntityId(),
				//	*RepData->Property->GetName(),
				//	Handle);
				////bWriteObjectProperty = false;
				//Interop->QueueIncomingObjectRepUpdate_Internal(ObjectRef, ActorChannel, RepData);
			}
		}

		return *this;
	}

protected:
	USpatialPackageMapClient* PackageMap;
};

