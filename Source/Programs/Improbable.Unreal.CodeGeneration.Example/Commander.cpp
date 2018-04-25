// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 
#include "Commander.h"
#include "RequestId.h"
#include "TestType2.h"
#include "TestType1.h"
#include "improbable/standard_library.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "improbable/vector3.h"
#include "TestType1.h"
#include "TestType2.h"
#include "improbable/standard_library.h"
#include "SpatialOSConversionFunctionLibrary.h"
#include "improbable/vector3.h"
#include "SpatialOSCommandResult.h"
#include "SpatialOsComponent.h"
#include "SpatialOSViewTypes.h"
#include "SpatialOSWorkerTypes.h"

ECommandResponseCode UCommander::GetCommandResponseCode(const worker::StatusCode UnderlyingStatusCode)
{
	switch (UnderlyingStatusCode)
	{
		case worker::StatusCode::kSuccess:
			return ECommandResponseCode::Success;
		case worker::StatusCode::kTimeout:
			return ECommandResponseCode::Timeout;
		case worker::StatusCode::kNotFound:
			return ECommandResponseCode::NotFound;
		case worker::StatusCode::kAuthorityLost:
			return ECommandResponseCode::AuthorityLost;
		case worker::StatusCode::kPermissionDenied:
			return ECommandResponseCode::PermissionDenied;
		case worker::StatusCode::kApplicationError:
			return ECommandResponseCode::ApplicationError;
		default:
			return ECommandResponseCode::Unknown;
	}	
}

UCommander::UCommander()
: Component(nullptr)
, EntityQueryCountCommandResult(nullptr)
, EntityQuerySnapshotCommandResult(nullptr)
, RequestTestdata2Response(nullptr)
, RequestTestdata1Response(nullptr)
, TestCommandUserTypeResponse(nullptr)
{
}

UCommander* UCommander::Init(USpatialOsComponent* InComponent, const TWeakPtr<SpatialOSConnection>& InConnection, const TWeakPtr<SpatialOSView>& InView)
{
	Connection = InConnection;
	View = InView;
	Component = InComponent;
		
	RequestTestdata2Response = NewObject<UTestType2>(this, UTestType2::StaticClass());
	RequestTestdata1Response = NewObject<UTestType1>(this, UTestType1::StaticClass());
	TestCommandUserTypeResponse = NewObject<UTestType1>(this, UTestType1::StaticClass());

	auto LockedView = View.Pin();

	if(LockedView.IsValid())
	{
		Callbacks.Reset(new improbable::unreal::callbacks::FScopedViewCallbacks(View));

		Callbacks->Add(LockedView->OnReserveEntityIdResponse(std::bind(
			&UCommander::OnReserveEntityIdResponseDispatcherCallback, this, std::placeholders::_1)));

		Callbacks->Add(LockedView->OnReserveEntityIdsResponse(std::bind(
			&UCommander::OnReserveEntityIdsResponseDispatcherCallback, this, std::placeholders::_1)));

		Callbacks->Add(LockedView->OnCreateEntityResponse(std::bind(
			&UCommander::OnCreateEntityResponseDispatcherCallback, this, std::placeholders::_1)));

		Callbacks->Add(LockedView->OnDeleteEntityResponse(std::bind(
			&UCommander::OnDeleteEntityResponseDispatcherCallback, this, std::placeholders::_1)));

		Callbacks->Add(LockedView->OnEntityQueryResponse(std::bind(
			&UCommander::OnEntityQueryCommandResponseDispatcherCallback, this, std::placeholders::_1)));

				Callbacks->Add(LockedView->OnCommandResponse<test::TestData1::Commands::RequestTestdata2>(std::bind(
			&UCommander::OnRequestTestdata2CommandResponseDispatcherCallback, this, std::placeholders::_1)));

				Callbacks->Add(LockedView->OnCommandResponse<test::TestData2::Commands::RequestTestdata1>(std::bind(
			&UCommander::OnRequestTestdata1CommandResponseDispatcherCallback, this, std::placeholders::_1)));

				Callbacks->Add(LockedView->OnCommandResponse<test::TestCommandResponseTypes::Commands::TestCommandUserType>(std::bind(
			&UCommander::OnTestCommandUserTypeCommandResponseDispatcherCallback, this, std::placeholders::_1)));

				Callbacks->Add(LockedView->OnCommandResponse<test::TestCommandResponseTypes::Commands::TestCommandCoordinates>(std::bind(
			&UCommander::OnTestCommandCoordinatesCommandResponseDispatcherCallback, this, std::placeholders::_1)));

				Callbacks->Add(LockedView->OnCommandResponse<test::TestCommandResponseTypes::Commands::TestCommandVector3d>(std::bind(
			&UCommander::OnTestCommandVector3dCommandResponseDispatcherCallback, this, std::placeholders::_1)));

				Callbacks->Add(LockedView->OnCommandResponse<test::TestCommandResponseTypes::Commands::TestCommandVector3f>(std::bind(
			&UCommander::OnTestCommandVector3fCommandResponseDispatcherCallback, this, std::placeholders::_1)));

			
	}

	return this;
}

