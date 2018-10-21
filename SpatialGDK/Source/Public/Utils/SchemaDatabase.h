#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SchemaDatabase.generated.h"

USTRUCT()
struct FSubobjectSchemaData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	UClass* Class = nullptr;

	UPROPERTY()
	uint32 SchemaComponents[7] = {};
};

USTRUCT()
struct FSchemaData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	UClass* Class = nullptr;

	UPROPERTY()
	uint32 SchemaComponents[7] = {};

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
