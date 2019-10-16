// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaGenObjectStub.h"

#include "Net/UnrealNetwork.h"

void USchemaGenObjectStub::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USchemaGenObjectStub, IntValue);
	DOREPLIFETIME(USchemaGenObjectStub, BoolValue);
}

void ASpatialTypeActorWithActorComponent ::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTypeActorWithActorComponent, SpatialActorComponent);
}

void ASpatialTypeActorWithMultipleActorComponents ::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTypeActorWithMultipleActorComponents, FirstSpatialActorComponent);
	DOREPLIFETIME(ASpatialTypeActorWithMultipleActorComponents, SecondSpatialActorComponent);
}

void ASpatialTypeActorWithMultipleObjectComponents ::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTypeActorWithMultipleObjectComponents, FirstSpatialObjectComponent);
	DOREPLIFETIME(ASpatialTypeActorWithMultipleObjectComponents, SecondSpatialObjectComponent);
}
