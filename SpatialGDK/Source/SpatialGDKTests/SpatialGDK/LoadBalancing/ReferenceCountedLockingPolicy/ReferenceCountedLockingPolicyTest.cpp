// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialPackageMapClientMock.h"
#include "SpatialStaticComponentViewMock.h"
#include "SpatialVirtualWorkerTranslatorMock.h"

#include "EngineClasses/AbstractSpatialPackageMapClient.h"
#include "EngineClasses/SpatialVirtualWorkerTranslator.h"
#include "Interop/SpatialStaticComponentView.h"
#include "LoadBalancing/ReferenceCountedLockingPolicy.h"
#include "SpatialConstants.h"
#include "Tests/TestDefinitions.h"

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/DefaultPawn.h"
#include "Improbable/SpatialEngineDelegates.h"
#include "Tests/AutomationCommon.h"
#include "Templates/SharedPointer.h"
#include "UObject/UObjectGlobals.h"

#define REFERENCECOUNTEDLOCKINGPOLICY_TEST(TestName) \
	GDK_TEST(Core, UReferenceCountedLockingPolicy, TestName)

namespace
{

using LockingTokenAndDebugString = TPair<ActorLockToken, FString>;

struct TestData
{
	UWorld* TestWorld;
	TMap<FName, AActor*> TestActors;
	TMap<AActor*, TArray<LockingTokenAndDebugString>> TestActorToLockingTokenAndDebugStrings;
	UReferenceCountedLockingPolicy* LockingPolicy;
	USpatialStaticComponentView* StaticComponentView;
	UAbstractSpatialPackageMapClient* PackageMap;
	AbstractVirtualWorkerTranslator* VirtualWorkerTranslator;
	SpatialDelegates::FAcquireLockDelegate AcquireLockDelegate;
	SpatialDelegates::FReleaseLockDelegate ReleaseLockDelegate;
};

struct TestDataDeleter
{
	void operator()(TestData* Data) const noexcept
	{
		Data->LockingPolicy->RemoveFromRoot();
		Data->StaticComponentView->RemoveFromRoot();
		Data->PackageMap->RemoveFromRoot();
		delete Data;
	}
};

TSharedPtr<TestData> MakeNewTestData(Worker_EntityId EntityId, Worker_Authority EntityAuthority, VirtualWorkerId VirtWorkerId)
{
	TSharedPtr<TestData> Data(new TestData, TestDataDeleter());
	USpatialStaticComponentViewMock* StaticComponentView = NewObject<USpatialStaticComponentViewMock>();
	StaticComponentView->Init(EntityId, EntityAuthority, VirtWorkerId);

	Data->StaticComponentView = StaticComponentView;
	Data->StaticComponentView->AddToRoot();

	USpatialPackageMapClientMock* PackageMap = NewObject<USpatialPackageMapClientMock>();
	PackageMap->Init(EntityId);
	Data->PackageMap = PackageMap;
	Data->PackageMap->AddToRoot();

	Data->VirtualWorkerTranslator = new USpatialVirtualWorkerTranslatorMock(VirtWorkerId);

	Data->LockingPolicy = NewObject<UReferenceCountedLockingPolicy>();
	Data->LockingPolicy->Init(Data->StaticComponentView, Data->PackageMap, Data->VirtualWorkerTranslator, Data->AcquireLockDelegate, Data->ReleaseLockDelegate);
	Data->LockingPolicy->AddToRoot();

	return Data;
}

// Copied from AutomationCommon::GetAnyGameWorld()
UWorld* GetAnyGameWorld()
{
	UWorld* World = nullptr;
	const TIndirectArray<FWorldContext>& WorldContexts = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldContexts)
	{
		if ((Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
			&& (Context.World() != nullptr))
		{
			World = Context.World();
			break;
		}
	}

	return World;
}

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FWaitForWorld, TSharedPtr<TestData>, Data);
bool FWaitForWorld::Update()
{
	Data->TestWorld = GetAnyGameWorld();

	if (Data->TestWorld && Data->TestWorld->AreActorsInitialized())
	{
		AGameStateBase* GameState = Data->TestWorld->GetGameState();
		if (GameState && GameState->HasMatchStarted())
		{
			return true;
		}
	}

	return false;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FSpawnActor, TSharedPtr<TestData>, Data, FName, Handle);
