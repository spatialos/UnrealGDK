// Copyright (c) Improbable Worlds Ltd, All Rights Reserved


#include "ReplicatedVisibilityTestActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"

AReplicatedVisibilityTestActor::AReplicatedVisibilityTestActor()
{
	bNetLoadOnClient = false;
	bNetLoadOnNonAuthServer = true;
	SetActorEnableCollision(false);

}

void AReplicatedVisibilityTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
