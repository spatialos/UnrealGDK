// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTestSoftAssertHandler.h"
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTestsPrivate.h"
#include "VisualLogger/VisualLogger.h"

namespace
{
template<typename T>
bool Compare(const T& A, EComparisonMethod Operator, const T& B)
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
	return bPassed;
}

FString GetComparisonMethodAsString(EComparisonMethod Operator)
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

FString GetTransformAsString(const FTransform& Transform)
{
	FVector T = Transform.GetLocation();
	FRotator R = Transform.Rotator();
	FVector S = Transform.GetScale3D();
	return FString::Printf(TEXT("T(%f,%f,%f) | R(%f, %f, %f), S(%f, %f, %f)"), T.X, T.Y, T.Z, R.Pitch, R.Yaw, R.Roll, S.X, S.Y, S.Z);
}
}

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

void SpatialFunctionalTestSoftAssertHandler::SoftAssertCompare(int A, EComparisonMethod Operator, int B, const FString& Msg)
{
	bool bPassed = Compare(A, Operator, B);

	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Expected %d %s %d"), A, *GetComparisonMethodAsString(Operator), B);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertCompare(float A, EComparisonMethod Operator, float B, const FString& Msg)
{
	bool bPassed = Compare(A, Operator, B);

	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Expected %f %s %f"), A, *GetComparisonMethodAsString(Operator), B);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertEqual(bool bValue, bool bExpected, const FString& Msg)
{
	bool bPassed = bValue == bExpected;
	FString ErrorMsg;

	if (!bPassed)
	{
		FString ValueStr = bValue ? TEXT("True") : TEXT("False");
		FString ExpectedStr = bExpected ? TEXT("True") : TEXT("False");
		ErrorMsg = FString::Printf(TEXT("Received %s but was expecting %s"), *ValueStr, *ExpectedStr);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertEqual(int Value, int Expected, const FString& Msg)
{
	bool bPassed = Value == Expected;
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received %d but was expecting %d"), Value, Expected);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertEqual(float Value, float Expected, const FString& Msg, float Tolerance)
{
	bool bPassed = FMath::Abs(Value-Expected) < Tolerance;
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received %f but was expecting %f (tolerance %f)"), Value, Expected, Tolerance);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertEqual(const FString& Value, const FString& Expected, const FString& Msg)
{
	bool bPassed = Value == Expected;
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received %s but was expecting %s"), *Value, *Expected);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertEqual(const FName& Value, const FName& Expected, const FString& Msg)
{
	bool bPassed = Value == Expected;
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received %s but was expecting %s"), *Value.ToString(), *Expected.ToString());
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertEqual(const FVector& Value, const FVector& Expected, const FString& Msg,
															 float Tolerance)
{
	bool bPassed = Value.Equals(Expected, Tolerance);
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received (%s) but was expecting (%s) (tolerance %f)"), *Value.ToString(), *Expected.ToString(), Tolerance);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertEqual(const FRotator& Value, const FRotator& Expected, const FString& Msg,
															 float Tolerance)
{
	bool bPassed = Value.Equals(Expected, Tolerance);
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received (%s) but was expecting (%s) (tolerance %f)"), *Value.ToString(), *Expected.ToString(), Tolerance);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertEqual(const FTransform& Value, const FTransform& Expected, const FString& Msg,
															 float Tolerance)
{
	bool bPassed = Value.Equals(Expected, Tolerance);
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received {%s} but was expecting {%s} (tolerance %f)"), *GetTransformAsString(Value), *GetTransformAsString(Expected), Tolerance);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertNotEqual(bool bValue, bool bNotExpected, const FString& Msg)
{
	bool bPassed = bValue != bNotExpected;
	FString ErrorMsg;

	if (!bPassed)
	{
		FString ValueStr = bValue ? TEXT("True") : TEXT("False");
		ErrorMsg = FString::Printf(TEXT("Received %s but wasn't expecting it"), *ValueStr);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertNotEqual(int Value, int NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received %d but wasn't expecting it"), Value);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertNotEqual(float Value, float NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received %f but wasn't expecting it"), Value);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertNotEqual(const FString& Value, const FString& NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received %s but wasn't expecting it"), *Value);
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertNotEqual(const FName& Value, const FName& NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received %s but wasn't expecting it"), *Value.ToString());
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertNotEqual(const FVector& Value, const FVector& NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received (%s) but wasn't expecting it"), *Value.ToString());
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertNotEqual(const FRotator& Value, const FRotator& NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received (%s) but wasn't expecting it"), *Value.ToString());
	}

	GenericSoftAssert(Msg, bPassed, ErrorMsg);
}

void SpatialFunctionalTestSoftAssertHandler::SoftAssertNotEqual(const FTransform& Value, const FTransform& NotExpected, const FString& Msg)
{
	bool bPassed = Value.Equals(NotExpected);
	FString ErrorMsg;

	if (!bPassed)
	{
		ErrorMsg = FString::Printf(TEXT("Received {%s} but wasn't expecting it"), *GetTransformAsString(Value));
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
