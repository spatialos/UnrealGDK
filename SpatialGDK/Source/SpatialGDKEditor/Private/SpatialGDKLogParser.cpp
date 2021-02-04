// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKLogParser.h"

#include "Algo/AllOf.h"
#include "Dialogs/CustomDialog.h"
#include "Editor.h"
#include "Internationalization/Regex.h"
#include "Logging/MessageLog.h"
#include "MessageLogModule.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Input/SHyperlink.h"

#define LOCTEXT_NAMESPACE "SpatialGDKLogParser"

const TCHAR MessageLogCategoryName[] = TEXT("MissingSchemaMessageLog");

static void ShowMissingSchemaDialog(const TSet<FString>& MissingSchemaPaths)
{
	struct Local
	{
		static void OnHyperlinkClicked(const FString& InBlueprint, TSharedPtr<SCustomDialog> InDialog)
		{
			const FSoftObjectPath ObjectAtPath(InBlueprint);

			UObject* ObjectToOpen = ObjectAtPath.ResolveObject();

			if (!IsValid((ObjectToOpen)))
			{
				// The object could be unloaded; try and load it from the path.
				ObjectToOpen = ObjectAtPath.TryLoad();
			}

			if (IsValid(ObjectToOpen))
			{
				if (ObjectToOpen->IsA<UBlueprintGeneratedClass>())
				{
					// BlueprintGeneratedClass can't be edited, but ClassGeneratedBy references
					// the UBlueprint that has caused the class to be generated - open it instead.
					ObjectToOpen = Cast<UBlueprintGeneratedClass>(ObjectToOpen)->ClassGeneratedBy;
				}
			}

			if (IsValid(ObjectToOpen))
			{
				// Finally, if the object is available - ask the editor to edit it.
				GEditor->EditObject(ObjectToOpen);
			}

			if (InDialog.IsValid())
			{
				// Opening the blueprint editor above may end up creating an invisible new window on top of the dialog,
				// thus making it not interactable, so we have to force the dialog back to the front
				InDialog->BringToFront(true);
			}
		}
	};

	const FText MissingSchemaLabel =
		LOCTEXT("MissingSchemaLabel", "The following objects have missing schema, check the logs for more information.");

	TSharedRef<SVerticalBox> DialogContents =
		SNew(SVerticalBox) + SVerticalBox::Slot().Padding(0, 0, 0, 16)[SNew(STextBlock).Text(MissingSchemaLabel)];

	TSharedPtr<SCustomDialog> CustomDialog;

	FMessageLog MissingSchemaLog(MessageLogCategoryName);

	MissingSchemaLog.NewPage(MissingSchemaLabel);

	const FText LogMessage = LOCTEXT("MissingSchemaLogMessage", "Object doesn't have schema generated for it.");

	for (const FString& MissingSchemaPath : MissingSchemaPaths)
	{
		const FText MissingSchemaPathText = FText::FromString(MissingSchemaPath);

		DialogContents->AddSlot().AutoHeight().HAlign(
			HAlign_Left)[SNew(SHyperlink)
							 .OnNavigate(FSimpleDelegate::CreateLambda([MissingSchemaPath, &CustomDialog]() {
								 Local::OnHyperlinkClicked(MissingSchemaPath, CustomDialog);
							 }))
							 .Text(MissingSchemaPathText)
							 .ToolTipText(LOCTEXT("MissingSchemaDialogLinkTT", "Click to open the object"))];

		MissingSchemaLog.Error(LogMessage)->AddToken(FAssetNameToken::Create(MissingSchemaPath));
	}

	MissingSchemaLog.Open();

	const FText DialogTitle = LOCTEXT("MissingSchemaDialogTitle", "Missing Schema");

	const FText OKText = LOCTEXT("MissingSchemaDialogOk", "OK");

	CustomDialog = SNew(SCustomDialog).Title(DialogTitle).DialogContent(DialogContents).Buttons({ SCustomDialog::FButton(OKText) });

	CustomDialog->ShowModal();
}

class FMissingSchemaLogParser : public FOutputDevice
{
public:
	~FMissingSchemaLogParser()
	{
		if (bShouldReportErrors && Paths.Num() > 0)
		{
			ShowMissingSchemaDialog(Paths);
		}
	}

	virtual void Serialize(const TCHAR* LogMessage, ELogVerbosity::Type Verbosity, const FName& Category) override
	{
		static const FName SpatialClassInfoManagerLogCategoryName(TEXT("LogSpatialClassInfoManager"));

		if (Category == SpatialClassInfoManagerLogCategoryName && Verbosity <= ELogVerbosity::Warning)
		{
			const FString Message(LogMessage);

			const static TSet<FString> Keywords{ TEXT("no"), TEXT("schema") };

			const bool bMatchesKeywords = Algo::AllOf(Keywords, [&Message](const FString& Keyword) {
				return Message.Contains(Keyword);
			});

			if (bMatchesKeywords)
			{
				// Alphanumeric, dot and underscore seem to cover UE class paths:
				// /Game/Directory/AnotherDirectory_4/Asset.Asset_C
				static const FRegexPattern ClassPathPattern(TEXT("(/[\\w\\d/\\._]+_C)"));

				FRegexMatcher ClassPathMatcher(ClassPathPattern, Message);

				if (ClassPathMatcher.FindNext())
				{
					// Use 1 to extract the first match. Class path is the part of the log that we're looking for.
					const FString ClassName = ClassPathMatcher.GetCaptureGroup(1);

					Paths.Add(ClassName);
				}
			}
		}
	}

	TSet<FString> Paths;
	bool bShouldReportErrors = true;
};

FSpatialGDKLogParser::FSpatialGDKLogParser()
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	MessageLogModule.RegisterLogListing(MessageLogCategoryName, LOCTEXT("MissingSchemaMessageLogLabel", "Missing Schema"));

	PreBeginPIEDelegateHandle = FEditorDelegates::PreBeginPIE.AddLambda([this](bool) {
		ensure(!MissingSchemaErrorParser.IsValid());
		MissingSchemaErrorParser = MakeShared<FMissingSchemaLogParser>();
		FOutputDeviceRedirector::Get()->AddOutputDevice(MissingSchemaErrorParser.Get());
	});

	EndPIEDelegateHandle = FEditorDelegates::EndPIE.AddLambda([this](bool) {
		ensure(MissingSchemaErrorParser.IsValid());
		FOutputDeviceRedirector::Get()->RemoveOutputDevice(MissingSchemaErrorParser.Get());
		MissingSchemaErrorParser.Reset();
	});
}

FSpatialGDKLogParser::~FSpatialGDKLogParser()
{
	if (PreBeginPIEDelegateHandle.IsValid())
	{
		FEditorDelegates::PreBeginPIE.Remove(PreBeginPIEDelegateHandle);
		PreBeginPIEDelegateHandle.Reset();
	}

	if (EndPIEDelegateHandle.IsValid())
	{
		FEditorDelegates::EndPIE.Remove(EndPIEDelegateHandle);
		EndPIEDelegateHandle.Reset();
	}

	if (MissingSchemaErrorParser.IsValid())
	{
		MissingSchemaErrorParser->bShouldReportErrors = false;
	}
}

#undef LOCTEXT_NAMESPACE
