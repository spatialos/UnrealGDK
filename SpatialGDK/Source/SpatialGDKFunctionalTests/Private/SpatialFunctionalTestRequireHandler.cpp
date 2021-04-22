// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialFunctionalTestRequireHandler.h"
#include "Logging/LogMacros.h"
#include "Misc/AssertionMacros.h"
#include "SpatialFunctionalTest.h"
#include "SpatialFunctionalTestFlowController.h"
#include "SpatialGDKFunctionalTestsPrivate.h"
#include "VisualLogger/VisualLogger.h"

namespace
{
template <typename T>
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
	default:
		checkNoEntry();
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
	default:
		checkNoEntry();
		break;
	}

	return FString(); // For compilation.
}

FString GetTransformAsString(const FTransform& Transform)
{
	FVector T = Transform.GetLocation();
	FRotator R = Transform.Rotator();
	FVector S = Transform.GetScale3D();
	return FString::Printf(TEXT("T(%f,%f,%f) | R(%f, %f, %f), S(%f, %f, %f)"), T.X, T.Y, T.Z, R.Pitch, R.Yaw, R.Roll, S.X, S.Y, S.Z);
}
} // namespace

FString GenerateStatusMessage(bool bPassed, FString Received, FString Expected, FString Tolerance = FString(), bool bNotEqual = false)
{
	if (bNotEqual)
	{
		if (bPassed)
		{
			return FString::Printf(TEXT("Received (%s), not equal to (%s), as expected"), *Received, *Expected);
		}
		return FString::Printf(TEXT("Received (%s) but wasn't expecting it"), *Received);
	}
	if (bPassed)
	{
		if (Tolerance.IsEmpty())
		{
			return FString::Printf(TEXT("Received (%s) as expected"), *Received);
		}
		return FString::Printf(TEXT("Received (%s) as expected (within tolerance %s of (%s))"), *Received, *Tolerance, *Expected);
	}
	else
	{
		if (Tolerance.IsEmpty())
		{
			return FString::Printf(TEXT("Received (%s) but was expecting (%s)"), *Received, *Expected);
		}
		return FString::Printf(TEXT("Received (%s) but was expecting (%s) (tolerance %s)"), *Received, *Expected, *Tolerance);
	}
}

SpatialFunctionalTestRequireHandler::SpatialFunctionalTestRequireHandler()
	: NextOrder(0)
{
}

