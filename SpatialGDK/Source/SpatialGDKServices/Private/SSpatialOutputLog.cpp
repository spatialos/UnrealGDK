

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

#define LOCTEXT_NAMESPACE "SSpatialOutputLog"

/** Expression context to test the given messages against the current text filter */
class FLogFilter_TextFilterExpressionContext : public ITextFilterExpressionContext
{
public:
	explicit FLogFilter_TextFilterExpressionContext(const FSpatialLogMessage& InMessage) : Message(&InMessage) {}

	/** Test the given value against the strings extracted from the current item */
	virtual bool TestBasicStringExpression(const FTextFilterString& InValue, const ETextFilterTextComparisonMode InTextComparisonMode) const override { return TextFilterUtils::TestBasicStringExpression(*Message->Message, InValue, InTextComparisonMode); }

	/**
	* Perform a complex expression test for the current item
	* No complex expressions in this case - always returns false
	*/
	virtual bool TestComplexExpression(const FName& InKey, const FTextFilterString& InValue, const ETextFilterComparisonOperation InComparisonOperation, const ETextFilterTextComparisonMode InTextComparisonMode) const override { return false; }

private:
	/** Message that is being filtered */
	const FSpatialLogMessage* Message;
};


TSharedRef< FSpatialOutputLogTextLayoutMarshaller > FSpatialOutputLogTextLayoutMarshaller::Create(TArray< TSharedPtr<FSpatialLogMessage> > InMessages, FSpatialLogFilter* InFilter)
{
	return MakeShareable(new FSpatialOutputLogTextLayoutMarshaller(MoveTemp(InMessages), InFilter));
}

FSpatialOutputLogTextLayoutMarshaller::~FSpatialOutputLogTextLayoutMarshaller()
{
}

void FSpatialOutputLogTextLayoutMarshaller::SetText(const FString& SourceString, FTextLayout& TargetTextLayout)
{
	TextLayout = &TargetTextLayout;
	AppendMessagesToTextLayout(Messages);
}

void FSpatialOutputLogTextLayoutMarshaller::GetText(FString& TargetString, const FTextLayout& SourceTextLayout)
{
	SourceTextLayout.GetAsText(TargetString);
}

bool FSpatialOutputLogTextLayoutMarshaller::AppendMessage(const TCHAR* InText, const ELogVerbosity::Type InVerbosity, const FName& InCategory)
{
	TArray< TSharedPtr<FSpatialLogMessage> > NewMessages;
	if (SSpatialOutputLog::CreateLogMessages(InText, InVerbosity, InCategory, NewMessages))
	{
		const bool bWasEmpty = Messages.Num() == 0;
		Messages.Append(NewMessages);

		// Add new message categories to the filter's available log categories
		for (const auto& NewMessage : NewMessages)
		{
			Filter->AddAvailableLogCategory(NewMessage->Category);
		}

		if (TextLayout)
		{
			// If we were previously empty, then we'd have inserted a dummy empty line into the document
			// We need to remove this line now as it would cause the message indices to get out-of-sync with the line numbers, which would break auto-scrolling
			if (bWasEmpty)
			{
				TextLayout->ClearLines();
			}

			// If we've already been given a text layout, then append these new messages rather than force a refresh of the entire document
			AppendMessagesToTextLayout(NewMessages);
		}
		else
		{
			MarkMessagesCacheAsDirty();
			MakeDirty();
		}

		return true;
	}

	return false;
}

void FSpatialOutputLogTextLayoutMarshaller::AppendMessageToTextLayout(const TSharedPtr<FSpatialLogMessage>& InMessage)
{
	if (!Filter->IsMessageAllowed(InMessage))
	{
		return;
	}

	// Increment the cached count if we're not rebuilding the log
	if (!IsDirty())
	{
		CachedNumMessages++;
	}

	const FTextBlockStyle& MessageTextStyle = FEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>(InMessage->Style);

	TSharedRef<FString> LineText = InMessage->Message;

	TArray<TSharedRef<IRun>> Runs;
	Runs.Add(FSlateTextRun::Create(FRunInfo(), LineText, MessageTextStyle));

	TextLayout->AddLine(FSlateTextLayout::FNewLineData(MoveTemp(LineText), MoveTemp(Runs)));
}

