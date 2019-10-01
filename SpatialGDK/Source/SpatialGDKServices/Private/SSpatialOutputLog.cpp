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

#define LOCTEXT_NAMESPACE "SSpatialOutputLog"

DEFINE_LOG_CATEGORY(LogSpatialOutputLog);

static const FString LocalDeploymentLogsDir(FSpatialGDKServicesModule::GetSpatialOSDirectory(TEXT("logs/localdeployment")));
static const FString LaunchLogFilename(TEXT("launch.log"));

void FArchiveLogFileReader::UpdateFileSize()
{
	Size = IFileManager::Get().GetStatData(*Filename).FileSize;
}

TUniquePtr<FArchiveLogFileReader> SSpatialOutputLog::CreateLogFileReader(const TCHAR* InFilename, uint32 Flags, uint32 BufferSize)
{
	IFileHandle* Handle = FPlatformFileManager::Get().GetPlatformFile().OpenRead(InFilename, !!(Flags & FILEREAD_AllowWrite));
	if (!Handle)
	{
		if (Flags & FILEREAD_NoFail)
		{
			UE_LOG(LogSpatialOutputLog, Error, TEXT("Failed to read file: %s"), InFilename);
		}

		return nullptr;
	}

	return TUniquePtr<FArchiveLogFileReader>(new FArchiveLogFileReader(Handle, InFilename, Handle->Size(), BufferSize));
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSpatialOutputLog::Construct(const FArguments& InArgs)
{
	SOutputLog::Construct(SOutputLog::FArguments());

	// Remove ourselves as the constructor of our parent (SOutputLog) added 'this' as a remote output device.
	GLog->RemoveOutputDevice(this);

	LogReader = nullptr;

	StartUpLogDirectoryWatcher(LocalDeploymentLogsDir);

	// Set the LogReader to the latest launch.log if we can.
	ReadLatestLogFile();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SSpatialOutputLog::ReadLatestLogFile()
{
	FString NewestLogDir;
	FDateTime NewestLogDirTime;

	// Go through all log directories in the spatial logs and find the most recently created (if one exists) and print the log file to the Spatial Output.
	bool GetNewestLogDir = IFileManager::Get().IterateDirectoryStat(*LocalDeploymentLogsDir, [&NewestLogDir, &NewestLogDirTime](const TCHAR* FileName, const FFileStatData& FileStats) {

		if (FileStats.bIsDirectory)
		{
			if (FileStats.CreationTime > NewestLogDirTime)
			{
				NewestLogDir = FString(FileName);
				NewestLogDirTime = FileStats.CreationTime;
			}
		}

		return true;
	});

	if (GetNewestLogDir)
	{
		StartPollingLogFile(FPaths::Combine(NewestLogDir, LaunchLogFilename));
	}
}

SSpatialOutputLog::~SSpatialOutputLog()
{
	CloseLogReader();

	ShutdownLogDirectoryWatcher(LocalDeploymentLogsDir);

	FCoreDelegates::OnHandleSystemError.RemoveAll(this);
}

void SSpatialOutputLog::OnCrash()
{
	CloseLogReader();

	ShutdownLogDirectoryWatcher(LocalDeploymentLogsDir);
}

void SSpatialOutputLog::StartUpLogDirectoryWatcher(FString LogDirectory)
{
	AsyncTask(ENamedThreads::GameThread, [this, LogDirectory]
	{
		FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
		if (IDirectoryWatcher * DirectoryWatcher = DirectoryWatcherModule.Get())
		{
			// Watch the log directory for changes.
			if (FPaths::DirectoryExists(LogDirectory))
			{
				LogDirectoryChangedDelegate = IDirectoryWatcher::FDirectoryChanged::CreateRaw(this, &SSpatialOutputLog::OnLogDirectoryChanged);
				DirectoryWatcher->RegisterDirectoryChangedCallback_Handle(LogDirectory, LogDirectoryChangedDelegate, LogDirectoryChangedDelegateHandle, IDirectoryWatcher::WatchOptions::IncludeDirectoryChanges | IDirectoryWatcher::WatchOptions::IgnoreChangesInSubtree);
			}
			else
			{
				UE_LOG(LogSpatialOutputLog, Error, TEXT("Local deployment log directory does not exist!"));
			}
		}
	});
}

void SSpatialOutputLog::OnLogDirectoryChanged(const TArray<FFileChangeData>& FileChanges)
{
	// If this is a new folder creation then switch to watching the log files in that new log folder.
	for (FFileChangeData FileChange : FileChanges)
	{
		if (FileChange.Action == FFileChangeData::FCA_Added)
		{
			// Now we can start reading the new log file in the new log folder.
			StartPollingLogFile(FPaths::Combine(FileChange.Filename, LaunchLogFilename));
		}
	}
}

void SSpatialOutputLog::ShutdownLogDirectoryWatcher(FString LogDirectory)
{
	AsyncTask(ENamedThreads::GameThread, [this, LogDirectory]
	{
		FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
		if (IDirectoryWatcher * DirectoryWatcher = DirectoryWatcherModule.Get())
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

	// Clean up the the previous file reader if it existed.
	if (LogReader.IsValid())
	{
		LogReader->Close();
		LogReader = nullptr;
	}
}

void SSpatialOutputLog::StartPollingLogFile(FString LogFilePath)
{
	CloseLogReader();

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

void SSpatialOutputLog::PollLogFile(FString LogFilePath)
{
	// Poll log files in a background thread since we are doing a lot of string operations.
	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, LogFilePath]
	{
		if (!LogReader.IsValid())
		{
			UE_LOG(LogSpatialOutputLog, Error, TEXT("Attempted to read from log file but LogReader is not valid."));
			return;
		}

		FScopedLoadingState ScopedLoadingState(*LogFilePath);

		// Find out the current size of the log file. This is a cheaper operation than opening a new file reader on every poll.
		LogReader->UpdateFileSize();

		int32 SizeDifference = LogReader->TotalSize() - LogReader->Tell();

		// New log lines have been added, serialize them.
		if (SizeDifference > 0)
		{
			uint8* Ch = (uint8*)FMemory::Malloc(SizeDifference);

			LogReader->Serialize(Ch, SizeDifference);

			FString ReadResult;
			FFileHelper::BufferToString(ReadResult, Ch, SizeDifference);

			TArray<FString> LogLines;

			// All log lines begin with 'time='. We use this as our log line delimiter.
			ReadResult.ParseIntoArray(LogLines, TEXT("time="), true);

			for (FString LogLine : LogLines)
			{
				FormatRawLogLine(LogLine);
			}

			FMemory::Free(Ch);
		}

		StartPollTimer(LogFilePath);
	});
}

void SSpatialOutputLog::StartPollTimer(FString LogFilePath)
{
	// Start a timer to read the log file every 0.1s
	// Timers must be started on the game thread.
	AsyncTask(ENamedThreads::GameThread, [this, LogFilePath]
	{
		// It's possible that GEditor won't exist when shutting down.
		if (GEditor != nullptr)
		{
			// Start checking for the service status.
			GEditor->GetTimerManager()->SetTimer(PollTimer, [this, LogFilePath]()
			{
				PollLogFile(LogFilePath);
			}, 0.05f, false);
		}
	});
}

void SSpatialOutputLog::FormatRawLogLine(FString& LogLine)
{
	// Log lines have the format time=LOG_TIME level=LOG_LEVEL logger=LOG_CATEGORY msg=LOG_MESSAGE
	FString LogTime;
	FString LogLevelAndRest;
	FString LogLevelText;
	FString LogCategoryAndRest;
	FString LogCategory;
	FString LogMessage;

	LogLine.Split(TEXT("level="), &LogTime, &LogLevelAndRest);
	LogLevelAndRest.Split(TEXT("logger="), &LogLevelText, &LogCategoryAndRest);
	LogCategoryAndRest.Split(TEXT("msg="), &LogCategory, &LogMessage);

	// Log categories take the form "improbable.deployment.InternalGameLauncher", we filter to the last category to make it more human readable.
	LogCategory.Split(TEXT("."), nullptr, &LogCategory, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

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
