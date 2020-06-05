// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorLayoutDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Notifications/SPopupErrorText.h"
#include "Widgets/Text/STextBlock.h"

#include "SpatialCommandUtils.h"
#include "SpatialGDKEditor.h"
#include "SpatialGDKEditorCommandLineArgsManager.h"
#include "SpatialGDKEditorModule.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKSettings.h"

TSharedRef<IDetailCustomization> FSpatialGDKEditorLayoutDetails::MakeInstance()
{
	return MakeShareable(new FSpatialGDKEditorLayoutDetails);
}

void FSpatialGDKEditorLayoutDetails::ForceRefreshLayout()
{
	if (CurrentLayout != nullptr)
	{
		TArray<TWeakObjectPtr<UObject>> Objects;
		CurrentLayout->GetObjectsBeingCustomized(Objects);
		USpatialGDKEditorSettings* Settings = Objects.Num() > 0 ? Cast<USpatialGDKEditorSettings>(Objects[0].Get()) : nullptr;
		CurrentLayout->ForceRefreshDetails();
	}
}

void FSpatialGDKEditorLayoutDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	CurrentLayout = &DetailBuilder;
	const USpatialGDKSettings* GDKSettings = GetDefault<USpatialGDKSettings>();

	IDetailCategoryBuilder& CloudConnectionCategory = DetailBuilder.EditCategory("Cloud Connection");
	CloudConnectionCategory.AddCustomRow(FText::FromString("Project Name"))
		.NameContent()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString(TEXT("Project Name"))))
				.ToolTipText(FText::FromString(FString(TEXT("The name of the SpatialOS project."))))
			]
		]
		.ValueContent()
		.VAlign(VAlign_Center)
		.MinDesiredWidth(250)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(ProjectName))
				.ToolTipText(FText::FromString(FString(TEXT("The name of the SpatialOS project."))))
				.OnTextCommitted(this, &FSpatialGDKEditorLayoutDetails::OnProjectNameCommitted)
				.ErrorReporting(ProjectNameInputErrorReporting)
			]
		];

	CloudConnectionCategory.AddCustomRow(FText::FromString("Generate Development Authentication Token"))
		.ValueContent()
		.VAlign(VAlign_Center)
		.MinDesiredWidth(250)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.OnClicked_Static(FSpatialGDKEditorCommandLineArgsManager::GenerateDevAuthToken)
			.Content()
			[
				SNew(STextBlock).Text(FText::FromString("Generate Dev Auth Token"))
			]
		];
		
	IDetailCategoryBuilder& MobileCategory = DetailBuilder.EditCategory("Mobile");
	MobileCategory.AddCustomRow(FText::FromString("Push SpatialOS settings to Android device"))
		.ValueContent()
		.VAlign(VAlign_Center)
		.MinDesiredWidth(550)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.OnClicked_Static(FSpatialGDKEditorCommandLineArgsManager::PushCommandLineToAndroidDevice)
				.Content()
				[
					SNew(STextBlock).Text(FText::FromString("Push SpatialOS settings to Android device"))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.OnClicked_Static(FSpatialGDKEditorCommandLineArgsManager::RemoveCommandLineFromAndroidDevice)
				.Content()
				[
					SNew(STextBlock).Text(FText::FromString("Remove SpatialOS settings from Android device"))
				]
			]
		];

	MobileCategory.AddCustomRow(FText::FromString("Push SpatialOS settings to iOS device"))
		.ValueContent()
		.VAlign(VAlign_Center)
		.MinDesiredWidth(275)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.OnClicked_Static(FSpatialGDKEditorCommandLineArgsManager::PushCommandLineToIOSDevice)
			.Content()
			[
				SNew(STextBlock).Text(FText::FromString("Push SpatialOS settings to iOS device"))
			]
		];
}

void FSpatialGDKEditorLayoutDetails::OnProjectNameCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	FString NewProjectName = InText.ToString();
	if (!USpatialGDKEditorSettings::IsProjectNameValid(NewProjectName))
	{
		ProjectNameInputErrorReporting->SetError(SpatialConstants::ProjectPatternHint);
		return;
	}
	ProjectNameInputErrorReporting->SetError(TEXT(""));

	TSharedPtr<FSpatialGDKEditor> SpatialGDKEditorInstance = FModuleManager::GetModuleChecked<FSpatialGDKEditorModule>("SpatialGDKEditor").GetSpatialGDKEditorInstance();
	SpatialGDKEditorInstance->SetProjectName(NewProjectName);
}
