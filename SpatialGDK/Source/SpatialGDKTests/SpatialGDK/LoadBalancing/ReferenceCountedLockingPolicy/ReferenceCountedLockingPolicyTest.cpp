#include "LoadBalancing/ReferenceCountedLockingPolicy.h"
#include "TestDefinitions.h"

#include "Engine/Engine.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/DefaultPawn.h"
#include "Tests/AutomationCommon.h"

#define REFERENCECOUNTEDLOCKINGPOLICY_TEST(TestName) \
	GDK_TEST(Core, UReferenceCountedLockingPolicy, TestName)

namespace
{

struct TestData
{
	UWorld* TestWorld;
	TMap<FName, AActor*> TestActors;
	UReferenceCountedLockingPolicy* LockingPolicy;
};

struct TestDataDeleter
{
	void operator()(TestData* Data) const noexcept
	{
		Data->LockingPolicy->RemoveFromRoot();
		delete Data;
	}
};

TSharedPtr<TestData> MakeNewTestData()
{
	TSharedPtr<TestData> Data(new TestData, TestDataDeleter());
	Data->LockingPolicy = NewObject<UReferenceCountedLockingPolicy>();
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

DEFINE_LATENT_AUTOMATION_COMMAND_FOUR_PARAMETER(FTestIsLocked, FAutomationTestBase*, Test, TSharedPtr<TestData>, Data, FName, Handle, bool, bExpected);
bool FTestIsLocked::Update()
{
	AActor* Actor = Data->TestActors[Handle];
	const bool bIsLocked = Data->LockingPolicy->IsLocked(Actor);
	Test->TestEqual(FString::Printf(TEXT("Is locked. Actual: %d. Expected: %d"), bIsLocked, bExpected), bIsLocked, bExpected);
	return true;
}

}  // anonymous namespace


REFERENCECOUNTEDLOCKINGPOLICY_TEST(GIVEN_an_actor_has_not_been_locked_WHEN_IsLocked_is_called_THEN_returns_false)
{
	AutomationOpenMap("/Engine/Maps/Entry");

	TSharedPtr<TestData> Data = MakeNewTestData();
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForWorld(Data));
	ADD_LATENT_AUTOMATION_COMMAND(FSpawnActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForActor(Data, "Actor"));
	ADD_LATENT_AUTOMATION_COMMAND(FTestIsLocked(this, Data, "Actor", false));

	return true;
}
