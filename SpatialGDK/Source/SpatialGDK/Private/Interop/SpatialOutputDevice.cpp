// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/SpatialOutputDevice.h"
#include "Utils/SpatialStatics.h"

#include "Interop/Connection/SpatialWorkerConnection.h"

FSpatialOutputDevice::FSpatialOutputDevice(USpatialWorkerConnection* InConnection, FName InLoggerName, int32 InPIEIndex)
	: LocalFilterLevel(ELogVerbosity::Type(GetDefault<USpatialGDKSettings>()->LocalWorkerLogLevel.GetValue()))
	, CloudFilterLevel(ELogVerbosity::Type(GetDefault<USpatialGDKSettings>()->CloudWorkerLogLevel.GetValue()))
	, Connection(InConnection)
	, LoggerName(InLoggerName)
	, PIEIndex(InPIEIndex)
{
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
	// Log category LogSpatial ignores the verbosity check.

	if (bLogToSpatial && Connection != nullptr)
	{
#if WITH_EDITOR
		if ((Verbosity > LocalFilterLevel && Category != FName("LogSpatial")) || GPlayInEditorID != PIEIndex
			|| LocalFilterLevel == ELogVerbosity::NoLogging)
		{
			return;
		}
#else  // !WITH_EDITOR
		if ((Verbosity > CloudFilterLevel && Category != FName("LogSpatial")) || CloudFilterLevel == ELogVerbosity::NoLogging)
		{
			return;
		}
#endif // WITH_EDITOR
		Connection->SendLogMessage(ConvertLogLevelToSpatial(Verbosity), LoggerName, InData);
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

void FSpatialOutputDevice::SetVerbosityLocalFilterLevel(ELogVerbosity::Type Verbosity)
{
	LocalFilterLevel = Verbosity;
}

void FSpatialOutputDevice::SetVerbosityCloudFilterLevel(ELogVerbosity::Type Verbosity)
{
	CloudFilterLevel = Verbosity;
}

Worker_LogLevel FSpatialOutputDevice::ConvertLogLevelToSpatial(ELogVerbosity::Type Verbosity)
{
	switch (Verbosity)
	{
	case ELogVerbosity::Fatal:
		return WORKER_LOG_LEVEL_ERROR;
	case ELogVerbosity::Error:
		return WORKER_LOG_LEVEL_ERROR;
	case ELogVerbosity::Warning:
		return WORKER_LOG_LEVEL_WARN;
	case ELogVerbosity::Display:
		return WORKER_LOG_LEVEL_INFO;
	case ELogVerbosity::Log:
		return WORKER_LOG_LEVEL_INFO;
	default:
		return WORKER_LOG_LEVEL_DEBUG;
	}
}
