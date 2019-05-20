#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/World.h"
#include "SpatialConstants.h"

#include "SchemaDatabase.generated.h"

// Schema data related to a default Subobject owned by a specific Actor class.
USTRUCT()
struct FActorSpecificSubobjectSchemaData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere)
	FString ClassPath;

	UPROPERTY(VisibleAnywhere)
	FName Name;

	UPROPERTY(VisibleAnywhere)
	uint32 SchemaComponents[SCHEMA_Count] = {};
};

// Schema data related to an Actor class
USTRUCT()
struct FActorSchemaData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere)
	uint32 SchemaComponents[SCHEMA_Count] = {};

	UPROPERTY(VisibleAnywhere)
	TMap<uint32, FActorSpecificSubobjectSchemaData> SubobjectData;
};

// Schema data related to a Subobject class
USTRUCT()
struct FSubobjectSchemaData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere)
	TArray<uint32[SCHEMA_Count]> DynamicSubobjectComponents;
};

UCLASS()
class SPATIALGDK_API USchemaDatabase : public UDataAsset
{
	GENERATED_BODY()

public:

	USchemaDatabase() : NextAvailableComponentId(SpatialConstants::STARTING_GENERATED_COMPONENT_ID) {}

	uint32 GetComponentIdFromLevelPath(const FString& LevelPath) const
	{
		FString CleanLevelPath = UWorld::RemovePIEPrefix(LevelPath);
		if (const uint32* ComponentId = LevelPathToComponentId.Find(CleanLevelPath))
		{
			return *ComponentId;
		}
		return SpatialConstants::INVALID_COMPONENT_ID;
	}

	UPROPERTY(VisibleAnywhere)
	TMap<FString, FActorSchemaData> ActorClassPathToSchema;

	UPROPERTY(VisibleAnywhere)
	TMap<FString, FSubobjectSchemaData> SubobjectClassPathToSchema;

	UPROPERTY(VisibleAnywhere)
	TMap<FString, uint32> LevelPathToComponentId;

	UPROPERTY(VisibleAnywhere)
	TSet<uint32> LevelComponentIds;

	UPROPERTY(VisibleAnywhere)
	uint32 NextAvailableComponentId;
};

