// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialMemoryWriter.h"

#include "SpatialPackageMapClient.h"
#include "WeakObjectPtr.h"
#include <improbable/unreal/gdk/core_types.h>

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

	*this << ObjectRef.entity();
	*this << ObjectRef.offset();

	return *this;
}

FArchive& FSpatialMemoryWriter::operator<<(FWeakObjectPtr& Value)
{
	UObject* Object = Value.Get(true);
	*this << Object;

	return *this;
}
