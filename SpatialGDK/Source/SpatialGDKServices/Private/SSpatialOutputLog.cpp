// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SSpatialOutputLog.h"

#include "Async/Async.h"
#include "DirectoryWatcherModule.h"
#include "Editor.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Modules/ModuleManager.h"
#include "SlateOptMacros.h"
#include "SpatialGDKServicesModule.h"
#include "Internationalization/Regex.h"

#define LOCTEXT_NAMESPACE "SSpatialOutputLog"

DEFINE_LOG_CATEGORY(LogSpatialOutputLog);

static const FString LocalDeploymentLogsDir(FSpatialGDKServicesModule::GetSpatialOSDirectory(TEXT("logs/localdeployment")));
static const FString LaunchLogFilename(TEXT("launch.log"));
static const float PollTimeInterval(0.05f);

void FArchiveLogFileReader::UpdateFileSize()
{
	Size = IFileManager::Get().GetStatData(*Filename).FileSize;
}

TUniquePtr<FArchiveLogFileReader> SSpatialOutputLog::CreateLogFileReader(const TCHAR* InFilename, uint32 Flags, uint32 BufferSize)
{
	IFileHandle* Handle = FPlatformFileManager::Get().GetPlatformFile().OpenRead(InFilename, !!(Flags & FILEREAD_AllowWrite));
	if (Handle == nullptr)
	{
		if (!(Flags & FILEREAD_NoFail))
		{
			UE_LOG(LogSpatialOutputLog, Error, TEXT("Failed to read file: %s"), InFilename);
		}

		return nullptr;
	}

	return MakeUnique<FArchiveLogFileReader>(Handle, InFilename, Handle->Size(), BufferSize);
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSpatialOutputLog::Construct(const FArguments& InArgs)
{
	SOutputLog::Construct(SOutputLog::FArguments());

	// Remove ourselves as the constructor of our parent (SOutputLog) added 'this' as a remote output device.
	GLog->RemoveOutputDevice(this);

	LogReader.Reset();

	StartUpLogDirectoryWatcher(LocalDeploymentLogsDir);

	// Set the LogReader to the latest launch.log if we can.
	ReadLatestLogFile();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SSpatialOutputLog::ReadLatestLogFile()
{
	FString LatestLogDir;
	FDateTime LatestLogDirTime;

	// Go through all log directories in the spatial logs and find the most recently created (if one exists) and print the log file to the Spatial Output.
	bool bGetLatestLogDir = IFileManager::Get().IterateDirectoryStat(*LocalDeploymentLogsDir, [&LatestLogDir, &LatestLogDirTime](const TCHAR* FileName, const FFileStatData& FileStats)
	{
		if (FileStats.bIsDirectory)
		{
			if (FileStats.CreationTime > LatestLogDirTime)
			{
				LatestLogDir = FString(FileName);
				LatestLogDirTime = FileStats.CreationTime;
			}
		}

		return true;
	});

	if (bGetLatestLogDir)
	{
		ResetPollingLogFile(FPaths::Combine(LatestLogDir, LaunchLogFilename));
	}
}

SSpatialOutputLog::~SSpatialOutputLog()
{
	CloseLogReader();

	ShutdownLogDirectoryWatcher(LocalDeploymentLogsDir);
}

void SSpatialOutputLog::StartUpLogDirectoryWatcher(const FString& LogDirectory)
{
	// This function will be called from the Slate thread and thus we must switch to the Game thread to create the Directory Watcher.
	AsyncTask(ENamedThreads::GameThread, [this, LogDirectory]
	{
		FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
		if (IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get())
		{
			// Watch the log directory for changes.
			if (!FPaths::DirectoryExists(LogDirectory))
			{
				UE_LOG(LogSpatialOutputLog, Error, TEXT("Local deployment log directory does not exist!"));
				return;
			}

			LogDirectoryChangedDelegate = IDirectoryWatcher::FDirectoryChanged::CreateRaw(this, &SSpatialOutputLog::OnLogDirectoryChanged);
			DirectoryWatcher->RegisterDirectoryChangedCallback_Handle(LogDirectory, LogDirectoryChangedDelegate, LogDirectoryChangedDelegateHandle, IDirectoryWatcher::WatchOptions::IncludeDirectoryChanges | IDirectoryWatcher::WatchOptions::IgnoreChangesInSubtree);
		}
	});
}

void SSpatialOutputLog::OnLogDirectoryChanged(const TArray<FFileChangeData>& FileChanges)
{
	// If this is a new folder creation then switch to watching the log files in that new log folder.
	for (const FFileChangeData& FileChange : FileChanges)
	{
		if (FileChange.Action == FFileChangeData::FCA_Added)
		{
			// Now we can start reading the new log file in the new log folder.
			ResetPollingLogFile(FPaths::Combine(FileChange.Filename, LaunchLogFilename));
			return;
		}
	}
}

void SSpatialOutputLog::OnClearLog()
{
	// SOutputLog will clear the messages and the SelectedLogCategories.
	SOutputLog::OnClearLog();

	// Clear the AvailableLogCategories and SelectedLogCategories as we generate many worker categories which are hard to parse.
	Filter.AvailableLogCategories.Reset();
	Filter.SelectedLogCategories.Reset();
}

void SSpatialOutputLog::ShutdownLogDirectoryWatcher(const FString& LogDirectory)
{
	AsyncTask(ENamedThreads::GameThread, [LogDirectory, LogDirectoryChangedDelegateHandle = LogDirectoryChangedDelegateHandle]
	{
		FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
		if (IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get())
		{
			DirectoryWatcher->UnregisterDirectoryChangedCallback_Handle(LogDirectory, LogDirectoryChangedDelegateHandle);
		}
	});
}

void SSpatialOutputLog::CloseLogReader()
{
	if (GEditor != nullptr)
	{
		// Delete the old timer if one exists.
		GEditor->GetTimerManager()->ClearTimer(PollTimer);
	}

	FScopeLock CloseLock(&LogReaderMutex);

	// Clean up the the previous file reader if it existed.
	if (LogReader.IsValid())
	{
		LogReader->Close();
		LogReader = nullptr;
	}
}

void SSpatialOutputLog::ResetPollingLogFile(const FString& LogFilePath)
{
	CloseLogReader();

	FScopeLock CreateLock(&LogReaderMutex);

	// FILEREAD_AllowWrite is required as we must match the permissions of the other processes writing to our log file in order to read from it.
	LogReader = CreateLogFileReader(*LogFilePath, FILEREAD_AllowWrite, PLATFORM_FILE_READER_BUFFER_SIZE);

	if (LogReader.IsValid())
	{
		PollLogFile(LogFilePath);
	}
	else
	{
		UE_LOG(LogSpatialOutputLog, Error, TEXT("Could not set up log file reader for %s"), *LogFilePath);
	}
}

void SSpatialOutputLog::PollLogFile(const FString& LogFilePath)
{
	// Poll log files in a background thread since we are doing a lot of string operations.
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, LogFilePath]
	{
		FScopeLock PollLock(&LogReaderMutex);

		if (!LogReader.IsValid())
		{
			UE_LOG(LogSpatialOutputLog, Error, TEXT("Attempted to read from log file but LogReader is not valid."));
			return;
		}

		FScopedLoadingState ScopedLoadingState(*LogFilePath);

		// Find out the current size of the log file. This is a cheaper operation than opening a new file reader on every poll.
		LogReader->UpdateFileSize();

		const int32 SizeDifference = LogReader->TotalSize() - LogReader->Tell();

		// New log lines have been added, serialize them.
		if (SizeDifference > 0)
		{
			uint8* Ch = static_cast<uint8*>(FMemory::Malloc(SizeDifference));

			LogReader->Serialize(Ch, SizeDifference);

			FString ReadResult;
			FFileHelper::BufferToString(ReadResult, Ch, SizeDifference);

			TArray<FString> LogLines;

			// All log lines begin with 'time='. We use this as our log line delimiter.
			ReadResult.ParseIntoArray(LogLines, TEXT("time="), true);

			for (const FString& LogLine : LogLines)
			{
				FormatAndPrintRawLogLine(LogLine);
			}

			FMemory::Free(Ch);
		}

		StartPollTimer(LogFilePath);
	});
}

