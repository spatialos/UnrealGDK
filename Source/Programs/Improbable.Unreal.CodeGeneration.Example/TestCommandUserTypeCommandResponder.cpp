// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestCommandUserTypeCommandResponder.h"

#include "SpatialOSWorkerTypes.h"

UTestCommandUserTypeCommandResponder::UTestCommandUserTypeCommandResponder()
{
}

UTestCommandUserTypeCommandResponder* UTestCommandUserTypeCommandResponder::Init(
    const TWeakPtr<SpatialOSConnection>& InConnection,
    worker::RequestId<
        worker::IncomingCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandUserType>> 
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

UTestType1* UTestCommandUserTypeCommandResponder::GetRequest()
{
	return Request;
}

FString UTestCommandUserTypeCommandResponder::GetCallerWorkerId()
{
	return CallerWorkerId;
}

void UTestCommandUserTypeCommandResponder::SendResponse(UTestType1* response)
{
	auto underlyingResponse = response->GetUnderlying();

	auto LockedConnection = Connection.Pin();

	if(LockedConnection.IsValid())
	{
	    LockedConnection->SendCommandResponse(
			RequestId, test::TestCommandResponseTypes::Commands::TestCommandUserType::Response(underlyingResponse));
	}
}