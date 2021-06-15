// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ReplicatedTestActorBase.h"
#include "Components/StaticMeshComponent.h"
#include "EngineClasses/SpatialNetDriver.h"
#include "Materials/Material.h"

AReplicatedTestActorBase::AReplicatedTestActorBase()
{
	bReplicates = true;
	SetReplicateMovement(true);

	CubeComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeComponent"));
	CubeComponent->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")));
	CubeComponent->SetMaterial(0,
							   LoadObject<UMaterial>(nullptr, TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'")));
	CubeComponent->SetVisibility(true);

	RootComponent = CubeComponent;
}
PRAGMA_DISABLE_OPTIMIZATION
static void HackWreckHavok()
{
	// GWorld->SpawnActor<AEvilReplicatedTestActor>();
}

void AEvilReplicatedTestActor::PostCDOContruct()
{
	Super::PostCDOContruct();

	if (HasAnyFlags(EObjectFlags::RF_ClassDefaultObject))
	{
		USpatialNetDriver::BeforeStartup.AddStatic(&HackWreckHavok);
	}
}

bool AEvilReplicatedTestActor::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	const bool bSuperSucceeded = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	// Destroy();
	GetWorld()->GetTimerManager().SetTimerForNextTick([this] {
		Destroy();
	});

	return bSuperSucceeded;
}
PRAGMA_ENABLE_OPTIMIZATION
