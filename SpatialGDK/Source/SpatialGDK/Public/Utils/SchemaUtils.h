// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Serialization/BitWriter.h"

#include "Schema/UnrealObjectRef.h"
#include "SpatialConstants.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

using StringToEntityMap = TMap<FString, Worker_EntityId>;

namespace SpatialGDK
{
inline void AddStringToSchema(Schema_Object* Object, Schema_FieldId Id, const FString& Value)
{
	FTCHARToUTF8 CStrConversion(*Value);
	uint32 StringLength = CStrConversion.Length();
	uint8* StringBuffer = Schema_AllocateBuffer(Object, sizeof(char) * StringLength);
	FMemory::Memcpy(StringBuffer, CStrConversion.Get(), sizeof(char) * StringLength);
	Schema_AddBytes(Object, Id, StringBuffer, sizeof(char) * StringLength);
}

inline FString IndexStringFromSchema(const Schema_Object* Object, Schema_FieldId Id, uint32 Index)
{
	int32 StringLength = (int32)Schema_IndexBytesLength(Object, Id, Index);
	const uint8_t* Bytes = Schema_IndexBytes(Object, Id, Index);
	FUTF8ToTCHAR FStringConversion(reinterpret_cast<const ANSICHAR*>(Bytes), StringLength);
	return FString(FStringConversion.Length(), FStringConversion.Get());
}

inline FString GetStringFromSchema(const Schema_Object* Object, Schema_FieldId Id)
{
	return IndexStringFromSchema(Object, Id, 0);
}

inline bool GetBoolFromSchema(const Schema_Object* Object, Schema_FieldId Id)
{
	return !!Schema_GetBool(Object, Id);
}

inline void AddBytesToSchema(Schema_Object* Object, Schema_FieldId Id, const uint8* Data, uint32 NumBytes)
{
	uint8* PayloadBuffer = Schema_AllocateBuffer(Object, sizeof(char) * NumBytes);
	FMemory::Memcpy(PayloadBuffer, Data, sizeof(char) * NumBytes);
	Schema_AddBytes(Object, Id, PayloadBuffer, sizeof(char) * NumBytes);
}

inline void AddBytesToSchema(Schema_Object* Object, Schema_FieldId Id, FBitWriter& Writer)
{
	AddBytesToSchema(Object, Id, Writer.GetData(), Writer.GetNumBytes());
}

inline TArray<uint8> IndexBytesFromSchema(const Schema_Object* Object, Schema_FieldId Id, uint32 Index)
{
	int32 PayloadSize = (int32)Schema_IndexBytesLength(Object, Id, Index);
	return TArray<uint8>((const uint8*)Schema_IndexBytes(Object, Id, Index), PayloadSize);
}

inline TArray<uint8> GetBytesFromSchema(const Schema_Object* Object, Schema_FieldId Id)
{
	return IndexBytesFromSchema(Object, Id, 0);
}

inline void AddWorkerRequirementSetToSchema(Schema_Object* Object, Schema_FieldId Id, const WorkerRequirementSet& Value)
{
	Schema_Object* RequirementSetObject = Schema_AddObject(Object, Id);
	for (const WorkerAttributeSet& AttributeSet : Value)
	{
		Schema_Object* AttributeSetObject = Schema_AddObject(RequirementSetObject, 1);

		for (const FString& Attribute : AttributeSet)
		{
			AddStringToSchema(AttributeSetObject, 1, Attribute);
		}
	}
}

inline WorkerRequirementSet IndexWorkerRequirementSetFromSchema(Schema_Object* Object, Schema_FieldId Id, uint32 Index)
{
	Schema_Object* RequirementSetObject = Schema_IndexObject(Object, Id, Index);

	int32 AttributeSetCount = (int32)Schema_GetObjectCount(RequirementSetObject, 1);
	WorkerRequirementSet RequirementSet;
	RequirementSet.Reserve(AttributeSetCount);

	for (int32 i = 0; i < AttributeSetCount; i++)
	{
		Schema_Object* AttributeSetObject = Schema_IndexObject(RequirementSetObject, 1, i);

		int32 AttributeCount = (int32)Schema_GetBytesCount(AttributeSetObject, 1);
		WorkerAttributeSet AttributeSet;
		AttributeSet.Reserve(AttributeCount);

		for (int32 j = 0; j < AttributeCount; j++)
		{
			AttributeSet.Add(IndexStringFromSchema(AttributeSetObject, 1, j));
		}

		RequirementSet.Add(AttributeSet);
	}

	return RequirementSet;
}

inline WorkerRequirementSet GetWorkerRequirementSetFromSchema(Schema_Object* Object, Schema_FieldId Id)
{
	return IndexWorkerRequirementSetFromSchema(Object, Id, 0);
}

inline void AddObjectRefToSchema(Schema_Object* Object, Schema_FieldId Id, const FUnrealObjectRef& ObjectRef)
{
	using namespace SpatialConstants;

	Schema_Object* ObjectRefObject = Schema_AddObject(Object, Id);

	Schema_AddEntityId(ObjectRefObject, UNREAL_OBJECT_REF_ENTITY_ID, ObjectRef.Entity);
	Schema_AddUint32(ObjectRefObject, UNREAL_OBJECT_REF_OFFSET_ID, ObjectRef.Offset);
	if (ObjectRef.Path)
	{
		AddStringToSchema(ObjectRefObject, UNREAL_OBJECT_REF_PATH_ID, *ObjectRef.Path);
		Schema_AddBool(ObjectRefObject, UNREAL_OBJECT_REF_NO_LOAD_ON_CLIENT_ID, ObjectRef.bNoLoadOnClient);
	}
	if (ObjectRef.Outer)
	{
		AddObjectRefToSchema(ObjectRefObject, UNREAL_OBJECT_REF_OUTER_ID, *ObjectRef.Outer);
	}
	if (ObjectRef.bUseClassPathToLoadObject)
	{
		Schema_AddBool(ObjectRefObject, UNREAL_OBJECT_REF_USE_CLASS_PATH_TO_LOAD_ID, ObjectRef.bUseClassPathToLoadObject);
	}
}

FUnrealObjectRef GetObjectRefFromSchema(Schema_Object* Object, Schema_FieldId Id);

inline FUnrealObjectRef IndexObjectRefFromSchema(Schema_Object* Object, Schema_FieldId Id, uint32 Index)
{
	using namespace SpatialConstants;

	FUnrealObjectRef ObjectRef;

	Schema_Object* ObjectRefObject = Schema_IndexObject(Object, Id, Index);

	ObjectRef.Entity = Schema_GetEntityId(ObjectRefObject, UNREAL_OBJECT_REF_ENTITY_ID);
	ObjectRef.Offset = Schema_GetUint32(ObjectRefObject, UNREAL_OBJECT_REF_OFFSET_ID);
	if (Schema_GetObjectCount(ObjectRefObject, UNREAL_OBJECT_REF_PATH_ID) > 0)
	{
		ObjectRef.Path = GetStringFromSchema(ObjectRefObject, UNREAL_OBJECT_REF_PATH_ID);
	}
	if (Schema_GetBoolCount(ObjectRefObject, UNREAL_OBJECT_REF_NO_LOAD_ON_CLIENT_ID) > 0)
	{
		ObjectRef.bNoLoadOnClient = GetBoolFromSchema(ObjectRefObject, UNREAL_OBJECT_REF_NO_LOAD_ON_CLIENT_ID);
	}
	if (Schema_GetObjectCount(ObjectRefObject, UNREAL_OBJECT_REF_OUTER_ID) > 0)
	{
		ObjectRef.Outer = GetObjectRefFromSchema(ObjectRefObject, UNREAL_OBJECT_REF_OUTER_ID);
	}
	if (Schema_GetBoolCount(ObjectRefObject, UNREAL_OBJECT_REF_USE_CLASS_PATH_TO_LOAD_ID) > 0)
	{
		ObjectRef.bUseClassPathToLoadObject = GetBoolFromSchema(ObjectRefObject, UNREAL_OBJECT_REF_USE_CLASS_PATH_TO_LOAD_ID);
	}

	return ObjectRef;
}

inline FUnrealObjectRef GetObjectRefFromSchema(Schema_Object* Object, Schema_FieldId Id)
{
	return IndexObjectRefFromSchema(Object, Id, 0);
}

inline void AddStringToEntityMapToSchema(Schema_Object* Object, Schema_FieldId Id, StringToEntityMap& Map)
{
	for (auto& Pair : Map)
	{
		Schema_Object* PairObject = Schema_AddObject(Object, Id);
		AddStringToSchema(PairObject, SCHEMA_MAP_KEY_FIELD_ID, Pair.Key);
		Schema_AddEntityId(PairObject, SCHEMA_MAP_VALUE_FIELD_ID, Pair.Value);
	}
}

inline StringToEntityMap GetStringToEntityMapFromSchema(Schema_Object* Object, Schema_FieldId Id)
{
	StringToEntityMap Map;

	int32 MapCount = (int32)Schema_GetObjectCount(Object, Id);
	for (int32 i = 0; i < MapCount; i++)
	{
		Schema_Object* PairObject = Schema_IndexObject(Object, Id, i);

		FString String = GetStringFromSchema(PairObject, SCHEMA_MAP_KEY_FIELD_ID);
		Worker_EntityId Entity = Schema_GetEntityId(PairObject, SCHEMA_MAP_VALUE_FIELD_ID);

		Map.Add(String, Entity);
	}

	return Map;
}

inline void AddRotatorToSchema(Schema_Object* Object, Schema_FieldId Id, FRotator Rotator)
{
	Schema_Object* RotatorObject = Schema_AddObject(Object, Id);

	Schema_AddFloat(RotatorObject, 1, Rotator.Pitch);
	Schema_AddFloat(RotatorObject, 2, Rotator.Yaw);
	Schema_AddFloat(RotatorObject, 3, Rotator.Roll);
}

inline FRotator IndexRotatorFromSchema(Schema_Object* Object, Schema_FieldId Id, uint32 Index)
{
	FRotator Rotator;

	Schema_Object* RotatorObject = Schema_IndexObject(Object, Id, Index);

	Rotator.Pitch = Schema_GetFloat(RotatorObject, 1);
	Rotator.Yaw = Schema_GetFloat(RotatorObject, 2);
	Rotator.Roll = Schema_GetFloat(RotatorObject, 3);

	return Rotator;
}

inline FRotator GetRotatorFromSchema(Schema_Object* Object, Schema_FieldId Id)
{
	return IndexRotatorFromSchema(Object, Id, 0);
}

inline void AddVectorToSchema(Schema_Object* Object, Schema_FieldId Id, FVector Vector)
{
	Schema_Object* VectorObject = Schema_AddObject(Object, Id);

	Schema_AddFloat(VectorObject, 1, Vector.X);
	Schema_AddFloat(VectorObject, 2, Vector.Y);
	Schema_AddFloat(VectorObject, 3, Vector.Z);
}

inline FVector IndexVectorFromSchema(Schema_Object* Object, Schema_FieldId Id, uint32 Index)
{
	FVector Vector;

	Schema_Object* VectorObject = Schema_IndexObject(Object, Id, Index);

	Vector.X = Schema_GetFloat(VectorObject, 1);
	Vector.Y = Schema_GetFloat(VectorObject, 2);
	Vector.Z = Schema_GetFloat(VectorObject, 3);

	return Vector;
}

inline FVector GetVectorFromSchema(Schema_Object* Object, Schema_FieldId Id)
{
	return IndexVectorFromSchema(Object, Id, 0);
}

// Generates the full path from an ObjectRef, if it has paths. Writes the result to OutPath.
// Does not clear OutPath first.
void GetFullPathFromUnrealObjectReference(const FUnrealObjectRef& ObjectRef, FString& OutPath);

} // namespace SpatialGDK
