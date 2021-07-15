// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "StaticSubobjectTestActor.h"
#include "Net/UnrealNetwork.h"

AStaticSubobjectTestActor::AStaticSubobjectTestActor()
{
	TestIntProperty = 0;
	bNetLoadOnClient = true;
	bNetLoadOnNonAuthServer = true;
	bReplicates = true;

	TestStaticComponent1 = CreateDefaultSubobject<USceneComponent>(TEXT("ToRemoveComponent1"));
	TestStaticComponent1->SetIsReplicated(true);

	TestStaticComponent2 = CreateDefaultSubobject<USceneComponent>(TEXT("ToRemoveComponent2"));
	TestStaticComponent2->SetIsReplicated(true);
}

void AStaticSubobjectTestActor::InitialiseTestIntProperty()
{
	TestIntProperty = 0;
}

void AStaticSubobjectTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStaticSubobjectTestActor, TestIntProperty);
	DOREPLIFETIME(AStaticSubobjectTestActor, TestStaticComponent1);
	DOREPLIFETIME(AStaticSubobjectTestActor, TestStaticComponent2);
}
