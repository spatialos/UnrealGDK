// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FunctionalTest.h"
#include "Improbable/SpatialGDKSettingsBridge.h"
#include "SpatialFunctionalTestFlowControllerSpawner.h"
#include "SpatialFunctionalTestRequireHandler.h"
#include "SpatialFunctionalTestStep.h"
#include "SpatialFunctionalTest.generated.h"

// Blueprint Delegate
DECLARE_DYNAMIC_DELEGATE_OneParam(FSpatialFunctionalTestSnapshotTakenDelegate, bool, bSuccess);

namespace
{
// C++ hooks for Lambdas.
typedef TFunction<bool()> FIsReadyEventFunc;
typedef TFunction<void()> FStartEventFunc;
typedef TFunction<void(float DeltaTime)> FTickEventFunc;

typedef TFunction<void(bool bSuccess)> FSnapshotTakenFunc;

// We need 2 values since the way we clean up tests is based on replication of variables,
// so if the test fails to start, the cleanup process would never be triggered.
constexpr int SPATIAL_FUNCTIONAL_TEST_NOT_STARTED = -1; // Represents test waiting to run.
constexpr int SPATIAL_FUNCTIONAL_TEST_FINISHED = -2;	// Represents test already ran.
} // namespace

class ULayeredLBStrategy;

/*
 * A Spatial Functional NetTest allows you to define a series of steps, and control which server/client context they execute on
 * Servers and Clients are registered as Test Players by the framework, and request individual steps to be executed in the correct Player
 */
UCLASS(Blueprintable, SpatialType = NotPersistent,
	   hidecategories = (Input, Movement, Collision, Rendering, Replication, LOD, "Utilities|Transformation"))
class SPATIALGDKFUNCTIONALTESTS_API ASpatialFunctionalTest : public AFunctionalTest
{
	GENERATED_BODY()

protected:
	TSubclassOf<ASpatialFunctionalTestFlowController> FlowControllerActorClass;

private:
	SpatialFunctionalTestFlowControllerSpawner FlowControllerSpawner;

	UPROPERTY(ReplicatedUsing = StartServerFlowControllerSpawn)
	uint8 bReadyToSpawnServerControllers : 1;

public:
	ASpatialFunctionalTest();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	virtual void OnAuthorityGained() override;

	// Should be called from the server with authority over this actor.
	virtual void RegisterAutoDestroyActor(AActor* ActorToAutoDestroy) override;

	virtual void LogStep(ELogVerbosity::Type Verbosity, const FString& Message) override;

	// # Test APIs

	int GetNumRequiredClients() const { return NumRequiredClients; }

	// Called at the beginning of the test, use it to setup your steps. Contrary to AFunctionalTest, this will
	// run on all Workers (Server and Client).
	virtual void PrepareTest() override;

	// Lets you know if PrepareTest() has been called.
	bool HasPreparedTest() const { return bPreparedTest; }

	// Starts being called after PrepareTest, until it returns true. This is only called on Authority.
	virtual bool IsReady_Implementation() override;

	// Called once after IsReady is true. This is only called on Authority.
	virtual void StartTest() override;

	// Ends the Test, can be called from any place.
	virtual void FinishTest(EFunctionalTestResult TestResult, const FString& Message) override;

	// Add expected log errors in C++. This can only be called when setting up the steps in PrepareTest() or in
	// the steps themselves. Keep in mind that if the expected number of occurrences aren't met, the test fails.
	// The same pattern can only be added once, so make sure to execute only in the test Authority or in a step that
	// is running only on one worker.
	void AddExpectedLogError(const FString& ExpectedPatternString, int32 Occurrences = 1, bool bExactMatch = false);

	UFUNCTION(CrossServer, Reliable)
	void CrossServerFinishTest(EFunctionalTestResult TestResult, const FString& Message);

