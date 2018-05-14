// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "CommanderTest.h"

#include "Commander.h"
#include "Mock/SpatialGDKViewTypes.h"
#include "Mock/SpatialOSMockWorkerTypes.h"
#include "SpatialOSCommon.h"
#include "Utils/RequestCallbackWrapper.h"
#include "Utils/ReserveEntityIdCallbackWrapper.h"
#include "Utils/SpatialOSConnectCallbackWrapper.h"

DEFINE_SPATIAL_AUTOMATION_TEST(Commander)
DEFINE_LOG_CATEGORY(LogSdkTest);

void UCommanderTest::TestSendReserveEntityIdCommand() const
{
	TSharedPtr<MockConnection> Connection = MakeShareable(new MockConnection());
	TSharedPtr<MockView> View = MakeShareable(new MockView(improbable::unreal::Components{}));

	auto Commander = NewObject<UCommander>((UObject*)this, UCommander::StaticClass())
		->Init(nullptr, Connection, View);
	auto ReserveIdDelegate = NewObject<UReserveEntityIdCallbackWrapper>(
		(UObject*)this, UReserveEntityIdCallbackWrapper::StaticClass());

	worker::RequestId<worker::ReserveEntityIdRequest> MockRequestId =
		worker::RequestId<worker::ReserveEntityIdRequest>{ 10 };

	Connection->MockSendReserveEntityIdRequestReturn = MockRequestId;

	bool bReserveResponded = false;
	auto Callback = FReserveEntityIdResultDelegate();
	Callback.BindDynamic(ReserveIdDelegate,
		&UReserveEntityIdCallbackWrapper::ReserveEntityIdCallbackInternal);
	ReserveIdDelegate->ReserveEntityIdCallback = [this, &bReserveResponded](
		const FSpatialOSCommandResult& result, FEntityId reservedEntityId) {

		bReserveResponded = true;
		T->TestTrue(TEXT("reserve response message should match expectation"),
			result.GetErrorMessage() == "mock-response");
		T->TestTrue(TEXT("reserve response status should be success"),
			result.StatusCode == ECommandResponseCode::Success);
	};

	auto ActualRequestId = Commander->ReserveEntityId(Callback, 0);
	UE_LOG(LogSdkTest, Display, TEXT("actualRequestId: %u"), ActualRequestId.GetUnderlying());
	T->TestTrue(TEXT("request should go through sending as expected"),
		ActualRequestId == MockRequestId);

	View->Process(Connection->GetOpList(0));

	T->TestTrue(TEXT("reserve request should be responded"), bReserveResponded);
}

void UCommanderTest::TestSendRequestTestdata1() const
{
	TSharedPtr<MockConnection> Connection = MakeShareable(new MockConnection());
	TSharedPtr<MockView> View = MakeShareable(new MockView(improbable::unreal::Components{}));

	auto Commander = NewObject<UCommander>((UObject*)this, UCommander::StaticClass())
		->Init(nullptr, Connection, View);

	// setup mock response
	const int32_t MockIntProperty = 10;
	auto MockResponse =
		improbable::TestType1("testresponse", MockIntProperty, improbable::Coordinates(1, 1, 1));
	worker::Variant<improbable::TestType1, improbable::TestType2> CommandObject{
		static_cast<improbable::TestType1>(MockResponse) };
	improbable::detail::GenericCommandObject_TestData2 GenericResponse{
		improbable::TestData2::Commands::RequestTestdata1::CommandId, CommandObject };
	MockCommandResponse ResponseObj{ ComponentCommandObject{ GenericResponse } };
	Connection.Get()->MockCommandResponses.emplace(ResponseObj);

	bool bCommandResponded = false;
	auto RequestCallback =
		NewObject<URequestCallbackWrapper>((UObject*)this, URequestCallbackWrapper::StaticClass());
	RequestCallback->RequestTestdata1Callback = [this, &bCommandResponded, MockResponse](
		const FSpatialOSCommandResult& result, UTestType1* response) {
		bCommandResponded = true;

		T->TestTrue(TEXT("command response should have the same string"),
			response->GetUnderlying().string_property() == MockResponse.string_property());
		T->TestTrue(TEXT("command response should have the same int"),
			response->GetUnderlying().int32_property() == MockResponse.int32_property());
		T->TestTrue(TEXT("command response should have the same coordinates"),
			response->GetUnderlying().coordinates_property() ==
			MockResponse.coordinates_property());
	};
	auto Callback = FRequestTestdata1CommandResultDelegate();
	Callback.BindDynamic(RequestCallback,
		&URequestCallbackWrapper::RequestTestdata1CallbackInternal);
	auto Request = NewObject<UTestType2>((UObject*)this, UTestType2::StaticClass());

	const worker::EntityId MockEntityId = 1;
	auto RequestId = Commander->RequestTestdata1(FEntityId(MockEntityId), Request, Callback, 0);
	T->TestTrue(TEXT("command request id should be valid"), RequestId.IsValid());

	View->Process(Connection->GetOpList(0));

	T->TestTrue(TEXT("command RequestTestdata1 should be responded"), bCommandResponded);
}