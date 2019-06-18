#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/World.h"
#include "SpatialConstants.h"

#include "SchemaDatabase.generated.h"

USTRUCT()
struct FSubobjectSchemaData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	FString ClassPath;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	FName Name;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	uint32 SchemaComponents[SCHEMA_Count] = {};
};

USTRUCT()
struct FSchemaData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	FString GeneratedSchemaName;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	uint32 SchemaComponents[SCHEMA_Count] = {};

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	TMap<uint32, FSubobjectSchemaData> SubobjectData;
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

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	TMap<FString, FSchemaData> ClassPathToSchema;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	TMap<FString, uint32> LevelPathToComponentId;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	TSet<uint32> LevelComponentIds;

	UPROPERTY(Category = "SpatialGDK", VisibleAnywhere)
	uint32 NextAvailableComponentId;
};

