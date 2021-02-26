// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "EventTracingMap.h"

#include "ComponentUpdateEventTracingTest.h"
#include "ProcessRPCEventTracingTest.h"
#include "PropertyUpdateEventTracingTest.h"
#include "UserProcessRPCEventTracingTest.h"
#include "UserReceivePropertyEventTracingTest.h"
#include "UserSendPropertyEventTracingTest.h"
#include "UserSendRPCEventTracingTest.h"

#include "EngineClasses/SpatialWorldSettings.h"

UEventTracingMap::UEventTracingMap()
	// incomplete, TODO:
	// - add settings overrides to enable event tracing
	// - figure out what to do with blueprint character
	: UGeneratedTestMap(EMapCategory::NO_CI, TEXT("EventTracingMap"))
{
}

void UEventTracingMap::CreateCustomContentForMap()
{
	ULevel* CurrentLevel = World->GetCurrentLevel();

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	// Currently, event tracing test map uses a blueprint game mode in order to use BP_EventTracerCharacter.
	// BP_EventTracerCharacter has some logic that would need to be converted. Some of it (all of it?) is actually testing the event tracing
	// blueprint APIs. If we fully convert the tests to C++, how do we test the blueprint API? WorldSettings->DefaultGameMode =
	// FunctionalTestBasicGameMode;
	WorldSettings->TestingSettings.TestingMode = EMapTestingMode::Detect;

	// Add the tests
	AddActorToLevel<AComponentUpdateEventTracingTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<AProcessRPCEventTracingTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<APropertyUpdateEventTracingTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<AUserProcessRPCEventTracingTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<AUserReceivePropertyEventTracingTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<AUserSendPropertyEventTracingTest>(CurrentLevel, FTransform::Identity);
	AddActorToLevel<AUserSendRPCEventTracingTest>(CurrentLevel, FTransform::Identity);
}
