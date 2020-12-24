// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "SpatialCommonTypes.h"
#include "SpatialFunctionalTest.h"

#include "GameFramework/GameModeBase.h"

#include "GameModeReplicationTest.generated.h"

UCLASS(BlueprintType)
class SPATIALGDKFUNCTIONALTESTS_API AGameModeReplicationTestGameMode : public AGameModeBase
{
public:
	GENERATED_BODY()

	AGameModeReplicationTestGameMode();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const override;

	constexpr static int StartingValue = 0;

	constexpr static int UpdatedValue = 500;

	UPROPERTY(Replicated, Transient)
	int ReplicatedValue = StartingValue;
};

UCLASS(BlueprintType)
class SPATIALGDKFUNCTIONALTESTS_API AGameModeReplicationTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	AGameModeReplicationTest();

	UFUNCTION(CrossServer, Reliable)
	void MarkWorkerGameModeAuthority(bool bHasGameModeAuthority);

	virtual void PrepareTest() override;

	int AuthorityServersCount = 0;

	int ServerResponsesCount = 0;

	float TimeWaited = 0;
};
