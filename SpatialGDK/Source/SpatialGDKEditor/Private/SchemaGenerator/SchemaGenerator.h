// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "TypeStructure.h"
#include "Utils/SchemaDatabase.h"

using ComponentIdPerType = Worker_ComponentId[ESchemaComponentType::SCHEMA_Count];

DECLARE_LOG_CATEGORY_EXTERN(LogSchemaGenerator, Log, All);

class FCodeWriter;
struct FComponentIdGenerator;

struct SchemaGeneratorData
{
	SchemaGeneratorData();
	// TArray<UClass*> SchemaGeneratedClasses;
	TArray<FString> SchemaGeneratedClasses;
	TMap<FString, FActorSchemaData> ActorClassPathToSchema;
	TMap<FString, FSubobjectSchemaData> SubobjectClassPathToSchema;
	Worker_ComponentId NextAvailableComponentId = SpatialConstants::STARTING_GENERATED_COMPONENT_ID;

	// Sets of data/owner only/handover components
	TMap<ESchemaComponentType, TSet<Worker_ComponentId>> SchemaComponentTypeToComponents;

	// LevelStreaming
	TMap<FString, Worker_ComponentId> LevelPathToComponentId;

	// Prevent name collisions.
	TMap<FString, FString> ClassPathToSchemaName;
	TMap<FString, FString> SchemaNameToClassPath;
	TMap<FString, TSet<FString>> PotentialSchemaNameCollisions;

	// QBI
	TMap<float, Worker_ComponentId> NetCullDistanceToComponentId;
};

class SchemaGenerator
{
public:
	SchemaGenerator(SchemaGeneratorData& InData)
		: Data(InData)
	{
	}

	// Generates schema for an Actor
	void GenerateActorSchema(FComponentIdGenerator& IdGenerator, const FUnrealClassDesc& Class, TSharedPtr<FUnrealOfflineType> TypeInfo,
							 FString SchemaPath);
	// Generates schema for a Subobject class - the schema type and the dynamic schema components
	void GenerateSubobjectSchema(FComponentIdGenerator& IdGenerator, const FUnrealClassDesc& Class, TSharedPtr<FUnrealOfflineType> TypeInfo,
								 FString SchemaPath);

	// Generates schema for an Actor
	void GenerateActorSchema(FComponentIdGenerator& IdGenerator, const FUnrealClassDesc& Class, TSharedPtr<FUnrealOfflineType> TypeInfo,
							 FCodeWriter& Writer, FCodeWriter& SubObjectsWriter);
	// Generates schema for a Subobject class - the schema type and the dynamic schema components
	void GenerateSubobjectSchema(FComponentIdGenerator& IdGenerator, const FUnrealClassDesc& Class, TSharedPtr<FUnrealOfflineType> TypeInfo,
								 FCodeWriter& Writer);

	// Generates schema for RPC endpoints.
	void GenerateRPCEndpointsSchema(FString SchemaPath);

	void AddComponentId(const Worker_ComponentId ComponentId, ComponentIdPerType& SchemaComponents,
						const ESchemaComponentType ComponentType);

	SchemaGeneratorData& Data;

private:
	void GenerateRPCEndpoint(FCodeWriter& Writer, FString EndpointName, Worker_ComponentId ComponentId, TArray<ERPCType> SentRPCTypes,
							 TArray<ERPCType> AckedRPCTypes);

	void GenerateSubobjectSchemaForActor(FComponentIdGenerator& IdGenerator, FString const& ActorClassPathName,
										 TSharedPtr<FUnrealOfflineType> TypeInfo, FCodeWriter& Writer, FActorSchemaData& ActorSchemaData,
										 const FActorSchemaData* ExistingSchemaData);

	void GenerateSubobjectSchemaForActorIncludes(FCodeWriter& Writer, TSharedPtr<FUnrealOfflineType>& TypeInfo);

	FActorSpecificSubobjectSchemaData GenerateSchemaForStaticallyAttachedSubobject(
		FCodeWriter& Writer, FComponentIdGenerator& IdGenerator, FString PropertyName, TSharedPtr<FUnrealOfflineType>& TypeInfo,
		FString const& ComponentClassPath, FString const& ActorClassPath, int MapIndex,
		const FActorSpecificSubobjectSchemaData* ExistingSchemaData);
};
