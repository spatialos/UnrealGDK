// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"

class ASpatialFunctionalTest;

struct FSpatialFunctionalTestRequire
{
	FString Msg;
	bool bPassed;
	FString ErrorMsg;
	uint32 Order; // Used to be able to log the messages in the same order they occurred at the end.
};

/* This class handles all the Require functionality used by the ASpatialFunctionalTest. Because of the way networked tests
 * work we can't simply assert when checking values since we need to wait for them to propagate through the network. This
 * handler provides an API that is similar to asserts but doesn't actually abort on failure, and instead gathers data to
 * print at the end of a step / test.
 * Note that if there's fails in the Requires, ASpatialFunctionalTest::FinishStep() will not proceed.
 */
class SPATIALGDKFUNCTIONALTESTS_API
	SpatialFunctionalTestRequireHandler // Apparently, this requires the SPATIALGDKFUNCTIONALTESTS_API macro in Mac builds, even though the
										// functions here are not directly exposed
{
public:
	SpatialFunctionalTestRequireHandler();

	void SetOwnerTest(ASpatialFunctionalTest* SpatialFunctionalTest) { OwnerTest = SpatialFunctionalTest; }

	bool RequireTrue(bool bCheckTrue, const FString& Msg);
	bool RequireFalse(bool bCheckFalse, const FString& Msg);

	bool RequireCompare(int A, EComparisonMethod Operator, int B, const FString& Msg);
	bool RequireCompare(float A, EComparisonMethod Operator, float B, const FString& Msg);

	bool RequireEqual(bool bValue, bool bExpected, const FString& Msg);
	bool RequireEqual(int Value, int Expected, const FString& Msg);
	bool RequireEqual(float Value, float Expected, const FString& Msg, float Tolerance);
	bool RequireEqual(const FString& Value, const FString& Expected, const FString& Msg);
	bool RequireEqual(const FName& Value, const FName& Expected, const FString& Msg);
	bool RequireEqual(const FVector& Value, const FVector& Expected, const FString& Msg, float Tolerance);
	bool RequireEqual(const FRotator& Value, const FRotator& Expected, const FString& Msg, float Tolerance);
	bool RequireEqual(const FTransform& Value, const FTransform& Expected, const FString& Msg, float Tolerance);

	bool RequireNotEqual(bool bValue, bool bNotExpected, const FString& Msg);
	bool RequireNotEqual(int Value, int NotExpected, const FString& Msg);
	bool RequireNotEqual(float Value, float NotExpected, const FString& Msg);
	bool RequireNotEqual(const FString& Value, const FString& NotExpected, const FString& Msg);
	bool RequireNotEqual(const FName& Value, const FName& NotExpected, const FString& Msg);
	bool RequireNotEqual(const FVector& Value, const FVector& NotExpected, const FString& Msg);
	bool RequireNotEqual(const FRotator& Value, const FRotator& NotExpected, const FString& Msg);
	bool RequireNotEqual(const FTransform& Value, const FTransform& NotExpected, const FString& Msg);

	bool GenericRequire(const FString& Key, bool bPassed, const FString& ErrorMsg);

	void LogAndClearStepRequires();

	bool HasFails();

private:
	ASpatialFunctionalTest* OwnerTest;

	uint32 NextOrder;

	TMap<FString, FSpatialFunctionalTestRequire> Requires;
};
