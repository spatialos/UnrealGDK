// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialAuthorityTest.generated.h"

class ASpatialAuthorityTestActor;
class ASpatialAuthorityTestReplicatedActor;
enum class ESpatialHasAuthority : uint8;

/** Check SpatialAuthorityTest.cpp for Test explanation. */
UCLASS()
class SPATIALGDKFUNCTIONALTESTS_API ASpatialAuthorityTest : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialAuthorityTest();

	virtual void PrepareTest() override;

	void CheckNumActorsInLevel();

	void CheckDoesNotMigrate(ASpatialAuthorityTestActor* Actor, int ServerId);

	void CheckMigration(int StartServerId, int EndServerId);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void FinishStep() override
	{
		ResetTimer();
		Super::FinishStep();
	};

	void ResetTimer() { Timer = 0.5; };

	bool VerifyTestActor(ASpatialAuthorityTestActor* Actor, ESpatialHasAuthority ExpectedAuthority, int AuthorityOnBeginPlay,
						 int AuthorityOnTick, int NumAuthorityGains, int NumAuthorityLosses);

	UFUNCTION(CrossServer, Reliable)
	void CrossServerSetDynamicReplicatedActor(ASpatialAuthorityTestReplicatedActor* Actor);

	UFUNCTION(CrossServer, Reliable)
	void CrossServerNotifyHadAuthorityOverGameMode();

	UFUNCTION(CrossServer, Reliable)
	void CrossServerNotifyHadAuthorityOverGameState();

	UPROPERTY(EditAnywhere, Category = "Default")
	ASpatialAuthorityTestActor* LevelActor;

	UPROPERTY(EditAnywhere, Category = "Default")
	ASpatialAuthorityTestReplicatedActor* LevelReplicatedActor;

	UPROPERTY(EditAnywhere, Category = "Default")
	ASpatialAuthorityTestReplicatedActor* LevelReplicatedActorOnBorder;

	// This needs to be a position that belongs to Server 1.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server1Position;

	// This needs to be a position that belongs to Server 2.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server2Position;

	// This needs to be a position that belongs to Server 3.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server3Position;

	// This needs to be a position that belongs to Server 4.
	UPROPERTY(EditAnywhere, Category = "Default")
	FVector Server4Position;

	// This needs to be a position on the border between all servers.
	FVector BorderPosition;

	UPROPERTY(Replicated)
	ASpatialAuthorityTestReplicatedActor* DynamicReplicatedActor;

	UPROPERTY()
	ASpatialAuthorityTestActor* DynamicNonReplicatedActor;

	UPROPERTY(Replicated)
	int NumHadAuthorityOverGameMode;

	UPROPERTY(Replicated)
	int NumHadAuthorityOverGameState;

	// Local timer used for some active waits.
	float Timer;
};
