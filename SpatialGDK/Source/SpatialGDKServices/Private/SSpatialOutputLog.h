// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

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

class FMenuBuilder;
class FOutputLogTextLayoutMarshaller;
class FTextLayout;
class SMenuAnchor;

/**
* A single log message for the output log, holding a message and
* a style, for color and bolding of the message.
*/
//struct FLogMessage
//{
//	TSharedRef<FString> Message;
//	ELogVerbosity::Type Verbosity;
//	FName Category;
//	FName Style;
//
//	FLogMessage(const TSharedRef<FString>& NewMessage, FName NewCategory, FName NewStyle = NAME_None)
//		: Message(NewMessage)
//		, Verbosity(ELogVerbosity::Log)
//		, Category(NewCategory)
//		, Style(NewStyle)
//	{
//	}
//
//	FLogMessage(const TSharedRef<FString>& NewMessage, ELogVerbosity::Type NewVerbosity, FName NewCategory, FName NewStyle = NAME_None)
//		: Message(NewMessage)
//		, Verbosity(NewVerbosity)
//		, Category(NewCategory)
//		, Style(NewStyle)
//	{
//	}
//};

/**
* Holds information about filters
*/
//struct FLogFilter
//{
//	/** true to show Logs. */
//	bool bShowLogs;
//
//	/** true to show Warnings. */
//	bool bShowWarnings;
//
//	/** true to show Errors. */
//	bool bShowErrors;
//
//	/** true to allow all Log Categories */
//	bool bShowAllCategories;
//
//	/** Enable all filters by default */
//	FLogFilter() : TextFilterExpressionEvaluator(ETextFilterExpressionEvaluatorMode::BasicString)
//	{
//		bShowErrors = bShowLogs = bShowWarnings = bShowAllCategories = true;
//	}
//
//	/** Returns true if any messages should be filtered out */
//	bool IsFilterSet() { return !bShowErrors || !bShowLogs || !bShowWarnings || TextFilterExpressionEvaluator.GetFilterType() != ETextFilterExpressionType::Empty || !TextFilterExpressionEvaluator.GetFilterText().IsEmpty(); }
//
//	/** Checks the given message against set filters */
//	bool IsMessageAllowed(const TSharedPtr<FLogMessage>& Message);
//
//	/** Set the Text to be used as the Filter's restrictions */
//	void SetFilterText(const FText& InFilterText) { TextFilterExpressionEvaluator.SetFilterText(InFilterText); }
//
//	/** Get the Text currently being used as the Filter's restrictions */
//	const FText GetFilterText() const { return TextFilterExpressionEvaluator.GetFilterText(); }
//
//	/** Returns Evaluator syntax errors (if any) */
//	FText GetSyntaxErrors() { return TextFilterExpressionEvaluator.GetFilterErrorText(); }
//
//	const TArray<FName>& GetAvailableLogCategories() { return AvailableLogCategories; }
//
//	/** Adds a Log Category to the list of available categories, if it isn't already present */
//	void AddAvailableLogCategory(FName& LogCategory);
//
//	/** Enables or disables a Log Category in the filter */
//	void ToggleLogCategory(const FName& LogCategory);
//
//	/** Returns true if the specified log category is enabled */
//	bool IsLogCategoryEnabled(const FName& LogCategory) const;
//
//	/** Empties the list of selected log categories */
//	void ClearSelectedLogCategories();
//
//private:
//	/** Expression evaluator that can be used to perform complex text filter queries */
//	FTextFilterExpressionEvaluator TextFilterExpressionEvaluator;
//
//	/** Array of Log Categories which are available for filter -- i.e. have been used in a log this session */
//	TArray<FName> AvailableLogCategories;
//
//	/** Array of Log Categories which are being used in the filter */
//	TArray<FName> SelectedLogCategories;
//};

