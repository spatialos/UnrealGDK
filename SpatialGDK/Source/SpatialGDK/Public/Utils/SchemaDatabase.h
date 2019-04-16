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

	UPROPERTY(VisibleAnywhere)
	FString ClassPath;

	UPROPERTY(VisibleAnywhere)
	FName Name;

	UPROPERTY(VisibleAnywhere)
	uint32 SchemaComponents[SCHEMA_Count] = {};
};

USTRUCT()
struct FSchemaData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere)
	uint32 SchemaComponents[SCHEMA_Count] = {};

	UPROPERTY(VisibleAnywhere)
	TMap<uint32, FSubobjectSchemaData> SubobjectData;
};

UCLASS()
class SPATIALGDK_API USchemaDatabase : public UDataAsset
{
	GENERATED_BODY()

public:

	USchemaDatabase() : NextAvailableComponentId(SpatialConstants::STARTING_GENERATED_COMPONENT_ID) {}

	bool GetComponentIdFromLevelPath(const FString& LevelPath, uint32& OutComponentId) const
	{
		FString CleanLevelPath = UWorld::RemovePIEPrefix(LevelPath);
		if (const uint32* ComponentId = LevelPathToComponentId.Find(CleanLevelPath))
		{
			OutComponentId = *ComponentId;
			return true;
		}
		return false;
	}

	UPROPERTY(VisibleAnywhere)
	TMap<FString, FSchemaData> ClassPathToSchema;

	UPROPERTY(VisibleAnywhere)
	TMap<FString, uint32> LevelPathToComponentId;

	UPROPERTY(VisibleAnywhere)
	TSet<uint32> LevelComponentIds;

	UPROPERTY(VisibleAnywhere)
	uint32 NextAvailableComponentId;
};

