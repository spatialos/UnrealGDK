#pragma once

#include <improbable/c_schema.h>
#include <improbable/c_worker.h>

#include <stdint.h>
#include <string>
#include <vector>

inline void Schema_AddString(Schema_Object* Object, Schema_FieldId Id, const std::string& Value)
{
	std::uint32_t StringLength = Value.size();
	std::uint8_t* StringBuffer = Schema_AllocateBuffer(Object, sizeof(char) * StringLength);
	memcpy(StringBuffer, Value.c_str(), sizeof(char) * StringLength);
	Schema_AddBytes(Object, Id, StringBuffer, sizeof(char) * StringLength);
}

inline std::string Schema_IndexString(const Schema_Object* Object, Schema_FieldId Id, std::uint32_t Index)
{
	std::uint32_t StringLength = Schema_IndexBytesLength(Object, Id, Index);
	return std::string((const char*)Schema_IndexBytes(Object, Id, Index), StringLength);
}

inline std::string Schema_GetString(const Schema_Object* Object, Schema_FieldId Id)
{
	return Schema_IndexString(Object, Id, 0);
}

using WorkerAttributeSet = std::vector<std::string>;
using WorkerRequirementSet = std::vector<WorkerAttributeSet>;

inline void Schema_AddWorkerRequirementSet(Schema_Object* Object, Schema_FieldId Id, const WorkerRequirementSet& Value)
{
	Schema_Object* RequirementSetObject = Schema_AddObject(Object, Id);
	for (const WorkerAttributeSet& AttributeSet : Value)
	{
		Schema_Object* AttributeSetObject = Schema_AddObject(RequirementSetObject, 1);

		for (const std::string& Attribute : AttributeSet)
		{
			Schema_AddString(AttributeSetObject, 1, Attribute);
		}
	}
}

inline WorkerRequirementSet Schema_GetWorkerRequirementSet(Schema_Object* Object, Schema_FieldId Id)
{
	Schema_Object* RequirementSetObject = Schema_GetObject(Object, Id);

	std::uint32_t AttributeSetCount = Schema_GetObjectCount(RequirementSetObject, 1);
	WorkerRequirementSet RequirementSet;
	RequirementSet.reserve(AttributeSetCount);

	for (std::uint32_t i = 0; i < AttributeSetCount; i++)
	{
		Schema_Object* AttributeSetObject = Schema_IndexObject(RequirementSetObject, 1, i);

		std::uint32_t AttributeCount = Schema_GetBytesCount(AttributeSetObject, 1);
		WorkerAttributeSet AttributeSet;
		AttributeSet.reserve(AttributeCount);

		for (std::uint32_t j = 0; j < AttributeCount; j++)
		{
			std::uint32_t AttributeLength = Schema_IndexBytesLength(AttributeSetObject, 1, j);
			AttributeSet.emplace_back((const char*)Schema_IndexBytes(AttributeSetObject, 1, j), AttributeLength);
		}

		RequirementSet.push_back(AttributeSet);
	}

	return RequirementSet;
}

void Schema_AddObjectRef(Schema_Object* Object, Schema_FieldId Id, const UnrealObjectRef& ObjectRef)
{
	Schema_Object* ObjectRefObject = Schema_AddObject(Object, Id);

	Schema_AddEntityId(ObjectRefObject, 1, ObjectRef.Entity);
	Schema_AddUint32(ObjectRefObject, 2, ObjectRef.Offset);
	if (ObjectRef.Path)
	{
		Schema_AddString(ObjectRefObject, 3, *ObjectRef.Path);
	}
	if (ObjectRef.Outer)
	{
		Schema_AddObjectRef(ObjectRefObject, 4, *ObjectRef.Outer);
	}
}
