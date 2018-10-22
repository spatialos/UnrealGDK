#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SpatialConstants.h"

#include "SchemaDatabase.generated.h"

USTRUCT()
struct FSubobjectSchemaData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	UClass* Class = nullptr;

	UPROPERTY()
	UObjectProperty* Property = nullptr;

	UPROPERTY()
	uint32 SchemaComponents[EComponentType::TYPE_Count] = {};
};

USTRUCT()
struct FSchemaData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	UClass* Class = nullptr;

	UPROPERTY()
	uint32 SchemaComponents[EComponentType::TYPE_Count] = {};

	UPROPERTY()
	TMap<uint32, FSubobjectSchemaData> SubobjectData;
};

UCLASS()
class SPATIALGDK_API USchemaDatabase : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<UClass*, FSchemaData> ClassToSchema;
};