void UCommander::BeginDestroy()
{
	Super::BeginDestroy();

	Callbacks.Reset(nullptr);
}

FRequestId UCommander::ReserveEntityId(const FReserveEntityIdResultDelegate& callback, int timeoutMs)
{
	FRequestId WrappedRequestId;
	if (Component != nullptr && Component->GetAuthority() != EAuthority::Authoritative && Component->GetAuthority() != EAuthority::AuthorityLossImminent)
	{
		FSpatialOSCommandResult CommandResult;
		CommandResult.StatusCode = ECommandResponseCode::PermissionDenied;
		CommandResult.ErrorMessage = FString("Tried to reserve an entity id from a component you do not have authority on.");
		callback.ExecuteIfBound(CommandResult, 0);
	}
	else {
		auto LockedConnection = Connection.Pin();

		if(LockedConnection.IsValid())
		{
			auto requestId = LockedConnection->SendReserveEntityIdRequest(timeoutMs);
			RequestIdToReserveEntityIdCallback.Emplace(requestId.Id, callback);
			WrappedRequestId = FRequestId(requestId.Id, true);
		}
	}
	return WrappedRequestId;
}

void UCommander::OnReserveEntityIdResponseDispatcherCallback(const worker::ReserveEntityIdResponseOp& op)
{
	if (RequestIdToReserveEntityIdCallback.Find(op.RequestId.Id) == nullptr)
	{
		return;
	}
	auto callback = RequestIdToReserveEntityIdCallback[op.RequestId.Id];
    FSpatialOSCommandResult CommandResult;
	CommandResult.StatusCode = GetCommandResponseCode(op.StatusCode);
	CommandResult.ErrorMessage = FString(op.Message.c_str());
	CommandResult.RequestId = FRequestId(op.RequestId.Id, true);
	FEntityId EntityId = FEntityId((op.EntityId.empty() ? 0 : *(op.EntityId.data())));
	if (!callback.IsBound())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("OnReserveEntityIdResponse callback is no longer bound!"),
			*GetName())
	}
	callback.ExecuteIfBound(CommandResult, EntityId);
	RequestIdToReserveEntityIdCallback.Remove(op.RequestId.Id);
}

FRequestId UCommander::ReserveEntityIds(int NumEntitiesToReserve, const FReserveEntityIdsResultDelegate& callback, int timeoutMs)
{
	FRequestId WrappedRequestId;
	if (Component != nullptr && Component->GetAuthority() != EAuthority::Authoritative && Component->GetAuthority() != EAuthority::AuthorityLossImminent)
	{
		TArray<FEntityId> EmptyResult;
		FSpatialOSCommandResult CommandResult;
		CommandResult.StatusCode = ECommandResponseCode::PermissionDenied;
		CommandResult.ErrorMessage = FString("Tried to reserve entity ids from a component you do not have authority on.");
		callback.ExecuteIfBound(CommandResult, EmptyResult);
	}
	else 
	{
		auto LockedConnection = Connection.Pin();

		if (LockedConnection.IsValid())
		{
			auto requestId = LockedConnection->SendReserveEntityIdsRequest(NumEntitiesToReserve, timeoutMs);
			RequestIdToReserveEntityIdsCallback.Emplace(requestId.Id, callback);
			WrappedRequestId = FRequestId(requestId.Id, true);
		}
	}
	return WrappedRequestId;
}

