// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKSimulatedPlayerDeployment.h"

#include "Async/Async.h"
#include "DesktopPlatformModule.h"
#include "Editor.h"
#include "EditorDirectories.h"
#include "EditorStyleSet.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Notifications/NotificationManager.h"
#include "HAL/PlatformFilemanager.h"
#include "InstalledPlatformInfo.h"
#include "Internationalization/Regex.h"
#include "Misc/MessageDialog.h"
#include "Runtime/Launch/Resources/Version.h"
#include "Templates/SharedPointer.h"
#include "Textures/SlateIcon.h"
#include "UnrealEd/Classes/Settings/ProjectPackagingSettings.h"
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
#include "Widgets/Notifications/SPopupErrorText.h"
#include "Widgets/Text/STextBlock.h"

#include "SpatialCommandUtils.h"
#include "SpatialConstants.h"
#include "SpatialGDKDefaultLaunchConfigGenerator.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKEditorToolbar.h"
#include "SpatialGDKEditorPackageAssembly.h"
#include "SpatialGDKEditorSnapshotGenerator.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKSimulatedPlayerDeployment);

namespace
{
	//Build Configurations
	const FString DebugConfiguration(TEXT("Debug"));
	const FString DebugGameConfiguration(TEXT("DebugGame"));
	const FString DevelopmentConfiguration(TEXT("Development"));
	const FString TestConfiguration(TEXT("Test"));
	const FString ShippingConfiguration(TEXT("Shipping"));
} // anonymous namespace

