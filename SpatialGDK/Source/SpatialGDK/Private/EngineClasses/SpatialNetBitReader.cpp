// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetBitReader.h"

#include "UObject/WeakObjectPtr.h"

#include "EngineClasses/SpatialPackageMapClient.h"
#include "SpatialConstants.h"

DEFINE_LOG_CATEGORY(LogSpatialNetBitReader);

static thread_local FSpatialNetBitReader::ReadScope* s_ReadScope = nullptr;

FSpatialNetBitReader::ReadScope::ReadScope()
{
	check(s_ReadScope == nullptr);
	s_ReadScope = this;
}

FSpatialNetBitReader::ReadScope::~ReadScope()
{
	s_ReadScope = nullptr;
}

FSpatialNetBitReader::ReadScope* FSpatialNetBitReader::GetReadScope()
{
	return s_ReadScope;
}

namespace
{
	void DeserializeObjectRef(FUnrealObjectRef& ObjectRef, FArchive& Archive)
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
			DeserializeObjectRef(*ObjectRef.Outer, Archive);
		}

		Archive.SerializeBits(&ObjectRef.bNoLoadOnClient, 1);
		Archive.SerializeBits(&ObjectRef.bUseClassPathToLoadObject, 1);
	}
}

UObject* FSpatialNetBitReader::ReadObject(bool& bUnresolved, USpatialPackageMapClient* PackageMap, FArchive& Archive)
{
	FUnrealObjectRef ObjectRef;

	DeserializeObjectRef(ObjectRef, Archive);
	check(ObjectRef != FUnrealObjectRef::UNRESOLVED_OBJECT_REF);

	UObject* Value = FUnrealObjectRef::ToObjectPtr(ObjectRef, PackageMap, bUnresolved);

	if (FSpatialNetBitReader::ReadScope* Scope = FSpatialNetBitReader::GetReadScope())
	{
		if (bUnresolved)
		{
			Scope->UnresolvedRefs.Add(ObjectRef);
		}
		else if (Value && !Value->IsFullNameStableForNetworking())
		{
			Scope->DynamicRefs.Add(ObjectRef);
		}
	}

	return Value;
}
