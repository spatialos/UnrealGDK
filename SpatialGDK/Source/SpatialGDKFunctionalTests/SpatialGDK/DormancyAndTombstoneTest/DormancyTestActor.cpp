// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "DormancyTestActor.h"

#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "Engine/Classes/Materials/Material.h"
#include "Net/UnrealNetwork.h"

ADormancyTestActor::ADormancyTestActor()
{
	bReplicates = true;
	TestIntProp = 0;

	GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'")));
	GetStaticMeshComponent()->SetMaterial(
		0, LoadObject<UMaterial>(nullptr, TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'")));

	NetDormancy = DORM_Initial; // By default dormant initially, as we have no way to correctly set this at runtime.
#if ENGINE_MINOR_VERSION < 24
	bHidden = true;
#else
	SetHidden(true);
#endif
}

void ADormancyTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADormancyTestActor, TestIntProp);
}
