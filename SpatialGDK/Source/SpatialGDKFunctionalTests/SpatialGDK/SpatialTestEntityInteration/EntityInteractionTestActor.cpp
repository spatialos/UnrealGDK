// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EntityInteractionTestActor.h"
#include "Components/SceneComponent.h"

const FString AEntityInteractionTestActor::s_NetWriteFenceName(TEXT("NetWriteFence"));
const FString AEntityInteractionTestActor::s_ReliableName(TEXT("Reliable"));
const FString AEntityInteractionTestActor::s_UnreliableName(TEXT("Unreliable"));
const FString AEntityInteractionTestActor::s_UnorderedName(TEXT("Unordered"));
const FString AEntityInteractionTestActor::s_NoLoopbackName(TEXT("NoLoopback"));

AEntityInteractionTestActor::AEntityInteractionTestActor()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>("Root");
	bReplicates = true;
}

void AEntityInteractionTestActor::TestNetWriteFence_Implementation(int32 Param)
{
	Steps.Add(Param, s_NetWriteFenceName);
}

void AEntityInteractionTestActor::TestReliable_Implementation(int32 Param)
{
	Steps.Add(Param, s_ReliableName);
}

void AEntityInteractionTestActor::TestUnreliable_Implementation(int32 Param)
{
	Steps.Add(Param, s_UnreliableName);
}

void AEntityInteractionTestActor::TestUnordered_Implementation(int32 Param)
{
	Steps.Add(Param, s_UnorderedName);
}

void AEntityInteractionTestActor::TestNoLoopback_Implementation(int32 Param)
{
	Steps.Add(Param, s_NoLoopbackName);
}
