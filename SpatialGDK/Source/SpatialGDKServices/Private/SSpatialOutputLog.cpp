// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SSpatialOutputLog.h"
#include "Framework/Text/TextRange.h"
#include "Framework/Text/IRun.h"
#include "Framework/Text/TextLayout.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/OutputDeviceHelper.h"
#include "SlateOptMacros.h"
#include "Textures/SlateIcon.h"
#include "Framework/Commands/UIAction.h"
#include "Framework/Text/SlateTextLayout.h"
#include "Framework/Text/SlateTextRun.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SMenuAnchor.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Views/SListView.h"
#include "EditorStyleSet.h"
#include "Classes/EditorStyleSettings.h"
#include "Widgets/Input/SSearchBox.h"
#include "Features/IModularFeatures.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "DirectoryWatcherModule.h"
#include "Modules/ModuleManager.h"
#include "SpatialGDKServicesModule.h"
#include "Async/Async.h"
#include "Editor.h"

#define LOCTEXT_NAMESPACE "SSpatialOutputLog"

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
			UE_LOG(LogTemp, Fatal, TEXT("Failed to read file: %s"), InFilename);
		}

		return nullptr;
	}

	return TUniquePtr<FArchiveLogFileReader>(new FArchiveLogFileReader(Handle, InFilename, Handle->Size(), BufferSize));
}

void SSpatialOutputLog::StartPollingLogFile(FString LogFilePath)
{
	if (GEditor != nullptr)
	{
		// Delete the old timer if one exists.
		GEditor->GetTimerManager()->ClearTimer(PollTimer);
	}

	// Clean up the the previous file reader if it existed.
	if (LogReader)
	{
		LogReader->Close();
		LogReader = nullptr;
	}

	// FILEREAD_AllowWrite is required as we must match the permissions of the other processes writing to our log file in order to read from it.
	LogReader = CreateLogFileReader(*LogFilePath, FILEREAD_AllowWrite, PLATFORM_FILE_READER_BUFFER_SIZE);

	PollLogFile(LogFilePath);
}

void SSpatialOutputLog::PollLogFile(FString LogFilePath)
{
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

		// TODO: This is apparently inefficient
		int32 lineCount = ReadResult.ParseIntoArray(LogLines, TEXT("\n"), true);

		for (FString LogLine : LogLines)
		{
			FormatRawLogLine(LogLine);
		}

		FMemory::Free(Ch);
	}

	StartPollTimer(LogFilePath);
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

// DO NOT REVIEW THIS FUNCTION -- GOING TO CHANGE BASED ON SPATIALD IMPLEMENTATION
void SSpatialOutputLog::FormatRawLogLine(FString& LogLine)
{
	LogLine = LogLine.ReplaceEscapedCharWithChar();

	// TODO: Do this with some proper formatting or with regex
	ELogVerbosity::Type LogVerbosity = ELogVerbosity::Display;

	FString Left;
	FString Verbosity;

	LogLine.Split(TEXT("level"), &Left, &Verbosity);

	if (LogLine.Contains(TEXT("error")))
	{
		LogVerbosity = ELogVerbosity::Error;
	}
	else if (LogLine.Contains(TEXT("warn")))
	{
		LogVerbosity = ELogVerbosity::Warning;
	}
	else if (LogLine.Contains(TEXT("debug")))
	{
		LogVerbosity = ELogVerbosity::Verbose;
	}
	else if (LogLine.Contains(TEXT("verbose")))
	{
		LogVerbosity = ELogVerbosity::Verbose;
	}
	else
	{
		LogVerbosity = ELogVerbosity::Log;
	}

	FString Content;
	FString LogCategory;

	Verbosity.Split(TEXT("msg"), &Left, &Content);
	Content.Split(TEXT("]"), &LogCategory, &Content);
	LogCategory.Split(TEXT("."), nullptr, &LogCategory, ESearchCase::IgnoreCase, ESearchDir::FromEnd);

	// Filter out java connection exceptions for the time being.
	if (LogCategory.Contains(TEXT("connections")))
	{
		return;
	}

	Serialize(*Content, LogVerbosity, FName(*LogCategory));
}
// DO NOT REVIEW THIS FUNCTION -- GOING TO CHANGE BASED ON SPATIALD IMPLEMENTATION

void SSpatialOutputLog::StartUpRootLogDirWatcher()
{
	FString RootLogDir = FSpatialGDKServicesModule::GetSpatialOSDirectory(TEXT("logs/localdeployment"));
	AsyncTask(ENamedThreads::GameThread, [this, RootLogDir]
		{
			FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
			if (IDirectoryWatcher * DirectoryWatcher = DirectoryWatcherModule.Get())
			{
				// Watch the log directory for changes.
				if (FPaths::DirectoryExists(RootLogDir))
				{
					LogDirectoryChangedDelegate = IDirectoryWatcher::FDirectoryChanged::CreateRaw(this, &SSpatialOutputLog::OnRootLogDirectoryChanged);
					// TODO: Change this to use the IDirectoryWatcher::Flags to only watch for folder creations and thus reduce how much the delegate gets triggered.
					// We can use the name of the new folders and simply append launch.log to get the correct log files.
					// We can also use this as a point to stop the old polling.
					DirectoryWatcher->RegisterDirectoryChangedCallback_Handle(RootLogDir, LogDirectoryChangedDelegate, LogDirectoryChangedDelegateHandle, IDirectoryWatcher::WatchOptions::IncludeDirectoryChanges | IDirectoryWatcher::WatchOptions::IgnoreChangesInSubtree);
				}
				else
				{
					// TODO: Create it?
					UE_LOG(LogTemp, Error, TEXT("Log directory does not exist!"));
				}
			}
		});
}

void SSpatialOutputLog::ShutdownLogDirectoryWatcher(FString LogDirectory)
{
	AsyncTask(ENamedThreads::GameThread, [this, LogDirectory]
	{
		FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
		if (IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get())
		{
			// TODO: Logging and bool check.
			DirectoryWatcher->UnregisterDirectoryChangedCallback_Handle(LogDirectory, LogDirectoryChangedDelegateHandle);
		}
	});
}

void SSpatialOutputLog::OnRootLogDirectoryChanged(const TArray<FFileChangeData>& FileChanges)
{
	// If this is a new folder creation then switch to watching the log files in that new log folder.
	for (FFileChangeData FileChange : FileChanges)
	{
		if (FileChange.Action == FFileChangeData::FCA_Added)
		{
			// Now we can start reading the new log file in the new log folder. (Hopefully exists)
			UE_LOG(LogTemp, Display, TEXT("New folder added: %s"), *FileChange.Filename);
			StartPollingLogFile(FPaths::Combine(FileChange.Filename, TEXT("launch.log")));
		}
	}
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSpatialOutputLog::Construct(const FArguments& InArgs)
{
	SOutputLog::FArguments MyArgs;
	MyArgs._Messages = InArgs._Messages;
	SOutputLog::Construct(MyArgs);

	// Remove ourselves as the constructor for outputlog added ourselves
	GLog->RemoveOutputDevice(this);

	StartUpRootLogDirWatcher();
}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