/** Output log text marshaller to convert an array of FLogMessages into styled lines to be consumed by an FTextLayout */
//class FOutputLogTextLayoutMarshaller : public FBaseTextLayoutMarshaller
//{
//public:
//
//	static TSharedRef< FOutputLogTextLayoutMarshaller > Create(TArray< TSharedPtr<FLogMessage> > InMessages, FLogFilter* InFilter);
//
//	virtual ~FOutputLogTextLayoutMarshaller();
//
//	// ITextLayoutMarshaller
//	virtual void SetText(const FString& SourceString, FTextLayout& TargetTextLayout) override;
//	virtual void GetText(FString& TargetString, const FTextLayout& SourceTextLayout) override;
//
//	bool AppendMessage(const TCHAR* InText, const ELogVerbosity::Type InVerbosity, const FName& InCategory);
//	void ClearMessages();
//
//	void CountMessages();
//
//	int32 GetNumMessages() const;
//	int32 GetNumFilteredMessages();
//
//	void MarkMessagesCacheAsDirty();
//
//protected:
//
//	FOutputLogTextLayoutMarshaller(TArray< TSharedPtr<FLogMessage> > InMessages, FLogFilter* InFilter);
//
//	void AppendMessageToTextLayout(const TSharedPtr<FLogMessage>& InMessage);
//	void AppendMessagesToTextLayout(const TArray<TSharedPtr<FLogMessage>>& InMessages);
//
//	/** All log messages to show in the text box */
//	TArray< TSharedPtr<FLogMessage> > Messages;
//
//	/** Holds cached numbers of messages to avoid unnecessary re-filtering */
//	int32 CachedNumMessages;
//
//	/** Flag indicating the messages count cache needs rebuilding */
//	bool bNumMessagesCacheDirty;
//
//	/** Visible messages filter */
//	FLogFilter* Filter;
//
//	FTextLayout* TextLayout;
//};

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

	/** Destructor for output log, so we can unregister from notifications */
	~SSpatialOutputLog();

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

	/**
	 * Creates FLogMessage objects from FOutputDevice log callback
	 *
	 * @param V Message text
	 * @param Verbosity Message verbosity
	 * @param Category Message category
	 * @param OutMessages Array to receive created FLogMessage messages
	 * @param Filters [Optional] Filters to apply to Messages
	 *
	 * @return true if any messages have been created, false otherwise
	 */
	static bool CreateLogMessages(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category, TArray< TSharedPtr<FLogMessage> >& OutMessages);

protected:

	//virtual void Serialize( const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category ) override;

	///* Remove itself on crash to prevent adding log lines here */
	//void OnCrash();

protected:
	///**
	// * Extends the context menu used by the text box
	// */
	//void ExtendTextBoxMenu(FMenuBuilder& Builder);

	///**
	// * Called when delete all is selected
	// */
	//void OnClearLog();

	///**
	// * Called when the user scrolls the log window vertically
	// */
	//void OnUserScrolled(float ScrollOffset);

	///**
	// * Called to determine whether delete all is currently a valid command
	// */
	//bool CanClearLog() const;

	///** Called when a console command is entered for this output log */
	//void OnConsoleCommandExecuted();

	///** Request we immediately force scroll to the bottom of the log */
	//void RequestForceScroll();

	///** Converts the array of messages into something the text box understands */
	//TSharedPtr< FOutputLogTextLayoutMarshaller > MessagesTextMarshaller;

	///** The editable text showing all log messages */
	//TSharedPtr< SMultiLineEditableTextBox > MessagesTextBox;

	///** The editable text showing all log messages */
	//TSharedPtr< SSearchBox > FilterTextBox;

	///** True if the user has scrolled the window upwards */
	//bool bIsUserScrolled;

private:
	///** Called by Slate when the filter box changes text. */
	//void OnFilterTextChanged(const FText& InFilterText);

	///** Called by Slate when the filter text box is confirmed. */
	//void OnFilterTextCommitted(const FText& InFilterText, ETextCommit::Type InCommitType); 

	///** Make the "Filters" menu. */
	//TSharedRef<SWidget> MakeAddFilterMenu();

	///** Make the "Categories" sub-menu. */
	//void MakeSelectCategoriesSubMenu(FMenuBuilder& MenuBuilder);

	///** Toggles Verbosity "Logs" true/false. */
	//void VerbosityLogs_Execute();

	///** Returns the state of Verbosity "Logs". */
	//bool VerbosityLogs_IsChecked() const;

	///** Toggles Verbosity "Warnings" true/false. */
	//void VerbosityWarnings_Execute();

	///** Returns the state of Verbosity "Warnings". */
	//bool VerbosityWarnings_IsChecked() const;

	///** Toggles Verbosity "Errors" true/false. */
	//void VerbosityErrors_Execute();

	///** Returns the state of Verbosity "Errors". */
	//bool VerbosityErrors_IsChecked() const;

	///** Toggles All Categories true/false. */
	//void CategoriesShowAll_Execute();

	///** Returns the state of "Show All" */
	//bool CategoriesShowAll_IsChecked() const;

	///** Toggles the given category true/false. */
	//void CategoriesSingle_Execute(FName InName);

	///** Returns the state of the given category */
	//bool CategoriesSingle_IsChecked(FName InName) const;

	///** Forces re-population of the messages list */
	//void Refresh();

	FString CurrentLogDir;
	FString CurrentLogFile;

public:
	///** Visible messages filter */
	//FLogFilter Filter;
};
