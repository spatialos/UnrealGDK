// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKSimulatedPlayerDeployment.h"

#include "SpatialCommandUtils.h"
#include "SpatialGDKDefaultLaunchConfigGenerator.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKEditorToolbar.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#include "SpatialGDKSettings.h"

#include "Async/Async.h"
#include "DesktopPlatformModule.h"
#include "Editor.h"
#include "EditorDirectories.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/MessageDialog.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Templates/SharedPointer.h"
#include "Textures/SlateIcon.h"
#include "Utils/LaunchConfigEditor.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SFilePathPicker.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/Text/STextBlock.h"

#include "Internationalization/Regex.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKSimulatedPlayerDeployment);

void SSpatialGDKSimulatedPlayerDeployment::Construct(const FArguments& InArgs)
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	FString ProjectName = FSpatialGDKServicesModule::GetProjectName();

	ParentWindowPtr = InArgs._ParentWindow;
	SpatialGDKEditorPtr = InArgs._SpatialGDKEditor;

	ProjectNameEdit = SNew(SEditableTextBox)
		.Text(FText::FromString(ProjectName))
		.ToolTipText(FText::FromString(FString(TEXT("The name of the SpatialOS project."))))
		.OnTextCommitted(this, &SSpatialGDKSimulatedPlayerDeployment::OnProjectNameCommitted);

	ChildSlot
		[
			SNew(SBorder)
			.HAlign(HAlign_Fill)
			.BorderImage(FEditorStyle::GetBrush("ChildWindow.Background"))
			.Padding(4.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(0.0f, 6.0f, 0.0f, 0.0f)
				[
					SNew(SBorder)
					.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
					.Padding(4.0f)
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(1.0f)
						[
							SNew(SVerticalBox)
							// Build explanation set
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							.VAlign(VAlign_Center)
							[
								SNew(SWrapBox)
								.UseAllottedWidth(true)
								+ SWrapBox::Slot()
								.VAlign(VAlign_Bottom)
								[
									SNew(STextBlock)
									.AutoWrapText(true)
									.Text(FText::FromString(FString(TEXT("NOTE: You can set default values in the SpatialOS settings under \"Cloud\"."))))
								]
								+ SWrapBox::Slot()
								.VAlign(VAlign_Bottom)
								[
									SNew(STextBlock)
									.AutoWrapText(true)
								.Text(FText::FromString(FString(TEXT("The assembly has to be built and uploaded manually. Follow the docs "))))
								]
								+ SWrapBox::Slot()
								[
									SNew(SHyperlink)
									.Text(FText::FromString(FString(TEXT("here."))))
									.OnNavigate(this, &SSpatialGDKSimulatedPlayerDeployment::OnCloudDocumentationClicked)
								]
							]
							// Separator
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							.VAlign(VAlign_Center)
							[
								SNew(SSeparator)
							]
							// Project 
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Project Name"))))
									.ToolTipText(FText::FromString(FString(TEXT("The name of the SpatialOS project."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									ProjectNameEdit.ToSharedRef()
								]
							]
							// Assembly Name 
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Assembly Name"))))
									.ToolTipText(FText::FromString(FString(TEXT("The name of the assembly."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SEditableTextBox)
									.Text(FText::FromString(SpatialGDKSettings->GetAssemblyName()))
									.ToolTipText(FText::FromString(FString(TEXT("The name of the assembly."))))
									.OnTextCommitted(this, &SSpatialGDKSimulatedPlayerDeployment::OnDeploymentAssemblyCommited)
									.OnTextChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnDeploymentAssemblyCommited, ETextCommit::Default)
								]
							]
							// RuntimeVersion
							+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(2.0f)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Use GDK Pinned Version"))))
									.ToolTipText(FText::FromString(FString(TEXT("Whether to use the SpatialOS Runtime version associated to the current GDK version"))))
								]
							+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SSpatialGDKSimulatedPlayerDeployment::IsUsingGDKPinnedRuntimeVersion)
									.OnCheckStateChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnCheckedUsePinnedVersion)
								]
								]
							+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(2.0f)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Runtime Version"))))
									.ToolTipText(FText::FromString(FString(TEXT("User supplied version of the SpatialOS runtime to use"))))
								]
							+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SEditableTextBox)
									.Text(this, &SSpatialGDKSimulatedPlayerDeployment::GetSpatialOSRuntimeVersionToUseText)
									.OnTextCommitted(this, &SSpatialGDKSimulatedPlayerDeployment::OnRuntimeCustomVersionCommited)
									.OnTextChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnRuntimeCustomVersionCommited, ETextCommit::Default)
									.IsEnabled(this, &SSpatialGDKSimulatedPlayerDeployment::IsUsingCustomRuntimeVersion)
								]
								]
							// Pirmary Deployment Name 
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Deployment Name"))))
									.ToolTipText(FText::FromString(FString(TEXT("The name of the cloud deployment. Must be unique."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SEditableTextBox)
									.Text(FText::FromString(SpatialGDKSettings->GetPrimaryDeploymentName()))
									.ToolTipText(FText::FromString(FString(TEXT("The name of the cloud deployment. Must be unique."))))
									.OnTextCommitted(this, &SSpatialGDKSimulatedPlayerDeployment::OnPrimaryDeploymentNameCommited)
									.OnTextChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnPrimaryDeploymentNameCommited, ETextCommit::Default)
								]
							]
							// Snapshot File + File Picker
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Snapshot File"))))
									.ToolTipText(FText::FromString(FString(TEXT("The relative path to the snapshot file."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SFilePathPicker)
									.BrowseButtonImage(FEditorStyle::GetBrush("PropertyWindow.Button_Ellipsis"))
									.BrowseButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
									.BrowseButtonToolTip(FText::FromString(FString(TEXT("Path to the snapshot file."))))
									.BrowseDirectory(SpatialGDKSettings->GetSpatialOSSnapshotFolderPath())
									.BrowseTitle(FText::FromString(FString(TEXT("File picker..."))))
									.FilePath_UObject(SpatialGDKSettings, &USpatialGDKEditorSettings::GetSnapshotPath)
									.FileTypeFilter(TEXT("Snapshot files (*.snapshot)|*.snapshot"))
									.OnPathPicked(this, &SSpatialGDKSimulatedPlayerDeployment::OnSnapshotPathPicked)
								]
							]
							// Primary Launch Config + File Picker
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Launch Config File"))))
									.ToolTipText(FText::FromString(FString(TEXT("The relative path to the launch configuration file."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SFilePathPicker)
									.BrowseButtonImage(FEditorStyle::GetBrush("PropertyWindow.Button_Ellipsis"))
									.BrowseButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
									.BrowseButtonToolTip(FText::FromString(FString(TEXT("Path to the launch configuration file."))))
									.BrowseDirectory(SpatialGDKServicesConstants::SpatialOSDirectory)
									.BrowseTitle(FText::FromString(FString(TEXT("File picker..."))))
									.FilePath_UObject(SpatialGDKSettings, &USpatialGDKEditorSettings::GetPrimaryLaunchConfigPath)
									.FileTypeFilter(TEXT("Launch configuration files (*.json)|*.json"))
									.OnPathPicked(this, &SSpatialGDKSimulatedPlayerDeployment::OnPrimaryLaunchConfigPathPicked)
								]
							]
							+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(2.0f)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT(""))))
								.ToolTipText(FText::FromString(FString(TEXT(""))))
								]
							+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SButton)
									.Text(FText::FromString(FString(TEXT("Generate from current map"))))
									.OnClicked(this, &SSpatialGDKSimulatedPlayerDeployment::OnGenerateConfigFromCurrentMap)
								]
								]
							+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(2.0f)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT(""))))
								.ToolTipText(FText::FromString(FString(TEXT(""))))
								]
							+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SButton)
									.Text(FText::FromString(FString(TEXT("Open Launch Configuration editor"))))
									.OnClicked(this, &SSpatialGDKSimulatedPlayerDeployment::OnOpenLaunchConfigEditor)
								]
							]
							// Primary Deployment Region Picker
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Region"))))
									.ToolTipText(FText::FromString(FString(TEXT("The region in which the deployment will be deployed."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SComboButton)
									.OnGetMenuContent(this, &SSpatialGDKSimulatedPlayerDeployment::OnGetPrimaryDeploymentRegionCode)
									.ContentPadding(FMargin(2.0f, 2.0f))
									.ButtonContent()
									[
										SNew(STextBlock)
										.Text_UObject(SpatialGDKSettings, &USpatialGDKEditorSettings::GetPrimaryRegionCode)
									]
								]
							]
							// Main Deployment Cluster
							+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(2.0f)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Deployment Cluster"))))
								.ToolTipText(FText::FromString(FString(TEXT("The name of the cluster to deploy to."))))
								]
							+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SEditableTextBox)
									.Text(FText::FromString(SpatialGDKSettings->GetMainDeploymentCluster()))
								.ToolTipText(FText::FromString(FString(TEXT("The name of the cluster to deploy to."))))
								.OnTextCommitted(this, &SSpatialGDKSimulatedPlayerDeployment::OnDeploymentClusterCommited)
								.OnTextChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnDeploymentClusterCommited, ETextCommit::Default)
								]
							]
							// Separator
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							.VAlign(VAlign_Center)
							[
								SNew(SSeparator)
							]
							// Explanation text
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							.VAlign(VAlign_Center)
							.HAlign(HAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(FString(TEXT("Simulated Players"))))
							]
							// Toggle
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							.VAlign(VAlign_Center)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SVerticalBox)
									+ SVerticalBox::Slot()
									.AutoHeight()
									.Padding(2.0f)
									.VAlign(VAlign_Center)
									[
										SNew(SHorizontalBox)
										+ SHorizontalBox::Slot()
										.AutoWidth()
										[
											SNew(SCheckBox)
											.IsChecked(this, &SSpatialGDKSimulatedPlayerDeployment::IsSimulatedPlayersEnabled)
											.OnCheckStateChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnCheckedSimulatedPlayers)
										]
										+ SHorizontalBox::Slot()
										.AutoWidth()
										.HAlign(HAlign_Center)
										[
											SNew(STextBlock)
											.Text(FText::FromString(FString(TEXT("Add simulated players"))))
										]
									]
								]
							]
							// Simulated Players Deployment Name
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Deployment Name"))))
									.ToolTipText(FText::FromString(FString(TEXT("The name of the simulated player deployment."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SEditableTextBox)
									.Text(FText::FromString(SpatialGDKSettings->GetSimulatedPlayerDeploymentName()))
									.ToolTipText(FText::FromString(FString(TEXT("The name of the simulated player deployment."))))
									.OnTextCommitted(this, &SSpatialGDKSimulatedPlayerDeployment::OnSimulatedPlayerDeploymentNameCommited)
									.OnTextChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnSimulatedPlayerDeploymentNameCommited, ETextCommit::Default)
									.IsEnabled_UObject(SpatialGDKSettings, &USpatialGDKEditorSettings::IsSimulatedPlayersEnabled)
								]
							]
							// Simulated Players Number 
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Number of Simulated Players"))))
									.ToolTipText(FText::FromString(FString(TEXT("The number of Simulated Players to be launch and connect to the game."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SSpinBox<uint32>)
									.ToolTipText(FText::FromString(FString(TEXT("Number of Simulated Players."))))
									.MinValue(1)
									.MaxValue(8192)
									.Value(SpatialGDKSettings->GetNumberOfSimulatedPlayer())
									.OnValueChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnNumberOfSimulatedPlayersCommited)
									.IsEnabled_UObject(SpatialGDKSettings, &USpatialGDKEditorSettings::IsSimulatedPlayersEnabled)
								]
							]
							// Simulated Players Deployment Region Picker
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Region"))))
									.ToolTipText(FText::FromString(FString(TEXT("The region in which the simulated player deployment will be deployed."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SComboButton)
									.OnGetMenuContent(this, &SSpatialGDKSimulatedPlayerDeployment::OnGetSimulatedPlayerDeploymentRegionCode)
									.ContentPadding(FMargin(2.0f, 2.0f))
									.IsEnabled_UObject(SpatialGDKSettings, &USpatialGDKEditorSettings::IsSimulatedPlayersEnabled)
									.ButtonContent()
									[
										SNew(STextBlock)
										.Text_UObject(SpatialGDKSettings, &USpatialGDKEditorSettings::GetSimulatedPlayerRegionCode)
									]
								]
							]
							// Simulated Player Cluster
							+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(2.0f)
								[
									SNew(SHorizontalBox)
									+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Deployment Cluster"))))
								.ToolTipText(FText::FromString(FString(TEXT("The name of the cluster to deploy to."))))
								]
							+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SEditableTextBox)
									.Text(FText::FromString(SpatialGDKSettings->GetSimulatedPlayerCluster()))
								.ToolTipText(FText::FromString(FString(TEXT("The name of the cluster to deploy to."))))
								.OnTextCommitted(this, &SSpatialGDKSimulatedPlayerDeployment::OnSimulatedPlayerClusterCommited)
								.OnTextChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnSimulatedPlayerClusterCommited, ETextCommit::Default)
								.IsEnabled_UObject(SpatialGDKSettings, &USpatialGDKEditorSettings::IsSimulatedPlayersEnabled)
								]
							]
							// Buttons
							+ SVerticalBox::Slot()
							.FillHeight(1.0f)
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.HAlign(HAlign_Right)
								[
									// Launch Simulated Players Deployment Button
									SNew(SUniformGridPanel)
									.SlotPadding(FMargin(2.0f, 20.0f, 0.0f, 0.0f))
									+ SUniformGridPanel::Slot(0, 0)
									[
										SNew(SButton)
										.HAlign(HAlign_Center)
										.Text(FText::FromString(FString(TEXT("Launch Deployment"))))
										.OnClicked(this, &SSpatialGDKSimulatedPlayerDeployment::OnLaunchClicked)
										.IsEnabled(this, &SSpatialGDKSimulatedPlayerDeployment::IsDeploymentConfigurationValid)
									]
								]
							]
						]
					]
				]
			]
		];
}

