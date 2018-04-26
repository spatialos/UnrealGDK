
#include "EntityQueryCommandResult.h"
#include "Commander.h"
#include "RequestId.h"

UEntityQueryCommandResultBase::UEntityQueryCommandResultBase()
{
}

bool UEntityQueryCommandResultBase::Success() const
{
	return Underlying.StatusCode == worker::StatusCode::kSuccess;
}

FString UEntityQueryCommandResultBase::GetErrorMessage() const
{
	return FString(Underlying.Message.c_str());
}

ECommandResponseCode UEntityQueryCommandResultBase::GetErrorCode() const
{
	return UCommander::GetCommandResponseCode(Underlying.StatusCode);
}

FRequestId UEntityQueryCommandResultBase::GetRequestId() const
{
	return CachedRequestId;
}

/**
*
*/
UEntityQueryCountCommandResult::UEntityQueryCountCommandResult()
{
}

UEntityQueryCommandResultBase *
UEntityQueryCountCommandResult::Init(const worker::EntityQueryResponseOp &underlying)
{
	Underlying = underlying;
	CachedRequestId = FRequestId(Underlying.RequestId.Id, true);
	return this;
}

int UEntityQueryCountCommandResult::GetCount() const
{
	return Underlying.ResultCount;
}

/**
*
*/
UEntityQuerySnapshotCommandResult::UEntityQuerySnapshotCommandResult()
{
}

UEntityQueryCommandResultBase *
UEntityQuerySnapshotCommandResult::Init(const worker::EntityQueryResponseOp &underlying)
{
	Underlying = underlying;
	CachedRequestId = FRequestId(underlying.RequestId.Id, true);
	return this;
}

int UEntityQuerySnapshotCommandResult::GetSnapshotCount() const
{
	return Underlying.Result.size();
}

TArray<FEntityId> UEntityQuerySnapshotCommandResult::GetEntityIDs() const
{
	TArray<FEntityId> returnArray;
	for (auto it = Underlying.Result.begin(); it != Underlying.Result.end(); ++it)
	{
		FEntityId id(it->first);
		returnArray.Emplace(id);
	}

	return returnArray;
}