void UCommander::OnReserveEntityIdsResponseDispatcherCallback(const worker::ReserveEntityIdsResponseOp& op)
{
	auto callback = RequestIdToReserveEntityIdsCallback.Find(op.RequestId.Id);
	if (callback == nullptr)
	{
		return;
	}

	FSpatialOSCommandResult CommandResult;
	CommandResult.StatusCode = GetCommandResponseCode(op.StatusCode);
	CommandResult.ErrorMessage = FString(op.Message.c_str());
	CommandResult.RequestId = FRequestId(op.RequestId.Id, true);
	if (!(*callback).IsBound())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("OnReserveEntityIdsResponse callback is no longer bound!"),
			*GetName())
	}
	else 
	{
		TArray<FEntityId> ReservedEntityIds;
		if (CommandResult.StatusCode == ECommandResponseCode::Success && !op.FirstEntityId.empty())
		{
			const auto FirstEntityId = *(op.FirstEntityId.data());
			for (auto i = 0; i < op.NumberOfEntityIds; ++i)
			{
				ReservedEntityIds.Emplace(FEntityId(FirstEntityId + i));
			}
		}

		(*callback).ExecuteIfBound(CommandResult, ReservedEntityIds);
	}

	RequestIdToReserveEntityIdsCallback.Remove(op.RequestId.Id);
}

FRequestId UCommander::CreateEntity(UEntityTemplate* entityTemplate, FEntityId entityId, const FCreateEntityResultDelegate& callback, int timeoutMs)
{
	FRequestId WrappedRequestId;
	if (Component != nullptr && Component->GetAuthority() != EAuthority::Authoritative && Component->GetAuthority() != EAuthority::AuthorityLossImminent)
	{
		FSpatialOSCommandResult CommandResult;
		CommandResult.StatusCode = ECommandResponseCode::PermissionDenied;
		CommandResult.ErrorMessage = FString("Tried to create an entity from a component you do not have authority on.");

		callback.ExecuteIfBound(CommandResult, 0);
	}
	else
	{
		auto LockedConnection = Connection.Pin();

		if(LockedConnection.IsValid())
		{
			auto requestId = LockedConnection->SendCreateEntityRequest(entityTemplate->GetUnderlying(), worker::Option<worker::EntityId>(entityId.ToSpatialEntityId()), timeoutMs);
			RequestIdToCreateEntityCallback.Emplace(requestId.Id, callback);
			WrappedRequestId = FRequestId(requestId.Id, true);
		}
	}
	return WrappedRequestId;
}

void UCommander::OnCreateEntityResponseDispatcherCallback(const worker::CreateEntityResponseOp& op)
{
	if (RequestIdToCreateEntityCallback.Find(op.RequestId.Id) == nullptr)
	{
		return;
	}
	auto callback = RequestIdToCreateEntityCallback[op.RequestId.Id];
    FSpatialOSCommandResult CommandResult;
	CommandResult.StatusCode = GetCommandResponseCode(op.StatusCode);
	CommandResult.ErrorMessage = FString(op.Message.c_str());
	CommandResult.RequestId = FRequestId(op.RequestId.Id, true);
	FEntityId EntityId = FEntityId((op.EntityId.empty() ? 0 : *(op.EntityId.data())));
	if (!callback.IsBound())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("OnCreateEntityResponse callback is no longer bound!"),
			*GetName())
	}
	callback.ExecuteIfBound(CommandResult, EntityId);
	RequestIdToCreateEntityCallback.Remove(op.RequestId.Id);
}

FRequestId UCommander::DeleteEntity(FEntityId entityId, const FDeleteEntityResultDelegate& callback, int timeoutMs)
{
	FRequestId WrappedRequestId;
	if (Component != nullptr && Component->GetAuthority() != EAuthority::Authoritative && Component->GetAuthority() != EAuthority::AuthorityLossImminent)
	{
		FSpatialOSCommandResult CommandResult;
		CommandResult.StatusCode = ECommandResponseCode::PermissionDenied;
		CommandResult.ErrorMessage = FString("Tried to delete an entity from a component you do not have authority on.");

		callback.ExecuteIfBound(CommandResult);
	}
	else 
	{
		auto LockedConnection = Connection.Pin();

		if(LockedConnection.IsValid())
		{
			auto requestId = LockedConnection->SendDeleteEntityRequest(entityId.ToSpatialEntityId(), timeoutMs);
			RequestIdToDeleteEntityCallback.Emplace(requestId.Id, callback);
			WrappedRequestId = FRequestId(requestId.Id, true);
		}
	}
	return WrappedRequestId;
}