void SSpatialGDKSimulatedPlayerDeployment::OnDeploymentAssemblyCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetAssemblyName(InText.ToString());
}

void SSpatialGDKSimulatedPlayerDeployment::OnProjectNameCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	FString NewProjectName = InText.ToString();
	if (!USpatialGDKEditorSettings::IsProjectNameValid(NewProjectName))
	{
		ProjectNameEdit->SetError(TEXT("Project name may only contain lowercase alphanumeric characters or '_', and must be between 3 and 32 characters long."));
		return;
	}
	else
	{
		ProjectNameEdit->SetError(TEXT(""));
	}

	FSpatialGDKServicesModule::SetProjectName(NewProjectName);
}

void SSpatialGDKSimulatedPlayerDeployment::OnPrimaryDeploymentNameCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetPrimaryDeploymentName(InText.ToString());
}

void SSpatialGDKSimulatedPlayerDeployment::OnCheckedUsePinnedVersion(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetUseGDKPinnedRuntimeVersion(NewCheckedState == ECheckBoxState::Checked);
}

void SSpatialGDKSimulatedPlayerDeployment::OnRuntimeCustomVersionCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetCustomCloudSpatialOSRuntimeVersion(InText.ToString());
}

void SSpatialGDKSimulatedPlayerDeployment::OnSnapshotPathPicked(const FString& PickedPath)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetSnapshotPath(PickedPath);
}

