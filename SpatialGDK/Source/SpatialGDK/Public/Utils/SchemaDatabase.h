// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/StaticArray.h"
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/World.h"
#include "SpatialConstants.h"

#include "SchemaDatabase.generated.h"

struct FComponentIdGenerator;

// Schema data related to a default Subobject owned by a specific Actor class.
USTRUCT()
struct FActorSpecificSubobjectSchemaData
{
	GENERATED_USTRUCT_BODY()

	static FActorSpecificSubobjectSchemaData Generate(FComponentIdGenerator& IdGenerator, const UClass* Class);

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	FString ClassPath;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	FName Name;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	uint32 SchemaComponents[SCHEMA_Count] = {};
};

// Schema data related to an Actor class
USTRUCT()
struct FActorSchemaData
{
	GENERATED_USTRUCT_BODY()

	static FActorSchemaData Generate(FComponentIdGenerator& IdGenerator, const UClass* Class);

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	FString GeneratedSchemaName;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	uint32 SchemaComponents[SCHEMA_Count] = {};

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	TMap<uint32, FActorSpecificSubobjectSchemaData> SubobjectData;
};

USTRUCT()
struct FDynamicSubobjectSchemaData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	uint32 SchemaComponents[SCHEMA_Count] = {};
};

// Schema data related to a Subobject class
USTRUCT()
struct FSubobjectSchemaData
{
	GENERATED_USTRUCT_BODY()

	static FSubobjectSchemaData Generate(FComponentIdGenerator& IdGenerator, const UClass* Class);

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	FString GeneratedSchemaName;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	TArray<FDynamicSubobjectSchemaData> DynamicSubobjectComponents;

	FORCEINLINE Worker_ComponentId GetDynamicSubobjectComponentId(int Idx, ESchemaComponentType ComponentType) const
	{
		Worker_ComponentId ComponentId = 0;
		if (Idx < DynamicSubobjectComponents.Num())
		{
			ComponentId = DynamicSubobjectComponents[Idx].SchemaComponents[ComponentType];
		}
		return ComponentId;
	}
};

UCLASS()
class SPATIALGDK_API USchemaDatabase : public UDataAsset
{
	GENERATED_BODY()

public:

	USchemaDatabase() : NextAvailableComponentId(SpatialConstants::STARTING_GENERATED_COMPONENT_ID) {}

	const FActorSchemaData& GetOrCreateActorSchemaData(const UClass* Class);
	const FSubobjectSchemaData& GetOrCreateSubobjectSchemaData(const UClass* Class);
	uint32 GetOrCreateLevelComponentId(const FString& Level);

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	TMap<FString, FActorSchemaData> ActorClassPathToSchema;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	TMap<FString, FSubobjectSchemaData> SubobjectClassPathToSchema;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	TMap<FString, uint32> LevelPathToComponentId;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	TMap<uint32, FString> ComponentIdToClassPath;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	TSet<uint32> LevelComponentIds;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	uint32 NextAvailableComponentId;
};

