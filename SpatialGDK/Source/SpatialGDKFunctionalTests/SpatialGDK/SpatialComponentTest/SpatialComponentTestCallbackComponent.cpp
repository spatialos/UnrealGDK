// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialComponentTestCallbackComponent.h"

#include "SpatialComponentTestDummyComponent.h"

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
	TestComponent1->RegisterComponent();

	USpatialComponentTestDummyComponent* TestComponent2 = NewObject<USpatialComponentTestDummyComponent>(this);
	TestComponent2->RegisterComponent();

	// Remove 1 component
	TestComponent2->UnregisterComponent();
	TestComponent2->DestroyComponent();
	TestComponent2 = nullptr;
}
