// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "RequestTestdata2CommandResponder.h"

#include "SpatialOSWorkerTypes.h"

URequestTestdata2CommandResponder::URequestTestdata2CommandResponder()
{
}

URequestTestdata2CommandResponder* URequestTestdata2CommandResponder::Init(
    const TWeakPtr<SpatialOSConnection>& InConnection,
    worker::RequestId<
        worker::IncomingCommandRequest<test::TestData1::Commands::RequestTestdata2>> 
	InRequestId,
    UTestType1* InRequest, 
	const std::string& InCallerWorkerId
)
{
    Connection = InConnection;
    RequestId = InRequestId;
    Request = InRequest;
	CallerWorkerId = FString(InCallerWorkerId.c_str());
	return this;
}

UTestType1* URequestTestdata2CommandResponder::GetRequest()
{
	return Request;
}

FString URequestTestdata2CommandResponder::GetCallerWorkerId()
{
	return CallerWorkerId;
}

void URequestTestdata2CommandResponder::SendResponse(UTestType2* response)
{
	auto underlyingResponse = response->GetUnderlying();

	auto LockedConnection = Connection.Pin();

	if(LockedConnection.IsValid())
	{
	    LockedConnection->SendCommandResponse(
			RequestId, test::TestData1::Commands::RequestTestdata2::Response(underlyingResponse));
	}
}