// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Containers/Set.h"
#include "Templates/SharedPointer.h"

#include "Interop/WorkingSetsCommon.h"
#include "SpatialCommonTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialWorkingSetsHandler, Log, All);

namespace SpatialGDK
{
class FSubView;
class ISpatialOSWorker;
class ViewCoordinator;

class FWorkingSetChangesHandler
{
public:
	DECLARE_DELEGATE_OneParam(FOnWorkingSetEvent, bool);

	FWorkingSetChangesHandler(Worker_PartitionId InWorkerStagingPartitionId, ISpatialOSWorker& InConnection,
							  FWorkingSetDataStorage& InDataStorage)
		: Connection(&InConnection)
		, DataStorage(&InDataStorage)
		, WorkerStagingPartitionId(InWorkerStagingPartitionId)
	{
	}
	void Advance();

	void ApplyLocalWorkingSetUpdateRequest(Worker_EntityId WorkingSetEntityId, const FWorkingSetMarkerRequest& Request,
										   FOnWorkingSetEvent Callback);
	void ApplyLocalWorkingSetCreationRequest(Worker_EntityId WorkingSetEntityId, const FWorkingSetMarkerRequest& Request,
											 FOnWorkingSetEvent Callback);

private:
	struct FLocalWorkingSetRequest
	{
		FWorkingSetMarkerRequest Request;
		FOnWorkingSetEvent Callback;
	};
	struct FLocalWorkingSetData
	{
		TArray<FLocalWorkingSetRequest> SetData;
		bool bWasSetCreationLocallyRequested = false;
	};
	TMap<Worker_EntityId_Key, FLocalWorkingSetData> LocalWorkingSetRequests;

	ISpatialOSWorker* Connection;
	FWorkingSetDataStorage* DataStorage;

	Worker_PartitionId WorkerStagingPartitionId;
};

class FWorkingSetCompletenessHandler
{
public:
	FWorkingSetCompletenessHandler(const FWorkingSetDataStorage& InDataStorage, const FSubView& InBaseAuthoritySubview)
		: DataStorage(&InDataStorage)
		, BaseAuthoritySubview(&InBaseAuthoritySubview)
		, AuthEntities(MakeShared<TSet<Worker_EntityId_Key>>())
	{
	}

	bool IsWorkingSetComplete(const FWorkingSetCommonData& WorkingSet) const;
	void Advance(ViewCoordinator& Coordinator);

	const FWorkingSetCommonData* GetOwningSet(const Worker_EntityId MaybeMemberEntityId) const;

	const FWorkingSetCommonData* GetMarkerEntitySet(const Worker_EntityId MarkerEntityId) const;

private:
	const FWorkingSetDataStorage* DataStorage;

	const FSubView* BaseAuthoritySubview;

	TMap<Worker_EntityId_Key, bool> WorkingSetCompleteness;
	TSharedRef<TSet<Worker_EntityId_Key>> AuthEntities;
};

} // namespace SpatialGDK