void FSpatialOutputLogTextLayoutMarshaller::AppendMessagesToTextLayout(const TArray<TSharedPtr<FSpatialLogMessage>>& InMessages)
{
	TArray<FTextLayout::FNewLineData> LinesToAdd;
	LinesToAdd.Reserve(InMessages.Num());

	int32 NumAddedMessages = 0;

	for (const auto& CurrentMessage : InMessages)
	{
		if (!Filter->IsMessageAllowed(CurrentMessage))
		{
			continue;
		}

		++NumAddedMessages;

		const FTextBlockStyle& MessageTextStyle = FEditorStyle::Get().GetWidgetStyle<FTextBlockStyle>(CurrentMessage->Style);

		TSharedRef<FString> LineText = CurrentMessage->Message;

		TArray<TSharedRef<IRun>> Runs;
		Runs.Add(FSlateTextRun::Create(FRunInfo(), LineText, MessageTextStyle));

		LinesToAdd.Emplace(MoveTemp(LineText), MoveTemp(Runs));
	}

	// Increment the cached message count if the log is not being rebuilt
	if (!IsDirty())
	{
		CachedNumMessages += NumAddedMessages;
	}

	TextLayout->AddLines(LinesToAdd);
}

void FSpatialOutputLogTextLayoutMarshaller::ClearMessages()
{
	Messages.Empty();
	MakeDirty();
}

void FSpatialOutputLogTextLayoutMarshaller::CountMessages()
{
	// Do not re-count if not dirty
	if (!bNumMessagesCacheDirty)
	{
		return;
	}

	CachedNumMessages = 0;

	for (const auto& CurrentMessage : Messages)
	{
		if (Filter->IsMessageAllowed(CurrentMessage))
		{
			CachedNumMessages++;
		}
	}

	// Cache re-built, remove dirty flag
	bNumMessagesCacheDirty = false;
}

int32 FSpatialOutputLogTextLayoutMarshaller::GetNumMessages() const
{
	return Messages.Num();
}

int32 FSpatialOutputLogTextLayoutMarshaller::GetNumFilteredMessages()
{
	// No need to filter the messages if the filter is not set
	if (!Filter->IsFilterSet())
	{
		return GetNumMessages();
	}

	// Re-count messages if filter changed before we refresh
	if (bNumMessagesCacheDirty)
	{
		CountMessages();
	}

	return CachedNumMessages;
}

void FSpatialOutputLogTextLayoutMarshaller::MarkMessagesCacheAsDirty()
{
	bNumMessagesCacheDirty = true;
}

FSpatialOutputLogTextLayoutMarshaller::FSpatialOutputLogTextLayoutMarshaller(TArray<TSharedPtr<FSpatialLogMessage>> InMessages, FSpatialLogFilter* InFilter)
	: Messages(MoveTemp(InMessages))
	, CachedNumMessages(0)
	, Filter(InFilter)
	, TextLayout(nullptr)
{
}


BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SSpatialOutputLog::Construct( const FArguments& InArgs )
{
	// Build list of available log categories from historical logs
	for (const auto& Message : InArgs._Messages)
	{
		Filter.AddAvailableLogCategory(Message->Category);
	}

	MessagesTextMarshaller = FSpatialOutputLogTextLayoutMarshaller::Create(InArgs._Messages, &Filter);


	MessagesTextBox = SNew(SMultiLineEditableTextBox)
		.Style(FEditorStyle::Get(), "Log.TextBox")
		.TextStyle(FEditorStyle::Get(), "Log.Normal")
		.ForegroundColor(FLinearColor::Gray)
		.Marshaller(MessagesTextMarshaller)
		.IsReadOnly(true)
		.AlwaysShowScrollbars(true)
		.OnVScrollBarUserScrolled(this, &SSpatialOutputLog::OnUserScrolled)
		.ContextMenuExtender(this, &SSpatialOutputLog::ExtendTextBoxMenu);

	ChildSlot
	[
		SNew(SBorder)
		.Padding(3)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SNew(SVerticalBox)

			// Output Log Filter
			+SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.0f, 0.0f, 0.0f, 4.0f))
			[
				SNew(SHorizontalBox)
			
				+SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SComboButton)
					.ComboButtonStyle(FEditorStyle::Get(), "GenericFilters.ComboButtonStyle")
					.ForegroundColor(FLinearColor::White)
					.ContentPadding(0)
					.ToolTipText(LOCTEXT("AddFilterToolTip", "Add an output log filter."))
					.OnGetMenuContent(this, &SSpatialOutputLog::MakeAddFilterMenu)
					.HasDownArrow(true)
					.ContentPadding(FMargin(1, 0))
					.ButtonContent()
					[
						SNew(SHorizontalBox)

						+SHorizontalBox::Slot()
						.AutoWidth()
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "GenericFilters.TextStyle")
							.Font(FEditorStyle::Get().GetFontStyle("FontAwesome.9"))
							.Text(FText::FromString(FString(TEXT("\xf0b0"))) /*fa-filter*/)
						]

						+SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(2, 0, 0, 0)
						[
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "GenericFilters.TextStyle")
							.Text(LOCTEXT("Filters", "Filters"))
						]
					]
				]

				+SHorizontalBox::Slot()
				.Padding(4, 1, 0, 0)
				[
					SAssignNew(FilterTextBox, SSearchBox)
					.HintText(LOCTEXT("SearchLogHint", "Search Log"))
					.OnTextChanged(this, &SSpatialOutputLog::OnFilterTextChanged)
					.OnTextCommitted(this, &SSpatialOutputLog::OnFilterTextCommitted)
					.DelayChangeNotificationsWhileTyping(true)
				]
			]

			// Output log area
			+SVerticalBox::Slot()
			.FillHeight(1)
			[
				MessagesTextBox.ToSharedRef()
			]
		]
	];

	// Remove itself on crash (crashmalloc has limited memory and echoing logs here at that point is useless).
	FCoreDelegates::OnHandleSystemError.AddRaw(this, &SSpatialOutputLog::OnCrash);

	bIsUserScrolled = false;
	RequestForceScroll();

	WatchLogFile();

}
END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SSpatialOutputLog::WatchLogFile()
{
	ReadLogFile();

	StartUpLogDirectoryWatcher();
}

int32 SizeDifference;
int32 OldSize;
int32 NewSize;

void SSpatialOutputLog::ReadLogFile()
{
	// Read log file live.
	FString Result;
	//FString SpatialDLogPath = TEXT("F:/UnrealEngine422/Samples/UnrealGDKExampleProject/spatial/logs/testing/locallaunch_log.log");
	//FString SpatialDLogPath = TEXT("C:/Users/joshuahuburn/Projects/spatial-farm/spatial/logs/2019-08-02_14-14-51/runtime.log");
	FString SpatialDLogPath = TEXT("C:/Users/joshuahuburn/AppData/Local/.improbable/spatiald/log/spatiald.log");

	FScopedLoadingState ScopedLoadingState(*SpatialDLogPath);

	TUniquePtr<FArchive> LogReader(IFileManager::Get().CreateFileReader(*SpatialDLogPath, FILEREAD_AllowWrite));

	if (!LogReader)
	{
		UE_LOG(LogTemp, Error, TEXT("Log file does not exist!"));
		return;
	}

	int32 Size = LogReader->TotalSize();
	OldSize = Size;

	if (!Size)
	{
		Result.Empty();
		UE_LOG(LogTemp, Error, TEXT("Log empty!"));
		return;
	}

	uint8* Ch = (uint8*)FMemory::Malloc(Size);
	LogReader->Serialize(Ch, Size);

	LogReader->Close();
	LogReader = nullptr;

	FFileHelper::BufferToString(Result, Ch, Size);

	TArray<FString> LogLines;

	//// TODO: This is apparently inefficient
	//int32 lineCount = Result.ParseIntoArray(LogLines, TEXT("\n"), true);

	for (FString LogLine : LogLines)
	{
		// TODO: We need a better way of getting the log categories and shit here.
		ELogVerbosity::Type LogVerbosity = ELogVerbosity::Display;

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

		Serialize(*LogLine, LogVerbosity, FName(TEXT("Spatial")));
	}

	FMemory::Free(Ch);
}

