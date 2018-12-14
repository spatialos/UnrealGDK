// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialOutputDevice.h"

#include "Interop/Connection/SpatialWorkerConnection.h"

FSpatialOutputDevice::FSpatialOutputDevice(USpatialWorkerConnection* InConnection, FString LoggerName)
{
	Connection = InConnection;
	Name = LoggerName;
	FilterLevel = ELogVerbosity::Warning;

	const TCHAR* CommandLine = FCommandLine::Get();
	bLogToSpatial = !FParse::Param(CommandLine, TEXT("NoLogToSpatial"));

	FOutputDeviceRedirector::Get()->AddOutputDevice(this);
}

FSpatialOutputDevice::~FSpatialOutputDevice()
{
	FOutputDeviceRedirector::Get()->RemoveOutputDevice(this);
}

void FSpatialOutputDevice::Serialize(const TCHAR* InData, ELogVerbosity::Type Verbosity, const class FName& Category)
{
	if (Verbosity > FilterLevel)
	{
		return;
	}

	if (bLogToSpatial && Connection->IsConnected())
	{
		Connection->SendLogMessage(ConvertLogLevelToSpatial(Verbosity), TCHAR_TO_UTF8(*Name), TCHAR_TO_UTF8(InData));
	}
}

void FSpatialOutputDevice::AddRedirectCategory(const FName& Category)
{
	CategoriesToRedirect.Add(Category);
}

void FSpatialOutputDevice::RemoveRedirectCategory(const FName& Category)
{
	CategoriesToRedirect.Remove(Category);
}

void FSpatialOutputDevice::SetVerbosityFilterLevel(ELogVerbosity::Type Verbosity)
{
	FilterLevel = Verbosity;
}

Worker_LogLevel FSpatialOutputDevice::ConvertLogLevelToSpatial(ELogVerbosity::Type Verbosity)
{
	switch (Verbosity)
	{
	case ELogVerbosity::Fatal:
		return WORKER_LOG_LEVEL_FATAL;
	case ELogVerbosity::Error:
		return WORKER_LOG_LEVEL_ERROR;
	case ELogVerbosity::Warning:
		return WORKER_LOG_LEVEL_WARN;
	default:
		return WORKER_LOG_LEVEL_INFO;
	}
}
