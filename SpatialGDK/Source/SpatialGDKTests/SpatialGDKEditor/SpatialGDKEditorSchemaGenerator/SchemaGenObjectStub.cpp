// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SchemaGenObjectStub.h"

#include "Net/UnrealNetwork.h"

void USchemaGenObjectStub::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USchemaGenObjectStub, IntValue);
	DOREPLIFETIME(USchemaGenObjectStub, BoolValue);
}

void USchemaGenObjectStubCondOwnerOnly::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(USchemaGenObjectStubCondOwnerOnly, IntValue, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(USchemaGenObjectStubCondOwnerOnly, BoolValue, COND_OwnerOnly);
}

void USchemaGenObjectStubInitialOnly::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(USchemaGenObjectStubInitialOnly, IntValue, COND_InitialOnly);
	DOREPLIFETIME_CONDITION(USchemaGenObjectStubInitialOnly, BoolValue, COND_InitialOnly);
}

ASpatialTypeActorWithActorComponent::ASpatialTypeActorWithActorComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SpatialActorComponent = CreateDefaultSubobject<USpatialTypeActorComponent>(TEXT("SpatialActorComponent"));
}

void ASpatialTypeActorWithActorComponent ::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTypeActorWithActorComponent, SpatialActorComponent);
}

ASpatialTypeActorWithMultipleActorComponents::ASpatialTypeActorWithMultipleActorComponents(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FirstSpatialActorComponent = CreateDefaultSubobject<USpatialTypeActorComponent>(TEXT("FirstSpatialActorComponent"));
	SecondSpatialActorComponent = CreateDefaultSubobject<USpatialTypeActorComponent>(TEXT("SecondSpatialActorComponent"));
}

void ASpatialTypeActorWithMultipleActorComponents ::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTypeActorWithMultipleActorComponents, FirstSpatialActorComponent);
	DOREPLIFETIME(ASpatialTypeActorWithMultipleActorComponents, SecondSpatialActorComponent);
}

ASpatialTypeActorWithMultipleObjectComponents::ASpatialTypeActorWithMultipleObjectComponents(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FirstSpatialObjectComponent = CreateDefaultSubobject<USpatialTypeObjectStub>(TEXT("FirstSpatialActorComponent"));
	SecondSpatialObjectComponent = CreateDefaultSubobject<USpatialTypeObjectStub>(TEXT("SecondSpatialActorComponent"));
}

void ASpatialTypeActorWithMultipleObjectComponents ::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTypeActorWithMultipleObjectComponents, FirstSpatialObjectComponent);
	DOREPLIFETIME(ASpatialTypeActorWithMultipleObjectComponents, SecondSpatialObjectComponent);
}

ASpatialTypeActorWithSubobject::ASpatialTypeActorWithSubobject(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SpatialActorSubobject = CreateDefaultSubobject<USpatialTypeObjectStub>(TEXT("SpatialActorSubobject"));
}

void ASpatialTypeActorWithSubobject ::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTypeActorWithSubobject, SpatialActorSubobject);
}

ASpatialTypeActorWithOwnerOnly::ASpatialTypeActorWithOwnerOnly(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ASpatialTypeActorWithOwnerOnly::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASpatialTypeActorWithOwnerOnly, OwnerOnlyProperty, COND_OwnerOnly);
}

ASpatialTypeActorWithInitialOnly::ASpatialTypeActorWithInitialOnly(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ASpatialTypeActorWithInitialOnly::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASpatialTypeActorWithInitialOnly, InitialOnlyProperty, COND_InitialOnly);
}