void SSpatialGDKSimulatedPlayerDeployment::OnPrimaryLaunchConfigPathPicked(const FString& PickedPath)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetPrimaryLaunchConfigPath(PickedPath);
}

TSharedRef<SWidget> SSpatialGDKSimulatedPlayerDeployment::OnGetPrimaryDeploymentRegionCode()
{
	FMenuBuilder MenuBuilder(true, NULL);
	UEnum* pEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ERegionCode"), true);

	if (pEnum != nullptr)
	{
		for (int32 i = 0; i < pEnum->NumEnums() - 1; i++)
		{
			int64 CurrentEnumValue = pEnum->GetValueByIndex(i);
			FUIAction ItemAction(FExecuteAction::CreateSP(this, &SSpatialGDKSimulatedPlayerDeployment::OnPrimaryDeploymentRegionCodePicked, CurrentEnumValue));
			MenuBuilder.AddMenuEntry(pEnum->GetDisplayNameTextByValue(CurrentEnumValue), TAttribute<FText>(), FSlateIcon(), ItemAction);
		}
	}

	return MenuBuilder.MakeWidget();
}

void SSpatialGDKSimulatedPlayerDeployment::OnDeploymentClusterCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetMainDeploymentCluster(InText.ToString());
}

TSharedRef<SWidget> SSpatialGDKSimulatedPlayerDeployment::OnGetSimulatedPlayerDeploymentRegionCode()
{
	FMenuBuilder MenuBuilder(true, NULL);
	UEnum* pEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ERegionCode"), true);

	if (pEnum != nullptr)
	{
		for (int32 i = 0; i < pEnum->NumEnums() - 1; i++)
		{
			int64 CurrentEnumValue = pEnum->GetValueByIndex(i);
			FUIAction ItemAction(FExecuteAction::CreateSP(this, &SSpatialGDKSimulatedPlayerDeployment::OnSimulatedPlayerDeploymentRegionCodePicked, CurrentEnumValue));
			MenuBuilder.AddMenuEntry(pEnum->GetDisplayNameTextByValue(CurrentEnumValue), TAttribute<FText>(), FSlateIcon(), ItemAction);
		}
	}
	
	return MenuBuilder.MakeWidget();
}