bool FSpawnActor::Update()
{
	FActorSpawnParameters SpawnParams;
	SpawnParams.bNoFail = true;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* Actor = Data->TestWorld->SpawnActor<ADefaultPawn>(SpawnParams);
	Data->TestActors.Add(Handle, Actor);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FWaitForActor, TSharedPtr<TestData>, Data, FName, Handle);
bool FWaitForActor::Update()
{
	AActor* Actor = Data->TestActors[Handle];
	return (IsValid(Actor) && Actor->IsActorInitialized() && Actor->HasActorBegunPlay());
}

DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(FAcquireLock, FAutomationTestBase*, Test, TSharedPtr<TestData>, Data, FName, ActorHandle, FString, DebugString, bool, bExpectedSuccess);
bool FAcquireLock::Update()
{
	AActor* Actor = Data->TestActors[ActorHandle];
	const ActorLockToken Token = Data->LockingPolicy->AcquireLock(Actor, DebugString);
	const bool bAcquireLockSucceeded = Token != SpatialConstants::INVALID_ACTOR_LOCK_TOKEN;

	// If the token returned is valid, it MUST be unique
	if (bAcquireLockSucceeded)
	{
		for (const TPair<AActor*, TArray<LockingTokenAndDebugString>>& ActorLockingTokenAndDebugStrings : Data->TestActorToLockingTokenAndDebugStrings)
		{
			const TArray<LockingTokenAndDebugString>& LockingTokensAndDebugStrings = ActorLockingTokenAndDebugStrings.Value;
			bool TokenAlreadyExists = LockingTokensAndDebugStrings.ContainsByPredicate([Token](const LockingTokenAndDebugString& Data)
			{
				return Token == Data.Key;
			});
			if (TokenAlreadyExists)
			{
				Test->AddError(FString::Printf(TEXT("AcquireLock returned a valid ActorLockToken that had already been assigned. Token: %d"), Token));
			}
		}
	}

	Test->TestFalse(TEXT("Expected AcquireLock to succeed but it failed"), bExpectedSuccess && !bAcquireLockSucceeded);
	Test->TestFalse(TEXT("Expected AcquireLock to fail but it succeeded"), !bExpectedSuccess && bAcquireLockSucceeded);

	TArray<LockingTokenAndDebugString>& ActorLockTokens = Data->TestActorToLockingTokenAndDebugStrings.FindOrAdd(Actor);
	ActorLockTokens.Emplace(TPairInitializer<ActorLockToken, FString>(Token, DebugString));

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(FReleaseLock, FAutomationTestBase*, Test, TSharedPtr<TestData>, Data, FName, ActorHandle, FString, LockDebugString, bool, bExpectedSuccess);
bool FReleaseLock::Update()
{
	const AActor* Actor = Data->TestActors[ActorHandle];

	// Find lock token based on relevant lock debug string
	TArray<LockingTokenAndDebugString>* LockTokenAndDebugStrings = Data->TestActorToLockingTokenAndDebugStrings.Find(Actor);

	// If we're double releasing or releasing with a non-existent token then either we should expect to fail or the test is broken
	if (LockTokenAndDebugStrings == nullptr)
	{
		check(!bExpectedSuccess);
		Test->AddExpectedError(TEXT("Called ReleaseLock for unidentified Actor lock token."), EAutomationExpectedErrorFlags::Contains, 1);
		const bool bReleaseLockSucceeded = Data->LockingPolicy->ReleaseLock(SpatialConstants::INVALID_ACTOR_LOCK_TOKEN);
		Test->TestFalse(TEXT("Expected ReleaseLockDelegate to fail but it succeeded"), !bExpectedSuccess && bReleaseLockSucceeded);
		return true;
	}

	int32 TokenIndex = LockTokenAndDebugStrings->IndexOfByPredicate([this](const LockingTokenAndDebugString& Data)
	{
		return Data.Value == LockDebugString;
	});
	Test->TestTrue("Found valid lock token", TokenIndex != INDEX_NONE);

	LockingTokenAndDebugString& LockTokenAndDebugString = (*LockTokenAndDebugStrings)[TokenIndex];

	const bool bReleaseLockSucceeded = Data->LockingPolicy->ReleaseLock(LockTokenAndDebugString.Key);

	Test->TestFalse(TEXT("Expected ReleaseLockDelegate to succeed but it failed"), bExpectedSuccess && !bReleaseLockSucceeded);

	LockTokenAndDebugStrings->RemoveAt(TokenIndex);

	// If removing last token for Actor, delete map entry
	if (LockTokenAndDebugStrings->Num() == 0)
	{
		Data->TestActorToLockingTokenAndDebugStrings.Remove(Actor);
	}

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(FAcquireLockViaDelegate, FAutomationTestBase*, Test, TSharedPtr<TestData>, Data, FName, ActorHandle, FString, DelegateLockIdentifier, bool, bExpectedSuccess);
bool FAcquireLockViaDelegate::Update()
{
	AActor* Actor = Data->TestActors[ActorHandle];

	check(Data->AcquireLockDelegate.IsBound());
	const bool bAcquireLockSucceeded = Data->AcquireLockDelegate.Execute(Actor, DelegateLockIdentifier);

	Test->TestFalse(TEXT("Expected AcquireLockDelegate to succeed but it failed"), bExpectedSuccess && !bAcquireLockSucceeded);
	Test->TestFalse(TEXT("Expected AcquireLockDelegate to fail but it succeeded"), !bExpectedSuccess && bAcquireLockSucceeded);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(FReleaseLockViaDelegate, FAutomationTestBase*, Test, TSharedPtr<TestData>, Data, FName, ActorHandle, FString, DelegateLockIdentifier, bool, bExpectedSuccess);
bool FReleaseLockViaDelegate::Update()
{
	AActor* Actor = Data->TestActors[ActorHandle];

	check(Data->ReleaseLockDelegate.IsBound());

	if (!bExpectedSuccess)
	{
		Test->AddExpectedError(TEXT("Executed ReleaseLockDelegate for unidentified delegate lock identifier."), EAutomationExpectedErrorFlags::Contains, 1);
	}

	const bool bReleaseLockSucceeded = Data->ReleaseLockDelegate.Execute(Actor, DelegateLockIdentifier);

	Test->TestFalse(TEXT("Expected ReleaseLockDelegate to succeed but it failed"), bExpectedSuccess && !bReleaseLockSucceeded);
	Test->TestFalse(TEXT("Expected ReleaseLockDelegate to fail but it succeeded"), !bExpectedSuccess && bReleaseLockSucceeded);

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(FTestIsLocked, FAutomationTestBase*, Test, TSharedPtr<TestData>, Data, FName, Handle, bool, bIsLockedExpected);
bool FTestIsLocked::Update()
{
	const AActor* Actor = Data->TestActors[Handle];
	const bool bIsLocked = Data->LockingPolicy->IsLocked(Actor);
	Test->TestEqual(FString::Printf(TEXT("Is locked. Actual: %d. Expected: %d"), bIsLocked, bIsLockedExpected), bIsLocked, bIsLockedExpected);
	return true;
}

}  // anonymous namespace

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_an_actor_has_not_been_locked_WHEN_IsLocked_is_called_THEN_returns_false_with_no_lock_tokens)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(SpatialConstants::INVALID_ENTITY_ID, WORKER_AUTHORITY_NOT_AUTHORITATIVE, SpatialConstants::INVALID_VIRTUAL_WORKER_ID);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_Actor_is_not_locked_WHEN_ReleaseLock_is_called_THEN_it_errors_and_returns_false)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "Actor", "First lock", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_are_called_WHEN_IsLocked_is_called_THEN_returns_correctly_between_calls)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_are_called_twice_WHEN_IsLocked_is_called_THEN_returns_correctly_between_calls)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Actor", "Second lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "Actor", "Second lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_are_called_WHEN_ReleaseLock_is_called_again_THEN_it_errors_and_returns_false)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "Actor", "First lock", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_Actor_is_not_locked_WHEN_ReleaseLock_delegate_is_executed_THEN_it_errors_and_returns_false)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLockViaDelegate(this, Data, "Actor", "First lock", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_delegates_are_executed_WHEN_IsLocked_is_called_THEN_returns_correctly_between_calls)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLockViaDelegate(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLockViaDelegate(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_delegates_are_executed_twice_WHEN_IsLocked_is_called_THEN_returns_correctly_between_calls)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLockViaDelegate(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLockViaDelegate(this, Data, "Actor", "Second lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLockViaDelegate(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLockViaDelegate(this, Data, "Actor", "Second lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLockDelegate_and_ReleaseLockDelegate_are_executed_WHEN_ReleaseLockDelegate_is_executed_again_THEN_it_errors_and_returns_false)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLockViaDelegate(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLockViaDelegate(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLockViaDelegate(this, Data, "Actor", "First lock", false));

	return true;
}
