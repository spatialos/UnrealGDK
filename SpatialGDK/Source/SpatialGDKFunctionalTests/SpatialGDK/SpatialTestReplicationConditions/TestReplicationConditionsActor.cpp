// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestReplicationConditionsActor.h"

#include "Net/UnrealNetwork.h"
#include "PhysXPublicCore.h"

using namespace PhysicsInterfaceTypes;

UTestReplicationConditionsComponentBase::UTestReplicationConditionsComponentBase()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

ATestReplicationConditionsActorBase::ATestReplicationConditionsActorBase()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ATestReplicationConditionsActorBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, DynamicComponents);
}

bool ATestReplicationConditionsActorBase::AreAllDynamicComponentsValid() const
{
	bool bAllDynamicComponentsValid = true;

	for (const auto& Component : DynamicComponents)
	{
		bAllDynamicComponentsValid &= Component != nullptr;
	}

	return bAllDynamicComponentsValid;
}

void UTestReplicationConditionsComponent_Common::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CondNone_Var);
	// COND_InitialOnly - tested in SpatialTestInitialOnly* tests
	DOREPLIFETIME_CONDITION(ThisClass, CondOwnerOnly_Var, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, CondSkipOwner_Var, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOnly_Var, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ThisClass, CondAutonomousOnly_Var, COND_AutonomousOnly);
	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOrPhysics_Var, COND_SimulatedOrPhysics);
	// COND_InitialOrOwner - not supported
	// COND_Custom - test in ATesetReplicationConditionsActor_Custom
	DOREPLIFETIME_CONDITION(ThisClass, CondReplayOrOwner_Var, COND_ReplayOrOwner);
	// COND_ReplayOnly - not tested
	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOnlyNoReplay_Var, COND_SimulatedOnlyNoReplay);
	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOrPhysicsNoReplay_Var, COND_SimulatedOrPhysics);
	DOREPLIFETIME_CONDITION(ThisClass, CondSkipReplay_Var, COND_SkipReplay);

	// Our added conditions
	DOREPLIFETIME_CONDITION(ThisClass, CondServerOnly_Var, COND_ServerOnly);
}

ATestReplicationConditionsActor_Common::ATestReplicationConditionsActor_Common()
{
	StaticComponent =
		CreateDefaultSubobject<UTestReplicationConditionsComponent_Common>(TEXT("UTestReplicationConditionsComponent_Common"));

	SetRootComponent(StaticComponent);
}

void ATestReplicationConditionsActor_Common::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CondNone_Var);
	// COND_InitialOnly - tested in SpatialTestInitialOnly* tests
	DOREPLIFETIME_CONDITION(ThisClass, CondOwnerOnly_Var, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ThisClass, CondSkipOwner_Var, COND_SkipOwner);
	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOnly_Var, COND_SimulatedOnly);
	DOREPLIFETIME_CONDITION(ThisClass, CondAutonomousOnly_Var, COND_AutonomousOnly);
	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOrPhysics_Var, COND_SimulatedOrPhysics);
	// COND_InitialOrOwner - not supported
	// COND_Custom - test in ATesetReplicationConditionsActor_Custom
	DOREPLIFETIME_CONDITION(ThisClass, CondReplayOrOwner_Var, COND_ReplayOrOwner);
	// COND_ReplayOnly - not tested
	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOnlyNoReplay_Var, COND_SimulatedOnlyNoReplay);
	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOrPhysicsNoReplay_Var, COND_SimulatedOrPhysics);
	DOREPLIFETIME_CONDITION(ThisClass, CondSkipReplay_Var, COND_SkipReplay);

	// Our added conditions
	DOREPLIFETIME_CONDITION(ThisClass, CondServerOnly_Var, COND_ServerOnly);

	DOREPLIFETIME(ThisClass, StaticComponent);
	DOREPLIFETIME(ThisClass, DynamicComponent);
}

void ATestReplicationConditionsActor_Common::SpawnDynamicComponents()
{
	DynamicComponent =
		SpawnDynamicComponent<UTestReplicationConditionsComponent_Common>(TEXT("DynamicTestReplicationConditionsComponent_Common"));
}

void UTestReplicationConditionsComponent_Custom::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, CondCustom_Var, COND_Custom);
}

void UTestReplicationConditionsComponent_Custom::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE(ThisClass, CondCustom_Var, bCustomReplicationEnabled);
}

void UTestReplicationConditionsComponent_Custom::SetCustomReplicationEnabled(bool bEnabled)
{
	bCustomReplicationEnabled = bEnabled;
}

ATestReplicationConditionsActor_Custom::ATestReplicationConditionsActor_Custom()
{
	StaticComponent =
		CreateDefaultSubobject<UTestReplicationConditionsComponent_Custom>(TEXT("UTestReplicationConditionsComponent_Custom"));

	SetRootComponent(StaticComponent);
}

void ATestReplicationConditionsActor_Custom::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, CondCustom_Var, COND_Custom);
	DOREPLIFETIME(ThisClass, StaticComponent);
	DOREPLIFETIME(ThisClass, DynamicComponent);
}

