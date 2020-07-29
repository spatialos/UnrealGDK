// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetBitReader.h"

#include "UObject/WeakObjectPtr.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "SpatialConstants.h"

DEFINE_LOG_CATEGORY(LogSpatialNetBitReader);

static thread_local FSpatialNetBitReader* GCurrentReader = nullptr;

FSpatialNetBitReader::FSpatialNetBitReader(USpatialPackageMapClient* InPackageMap, uint8* Source, int64 CountBits, TSet<FUnrealObjectRef>& InDynamicRefs, TSet<FUnrealObjectRef>& InUnresolvedRefs)
	: FNetBitReader(InPackageMap, Source, CountBits)
	, DynamicRefs(InDynamicRefs)
	, UnresolvedRefs(InUnresolvedRefs)
{
	check(GCurrentReader == nullptr);
	GCurrentReader = this;
}

FSpatialNetBitReader::~FSpatialNetBitReader()
{
	GCurrentReader = nullptr;
}

void FSpatialNetBitReader::DeserializeObjectRef(FArchive& Archive, FUnrealObjectRef& ObjectRef)
{
	int64 EntityId;
	Archive << EntityId;
	ObjectRef.Entity = EntityId;
	Archive << ObjectRef.Offset;

	uint8 HasPath;
	Archive.SerializeBits(&HasPath, 1);
	if (HasPath)
	{
		FString Path;
		Archive << Path;

		ObjectRef.Path = Path;
	}

	uint8 HasOuter;
	Archive.SerializeBits(&HasOuter, 1);
	if (HasOuter)
	{
		ObjectRef.Outer = FUnrealObjectRef();
		DeserializeObjectRef(Archive, *ObjectRef.Outer);
	}

	Archive.SerializeBits(&ObjectRef.bNoLoadOnClient, 1);
	Archive.SerializeBits(&ObjectRef.bUseClassPathToLoadObject, 1);
}

UObject* FSpatialNetBitReader::ReadObject(FArchive& Archive, USpatialPackageMapClient* PackageMap, bool& bUnresolved)
{
	FUnrealObjectRef ObjectRef;

	DeserializeObjectRef(Archive, ObjectRef);
	check(ObjectRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);

	UObject* Value = FUnrealObjectRef::ToObjectPtr(ObjectRef, PackageMap, bUnresolved);

	if (GCurrentReader != nullptr)
	{
		if (bUnresolved)
		{
			GCurrentReader->UnresolvedRefs.Add(ObjectRef);
		}
		else if (Value && !Value->IsFullNameStableForNetworking())
		{
			GCurrentReader->DynamicRefs.Add(ObjectRef);
		}
	}

	return Value;
}

FArchive& FSpatialNetBitReader::operator<<(FWeakObjectPtr& Value)
{
	UObject* Object;
	*this << Object;

	Value = Object;

	return *this;
}
