#include "Misc/AutomationTest.h"
#include "Interop/SpatialReceiver.h"

namespace SpatialReceiverTestUtils {
	bool SpatialReceiverOnEntityQueryResponseTest(Worker_EntityQueryResponseOp& TestOp)
	{
		USpatialReceiver* Receiver = NewObject<USpatialReceiver>();
		Worker_RequestId RequestId = 0;
		EntityQueryDelegate TestQueryDelegate;

		bool IsDelegateCalled = false;

		TestQueryDelegate.BindLambda([&IsDelegateCalled](Worker_EntityQueryResponseOp& _)
		{
			IsDelegateCalled = true;
		});

		Receiver->AddEntityQueryDelegate(RequestId, TestQueryDelegate);

		Receiver->OnEntityQueryResponse(TestOp);

		return IsDelegateCalled;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpatialReceiverOnEntityQueryResponseTestStatusCodeSuccess, "SpatialGDK.USpatialReceiver.OnEntityQueryResponse.CallsDelegateIfStatusCodeIsSuccess", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSpatialReceiverOnEntityQueryResponseTestStatusCodeSuccess::RunTest(const FString& Parameters)
{
	Worker_EntityQueryResponseOp TestQueryResponseOp{};
	TestQueryResponseOp.status_code = Worker_StatusCode::WORKER_STATUS_CODE_SUCCESS;

	return SpatialReceiverTestUtils::SpatialReceiverOnEntityQueryResponseTest(TestQueryResponseOp);
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSpatialReceiverOnEntityQueryResponseTestStatusCodeFail, "SpatialGDK.USpatialReceiver.OnEntityQueryResponse.CallsDelegateIfStatusCodeIsFailure", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSpatialReceiverOnEntityQueryResponseTestStatusCodeFail::RunTest(const FString& Parameters)
{
	Worker_EntityQueryResponseOp TestQueryResponseOp{};
	TestQueryResponseOp.status_code = Worker_StatusCode::WORKER_STATUS_CODE_INTERNAL_ERROR;

	AddExpectedError(TEXT("EntityQuery failed"));
	return SpatialReceiverTestUtils::SpatialReceiverOnEntityQueryResponseTest(TestQueryResponseOp);
}