void SSpatialGDKSimulatedPlayerDeployment::Construct(const FArguments& InArgs)
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	FString ProjectName = FSpatialGDKServicesModule::GetProjectName();
	FString AssemblyName = SpatialGDKSettings->GetAssemblyName();
	FString DeploymentName = SpatialGDKSettings->GetPrimaryDeploymentName();
	FSpatialGDKEditorToolbarModule* ToolbarPtr = FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar");

	ParentWindowPtr = InArgs._ParentWindow;
	SpatialGDKEditorPtr = InArgs._SpatialGDKEditor;

	ProjectNameInputErrorReporting = SNew(SPopupErrorText);
	ProjectNameInputErrorReporting->SetError(TEXT(""));
	AssemblyNameInputErrorReporting = SNew(SPopupErrorText);
	AssemblyNameInputErrorReporting->SetError(AssemblyName.IsEmpty() ? SpatialConstants::AssemblyPatternHint : TEXT(""));
	DeploymentNameInputErrorReporting = SNew(SPopupErrorText);
	DeploymentNameInputErrorReporting->SetError(DeploymentName.IsEmpty() ? SpatialConstants::DeploymentPatternHint : TEXT(""));
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
									SNew(SEditableTextBox)
									.Text(FText::FromString(ProjectName))
									.ToolTipText(FText::FromString(FString(TEXT("The name of the SpatialOS project."))))
									.OnTextCommitted(this, &SSpatialGDKSimulatedPlayerDeployment::OnProjectNameCommitted)
									.ErrorReporting(ProjectNameInputErrorReporting)
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
									.Text(FText::FromString(AssemblyName))
									.ToolTipText(FText::FromString(FString(TEXT("The name of the assembly."))))
									.OnTextCommitted(this, &SSpatialGDKSimulatedPlayerDeployment::OnDeploymentAssemblyCommited)
									.ErrorReporting(AssemblyNameInputErrorReporting)
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
									.Text(FText::FromString(DeploymentName))
									.ToolTipText(FText::FromString(FString(TEXT("The name of the cloud deployment. Must be unique."))))
									.OnTextCommitted(this, &SSpatialGDKSimulatedPlayerDeployment::OnPrimaryDeploymentNameCommited)
									.ErrorReporting(DeploymentNameInputErrorReporting)
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
							// Deployment Tags
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Deployment Tags"))))
									.ToolTipText(FText::FromString(FString(TEXT("Tags for the deployment (separated by spaces)."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SEditableTextBox)
									.Text(FText::FromString(SpatialGDKSettings->GetDeploymentTags()))
									.ToolTipText(FText::FromString(FString(TEXT("Tags for the deployment (separated by spaces)."))))
									.OnTextCommitted(this, &SSpatialGDKSimulatedPlayerDeployment::OnDeploymentTagsCommitted)
									.OnTextChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnDeploymentTagsCommitted, ETextCommit::Default)
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
							// Toggle Simulated Players
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Add simulated players"))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SSpatialGDKSimulatedPlayerDeployment::IsSimulatedPlayersEnabled)
									.OnCheckStateChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnCheckedSimulatedPlayers)
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
									.Value(SpatialGDKSettings->GetNumberOfSimulatedPlayers())
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
								.Text(FText::FromString(FString(TEXT("Build and Upload Assembly"))))
							]
							// Generate Schema
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Generate Schema"))))
									.ToolTipText(FText::FromString(FString(TEXT("Whether to generate the schema automatically when building the assembly."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SSpatialGDKSimulatedPlayerDeployment::IsGenerateSchemaEnabled)
									.OnCheckStateChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnCheckedGenerateSchema)
								]
							]
							// Generate Snapshot
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Generate Snapshot"))))
									.ToolTipText(FText::FromString(FString(TEXT("Whether to generate the snapshot automatically when building the assembly."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SSpatialGDKSimulatedPlayerDeployment::IsGenerateSnapshotEnabled)
									.OnCheckStateChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnCheckedGenerateSnapshot)
								]
							]
							// Build Configuration
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Build Configuration"))))
									.ToolTipText(FText::FromString(FString(TEXT("The configuration to build."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SComboButton)
									.OnGetMenuContent(this, &SSpatialGDKSimulatedPlayerDeployment::OnGetBuildConfiguration)
									.ContentPadding(FMargin(2.0f, 2.0f))
									.ButtonContent()
									[
										SNew(STextBlock)
										.Text_UObject(SpatialGDKSettings, &USpatialGDKEditorSettings::GetAssemblyBuildConfiguration)
									]
								]
							]
							// Enable/Disable Build Client Worker
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Build Client Worker"))))
									.ToolTipText(FText::FromString(FString(TEXT("Whether to build the client worker as part of the assembly."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SSpatialGDKSimulatedPlayerDeployment::IsBuildClientWorkerEnabled)
									.OnCheckStateChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnCheckedBuildClientWorker)
								]
							]
							// Force Overwrite on Upload
							+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(STextBlock)
									.Text(FText::FromString(FString(TEXT("Force Overwrite on Upload"))))
									.ToolTipText(FText::FromString(FString(TEXT("Whether to overwrite an existing assembly when uploading."))))
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								[
									SNew(SCheckBox)
									.IsChecked(this, &SSpatialGDKSimulatedPlayerDeployment::ForceAssemblyOverwrite)
									.OnCheckStateChanged(this, &SSpatialGDKSimulatedPlayerDeployment::OnCheckedForceAssemblyOverwrite)
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
							// Buttons
							+ SVerticalBox::Slot()
							.FillHeight(1.0f)
							.Padding(2.0f)
							[
								SNew(SHorizontalBox)
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.HAlign(HAlign_Left)
								[
									// Open Deployment Page
									SNew(SUniformGridPanel)
									.SlotPadding(FMargin(2.0f, 20.0f, 0.0f, 0.0f))
									+ SUniformGridPanel::Slot(0, 0)
									[
										SNew(SButton)
										.HAlign(HAlign_Center)
										.Text(FText::FromString(FString(TEXT("Open Deployment Page"))))
										.OnClicked(this, &SSpatialGDKSimulatedPlayerDeployment::OnOpenCloudDeploymentPageClicked)
										.IsEnabled(this, &SSpatialGDKSimulatedPlayerDeployment::CanOpenCloudDeploymentPage)
									]
								]
								+ SHorizontalBox::Slot()
								.FillWidth(1.0f)
								.HAlign(HAlign_Right)
								[
									// Launch Deployment Button
									SNew(SUniformGridPanel)
									.SlotPadding(FMargin(2.0f, 20.0f, 0.0f, 0.0f))
									+ SUniformGridPanel::Slot(1, 0)
									[
										SNew(SButton)
										.HAlign(HAlign_Center)
										.Text(FText::FromString(FString(TEXT("Launch Deployment"))))
										.OnClicked_Raw(ToolbarPtr, &FSpatialGDKEditorToolbarModule::OnLaunchCloudDeployment)
										.IsEnabled_Raw(ToolbarPtr, &FSpatialGDKEditorToolbarModule::CanLaunchCloudDeployment)
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
	const FString& InputAssemblyName = InText.ToString();
	if (!USpatialGDKEditorSettings::IsAssemblyNameValid(InputAssemblyName))
	{
		AssemblyNameInputErrorReporting->SetError(SpatialConstants::AssemblyPatternHint);
		return;
	}
	AssemblyNameInputErrorReporting->SetError(TEXT(""));

	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetAssemblyName(InputAssemblyName);
}

void SSpatialGDKSimulatedPlayerDeployment::OnProjectNameCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	FString NewProjectName = InText.ToString();
	if (!USpatialGDKEditorSettings::IsProjectNameValid(NewProjectName))
	{
		ProjectNameInputErrorReporting->SetError(SpatialConstants::ProjectPatternHint);
		return;
	}
	ProjectNameInputErrorReporting->SetError(TEXT(""));

	FSpatialGDKServicesModule::SetProjectName(NewProjectName);
}

void SSpatialGDKSimulatedPlayerDeployment::OnPrimaryDeploymentNameCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	const FString& InputDeploymentName = InText.ToString();
	if (!USpatialGDKEditorSettings::IsDeploymentNameValid(InputDeploymentName))
	{
		DeploymentNameInputErrorReporting->SetError(SpatialConstants::DeploymentPatternHint);
		return;
	}
	DeploymentNameInputErrorReporting->SetError(TEXT(""));
	
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetPrimaryDeploymentName(InputDeploymentName);
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

void SSpatialGDKSimulatedPlayerDeployment::OnDeploymentTagsCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetDeploymentTags(InText.ToString());
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

TSharedRef<SWidget> SSpatialGDKSimulatedPlayerDeployment::OnGetBuildConfiguration()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(FText::FromString(DebugConfiguration), TAttribute<FText>(), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SSpatialGDKSimulatedPlayerDeployment::OnBuildConfigurationPicked, DebugConfiguration))
	);

	MenuBuilder.AddMenuEntry(FText::FromString(DebugGameConfiguration), TAttribute<FText>(), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SSpatialGDKSimulatedPlayerDeployment::OnBuildConfigurationPicked, DebugGameConfiguration))
	);

	MenuBuilder.AddMenuEntry(FText::FromString(DevelopmentConfiguration), TAttribute<FText>(), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SSpatialGDKSimulatedPlayerDeployment::OnBuildConfigurationPicked, DevelopmentConfiguration))
	);

	MenuBuilder.AddMenuEntry(FText::FromString(TestConfiguration), TAttribute<FText>(), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SSpatialGDKSimulatedPlayerDeployment::OnBuildConfigurationPicked, TestConfiguration))
	);

	MenuBuilder.AddMenuEntry(FText::FromString(ShippingConfiguration), TAttribute<FText>(), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SSpatialGDKSimulatedPlayerDeployment::OnBuildConfigurationPicked, ShippingConfiguration))
	);

	return MenuBuilder.MakeWidget();
}

