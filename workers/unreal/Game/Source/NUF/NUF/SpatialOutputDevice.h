// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Misc/OutputDevice.h"
#include "SpatialOS.h"

class NUF_API FSpatialOutputDevice : public FOutputDevice
{
public:
	FSpatialOutputDevice(USpatialOS* SpatialOSInstance, FString LoggerName);
	~FSpatialOutputDevice();

	void AddRedirectCategory(const FName& Category);
	void RemoveRedirectCategory(const FName& Category);
	void SetVerbosityFilterLevel(ELogVerbosity::Type Verbosity);
	void Serialize(const TCHAR* InData, ELogVerbosity::Type Verbosity, const FName& Category) override;

	static worker::LogLevel ConvertLogLevelToSpatial(ELogVerbosity::Type Verbosity);

protected:
	ELogVerbosity::Type FilterLevel;
	TSet<FName> CategoriesToRedirect;
	USpatialOS* SpatialOS;
	FString Name;
};
