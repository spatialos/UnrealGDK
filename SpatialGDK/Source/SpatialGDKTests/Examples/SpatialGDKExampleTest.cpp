// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "TestDefinitions.h"

#include "Misc/ScopeTryLock.h"

#define EXAMPLE_SIMPLE_TEST(TestName) \
	TEST(SpatialGDKExamples, SimpleExamples, TestName)

#define EXAMPLE_COMPLEX_TEST(TestName) \
	COMPLEX_TEST(SpatialGDKExamples, ComplexExamples, TestName)

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKExamples, Log, All);
DEFINE_LOG_CATEGORY(LogSpatialGDKExamples);

namespace
{
	const double MAX_WAIT_TIME_FOR_SLOW_COMPUTATION = 2.0;
	const double MIN_WAIT_TIME_FOR_SLOW_COMPUTATION = 1.0;
	const double COMPUTATION_DURATION = 1.0;

	struct
	{
		FCriticalSection Mutex;
		int Value;
	} ComputationResult;
}

EXAMPLE_SIMPLE_TEST(GIVEN_one_and_two_WHEN_summed_THEN_the_sum_is_three)
{
	int X = 1;
	int Y = 2;
	int Sum = X + Y;

	TestTrue("The sum is correct", Sum == 3);

	return true;
}

EXAMPLE_COMPLEX_TEST(ComplexTest)
{
	TArray<FString> OutArray;
	Parameters.ParseIntoArrayWS(OutArray);

	if (OutArray.Num() != 3)
	{
		UE_LOG(LogSpatialGDKExamples, Error, TEXT("Invalid Test Input"));
		return true;
	}

	TArray<int> ArgsAndResult;
	for (const auto& Value : OutArray)
	{
		if (Value.IsNumeric())
		{
			ArgsAndResult.Push(FCString::Atoi(*Value));
		}
	}

	int Sum = ArgsAndResult[0] + ArgsAndResult[1];
	TestTrue("The sum is correct", Sum == ArgsAndResult[2]);

	return true;
}

void ComplexTest::GetTests(TArray<FString>& OutBeautifiedNames, TArray <FString>& OutTestCommands) const
{
	OutBeautifiedNames.Add(TEXT("GIVEN_two_and_two_WHEN_summed_THEN_the_sum_is_four"));
	OutTestCommands.Add(TEXT("2 2 4"));

	OutBeautifiedNames.Add(TEXT("GIVEN_three_and_five_WHEN_summed_THEN_the_sum_is_eight"));
	OutTestCommands.Add(TEXT("3 5 8"));
}

DEFINE_LATENT_COMMAND(StartSlowComputation)
{
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, []
	{
		FScopeLock SlowComputationLock(&ComputationResult.Mutex);
		FPlatformProcess::Sleep(COMPUTATION_DURATION);
		ComputationResult.Value = 42;
		// TODO(Alex): do some computation and return a variable here
	});

	return true;
}

DEFINE_LATENT_COMMAND(WaitForComputationToFinish)
{
	const double NewTime = FPlatformTime::Seconds();
	const double TimePassed = FPlatformTime::Seconds() - StartTime;

	if (TimePassed >= MIN_WAIT_TIME_FOR_SLOW_COMPUTATION)
	{
		FScopeTryLock SlowComputationLock(&ComputationResult.Mutex);

		if (SlowComputationLock.IsLocked())
		{
			return true;
		}

		if (TimePassed >= MAX_WAIT_TIME_FOR_SLOW_COMPUTATION)
		{
			UE_LOG(LogSpatialGDKExamples, Error, TEXT("Computation timed out"));
			return true;
		}
	}

	return false;
}

DEFINE_LATENT_COMMAND_ONE_PARAMETER(WaitForComputationAndCheckResult, FAutomationTestBase*, Test)
{
	const double NewTime = FPlatformTime::Seconds();
	const double TimePassed = FPlatformTime::Seconds() - StartTime;

	if (TimePassed >= MIN_WAIT_TIME_FOR_SLOW_COMPUTATION)
	{
		FScopeTryLock SlowComputationLock(&ComputationResult.Mutex);

		if (SlowComputationLock.IsLocked())
		{
			Test->TestTrue("", ComputationResult.Value == 42);
			return true;
		}

		if (TimePassed >= MAX_WAIT_TIME_FOR_SLOW_COMPUTATION)
		{
			UE_LOG(LogSpatialGDKExamples, Error, TEXT("Computation timed out"));
			return true;
		}
	}

	return false;
}

EXAMPLE_SIMPLE_TEST(GIVEN_WHEN_THEN)
{
	ADD_LATENT_AUTOMATION_COMMAND(StartSlowComputation());
	//ADD_LATENT_AUTOMATION_COMMAND(WaitForComputationToFinish());
	ADD_LATENT_AUTOMATION_COMMAND(WaitForComputationAndCheckResult(this));

    return true;
}
