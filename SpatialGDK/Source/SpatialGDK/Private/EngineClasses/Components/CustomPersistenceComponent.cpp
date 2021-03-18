// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EngineClasses/Components/CustomPersistenceComponent.h"

#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Runtime/Launch/Resources/Version.h"
#include "TimerManager.h"

UCustomPersistenceComponent::UCustomPersistenceComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

#if ENGINE_MINOR_VERSION <= 23
	bReplicates = false;
#else
	SetIsReplicatedByDefault(false);
#endif
}
