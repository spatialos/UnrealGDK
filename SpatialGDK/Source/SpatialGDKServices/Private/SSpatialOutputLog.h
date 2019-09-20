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
	FArchiveLogFileReader(IFileHandle* InHandle, const TCHAR* InFilename, int64 InSize, uint32 InBufferSize /*= PLATFORM_FILE_READER_BUFFER_SIZE*/)
		: FArchiveFileReaderGeneric(InHandle, InFilename, InSize, InBufferSize)
	{}
	void UpdateFileSize();
};

class SSpatialOutputLog
	: public SOutputLog
{

public:
	SLATE_BEGIN_ARGS( SSpatialOutputLog )
	: _Messages()
	{}
		
	/** All messages captured before this log window has been created */
	SLATE_ARGUMENT( TArray< TSharedPtr<FLogMessage> >, Messages )

	SLATE_END_ARGS()

	~SSpatialOutputLog();

	void Construct( const FArguments& InArgs );

	//END_SLATE_FUNCTION_BUILD_OPTIMIZATION void ReadLatestLogFile();
	TUniquePtr<FArchiveLogFileReader> CreateLogFileReader(const TCHAR* InFilename, uint32 Flags, uint32 BufferSize);

protected:
	void OnCrash();

	void StartPollingLogFile(FString LogFilePath);
	void StartPollTimer(FString LogFilePath);
	void PollLogFile(FString LogFilePath);
	void CloseLogReader();

	void FormatRawLogLine(FString& LogLine);

	void StartUpLogDirectoryWatcher(FString LogDirectory);
	void ShutdownLogDirectoryWatcher(FString LogDirectory);
	void OnLogDirectoryChanged(const TArray<FFileChangeData>& FileChanges);

	FDelegateHandle LogDirectoryChangedDelegateHandle;
	IDirectoryWatcher::FDirectoryChanged LogDirectoryChangedDelegate;

	FTimerHandle PollTimer;
	TUniquePtr<FArchiveLogFileReader> LogReader;
};
