// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialMemoryWriter.h"

#include "SpatialPackageMapClient.h"
#include "WeakObjectPtr.h"

void FSpatialMemoryWriter::SerializeObjectRef(improbable::unreal::UnrealObjectRef& ObjectRef)
{
	*this << ObjectRef.entity();
	*this << ObjectRef.offset();

	bool bHasPath = !ObjectRef.path().empty();
	*this << bHasPath;
	if (bHasPath)
	{
		FString Path = FString(UTF8_TO_TCHAR(ObjectRef.path()->c_str()));
		*this << Path;
	}

	bool bHasOuter = !ObjectRef.outer().empty();
	*this << bHasOuter;
	if (bHasOuter)
	{
		SerializeObjectRef(*ObjectRef.outer());
	}
}

FArchive& FSpatialMemoryWriter::operator<<(UObject*& Value)
{
	improbable::unreal::UnrealObjectRef ObjectRef;
	if (Value != nullptr)
	{
		FNetworkGUID NetGUID = PackageMap->GetNetGUIDFromObject(Value);
		ObjectRef = PackageMap->GetUnrealObjectRefFromNetGUID(NetGUID);
		if (ObjectRef == SpatialConstants::UNRESOLVED_OBJECT_REF)
		{
			// TODO: Collect unresolved objects in a set to queue up
			ObjectRef = SpatialConstants::NULL_OBJECT_REF;
		}
	}
	else
	{
		ObjectRef = SpatialConstants::NULL_OBJECT_REF;
	}

	SerializeObjectRef(ObjectRef);

	return *this;
}

FArchive& FSpatialMemoryWriter::operator<<(FWeakObjectPtr& Value)
{
	UObject* Object = Value.Get(true);
	*this << Object;

	return *this;
}
