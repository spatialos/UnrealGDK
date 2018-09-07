#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SchemaDatabase.generated.h"

USTRUCT(BlueprintType)
struct FSchemaData
{
	GENERATED_USTRUCT_BODY()

	FSchemaData()
		: SingleClientRepData(0)
		, MultiClientRepData(0)
		, HandoverData(0)
		, ClientRPCs(0)
		, ServerRPCs(0)
		, CrossServerRPCs(0)
	{}

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
	TMap<UClass*, FSchemaData> ClassToSchema;
};
