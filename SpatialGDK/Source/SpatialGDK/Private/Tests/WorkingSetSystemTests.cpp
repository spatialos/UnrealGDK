// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "LoadBalancing/WorkingSetSystem.h"

#include "Algo/Count.h"
#include "Misc/AutomationTest.h"

#include "Schema/WorkingSet.h"
#include "Tests/SpatialView/TestWorker.h"

namespace SpatialGDK
{
template <typename TCollection>
static FWorkingSetMarkerRequest CreateRequest(const int64 Epoch, const Worker_EntityId LeaderEntityId, TCollection MemberEntities)
{
	FWorkingSetState RequestedState;
	RequestedState.Epoch = Epoch;
	RequestedState.MemberEntities = TSet<Worker_EntityId_Key>(MemberEntities);
	RequestedState.LeaderEntityId = LeaderEntityId;
	return FWorkingSetMarkerRequest(RequestedState);
}

bool AreSetsEqual(const TSet<Worker_EntityId_Key>& Lhs, const TSet<Worker_EntityId_Key>& Rhs)
{
	return Lhs.Num() == Rhs.Num() && Lhs.Intersect(Rhs).Num() == Rhs.Num();
}

struct FWorkingSetSystemTestResult
{
	struct FOutcome
	{
		struct FWorkingSetState
		{
			TSet<Worker_EntityId_Key> TargetEntities;
			TSet<Worker_EntityId_Key> ConfirmedEntities;
			Worker_EntityId LeaderEntityId;
		};

		FOutcome(std::initializer_list<TPairInitializer<const Worker_EntityId_Key&, const FWorkingSetState&>> InitList)
			: States(InitList)
		{
		}

		TMap<Worker_EntityId_Key, FWorkingSetState> States;

		bool SystemMatchesOutcome(const FWorkingSetSystem& InSystem, TArray<FString>& OutDescription) const
		{
			for (const auto& ExpectedWorkingSetState : States)
			{
				const auto ActualSetState = InSystem.GetSet(ExpectedWorkingSetState.Key);

				if (!ActualSetState)
				{
					OutDescription.Emplace(FString::Printf(TEXT("Working set missing. MarkerEntityId: %lld"), ExpectedWorkingSetState.Key));
					return false;
				}

				if (!AreSetsEqual(ActualSetState->TargetEntities, ExpectedWorkingSetState.Value.TargetEntities))
				{
					OutDescription.Emplace(
						FString::Printf(TEXT("Target entities wrong. MarkerEntityId: %lld"), ExpectedWorkingSetState.Key));

					return false;
				}

				if (!AreSetsEqual(ActualSetState->ConfirmedEntities, ExpectedWorkingSetState.Value.ConfirmedEntities))
				{
					OutDescription.Emplace(
						FString::Printf(TEXT("Confirmed entities wrong. MarkerEntityId: %lld"), ExpectedWorkingSetState.Key));

					return false;
				}

				if (ActualSetState->LeaderEntity != ExpectedWorkingSetState.Value.LeaderEntityId)
				{
					OutDescription.Emplace(FString::Printf(TEXT("Leader entity wrong. MarkerEntityId: %lld"), ExpectedWorkingSetState.Key));

					return false;
				}
			}
			return true;
		}
	};

	FWorkingSetSystemTestResult(TArray<FOutcome> InOutcomes)
		: AcceptableOutcomes(InOutcomes)
	{
	}

	TArray<FOutcome> AcceptableOutcomes;

	bool SystemMatchesOneOutcome(const FWorkingSetSystem& InSystem, TArray<FString>& OutDescription) const
	{
		int32 OutcomesMatched = 0;
		for (int32 OutcomeIndex = 0; OutcomeIndex < AcceptableOutcomes.Num(); ++OutcomeIndex)
		{
			OutDescription.Emplace(FString::Printf(TEXT("Checking outcome: %d"), OutcomeIndex));
			TArray<FString> OutcomeErrors;
			if (AcceptableOutcomes[OutcomeIndex].SystemMatchesOutcome(InSystem, OutcomeErrors))
			{
				++OutcomesMatched;
			}
			for (FString& OutcomeError : OutcomeErrors)
			{
				OutcomeError = FString::Printf(TEXT("\t%s"), *OutcomeError);
			}
			OutDescription.Append(OutcomeErrors);
		}
		return OutcomesMatched == 1;
	}
};

class FWorkingSetSystemTestFixture
{
public:
	FWorkingSetSystemTestFixture()
		: TestWorker(FTestWorker::Create({}))
		, System([](Worker_EntityId) {
			return SpatialConstants::INVALID_ENTITY_ID;
		})
		, MarkerEntitiesSubview(CreateWorkingSetMarkersSubview(TestWorker.GetCoordinator()))
	{
	}