void SSpatialOutputLog::StartPollTimer(const FString& LogFilePath)
{
	// Start a timer to read the log file every PollTimeInterval seconds
	// Timers must be started on the game thread.
	AsyncTask(ENamedThreads::GameThread, [this, LogFilePath]
	{
		// It's possible that GEditor won't exist when shutting down.
		if (GEditor != nullptr)
		{
			GEditor->GetTimerManager()->SetTimer(PollTimer, [this, LogFilePath]()
			{
				PollLogFile(LogFilePath);
			}, PollTimeInterval, false);
		}
	});
}

void SSpatialOutputLog::FormatAndPrintRawLogLine(const FString& LogLine)
{
	// Log lines have the format time=LOG_TIME level=LOG_LEVEL logger=LOG_CATEGORY msg=LOG_MESSAGE
	const FRegexPattern LogPattern = FRegexPattern(TEXT("level=(.*) logger=.*\\.(.*) msg=(.*)"));
	FRegexMatcher LogMatcher(LogPattern, LogLine);

	if (!LogMatcher.FindNext())
	{
		UE_LOG(LogSpatialOutputLog, Error, TEXT("Failed to parse log line: %s"), *LogLine);
		return;
	}

	FString LogLevelText = LogMatcher.GetCaptureGroup(1);
	FString LogCategory = LogMatcher.GetCaptureGroup(2);
	FString LogMessage = LogMatcher.GetCaptureGroup(3);

	// For worker logs 'WorkerLogMessageHandler' we use the worker name as the category. The worker name can be found in the msg.
	// msg=[WORKER_NAME:LOGGER_NAME] ... e.g. msg=[UnrealWorkerF6DD3:Unreal]
	if (LogCategory == TEXT("WorkerLogMessageHandler"))
	{
		const FRegexPattern WorkerLogPattern = FRegexPattern(TEXT("\\[([^:]*):[^\\]]*\\] (.*)"));
		FRegexMatcher WorkerLogMatcher(WorkerLogPattern, LogMessage);

		if (WorkerLogMatcher.FindNext())
		{
			LogCategory = WorkerLogMatcher.GetCaptureGroup(1);
			LogMessage = WorkerLogMatcher.GetCaptureGroup(2);
		}
	}

	ELogVerbosity::Type LogVerbosity = ELogVerbosity::Display;

	if (LogLevelText.Contains(TEXT("error")))
	{
		LogVerbosity = ELogVerbosity::Error;
	}
	else if (LogLevelText.Contains(TEXT("warn")))
	{
		LogVerbosity = ELogVerbosity::Warning;
	}
	else if (LogLevelText.Contains(TEXT("debug")))
	{
		LogVerbosity = ELogVerbosity::Verbose;
	}
	else if (LogLevelText.Contains(TEXT("verbose")))
	{
		LogVerbosity = ELogVerbosity::Verbose;
	}
	else
	{
		LogVerbosity = ELogVerbosity::Log;
	}

	// Serialization must be done on the game thread.
	AsyncTask(ENamedThreads::GameThread, [this, LogMessage, LogVerbosity, LogCategory]
	{
		Serialize(*LogMessage, LogVerbosity, FName(*LogCategory));
	});
}

#undef LOCTEXT_NAMESPACE
