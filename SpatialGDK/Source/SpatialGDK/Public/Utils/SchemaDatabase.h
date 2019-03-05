#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
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

USTRUCT()
struct FLevelData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere)
	TMap<FString, uint32> SublevelNameToComponentId;
};

UCLASS()
class SPATIALGDK_API USchemaDatabase : public UDataAsset
{
	GENERATED_BODY()

public:

	USchemaDatabase() : NextAvailableComponentId(SpatialConstants::STARTING_GENERATED_COMPONENT_ID) {}

	UPROPERTY(VisibleAnywhere)
	TMap<FString, FSchemaData> ClassPathToSchema;

	UPROPERTY(VisibleAnywhere)
	TMap<FString, FLevelData> LevelPathToLevelData;

	UPROPERTY(VisibleAnywhere)
	uint32 NextAvailableComponentId;
};