void UCommander::OnDeleteEntityResponseDispatcherCallback(const worker::DeleteEntityResponseOp& op)
{
	if (RequestIdToDeleteEntityCallback.Find(op.RequestId.Id) == nullptr)
	{
		return;
	}
	auto callback = RequestIdToDeleteEntityCallback[op.RequestId.Id];
    FSpatialOSCommandResult CommandResult;
	CommandResult.StatusCode = GetCommandResponseCode(op.StatusCode);
	CommandResult.ErrorMessage = FString(op.Message.c_str());
	CommandResult.RequestId = FRequestId(op.RequestId.Id, true);
	if (!callback.IsBound())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("OnDeleteEntityResponse callback is no longer bound!"),
			*GetName())
	}
	callback.ExecuteIfBound(CommandResult);
	RequestIdToDeleteEntityCallback.Remove(op.RequestId.Id);
}

FRequestId UCommander::EntityQueryCountRequest(UEntityQueryConstraint* EntityQuery, const FEntityQueryCountResultDelegate& callback, int timeoutMs)
{
	FRequestId WrappedRequestId;
	auto lockedConnection = Connection.Pin();
	if (lockedConnection.IsValid())
	{
		worker::query::EntityQuery query{ *(EntityQuery->Underlying), worker::query::CountResultType{} };
		auto queryId = lockedConnection->SendEntityQueryRequest(query, timeoutMs);
		RequestIdToEntityQueryCountCommandCallback.Emplace(queryId.Id, callback);
		WrappedRequestId = FRequestId(queryId.Id, true);
	}
	return WrappedRequestId;
}

FRequestId UCommander::EntityQuerySnapshotRequest(UEntityQueryConstraint* EntityQuery, const TArray<FComponentId>& ComponentIds, const FEntityQuerySnapshotResultDelegate& callback, int timeoutMs)
{
	FRequestId WrappedRequestId;
	auto lockedConnection = Connection.Pin();
	if (lockedConnection.IsValid())
	{
		worker::query::SnapshotResultType snapshotType{};
		if (ComponentIds.Num() > 0)
		{
			worker::List<worker::ComponentId> componentIdList;
			for (const FComponentId& id : ComponentIds)
			{
				componentIdList.emplace_back(id.ToSpatialComponentId());
			}
			snapshotType.ComponentIds = componentIdList;
		}

		worker::query::EntityQuery query{ *(EntityQuery->Underlying), snapshotType };
		auto queryId = lockedConnection->SendEntityQueryRequest(query, timeoutMs);
		RequestIdToEntityQuerySnapshotCommandCallback.Emplace(queryId.Id, callback);
		WrappedRequestId = FRequestId(queryId.Id, true);
	}
	return WrappedRequestId;
}

void UCommander::OnEntityQueryCommandResponseDispatcherCallback(const worker::EntityQueryResponseOp& op)
{
	auto countCallback = RequestIdToEntityQueryCountCommandCallback.Find(op.RequestId.Id);
	auto snapshotCallback = RequestIdToEntityQuerySnapshotCommandCallback.Find(op.RequestId.Id);
	if (countCallback != nullptr)
	{
		if (EntityQueryCountCommandResult == nullptr)
		{
			EntityQueryCountCommandResult = NewObject<UEntityQueryCountCommandResult>(this, UEntityQueryCountCommandResult::StaticClass());
		}
		EntityQueryCountCommandResult->Init(op);

		if (!(*countCallback).IsBound())
		{
			UE_LOG(LogTemp, Warning,
				TEXT("OnEntityQueryCommandResponse callback is no longer bound!"),
				*GetName())
		}
		(*countCallback).ExecuteIfBound(EntityQueryCountCommandResult);
		RequestIdToEntityQueryCountCommandCallback.Remove(op.RequestId.Id);
	}
	else if (snapshotCallback != nullptr)
	{
		if (EntityQuerySnapshotCommandResult == nullptr)
		{
			EntityQuerySnapshotCommandResult = NewObject<UEntityQuerySnapshotCommandResult>(this, UEntityQuerySnapshotCommandResult::StaticClass());
		}
		EntityQuerySnapshotCommandResult->Init(op);

		if (!(*snapshotCallback).IsBound())
		{
			UE_LOG(LogTemp, Warning,
				TEXT("OnEntityQueryCommandResponse callback is no longer bound!"),
				*GetName())
		}
		(*snapshotCallback).ExecuteIfBound(EntityQuerySnapshotCommandResult);
		RequestIdToEntityQuerySnapshotCommandCallback.Remove(op.RequestId.Id);
	}
}

