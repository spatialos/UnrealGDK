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

SpatialFunctionalTestRequireHandler::SpatialFunctionalTestRequireHandler()
	: NextOrder(0)
{
}

bool SpatialFunctionalTestRequireHandler::RequireTrue(bool bCheckTrue, const FString& Msg)
{
	const FString StatusMsg = TEXT("Received True as expected");

	return GenericRequire(Msg, bCheckTrue, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireFalse(bool bCheckFalse, const FString& Msg)
{
	const FString StatusMsg = TEXT("Received False as expected");

	return GenericRequire(Msg, !bCheckFalse, StatusMsg);
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
		StatusMsg = FString::Printf(TEXT("Received %f %s %f but was expected A %s B"), A, *OperatorStr, B, *OperatorStr);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(bool bValue, bool bExpected, const FString& Msg)
{
	bool bPassed = bValue == bExpected;
	FString StatusMsg;
	FString ValueStr = bValue ? TEXT("True") : TEXT("False");

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received %s as expected"), *ValueStr);
	}
	else
	{
		FString ExpectedStr = bExpected ? TEXT("True") : TEXT("False");
		StatusMsg = FString::Printf(TEXT("Received %s but was expecting %s"), *ValueStr, *ExpectedStr);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(int Value, int Expected, const FString& Msg)
{
	bool bPassed = Value == Expected;
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received %d as expected"), Value);
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received %d but was expecting %d"), Value, Expected);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(float Value, float Expected, const FString& Msg, float Tolerance)
{
	bool bPassed = FMath::Abs(Value - Expected) <= Tolerance;
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received %f as expected (within tolerance %f of %f)"), Value, Tolerance, Expected);
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received %f but was expecting %f (tolerance %f)"), Value, Expected, Tolerance);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(const FString& Value, const FString& Expected, const FString& Msg)
{
	bool bPassed = Value == Expected;
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received %s as expected"), *Value);
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received %s but was expecting %s"), *Value, *Expected);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(const FName& Value, const FName& Expected, const FString& Msg)
{
	bool bPassed = Value == Expected;
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received %s as expected"), *Value.ToString());
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received %s but was expecting %s"), *Value.ToString(), *Expected.ToString());
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(const FVector& Value, const FVector& Expected, const FString& Msg, float Tolerance)
{
	bool bPassed = Value.Equals(Expected, Tolerance);
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received (%s) as expected (within tolerance %f of (%s))"), *Value.ToString(), Tolerance,
									*Expected.ToString());
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received (%s) but was expecting (%s) (tolerance %f)"), *Value.ToString(), *Expected.ToString(),
									Tolerance);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(const FRotator& Value, const FRotator& Expected, const FString& Msg, float Tolerance)
{
	bool bPassed = Value.Equals(Expected, Tolerance);
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received (%s) as expected (within tolerance %f of (%s))"), *Value.ToString(), Tolerance,
									*Expected.ToString());
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received (%s) but was expecting (%s) (tolerance %f)"), *Value.ToString(), *Expected.ToString(),
									Tolerance);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireEqual(const FTransform& Value, const FTransform& Expected, const FString& Msg,
													   float Tolerance)
{
	bool bPassed = Value.Equals(Expected, Tolerance);
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received (%s) as expected (within tolerance %f of (%s))"), *GetTransformAsString(Value),
									Tolerance, *GetTransformAsString(Expected));
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received {%s} but was expecting {%s} (tolerance %f)"), *GetTransformAsString(Value),
									*GetTransformAsString(Expected), Tolerance);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(bool bValue, bool bNotExpected, const FString& Msg)
{
	bool bPassed = bValue != bNotExpected;
	FString StatusMsg;
	FString ValueStr = bValue ? TEXT("True") : TEXT("False");
	FString ExpectedStr = bNotExpected ? TEXT("True") : TEXT("False");

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received %s, not equal to %s, as expected"), *ValueStr, *ExpectedStr);
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received %s but wasn't expecting it"), *ValueStr);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(int Value, int NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received value %d, not equal to %d, as expected"), Value, NotExpected);
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received %d but wasn't expecting it"), Value);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(float Value, float NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received value %f, not equal to %f, as expected"), Value, NotExpected);
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received %f but wasn't expecting it"), Value);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(const FString& Value, const FString& NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received value %s, not equal to %s, as expected"), *Value, *NotExpected);
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received %s but wasn't expecting it"), *Value);
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(const FName& Value, const FName& NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received value %s, not equal to %s, as expected"), *Value.ToString(), *NotExpected.ToString());
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received %s but wasn't expecting it"), *Value.ToString());
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(const FVector& Value, const FVector& NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received value %s, not equal to %s, as expected"), *Value.ToString(), *NotExpected.ToString());
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received (%s) but wasn't expecting it"), *Value.ToString());
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(const FRotator& Value, const FRotator& NotExpected, const FString& Msg)
{
	bool bPassed = Value != NotExpected;
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received value %s, not equal to %s, as expected"), *Value.ToString(), *NotExpected.ToString());
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received (%s) but wasn't expecting it"), *Value.ToString());
	}

	return GenericRequire(Msg, bPassed, StatusMsg);
}

bool SpatialFunctionalTestRequireHandler::RequireNotEqual(const FTransform& Value, const FTransform& NotExpected, const FString& Msg)
{
	bool bPassed = !Value.Equals(NotExpected);
	FString StatusMsg;

	if (bPassed)
	{
		StatusMsg = FString::Printf(TEXT("Received value %s, not equal to %s, as expected"), *GetTransformAsString(Value),
									*GetTransformAsString(NotExpected));
	}
	else
	{
		StatusMsg = FString::Printf(TEXT("Received {%s} but wasn't expecting it"), *GetTransformAsString(Value));
	}

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
