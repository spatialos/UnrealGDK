
#include "SpatialOSTest.h"

#include "LogMacros.h"

DEFINE_LOG_CATEGORY(LogTestFramework);

namespace
{
const FString TestPrefix(TEXT("Test"));
const FString TestCaseMetaDataKey(TEXT("TestCase"));

bool IsTestFunction(const UFunction* Function)
{
  return Function->GetName().StartsWith(TestPrefix) && Function->GetReturnProperty() == nullptr;
}

bool IsValidTestCaseFunction(const UFunction* Function)
{
  if (!Function->HasMetaData(TEXT("TestCase")))
  {
    UE_LOG(LogTestFramework, Error, TEXT("%s has a parameter, but has no TestCase metadata."),
           *Function->GetName());
    return false;
  }

  if (!Function->Children->IsA(UStrProperty::StaticClass()))
  {
    UE_LOG(LogTestFramework, Error, TEXT("%s has a parameter, but it is not 'const FString&'."),
           *Function->GetName());
    return false;
  }

  return true;
}

FString GetTestName(const UFunction* Function)
{
  return Function->GetName().RightChop(4);
}

FString GetTestName(const UFunction* Function, const FString& Parameter)
{
  return FString::Printf(TEXT("%s(%s)"), *GetTestName(Function), *Parameter);
}
}  // anonymous namespace

USpatialOSTest::USpatialOSTest() : T(nullptr)
{
  /*
   * Reflectively gather all UFUNCTIONs (see rules for what qualifies in the header)
   * Then bind a delegate to each and store an association between the test's name and that
   * delegate.
   * This association is used in concert with the DEFINE_SPATIAL_AUTOMATION_TEST macro to build on
   * top of Unreal's
   * IMPLEMENT_COMPLEX_AUTOMATION_TEST (see
   * https://docs.unrealengine.com/latest/INT/Programming/Automation/TechnicalGuide/index.html)
   */
  for (TFieldIterator<UFunction> FuncIt(GetClass(), EFieldIteratorFlags::ExcludeSuper); FuncIt;
       ++FuncIt)
  {
    UFunction* Function = *FuncIt;

    if (!IsTestFunction(Function))
    {
      continue;
    }

    switch (Function->NumParms)
    {
      case 0:
      {
        // Simple test: map "TestName" -> Delegate
        SpatialOSTestDelegate Delegate;
        Delegate.BindUFunction(this, FName(*Function->GetName()));

        AddTest(GetTestName(Function), Delegate);
      }
      break;

      case 1:
      {
        // Test with TestCases
        if (IsValidTestCaseFunction(Function))
        {
          // Split the array into parameters
          TArray<FString> Parameters;
          Function->GetMetaData(*TestCaseMetaDataKey)
              .ParseIntoArray(Parameters, TEXT(","), /* CullEmpty */ false);

          // Sanity check...
          if (Parameters.Num() == 0)
          {
            UE_LOG(LogTestFramework, Error, TEXT("%s has 0 TestCases"), *Function->GetName());
          }

          // Then, for each available parameter, create a new test variant and bind the parameter.
          for (const auto& Parameter : Parameters)
          {
            SpatialOSTestDelegate Delegate;
            Delegate.BindUFunction(this, Function->GetFName(), Parameter);

            AddTest(GetTestName(Function, Parameter), Delegate);
          }
        }
      }
      break;

      default:
        UE_LOG(LogTestFramework, Error, TEXT("%s is marked as a test, but has %i parameters"),
               *Function->GetName(), Function->NumParms);
        break;
    }
  }
}

/*
 * Called by Unreal's automation testing framework.
 * See DEFINE_SPATIAL_AUTOMATION_TEST.
 */
void USpatialOSTest::GetTests(TArray<FString>& OutBeautifiedNames,
                              TArray<FString>& OutTestCommands) const
{
  for (const auto& Elem : Tests)
  {
    OutBeautifiedNames.Add(Elem.Key);
    OutTestCommands.Add(Elem.Key);
  }
}

/*
 * Called by Unreal's automation testing framework.
 * See DEFINE_SPATIAL_AUTOMATION_TEST.
 */
void USpatialOSTest::RunTest(FAutomationTestBase* T, const FString& TestName)
{
  // Capture the tester here; helps to keep the test signatures as simple as possible.
  this->T = T;
  Tests[TestName].Execute();
}

void USpatialOSTest::AddTest(const FString& TestName, SpatialOSTestDelegate Test)
{
  Tests.Emplace(TestName, Test);
}