void SSpatialOutputLog::TailLogFile()
{
	// Read log file live.
	FString Result;
	//FString SpatialDLogPath = TEXT("F:/UnrealEngine422/Samples/UnrealGDKExampleProject/spatial/logs/testing/locallaunch_log.log");
	//FString SpatialDLogPath = TEXT("C:/Users/joshuahuburn/Projects/spatial-farm/spatial/logs/2019-08-02_14-14-51/runtime.log");
	FString SpatialDLogPath = TEXT("C:/Users/joshuahuburn/AppData/Local/.improbable/spatiald/log/spatiald.log");

	FScopedLoadingState ScopedLoadingState(*SpatialDLogPath);

	TUniquePtr<FArchive> LogReader(IFileManager::Get().CreateFileReader(*SpatialDLogPath, FILEREAD_AllowWrite));

	if (!LogReader)
	{
		UE_LOG(LogTemp, Error, TEXT("Log file does not exist!"));
	}

	int32 Size = LogReader->TotalSize();

	if (!Size)
	{
		Result.Empty();
		UE_LOG(LogTemp, Error, TEXT("Log empty!"));
	}

	NewSize = Size;
	SizeDifference = NewSize - OldSize;

	if (SizeDifference <= 0)
	{
		LogReader->Close();
		LogReader = nullptr;
		OldSize = NewSize;
		return;
	}

	uint8* Ch = (uint8*)FMemory::Malloc(SizeDifference);

	LogReader->Seek(OldSize);
	LogReader->Serialize(Ch, SizeDifference);

	FFileHelper::BufferToString(Result, Ch, SizeDifference);

	TArray<FString> LogLines;

	//// TODO: This is apparently inefficient
	//int32 lineCount = Result.ParseIntoArray(LogLines, TEXT("\n"), true);

	for (FString LogLine : LogLines)
	{
		// TODO: We need a better way of getting the log categories and shit here.
		ELogVerbosity::Type LogVerbosity = ELogVerbosity::Display;

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

		Serialize(*LogLine, LogVerbosity, FName(TEXT("Spatial")));
	}

	FMemory::Free(Ch);
	OldSize = NewSize;
}

void SSpatialOutputLog::StartUpLogDirectoryWatcher()
{
	FDirectoryWatcherModule& DirectoryWatcherModule = FModuleManager::LoadModuleChecked<FDirectoryWatcherModule>(TEXT("DirectoryWatcher"));
	if (IDirectoryWatcher* DirectoryWatcher = DirectoryWatcherModule.Get())
	{
		// Watch the log directory for changes.
		//FString LogDirectory = TEXT("F:/UnrealEngine422/Samples/UnrealGDKExampleProject/spatial/logs/testing");
		//FString LogDirectory = TEXT("C:/Users/joshuahuburn/Projects/spatial-farm/spatial/logs/2019-08-02_14-14-51");
		FString LogDirectory = TEXT("C:/Users/joshuahuburn/AppData/Local/.improbable/spatiald/log");

		if (FPaths::DirectoryExists(LogDirectory))
		{
			LogDirectoryChangedDelegate = IDirectoryWatcher::FDirectoryChanged::CreateRaw(this, &SSpatialOutputLog::OnLogDirectoryChanged);
			DirectoryWatcher->RegisterDirectoryChangedCallback_Handle(LogDirectory, LogDirectoryChangedDelegate, LogDirectoryChangedDelegateHandle);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Log directory does not exist!"));
		}
	}
}

void SSpatialOutputLog::ShutdownLogDirectoryWatcher()
{

}

void SSpatialOutputLog::ChangeLogFolder(FString LogFolderPath)
{
	ShutdownLogDirectoryWatcher();
	FString LogDir = GetNewLogFolder();
	StartUpLogDirectoryWatcher();
}

