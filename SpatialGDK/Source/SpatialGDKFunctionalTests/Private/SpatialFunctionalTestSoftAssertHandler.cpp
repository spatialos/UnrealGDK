// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTestSoftAssertHandler.h"
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTestsPrivate.h"
#include "VisualLogger/VisualLogger.h"

SpatialFunctionalTestSoftAssertHandler::SpatialFunctionalTestSoftAssertHandler()
	: NextOrder(0)
{
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertTrue(bool bCheckTrue, const FString& Msg)
{
	GenericSoftAssert(Msg, bCheckTrue, FString());
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertFalse(bool bCheckFalse, const FString& Msg)
{
	GenericSoftAssert(Msg, !bCheckFalse, FString());
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertInt(int A, EComparisonMethod Operator, int B, const FString& Msg)
{
	bool bPassed = false;
	switch (Operator)
	{
	case EComparisonMethod::Equal_To:
		bPassed = A == B;
		break;
	case EComparisonMethod::Not_Equal_To:
		bPassed = A != B;
		break;
	case EComparisonMethod::Greater_Than_Or_Equal_To:
		bPassed = A >= B;
		break;
	case EComparisonMethod::Less_Than_Or_Equal_To:
		bPassed = A <= B;
		break;
	case EComparisonMethod::Greater_Than:
		bPassed = A > B;
		break;
	case EComparisonMethod::Less_Than:
		bPassed = A < B;
		break;
	}

	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Expected %d %s %d"), A, *GetComparisonMethodAsString(Operator), B);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

FString SpatialFunctionalTestSoftAssertHandler::GetComparisonMethodAsString(EComparisonMethod Operator)
{
	switch (Operator)
	{
	case EComparisonMethod::Equal_To:
		return TEXT("==");
	case EComparisonMethod::Not_Equal_To:
		return TEXT("!=");
	case EComparisonMethod::Greater_Than_Or_Equal_To:
		return TEXT(">=");
	case EComparisonMethod::Less_Than_Or_Equal_To:
		return TEXT("<=");
	case EComparisonMethod::Greater_Than:
		return TEXT(">");
	case EComparisonMethod::Less_Than:
		return TEXT("<");
	}

	check(false);
	return FString(); // For compilation.
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertFloat(float A, EComparisonMethod Operator, float B, const FString& Msg,
															 const float EqualityTolerance /*= 0.0001f*/)
{
	bool bPassed = false;
	switch (Operator)
	{
	case EComparisonMethod::Equal_To:
		bPassed = FMath::Abs(A - B) < EqualityTolerance;
		break;
	case EComparisonMethod::Not_Equal_To:
		bPassed = FMath::Abs(A - B) > EqualityTolerance;
		break;
	case EComparisonMethod::Greater_Than_Or_Equal_To:
		bPassed = A > B || FMath::Abs(A - B) < EqualityTolerance;
		break;
	case EComparisonMethod::Less_Than_Or_Equal_To:
		bPassed = A < B || FMath::Abs(A - B) < EqualityTolerance;
		break;
	case EComparisonMethod::Greater_Than:
		bPassed = A > B;
		break;
	case EComparisonMethod::Less_Than:
		bPassed = A < B;
		break;
	}

	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Expected %f %s %f (equality tolerance = %f)"), A, *GetComparisonMethodAsString(Operator), B,
								   EqualityTolerance);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::GenericSoftAssert(const FString& Msg, bool bPassed, const FString& ErrorMsg)
{
	ensureMsgf(!Msg.IsEmpty(), TEXT("SoftAsserts cannot have an empty message"));

	FSpatialFunctionalTestSoftAssert SoftAssert;
	SoftAssert.Msg = Msg;
	SoftAssert.bPassed = bPassed;
	SoftAssert.ErrorMsg = ErrorMsg;
	SoftAssert.Order = NextOrder++;

	SoftAsserts.Add(Msg, SoftAssert);
}

void SpatialFunctionalTestSoftAssertHandler::LogAndClearStepSoftAsserts()
{
	// Since it's a TMap, we need to order them for better readability.
	TArray<FSpatialFunctionalTestSoftAssert> SoftAssertsOrdered;
	SoftAssertsOrdered.Reserve(SoftAsserts.Num());

	for (const auto& SoftAssertEntry : SoftAsserts)
	{
		SoftAssertsOrdered.Add(SoftAssertEntry.Value);
	}

	SoftAssertsOrdered.Sort([](const FSpatialFunctionalTestSoftAssert& A, const FSpatialFunctionalTestSoftAssert& B) -> bool {
		return A.Order < B.Order;
	});

	const FString& WorkerName = OwnerTest->GetLocalFlowController()->GetDisplayName();

	for (const auto& SoftAssert : SoftAssertsOrdered)
	{
		FString Msg;
		if (SoftAssert.bPassed)
		{
			Msg = FString::Printf(TEXT("%s [Passed] %s"), *WorkerName, *SoftAssert.Msg);
			UE_VLOG(nullptr, LogSpatialGDKFunctionalTests, Display, TEXT("%s"), *Msg);
			UE_LOG(LogSpatialGDKFunctionalTests, Display, TEXT("%s"), *Msg);
		}
		else
		{
			Msg = FString::Printf(TEXT("%s [Failed] %s : %s"), *WorkerName, *SoftAssert.Msg, *SoftAssert.ErrorMsg);
			UE_VLOG(nullptr, LogSpatialGDKFunctionalTests, Error, TEXT("%s"), *Msg);
			UE_LOG(LogSpatialGDKFunctionalTests, Error, TEXT("%s"), *Msg);
		}
	}

	NextOrder = 0;
	SoftAsserts.Empty();
}

bool SpatialFunctionalTestSoftAssertHandler::HasFails()
{
	for (const auto& SoftAssertEntry : SoftAsserts)
	{
		if (!SoftAssertEntry.Value.bPassed)
		{
			return true;
		}
	}
	return false;
}