	UFUNCTION(CrossServer, Reliable)
	void CrossServerNotifyStepFinished(ASpatialFunctionalTestFlowController* FlowController, const int StepIndex);

	// # FlowController related APIs.

	void RegisterFlowController(ASpatialFunctionalTestFlowController* FlowController);

	// Get all the FlowControllers registered in this Test.
	const TArray<ASpatialFunctionalTestFlowController*>& GetFlowControllers() const { return FlowControllers; }

	// clang-format off
	UFUNCTION(BlueprintPure, Category = "Spatial Functional Test", meta = (WorkerId = "1",
		ToolTip = "Returns the FlowController for a specific Server / Client.\nKeep in mind that WorkerIds start from 1, and the Server's WorkerId will match their VirtualWorkerId while the Client's will be based on the order they connect.\n\n'All' Worker type will soft assert as it isn't supported."))
	// clang-format on
	ASpatialFunctionalTestFlowController* GetFlowController(ESpatialFunctionalTestWorkerType WorkerType, int WorkerId);

	// Get the FlowController that is Local to this instance.
	UFUNCTION(BlueprintPure, Category = "Spatial Functional Test")
	ASpatialFunctionalTestFlowController* GetLocalFlowController();

	// Helper to get the local Worker Type.
	UFUNCTION(BlueprintPure, Category = "Spatial Functional Test")
	ESpatialFunctionalTestWorkerType GetLocalWorkerType();

	// Helper to get the local Worker Id.
	UFUNCTION(BlueprintPure, Category = "Spatial Functional Test")
	int GetLocalWorkerId();

	// # Step APIs.

	// Add Steps for Blueprints.

