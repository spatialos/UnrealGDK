// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialTestRepNotify.h"
#include "SpatialFunctionalTestFlowController.h"
#include "Engine/World.h"
#include "GeneralProjectSettings.h"
#include "SpatialGDK/Public/EngineClasses/SpatialNetDriver.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"


ASpatialTestRepNotify::ASpatialTestRepNotify()
	: Super()
{
	Author = "Miron";
	Description = TEXT("Test RepNotify replication and shadow data");
}

void ASpatialTestRepNotify::BeginPlay()
{
	Super::BeginPlay();

	AddStep(TEXT("ServerSetReplicatedVariables"), FWorkerDefinition::Server(1), nullptr, [this]()
		{
			TestInt1 = 1;
			TestInt2 = 2;
			TestInt3 = 3;
			TestInt4 = 4;
			TestArray.Empty();
			TestArray.Add(1);
			TestArray.Add(2);
			FinishStep();
		});

	AddStep(TEXT("ClientCheckVariablesReplicated"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime)
		{
			if (bOnRepTestInt1Called && bOnRepTestInt2Called 
				&& TestArray.Num() == 2 && TestArray[0] == 1 && TestArray[1] == 2
				&& TestInt1 == 1 && TestInt2 == 2 && TestInt3 == 3 && TestInt4 == 4)
			{
				FinishStep();
			}
		}, 5.0f);

	AddStep(TEXT("ClientLocallyChangeVariables"), FWorkerDefinition::AllClients, nullptr, [this]()
		{
			bOnRepTestInt1Called = false;
			bOnRepTestInt2Called = false;
			OldTestInt3 = -3;
			OldTestInt4 = -4;
			OldTestArray.Empty();
			FinishStep();
		});

	// we check that local variables are modified correctly dude!
	AddStep(TEXT("ClientCheckLocallyChangedVariables"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime)
		{
			if (OldTestInt3 == -3 && OldTestInt4 == -4 && TestInt1 == 1 && TestInt2 == 2 && TestInt3 == 3 && TestInt4 == 4)
			{
				FinishStep();
			}
		}, 5.0f);

	// Changing from clients
	AddStep(TEXT("ClientModifyReplicatedVariables"), FWorkerDefinition::AllClients, nullptr, [this]()
		{
			TestInt1 = 10;
			TestInt2 = 20;
			TestInt4 = 40;
			FinishStep();
		});

	AddStep(TEXT("ServerChangeReplicatedVariables"), FWorkerDefinition::Server(1), nullptr, [this]()
		{
			TestInt1 = 10;
			TestInt2 = 20;
			TestInt3 = 30;
			TestInt4 = 40;
			TestArray.Add(30);
			FinishStep();
		});

	AddStep(TEXT("ClientCheckValuesAndRepNotifies"), FWorkerDefinition::AllClients, Move THE CHEKCS HERE LIKE MIORN !, nullptr, [this](float DeltaTime)
		{
			bool bHasReplicatedCorrectly = true;
			if (!(TestInt1 == 10 && TestInt2 == 20 && TestInt3 == 30 && TestInt4 == 40))
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("Integer properties did not replicate correctly"));
				return;
			}

			if (!(TestArray.Num() == 3 && TestArray[0] == 1 && TestArray[1] == 2 && TestArray[2] == 30))
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("Array did not replicate correctly"));
				return;
			}

			if (bOnRepTestInt1Called)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestInt1 should not be called on the clients"));
				return;
			}

			if (!bOnRepTestInt2Called)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestInt1 should be called on the clients"));
				return;
			}

			if (OldTestInt3 != 3)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestInt3 should have been called with the old value of 3"));
				return;
			}

			if (OldTestInt4 != 40)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestInt4 should have been called with the old value of 40"));
				return;
			}

			// We consciously differ from native UE here
			if (GetNetDriver()->IsA(USpatialNetDriver::StaticClass()))
			{
				if (OldTestArray.Num() != 2)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 2 entries in the old Array on Spatial"));
					return;
				}
			}
			else
			{
				if (OldTestArray.Num() != 3)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 3 entries in the old Array on Native"));
					return;
				}

				if (OldTestArray[2] != 0)
				{
					FinishTest(EFunctionalTestResult::Failed,  TEXT("OnRepTestArray should have been called with 0 as its third entry in the old Array on Native"));
					return;
				}
			}

			if (OldTestArray[0] != 1)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 1 as its first entry in the old Array"));
				return;
			}

			if (OldTestArray[1] != 2)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 2 as its second entry in the old Array"));
				return;
			}

			FinishStep();
		}, 5.0f);

	AddStep(TEXT("ServerTestRemovalFromArray"), FWorkerDefinition::Server(1), nullptr, [this]()
		{
			TestArray.Pop(true);
			FinishStep();
		});

	AddStep(TEXT("ClientCheckArrayRemoval"), FWorkerDefinition::AllClients, nullptr, nullptr, [this](float DeltaTime)
		{
			if (TestArray.Num() != 2)
			{
				FinishTest(EFunctionalTestResult::Failed, TEXT("TestArray should have 2 elements after shrinking"));
				return;
			}

			// We consciously differ from native UE here
			if (GetNetDriver()->IsA(USpatialNetDriver::StaticClass()))
			{
				if (OldTestArray.Num() != 3)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 3 elements after shrinking on Spatial"));
					return;
				}
				if (OldTestArray[2] != 30)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 30 as its third entry after shrinking on Spatial"));
					return;
				}
			}
			else
			{
				if (OldTestArray.Num() != 2)
				{
					FinishTest(EFunctionalTestResult::Failed, TEXT("OnRepTestArray should have been called with 2 elements after shrinking on native"));
					return;
				}
			}
			FinishStep();
		}, 5.0f);
}

void ASpatialTestRepNotify::OnRep_TestInt1(int32 OldTestInt1)
{
	bOnRepTestInt1Called = true;
}

void ASpatialTestRepNotify::OnRep_TestInt2(int32 OldTestInt2)
{
	bOnRepTestInt2Called = true;
}

void ASpatialTestRepNotify::OnRep_TestInt3(int32 InOldTestInt3)
{
	OldTestInt3 = InOldTestInt3;
}

void ASpatialTestRepNotify::OnRep_TestInt4(int32 InOldTestInt4)
{
	OldTestInt4 = InOldTestInt4;
}

void ASpatialTestRepNotify::OnRep_TestArray(TArray<int32> InOldTestArray)
{
	OldTestArray = InOldTestArray;
}

void ASpatialTestRepNotify::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASpatialTestRepNotify, TestInt1);
	DOREPLIFETIME_CONDITION_NOTIFY(ASpatialTestRepNotify, TestInt2, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ASpatialTestRepNotify, TestInt3);
	DOREPLIFETIME_CONDITION_NOTIFY(ASpatialTestRepNotify, TestInt4, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(ASpatialTestRepNotify, TestArray);
}