// Call this when a new local deployment has been started.
FString SSpatialOutputLog::GetNewLogFolder()
{
	// Get the spatial logs local deployment folder
	const FString RootLogDir = FSpatialGDKServicesModule::GetSpatialOSDirectory(TEXT("logs/localdeployment"));

	// List all folders
	IFileManager& FileManager = IFileManager::Get();

	FString FinalPath = RootLogDir / TEXT("*");
	TArray<FString> Folders;
	FileManager.FindFiles(Folders, *FinalPath, false, true);

	FString NewestLogDir;
	FDateTime NewestLogDirModTime;

	// Get the one that was modified most recently
	for (FString Folder : Folders)
	{
		FFileStatData StatData = FileManager.GetStatData(*Folder);
		if (StatData.ModificationTime < NewestLogDirModTime)
		{
			NewestLogDir = Folder;
		}
	}

	return NewestLogDir;
}

void SSpatialOutputLog::OnLogDirectoryChanged(const TArray<FFileChangeData>& FileChanges)
{
	UE_LOG(LogTemp, Display, TEXT("Log files updated."));
	TailLogFile();
}

SSpatialOutputLog::~SSpatialOutputLog()
{
	FCoreDelegates::OnHandleSystemError.RemoveAll(this);
}

void SSpatialOutputLog::OnCrash()
{
}

bool SSpatialOutputLog::CreateLogMessages( const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category, TArray< TSharedPtr<FSpatialLogMessage> >& OutMessages )
{
	if (Verbosity == ELogVerbosity::SetColor)
	{
		// Skip Color Events
		return false;
	}
	else
	{
		// Get the style for this message. When piping output from child processes (eg. when cooking through the editor), we want to highlight messages
		// according to their original verbosity, so also check for "Error:" and "Warning:" substrings. This is consistent with how the build system processes logs.
		FName Style;
		if (Category == NAME_Cmd)
		{
			Style = FName(TEXT("Log.Command"));
		}
		else if (Verbosity == ELogVerbosity::Error || FCString::Stristr(V, TEXT("Error:")) != nullptr)
		{
			Style = FName(TEXT("Log.Error"));
		}
		else if (Verbosity == ELogVerbosity::Warning || FCString::Stristr(V, TEXT("Warning:")) != nullptr)
		{
			Style = FName(TEXT("Log.Warning"));
		}
		else
		{
			Style = FName(TEXT("Log.Normal"));
		}

		// Determine how to format timestamps
		static ELogTimes::Type LogTimestampMode = ELogTimes::None;
		if (UObjectInitialized() && !GExitPurge)
		{
			// Logging can happen very late during shutdown, even after the UObject system has been torn down, hence the init check above
			LogTimestampMode = GetDefault<UEditorStyleSettings>()->LogTimestampMode;
		}

		const int32 OldNumMessages = OutMessages.Num();

		// handle multiline strings by breaking them apart by line
		TArray<FTextRange> LineRanges;
		FString CurrentLogDump = V;
		FTextRange::CalculateLineRangesFromString(CurrentLogDump, LineRanges);

		bool bIsFirstLineInMessage = true;
		for (const FTextRange& LineRange : LineRanges)
		{
			if (!LineRange.IsEmpty())
			{
				FString Line = CurrentLogDump.Mid(LineRange.BeginIndex, LineRange.Len());
				Line = Line.ConvertTabsToSpaces(4);

				// Hard-wrap lines to avoid them being too long
				static const int32 HardWrapLen = 360;
				for (int32 CurrentStartIndex = 0; CurrentStartIndex < Line.Len();)
				{
					int32 HardWrapLineLen = 0;
					if (bIsFirstLineInMessage)
					{
						FString MessagePrefix = FOutputDeviceHelper::FormatLogLine(Verbosity, Category, nullptr, LogTimestampMode);
						
						HardWrapLineLen = FMath::Min(HardWrapLen - MessagePrefix.Len(), Line.Len() - CurrentStartIndex);
						FString HardWrapLine = Line.Mid(CurrentStartIndex, HardWrapLineLen);

						OutMessages.Add(MakeShared<FSpatialLogMessage>(MakeShared<FString>(MessagePrefix + HardWrapLine), Verbosity, Category, Style));
					}
					else
					{
						HardWrapLineLen = FMath::Min(HardWrapLen, Line.Len() - CurrentStartIndex);
						FString HardWrapLine = Line.Mid(CurrentStartIndex, HardWrapLineLen);

						OutMessages.Add(MakeShared<FSpatialLogMessage>(MakeShared<FString>(MoveTemp(HardWrapLine)), Verbosity, Category, Style));
					}

					bIsFirstLineInMessage = false;
					CurrentStartIndex += HardWrapLineLen;
				}
			}
		}

		return OldNumMessages != OutMessages.Num();
	}
}

