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
	Data->LockingPolicy->Init(Data->StaticComponentView, Data->PackageMap, Data->VirtualWorkerTranslator);
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

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(FAcquireLock, FAutomationTestBase*, Test, TSharedPtr<TestData>, Data, FName, ActorHandle, FString, DebugString);
bool FAcquireLock::Update()
{
	AActor* Actor = Data->TestActors[ActorHandle];
	const ActorLockToken Token = Data->LockingPolicy->AcquireLock(Actor, DebugString);

	// If the token returned is valid, it MUST be unique
	if (Token != SpatialConstants::INVALID_ACTOR_LOCK_TOKEN)
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

	TArray<LockingTokenAndDebugString>& ActorLockTokens = Data->TestActorToLockingTokenAndDebugStrings.FindOrAdd(Actor);
	ActorLockTokens.Emplace(TPairInitializer<ActorLockToken, FString>(Token, DebugString));

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(FReleaseLock, FAutomationTestBase*, Test, TSharedPtr<TestData>, Data, FName, ActorHandle, FString, LockDebugString);
bool FReleaseLock::Update()
{
	const AActor* Actor = Data->TestActors[ActorHandle];

	// Find lock token based on relevant lock debug string
	TArray<LockingTokenAndDebugString>* LockTokenAndDebugStrings = Data->TestActorToLockingTokenAndDebugStrings.Find(Actor);
	int32 TokenIndex = LockTokenAndDebugStrings->IndexOfByPredicate([this](const LockingTokenAndDebugString& Data)
	{
		return Data.Value == LockDebugString;
	});

	bool bLockFound = TokenIndex != INDEX_NONE;
	Test->TestTrue(FString::Printf(TEXT("Found valid lock token? %d"), bLockFound), bLockFound);

	LockingTokenAndDebugString& LockTokenAndDebugString = (*LockTokenAndDebugStrings)[TokenIndex];

	Data->LockingPolicy->ReleaseLock(LockTokenAndDebugString.Key);

	LockTokenAndDebugStrings->RemoveAt(TokenIndex);

	// If removing last token for Actor, delete map entry
	if (LockTokenAndDebugStrings->Num() == 0)
	{
		Data->TestActorToLockingTokenAndDebugStrings.Remove(Actor);
	}

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_FIVE_PARAMETER(FTestIsLocked, FAutomationTestBase*, Test, TSharedPtr<TestData>, Data, FName, Handle, bool, bIsLockedExpected, int32, LockTokenCountExpected);
bool FTestIsLocked::Update()
{
	const AActor* Actor = Data->TestActors[Handle];
	const bool bIsLocked = Data->LockingPolicy->IsLocked(Actor);

	TSet<ActorLockToken> LockTokens;
	int32 LockTokenCount = 0;
	if (bIsLocked)
	{
		LockTokenCount = Data->TestActorToLockingTokenAndDebugStrings[Actor].Num();
	}
	
	Test->TestEqual(FString::Printf(TEXT("Is locked. Actual: %d. Expected: %d"), bIsLocked, bIsLockedExpected), bIsLocked, bIsLockedExpected);
	Test->TestEqual(FString::Printf(TEXT("Lock count. Actual: %d. Expected: %d"), LockTokenCount, LockTokenCountExpected), LockTokenCount, LockTokenCountExpected);
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
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false, 0));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_is_called_WHEN_IsLocked_is_called_THEN_returns_true_with_one_lock_token)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	Worker_EntityId EntityId = 1;
	Worker_Authority EntityAuthority = Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE;
	VirtualWorkerId VirtWorkerId = 1;

	TSharedPtr<TestData> Data = MakeNewTestData(EntityId, EntityAuthority, VirtWorkerId);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false, 0));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Actor", "First lock"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true, 1));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_are_called_WHEN_IsLocked_is_called_THEN_returns_correctly_between_calls)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	Worker_EntityId EntityId = 1;
	Worker_Authority EntityAuthority = Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE;
	VirtualWorkerId VirtWorkerId = 1;

	TSharedPtr<TestData> Data = MakeNewTestData(EntityId, EntityAuthority, VirtWorkerId);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false, 0));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Actor", "First lock"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "Actor", "First lock"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false, 0));

	return true;
}

REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_AcquireLock_and_ReleaseLock_are_called_twice_WHEN_IsLocked_is_called_THEN_returns_correctly_between_calls)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	Worker_EntityId EntityId = 1;
	Worker_Authority EntityAuthority = Worker_Authority::WORKER_AUTHORITY_AUTHORITATIVE;
	VirtualWorkerId VirtWorkerId = 1;

	TSharedPtr<TestData> Data = MakeNewTestData(EntityId, EntityAuthority, VirtWorkerId);

	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false, 0));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Actor", "First lock"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FAcquireLock(this, Data, "Actor", "Second lock"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true, 2));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "Actor", "First lock"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", true, 1));
	ADD_LATENT_AUTOMATION_COMMAND(FReleaseLock(this, Data, "Actor", "Second lock"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false, 0));

	return true;
}
