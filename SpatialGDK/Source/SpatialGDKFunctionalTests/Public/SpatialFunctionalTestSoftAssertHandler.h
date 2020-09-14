// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "FunctionalTest.h"

class ASpatialFunctionalTest;

struct FSpatialFunctionalTestSoftAssert
{
	FString Msg;
	bool bPassed;
	FString ErrorMsg;
	uint32 Order;
};

namespace
{
} // namespace

/* This class handles all the SoftAssert functionality used by the ASpatialFunctionalTest. Because of the way networked tests
 * work we can't simply assert when checking values since we need to wait for them to propagate through the network. This
 * handler provides an API that is similar to asserts but doesn't actually abort on failure, and instead gathers data to
 * print at the end of a step / test.
 */
class SpatialFunctionalTestSoftAssertHandler
{
public:
	SpatialFunctionalTestSoftAssertHandler();

	void SetOwnerTest(ASpatialFunctionalTest* SpatialFunctionalTest) { OwnerTest = SpatialFunctionalTest; }

	void SoftAssertTrue(bool bCheckTrue, const FString& Msg);

	void SoftAssertFalse(bool bCheckFalse, const FString& Msg);

	void SoftAssertInt(int A, EComparisonMethod Operator, int B, const FString& Msg);

	void SoftAssertFloat(float A, EComparisonMethod Operator, float B, const FString& Msg, const float EqualityTolerance = 0.0001f);

	void GenericSoftAssert(const FString& Key, bool bPassed, const FString& ErrorMsg);

	void LogAndClearStepSoftAsserts();

	bool HasFails();

private:
	ASpatialFunctionalTest* OwnerTest;

	FString GetComparisonMethodAsString(EComparisonMethod Operator);

	uint32 NextOrder;

	TMap<FString, FSpatialFunctionalTestSoftAssert> SoftAsserts;
};