bool SpatialFunctionalTestRequireHandler::RequireTrue(bool bCheckTrue, const FString& Msg)
{
	bool bPassed = bCheckTrue;
	FString ReceivedString = bCheckTrue ? TEXT("True") : TEXT("False");
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, /*Expected =*/TEXT("True"));

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireFalse(bool bCheckFalse, const FString& Msg)
{
	bool bPassed = !bCheckFalse;
	FString ReceivedString = bCheckFalse ? TEXT("True") : TEXT("False");
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, /*Expected = */ TEXT("False"));

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireCompare(int A, EComparisonMethod Operator, int B, const FString& Msg)
{
	bool bPassed = Compare(A, Operator, B);
	FString StatusMsg;
	FString OperatorStr = GetComparisonMethodAsString(Operator);

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received %d %s %d as expected"), A, *OperatorStr, B);
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received %d %s %d but was expecting A %s B"), A, *OperatorStr, B, *OperatorStr);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireCompare(float A, EComparisonMethod Operator, float B, const FString& Msg)
{
	bool bPassed = Compare(A, Operator, B);
	FString StatusMsg;
	FString OperatorStr = GetComparisonMethodAsString(Operator);

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received %f %s %f as expected"), A, *OperatorStr, B);
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received %f %s %f but expected A %s B"), A, *OperatorStr, B, *OperatorStr);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(bool bValue, bool bExpected, const FString& Msg)
{
	bool bPassed = bValue == bExpected;
	FString ReceivedString = bValue ? TEXT("True") : TEXT("False");
	FString ExpectedString = bExpected ? TEXT("True") : TEXT("False");
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(int Value, int Expected, const FString& Msg)
{
	bool bPassed = Value == Expected;
	FString ReceivedString = FString::Printf(TEXT("%d"), Value);
	FString ExpectedString = FString::Printf(TEXT("%d"), Expected);
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(float Value, float Expected, const FString& Msg, float Tolerance)
{
	bool bPassed = FMath::Abs(Value - Expected) <= Tolerance;
	FString ReceivedString = FString::Printf(TEXT("%f"), Value);
	FString ExpectedString = FString::Printf(TEXT("%f"), Expected);
	FString ToleranceString = FString::Printf(TEXT("%f"), Tolerance);
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString, ToleranceString);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(const FString& Value, const FString& Expected, const FString& Msg)
{
	bool bPassed = Value == Expected;
	FString StatusMsg = GenerateStatusMessage(bPassed, /*Received = */ Value, /*Expected = */ Expected);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(const FName& Value, const FName& Expected, const FString& Msg)
{
	bool bPassed = Value == Expected;
	FString ReceivedString = Value.ToString();
	FString ExpectedString = Expected.ToString();
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(const FVector& Value, const FVector& Expected, const FString& Msg, float Tolerance)
{
	bool bPassed = Value.Equals(Expected, Tolerance);
	FString ReceivedString = Value.ToString();
	FString ExpectedString = Expected.ToString();
	FString ToleranceString = FString::Printf(TEXT("%f"), Tolerance);
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString, ToleranceString);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(const FRotator& Value, const FRotator& Expected, const FString& Msg, float Tolerance)
{
	bool bPassed = Value.Equals(Expected, Tolerance);
	FString ReceivedString = Value.ToString();
	FString ExpectedString = Expected.ToString();
	FString ToleranceString = FString::Printf(TEXT("%f"), Tolerance);
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString, ToleranceString);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(const FTransform& Value, const FTransform& Expected, const FString& Msg,
													   float Tolerance)
{
	bool bPassed = Value.Equals(Expected, Tolerance);
	FString ReceivedString = FString::Printf(TEXT("%s"), *GetTransformAsString(Value));
	FString ExpectedString = FString::Printf(TEXT("%s"), *GetTransformAsString(Expected));
	FString ToleranceString = FString::Printf(TEXT("%f"), Tolerance);
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString, ToleranceString);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(bool bValue, bool bNotExpected, const FString& Msg)
{
	bool bPassed = bValue != bNotExpected;
	FString ReceivedString = bValue ? TEXT("True") : TEXT("False");
	FString ExpectedString = bNotExpected ? TEXT("True") : TEXT("False");
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString, /*Tolerance = */ FString(), /*bNotEqual = */ true);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(int Value, int NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString ReceivedString = FString::Printf(TEXT("%d"), Value);
	FString ExpectedString = FString::Printf(TEXT("%d"), NotExpected);
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString, /*Tolerance = */ FString(), /*bNotEqual = */ true);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(float Value, float NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString ReceivedString = FString::Printf(TEXT("%f"), Value);
	FString ExpectedString = FString::Printf(TEXT("%f"), NotExpected);
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString, /*Tolerance = */ FString(), /*bNotEqual = */ true);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(const FString& Value, const FString& NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString, /*Tolerance = */ FString(), /*bNotEqual = */ true);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(const FName& Value, const FName& NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString ReceivedString = FString::Printf(TEXT("%s"), *Value.ToString());
	FString ExpectedString = FString::Printf(TEXT("%s"), *NotExpected.ToString());
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString, /*Tolerance = */ FString(), /*bNotEqual = */ true);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(const FVector& Value, const FVector& NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString ReceivedString = FString::Printf(TEXT("%s"), *Value.ToString());
	FString ExpectedString = FString::Printf(TEXT("%s"), *NotExpected.ToString());
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString, /*Tolerance = */ FString(), /*bNotEqual = */ true);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(const FRotator& Value, const FRotator& NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString ReceivedString = FString::Printf(TEXT("%s"), *Value.ToString());
	FString ExpectedString = FString::Printf(TEXT("%s"), *NotExpected.ToString());
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString, /*Tolerance = */ FString(), /*bNotEqual = */ true);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(const FTransform& Value, const FTransform& NotExpected, const FString& Msg)
{
	bool bPassed = !Value.Equals(NotExpected);
	FString ReceivedString = FString::Printf(TEXT("%s"), *GetTransformAsString(Value));
	FString ExpectedString = FString::Printf(TEXT("%s"), *GetTransformAsString(NotExpected));
	FString StatusMsg = GenerateStatusMessage(bPassed, ReceivedString, ExpectedString, /*Tolerance = */ FString(), /*bNotEqual = */ true);

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::GenericRequire(const FString& Msg, bool bPassed, const FString& StatusMsg)
{
	ensureMsgf(!Msg.IsEmpty(), TEXT("Requires cannot have an empty message"));

	FSpatialFunctionalTestRequire Require;
	Require.Msg = Msg;
	Require.bPassed = bPassed;
	Require.StatusMsg = StatusMsg;
	Require.Order = NextOrder++;

	Requires.Add(Msg, Require);

	return bPassed;
}

void SpatialFunctionalTestRequireHandler::LogAndClearStepRequires()
{
	// Since it's a TMap, we need to order them for better readability.
	TArray<FSpatialFunctionalTestRequire> RequiresOrdered;
	RequiresOrdered.Reserve(Requires.Num());

	for (const auto& RequireEntry : Requires)
	{
		RequiresOrdered.Add(RequireEntry.Value);
	}

	RequiresOrdered.Sort([](const FSpatialFunctionalTestRequire& A, const FSpatialFunctionalTestRequire& B) -> bool {
		return A.Order < B.Order;
	});

	const FString& WorkerName = OwnerTest->GetLocalFlowController()->GetDisplayName();

	for (const auto& Require : RequiresOrdered)
	{
		FString Msg;
		if (Require.bPassed)
		{
			Msg = FString::Printf(TEXT("%s [Passed] %s : \"%s\""), *WorkerName, *Require.Msg, *Require.StatusMsg);
			UE_VLOG(nullptr, LogSpatialGDKFunctionalTests, Display, TEXT("%s"), *Msg);
			UE_LOG(LogSpatialGDKFunctionalTests, Display, TEXT("%s"), *Msg);
		}
		else
		{
			Msg = FString::Printf(TEXT("%s [Failed] %s : %s"), *WorkerName, *Require.Msg, *Require.StatusMsg);
			UE_VLOG(nullptr, LogSpatialGDKFunctionalTests, Error, TEXT("%s"), *Msg);
			UE_LOG(LogSpatialGDKFunctionalTests, Error, TEXT("%s"), *Msg);
		}
	}

	NextOrder = 0;
	Requires.Empty();
}

bool SpatialFunctionalTestRequireHandler::HasFails()
{
	for (const auto& RequireEntry : Requires)
	{
		if (!RequireEntry.Value.bPassed)
		{
			return true;
		}
	}
	return false;
}
