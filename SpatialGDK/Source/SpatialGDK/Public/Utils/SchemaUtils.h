// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Serialization/BitWriter.h"

#include "Schema/UnrealObjectRef.h"

#include <WorkerSDK/improbable/c_schema.h>
#include <WorkerSDK/improbable/c_worker.h>

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

inline bool GetBoolFromSchema(const Schema_Object* Object, Schema_FieldId Id)
{
	return !!Schema_GetBool(Object, Id);
}

inline void AddBytesToSchema(Schema_Object* Object, Schema_FieldId Id, FBitWriter& Writer)
{
	uint32 PayloadSize = Writer.GetNumBytes();
	uint8* PayloadBuffer = Schema_AllocateBuffer(Object, sizeof(char) * PayloadSize);
	FMemory::Memcpy(PayloadBuffer, Writer.GetData(), sizeof(char) * PayloadSize);
	Schema_AddBytes(Object, Id, PayloadBuffer, sizeof(char) * PayloadSize);
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
		Schema_AddBool(ObjectRefObject, 4, ObjectRef.bNoLoadOnClient);
	}
	if (ObjectRef.Outer)
	{
		AddObjectRefToSchema(ObjectRefObject, 5, *ObjectRef.Outer);
	}
}

FUnrealObjectRef GetObjectRefFromSchema(Schema_Object* Object, Schema_FieldId Id);

inline FUnrealObjectRef IndexObjectRefFromSchema(Schema_Object* Object, Schema_FieldId Id, uint32 Index)
{
	FUnrealObjectRef ObjectRef;

	Schema_Object* ObjectRefObject = Schema_IndexObject(Object, Id, Index);

	ObjectRef.Entity = Schema_GetEntityId(ObjectRefObject, 1);
	ObjectRef.Offset = Schema_GetUint32(ObjectRefObject, 2);
	if (Schema_GetObjectCount(ObjectRefObject, 3) > 0)
	{
		ObjectRef.Path = GetStringFromSchema(ObjectRefObject, 3);
	}
	if (Schema_GetBoolCount(ObjectRefObject, 4) > 0)
	{
		ObjectRef.bNoLoadOnClient = GetBoolFromSchema(ObjectRefObject, 4);
	}
	if (Schema_GetObjectCount(ObjectRefObject, 5) > 0)
	{
		ObjectRef.Outer = GetObjectRefFromSchema(ObjectRefObject, 5);
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

inline FRotator GetRotatorFromSchema(Schema_Object* Object, Schema_FieldId Id)
{
	FRotator Rotator;

	Schema_Object* RotatorObject = Schema_GetObject(Object, Id);

	Rotator.Pitch = Schema_GetFloat(RotatorObject, 1);
	Rotator.Yaw = Schema_GetFloat(RotatorObject, 2);
	Rotator.Roll = Schema_GetFloat(RotatorObject, 3);

	return Rotator;
}

inline void AddVectorToSchema(Schema_Object* Object, Schema_FieldId Id, FVector Vector)
{
	Schema_Object* VectorObject = Schema_AddObject(Object, Id);

	Schema_AddFloat(VectorObject, 1, Vector.X);
	Schema_AddFloat(VectorObject, 2, Vector.Y);
	Schema_AddFloat(VectorObject, 3, Vector.Z);
}

inline FVector GetVectorFromSchema(Schema_Object* Object, Schema_FieldId Id)
{
	FVector Vector;

	Schema_Object* VectorObject = Schema_GetObject(Object, Id);

	Vector.X = Schema_GetFloat(VectorObject, 1);
	Vector.Y = Schema_GetFloat(VectorObject, 2);
	Vector.Z = Schema_GetFloat(VectorObject, 3);

	return Vector;
}

inline void DeepCopySchemaObject(Schema_Object* Source, Schema_Object* Target)
{
	uint32_t Length = Schema_GetWriteBufferLength(Source);
	uint8_t* Buffer = Schema_AllocateBuffer(Target, Length);
	Schema_WriteToBuffer(Source, Buffer);
	Schema_Clear(Target);
	Schema_MergeFromBuffer(Target, Buffer, Length);
}

inline Schema_ComponentData* DeepCopyComponentData(Schema_ComponentData* Source)
{
	Schema_ComponentData* Copy = Schema_CreateComponentData(Schema_GetComponentDataComponentId(Source));
	DeepCopySchemaObject(Schema_GetComponentDataFields(Source), Schema_GetComponentDataFields(Copy));
	return Copy;
}

// Generates the full path from an ObjectRef, if it has paths. Writes the result to OutPath.
// Does not clear OutPath first.
void GetFullPathFromUnrealObjectReference(const FUnrealObjectRef& ObjectRef, FString& OutPath);

}
