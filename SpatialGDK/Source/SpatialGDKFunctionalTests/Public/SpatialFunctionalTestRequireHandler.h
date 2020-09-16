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
	uint32 Order; // Used to be able to the messages in the same order they occurred at the end.
};

/* This class handles all the Require functionality used by the ASpatialFunctionalTest. Because of the way networked tests
 * work we can't simply assert when checking values since we need to wait for them to propagate through the network. This
 * handler provides an API that is similar to asserts but doesn't actually abort on failure, and instead gathers data to
 * print at the end of a step / test.
 * Note that if there's fails in the Requires, ASpatialFunctionalTest::FinishStep() will not proceed.
 */
class SpatialFunctionalTestRequireHandler
{
public:
	SpatialFunctionalTestRequireHandler();

	void SetOwnerTest(ASpatialFunctionalTest* SpatialFunctionalTest) { OwnerTest = SpatialFunctionalTest; }

	void RequireTrue(bool bCheckTrue, const FString& Msg);
	void RequireFalse(bool bCheckFalse, const FString& Msg);

	void RequireCompare(int A, EComparisonMethod Operator, int B, const FString& Msg);
	void RequireCompare(float A, EComparisonMethod Operator, float B, const FString& Msg);

	void RequireEqual(bool bValue, bool bExpected, const FString& Msg);
	void RequireEqual(int Value, int Expected, const FString& Msg);
	void RequireEqual(float Value, float Expected, const FString& Msg, float Tolerance);
	void RequireEqual(const FString& Value, const FString& Expected, const FString& Msg);
	void RequireEqual(const FName& Value, const FName& Expected, const FString& Msg);
	void RequireEqual(const FVector& Value, const FVector& Expected, const FString& Msg, float Tolerance);
	void RequireEqual(const FRotator& Value, const FRotator& Expected, const FString& Msg, float Tolerance);
	void RequireEqual(const FTransform& Value, const FTransform& Expected, const FString& Msg, float Tolerance);

	void RequireNotEqual(bool bValue, bool bNotExpected, const FString& Msg);
	void RequireNotEqual(int Value, int NotExpected, const FString& Msg);
	void RequireNotEqual(float Value, float NotExpected, const FString& Msg);
	void RequireNotEqual(const FString& Value, const FString& NotExpected, const FString& Msg);
	void RequireNotEqual(const FName& Value, const FName& NotExpected, const FString& Msg);
	void RequireNotEqual(const FVector& Value, const FVector& NotExpected, const FString& Msg);
	void RequireNotEqual(const FRotator& Value, const FRotator& NotExpected, const FString& Msg);
	void RequireNotEqual(const FTransform& Value, const FTransform& NotExpected, const FString& Msg);

	void GenericRequire(const FString& Key, bool bPassed, const FString& ErrorMsg);

	void LogAndClearStepRequires();

	bool HasFails();

private:
	ASpatialFunctionalTest* OwnerTest;

	uint32 NextOrder;

	TMap<FString, FSpatialFunctionalTestRequire> Requires;
};
