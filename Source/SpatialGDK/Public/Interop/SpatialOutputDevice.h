// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Misc/OutputDevice.h"

#include <improbable/c_worker.h>

class USpatialConnection;

class SPATIALGDK_API FSpatialOutputDevice : public FOutputDevice
{
public:
	FSpatialOutputDevice(USpatialConnection* InConnection, FString LoggerName);
	~FSpatialOutputDevice();

	void AddRedirectCategory(const FName& Category);
	void RemoveRedirectCategory(const FName& Category);
	void SetVerbosityFilterLevel(ELogVerbosity::Type Verbosity);
	void Serialize(const TCHAR* InData, ELogVerbosity::Type Verbosity, const FName& Category) override;

	static Worker_LogLevel ConvertLogLevelToSpatial(ELogVerbosity::Type Verbosity);

protected:
	ELogVerbosity::Type FilterLevel;
	TSet<FName> CategoriesToRedirect;
	USpatialConnection* Connection;
	FString Name;
};
