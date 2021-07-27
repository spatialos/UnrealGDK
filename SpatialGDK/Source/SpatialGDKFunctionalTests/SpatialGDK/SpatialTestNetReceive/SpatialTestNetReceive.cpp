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

			RequireTrue(*TestActor->Subobject->RepStepToPrevRepStep.Find(RepStep::PreNetReceive) == RepStep::PreRep,
						TEXT("Nothing should be called before PreNetReceive"));
			RequireTrue(*TestActor->Subobject->RepStepToPrevRepStep.Find(RepStep::PostNetReceive) == RepStep::PreNetReceive,
						TEXT("PreNetReceive should be called before PreNetReceive"));
			RequireTrue(*TestActor->Subobject->RepStepToPrevRepStep.Find(RepStep::RepNotify) == RepStep::PostNetReceive,
						TEXT("PostNetReceive should be called before TestInt's RepNotify."));

			RepStep* SecondPreNetMapping = TestActor->Subobject->RepStepToPrevRepStep.Find(RepStep::SecondPreNetReceive);
			RequireTrue(SecondPreNetMapping == nullptr || *SecondPreNetMapping == RepStep::RepNotify,
						TEXT("SecondPreNetReceive should be called after RepNotify or not at all."));

			// Pre/Post net can be called even when new properties aren't being received. For example, if ownership status changes.
			RequireCompare_Int(TestActor->Subobject->PreNetNumTimesCalled, EComparisonMethod::Greater_Than_Or_Equal_To, 1,
							   TEXT("PreNetNumTimesCalled should be more than 1."));
			RequireCompare_Int(TestActor->Subobject->PostNetNumTimesCalled, EComparisonMethod::Greater_Than_Or_Equal_To, 1,
							   TEXT("PostNetNumTimesCalled should be more than 1."));
			RequireEqual_Int(TestActor->Subobject->PreNetNumTimesCalled, TestActor->Subobject->PostNetNumTimesCalled,
							 TEXT("Pre and PostNetReceive should be called the same number of times."));

			RequireEqual_Int(TestActor->Subobject->RepNotifyNumTimesCalled, 1, TEXT("RepNotifyNumTimesCalled should be 1."));

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

	PreNetNumTimesCalled = 0;
	PostNetNumTimesCalled = 0;
	RepNotifyNumTimesCalled = 0;
}

void USpatialTestNetReceiveSubobject::PreNetReceive()
{
	if (++PreNetNumTimesCalled == 1)
	{
		RepStepToPrevRepStep.Add(RepStep::PreNetReceive, PreviousReplicationStep);
		PreviousReplicationStep = RepStep::PreNetReceive;
	}
	else if (PreNetNumTimesCalled == 2)
	{
		RepStepToPrevRepStep.Add(RepStep::SecondPreNetReceive, PreviousReplicationStep);
	}
}

void USpatialTestNetReceiveSubobject::PostNetReceive()
{
	if (++PostNetNumTimesCalled == 1)
	{
		RepStepToPrevRepStep.Add(RepStep::PostNetReceive, PreviousReplicationStep);
		PreviousReplicationStep = RepStep::PostNetReceive;
	}
}

void USpatialTestNetReceiveSubobject::OnRep_TestInt(int32 OldTestInt)
{
	if (++RepNotifyNumTimesCalled == 1)
	{
		RepStepToPrevRepStep.Add(RepStep::RepNotify, PreviousReplicationStep);
		PreviousReplicationStep = RepStep::RepNotify;
	}
}

void USpatialTestNetReceiveSubobject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USpatialTestNetReceiveSubobject, TestInt);
	DOREPLIFETIME_CONDITION(USpatialTestNetReceiveSubobject, OwnerOnlyTestInt, COND_OwnerOnly);
}
