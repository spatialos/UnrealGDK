// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestCommandCoordinatesCommandResponder.h"

#include "SpatialOSWorkerTypes.h"

UTestCommandCoordinatesCommandResponder::UTestCommandCoordinatesCommandResponder()
{
}

UTestCommandCoordinatesCommandResponder* UTestCommandCoordinatesCommandResponder::Init(
    const TWeakPtr<SpatialOSConnection>& InConnection,
    worker::RequestId<
        worker::IncomingCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandCoordinates>> 
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

FVector UTestCommandCoordinatesCommandResponder::GetRequest()
{
	return Request;
}

FString UTestCommandCoordinatesCommandResponder::GetCallerWorkerId()
{
	return CallerWorkerId;
}

void UTestCommandCoordinatesCommandResponder::SendResponse(FVector response)
{
	auto underlyingResponse = USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(response);

	auto LockedConnection = Connection.Pin();

	if(LockedConnection.IsValid())
	{
	    LockedConnection->SendCommandResponse(
			RequestId, test::TestCommandResponseTypes::Commands::TestCommandCoordinates::Response(underlyingResponse));
	}
}