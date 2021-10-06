// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TestReplicationConditionsActor.generated.h"

UCLASS()
class UTestReplicationConditionsComponentBase : public USceneComponent
{
	GENERATED_BODY()

public:
	UTestReplicationConditionsComponentBase();
};

UCLASS()
class ATestReplicationConditionsActorBase : public AActor
{
	GENERATED_BODY()

public:
	ATestReplicationConditionsActorBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	bool AreAllDynamicComponentsValid() const;

	UPROPERTY(Replicated)
	TArray<USceneComponent*> DynamicComponents;

protected:
	template <class T>
	T* SpawnDynamicComponent(FName Name)
	{
		T* NewComponent = NewObject<T>(this, Name);
		NewComponent->SetupAttachment(GetRootComponent());
		NewComponent->RegisterComponent();
		DynamicComponents.Add(NewComponent);
		return NewComponent;
	}
};

UCLASS()
class UTestReplicationConditionsComponent_Common : public UTestReplicationConditionsComponentBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int32 CondNone_Var;

	UPROPERTY(Replicated)
	int32 CondOwnerOnly_Var;

	UPROPERTY(Replicated)
	int32 CondSkipOwner_Var;

	UPROPERTY(Replicated)
	int32 CondSimulatedOnly_Var;

	UPROPERTY(Replicated)
	int32 CondAutonomousOnly_Var;

	UPROPERTY(Replicated)
	int32 CondSimulatedOrPhysics_Var;

	UPROPERTY(Replicated)
	int32 CondReplayOrOwner_Var;

	UPROPERTY(Replicated)
	int32 CondSimulatedOnlyNoReplay_Var;

	UPROPERTY(Replicated)
	int32 CondSimulatedOrPhysicsNoReplay_Var;

	UPROPERTY(Replicated)
	int32 CondSkipReplay_Var;

	UPROPERTY(Replicated)
	int32 CondServerOnly_Var;
};

/**
 * A replicated, always relevant Actor used to test Unreal replication conditions.
 */

UCLASS()
class ATestReplicationConditionsActor_Common : public ATestReplicationConditionsActorBase
{
	GENERATED_BODY()

public:
	ATestReplicationConditionsActor_Common();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SpawnDynamicComponents();

	UPROPERTY(Replicated)
	int32 CondNone_Var;

	UPROPERTY(Replicated)
	int32 CondOwnerOnly_Var;

	UPROPERTY(Replicated)
	int32 CondSkipOwner_Var;

	UPROPERTY(Replicated)
	int32 CondSimulatedOnly_Var;

	UPROPERTY(Replicated)
	int32 CondAutonomousOnly_Var;

	UPROPERTY(Replicated)
	int32 CondSimulatedOrPhysics_Var;

	UPROPERTY(Replicated)
	int32 CondReplayOrOwner_Var;

	UPROPERTY(Replicated)
	int32 CondSimulatedOnlyNoReplay_Var;

	UPROPERTY(Replicated)
	int32 CondSimulatedOrPhysicsNoReplay_Var;

	UPROPERTY(Replicated)
	int32 CondSkipReplay_Var;

	UPROPERTY(Replicated)
	int32 CondServerOnly_Var;

	UPROPERTY(Replicated)
	UTestReplicationConditionsComponent_Common* StaticComponent;

	UPROPERTY(Replicated)
	UTestReplicationConditionsComponent_Common* DynamicComponent;
};

UCLASS()
class UTestReplicationConditionsComponent_Custom : public UTestReplicationConditionsComponentBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	void SetCustomReplicationEnabled(bool bEnabled);

	UPROPERTY(Replicated)
	int32 CondCustom_Var;

	bool bCustomReplicationEnabled;
};

UCLASS()
class ATestReplicationConditionsActor_Custom : public ATestReplicationConditionsActorBase
{
	GENERATED_BODY()

public:
	ATestReplicationConditionsActor_Custom();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override;

	void SpawnDynamicComponents();

	void SetCustomReplicationEnabled(bool bEnabled);

	UPROPERTY(Replicated)
	int32 CondCustom_Var;

	UPROPERTY(Replicated)
	UTestReplicationConditionsComponent_Custom* StaticComponent;

	UPROPERTY(Replicated)
	UTestReplicationConditionsComponent_Custom* DynamicComponent;

	bool bCustomReplicationEnabled;
};

UCLASS()
class UTestReplicationConditionsComponent_AutonomousOnly : public UTestReplicationConditionsComponentBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int32 CondAutonomousOnly_Var;

	UPROPERTY(Replicated)
	int32 CondSimulatedOnly_Var;
};

UCLASS()
class ATestReplicationConditionsActor_AutonomousOnly : public ATestReplicationConditionsActorBase
{
	GENERATED_BODY()

public:
	ATestReplicationConditionsActor_AutonomousOnly();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SpawnDynamicComponents();

	UPROPERTY(Replicated)
	int32 CondAutonomousOnly_Var;

	UPROPERTY(Replicated)
	int32 CondSimulatedOnly_Var;

	UPROPERTY(Replicated)
	UTestReplicationConditionsComponent_AutonomousOnly* StaticComponent;

	UPROPERTY(Replicated)
	UTestReplicationConditionsComponent_AutonomousOnly* DynamicComponent;
};

UCLASS()
class UTestReplicationConditionsPrimitiveComponent : public UPrimitiveComponent
{
	GENERATED_BODY()

public:
	virtual bool IsSimulatingPhysics(FName BoneName = NAME_None) const override;
};

UCLASS()
class UTestReplicationConditionsComponent_Physics : public UTestReplicationConditionsComponentBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	int32 CondSimulatedOrPhysics_Var;

	UPROPERTY(Replicated)
	int32 CondSimulatedOrPhysicsNoReplay_Var;
};

UCLASS()
class ATestReplicationConditionsActor_Physics : public ATestReplicationConditionsActorBase
{
	GENERATED_BODY()

public:
	ATestReplicationConditionsActor_Physics();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SpawnDynamicComponents();

	void SetPhysicsEnabled(bool bEnabled);

	UPROPERTY(Replicated)
	int32 CondSimulatedOrPhysics_Var;

	UPROPERTY(Replicated)
	int32 CondSimulatedOrPhysicsNoReplay_Var;

	UPROPERTY(Replicated)
	UTestReplicationConditionsComponent_Physics* StaticComponent;

	UPROPERTY(Replicated)
	UTestReplicationConditionsComponent_Physics* DynamicComponent;
};