void SSpatialGDKSimulatedPlayerDeployment::OnBuildConfigurationPicked(FString Configuration)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetAssemblyBuildConfiguration(Configuration);
}

ECheckBoxState SSpatialGDKSimulatedPlayerDeployment::ForceAssemblyOverwrite() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKSettings->IsForceAssemblyOverwriteEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSpatialGDKSimulatedPlayerDeployment::OnCheckedForceAssemblyOverwrite(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetForceAssemblyOverwrite(NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSpatialGDKSimulatedPlayerDeployment::IsBuildClientWorkerEnabled() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKSettings->IsBuildClientWorkerEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSpatialGDKSimulatedPlayerDeployment::OnCheckedBuildClientWorker(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetBuildClientWorker(NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSpatialGDKSimulatedPlayerDeployment::IsGenerateSchemaEnabled() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKSettings->IsGenerateSchemaEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSpatialGDKSimulatedPlayerDeployment::OnCheckedGenerateSchema(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetGenerateSchema(NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSpatialGDKSimulatedPlayerDeployment::IsGenerateSnapshotEnabled() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKSettings->IsGenerateSnapshotEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSpatialGDKSimulatedPlayerDeployment::OnCheckedGenerateSnapshot(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetGenerateSnapshot(NewCheckedState == ECheckBoxState::Checked);
}

FReply SSpatialGDKSimulatedPlayerDeployment::OnOpenCloudDeploymentPageClicked()
{
	FString ProjectName = FSpatialGDKServicesModule::GetProjectName();
	FString ConsoleHost = GetDefault<USpatialGDKSettings>()->IsRunningInChina() ? SpatialConstants::CONSOLE_HOST_CN : SpatialConstants::CONSOLE_HOST;
	FString Url = FString::Printf(TEXT("https://%s/projects/%s"), *ConsoleHost, *ProjectName);

	FString WebError;
	FPlatformProcess::LaunchURL(*Url, TEXT(""), &WebError);
	if (!WebError.IsEmpty())
	{
		FNotificationInfo Info(FText::FromString(WebError));
		Info.ExpireDuration = 3.0f;
		Info.bUseSuccessFailIcons = true;
		TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
		NotificationItem->SetCompletionState(SNotificationItem::CS_Fail);
		NotificationItem->ExpireAndFadeout();
		return FReply::Unhandled();
	}

	return FReply::Handled();
}

bool SSpatialGDKSimulatedPlayerDeployment::CanOpenCloudDeploymentPage() const
{
	return !FSpatialGDKServicesModule::GetProjectName().IsEmpty();
}