void ATestReplicationConditionsActor_Custom::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	DOREPLIFETIME_ACTIVE_OVERRIDE(ThisClass, CondCustom_Var, bCustomReplicationEnabled);
}

void ATestReplicationConditionsActor_Custom::SpawnDynamicComponents()
{
	DynamicComponent =
		SpawnDynamicComponent<UTestReplicationConditionsComponent_Custom>(TEXT("DynamicTestReplicationConditionsComponent_Custom"));
}

void ATestReplicationConditionsActor_Custom::SetCustomReplicationEnabled(bool bEnabled)
{
	bCustomReplicationEnabled = bEnabled;

	check(StaticComponent != nullptr);
	StaticComponent->SetCustomReplicationEnabled(bEnabled);

	check(DynamicComponent != nullptr);
	DynamicComponent->SetCustomReplicationEnabled(bEnabled);
}

void UTestReplicationConditionsComponent_AutonomousOnly::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, CondAutonomousOnly_Var, COND_AutonomousOnly);
	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOnly_Var, COND_SimulatedOnly);
}

ATestReplicationConditionsActor_AutonomousOnly::ATestReplicationConditionsActor_AutonomousOnly()
{
	StaticComponent = CreateDefaultSubobject<UTestReplicationConditionsComponent_AutonomousOnly>(
		TEXT("UTestReplicationConditionsComponent_AutonomousOnly"));

	SetRootComponent(StaticComponent);
}

void ATestReplicationConditionsActor_AutonomousOnly::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, CondAutonomousOnly_Var, COND_AutonomousOnly);
	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOnly_Var, COND_SimulatedOnly);
	DOREPLIFETIME(ThisClass, StaticComponent);
	DOREPLIFETIME(ThisClass, DynamicComponent);
}

void ATestReplicationConditionsActor_AutonomousOnly::SpawnDynamicComponents()
{
	DynamicComponent = SpawnDynamicComponent<UTestReplicationConditionsComponent_AutonomousOnly>(
		TEXT("DynamicTestReplicationConditionsComponent_AutonomousOnly"));
}

void UTestReplicationConditionsComponent_Physics::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOrPhysics_Var, COND_SimulatedOrPhysics);
	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOrPhysicsNoReplay_Var, COND_SimulatedOrPhysicsNoReplay);
}

ATestReplicationConditionsActor_Physics::ATestReplicationConditionsActor_Physics()
{
	StaticComponent =
		CreateDefaultSubobject<UTestReplicationConditionsComponent_Physics>(TEXT("UTestReplicationConditionsComponent_Physics"));

	SetRootComponent(StaticComponent);
}

void ATestReplicationConditionsActor_Physics::OnRep_ReplicatedMovement()
{
	InitFakePhysics();
	Super::OnRep_ReplicatedMovement();
}

void ATestReplicationConditionsActor_Physics::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOrPhysics_Var, COND_SimulatedOrPhysics);
	DOREPLIFETIME_CONDITION(ThisClass, CondSimulatedOrPhysicsNoReplay_Var, COND_SimulatedOrPhysicsNoReplay);
	DOREPLIFETIME(ThisClass, StaticComponent);
	DOREPLIFETIME(ThisClass, DynamicComponent);
}

void ATestReplicationConditionsActor_Physics::SpawnDynamicComponents()
{
	DynamicComponent =
		SpawnDynamicComponent<UTestReplicationConditionsComponent_Physics>(TEXT("DynamicTestReplicationConditionsComponent_Physics"));
}

void ATestReplicationConditionsActor_Physics::InitFakePhysics()
{
	// This setup is required to have Unreal attempt to replicate any physics information, including Actor.ReplicatedMovement.
	// Actor.ReplicatedMovement contains bRepPhysics, which is necessary to replicate to clients so that
	// FSpatialConditionMapFilter can check it when determining the value of the COND_*Physics conditions.
	if (UPrimitiveComponent* RootPrimComp = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		if (FBodyInstance* BodyInstance = RootPrimComp->GetBodyInstance())
		{
			if (BodyInstance->BodySetup == nullptr)
			{
				BodySetup = NewObject<UBodySetup>(this, NAME_None);
				BodySetup->CollisionTraceFlag = CTF_UseSimpleAndComplex;
				BodySetup->BodySetupGuid = FGuid::NewGuid();
				BodyInstance->BodySetup = BodySetup;

#if WITH_PHYSX
				RigidActor = GPhysXSDK->createRigidStatic(U2PTransform(FTransform::Identity));
				BodyInstance->ActorHandle.SyncActor = RigidActor;
#endif
			}
		}
	}
}

void ATestReplicationConditionsActor_Physics::SetPhysicsEnabled(bool bEnabled)
{
	InitFakePhysics();

	GetReplicatedMovement_Mutable().bRepPhysics = bEnabled;
	SetReplicatingMovement(bEnabled);

	if (UPrimitiveComponent* RootPrimComp = Cast<UPrimitiveComponent>(GetRootComponent()))
	{
		RootPrimComp->SetSimulatePhysics(bEnabled);
	}
}
