// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "EngineClasses/SpatialNetBitWriter.h"
#include "improbable/UnrealObjectRef.h"

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

using WorkerAttributeSet = TArray<FString>;
using WorkerRequirementSet = TArray<WorkerAttributeSet>;

using StringToEntityMap = TMap<FString, Worker_EntityId>;

namespace improbable
{

inline void AddStringToSchema(Schema_Object* Object, Schema_FieldId Id, const FString& Value)
{
	FTCHARToUTF8 CStrConvertion(*Value);
	uint32 StringLength = CStrConvertion.Length();
	uint8* StringBuffer = Schema_AllocateBuffer(Object, sizeof(char) * StringLength);
	FMemory::Memcpy(StringBuffer, CStrConvertion.Get(), sizeof(char) * StringLength);
	Schema_AddBytes(Object, Id, StringBuffer, sizeof(char) * StringLength);
}

inline FString IndexStringFromSchema(const Schema_Object* Object, Schema_FieldId Id, uint32 Index)
{
	int32 StringLength = (int32)Schema_IndexBytesLength(Object, Id, Index);
	return FString(StringLength, UTF8_TO_TCHAR(Schema_IndexBytes(Object, Id, Index)));
}

inline FString GetStringFromSchema(const Schema_Object* Object, Schema_FieldId Id)
{
	return IndexStringFromSchema(Object, Id, 0);
}

inline void AddPayloadToSchema(Schema_Object* Object, Schema_FieldId Id, FSpatialNetBitWriter& Writer)
{
	uint32 PayloadSize = Writer.GetNumBytes();
	uint8* PayloadBuffer = Schema_AllocateBuffer(Object, sizeof(char) * PayloadSize);
	FMemory::Memcpy(PayloadBuffer, Writer.GetData(), sizeof(char) * PayloadSize);
	Schema_AddBytes(Object, Id, PayloadBuffer, sizeof(char) * PayloadSize);
}

inline TArray<uint8> IndexPayloadFromSchema(const Schema_Object* Object, Schema_FieldId Id, uint32 Index)
{
	int32 PayloadSize = (int32)Schema_IndexBytesLength(Object, Id, Index);
	return TArray<uint8>((const uint8*)Schema_IndexBytes(Object, Id, Index), PayloadSize);
}

inline TArray<uint8> GetPayloadFromSchema(const Schema_Object* Object, Schema_FieldId Id)
{
	return IndexPayloadFromSchema(Object, Id, 0);
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

inline WorkerRequirementSet GetWorkerRequirementSetFromSchema(Schema_Object* Object, Schema_FieldId Id)
{
	Schema_Object* RequirementSetObject = Schema_GetObject(Object, Id);

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

inline void AddObjectRefToSchema(Schema_Object* Object, Schema_FieldId Id, const FUnrealObjectRef& ObjectRef)
{
	Schema_Object* ObjectRefObject = Schema_AddObject(Object, Id);

	Schema_AddEntityId(ObjectRefObject, 1, ObjectRef.Entity);
	Schema_AddUint32(ObjectRefObject, 2, ObjectRef.Offset);
	if (ObjectRef.Path)
	{
		AddStringToSchema(ObjectRefObject, 3, *ObjectRef.Path);
	}
	if (ObjectRef.Outer)
	{
		AddObjectRefToSchema(ObjectRefObject, 4, *ObjectRef.Outer);
	}
}

FUnrealObjectRef GetObjectRefFromSchema(Schema_Object* Object, Schema_FieldId Id);

inline FUnrealObjectRef IndexObjectRefFromSchema(Schema_Object* Object, Schema_FieldId Id, uint32 Index)
{
	FUnrealObjectRef ObjectRef;

	Schema_Object* ObjectRefObject = Schema_IndexObject(Object, Id, Index);

	ObjectRef.Entity = Schema_GetEntityId(ObjectRefObject, 1);
	ObjectRef.Offset = Schema_GetUint32(ObjectRefObject, 2);
	if (Schema_GetBytesCount(ObjectRefObject, 3) > 0)
	{
		ObjectRef.Path = GetStringFromSchema(ObjectRefObject, 3);
	}
	if (Schema_GetObjectCount(ObjectRefObject, 4) > 0)
	{
		ObjectRef.Outer = FUnrealObjectRef(GetObjectRefFromSchema(ObjectRefObject, 4));
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
		Schema_Object* PairObject = Schema_AddObject(Object, 1);
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

}
