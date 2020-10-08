// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialSnapshotDummyTest.h"

/**
 * This test is made to always just pass. The way we have to test snapshots, you need to have an additional
 * map with at least some test because of the way the whole automation works.
 * Please check ASpatialSnapshotTest for a full explanation.
 */

ASpatialSnapshotDummyTest::ASpatialSnapshotDummyTest()
	: Super()
{
	Author = "Nuno";
	Description = TEXT("Dummy Test that just passes");
	SetNumRequiredClients(1);
}

void ASpatialSnapshotDummyTest::PrepareTest()
{
	Super::PrepareTest();

	{
		AddStep(TEXT("Always Pass"), FWorkerDefinition::Server(1), nullptr, [this]() {
			FinishStep();
		});
	}
}