void SSpatialGDKSimulatedPlayerDeployment::OnSimulatedPlayerClusterCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetSimulatedPlayerCluster(InText.ToString());
}

void SSpatialGDKSimulatedPlayerDeployment::OnPrimaryDeploymentRegionCodePicked(const int64 RegionCodeEnumValue)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetPrimaryRegionCode((ERegionCode::Type) RegionCodeEnumValue);

}

void SSpatialGDKSimulatedPlayerDeployment::OnSimulatedPlayerDeploymentRegionCodePicked(const int64 RegionCodeEnumValue)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetSimulatedPlayerRegionCode((ERegionCode::Type) RegionCodeEnumValue);
}

void SSpatialGDKSimulatedPlayerDeployment::OnSimulatedPlayerDeploymentNameCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetSimulatedPlayerDeploymentName(InText.ToString());
}

void SSpatialGDKSimulatedPlayerDeployment::OnNumberOfSimulatedPlayersCommited(uint32 NewValue)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetNumberOfSimulatedPlayers(NewValue);
}

FReply SSpatialGDKSimulatedPlayerDeployment::OnLaunchClicked()
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();

	FSpatialGDKEditorToolbarModule* ToolbarPtr = FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar");

	if (!SpatialGDKSettings->IsDeploymentConfigurationValid())
	{
		if (ToolbarPtr)
		{
			ToolbarPtr->OnShowFailedNotification(TEXT("Deployment configuration is not valid."));
		}

		return FReply::Handled();
	}

	if (SpatialGDKSettings->IsSimulatedPlayersEnabled())
	{
		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
		FString BuiltWorkerFolder = GetDefault<USpatialGDKEditorSettings>()->GetBuiltWorkerFolder();
		FString BuiltSimPlayersName = TEXT("UnrealSimulatedPlayer@Linux.zip");
		FString BuiltSimPlayerPath = FPaths::Combine(BuiltWorkerFolder, BuiltSimPlayersName);

		if (!PlatformFile.FileExists(*BuiltSimPlayerPath))
		{
			FString MissingSimPlayerBuildText = FString::Printf(TEXT("Warning: Detected that %s is missing. To launch a successful SimPlayer deployment ensure that SimPlayers is built and uploaded.\n\nWould you still like to continue with the deployment?"), *BuiltSimPlayersName);
			EAppReturnType::Type UserAnswer = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(MissingSimPlayerBuildText));
			if (UserAnswer == EAppReturnType::No || UserAnswer == EAppReturnType::Cancel)
			{
				return FReply::Handled();
			}
		}
	}

	if (ToolbarPtr)
	{
		ToolbarPtr->OnShowTaskStartNotification(TEXT("Starting cloud deployment..."));
	}

	auto LaunchCloudDeployment = [this, ToolbarPtr]()
	{
		if (TSharedPtr<FSpatialGDKEditor> SpatialGDKEditorSharedPtr = SpatialGDKEditorPtr.Pin())
		{
			SpatialGDKEditorSharedPtr->LaunchCloudDeployment(
				FSimpleDelegate::CreateLambda([]()
				{
					if (FSpatialGDKEditorToolbarModule* ToolbarPtr = FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar"))
					{
						ToolbarPtr->OnShowSuccessNotification("Successfully launched cloud deployment.");
					}
				}),

				FSimpleDelegate::CreateLambda([]()
				{
					if (FSpatialGDKEditorToolbarModule* ToolbarPtr = FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar"))
					{
						ToolbarPtr->OnShowFailedNotification("Failed to launch cloud deployment. See output logs for details.");
					}
				})
			);

			return;
		}

		FNotificationInfo Info(FText::FromString(TEXT("Couldn't launch the deployment.")));
		Info.bUseSuccessFailIcons = true;
		Info.ExpireDuration = 3.0f;

		TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
		NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
	};

#if ENGINE_MINOR_VERSION <= 22
	AttemptSpatialAuthResult = Async<bool>(EAsyncExecution::Thread, []() { return SpatialCommandUtils::AttemptSpatialAuth(GetDefault<USpatialGDKSettings>()->IsRunningInChina()); },
#else
	AttemptSpatialAuthResult = Async(EAsyncExecution::Thread, []() { return SpatialCommandUtils::AttemptSpatialAuth(GetDefault<USpatialGDKSettings>()->IsRunningInChina()); },
#endif
		[this, LaunchCloudDeployment, ToolbarPtr]()
	{
		if (AttemptSpatialAuthResult.IsReady() && AttemptSpatialAuthResult.Get() == true)
		{
			LaunchCloudDeployment();
		}
		else
		{
			ToolbarPtr->OnShowTaskStartNotification(TEXT("Spatial auth failed attempting to launch cloud deployment."));
		}
	});

	return FReply::Handled();
}

FReply SSpatialGDKSimulatedPlayerDeployment::OnRefreshClicked()
{
	// TODO (UNR-1193): Invoke the Deployment Launcher script to list the deployments
	return FReply::Handled();
}

FReply SSpatialGDKSimulatedPlayerDeployment::OnStopClicked()
{
	if (TSharedPtr<FSpatialGDKEditor> SpatialGDKEditorSharedPtr = SpatialGDKEditorPtr.Pin()) {

		if (FSpatialGDKEditorToolbarModule* ToolbarPtr = FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar"))
		{
			ToolbarPtr->OnShowTaskStartNotification("Stopping cloud deployment ...");
		}

		SpatialGDKEditorSharedPtr->StopCloudDeployment(
			FSimpleDelegate::CreateLambda([]()
			{
				if (FSpatialGDKEditorToolbarModule* ToolbarPtr = FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar"))
				{
					ToolbarPtr->OnShowSuccessNotification("Successfully stopped cloud deployment.");
				}
			}),

			FSimpleDelegate::CreateLambda([]()
			{
				if (FSpatialGDKEditorToolbarModule* ToolbarPtr = FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar"))
				{
					ToolbarPtr->OnShowFailedNotification("Failed to stop cloud deployment.");
				}
			}));
	}
	return FReply::Handled();
}

void SSpatialGDKSimulatedPlayerDeployment::OnCloudDocumentationClicked()
{
	FString WebError;
	FPlatformProcess::LaunchURL(TEXT("https://documentation.improbable.io/gdk-for-unreal/docs/cloud-deployment-workflow#section-build-server-worker-assembly"), TEXT(""), &WebError);
	if (!WebError.IsEmpty())
	{
		FNotificationInfo Info(FText::FromString(WebError));
		Info.ExpireDuration = 3.0f;
		Info.bUseSuccessFailIcons = true;
		TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
		NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
		NotificationItem->ExpireAndFadeout();
	}
}

void SSpatialGDKSimulatedPlayerDeployment::OnCheckedSimulatedPlayers(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetSimulatedPlayersEnabledState(NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSpatialGDKSimulatedPlayerDeployment::IsSimulatedPlayersEnabled() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKSettings->IsSimulatedPlayersEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

bool SSpatialGDKSimulatedPlayerDeployment::IsDeploymentConfigurationValid() const
{
	return true;
}

ECheckBoxState SSpatialGDKSimulatedPlayerDeployment::IsUsingGDKPinnedRuntimeVersion() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKSettings->GetUseGDKPinnedRuntimeVersion() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

bool SSpatialGDKSimulatedPlayerDeployment::IsUsingCustomRuntimeVersion() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return !SpatialGDKSettings->GetUseGDKPinnedRuntimeVersion();
}

FText SSpatialGDKSimulatedPlayerDeployment::GetSpatialOSRuntimeVersionToUseText() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	const FString& RuntimeVersion = SpatialGDKSettings->bUseGDKPinnedRuntimeVersion ? SpatialGDKServicesConstants::SpatialOSRuntimePinnedVersion : SpatialGDKSettings->CloudRuntimeVersion;
	return FText::FromString(RuntimeVersion);
}


FReply SSpatialGDKSimulatedPlayerDeployment::OnGenerateConfigFromCurrentMap()
{
	UWorld* EditorWorld = GEditor->GetEditorWorldContext().World();
	check(EditorWorld != nullptr);

	const FString LaunchConfig = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectIntermediateDir()), FString::Printf(TEXT("Improbable/%s_CloudLaunchConfig.json"), *EditorWorld->GetMapName()));

	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();

	FSpatialLaunchConfigDescription LaunchConfiguration = SpatialGDKEditorSettings->LaunchConfigDesc;
	TMap<FName, FWorkerTypeLaunchSection> ServerWorkers;

	FillWorkerConfigurationFromCurrentMap(ServerWorkers, LaunchConfiguration.World.Dimensions);

	GenerateLaunchConfig(LaunchConfig, &LaunchConfiguration, ServerWorkers);

	OnPrimaryLaunchConfigPathPicked(LaunchConfig);

	return FReply::Handled();
}

FReply SSpatialGDKSimulatedPlayerDeployment::OnOpenLaunchConfigEditor()
{
	ULaunchConfigurationEditor* Editor = UTransientUObjectEditor::LaunchTransientUObjectEditor<ULaunchConfigurationEditor>("Launch Configuration Editor", ParentWindowPtr.Pin());

	Editor->OnConfigurationSaved.BindLambda([WeakThis = TWeakPtr<SWidget>(this->AsShared())](ULaunchConfigurationEditor*, const FString& FilePath)
	{
		if (TSharedPtr<SWidget> This = WeakThis.Pin())
		{
			static_cast<SSpatialGDKSimulatedPlayerDeployment*>(This.Get())->OnPrimaryLaunchConfigPathPicked(FilePath);
		}
	}
	);

	return FReply::Handled();
}
