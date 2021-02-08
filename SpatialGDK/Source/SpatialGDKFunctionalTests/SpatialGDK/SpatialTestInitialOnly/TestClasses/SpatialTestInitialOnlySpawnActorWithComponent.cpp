// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestInitialOnlySpawnActorWithComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ASpatialTestInitialOnlySpawnActorWithComponent::ASpatialTestInitialOnlySpawnActorWithComponent()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ASpatialTestInitialOnlySpawnActorWithComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestInitialOnlySpawnActorWithComponent, OnSpawnComponent);
	DOREPLIFETIME(ASpatialTestInitialOnlySpawnActorWithComponent, PostInitializeComponent);
	DOREPLIFETIME(ASpatialTestInitialOnlySpawnActorWithComponent, LateAddedComponent);
}

void ASpatialTestInitialOnlySpawnActorWithComponent::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (HasAuthority() && PostInitializeComponent == nullptr)
	{
		PostInitializeComponent = NewObject<USpatialTestInitialOnlySpawnComponent>(this, TEXT("PostInitializeComponent1"));
		PostInitializeComponent->SetupAttachment(GetRootComponent());
		PostInitializeComponent->RegisterComponent();
	}
}
