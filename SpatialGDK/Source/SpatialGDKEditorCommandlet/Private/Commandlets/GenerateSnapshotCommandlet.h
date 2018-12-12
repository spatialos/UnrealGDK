// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Commandlets/Commandlet.h"

#include "GenerateSnapshotCommandlet.generated.h"

UCLASS()
class UGenerateSnapshotCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UGenerateSnapshotCommandlet();

public:
	virtual int32 Main(const FString& Params) override;

private:
	void GenerateSnapshots();
	void GenerateSnapshotForMap(FString WorldPath);
	TArray<FString> GetAllMapPaths(FString InMapsPath);
};
