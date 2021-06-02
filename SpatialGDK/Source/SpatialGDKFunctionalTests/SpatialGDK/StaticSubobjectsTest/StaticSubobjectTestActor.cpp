// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "StaticSubobjectTestActor.h"
#include "Net/UnrealNetwork.h"

AStaticSubobjectTestActor::AStaticSubobjectTestActor()
{
	TestIntProperty = -1;
	bNetLoadOnClient = true;
	bNetLoadOnNonAuthServer = true;
	bReplicates = true;

	TestStaticComponent = CreateDefaultSubobject<USceneComponent>(TEXT("ToRemoveComponent"));
	TestStaticComponent->SetupAttachment(this->GetRootComponent(), TEXT("ToRemoveCompSocket"));
	TestStaticComponent->SetIsReplicated(true);
}

void AStaticSubobjectTestActor::InitialiseTestIntProperty()
{
	TestIntProperty = -1;
}

void AStaticSubobjectTestActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AStaticSubobjectTestActor, TestIntProperty);
	DOREPLIFETIME(AStaticSubobjectTestActor, TestStaticComponent);
}
