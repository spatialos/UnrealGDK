// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestNetReceive.h"

#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDK/Public/EngineClasses/SpatialNetDriver.h"

#include "Net/UnrealNetwork.h"

/**
 * This test tests Pre and PostNetReceives, along with their ordering with RepNotifies.
 *
 * The test includes 1 Server and 1 Client worker.
 * The flow is as follows:
 * - Test:
 *	- The server spawns a ASpatialTestNetReceiveActor, which has a static subobject. The static subobject is where the properties are
 *stored. We use an subobject instead of storing properties on the actor as the subobject has significantly fewer replicated properties
 *inherited from native.
 *		- The server also sets the values of the replicated int and the replicated OwnerOnly int. The actor's owner is set to the client's
 *PlayerController.
 *	- The client then checks callbacks were called in the right order
 */

static constexpr float StepTimeLimit = 15.0f;
ASpatialTestNetReceive::ASpatialTestNetReceive()
	: Super()
{
	Author = TEXT("Arthur");
	Description = TEXT("Test Pre and PostNetReceives, along with their ordering with RepNotifies");
}

void ASpatialTestNetReceive::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("SpatialTestNetReceiveInitialise"), FWorkerDefinition::Server(1), nullptr, [this]() {
		APlayerController* PlayerController =
			Cast<APlayerController>(GetFlowController(ESpatialFunctionalTestWorkerType::Client, 1)->GetOwner());

		FActorSpawnParameters Params;
		Params.Owner = PlayerController;
		TestActor = GetWorld()->SpawnActor<ASpatialTestNetReceiveActor>(Params);
		RegisterAutoDestroyActor(TestActor);

		checkf(IsValid(TestActor), TEXT("TestActor must be valid"));

		TestActor->Subobject->TestInt = 5;
		TestActor->Subobject->OwnerOnlyTestInt = 6;

		FinishStep();
	});

	AddStep(
		TEXT("SpatialTestNetReceiveCheckAllOk"), FWorkerDefinition::Client(1), nullptr, nullptr,
		[this](float Dt) {
			if (!RequireEqual_Bool(IsValid(TestActor), true, TEXT("TestActor should be valid")))
			{
				return;
			}
			if (!RequireEqual_Int(IsValid(TestActor->Subobject), true, TEXT("TestActor's subobject should be valid")))
			{
				return;
			}

			RequireEqual_Int(TestActor->Subobject->TestInt, 5, TEXT("TestInt property should be updated"));
			RequireEqual_Int(TestActor->Subobject->OwnerOnlyTestInt, 6, TEXT("ServerOnlyTestInt property should be updated"));

			const int NumMandatorySteps = TestActor->Subobject->NumMandatorySteps;
			TArray<ERepStep> RepSteps = TestActor->Subobject->RepSteps;
			TArray<ERepStep> ExpectedRepSteps = TestActor->Subobject->ExpectedRepSteps;
			RequireCompare_Int(RepSteps.Num(), EComparisonMethod::Greater_Than_Or_Equal_To, NumMandatorySteps,
							   FString::Printf(TEXT("RepSteps should contain at least %d elements."), NumMandatorySteps));

			for (int i = 0; i < TestActor->Subobject->RepSteps.Num(); ++i)
			{
				const ERepStep Step = RepSteps[i];
				const ERepStep ExpectedStep = ExpectedRepSteps.IsValidIndex(i) ? ExpectedRepSteps[i] : ERepStep::None;

				AssertTrue(Step == ExpectedStep, FString::Printf(TEXT("Got RepStep: %s expected RepStep: %s"),
																 *UEnum::GetValueAsString(Step), *UEnum::GetValueAsString(ExpectedStep)));
			}

			FinishStep();
		},
		StepTimeLimit);
}

void ASpatialTestNetReceive::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestNetReceive, TestActor);
}

ASpatialTestNetReceiveActor::ASpatialTestNetReceiveActor()
{
	Subobject = CreateDefaultSubobject<USpatialTestNetReceiveSubobject>(TEXT("USpatialTestNetReceiveSubobject"));
}

void ASpatialTestNetReceiveActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestNetReceiveActor, Subobject);
}

USpatialTestNetReceiveSubobject::USpatialTestNetReceiveSubobject()
	: Super()
{
	SetIsReplicatedByDefault(true);
}

void USpatialTestNetReceiveSubobject::PreNetReceive()
{
	RepSteps.Add(ERepStep::PreNetReceive);
}

void USpatialTestNetReceiveSubobject::PostNetReceive()
{
	RepSteps.Add(ERepStep::PostNetReceive);
}

void USpatialTestNetReceiveSubobject::OnRep_TestInt(int32 OldTestInt)
{
	RepSteps.Add(ERepStep::RepNotify);
}

void USpatialTestNetReceiveSubobject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USpatialTestNetReceiveSubobject, TestInt);
	DOREPLIFETIME_CONDITION(USpatialTestNetReceiveSubobject, OwnerOnlyTestInt, COND_OwnerOnly);
}
