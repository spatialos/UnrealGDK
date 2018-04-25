// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestCommandVector3fCommandResponder.h"

#include "SpatialOSWorkerTypes.h"

UTestCommandVector3fCommandResponder::UTestCommandVector3fCommandResponder()
{
}

UTestCommandVector3fCommandResponder* UTestCommandVector3fCommandResponder::Init(
    const TWeakPtr<SpatialOSConnection>& InConnection,
    worker::RequestId<
        worker::IncomingCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandVector3f>> 
	InRequestId,
    FVector InRequest, 
	const std::string& InCallerWorkerId
)
{
    Connection = InConnection;
    RequestId = InRequestId;
    Request = InRequest;
	CallerWorkerId = FString(InCallerWorkerId.c_str());
	return this;
}

FVector UTestCommandVector3fCommandResponder::GetRequest()
{
	return Request;
}

FString UTestCommandVector3fCommandResponder::GetCallerWorkerId()
{
	return CallerWorkerId;
}

void UTestCommandVector3fCommandResponder::SendResponse(FVector response)
{
	auto underlyingResponse = improbable::Vector3f(static_cast<double>(response.X), static_cast<double>(response.Y), static_cast<double>(response.Z));

	auto LockedConnection = Connection.Pin();

	if(LockedConnection.IsValid())
	{
	    LockedConnection->SendCommandResponse(
			RequestId, test::TestCommandResponseTypes::Commands::TestCommandVector3f::Response(underlyingResponse));
	}
}