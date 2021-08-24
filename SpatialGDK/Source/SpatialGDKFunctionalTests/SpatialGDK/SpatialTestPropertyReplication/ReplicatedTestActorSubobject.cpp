// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedTestActorSubobject.h"
#include "Net/UnrealNetwork.h"

UReplicatedSubobject::UReplicatedSubobject()
{
	SetIsReplicatedByDefault(true);
	TestReplicatedProperty = 0;
}

void UReplicatedSubobject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UReplicatedSubobject, TestReplicatedProperty);
}

AReplicatedTestActorSubobject::AReplicatedTestActorSubobject()
{
	bReplicates = true;
	ReplicatedSubobject = CreateDefaultSubobject<UReplicatedSubobject>("Subobject");
}

void AReplicatedTestActorSubobject::OnAuthorityGained()
{
	Super::OnAuthorityGained();
}

void AReplicatedTestActorSubobject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedTestActorSubobject, ReplicatedSubobject);
}