FRequestId UCommander::RequestTestdata2(FEntityId entityId, UTestType1* request, const FRequestTestdata2CommandResultDelegate& callback, int timeoutMs, ECommandDelivery commandDelivery)
{	
	FRequestId WrappedRequestId;
	if (Component != nullptr && Component->GetAuthority() != EAuthority::Authoritative && Component->GetAuthority() != EAuthority::AuthorityLossImminent)
	{
		FSpatialOSCommandResult CommandResult;
		CommandResult.StatusCode = ECommandResponseCode::PermissionDenied;
		CommandResult.ErrorMessage = FString("Error sending command: Tried to send a RequestTestdata2 command from a component which the worker does not have authority on.");
				
		callback.ExecuteIfBound(CommandResult, nullptr);
	}
	else
	{
		auto LockedConnection = Connection.Pin();

		if(LockedConnection.IsValid())
		{
			worker::CommandParameters parameters;
			parameters.AllowShortCircuit = (commandDelivery == ECommandDelivery::SHORT_CIRCUIT);
			auto underlyingRequest = request->GetUnderlying();
			auto requestId = LockedConnection->SendCommandRequest<test::TestData1::Commands::RequestTestdata2>(entityId.ToSpatialEntityId(), underlyingRequest, worker::Option<std::uint32_t>(timeoutMs), parameters);
			RequestIdToRequestTestdata2CommandCallback.Emplace(requestId.Id, callback);
			WrappedRequestId = FRequestId(requestId.Id, true);
		}
	}
	return WrappedRequestId;
}

void UCommander::OnRequestTestdata2CommandResponseDispatcherCallback(const worker::CommandResponseOp<test::TestData1::Commands::RequestTestdata2>& op)
{
	check(RequestTestdata2Response);
	auto callback = RequestIdToRequestTestdata2CommandCallback.Find(op.RequestId.Id);
	if (callback == nullptr)
	{
		return;
	}

	if (!callback->IsBound())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("OnRequestTestdata2CommandResponse callback is no longer bound!"),
			*GetName())

		RequestIdToRequestTestdata2CommandCallback.Remove(op.RequestId.Id);
		return;
	}

	FSpatialOSCommandResult CommandResult;
	CommandResult.StatusCode = GetCommandResponseCode(op.StatusCode);
	CommandResult.ErrorMessage = FString(op.Message.c_str());
	CommandResult.RequestId = FRequestId(op.RequestId.Id, true);

	if (CommandResult.StatusCode == ECommandResponseCode::Success && !op.Response.empty())
	{
		RequestTestdata2Response->Init(*(op.Response.data()));
	}

	callback->ExecuteIfBound(CommandResult, CommandResult.StatusCode == ECommandResponseCode::Success ? RequestTestdata2Response : nullptr);
	RequestIdToRequestTestdata2CommandCallback.Remove(op.RequestId.Id);
}

FRequestId UCommander::RequestTestdata1(FEntityId entityId, UTestType2* request, const FRequestTestdata1CommandResultDelegate& callback, int timeoutMs, ECommandDelivery commandDelivery)
{	
	FRequestId WrappedRequestId;
	if (Component != nullptr && Component->GetAuthority() != EAuthority::Authoritative && Component->GetAuthority() != EAuthority::AuthorityLossImminent)
	{
		FSpatialOSCommandResult CommandResult;
		CommandResult.StatusCode = ECommandResponseCode::PermissionDenied;
		CommandResult.ErrorMessage = FString("Error sending command: Tried to send a RequestTestdata1 command from a component which the worker does not have authority on.");
				
		callback.ExecuteIfBound(CommandResult, nullptr);
	}
	else
	{
		auto LockedConnection = Connection.Pin();

		if(LockedConnection.IsValid())
		{
			worker::CommandParameters parameters;
			parameters.AllowShortCircuit = (commandDelivery == ECommandDelivery::SHORT_CIRCUIT);
			auto underlyingRequest = request->GetUnderlying();
			auto requestId = LockedConnection->SendCommandRequest<test::TestData2::Commands::RequestTestdata1>(entityId.ToSpatialEntityId(), underlyingRequest, worker::Option<std::uint32_t>(timeoutMs), parameters);
			RequestIdToRequestTestdata1CommandCallback.Emplace(requestId.Id, callback);
			WrappedRequestId = FRequestId(requestId.Id, true);
		}
	}
	return WrappedRequestId;
}

