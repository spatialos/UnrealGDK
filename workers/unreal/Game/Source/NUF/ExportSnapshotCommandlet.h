// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Commandlets/Commandlet.h"
#include <improbable/worker.h>
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
	worker::Entity CreateLevelDataEntity() const;
	worker::Entity CreateSpawnerEntity() const;
};
