// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SlateFwd.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Input/Reply.h"
#include "Widgets/SWidget.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Framework/Text/BaseTextLayoutMarshaller.h"
#include "Misc/TextFilterExpressionEvaluator.h"
#include "HAL/IConsoleManager.h"

#include "IDirectoryWatcher.h"
#include "Developer/OutputLog/Private/SOutputLog.h"

/**
 * Widget which holds a list view of logs of the program output
 * as well as a combo box for entering in new commands
 */
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

	///** Destructor for output log, so we can unregister from notifications */
	//~SSpatialOutputLog();

	/**
	 * Construct this widget.  Called by the SNew() Slate macro.
	 *
	 * @param	InArgs	Declaration used by the SNew() macro to construct this widget
	 */
	void Construct( const FArguments& InArgs );

	void StartPollingLogFile(FString LogFilePath);
	void PollLogFile(FString LogFilePath);
	void StartPollTimer(FString LogFilePath);
	void FormatRawLogLine(FString& LogLine);

	void StartUpRootLogDirWatcher();
	void ShutdownLogDirectoryWatcher(FString LogDirectory);
	void OnRootLogDirectoryChanged(const TArray<FFileChangeData>& FileChanges);

	FDelegateHandle LogDirectoryChangedDelegateHandle;
	IDirectoryWatcher::FDirectoryChanged LogDirectoryChangedDelegate;

	FTimerHandle PollTimer;
	TUniquePtr<FArchive> LogReader;

	FString CurrentLogDir;
	FString CurrentLogFile;
};
