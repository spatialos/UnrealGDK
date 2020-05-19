// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Developer/OutputLog/Private/SOutputLog.h"
#include "HAL/FileManagerGeneric.h"
#include "IDirectoryWatcher.h"
#include "SlateFwd.h"
#include "Engine/EngineTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOutputLog, Log, All);

// Child class of the file reader used by Unreal but with the ability to update the known file size.
// This allows us to read a log file while it is being written to.
class FArchiveLogFileReader : public FArchiveFileReaderGeneric
{
public:
	FArchiveLogFileReader(IFileHandle* InHandle, const TCHAR* InFilename, int64 InSize, uint32 InBufferSize /*= PLATFORM_FILE_READER_BUFFER_SIZE*/)
		: FArchiveFileReaderGeneric(InHandle, InFilename, InSize, InBufferSize)
	{}
	void UpdateFileSize();
};

class FSpatialLogFileReader
{
public:
	explicit FSpatialLogFileReader(const FString& LogFilename, TFunction<void(const FString&)> ContentHandler);
	~FSpatialLogFileReader();

	void ResetLogDirectory(const FString& LogDirectory);

private:
	TUniquePtr<FArchiveLogFileReader> CreateLogFileReader(const TCHAR* InFilename, uint32 Flags, uint32 BufferSize) const;
	void CloseLogReader();
	void InternalResetDirectory(const FString& LogDirectory, uint32 TriesRemaining);

	void PollLogFile(const FString& LogFilePath);
	void StartPollTimer(const FString& LogFilePath);

	FString LogFilename;
	TFunction<void(const FString&)> ContentHandler;

	FTimerHandle PollTimer;
	FCriticalSection LogReaderMutex;
	TUniquePtr<FArchiveLogFileReader> LogReader;
};

class SSpatialOutputLog : public SOutputLog
{
public:
	SLATE_BEGIN_ARGS(SSpatialOutputLog) {}

	SLATE_END_ARGS()

	~SSpatialOutputLog();

	void Construct(const FArguments& InArgs);

private:
	void ResetReadersWithLatestLogDir();

	void StartUpLogDirectoryWatcher(const FString& LogDirectory);
	void ShutdownLogDirectoryWatcher(const FString& LogDirectory) const;
	void OnLogDirectoryChanged(const TArray<FFileChangeData>& FileChanges);

	void OnClearLog() override;
	void AppendOutput(const FString& LogMessage, ELogVerbosity::Type Verbosity, const FString& Category);

	void ParseLaunchLogContent(const FString& Content);
	void FormatAndPrintRawLaunchLogErrorLine(const FString& LogLine);
	void FormatAndPrintRawLaunchLogLine(const FString& LogLine);

	void ParseSingleNodeRuntimeLogContent(const FString& Content);
	void FormatAndPrintSingleNodeRuntimeLogLine(const FString& LogLine);

	FDelegateHandle LogDirectoryChangedDelegateHandle;
	IDirectoryWatcher::FDirectoryChanged LogDirectoryChangedDelegate;

	TArray<TUniquePtr<FSpatialLogFileReader>> LogFileReaders;
};