void SSpatialOutputLog::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category)
{
	if ( MessagesTextMarshaller->AppendMessage(V, Verbosity, Category) )
	{
		// Don't scroll to the bottom automatically when the user is scrolling the view or has scrolled it away from the bottom.
		if( !bIsUserScrolled )
		{
			RequestForceScroll();
		}
	}
}

void SSpatialOutputLog::ExtendTextBoxMenu(FMenuBuilder& Builder)
{
	FUIAction ClearOutputLogAction(
		FExecuteAction::CreateRaw( this, &SSpatialOutputLog::OnClearLog ),
		FCanExecuteAction::CreateSP( this, &SSpatialOutputLog::CanClearLog )
		);

	Builder.AddMenuEntry(
		NSLOCTEXT("OutputLog", "ClearLogLabel", "Clear Log"), 
		NSLOCTEXT("OutputLog", "ClearLogTooltip", "Clears all log messages"), 
		FSlateIcon(), 
		ClearOutputLogAction
		);
}

void SSpatialOutputLog::OnClearLog()
{
	// Make sure the cursor is back at the start of the log before we clear it
	MessagesTextBox->GoTo(FTextLocation(0));

	MessagesTextMarshaller->ClearMessages();
	MessagesTextBox->Refresh();
	bIsUserScrolled = false;
}

void SSpatialOutputLog::OnUserScrolled(float ScrollOffset)
{
	bIsUserScrolled = ScrollOffset < 1.0 && !FMath::IsNearlyEqual(ScrollOffset, 1.0f);
}

bool SSpatialOutputLog::CanClearLog() const
{
	return MessagesTextMarshaller->GetNumMessages() > 0;
}

void SSpatialOutputLog::OnConsoleCommandExecuted()
{
	RequestForceScroll();
}

void SSpatialOutputLog::RequestForceScroll()
{
	if (MessagesTextMarshaller->GetNumFilteredMessages() > 0)
	{
		MessagesTextBox->ScrollTo(FTextLocation(MessagesTextMarshaller->GetNumFilteredMessages() - 1));
		bIsUserScrolled = false;
	}
}

void SSpatialOutputLog::Refresh()
{
	// Re-count messages if filter changed before we refresh
	MessagesTextMarshaller->CountMessages();

	MessagesTextBox->GoTo(FTextLocation(0));
	MessagesTextMarshaller->MakeDirty();
	MessagesTextBox->Refresh();
	RequestForceScroll();
}

void SSpatialOutputLog::OnFilterTextChanged(const FText& InFilterText)
{
	if (Filter.GetFilterText().ToString().Equals(InFilterText.ToString(), ESearchCase::CaseSensitive))
	{
		// nothing to do
		return;
	}

	// Flag the messages count as dirty
	MessagesTextMarshaller->MarkMessagesCacheAsDirty();

	// Set filter phrases
	Filter.SetFilterText(InFilterText);

	// Report possible syntax errors back to the user
	FilterTextBox->SetError(Filter.GetSyntaxErrors());

	// Repopulate the list to show only what has not been filtered out.
	Refresh();

	// Apply the new search text
	MessagesTextBox->BeginSearch(InFilterText);
}

void SSpatialOutputLog::OnFilterTextCommitted(const FText& InFilterText, ETextCommit::Type InCommitType)
{
	OnFilterTextChanged(InFilterText);
}

