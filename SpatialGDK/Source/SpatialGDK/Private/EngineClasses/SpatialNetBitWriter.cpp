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

	SerializeBits(&ObjectRef.bNoLoadOnClient, 1);
	SerializeBits(&ObjectRef.bUseSingletonClassPath, 1);
}

FArchive& FSpatialNetBitWriter::operator<<(UObject*& Value)
{
	FUnrealObjectRef ObjectRef = FUnrealObjectRef::FromObjectPtr(Value, Cast<USpatialPackageMapClient>(PackageMap));
	SerializeObjectRef(ObjectRef);

	return *this;
}

FArchive& FSpatialNetBitWriter::operator<<(FWeakObjectPtr& Value)
{
	UObject* Object = Value.Get(true);
	*this << Object;

	return *this;
}
