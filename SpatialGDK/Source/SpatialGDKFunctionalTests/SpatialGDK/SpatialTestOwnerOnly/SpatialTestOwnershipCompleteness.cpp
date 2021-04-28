// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDK/SpatialTestOwnerOnly/SpatialTestOwnershipCompleteness.h"

#include "EngineClasses/SpatialWorldSettings.h"
#include "TestWorkerSettings.h"

UOwnershipCompletenessGeneratedMap::UOwnershipCompletenessGeneratedMap()
	: Super(EMapCategory::CI_PREMERGE, TEXT("OwnershipCompletenessMap"))
{
	// clang-format off
	SetCustomConfig(
        TEXT("[/Script/UnrealEd.LevelEditorPlaySettings]") LINE_TERMINATOR
        TEXT("PlayNumberOfClients=1")
    );
	// clang-format on
}

void UOwnershipCompletenessGeneratedMap::CreateCustomContentForMap()
{
	Super::CreateCustomContentForMap();

	ULevel* Level = World->GetCurrentLevel();

	check(IsValid(Level));

	ASpatialWorldSettings* WorldSettings = CastChecked<ASpatialWorldSettings>(World->GetWorldSettings());
	WorldSettings->SetMultiWorkerSettingsClass(UTest1x2FullInterestWorkerSettings::StaticClass());
	WorldSettings->DefaultGameMode = AOwnershipCompletenessGameMode::StaticClass();

	AddActorToLevel<AOwnershipCompletenessTest>(Level, FTransform::Identity);
}

AOwnershipCompletenessGameMode::AOwnershipCompletenessGameMode()
{
	DefaultPawnClass = AOwnershipCompletenessPawn::StaticClass();
	PlayerControllerClass = AOwnershipCompletenessController::StaticClass();
}

template <typename T>
T* First(UWorld& World)
{
	for (T* Actor : TActorRange<T>(&World))
	{
		return Actor;
	}

	return nullptr;
}

AOwnershipCompletenessTest::AOwnershipCompletenessTest()
{
	Author = TEXT("Dmitrii Kozlov <dmitriikozlov@improbable.io>");
	Description = TEXT("Ownership Completeness");
}

void AOwnershipCompletenessTest::PrepareTest()
{
	Super::PrepareTest();

	constexpr float Timeout = 2.0f;

	AddStep(
		TEXT("Grabbing required objects"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float) {
			Controller = GetWorld()->GetFirstPlayerController<AOwnershipCompletenessController>();
			if (RequireTrue(IsValid(Controller), TEXT("Controller is valid")))
			{
				Pawn = Controller->GetPawn<AOwnershipCompletenessPawn>();
				RequireTrue(IsValid(Pawn), TEXT("Pawn is valid"));
			}
			FinishStep();
		},
		Timeout);

	AddStep(TEXT("Spawn additional actors"), FWorkerDefinition::AllServers, nullptr, [this]() {
		if (Controller->HasAuthority() && Pawn->HasAuthority())
		{
			PawnActor = GetWorld()->SpawnActor<AOwnershipCompletenessTestPawnActor>();
			PawnActor->SetOwner(Pawn);
			ControllerActor = GetWorld()->SpawnActor<AOwnershipCompletenessTestControllerActor>();
			ControllerActor->SetOwner(Controller);
			FreeActor = GetWorld()->SpawnActor<AOwnershipCompletenessTestFreeActor>();
		}
		FinishStep();
	});

	AddStep(
		TEXT("Wait until additional actors are received"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float) {
			if (!IsValid(PawnActor))
			{
				PawnActor = First<AOwnershipCompletenessTestPawnActor>(*GetWorld());
			}
			if (!IsValid(ControllerActor))
			{
				ControllerActor = First<AOwnershipCompletenessTestControllerActor>(*GetWorld());
			}
			if (!IsValid(FreeActor))
			{
				FreeActor = First<AOwnershipCompletenessTestFreeActor>(*GetWorld());
			}

			RequireTrue(IsValid(PawnActor), TEXT("PawnActor received"));
			RequireTrue(IsValid(ControllerActor), TEXT("ControllerActor received"));
			RequireTrue(IsValid(FreeActor), TEXT("FreeActor received"));

			FinishStep();
		},
		Timeout);

	AddStep(
		TEXT("Validate additional actor roles and ownership"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float) {
			RequireTrue(PawnActor->GetOwner() == Pawn, TEXT("Pawn actor is owned by Pawn"));
			RequireTrue(ControllerActor->GetOwner() == Controller, TEXT("Controller actor is owned by Controller"));
			RequireTrue(FreeActor->GetOwner() == nullptr, TEXT("Free actor is owned by nothing"));

			FinishStep();
		},
		Timeout);

	AddStep(TEXT("Move FreeActor to Pawn"), FWorkerDefinition::AllServers, nullptr, [this] {
		if (FreeActor->HasAuthority())
		{
			FreeActor->SetOwner(PawnActor);
		}
		FinishStep();
	});

	AddStep(
		TEXT("Wait until all workers receive FreeActor ownership change"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float) {
			RequireTrue(FreeActor->IsOwnedBy(Pawn), TEXT("FreeActor is now owned by Pawn"));

			FinishStep();
		},
		Timeout);

	AddStep(TEXT("Remove PawnActor from Pawn"), FWorkerDefinition::AllServers, nullptr, [this] {
		if (PawnActor->HasAuthority())
		{
			PawnActor->SetOwner(nullptr);
		}
		FinishStep();
	});

	AddStep(
		TEXT("Wait until all workers receive PawnActor and FreeActor ownership change"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float) {
			RequireTrue(FreeActor->IsOwnedBy(PawnActor), TEXT("FreeActor is now owned by PawnActor"));
			RequireFalse(FreeActor->IsOwnedBy(Pawn), TEXT("FreeActor is now NOT owned by Pawn"));
			RequireFalse(PawnActor->IsOwnedBy(Pawn), TEXT("PawnActor is now NOT owned by Pawn"));
			FinishStep();
		},
		Timeout);

	AddStep(TEXT("Move PawnActor to ControllerActor"), FWorkerDefinition::AllServers, nullptr, [this] {
		if (PawnActor->HasAuthority())
		{
			PawnActor->SetOwner(ControllerActor);
		}
		FinishStep();
	});

	AddStep(
		TEXT("Wait until all workers receive PawnActor and FreeActor ownership change"), FWorkerDefinition::AllWorkers, nullptr, nullptr,
		[this](float) {
			RequireTrue(FreeActor->IsOwnedBy(Controller), TEXT("FreeActor is now owned by Controller"));
			RequireTrue(PawnActor->IsOwnedBy(Controller), TEXT("PawnActor is now owned by Controller"));
			FinishStep();
		},
		Timeout);
}