TSharedRef<SWidget> SSpatialOutputLog::MakeAddFilterMenu()
{
	FMenuBuilder MenuBuilder(/*bInShouldCloseWindowAfterMenuSelection=*/true, nullptr);
	
	MenuBuilder.BeginSection("OutputLogVerbosityEntries", LOCTEXT("OutputLogVerbosityHeading", "Verbosity"));
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ShowMessages", "Messages"), 
			LOCTEXT("ShowMessages_Tooltip", "Filter the Output Log to show messages"), 
			FSlateIcon(), 
			FUIAction(FExecuteAction::CreateSP(this, &SSpatialOutputLog::VerbosityLogs_Execute), 
				FCanExecuteAction::CreateLambda([] { return true; }), 
				FIsActionChecked::CreateSP(this, &SSpatialOutputLog::VerbosityLogs_IsChecked)), 
			NAME_None, 
			EUserInterfaceActionType::ToggleButton
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("ShowWarnings", "Warnings"), 
			LOCTEXT("ShowWarnings_Tooltip", "Filter the Output Log to show warnings"), 
			FSlateIcon(), 
			FUIAction(FExecuteAction::CreateSP(this, &SSpatialOutputLog::VerbosityWarnings_Execute), 
				FCanExecuteAction::CreateLambda([] { return true; }), 
				FIsActionChecked::CreateSP(this, &SSpatialOutputLog::VerbosityWarnings_IsChecked)), 
			NAME_None, 
			EUserInterfaceActionType::ToggleButton
		);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("ShowErrors", "Errors"), 
			LOCTEXT("ShowErrors_Tooltip", "Filter the Output Log to show errors"), 
			FSlateIcon(), 
			FUIAction(FExecuteAction::CreateSP(this, &SSpatialOutputLog::VerbosityErrors_Execute), 
				FCanExecuteAction::CreateLambda([] { return true; }), 
				FIsActionChecked::CreateSP(this, &SSpatialOutputLog::VerbosityErrors_IsChecked)), 
			NAME_None, 
			EUserInterfaceActionType::ToggleButton
		);
	}
	MenuBuilder.EndSection();

	MenuBuilder.BeginSection("OutputLogMiscEntries", LOCTEXT("OutputLogMiscHeading", "Miscellaneous"));
	{
		MenuBuilder.AddSubMenu(
			LOCTEXT("Categories", "Categories"), 
			LOCTEXT("SelectCategoriesToolTip", "Select Categories to display."), 
			FNewMenuDelegate::CreateSP(this, &SSpatialOutputLog::MakeSelectCategoriesSubMenu)
		);
	}

	return MenuBuilder.MakeWidget();
}

void SSpatialOutputLog::MakeSelectCategoriesSubMenu(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("OutputLogCategoriesEntries");
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("ShowAllCategories", "Show All"),
			LOCTEXT("ShowAllCategories_Tooltip", "Filter the Output Log to show all categories"),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateSP(this, &SSpatialOutputLog::CategoriesShowAll_Execute),
			FCanExecuteAction::CreateLambda([] { return true; }),
			FIsActionChecked::CreateSP(this, &SSpatialOutputLog::CategoriesShowAll_IsChecked)),
			NAME_None,
			EUserInterfaceActionType::ToggleButton
		);
		
		for (const FName Category : Filter.GetAvailableLogCategories())
		{
			MenuBuilder.AddMenuEntry(
				FText::AsCultureInvariant(Category.ToString()),
				FText::Format(LOCTEXT("Category_Tooltip", "Filter the Output Log to show category: {0}"), FText::AsCultureInvariant(Category.ToString())),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateSP(this, &SSpatialOutputLog::CategoriesSingle_Execute, Category),
				FCanExecuteAction::CreateLambda([] { return true; }),
				FIsActionChecked::CreateSP(this, &SSpatialOutputLog::CategoriesSingle_IsChecked, Category)),
				NAME_None,
				EUserInterfaceActionType::ToggleButton
			);
		}
	}
	MenuBuilder.EndSection();
}

bool SSpatialOutputLog::VerbosityLogs_IsChecked() const
{
	return Filter.bShowLogs;
}

bool SSpatialOutputLog::VerbosityWarnings_IsChecked() const
{
	return Filter.bShowWarnings;
}

bool SSpatialOutputLog::VerbosityErrors_IsChecked() const
{
	return Filter.bShowErrors;
}

void SSpatialOutputLog::VerbosityLogs_Execute()
{ 
	Filter.bShowLogs = !Filter.bShowLogs;

	// Flag the messages count as dirty
	MessagesTextMarshaller->MarkMessagesCacheAsDirty();

	Refresh();
}

