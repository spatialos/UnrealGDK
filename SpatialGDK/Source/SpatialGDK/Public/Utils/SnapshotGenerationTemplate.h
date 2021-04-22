// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include <WorkerSDK/improbable/c_worker.h>
#include "SnapshotGenerationTemplate.generated.h"

UCLASS(abstract)
class SPATIALGDK_API USnapshotGenerationTemplate : public UObject
{
	GENERATED_BODY()

public:
	USnapshotGenerationTemplate() = default;
	~USnapshotGenerationTemplate() = default;

	/**
	 * Write to the snapshot generation output stream
	 * @param OutputStream the output stream for the snapshot being created.
	 * @param NextEntityId the next available entity ID in the snapshot, this reference should be incremented appropriately.
	 * @return bool the success of writing to the snapshot output stream, this is returned to the overall snapshot generation.
	 */
	virtual bool WriteToSnapshotOutput(Worker_SnapshotOutputStream* OutputStream, Worker_EntityId& NextEntityId)
		PURE_VIRTUAL(USnapshotGenerationTemplate::WriteToSnapshotOutput, return false;);
};
