// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialComponentTestCallbackComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "LoadBalancing/AbstractLBStrategy.h"
#include "Net/UnrealNetwork.h"
#include "SpatialComponentTestDummyComponent.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"

USpatialComponentTestCallbackComponent::USpatialComponentTestCallbackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void USpatialComponentTestCallbackComponent::OnAuthorityGained()
{
	AddAndRemoveComponents();
}

void USpatialComponentTestCallbackComponent::OnAuthorityLost()
{
	AddAndRemoveComponents();
}

void USpatialComponentTestCallbackComponent::OnActorReady(bool bHasAuthority)
{
	AddAndRemoveComponents();
}

void USpatialComponentTestCallbackComponent::OnClientOwnershipGained()
{
	AddAndRemoveComponents();
}

void USpatialComponentTestCallbackComponent::OnClientOwnershipLost()
{
	AddAndRemoveComponents();
}

void USpatialComponentTestCallbackComponent::AddAndRemoveComponents()
{
	// Add 2 components
	USpatialComponentTestDummyComponent* TestComponent1 = NewObject<USpatialComponentTestDummyComponent>(this);
	TestComponent1->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	TestComponent1->RegisterComponent();

	USpatialComponentTestDummyComponent* TestComponent2 = NewObject<USpatialComponentTestDummyComponent>(this);
	TestComponent2->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
	TestComponent2->RegisterComponent();

	// Remove 1 component
	TestComponent2->UnregisterComponent();
	TestComponent2->DestroyComponent();
	TestComponent2 = nullptr;
}
