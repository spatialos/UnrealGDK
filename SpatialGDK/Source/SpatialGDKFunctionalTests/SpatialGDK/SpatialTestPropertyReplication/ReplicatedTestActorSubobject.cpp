// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedTestActorSubobject.h"
#include "Net/UnrealNetwork.h"

AReplicatedTestActorSubobject::AReplicatedTestActorSubobject()
{
	ReplicatedSubActor = nullptr;
}

void AReplicatedTestActorSubobject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AReplicatedTestActorSubobject, ReplicatedSubActor);
}

void AReplicatedTestActorSubobject::OnAuthorityGained()
{
	Super::OnAuthorityGained();

	// Spawn sub-actor
	ReplicatedSubActor =
		GetWorld()->SpawnActor<AReplicatedTestActor>(FVector(0.0f, 0.0f, 0.0f), FRotator::ZeroRotator, FActorSpawnParameters());
	ReplicatedSubActor->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
}
