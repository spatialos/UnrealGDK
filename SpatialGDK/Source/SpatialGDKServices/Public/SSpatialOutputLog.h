// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Developer/OutputLog/Private/SOutputLog.h"
#include "HAL/FileManagerGeneric.h"
#include "IDirectoryWatcher.h"
#include "SlateFwd.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialOutputLog, Log, All);

// Child class of the file reader used by Unreal but with the ability to update the known file size.
// This allows us to read a log file while it is being written to.
class FArchiveLogFileReader : public FArchiveFileReaderGeneric
{
public:
	FArchiveLogFileReader(IFileHandle* InHandle, const TCHAR* InFilename, int64 InSize,
						  uint32 InBufferSize /*= PLATFORM_FILE_READER_BUFFER_SIZE*/)
		: FArchiveFileReaderGeneric(InHandle, InFilename, InSize, InBufferSize)
	{
	}
	void UpdateFileSize();
};

class SSpatialOutputLog : public SOutputLog
{
public:
	SLATE_BEGIN_ARGS(SSpatialOutputLog) {}

	SLATE_END_ARGS()

	~SSpatialOutputLog();

	void Construct(const FArguments& InArgs);

	TUniquePtr<FArchiveLogFileReader> CreateLogFileReader(const TCHAR* InFilename, uint32 Flags, uint32 BufferSize);

private:
	void ReadLatestLogFile();
	void ResetPollingLogFile(const FString& LogFilePath);
	void StartPollTimer(const FString& LogFilePath);
	void PollLogFile(const FString& LogFilePath);
	void CloseLogReader();

	void FormatAndPrintRawLogLine(const FString& LogLine);
	void FormatAndPrintRawErrorLine(const FString& LogLine);

	void StartUpLogDirectoryWatcher(const FString& LogDirectory);
	void ShutdownLogDirectoryWatcher(const FString& LogDirectory);
	void OnLogDirectoryChanged(const TArray<FFileChangeData>& FileChanges);

	void OnClearLog() override;

	FDelegateHandle LogDirectoryChangedDelegateHandle;
	IDirectoryWatcher::FDirectoryChanged LogDirectoryChangedDelegate;

	FTimerHandle PollTimer;
	TUniquePtr<FArchiveLogFileReader> LogReader;
	FCriticalSection LogReaderMutex;
};
