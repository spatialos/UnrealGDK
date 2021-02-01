// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "GameFramework/GameModeBase.h"
#include "SpatialSnapshotTestGameMode.generated.h"

UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialSnapshotTestGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ASpatialSnapshotTestGameMode();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(CrossServer, Reliable)
	void CrossServerSetProperties();

	UFUNCTION()
	bool VerifyBool();

	UFUNCTION()
	bool VerifyInt32();

	UFUNCTION()
	bool VerifyInt64();

	UFUNCTION()
	bool VerifyFloat();

	UFUNCTION()
	bool VerifyString();

	UFUNCTION()
	bool VerifyName();

	UFUNCTION()
	bool VerifyIntArray();

	UPROPERTY(Replicated)
	bool bBoolProperty = false;

	UPROPERTY(Replicated)
	int32 Int32Property = 0;

	UPROPERTY(Replicated)
	int64 Int64Property = 0;

	UPROPERTY(Replicated)
	float FloatProperty = 0.0f;

	UPROPERTY(Replicated)
	FString StringProperty = "";

	UPROPERTY(Replicated)
	FName NameProperty = "";

	UPROPERTY(Replicated)
	TArray<int> IntArrayProperty;
};
