// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestNetReceive.h"

#include "SpatialGDK/Public/EngineClasses/SpatialNetDriver.h"

#include "Net/UnrealNetwork.h"

/**
 * TODO CHANGE
 *
 * This test tests RepNotifies and shadow data implementation.
 *
 * The test includes 1 Server and 2 Client workers.
 * The flow is as follows:
 * - Setup:
 *	- The test itself is used as the test Actor.
 *  - It contains a number of replicated properties and OnRep functions that either set boolean flags confirming they have been called, or
 *save the old value of the argument passed to the OnRep function.
 * We also check the ordering of RepNotifies being called, expecting that both an actor and its subobjects will have their new data applied
 *before repnotifies are called on either.
 *
 * - Test:
 *	- The Server changes the values of the replicated properties multiple times.
 *  - The clients check whether the RepNotifies have or have not been called correctly.
 * - Cleanup:
 *	- None.
 */

static constexpr float StepTimeLimit = 15.0f;
ASpatialTestNetReceive::ASpatialTestNetReceive()
	: Super()
{
	Author = "Arthur";
	Description = TEXT("Test Pre/PostNetReceive events");
}

void ASpatialTestNetReceive::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("SpatialTestNetReceiveInitialise"), FWorkerDefinition::Server(1), nullptr, [this]() {
		TestActor = GetWorld()->SpawnActor<ASpatialTestNetReceiveActor>(Server1Pos, FRotator::ZeroRotator, FActorSpawnParameters());
		TestActor->SetReplicates(true);

		if (!AssertIsValid(TestActor, TEXT("Failed to spawn TestActor")))
		{
			return;
		}

		RegisterAutoDestroyActor(TestActor);

		TestActor->Subobject->TestInt = 5;
		TestActor->Subobject->ServerOnlyTestInt = 6;

		FinishStep();
	});

	AddStep(
		TEXT("SpatialTestNetReceiveCheckAllOk"), FWorkerDefinition::Server(2), nullptr, nullptr,
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
			RequireEqual_Int(TestActor->Subobject->ServerOnlyTestInt, 6, TEXT("ServerOnlyTestInt property should be updated"));

			RequireEqual_Bool(TestActor->Subobject->bOnRepTestInt1Called, true, TEXT("bOnRepTestInt1Called should be the correct value"));
			RequireEqual_Bool(TestActor->Subobject->bPreNetReceiveCalled, true, TEXT("bPreNetReceiveCalled should be the correct value"));
			RequireEqual_Bool(TestActor->Subobject->bPostNetReceiveCalled, true, TEXT("bPostNetReceiveCalled should be the correct value"));

			RequireEqual_Bool(TestActor->Subobject->bPostNetCalledBeforePreNet, false,
							  TEXT("bPostNetCalledBeforePreNet should be the correct value"));
			RequireEqual_Bool(TestActor->Subobject->bRepNotifyCalledBeforePreNet, false,
							  TEXT("bRepNotifyCalledBeforePreNet should be the correct value"));

			RequireEqual_Bool(TestActor->Subobject->bPreNetCalledBeforePostNet, true,
							  TEXT("bPreNetCalledBeforePostNet should be the correct value"));
			RequireEqual_Bool(TestActor->Subobject->bRepNotifyCalledBeforePostNet, false,
							  TEXT("bRepNotifyCalledBeforePostNet should be the correct value"));

			RequireEqual_Bool(TestActor->Subobject->bPreNetCalledBeforeRepNotify, true,
							  TEXT("bPreNetCalledBeforeRepNotify should be the correct value"));
			RequireEqual_Bool(TestActor->Subobject->bPostNetCalledBeforeRepNotify, true,
							  TEXT("bPostNetCalledBeforeRepNotify should be the correct value"));

			RequireEqual_Int(TestActor->Subobject->PreNetNumTimesCalled, 1, TEXT("PreNetNumTimesCalled should be the correct value"));
			RequireEqual_Int(TestActor->Subobject->PostNetNumTimesCalled, 1, TEXT("PostNetNumTimesCalled should be the correct value"));
			RequireEqual_Int(TestActor->Subobject->RepNotifyNumTimesCalled, 1, TEXT("RepNotifyNumTimesCalled should be the correct value"));

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
	bReplicates = true;

	Subobject = CreateDefaultSubobject<USpatialTestNetReceiveSubobject>(TEXT("USpatialTestNetReceiveSubobject"));
	Subobject->SetIsReplicated(true);
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

	TestInt = 0;
	bOnRepTestInt1Called = false;

	bPreNetReceiveCalled = false;
	bPostNetReceiveCalled = false;

	bPostNetCalledBeforePreNet = false;
	bRepNotifyCalledBeforePreNet = false;

	bPreNetCalledBeforePostNet = false;
	bRepNotifyCalledBeforePostNet = false;

	bPreNetCalledBeforeRepNotify = false;
	bPostNetCalledBeforeRepNotify = false;

	PreNetNumTimesCalled = 0;
	PostNetNumTimesCalled = 0;
	RepNotifyNumTimesCalled = 0;
}

void USpatialTestNetReceiveSubobject::PreNetReceive()
{
	bPreNetReceiveCalled = true;

	bPostNetCalledBeforePreNet = bPostNetReceiveCalled;
	bRepNotifyCalledBeforePreNet = bOnRepTestInt1Called;

	++PreNetNumTimesCalled;
}

void USpatialTestNetReceiveSubobject::PostNetReceive()
{
	bPostNetReceiveCalled = true;

	bPreNetCalledBeforePostNet = bPreNetReceiveCalled;
	bRepNotifyCalledBeforePostNet = bOnRepTestInt1Called;

	++PostNetNumTimesCalled;
}

void USpatialTestNetReceiveSubobject::OnRep_TestInt1(int32 OldTestInt1)
{
	bOnRepTestInt1Called = true;

	bPreNetCalledBeforeRepNotify = bPreNetReceiveCalled;
	bPostNetCalledBeforeRepNotify = bPostNetReceiveCalled;

	++RepNotifyNumTimesCalled;
}

void USpatialTestNetReceiveSubobject::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USpatialTestNetReceiveSubobject, TestInt);
	DOREPLIFETIME_CONDITION(USpatialTestNetReceiveSubobject, ServerOnlyTestInt, COND_ServerOnly);
}
