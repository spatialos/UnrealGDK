#pragma once

#include "AutomationTest.h"

#include "SpatialOSTest.generated.h"

/*
 * Base class for all SpatialOS tests.
 * This is a UObject so that adding/removing/updating tests can work with hot-reloading in the
 * editor.
 *
 * Tests are gathered via Unreal's reflection system.
 *
 * Your test class name must be suffixed with "Test", e.g. "UEntityIdTest"
 * Your test class .cpp file must contain the DEFINE_SPATIAL_AUTOMATION_TEST macro:
 *   Using the above example of "UEntityIdTest", you should write:
 *
 *   DEFINE_SPATIAL_AUTOMATION_TEST(EntityId)
 *
 *   Your test functions must be marked as UFUNCTION(), and prefixed with "Test":
 *
 *   UFUNCTION()
 *   void TestMyFirstUnitTest()
 *   {
 *     T->TestTrue(TEXT("Easy test", true);
 *   }
 *
 *   The above will generate a test named "MyFirstUnitTest".
 *
 *   You can also create a test that is parameterized by a comma-separated list of inputs:
 *
 *   UFUNCTION(meta=(TestCase="str1,str2,str3"))
 *   void TestMyParameterizedTest(const FString& Parameter)
 *   {
 *     T->TestTrue(TEXT("It's not empty.", Parameter.Length() > 0);
 *   }
 *
 *   The above will generate three tests:
 *   MyParameterizedTest(str1)
 *   MyParameterizedTest(str2)
 *   MyParameterizedTest(str3)
 */

DECLARE_DELEGATE(SpatialOSTestDelegate)
// clang-format off
DECLARE_DELEGATE_OneParam(SpatialOSTestCaseDelegate, const FString&)

DECLARE_LOG_CATEGORY_EXTERN(LogTestFramework, Log, All);

UCLASS(Abstract) 
class SDK_API USpatialOSTest : public UObject
// clang-format on
{
  GENERATED_BODY()

public:
  USpatialOSTest();

  void GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const;
  void RunTest(FAutomationTestBase* T, const FString& TestName);

protected:
  void AddTest(const FString& TestName, SpatialOSTestDelegate Test);

  /*
   * The instance of the test runner. Use this in your tests.
   */
  FAutomationTestBase* T;

  TMap<FString, SpatialOSTestDelegate> Tests;
};

/*
 * Implements a complex automation test that supports hot-reloading.
 * This is what makes the reflectively-discovered tests visible to Unreal's
 * automation testing framework.
 */
#define DEFINE_SPATIAL_AUTOMATION_TEST(ClassName)                                         \
                                                                                          \
  IMPLEMENT_COMPLEX_AUTOMATION_TEST(                                                      \
      F##ClassName##Test, "SpatialOS." #ClassName, EAutomationTestFlags::ServerContext |  \
          EAutomationTestFlags::CommandletContext | EAutomationTestFlags::EditorContext | \
          EAutomationTestFlags::ClientContext | EAutomationTestFlags::CriticalPriority |  \
          EAutomationTestFlags::ProductFilter);                                           \
                                                                                          \
  bool F##ClassName##Test::RunTest(const FString& Parameters)                             \
                                                                                          \
  {                                                                                       \
    USpatialOSTest* Test = NewObject<U##ClassName##Test>();                               \
    Test->RunTest(this, Parameters);                                                      \
    return true;                                                                          \
  }                                                                                       \
                                                                                          \
  void F##ClassName##Test::GetTests(TArray<FString>& OutBeautifiedNames,                  \
                                    TArray<FString>& OutTestCommands) const               \
                                                                                          \
  {                                                                                       \
    USpatialOSTest* Test = NewObject<U##ClassName##Test>();                               \
    Test->GetTests(OutBeautifiedNames, OutTestCommands);                                  \
  }
