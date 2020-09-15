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

	void SoftAssertCompare(int A, EComparisonMethod Operator, int B, const FString& Msg);
	void SoftAssertCompare(float A, EComparisonMethod Operator, float B, const FString& Msg);

	void SoftAssertEqual(bool bValue, bool bExpected, const FString& Msg);
	void SoftAssertEqual(int Value, int Expected, const FString& Msg);
	void SoftAssertEqual(float Value, float Expected, const FString& Msg, float Tolerance);
	void SoftAssertEqual(const FString& Value, const FString& Expected, const FString& Msg);
	void SoftAssertEqual(const FName& Value, const FName& Expected, const FString& Msg);
	void SoftAssertEqual(const FVector& Value, const FVector& Expected, const FString& Msg, float Tolerance);
	void SoftAssertEqual(const FRotator& Value, const FRotator& Expected, const FString& Msg, float Tolerance);
	void SoftAssertEqual(const FTransform& Value, const FTransform& Expected, const FString& Msg, float Tolerance);

	void SoftAssertNotEqual(bool bValue, bool bNotExpected, const FString& Msg);
	void SoftAssertNotEqual(int Value, int NotExpected, const FString& Msg);
	void SoftAssertNotEqual(float Value, float NotExpected, const FString& Msg);
	void SoftAssertNotEqual(const FString& Value, const FString& NotExpected, const FString& Msg);
	void SoftAssertNotEqual(const FName& Value, const FName& NotExpected, const FString& Msg);
	void SoftAssertNotEqual(const FVector& Value, const FVector& NotExpected, const FString& Msg);
	void SoftAssertNotEqual(const FRotator& Value, const FRotator& NotExpected, const FString& Msg);
	void SoftAssertNotEqual(const FTransform& Value, const FTransform& NotExpected, const FString& Msg);

	void GenericSoftAssert(const FString& Key, bool bPassed, const FString& ErrorMsg);

	void LogAndClearStepSoftAsserts();

	bool HasFails();

private:
	ASpatialFunctionalTest* OwnerTest;

	uint32 NextOrder;

	TMap<FString, FSpatialFunctionalTestSoftAssert> SoftAsserts;
};
