// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestCommandVector3dCommandResponder.h"

#include "SpatialOSWorkerTypes.h"

UTestCommandVector3dCommandResponder::UTestCommandVector3dCommandResponder()
{
}

UTestCommandVector3dCommandResponder* UTestCommandVector3dCommandResponder::Init(
    const TWeakPtr<SpatialOSConnection>& InConnection,
    worker::RequestId<
        worker::IncomingCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandVector3d>> 
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

FVector UTestCommandVector3dCommandResponder::GetRequest()
{
	return Request;
}

FString UTestCommandVector3dCommandResponder::GetCallerWorkerId()
{
	return CallerWorkerId;
}

void UTestCommandVector3dCommandResponder::SendResponse(FVector response)
{
	auto underlyingResponse = improbable::Vector3d(static_cast<double>(response.X), static_cast<double>(response.Y), static_cast<double>(response.Z));

	auto LockedConnection = Connection.Pin();

	if(LockedConnection.IsValid())
	{
	    LockedConnection->SendCommandResponse(
			RequestId, test::TestCommandResponseTypes::Commands::TestCommandVector3d::Response(underlyingResponse));
	}
}