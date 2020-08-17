// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Tests/TestDefinitions.h"

#include "HAL/IPlatformFileProfilerWrapper.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/Paths.h"
#include "Misc/ScopeTryLock.h"

#define EXAMPLE_SIMPLE_TEST(TestName) GDK_TEST(SpatialGDKExamples, SimpleExamples, TestName)

#define EXAMPLE_COMPLEX_TEST(TestName) GDK_COMPLEX_TEST(SpatialGDKExamples, ComplexExamples, TestName)

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKExamples, Log, All);
DEFINE_LOG_CATEGORY(LogSpatialGDKExamples);

// 1. Latent command example
namespace
{
const double MAX_WAIT_TIME_FOR_BACKGROUND_COMPUTATION = 2.0;
const double MIN_WAIT_TIME_FOR_BACKGROUND_COMPUTATION = 1.0;
const double COMPUTATION_DURATION = 1.0;

struct ComputationResult
{
	FCriticalSection Mutex;
	int Value = 0;
};

} // anonymous namespace

DEFINE_LATENT_AUTOMATION_COMMAND_ONE_PARAMETER(FStartBackgroundThreadComputation, TSharedPtr<ComputationResult>, InResult);
bool FStartBackgroundThreadComputation::Update()
{
	TSharedPtr<ComputationResult> LocalResult = InResult;
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [LocalResult] {
		FScopeLock BackgroundComputationLock(&LocalResult->Mutex);
		FPlatformProcess::Sleep(COMPUTATION_DURATION);
		LocalResult->Value = 42;
	});

	return true;
}

DEFINE_LATENT_AUTOMATION_COMMAND_TWO_PARAMETER(FWaitForComputationAndCheckResult, FAutomationTestBase*, Test, TSharedPtr<ComputationResult>,
											   InResult);
bool FWaitForComputationAndCheckResult::Update()
{
	const double TimePassed = FPlatformTime::Seconds() - StartTime;

	if (TimePassed >= MIN_WAIT_TIME_FOR_BACKGROUND_COMPUTATION)
	{
		FScopeTryLock BackgroundComputationLock(&InResult->Mutex);

		if (BackgroundComputationLock.IsLocked())
		{
			Test->TestTrue("Computation result is equal to expected value", InResult->Value == 42);
			return true;
		}

		if (TimePassed >= MAX_WAIT_TIME_FOR_BACKGROUND_COMPUTATION)
		{
			Test->TestTrue("Computation finished in time", false);
			return true;
		}
	}

	return false;
}

EXAMPLE_SIMPLE_TEST(GIVEN_initial_value_WHEN_performing_background_compuation_THEN_the_result_is_correct)
{
	TSharedPtr<ComputationResult> ResultPtr = MakeShared<ComputationResult>();

	ADD_LATENT_AUTOMATION_COMMAND(FStartBackgroundThreadComputation(ResultPtr));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitForComputationAndCheckResult(this, ResultPtr));

	return true;
}

// 2. Simple test example
EXAMPLE_SIMPLE_TEST(GIVEN_one_and_two_WHEN_summed_THEN_the_sum_is_three)
{
	int X = 1;
	int Y = 2;
	int Sum = X + Y;

	TestTrue("The sum is correct", Sum == 3);

	return true;
}

// 3. Complex test example
EXAMPLE_COMPLEX_TEST(ComplexTest);
bool ComplexTest::RunTest(const FString& Parameters)
{
	TArray<FString> OutArray;
	Parameters.ParseIntoArrayWS(OutArray);

	if (OutArray.Num() != 3)
	{
		UE_LOG(LogSpatialGDKExamples, Error, TEXT("Invalid Test Input"));
		TestTrue("The input is valid", false);
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

void ComplexTest::GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	OutBeautifiedNames.Add(TEXT("GIVEN_two_and_two_WHEN_summed_THEN_the_sum_is_four"));
	OutTestCommands.Add(TEXT("2 2 4"));

	OutBeautifiedNames.Add(TEXT("GIVEN_three_and_five_WHEN_summed_THEN_the_sum_is_eight"));
	OutTestCommands.Add(TEXT("3 5 8"));
}

// 4. Example of test fixture
namespace
{
const FString ExampleTestFolder = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("ExampleTests/"));

class ExampleTestFixture
{
public:
	ExampleTestFixture() { CreateTestFolders(); }
	~ExampleTestFixture() { DeleteTestFolders(); }

private:
	void CreateTestFolders()
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.CreateDirectoryTree(*ExampleTestFolder);
	}

	void DeleteTestFolders()
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		PlatformFile.DeleteDirectoryRecursively(*ExampleTestFolder);
	}
};
} // anonymous namespace

EXAMPLE_SIMPLE_TEST(GIVEN_empty_folder_WHEN_creating_a_file_THEN_the_file_has_been_created)
{
	ExampleTestFixture Fixture;

	FString FilePath = FPaths::Combine(ExampleTestFolder, TEXT("Example.txt"));

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.OpenWrite(*FilePath);

	TestTrue("Example.txt exists", PlatformFile.FileExists(*FilePath));

	return true;
}