void UCommander::OnRequestTestdata1CommandResponseDispatcherCallback(const worker::CommandResponseOp<test::TestData2::Commands::RequestTestdata1>& op)
{
	check(RequestTestdata1Response);
	auto callback = RequestIdToRequestTestdata1CommandCallback.Find(op.RequestId.Id);
	if (callback == nullptr)
	{
		return;
	}

	if (!callback->IsBound())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("OnRequestTestdata1CommandResponse callback is no longer bound!"),
			*GetName())

		RequestIdToRequestTestdata1CommandCallback.Remove(op.RequestId.Id);
		return;
	}

	FSpatialOSCommandResult CommandResult;
	CommandResult.StatusCode = GetCommandResponseCode(op.StatusCode);
	CommandResult.ErrorMessage = FString(op.Message.c_str());
	CommandResult.RequestId = FRequestId(op.RequestId.Id, true);

	if (CommandResult.StatusCode == ECommandResponseCode::Success && !op.Response.empty())
	{
		RequestTestdata1Response->Init(*(op.Response.data()));
	}

	callback->ExecuteIfBound(CommandResult, CommandResult.StatusCode == ECommandResponseCode::Success ? RequestTestdata1Response : nullptr);
	RequestIdToRequestTestdata1CommandCallback.Remove(op.RequestId.Id);
}

FRequestId UCommander::TestCommandUserType(FEntityId entityId, UTestType1* request, const FTestCommandUserTypeCommandResultDelegate& callback, int timeoutMs, ECommandDelivery commandDelivery)
{	
	FRequestId WrappedRequestId;
	if (Component != nullptr && Component->GetAuthority() != EAuthority::Authoritative && Component->GetAuthority() != EAuthority::AuthorityLossImminent)
	{
		FSpatialOSCommandResult CommandResult;
		CommandResult.StatusCode = ECommandResponseCode::PermissionDenied;
		CommandResult.ErrorMessage = FString("Error sending command: Tried to send a TestCommandUserType command from a component which the worker does not have authority on.");
				
		callback.ExecuteIfBound(CommandResult, nullptr);
	}
	else
	{
		auto LockedConnection = Connection.Pin();

		if(LockedConnection.IsValid())
		{
			worker::CommandParameters parameters;
			parameters.AllowShortCircuit = (commandDelivery == ECommandDelivery::SHORT_CIRCUIT);
			auto underlyingRequest = request->GetUnderlying();
			auto requestId = LockedConnection->SendCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandUserType>(entityId.ToSpatialEntityId(), underlyingRequest, worker::Option<std::uint32_t>(timeoutMs), parameters);
			RequestIdToTestCommandUserTypeCommandCallback.Emplace(requestId.Id, callback);
			WrappedRequestId = FRequestId(requestId.Id, true);
		}
	}
	return WrappedRequestId;
}

void UCommander::OnTestCommandUserTypeCommandResponseDispatcherCallback(const worker::CommandResponseOp<test::TestCommandResponseTypes::Commands::TestCommandUserType>& op)
{
	check(TestCommandUserTypeResponse);
	auto callback = RequestIdToTestCommandUserTypeCommandCallback.Find(op.RequestId.Id);
	if (callback == nullptr)
	{
		return;
	}

	if (!callback->IsBound())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("OnTestCommandUserTypeCommandResponse callback is no longer bound!"),
			*GetName())

		RequestIdToTestCommandUserTypeCommandCallback.Remove(op.RequestId.Id);
		return;
	}

	FSpatialOSCommandResult CommandResult;
	CommandResult.StatusCode = GetCommandResponseCode(op.StatusCode);
	CommandResult.ErrorMessage = FString(op.Message.c_str());
	CommandResult.RequestId = FRequestId(op.RequestId.Id, true);

	if (CommandResult.StatusCode == ECommandResponseCode::Success && !op.Response.empty())
	{
		TestCommandUserTypeResponse->Init(*(op.Response.data()));
	}

	callback->ExecuteIfBound(CommandResult, CommandResult.StatusCode == ECommandResponseCode::Success ? TestCommandUserTypeResponse : nullptr);
	RequestIdToTestCommandUserTypeCommandCallback.Remove(op.RequestId.Id);
}

