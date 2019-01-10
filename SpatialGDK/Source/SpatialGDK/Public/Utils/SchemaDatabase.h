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

UCLASS()
class SPATIALGDK_API USchemaDatabase : public UDataAsset
{
	GENERATED_BODY()

	USchemaDatabase();

public:
	UPROPERTY(VisibleAnywhere)
	TMap<FString, FSchemaData> ClassPathToSchema;

	UPROPERTY(VisibleAnywhere)
	uint32 NextAvailableComponentId;
};