void SSpatialOutputLog::VerbosityWarnings_Execute()
{
	Filter.bShowWarnings = !Filter.bShowWarnings;

	// Flag the messages count as dirty
	MessagesTextMarshaller->MarkMessagesCacheAsDirty();

	Refresh();
}

void SSpatialOutputLog::VerbosityErrors_Execute()
{
	Filter.bShowErrors = !Filter.bShowErrors;

	// Flag the messages count as dirty
	MessagesTextMarshaller->MarkMessagesCacheAsDirty();

	Refresh();
}

bool SSpatialOutputLog::CategoriesShowAll_IsChecked() const
{
	return Filter.bShowAllCategories;
}

bool SSpatialOutputLog::CategoriesSingle_IsChecked(FName InName) const
{
	return Filter.IsLogCategoryEnabled(InName);
}

void SSpatialOutputLog::CategoriesShowAll_Execute()
{
	Filter.bShowAllCategories = !Filter.bShowAllCategories;

	Filter.ClearSelectedLogCategories();
	if (Filter.bShowAllCategories)
	{
		for (const auto& AvailableCategory : Filter.GetAvailableLogCategories())
		{
			Filter.ToggleLogCategory(AvailableCategory);
		}
	}

	// Flag the messages count as dirty
	MessagesTextMarshaller->MarkMessagesCacheAsDirty();

	Refresh();
}

void SSpatialOutputLog::CategoriesSingle_Execute(FName InName)
{
	Filter.ToggleLogCategory(InName);

	// Flag the messages count as dirty
	MessagesTextMarshaller->MarkMessagesCacheAsDirty();

	Refresh();
}

bool FSpatialLogFilter::IsMessageAllowed(const TSharedPtr<FSpatialLogMessage>& Message)
{
	// Filter Verbosity
	{
		if (Message->Verbosity == ELogVerbosity::Error && !bShowErrors)
		{
			return false;
		}

		if (Message->Verbosity == ELogVerbosity::Warning && !bShowWarnings)
		{
			return false;
		}

		if (Message->Verbosity != ELogVerbosity::Error && Message->Verbosity != ELogVerbosity::Warning && !bShowLogs)
		{
			return false;
		}
	}

	// Filter by Category
	{
		if (!IsLogCategoryEnabled(Message->Category))
		{
			return false;
		}
	}

	// Filter search phrase
	{
		if (!TextFilterExpressionEvaluator.TestTextFilter(FLogFilter_TextFilterExpressionContext(*Message)))
		{
			return false;
		}
	}

	return true;
}

void FSpatialLogFilter::AddAvailableLogCategory(FName& LogCategory)
{
	// Use an insert-sort to keep AvailableLogCategories alphabetically sorted
	int32 InsertIndex = 0;
	for (InsertIndex = AvailableLogCategories.Num() - 1; InsertIndex >= 0; --InsertIndex)
	{
		FName CheckCategory = AvailableLogCategories[InsertIndex];
		// No duplicates
		if (CheckCategory == LogCategory)
		{
			return;
		}
		else if (CheckCategory.Compare(LogCategory) < 0)
		{
			break;
		}
	}
	AvailableLogCategories.Insert(LogCategory, InsertIndex + 1);
	if (bShowAllCategories)
	{
		ToggleLogCategory(LogCategory);
	}
}

void FSpatialLogFilter::ToggleLogCategory(const FName& LogCategory)
{
	int32 FoundIndex = SelectedLogCategories.Find(LogCategory);
	if (FoundIndex == INDEX_NONE)
	{
		SelectedLogCategories.Add(LogCategory);
	}
	else
	{
		SelectedLogCategories.RemoveAt(FoundIndex, /*Count=*/1, /*bAllowShrinking=*/false);
	}
}

bool FSpatialLogFilter::IsLogCategoryEnabled(const FName& LogCategory) const
{
	return SelectedLogCategories.Contains(LogCategory);
}

void FSpatialLogFilter::ClearSelectedLogCategories()
{
	// No need to churn memory each time the selected categories are cleared
	SelectedLogCategories.Reset(SelectedLogCategories.GetAllocatedSize());
}

#undef LOCTEXT_NAMESPACE