FRequestId UCommander::TestCommandCoordinates(FEntityId entityId, FVector request, const FTestCommandCoordinatesCommandResultDelegate& callback, int timeoutMs, ECommandDelivery commandDelivery)
{	
	FRequestId WrappedRequestId;
	if (Component != nullptr && Component->GetAuthority() != EAuthority::Authoritative && Component->GetAuthority() != EAuthority::AuthorityLossImminent)
	{
		FSpatialOSCommandResult CommandResult;
		CommandResult.StatusCode = ECommandResponseCode::PermissionDenied;
		CommandResult.ErrorMessage = FString("Error sending command: Tried to send a TestCommandCoordinates command from a component which the worker does not have authority on.");
				
		callback.ExecuteIfBound(CommandResult, FVector());
	}
	else
	{
		auto LockedConnection = Connection.Pin();

		if(LockedConnection.IsValid())
		{
			worker::CommandParameters parameters;
			parameters.AllowShortCircuit = (commandDelivery == ECommandDelivery::SHORT_CIRCUIT);
			auto underlyingRequest = USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(request);
			auto requestId = LockedConnection->SendCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandCoordinates>(entityId.ToSpatialEntityId(), underlyingRequest, worker::Option<std::uint32_t>(timeoutMs), parameters);
			RequestIdToTestCommandCoordinatesCommandCallback.Emplace(requestId.Id, callback);
			WrappedRequestId = FRequestId(requestId.Id, true);
		}
	}
	return WrappedRequestId;
}

void UCommander::OnTestCommandCoordinatesCommandResponseDispatcherCallback(const worker::CommandResponseOp<test::TestCommandResponseTypes::Commands::TestCommandCoordinates>& op)
{
	auto callback = RequestIdToTestCommandCoordinatesCommandCallback.Find(op.RequestId.Id);
	if (callback == nullptr)
	{
		return;
	}

	if (!callback->IsBound())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("OnTestCommandCoordinatesCommandResponse callback is no longer bound!"),
			*GetName())

		RequestIdToTestCommandCoordinatesCommandCallback.Remove(op.RequestId.Id);
		return;
	}

	FSpatialOSCommandResult CommandResult;
	CommandResult.StatusCode = GetCommandResponseCode(op.StatusCode);
	CommandResult.ErrorMessage = FString(op.Message.c_str());
	CommandResult.RequestId = FRequestId(op.RequestId.Id, true);


	callback->ExecuteIfBound(CommandResult, CommandResult.StatusCode == ECommandResponseCode::Success ? USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(FVector(static_cast<float>((*(op.Response.data())).x()), static_cast<float>((*(op.Response.data())).y()), static_cast<float>((*(op.Response.data())).z()))) : FVector());
	RequestIdToTestCommandCoordinatesCommandCallback.Remove(op.RequestId.Id);
}

FRequestId UCommander::TestCommandVector3d(FEntityId entityId, FVector request, const FTestCommandVector3dCommandResultDelegate& callback, int timeoutMs, ECommandDelivery commandDelivery)
{	
	FRequestId WrappedRequestId;
	if (Component != nullptr && Component->GetAuthority() != EAuthority::Authoritative && Component->GetAuthority() != EAuthority::AuthorityLossImminent)
	{
		FSpatialOSCommandResult CommandResult;
		CommandResult.StatusCode = ECommandResponseCode::PermissionDenied;
		CommandResult.ErrorMessage = FString("Error sending command: Tried to send a TestCommandVector3d command from a component which the worker does not have authority on.");
				
		callback.ExecuteIfBound(CommandResult, FVector());
	}
	else
	{
		auto LockedConnection = Connection.Pin();

		if(LockedConnection.IsValid())
		{
			worker::CommandParameters parameters;
			parameters.AllowShortCircuit = (commandDelivery == ECommandDelivery::SHORT_CIRCUIT);
			auto underlyingRequest = improbable::Vector3d(static_cast<double>(request.X), static_cast<double>(request.Y), static_cast<double>(request.Z));
			auto requestId = LockedConnection->SendCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandVector3d>(entityId.ToSpatialEntityId(), underlyingRequest, worker::Option<std::uint32_t>(timeoutMs), parameters);
			RequestIdToTestCommandVector3dCommandCallback.Emplace(requestId.Id, callback);
			WrappedRequestId = FRequestId(requestId.Id, true);
		}
	}
	return WrappedRequestId;
}

