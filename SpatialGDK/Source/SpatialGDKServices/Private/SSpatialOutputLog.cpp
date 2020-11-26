// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SSpatialOutputLog.h"

#include "Async/Async.h"
#include "DirectoryWatcherModule.h"
#include "Editor.h"
#include "Internationalization/Regex.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"
#include "SlateOptMacros.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"

#define LOCTEXT_NAMESPACE "SSpatialOutputLog"

DEFINE_LOG_CATEGORY(LogSpatialOutputLog);

TTuple<bool, FString> ErrorLogFlagInfo;
const FString DefaultLogCategory = TEXT("Runtime");

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSpatialOutputLog::Construct(const FArguments& InArgs)
{
	SOutputLog::Construct(SOutputLog::FArguments());

	// Remove ourselves as the constructor of our parent (SOutputLog) added 'this' as a remote output device.
	GLog->RemoveOutputDevice(this);
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SSpatialOutputLog::OnClearLog()
{
	// SOutputLog will clear the messages and the SelectedLogCategories.
	SOutputLog::OnClearLog();

	// Clear the AvailableLogCategories and SelectedLogCategories as we generate many worker categories which are hard to parse.
	Filter.AvailableLogCategories.Reset();
	Filter.SelectedLogCategories.Reset();
}

void SSpatialOutputLog::FormatAndPrintRawErrorLine(const FString& LogLine)
{
	FString LogCategory = DefaultLogCategory;

	if (ErrorLogFlagInfo.Key)
	{
		LogCategory = ErrorLogFlagInfo.Value;
	}

	// Serialization must be done on the game thread.
	AsyncTask(ENamedThreads::GameThread, [this, LogLine, LogCategory] {
		Serialize(*LogLine, ELogVerbosity::Error, FName(*LogCategory));
	});
}

void SSpatialOutputLog::FormatAndPrintRawLogLine(const FString& LogLine)
{
	// Log line format [time] [category] [level] [message] or [time] [category] [level] [UnrealWorkerCF00FF...5B:UnrealWorker] [message]
	const FRegexPattern LogPattern = FRegexPattern(TEXT("\\[(\\w*)\\] \\[(\\w*)\\] (.*)"));
	FRegexMatcher LogMatcher(LogPattern, LogLine);

	if (!LogMatcher.FindNext())
	{
		// If this log line did not match the log line regex then it is an error line which is parsed differently.
		FormatAndPrintRawErrorLine(LogLine);
		return;
	}
	FString LogCategory = LogMatcher.GetCaptureGroup(1);
	FString LogLevelText = LogMatcher.GetCaptureGroup(2);
	FString LogMessage = LogMatcher.GetCaptureGroup(3);

	// Log message could have the format [UnrealWorkerCF00FF5D420435E4C4827D8AAC7FFA5B:UnrealWorker] [message]
	const FRegexPattern WorkerLogPattern = FRegexPattern(TEXT("\\[(.*)\\] (.*)"));
	FRegexMatcher WorkerLogMatcher(WorkerLogPattern, LogMessage);

	if (WorkerLogMatcher.FindNext())
	{
		FString LogMessageCategory = WorkerLogMatcher.GetCaptureGroup(1);
		LogMessage = WorkerLogMatcher.GetCaptureGroup(2);
		LogCategory = LogMessageCategory.Left(20);
	}
	else
	{
		// If the Log Category is not of type Worker, then it should be categorised as Runtime instead.
		LogCategory = DefaultLogCategory;
	}

	ELogVerbosity::Type LogVerbosity;
	ErrorLogFlagInfo.Key = false;

	if (LogLevelText.Contains(TEXT("error")))
	{
		LogVerbosity = ELogVerbosity::Error;
		ErrorLogFlagInfo.Key = true;
		ErrorLogFlagInfo.Value = LogCategory;
	}
	else if (LogLevelText.Contains(TEXT("warn")))
	{
		LogVerbosity = ELogVerbosity::Warning;
	}
	else if (LogLevelText.Contains(TEXT("debug")))
	{
		LogVerbosity = ELogVerbosity::Verbose;
	}
	else if (LogLevelText.Contains(TEXT("trace")))
	{
		LogVerbosity = ELogVerbosity::VeryVerbose;
	}
	else
	{
		LogVerbosity = ELogVerbosity::Log;
	}

	// Serialization must be done on the game thread.
	AsyncTask(ENamedThreads::GameThread, [this, LogMessage, LogVerbosity, LogCategory] {
		Serialize(*LogMessage, LogVerbosity, FName(*LogCategory));
	});
}

#undef LOCTEXT_NAMESPACE
