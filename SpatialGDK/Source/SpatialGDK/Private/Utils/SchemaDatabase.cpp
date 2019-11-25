// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/SchemaDatabase.h"
#include "Hash/CityHash.h"
#include "Serialization/BufferArchive.h"

#if WITH_EDITOR
FArchive& operator<<(FArchive& Archive, FActorSchemaData& Data)
{
	Archive << Data.GeneratedSchemaName;
	for (size_t i = 0; i < SCHEMA_Count; i++)
		Archive << Data.SchemaComponents[i];
	Archive << Data.SubobjectData;
	return Archive;
}
FArchive& operator<<(FArchive& Archive, FActorSpecificSubobjectSchemaData& Data)
{
	Archive << Data.ClassPath;
	Archive << Data.Name;
	for (size_t i = 0; i < SCHEMA_Count; i++)
		Archive << Data.SchemaComponents[i];
	return Archive;
}
FArchive& operator<<(FArchive& Archive, FSubobjectSchemaData& Data)
{
	return Archive
		<< Data.GeneratedSchemaName
		<< Data.DynamicSubobjectComponents;
}
FArchive& operator<<(FArchive& Archive, FDynamicSubobjectSchemaData& Data)
{
	for (size_t i = 0; i < SCHEMA_Count; i++)
		Archive << Data.SchemaComponents[i];
	return Archive;
}
uint32 USchemaDatabase::GenerateHash()
{
	FBufferArchive Buffer;
	Buffer
		<< ActorClassPathToSchema
		<< SubobjectClassPathToSchema
		<< LevelPathToComponentId
		<< ComponentIdToClassPath
		<< LevelComponentIds
		<< NextAvailableComponentId;

	// Unpack and sort to ensure deterministic storage
	return CityHash32((const char*)Buffer.GetData(), Buffer.Num());
}
#endif