void UCommander::OnTestCommandVector3dCommandResponseDispatcherCallback(const worker::CommandResponseOp<test::TestCommandResponseTypes::Commands::TestCommandVector3d>& op)
{
	auto callback = RequestIdToTestCommandVector3dCommandCallback.Find(op.RequestId.Id);
	if (callback == nullptr)
	{
		return;
	}

	if (!callback->IsBound())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("OnTestCommandVector3dCommandResponse callback is no longer bound!"),
			*GetName())

		RequestIdToTestCommandVector3dCommandCallback.Remove(op.RequestId.Id);
		return;
	}

	FSpatialOSCommandResult CommandResult;
	CommandResult.StatusCode = GetCommandResponseCode(op.StatusCode);
	CommandResult.ErrorMessage = FString(op.Message.c_str());
	CommandResult.RequestId = FRequestId(op.RequestId.Id, true);


	callback->ExecuteIfBound(CommandResult, CommandResult.StatusCode == ECommandResponseCode::Success ? FVector(static_cast<float>((*(op.Response.data())).x()), static_cast<float>((*(op.Response.data())).y()), static_cast<float>((*(op.Response.data())).z())) : FVector());
	RequestIdToTestCommandVector3dCommandCallback.Remove(op.RequestId.Id);
}

FRequestId UCommander::TestCommandVector3f(FEntityId entityId, FVector request, const FTestCommandVector3fCommandResultDelegate& callback, int timeoutMs, ECommandDelivery commandDelivery)
{	
	FRequestId WrappedRequestId;
	if (Component != nullptr && Component->GetAuthority() != EAuthority::Authoritative && Component->GetAuthority() != EAuthority::AuthorityLossImminent)
	{
		FSpatialOSCommandResult CommandResult;
		CommandResult.StatusCode = ECommandResponseCode::PermissionDenied;
		CommandResult.ErrorMessage = FString("Error sending command: Tried to send a TestCommandVector3f command from a component which the worker does not have authority on.");
				
		callback.ExecuteIfBound(CommandResult, FVector());
	}
	else
	{
		auto LockedConnection = Connection.Pin();

		if(LockedConnection.IsValid())
		{
			worker::CommandParameters parameters;
			parameters.AllowShortCircuit = (commandDelivery == ECommandDelivery::SHORT_CIRCUIT);
			auto underlyingRequest = improbable::Vector3f(static_cast<double>(request.X), static_cast<double>(request.Y), static_cast<double>(request.Z));
			auto requestId = LockedConnection->SendCommandRequest<test::TestCommandResponseTypes::Commands::TestCommandVector3f>(entityId.ToSpatialEntityId(), underlyingRequest, worker::Option<std::uint32_t>(timeoutMs), parameters);
			RequestIdToTestCommandVector3fCommandCallback.Emplace(requestId.Id, callback);
			WrappedRequestId = FRequestId(requestId.Id, true);
		}
	}
	return WrappedRequestId;
}

void UCommander::OnTestCommandVector3fCommandResponseDispatcherCallback(const worker::CommandResponseOp<test::TestCommandResponseTypes::Commands::TestCommandVector3f>& op)
{
	auto callback = RequestIdToTestCommandVector3fCommandCallback.Find(op.RequestId.Id);
	if (callback == nullptr)
	{
		return;
	}

	if (!callback->IsBound())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("OnTestCommandVector3fCommandResponse callback is no longer bound!"),
			*GetName())

		RequestIdToTestCommandVector3fCommandCallback.Remove(op.RequestId.Id);
		return;
	}

	FSpatialOSCommandResult CommandResult;
	CommandResult.StatusCode = GetCommandResponseCode(op.StatusCode);
	CommandResult.ErrorMessage = FString(op.Message.c_str());
	CommandResult.RequestId = FRequestId(op.RequestId.Id, true);


	callback->ExecuteIfBound(CommandResult, CommandResult.StatusCode == ECommandResponseCode::Success ? FVector(static_cast<float>((*(op.Response.data())).x()), static_cast<float>((*(op.Response.data())).y()), static_cast<float>((*(op.Response.data())).z())) : FVector());
	RequestIdToTestCommandVector3fCommandCallback.Remove(op.RequestId.Id);
}

