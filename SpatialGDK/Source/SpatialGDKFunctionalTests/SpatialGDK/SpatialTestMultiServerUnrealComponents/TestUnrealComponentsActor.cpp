// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestUnrealComponentsActor.h"
#include "TestUnrealComponents.h"

#include "Net/UnrealNetwork.h"

ATestUnrealComponentsActor::ATestUnrealComponentsActor()
{
	bReplicates = true;
	bAlwaysRelevant = true;

	StaticDataOnlyComponent = CreateDefaultSubobject<UDataOnlyComponent>(TEXT("DataOnlyComponent"));
	StaticDataAndHandoverComponent = CreateDefaultSubobject<UDataAndHandoverComponent>(TEXT("DataAndHandoverComponent"));
	StaticDataAndOwnerOnlyComponent = CreateDefaultSubobject<UDataAndOwnerOnlyComponent>(TEXT("DataAndOwnerOnlyComponent"));
	StaticDataAndInitialOnlyComponent = CreateDefaultSubobject<UDataAndInitialOnlyComponent>(TEXT("DataAndInitialOnlyComponent"));

	SetRootComponent(StaticDataOnlyComponent);
}

void ATestUnrealComponentsActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StaticDataOnlyComponent);
	DOREPLIFETIME(ThisClass, DynamicDataOnlyComponent);
	DOREPLIFETIME(ThisClass, StaticDataAndHandoverComponent);
	DOREPLIFETIME(ThisClass, DynamicDataAndHandoverComponent);
	DOREPLIFETIME(ThisClass, StaticDataAndOwnerOnlyComponent);
	DOREPLIFETIME(ThisClass, DynamicDataAndOwnerOnlyComponent);
	DOREPLIFETIME(ThisClass, StaticDataAndInitialOnlyComponent);
	DOREPLIFETIME(ThisClass, DynamicDataAndInitialOnlyComponent);
}

void ATestUnrealComponentsActor::SpawnDynamicComponents()
{
	DynamicDataOnlyComponent = SpawnDynamicComponent<UDataOnlyComponent>(TEXT("DynamicDataOnlyComponent"));
	DynamicDataAndHandoverComponent = SpawnDynamicComponent<UDataAndHandoverComponent>(TEXT("DynamicDataAndHandoverComponent"));
	DynamicDataAndOwnerOnlyComponent = SpawnDynamicComponent<UDataAndOwnerOnlyComponent>(TEXT("DynamicDataAndOwnerOnlyComponent"));
	DynamicDataAndInitialOnlyComponent = SpawnDynamicComponent<UDataAndInitialOnlyComponent>(TEXT("DynamicDataAndInitialOnlyComponent"));
}

bool ATestUnrealComponentsActor::AreAllDynamicComponentsValid() const
{
	bool bAllDynamicComponentsValid = true;

	bAllDynamicComponentsValid &= DynamicDataOnlyComponent != nullptr;
	bAllDynamicComponentsValid &= DynamicDataAndHandoverComponent != nullptr;
	bAllDynamicComponentsValid &= DynamicDataAndOwnerOnlyComponent != nullptr;
	bAllDynamicComponentsValid &= DynamicDataAndInitialOnlyComponent != nullptr;

	return bAllDynamicComponentsValid;
}
