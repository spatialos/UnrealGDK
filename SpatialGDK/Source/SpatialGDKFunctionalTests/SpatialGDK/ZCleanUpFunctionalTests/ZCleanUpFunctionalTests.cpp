// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "ZCleanUpFunctionalTests.h"
#include "Editor.h"

/**
 * This test cleans up the resources used by the functional tests.
 * Since tests are ran in alphabetical order, this test is named such that it runs last.
 *
 * The TestGym map this test is ran on is similarly named.
 */

AZCleanUpFunctionalTests::AZCleanUpFunctionalTests()
	: Super()
{
	Author = "Thomas Yung (thomasyung@improbable.io)";
	Description = TEXT("This test cleans up the resources used by the functional tests.");
}

void AZCleanUpFunctionalTests::PrepareTest()
{
	Super::PrepareTest();

	// This kills any workers and stops the deployment
	GEditor->RequestEndPlayMap();
}