	FTargetView& GetTargetView() { return TestWorker.GetTargetView(); }
	FWorkingSetSystem& GetSystem() { return System; }
	const FSubView& GetMarkerEntitiesSubview() const { return MarkerEntitiesSubview; }
	void Advance(const TSet<Worker_EntityId_Key>& MigratingEntities)
	{
		TestWorker.AdvanceToTargetView(0.0f);
		System.Advance(MarkerEntitiesSubview);
		System.UpdateMigratingEntities(MigratingEntities, TestWorker.GetCoordinator());
	}
	FWorkingSetState GetWorkingSetResponse(Worker_EntityId MarkerEntityId) const
	{
		return FWorkingSetCommonData(TestWorker.GetCoordinator().GetView()[MarkerEntityId]).ConfirmedState;
	}

	bool IsActualSetEqual(Worker_EntityId MarkerEntityId, const TSet<Worker_EntityId_Key>& MemberEntities) const
	{
		const auto WorkingSet = System.GetSet(MarkerEntityId);
		if (WorkingSet)
		{
			return AreSetsEqual(WorkingSet->ConfirmedEntities, MemberEntities);
		}
		return false;
	}
	bool IsRequestedSetEqual(Worker_EntityId MarkerEntityId, const TSet<Worker_EntityId_Key>& MemberEntities) const
	{
		const auto WorkingSet = System.GetSet(MarkerEntityId);
		if (WorkingSet)
		{
			return AreSetsEqual(WorkingSet->TargetEntities, MemberEntities);
		}
		return false;
	}

private:
	FTestWorker TestWorker;
	FWorkingSetSystem System;
	FSubView& MarkerEntitiesSubview;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWorkingSetSystemTest, "SpatialGDK.WorkingSets.Test",
								 EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FWorkingSetSystemTest::RunTest(const FString& Parameters)
{
	FWorkingSetSystemTestFixture Fixture;

	FWorkingSetSystem& System = Fixture.GetSystem();

	constexpr Worker_EntityId LeaderEntityId = 100;
	constexpr Worker_EntityId MarkerEntityId = 1;
	const TSet<Worker_EntityId_Key> EmptySet;
	const TSet<Worker_EntityId_Key> FilledSet{ 2 };

	{
		for (const Worker_EntityId_Key MemberEntityId : FilledSet)
		{
			Fixture.GetTargetView().AddEntity(MemberEntityId);
		}
		Fixture.Advance(EmptySet);
		TestFalse(TEXT("Marker entity isn't in view"), System.GetSet(MarkerEntityId).IsSet());
	}
	{
		// A marker entity is now in view, requesting FilledSet as members.
		Fixture.GetTargetView().AddEntity(MarkerEntityId);
		Fixture.GetTargetView().AddOrSetComponent(MarkerEntityId, CreateRequest(1, LeaderEntityId, FilledSet).CreateComponentData());
		Fixture.GetTargetView().AddOrSetComponent(MarkerEntityId, FWorkingSetMarkerResponse().CreateComponentData());
		Fixture.GetTargetView().AddOrSetComponent(MarkerEntityId, ComponentData(SpatialConstants::WORKING_SET_MARKER_TAG_COMPONENT_ID));

		// FilledSet entities are being migrated right now.
		Fixture.Advance(FilledSet);

		// Expect set not to be formed yet.
		TestTrue(TEXT("No entities in set yet"), Fixture.IsActualSetEqual(MarkerEntityId, EmptySet));
	}
	{
		// All entities have migrated.
		Fixture.Advance(EmptySet);

		// Expect set to be formed.
		TestTrue(TEXT("Marker entity is now in view"), System.GetSet(MarkerEntityId).IsSet());
		TestTrue(TEXT("Correct entities in set"), Fixture.IsActualSetEqual(MarkerEntityId, FilledSet));
		TestTrue(TEXT("Correct entities in requested set"), Fixture.IsRequestedSetEqual(MarkerEntityId, FilledSet));

		TestEqual(TEXT("Local view updated with correct epoch"), Fixture.GetWorkingSetResponse(MarkerEntityId).Epoch, 1LL);
		TestTrue(TEXT("Local view updated with correct epoch"),
				 AreSetsEqual(Fixture.GetWorkingSetResponse(MarkerEntityId).MemberEntities, FilledSet));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWorkingSetSystemConflictResolution, "SpatialGDK.WorkingSets.ConflictResolution",
								 EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FWorkingSetSystemConflictResolution::RunTest(const FString& Parameters)
{
	FWorkingSetSystemTestFixture Fixture;

	FWorkingSetSystem& System = Fixture.GetSystem();

	constexpr Worker_EntityId LeaderEntityId = 100;
	constexpr Worker_EntityId MarkerEntityIds[] = { 1, 2 };
	const TSet<Worker_EntityId_Key> EmptySet;
	const TSet<Worker_EntityId_Key> FilledSet{ 3 };

	{
		// All entities in FilledSet are in view.
		for (const Worker_EntityId_Key MemberEntityId : FilledSet)
		{
			Fixture.GetTargetView().AddEntity(MemberEntityId);
		}

		Fixture.Advance(EmptySet);

		// Expect entities in view not to be set members.
		for (const Worker_EntityId_Key MarkerEntityId : MarkerEntityIds)
		{
			TestFalse(TEXT("Marker entity isn't in view"), System.GetSet(MarkerEntityId).IsSet());
		}
	}
	{
		// Multiple marker entities appear, all claiming FilledSet entities.
		for (const Worker_EntityId_Key MarkerEntityId : MarkerEntityIds)
		{
			Fixture.GetTargetView().AddEntity(MarkerEntityId);

			Fixture.GetTargetView().AddOrSetComponent(MarkerEntityId, CreateRequest(1, LeaderEntityId, FilledSet).CreateComponentData());
			Fixture.GetTargetView().AddOrSetComponent(MarkerEntityId, FWorkingSetMarkerResponse().CreateComponentData());
			Fixture.GetTargetView().AddOrSetComponent(MarkerEntityId, ComponentData(SpatialConstants::WORKING_SET_MARKER_TAG_COMPONENT_ID));
		}

		// FilledSet entities are being migrated.
		Fixture.Advance(FilledSet);

		// Expect no working sets to be formed yet.
		for (const Worker_EntityId_Key MarkerEntityId : MarkerEntityIds)
		{
			TestTrue(TEXT("No entities in set yet"), Fixture.IsActualSetEqual(MarkerEntityId, EmptySet));
		}
	}

	// Exit early to avoid crashing in the next step as we're accessing sets without checking if they exist.
	if (HasAnyErrors())
	{
		return false;
	}
	{
		// All entities have been migrated.
		Fixture.Advance(EmptySet);

		// Expect a single working set to be formed as sets were requesting same entities.
		int32 TotalWorkingSets = 0;
		for (const Worker_EntityId_Key MarkerEntityId : MarkerEntityIds)
		{
			TotalWorkingSets += System.GetSet(MarkerEntityId)->ConfirmedEntities.Num() != 0 ? 1 : 0;

			TestEqual(TEXT("Local view updated with correct epoch"), Fixture.GetWorkingSetResponse(MarkerEntityId).Epoch, 1LL);
			TestTrue(TEXT("Local view updated with correct epoch"),
					 AreSetsEqual(Fixture.GetWorkingSetResponse(MarkerEntityId).MemberEntities,
								  System.GetSet(MarkerEntityId)->ConfirmedEntities));
		}
		TestEqual(TEXT("Only one working set approved"), TotalWorkingSets, 1);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWorkingSetSystemConflictResolution3, "SpatialGDK.WorkingSets.ConflictResolution3",
								 EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::EngineFilter)

bool FWorkingSetSystemConflictResolution3::RunTest(const FString& Parameters)
{
	FWorkingSetSystemTestFixture Fixture;
	const FWorkingSetSystem& System = Fixture.GetSystem();

	constexpr Worker_EntityId LeaderEntityId = 100;
	constexpr Worker_EntityId MarkerEntityIds[] = { 1, 2 };
	const TSet<Worker_EntityId_Key> EmptySet;
	const TSet<Worker_EntityId_Key> FilledSet{ 3 };
	const TSet<Worker_EntityId_Key> UpdatedSets[]{ { 3, 4 }, { 4 } };

	const FWorkingSetSystemTestResult::FOutcome Outcome1{ { MarkerEntityIds[0], { UpdatedSets[0], UpdatedSets[0], LeaderEntityId } },
														  { MarkerEntityIds[1],
															{ EmptySet, EmptySet, SpatialConstants::INVALID_ENTITY_ID } } };

	const FWorkingSetSystemTestResult::FOutcome Outcome2{ { MarkerEntityIds[0], { FilledSet, FilledSet, LeaderEntityId } },
														  { MarkerEntityIds[1], { UpdatedSets[1], UpdatedSets[1], LeaderEntityId } } };

	const FWorkingSetSystemTestResult::FOutcome Outcome3{ { MarkerEntityIds[0],
															{ EmptySet, EmptySet, SpatialConstants::INVALID_ENTITY_ID } },
														  { MarkerEntityIds[1], { UpdatedSets[1], UpdatedSets[1], LeaderEntityId } } };

	const FWorkingSetSystemTestResult ExpectedResult({ Outcome1, Outcome2, Outcome3 });

	{
		// All member entities are in view.
		TSet<Worker_EntityId_Key> AllMemberEntityIds;
		for (const Worker_EntityId_Key MemberEntityId : FilledSet)
		{
			AllMemberEntityIds.Emplace(MemberEntityId);
		}
		for (const auto& UpdatedSet : UpdatedSets)
		{
			for (const Worker_EntityId_Key MemberEntityId : UpdatedSet)
			{
				AllMemberEntityIds.Emplace(MemberEntityId);
			}
		}
		for (const Worker_EntityId_Key MemberEntityId : AllMemberEntityIds)
		{
			Fixture.GetTargetView().AddEntity(MemberEntityId);
		}

		// No entities are migrating.
		Fixture.Advance(EmptySet);

		// Expect no working sets to be formed.
		for (const Worker_EntityId_Key MarkerEntityId : MarkerEntityIds)
		{
			TestFalse(TEXT("Marker entity isn't in view"), System.GetSet(MarkerEntityId).IsSet());
		}
	}
	{
		// Multiple marker entities all claiming FilledSet appear.
		for (const Worker_EntityId_Key MarkerEntityId : MarkerEntityIds)
		{
			Fixture.GetTargetView().AddEntity(MarkerEntityId);
			Fixture.GetTargetView().AddOrSetComponent(MarkerEntityId, CreateRequest(1, LeaderEntityId, FilledSet).CreateComponentData());
			Fixture.GetTargetView().AddOrSetComponent(MarkerEntityId, FWorkingSetMarkerResponse().CreateComponentData());
			Fixture.GetTargetView().AddOrSetComponent(MarkerEntityId, ComponentData(SpatialConstants::WORKING_SET_MARKER_TAG_COMPONENT_ID));
		}

		// Entities are still being migrated...
		Fixture.Advance(FilledSet);

		// Entities have been migrated, some working sets should have been formed as checked by other tests.
		Fixture.Advance(EmptySet);
	}
	{
		// Marker entities are now claiming two conflicting sets of entities.
		for (int32 MarkerEntityIndex = 0; MarkerEntityIndex < 2; ++MarkerEntityIndex)
		{
			const Worker_EntityId MarkerEntityId = MarkerEntityIds[MarkerEntityIndex];
			const TSet<Worker_EntityId_Key>& TargetSet = UpdatedSets[MarkerEntityIndex];

			Fixture.GetTargetView().UpdateComponent(MarkerEntityId, CreateRequest(2, LeaderEntityId, TargetSet).CreateComponentUpdate());
		}

		// All entities have been migrated.
		Fixture.Advance(EmptySet);

		// Expect an acceptable outcome, as defined in the array above.
		TArray<FString> Description;
		if (!TestTrue(TEXT("Working sets achieved one of the acceptable states"),
					  ExpectedResult.SystemMatchesOneOutcome(System, Description)))
		{
			AddError(FString::Join(Description, LINE_TERMINATOR));
		}
	}

	return true;
}
} // namespace SpatialGDK
