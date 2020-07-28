// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/SpatialNetBitWriter.h"

#include "UObject/WeakObjectPtr.h"

#include "EngineClasses/SpatialNetDriver.h"
#include "EngineClasses/SpatialPackageMapClient.h"
#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"
#include "Utils/EntityPool.h"

DEFINE_LOG_CATEGORY(LogSpatialNetSerialize);

FSpatialNetBitWriter::FSpatialNetBitWriter(USpatialPackageMapClient* InPackageMap)
	: FNetBitWriter(InPackageMap, 0)
{}

void FSpatialNetBitWriter::SerializeObjectRef(FArchive& Archive, FUnrealObjectRef& ObjectRef)
{
	int64 EntityId = ObjectRef.Entity;
	Archive << EntityId;
	Archive << ObjectRef.Offset;

	uint8 HasPath = ObjectRef.Path.IsSet();
	Archive.SerializeBits(&HasPath, 1);
	if (HasPath)
	{
		Archive << ObjectRef.Path.GetValue();
	}

	uint8 HasOuter = ObjectRef.Outer.IsSet();
	Archive.SerializeBits(&HasOuter, 1);
	if (HasOuter)
	{
		SerializeObjectRef(Archive, *ObjectRef.Outer);
	}

	Archive.SerializeBits(&ObjectRef.bNoLoadOnClient, 1);
	Archive.SerializeBits(&ObjectRef.bUseClassPathToLoadObject, 1);
}

void FSpatialNetBitWriter::WriteObject(FArchive& Archive, USpatialPackageMapClient* PackageMap, UObject* Object)
{
	FUnrealObjectRef ObjectRef = FUnrealObjectRef::FromObjectPtr(Object, PackageMap);
	SerializeObjectRef(Archive, ObjectRef);
}

FArchive& FSpatialNetBitWriter::operator<<(UObject*& Value)
{
	WriteObject(*this, Cast<USpatialPackageMapClient>(PackageMap), Value);
	return *this;
}

FArchive& FSpatialNetBitWriter::operator<<(FWeakObjectPtr& Value)
{
	UObject* Object = Value.Get(true);
	*this << Object;

	return *this;
}
