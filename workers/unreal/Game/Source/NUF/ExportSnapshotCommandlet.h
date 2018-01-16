// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Commandlets/Commandlet.h"
#define IMPROBABLE_MATH_NO_PROTO 1
#include <improbable/worker.h>
#undef IMPROBABLE_MATH_NO_PROTO
#include "ExportSnapshotCommandlet.generated.h"

UCLASS()
class UExportSnapshotCommandlet : public UCommandlet
{
	GENERATED_BODY()
public:
	UExportSnapshotCommandlet();
	~UExportSnapshotCommandlet();

	virtual int32 Main(const FString& Params) override;

private:
	void GenerateSnapshot(const FString& savePath) const;
	worker::Entity CreatePackageMapEntity() const;
	worker::Entity CreateSpawnerEntity() const;
};