	// clang-format off
	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test", meta = (DisplayName = "Add Step", AutoCreateRefTerm = "IsReadyEvent,StartEvent,TickEvent",
		ToolTip = "Adds a Test Step. Check GetAllWorkers(), GetAllServerWorkers() and GetAllClientWorkers() for convenience.\n\nIf you split the Worker pin you can define if you want to run on Server, Client or All.\n\nWorker Ids start from 1.\nIf you pass 0 it will run on all the Servers / Clients (there's also a convenience function GetAllWorkersId())\n\nIf you choose WorkerType 'All' it runs on all Servers and Clients (hence WorkerId is ignored).\n\nKeep in mind you can split the Worker pin for convenience."))
	// clang-format on
	void AddStepBlueprint(const FString& StepName, const FWorkerDefinition& Worker, const FStepIsReadyDelegate& IsReadyEvent,
						  const FStepStartDelegate& StartEvent, const FStepTickDelegate& TickEvent, float StepTimeLimit = 0.0f);

	// Add Steps for Blueprints and C++.

	// clang-format off
	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test",
		meta = (ToolTip = "Adds a Step from a Definition. Allows you to define a Step and add it / re-use it multiple times.\n\nKeep in mind you can split the Worker pin for convenience."))
	// clang-format on
	void AddStepFromDefinition(const FSpatialFunctionalTestStepDefinition& StepDefinition, const FWorkerDefinition& Worker);

	// clang-format off
	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test",
		meta = (ToolTip = "Adds a Step from a Definition. Allows you to define a Step and add it / re-use it multiple times.\n\nKeep in mind you can split the Worker pin for convenience.\nIt is a more extensible version of AddStepFromDefinition(), where you can pass an array with multiple specific Workers."))
	// clang-format on
	void AddStepFromDefinitionMulti(const FSpatialFunctionalTestStepDefinition& StepDefinition, const TArray<FWorkerDefinition>& Workers);

	// Add Steps for C++.
	/**
	 * Adds a Step to the Test. You can define if you want to run on Server, Client or All.
	 * There's helpers in FWorkerDefinition to make it easier / more concise. If you want to make a FWorkerDefinition from scratch,
	 * keep in mind that Worker Ids start from 1. If you pass FWorkerDefinition::ALL_WORKERS_ID (GetAllWorkersId()) it will
	 * run on all the Servers / Clients. If you pass WorkerType 'All' it runs on all Servers and Clients (hence WorkerId is ignored).
	 */
	FSpatialFunctionalTestStepDefinition& AddStep(const FString& StepName, const FWorkerDefinition& Worker,
												  FIsReadyEventFunc IsReadyEvent = nullptr, FStartEventFunc StartEvent = nullptr,
												  FTickEventFunc TickEvent = nullptr, float StepTimeLimit = 0.0f);

	// Start Running a Step.
	void StartStep(const int StepIndex);

	// Terminate the current running step (called once per FlowController executing it) if you have no failing Requires.
	// If you have failed Requires it will be ignored, making it easier for you to build tests without
	// having to manually check that there's no failed Requires before finishing the step.
	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test")
	virtual void FinishStep();

	const FSpatialFunctionalTestStepDefinition GetStepDefinition(int StepIndex) const;

	int GetCurrentStepIndex() { return CurrentStepIndex; }
	void SetCurrentStepIndex(const int StepIndex) { CurrentStepIndex = StepIndex; }

	// Convenience function that goes over all FlowControllers and counts how many are Servers.
	UFUNCTION(BlueprintPure, Category = "Spatial Functional Test")
	int GetNumberOfServerWorkers();

	// Convenience function that goes over all FlowControllers and counts how many are Clients.
	UFUNCTION(BlueprintPure, Category = "Spatial Functional Test")
	int GetNumberOfClientWorkers();

	// Convenience function that returns the Id used for executing steps on all Servers / Clients.
	// clang-format off
	UFUNCTION(BlueprintPure,
		meta = (ToolTip = "Returns the Id (0) that represents all Workers (ie Server / Client), useful for when you want to have a Server / Client Step run on all of them"),
		Category = "Spatial Functional Test")
	// clang-format on
	int GetAllWorkersId() { return FWorkerDefinition::ALL_WORKERS_ID; }

	UFUNCTION(BlueprintPure, meta = (ToolTip = "Returns a Worker Defnition that represents all of the Servers and Clients"),
			  Category = "Spatial Functional Test")
	FWorkerDefinition GetAllWorkers() { return FWorkerDefinition::AllWorkers; }

	UFUNCTION(BlueprintPure, meta = (ToolTip = "Returns a Worker Defnition that represents all of the Servers"),
			  Category = "Spatial Functional Test")
	FWorkerDefinition GetAllServers() { return FWorkerDefinition::AllServers; }

	UFUNCTION(BlueprintPure, meta = (ToolTip = "Returns a Worker Defnition that represents all of the Clients"),
			  Category = "Spatial Functional Test")
	FWorkerDefinition GetAllClients() { return FWorkerDefinition::AllClients; }

	ULayeredLBStrategy* GetLoadBalancingStrategy();

	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test",
			  meta = (ToolTip = "Add a debug tag to the given Actor that will be matched with interest and delegation declarations."))
	void AddDebugTag(AActor* Actor, FName Tag);

	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test", meta = (ToolTip = "Add a debug tag from the given Actor."))
	void RemoveDebugTag(AActor* Actor, FName Tag);

	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test",
			  meta = (ToolTip = "Add extra interest queries, allowing the current worker to see all Actors having the given tag."))
	void AddInterestOnTag(FName Tag);

	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test",
			  meta = (ToolTip = "Remove the extra interest query on the given tag."))
	void RemoveInterestOnTag(FName Tag);

	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test",
			  meta = (ToolTip = "Prevent the given actor from losing authority from this server worker."))
	void KeepActorOnCurrentWorker(AActor* Actor);

	// clang-format off
	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test", meta = (ToolTip = "Sets a Debug Tag to be delegated to a specific Server Worker, forcing the Authority to belong to it preventing the Load-Balancing Strategy from changing it."))
	// clang-format on
	void AddStepSetTagDelegation(FName Tag, int32 ServerWorkerId = 1);

	// clang-format off
	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test", meta = (ToolTip = "Clears delegation of a Debug Tag. If there's no delegation set, the Load-Balancing Strategy will decide which Server Worker should have Authority."))
	// clang-format on
	void AddStepClearTagDelegation(FName Tag);

	// clang-format off
	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test", meta = (ToolTip = "Clears all Debug Tag delegations and extra interest. Note that this is called automatically when a test ends, so if you use delegation / interest in the test, you don't need to clear it manually at the end."))
	// clang-format on
	void AddStepClearTagDelegationAndInterest();

	// # Require Functions. Requires mimic the assert behaviour but without the immediate failure. Since when you're
	// running networked tests you generally need to wait for state to be synced if you simply call asserts you'd get false
	// negatives. These functions work in a way that they record the expected behaviour, and when we FinishStep / FinishTest
	// it will let you know which of them passed and which failed. Keep in mind that failed requires will prevent FinishStep
	// from moving forward, so this allows you to make tests in a simpler way without having to keep track if anything failed
	// before calling FinishStep.

	// clang-format off
	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test")
	void RequireTrue(bool bCheckTrue, const FString& Msg) { RequireHandler.RequireTrue(bCheckTrue, Msg); }

	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test")
	void RequireFalse(bool bCheckFalse, const FString& Msg) { RequireHandler.RequireFalse(bCheckFalse, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Compare (Int)"), Category = "Spatial Functional Test")
	void RequireCompare_Int(int A, EComparisonMethod Operator, int B, const FString& Msg) { RequireHandler.RequireCompare(A, Operator, B, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Compare (Float)"), Category = "Spatial Functional Test")
	void RequireCompare_Float(float A, EComparisonMethod Operator, float B, const FString& Msg) { RequireHandler.RequireCompare(A, Operator, B, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Equal (Bool)"), Category = "Spatial Functional Test")
	void RequireEqual_Bool(bool bValue, bool bExpected, const FString& Msg) { RequireHandler.RequireEqual(bValue, bExpected, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Equal (Int)"), Category = "Spatial Functional Test")
	void RequireEqual_Int(int Value, int Expected, const FString& Msg) { RequireHandler.RequireEqual(Value, Expected, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Equal (Float)"), Category = "Spatial Functional Test")
	void RequireEqual_Float(float Value, float Expected, const FString& Msg, float Tolerance = 1.e-4) { RequireHandler.RequireEqual(Value, Expected, Msg, Tolerance); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Equal (String)"), Category = "Spatial Functional Test")
	void RequireEqual_String(const FString& Value, const FString& Expected, const FString& Msg) { RequireHandler.RequireEqual(Value, Expected, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Equal (Name)"), Category = "Spatial Functional Test")
	void RequireEqual_Name(const FName& Value, const FName& Expected, const FString& Msg) { RequireHandler.RequireEqual(Value, Expected, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Equal (Vector)"), Category = "Spatial Functional Test")
	void RequireEqual_Vector(const FVector& Value, const FVector& Expected, const FString& Msg, float Tolerance = 1.e-4) { RequireHandler.RequireEqual(Value, Expected, Msg, Tolerance); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Equal (Rotator)"), Category = "Spatial Functional Test")
	void RequireEqual_Rotator(const FRotator& Value, const FRotator& Expected, const FString& Msg, float Tolerance = 1.e-4) { RequireHandler.RequireEqual(Value, Expected, Msg, Tolerance); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Equal (Transform)"), Category = "Spatial Functional Test")
	void RequireEqual_Transform(const FTransform& Value, const FTransform& Expected, const FString& Msg, float Tolerance = 1.e-4) { RequireHandler.RequireEqual(Value, Expected, Msg, Tolerance); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Not Equal (Bool)"), Category = "Spatial Functional Test")
	void RequireNotEqual_Bool(bool bValue, bool bNotExpected, const FString& Msg) { RequireHandler.RequireNotEqual(bValue, bNotExpected, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Not Equal (Int)"), Category = "Spatial Functional Test")
	void RequireNotEqual_Int(int Value, int Expected, const FString& Msg) { RequireHandler.RequireNotEqual(Value, Expected, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Not Equal (Float)"), Category = "Spatial Functional Test")
	void RequireNotEqual_Float(float Value, float Expected, const FString& Msg) { RequireHandler.RequireNotEqual(Value, Expected, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Not Equal (String)"), Category = "Spatial Functional Test")
	void RequireNotEqual_String(const FString& Value, const FString& Expected, const FString& Msg) { RequireHandler.RequireNotEqual(Value, Expected, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Not Equal (Name)"), Category = "Spatial Functional Test")
	void RequireNotEqual_Name(const FName& Value, const FName& Expected, const FString& Msg) { RequireHandler.RequireNotEqual(Value, Expected, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Not Equal (Vector)"), Category = "Spatial Functional Test")
	void RequireNotEqual_Vector(const FVector& Value, const FVector& Expected, const FString& Msg) { RequireHandler.RequireNotEqual(Value, Expected, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Not Equal (Rotator)"), Category = "Spatial Functional Test")
	void RequireNotEqual_Rotator(const FRotator& Value, const FRotator& Expected, const FString& Msg) { RequireHandler.RequireNotEqual(Value, Expected, Msg); }

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Require Not Equal (Transform)"), Category = "Spatial Functional Test")
	void RequireNotEqual_Transform(const FTransform& Value, const FTransform& Expected, const FString& Msg) { RequireHandler.RequireNotEqual(Value, Expected, Msg); }
	// clang-format on

	// # Snapshot APIs.
	// clang-format off
	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test",
			  meta = (ToolTip = "Allows a Server Worker to request a SpatialOS snapshot to be taken. Keep in mind that this should be done at the last Step of your Test. Keep in mind that if you take a snapshot, you should eventually call ClearLoadedFromTakenSnapshot."))
	// clang-format on
	void TakeSnapshot(const FSpatialFunctionalTestSnapshotTakenDelegate& BlueprintCallback);

	// C++ version that allows you to hook up a lambda.
	void TakeSnapshot(const FSnapshotTakenFunc& CppCallback);

	// clang-format off
	UFUNCTION(BlueprintCallable, Category = "Spatial Functional Test",
			  meta = (Tooltip = "Clears the snapshot, making it start deployments with the default snapshot again. Tests that call TakeSnapshot should eventually also call ClearSnapshot."))
	// clang-format on
	void ClearSnapshot();

	// Allows you to know if the current deployment was started from a previously taken snapshot.
	UFUNCTION(BlueprintPure, Category = "Spatial Functional Test")
	bool WasLoadedFromTakenSnapshot();

	// Get the path of the taken snapshot for this world's map. Returns an empty string if it's using the default snapshot.
	static FString GetTakenSnapshotPath(UWorld* World);

	// Sets that this map was loaded by a taken snapshot, not meant to be used directly.
	static void SetLoadedFromTakenSnapshot();

	// Clears that this map was loaded by a taken snapshot, not meant to be used directly.
	static void ClearLoadedFromTakenSnapshot();

	// Clears all the snapshots taken, not meant to be used directly.
	static void ClearAllTakenSnapshots();

protected:
	void SetNumRequiredClients(int NewNumRequiredClients) { NumRequiredClients = FMath::Max(NewNumRequiredClients, 0); }

	int GetNumExpectedServers() const { return NumExpectedServers; }
	void DeleteActorsRegisteredForAutoDestroy();

	// Force Actors having the given tag to migrate and gain authority on the given worker. All server workers must declare
	// the same delegation at the same time, so we highly recommend that you use the AddStepSetTagDelegation() instead.
	void SetTagDelegation(FName Tag, int32 ServerWorkerId);

	// Remove the forced authority delegation. All server workers must declare the same delegation at the same time,
	// so we highly recommend that you use the AddStepClearTagDelegation() instead.
	void ClearTagDelegation(FName Tag);

	// Remove all the actor tags, extra interest, and authority delegation, resetting the Debug layer. All server workers must
	// call it at the same time to guarantee consistency, so we again highly recommend you use AddStepClearTagDelegationAndInterest().
	// Whenever a test finishes this will be called automatically.
	void ClearTagDelegationAndInterest();

	// # Built-in StepDefinitions for convenience

	// Step Definition that will take a SpatialOS snapshot for the current map. This snapshot will become the
	// default snapshot for the map it was taken with until you either clear it or the Automation Manager finishes
	// running the tests.
	UPROPERTY(BlueprintReadOnly, Category = "Spatial Functional Test")
	FSpatialFunctionalTestStepDefinition TakeSnapshotStepDefinition;

	// Step Definition that will clear the SpatialOS snapshot for the current map.
	UPROPERTY(BlueprintReadOnly, Category = "Spatial Functional Test")
	FSpatialFunctionalTestStepDefinition ClearSnapshotStepDefinition;

private:
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"), Category = "Spatial Functional Test")
	int NumRequiredClients = 2;

	// Number of servers that should be running in the world.
	int NumExpectedServers = 0;

	// FlowController which is locally owned.
	ASpatialFunctionalTestFlowController* LocalFlowController = nullptr;

	TArray<FSpatialFunctionalTestStepDefinition> StepDefinitions;

	TArray<ASpatialFunctionalTestFlowController*> FlowControllersExecutingStep;

	// Time current step has been running for, used if Step Definition has TimeLimit >= 0.
	float TimeRunningStep = 0.0f;

	// Cached test result while we wait all Workers to acknowledge they finished the test.
	EFunctionalTestResult CachedTestResult;

	// Cached test result message while we wait all Workers to acknowledge they finished the test.
	FString CachedTestMessage;

	// Handle for waiting for acknowledgment from all workers that the test is finished.
	FTimerHandle FinishTestTimerHandle;

	// Current Step Index, < 0 if not executing any, check consts at the top.
	UPROPERTY(ReplicatedUsing = OnReplicated_CurrentStepIndex, Transient)
	int CurrentStepIndex = SPATIAL_FUNCTIONAL_TEST_NOT_STARTED;

	UFUNCTION()
	void OnReplicated_CurrentStepIndex();

	UPROPERTY(ReplicatedUsing = OnReplicated_bPreparedTest, Transient)
	bool bPreparedTest = false;

	UFUNCTION()
	void OnReplicated_bPreparedTest();

	UPROPERTY(Replicated, Transient)
	TArray<ASpatialFunctionalTestFlowController*> FlowControllers;

	// Holds all the Requires calls / results for printing at the end of the step.
	SpatialFunctionalTestRequireHandler RequireHandler;

	UFUNCTION()
	void StartServerFlowControllerSpawn();

	void SetupClientPlayerRegistrationFlow();
	void EndPlay(const EEndPlayReason::Type Reason) override;

	FDelegateHandle PostLoginDelegate;

	// Sets the snapshot for the map loaded by this world. When launching the test maps, the AutomationManager will
	// check if there's a snapshot for that map and if so use it instead of the default snapshot. If PathToSnapshot
	// is empty, it clears the entry for that map.
	static bool SetSnapshotForMap(UWorld* World, const FString& PathToSnapshot);

	// Holds if currently we're running from a taken snapshot and not the default snapshot.
	static bool bWasLoadedFromTakenSnapshot;

	// Holds the paths of all the snapshots taken during tests. Test maps before running in the AutomationManager will
	// will check if there's a snapshot for them, and if so launch with it instead of the default snapshot.
	static TMap<FString, FString> TakenSnapshots;
};
