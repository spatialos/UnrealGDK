#include "SpatialActorResolutionTest.h"

#include "Engine/World.h"
#include "Misc/CoreDelegates.h"

#include "SpatialFunctionalTestStep.h"

ASelfDestroyingActor::ASelfDestroyingActor()
{
	bReplicates = true;
}

void ASelfDestroyingActor::PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker)
{
	TSharedPtr<FDelegateHandle> DelegateHandle = MakeShared<FDelegateHandle>();

	// OnEndFrame decouples us from tick rates and gives no opportunity for the NetDriver to tick in-between.
	*DelegateHandle = FCoreDelegates::OnEndFrame.AddLambda([this, DelegateHandle]() {
		Destroy();
		FCoreDelegates::OnEndFrame.Remove(*DelegateHandle);
	});

	return Super::PreReplication(ChangedPropertyTracker);
}

ASpatialActorResolutionTest::ASpatialActorResolutionTest()
{
	Author = TEXT("Dmitrii <dmitriikozlov@improbable.io>");
}

/*
 * This test creates an actor that destroys itself after replicating for the first time, then
 * waits to check if there are any errors. Created to validate UNR-5752.
 */
void ASpatialActorResolutionTest::PrepareTest()
{
	Super::PrepareTest();

	AddStep(TEXT("Spawn the self-destroying actor and save its EntityID"), FWorkerDefinition::Server(1), nullptr, [this]() {
		TestActor = GetWorld()->SpawnActor<ASelfDestroyingActor>();
		FinishStep();
	});

	AddStep(TEXT("Wait until the test actor is destroyed"), FWorkerDefinition::Server(1), nullptr, nullptr, [this](float) {
		RequireTrue(TestActor.IsStale(), TEXT("The actor became invalid"));
		FinishStep();
	});

	AddStep(
		TEXT("Wait for a predetermined timeout to see there's no errors"), FWorkerDefinition::Server(1), nullptr,
		[this]() {
			TimeElapsed = 0.0f;
		},
		[this](float DeltaTime) {
			TimeElapsed += DeltaTime;
			RequireCompare_Float(TimeElapsed, EComparisonMethod::Greater_Than, 1.0f, TEXT("Waited for required time"));
			FinishStep();
		});
}
