// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialFunctionalTest.h"
#include "SpatialTestReplicationConditions.generated.h"

class ATestReplicationConditionsActor_AutonomousOnly;
class ATestReplicationConditionsActor_Common;
class ATestReplicationConditionsActor_Custom;
class ATestReplicationConditionsActor_Physics;

UCLASS()
class ASpatialTestReplicationConditions : public ASpatialFunctionalTest
{
	GENERATED_BODY()

public:
	ASpatialTestReplicationConditions();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PrepareTest() override;

	bool ActorsReady() const;

	void ProcessCommonActorProperties(bool bWrite, bool bCondIgnore[COND_Max]);

	void ProcessCustomActorProperties(ATestReplicationConditionsActor_Custom* Actor, bool bWrite, bool bCustomEnabled);

	void ProcessAutonomousOnlyActorProperties(bool bWrite, bool bAutonomousExpected, bool bSimulatedExpected);

	void ProcessPhysicsActorProperties(ATestReplicationConditionsActor_Physics* Actor, bool bWrite, bool bPhysicsEnabled,
									   bool bPhysicsExpected);

	UPROPERTY(Replicated)
	ATestReplicationConditionsActor_Common* TestActor_Common;

	UPROPERTY(Replicated)
	ATestReplicationConditionsActor_Custom* TestActor_CustomEnabled;

	UPROPERTY(Replicated)
	ATestReplicationConditionsActor_Custom* TestActor_CustomDisabled;

	UPROPERTY(Replicated)
	ATestReplicationConditionsActor_AutonomousOnly* TestActor_AutonomousOnly;

	UPROPERTY(Replicated)
	ATestReplicationConditionsActor_Physics* TestActor_PhysicsEnabled;

	// UPROPERTY(Replicated)
	// ATestReplicationConditionsActor_Physics* TestActor_PhysicsDisabled;

	const FVector ActorSpawnPosition = FVector(0.0f, 0.0f, 50.0f);
	bool bSpatialEnabled;

	enum EPropertyReplicationStage
	{
		STAGE_InitialReplication,
		STAGE_UpdateReplication
	};

	EPropertyReplicationStage ReplicationStage;
	int32 PropertyOffset;

private:
	void Action(int32& Source, int32 Expected, ELifetimeCondition Cond, bool bWrite, bool bCondIgnore[COND_Max], FString AdditionalText);
};
