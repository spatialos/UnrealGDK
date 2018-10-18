#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SchemaDatabase.generated.h"

USTRUCT(BlueprintType)
struct FActorSchemaData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SingleClientRepData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MultiClientRepData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HandoverData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ClientRPCs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ServerRPCs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NetMulticastRPCs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CrossServerRPCs;
};

USTRUCT(BlueprintType)
struct FSubobjectData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SingleClientRepData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 MultiClientRepData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 HandoverData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 ClientRPCs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 ServerRPCs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 NetMulticastRPCs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		int32 CrossServerRPCs;
};

UCLASS()
class SPATIALGDK_API USchemaDatabase : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TMap<UClass*, FActorSchemaData> ClassToSchema;
};
