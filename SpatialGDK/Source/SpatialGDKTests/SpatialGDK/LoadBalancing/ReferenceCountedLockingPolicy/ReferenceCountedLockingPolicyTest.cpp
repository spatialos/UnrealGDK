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

// Copied from AutomationCommon::GetAnyGameWorld().
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
	SpawnParams.Name = Handle;

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

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FDestroyActor, TSharedPtr<TestData>, Data, FName, Handle);
bool FDestroyActor::Update()
{
	AActor* Actor = Data->TestActors.FindAndRemoveChecked(Handle);
	Actor->Destroy();

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FSetOwnership, TSharedPtr<TestData>, Data, FName, ActorBeingOwnedHandle, FName, ActorToOwnHandle);
bool FSetOwnership::Update()
{
	AActor* ActorBeingOwned = Data->TestActors[ActorBeingOwnedHandle];
	AActor* ActorToOwn = Data->TestActors[ActorToOwnHandle];
	ActorBeingOwned->SetOwner(ActorToOwn);
	Data->LockingPolicy->OnOwnerUpdated(ActorBeingOwned);
	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(FAcquireLock, FAutomationTestBase*, Test, TSharedPtr<TestData>, Data, FName, ActorHandle, FString, DebugString, bool, bExpectedSuccess);
bool FAcquireLock::Update()
{
	if (!bExpectedSuccess)
	{
		Test->AddExpectedError(TEXT("Called AcquireLock when CanAcquireLock returned false."), EAutomationExpectedErrorFlags::Contains, 1);
	}

	AActor* Actor = Data->TestActors[ActorHandle];
	const ActorLockToken Token = Data->LockingPolicy->AcquireLock(Actor, DebugString);
	const bool bAcquireLockSucceeded = Token != SpatialConstants::INVALID_ACTOR_LOCK_TOKEN;

	// If the token returned is valid, it MUST be unique.
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

	// Find lock token based on relevant lock debug string.
	TArray<LockingTokenAndDebugString>* LockTokenAndDebugStrings = Data->TestActorToLockingTokenAndDebugStrings.Find(Actor);

	// If we're double releasing or releasing with a non-existent token then either we should expect to fail or the test is broken.
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

	// If removing last token for Actor, delete map entry.
	if (LockTokenAndDebugStrings->Num() == 0)
	{
		Data->TestActorToLockingTokenAndDebugStrings.Remove(Actor);
	}

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_THREE_PARAMETER(FReleaseAllLocks, FAutomationTestBase*, Test, TSharedPtr<TestData>, Data, int32, ExpectedFailures);
bool FReleaseAllLocks::Update()
{
	const bool bExpectedSuccess = ExpectedFailures == 0;

	if (!bExpectedSuccess)
	{
		Test->AddExpectedError(TEXT("Called ReleaseLock for unidentified Actor lock token."), EAutomationExpectedErrorFlags::Contains, ExpectedFailures);
	}

	// Attempt to release every lock token for every Actor.
	for (TPair<AActor*, TArray<LockingTokenAndDebugString>>& ActorAndLockingTokenAndDebugStringsPair : Data->TestActorToLockingTokenAndDebugStrings)
	{
		for (LockingTokenAndDebugString& TokenAndDebugString : ActorAndLockingTokenAndDebugStringsPair.Value)
		{
			const ActorLockToken Token = TokenAndDebugString.Key;
			const bool bReleaseLockSucceeded = Data->LockingPolicy->ReleaseLock(Token);
			Test->TestFalse(FString::Printf(TEXT("Expected ReleaseAllLocks to fail but it succeeded. Token: %d"), Token), !bExpectedSuccess && bReleaseLockSucceeded);
			Test->TestFalse(FString::Printf(TEXT("Expected ReleaseAllLocks to succeed but it failed. Token: %d"), Token), bExpectedSuccess && !bReleaseLockSucceeded);
		}
	}

	// Cleanup the test mapping of Actors to tokens and debug strings.
	Data->TestActorToLockingTokenAndDebugStrings.Reset();

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
	Test->TestEqual(FString::Printf(TEXT("%s. Is locked. Actual: %d. Expected: %d"), *Handle.ToString(), bIsLocked, bIsLockedExpected), bIsLocked, bIsLockedExpected);
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

// AcquireLock and ReleaseLock

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

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_is_called_WHEN_the_locked_Actor_is_not_authoritative_THEN_AcquireLock_returns_false)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_NOT_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Actor", "First lock", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_is_called_WHEN_the_locked_Actor_is_deleted_THEN_ReleaseLock_returns_false)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FDestroyActor(Data, "Actor"));
	// We want to test that the Actor deletion has correctly been cleaned up in the locking policy.
	// We cannot call IsLocked with a deleted Actor so instead we try to release the lock we held
	// for the Actor and check that it fails.
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseAllLocks(this, Data, 1));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_is_called_twice_WHEN_the_locked_Actor_is_deleted_THEN_ReleaseLock_returns_false)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Actor", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Actor", "Second lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FDestroyActor(Data, "Actor"));
	// We want to test that the Actor deletion has correctly been cleaned up in the locking policy.
	// We cannot call IsLocked with a deleted Actor so instead we try to release the lock we held
	// for the Actor and check that it fails.
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseAllLocks(this, Data, 2));

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

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_are_called_WHEN_AcquireLock_is_called_again_THEN_it_succeeds)
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
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Actor", "Second lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_are_called_on_hierarchy_Actor_WHEN_IsLocked_is_called_on_hierarchy_Actors_THEN_returns_correctly_between_calls)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));

	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Character"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "State"));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Character"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "State"));

	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "State", "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "Character", "Controller"));

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", false));

	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Character", "First lock", true));

	//			Controller	
	//			/		 \
	//		State     Character (explicitly locked)

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", true));

	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "Character", "First lock", true));

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_are_called_on_multiple_hierarchy_Actors_WHEN_IsLocked_is_called_on_hierarchy_Actors_THEN_returns_correctly_between_calls)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));

	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Character"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "State"));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Character"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "State"));

	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "State", "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "Character", "Controller"));

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", false));

	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Character", "First lock", true));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "State", "Second lock", true));

	//							  Controller	
	//							   /	  \
	//		State (explicitly locked)     Character (explicitly locked)

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", true));

	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "Character", "First lock", true));

	//							 Controller	
	//							 /		 \
	//		State (explicitly locked)   Character

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", true));

	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "State", "Second lock", true));

	//		    Controller	
	//		   /		 \
	//		State     Character

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_are_called_on_hierarchy_root_Actor_WHEN_IsLocked_is_called_on_hierarchy_Actors_THEN_returns_correctly_between_calls)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));

	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Character"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "State"));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Character"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "State"));

	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "State", "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "Character", "Controller"));

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", false));

	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Controller", "First lock", true));

	//		Controller (explicitly locked)
	//		   /		 \
	//		State     Character 

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", true));

	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "Controller", "First lock", true));

	//			Controller
	//		   /		 \
	//		State     Character 

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_are_called_on_hierarchy_Actor_WHEN_Actor_hierarchy_changes_THEN_IsLocked_returns_correctly_for_all_Actors)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));

	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Character"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "State"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "OtherActor"));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Character"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "State"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "OtherActor"));

	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "State", "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "Character", "Controller"));

	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Character", "First lock", true));

	//			Controller			Other Actor
	//			/		 \
	//		State     Character (locked)

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "OtherActor", false));

	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "Character", "OtherActor"));

	//			Controller			Other Actor
	//				|					 |
	//		      State				 Character (locked)

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "OtherActor", true));

	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "Character", "First lock", true));

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "OtherActor", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_are_called_on_hierarchy_Actor_WHEN_Actor_is_destroyed_THEN_IsLocked_returns_false_for_other_hierarchy_Actors)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));

	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Character"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "State"));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Character"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "State"));

	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "State", "Controller"));
	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "Character", "Controller"));

	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Character", "First lock", true));

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Character", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", true));

	ADD_LATENT_AUTOMATION_COMMAND(FDestroyActor(Data, "Character"));

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Controller", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "State", false));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_are_called_on_hierarchy_Actor_WHEN_hierarchy_root_switches_owner_THEN_IsLocked_returns_correctly_for_all_Actors)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));

	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "A"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "B"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "C"));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "A"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "B"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "C"));

	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "B", "A"));

	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "B", "First lock", true));

	//				A					C
	//				|
	//		B (explicitly locked)

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "A", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "B", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "C", false));

	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "A", "C"));

	//				C
	//				|
	//				A
	//				|
	//		B (explicitly locked)

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "A", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "B", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "C", true));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_are_called_on_hierarchy_Actor_WHEN_Actor_on_hierarchy_path_switches_owner_THEN_IsLocked_returns_correctly_for_all_Actors)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData(1, Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE, 1);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));

	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "A"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "B"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "C"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "D"));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "E"));

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "A"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "B"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "C"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "D"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "E"));

	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "B", "A"));
	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "C", "B"));
	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "D", "C"));

	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "D", "First lock", true));

	//				A					E
	//				|
	//				B
	//				|
	//				C
	//				|
	//		D (explicitly locked)

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "A", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "B", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "C", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "D", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "E", false));

	ADD_LATENT_AUTOMATION_COMMAND(FSetOwnership(Data, "C", "E"));

	//		A					E
	//		|					|
	//		B					C
	//							|
	//					D (explicitly locked)

	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "A", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "B", false));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "C", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "D", true));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "E", true));

	return true;
}

// AcquireLockDelegate and ReleaseLockDelegate

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
