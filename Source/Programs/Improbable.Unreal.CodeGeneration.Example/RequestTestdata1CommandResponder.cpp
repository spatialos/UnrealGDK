// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "RequestTestdata1CommandResponder.h"

#include "SpatialOSWorkerTypes.h"

URequestTestdata1CommandResponder::URequestTestdata1CommandResponder()
{
}

URequestTestdata1CommandResponder* URequestTestdata1CommandResponder::Init(
    const TWeakPtr<SpatialOSConnection>& InConnection,
    worker::RequestId<
        worker::IncomingCommandRequest<test::TestData2::Commands::RequestTestdata1>> 
	InRequestId,
    UTestType2* InRequest, 
	const std::string& InCallerWorkerId
)
{
    Connection = InConnection;
    RequestId = InRequestId;
    Request = InRequest;
	CallerWorkerId = FString(InCallerWorkerId.c_str());
	return this;
}

UTestType2* URequestTestdata1CommandResponder::GetRequest()
{
	return Request;
}

FString URequestTestdata1CommandResponder::GetCallerWorkerId()
{
	return CallerWorkerId;
}

void URequestTestdata1CommandResponder::SendResponse(UTestType1* response)
{
	auto underlyingResponse = response->GetUnderlying();

	auto LockedConnection = Connection.Pin();

	if(LockedConnection.IsValid())
	{
	    LockedConnection->SendCommandResponse(
			RequestId, test::TestData2::Commands::RequestTestdata1::Response(underlyingResponse));
	}
}