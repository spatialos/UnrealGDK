// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDynamicComponentActor.h"
#include "TestDynamicComponent.h"

#include "Net/UnrealNetwork.h"

ATestDynamicComponentActor::ATestDynamicComponentActor()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ATestDynamicComponentActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ATestDynamicComponentActor, OnSpawnComponent);
	DOREPLIFETIME(ATestDynamicComponentActor, PostInitializeComponent);
	DOREPLIFETIME(ATestDynamicComponentActor, LateAddedComponent);
}

void ATestDynamicComponentActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority() && PostInitializeComponent == nullptr)
	{
		PostInitializeComponent = NewObject<UTestDynamicComponent>(this, TEXT("PostInitializeDynamicComponent"));
		PostInitializeComponent->SetupAttachment(GetRootComponent());
		PostInitializeComponent->RegisterComponent();
	}
}
