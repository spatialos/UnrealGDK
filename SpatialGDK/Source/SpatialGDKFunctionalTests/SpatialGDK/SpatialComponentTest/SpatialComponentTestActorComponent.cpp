// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialComponentTestActorComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"

USpatialComponentTestActorComponent::USpatialComponentTestActorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f;

	SetIsReplicatedByDefault(true);
}

void USpatialComponentTestActorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void USpatialComponentTestActorComponent::OnAuthorityGained()
{
	AddAndRemoveComponents();
}

void USpatialComponentTestActorComponent::OnAuthorityLost()
{
	AddAndRemoveComponents();
}

void USpatialComponentTestActorComponent::OnActorReady(bool bHasAuthority)
{
	AddAndRemoveComponents();
}

void USpatialComponentTestActorComponent::AddAndRemoveComponents()
{
	// Add 2 components
	UStaticMeshComponent* TestSceneComponent1 = NewObject<UStaticMeshComponent>(this);
	TestSceneComponent1->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	TestSceneComponent1->RegisterComponent();

	UStaticMeshComponent* TestSceneComponent2 = NewObject<UStaticMeshComponent>(this);
	TestSceneComponent2->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	TestSceneComponent2->RegisterComponent();

	// Remove 1 component
	TestSceneComponent2->UnregisterComponent();
	TestSceneComponent2->DestroyComponent();
	TestSceneComponent2 = nullptr;
}
