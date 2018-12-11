// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Commandlets/Commandlet.h"

#include "SnapshotCommandlet.generated.h"

UCLASS()
class USnapshotCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	USnapshotCommandlet();

public:
	virtual int32 Main(const FString& Params) override;

private:
	void GenerateSnapshots();
	void GenerateSnapshotForMap(FString WorldPath);
	TArray<FString> GetAllMapPaths(FString InMapsPath);
};
