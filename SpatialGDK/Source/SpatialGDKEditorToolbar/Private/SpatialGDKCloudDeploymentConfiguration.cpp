// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKCloudDeploymentConfiguration.h"

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
#include "Utils/LaunchConfigurationEditor.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
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
#include "SpatialGDKDevAuthTokenGenerator.h"
#include "SpatialGDKEditorPackageAssembly.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKEditorSnapshotGenerator.h"
#include "SpatialGDKEditorToolbar.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#include "SpatialGDKSettings.h"

DEFINE_LOG_CATEGORY(LogSpatialGDKCloudDeploymentConfiguration);

#define LOCTEXT_NAMESPACE "SpatialGDKCloudDeploymentConfiguration"

namespace
{
// Build Configurations
const FString DebugConfiguration(TEXT("Debug"));
const FString DebugGameConfiguration(TEXT("DebugGame"));
const FString DevelopmentConfiguration(TEXT("Development"));
const FString TestConfiguration(TEXT("Test"));
const FString ShippingConfiguration(TEXT("Shipping"));
} // anonymous namespace

void SSpatialGDKCloudDeploymentConfiguration::Construct(const FArguments& InArgs)
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	FString ProjectName = FSpatialGDKServicesModule::GetProjectName();
	FSpatialGDKEditorToolbarModule* ToolbarPtr = FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar");

	ParentWindowPtr = InArgs._ParentWindow;
	SpatialGDKEditorPtr = InArgs._SpatialGDKEditor;

	auto AddRequiredFieldAsterisk = [](TSharedRef<SWidget> TextBlock) {
		return SNew(SHorizontalBox) + SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Left)[TextBlock]
			   + SHorizontalBox::Slot()
					 .AutoWidth()
					 .HAlign(HAlign_Left)
					 .Padding(2.0f, 0.0f)[SNew(STextBlock)
											  .Text(LOCTEXT("RequiredFieldAsterisk", "*"))
											  .ToolTipText(LOCTEXT("RequiredField_Tooltip", "Required field"))
											  .ColorAndOpacity(FLinearColor(1.0f, 0.0f, 0.0f))];
	};

	ProjectNameInputErrorReporting = SNew(SPopupErrorText);
	ProjectNameInputErrorReporting->SetError(TEXT(""));
	AssemblyNameInputErrorReporting = SNew(SPopupErrorText);
	AssemblyNameInputErrorReporting->SetError(TEXT(""));
	DeploymentNameInputErrorReporting = SNew(SPopupErrorText);
	DeploymentNameInputErrorReporting->SetError(TEXT(""));
	ChildSlot
		[SNew(SBorder)
			 .HAlign(HAlign_Fill)
			 .BorderImage(FEditorStyle::GetBrush("ChildWindow.Background"))
			 .Padding(4.0f)
				 [SNew(SVerticalBox)
				  + SVerticalBox::Slot().FillHeight(1.0f).Padding(0.0f, 6.0f, 0.0f, 0.0f)
						[SNew(SBorder)
							 .BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
							 .Padding(4.0f)[SNew(SVerticalBox)
											+ SVerticalBox::Slot().AutoHeight().Padding(
												1.0f)[SNew(SVerticalBox)
													  // Project
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox)
																+ SHorizontalBox::Slot().FillWidth(1.0f)[AddRequiredFieldAsterisk(
																	SNew(STextBlock)
																		.Text(LOCTEXT("ProjectName_Label", "Project Name"))
																		.ToolTipText(LOCTEXT("ProjectName_Tooltip",
																							 "The name of the SpatialOS project.")))]
																+ SHorizontalBox::Slot().FillWidth(1.0f)
																	  [SNew(SEditableTextBox)
																		   .Text(FText::FromString(ProjectName))
																		   .ToolTipText(LOCTEXT("ProjectName_Tooltip",
																								"The name of the SpatialOS project."))
																		   .OnTextCommitted(this, &SSpatialGDKCloudDeploymentConfiguration::
																									  OnProjectNameCommitted)
																		   .ErrorReporting(ProjectNameInputErrorReporting)]]
													  // Assembly Name
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox)
																+ SHorizontalBox::Slot().FillWidth(1.0f)[AddRequiredFieldAsterisk(
																	SNew(STextBlock)
																		.Text(LOCTEXT("AssemblyName_Label", "Assembly Name"))
																		.ToolTipText(
																			LOCTEXT("AssemblyName_Tooltip", "The name of the assembly.")))]
																+ SHorizontalBox::Slot().FillWidth(1.0f)
																	  [SNew(SEditableTextBox)
																		   .Text(FText::FromString(SpatialGDKSettings->GetAssemblyName()))
																		   .ToolTipText(
																			   LOCTEXT("AssemblyName_Tooltip", "The name of the assembly."))
																		   .OnTextCommitted(this, &SSpatialGDKCloudDeploymentConfiguration::
																									  OnDeploymentAssemblyCommited)
																		   .ErrorReporting(AssemblyNameInputErrorReporting)]]
													  // RuntimeVersion
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox)
																+ SHorizontalBox::Slot().FillWidth(
																	1.0f)[SNew(STextBlock)
																			  .Text(LOCTEXT("UseGDKPinnedRuntime_Label",
																							"Use GDK Pinned Version For Cloud"))
																			  .ToolTipText(LOCTEXT("UseGDKPinnedRuntime_Tooltip",
																								   "Whether to use the SpatialOS Runtime "
																								   "version associated to the current GDK "
																								   "version for cloud deployments"))]
																+ SHorizontalBox::Slot().FillWidth(
																	1.0f)[SNew(SCheckBox)
																			  .IsChecked(this, &SSpatialGDKCloudDeploymentConfiguration::
																								   IsUsingGDKPinnedRuntimeVersion)
																			  .OnCheckStateChanged(
																				  this, &SSpatialGDKCloudDeploymentConfiguration::
																							OnCheckedUsePinnedVersion)]]
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox)
																+ SHorizontalBox::Slot().FillWidth(
																	1.0f)[SNew(STextBlock)
																			  .Text(LOCTEXT("RuntimeVersion_Label", "Runtime Version"))
																			  .ToolTipText(LOCTEXT("RuntimeVersion_Tooltip",
																								   "User supplied version of the SpatialOS "
																								   "runtime to use"))]
																+ SHorizontalBox::Slot().FillWidth(1.0f)
																	  [SNew(SEditableTextBox)
																		   .Text(this, &SSpatialGDKCloudDeploymentConfiguration::
																						   GetSpatialOSRuntimeVersionToUseText)
																		   .OnTextCommitted(this, &SSpatialGDKCloudDeploymentConfiguration::
																									  OnRuntimeCustomVersionCommited)
																		   .OnTextChanged(this,
																						  &SSpatialGDKCloudDeploymentConfiguration::
																							  OnRuntimeCustomVersionCommited,
																						  ETextCommit::Default)
																		   .IsEnabled(this, &SSpatialGDKCloudDeploymentConfiguration::
																								IsUsingCustomRuntimeVersion)]]
													  // Primary Deployment Name
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox)
																+ SHorizontalBox::Slot().FillWidth(1.0f)[AddRequiredFieldAsterisk(
																	SNew(STextBlock)
																		.Text(LOCTEXT("PrimaryDeploymentName_Label", "Deployment Name"))
																		.ToolTipText(LOCTEXT("PrimaryDeploymentName_Tooltip",
																							 "The name of the cloud deployment. Must be "
																							 "unique.")))]
																+ SHorizontalBox::Slot().FillWidth(1.0f)
																	  [SNew(SEditableTextBox)
																		   .Text(this, &SSpatialGDKCloudDeploymentConfiguration::
																						   GetPrimaryDeploymentNameText)
																		   .ToolTipText(LOCTEXT("PrimaryDeploymentName_Tooltip",
																								"The name of the cloud deployment. Must be "
																								"unique."))
																		   .OnTextCommitted(this, &SSpatialGDKCloudDeploymentConfiguration::
																									  OnPrimaryDeploymentNameCommited)
																		   .ErrorReporting(DeploymentNameInputErrorReporting)]]
													  // Snapshot File + File Picker
													  + SVerticalBox::Slot().AutoHeight().Padding(2.0f)
															[SNew(SHorizontalBox)
															 + SHorizontalBox::Slot().FillWidth(1.0f)[AddRequiredFieldAsterisk(
																 SNew(STextBlock)
																	 .Text(LOCTEXT("SnapshotFile_Label", "Snapshot File"))
																	 .ToolTipText(LOCTEXT("SnapshotFile_Tooltip",
																						  "The relative path to the snapshot file.")))]
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(SFilePathPicker)
																		.BrowseButtonImage(FEditorStyle::GetBrush("PropertyWindow.Button_"
																												  "Ellipsis"))
																		.BrowseButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
																		.BrowseButtonToolTip(LOCTEXT("SnapshotFilePicker_Tooltip",
																									 "Path to the snapshot file."))
																		.BrowseDirectory(
																			SpatialGDKSettings->GetSpatialOSSnapshotFolderPath())
																		.BrowseTitle(LOCTEXT("SnapshotFilePicker_Title", "File picker..."))
																		.FilePath_UObject(SpatialGDKSettings,
																						  &USpatialGDKEditorSettings::GetSnapshotPath)
																		.FileTypeFilter(TEXT("Snapshot files (*.snapshot)|*.snapshot"))
																		.OnPathPicked(this, &SSpatialGDKCloudDeploymentConfiguration::
																								OnSnapshotPathPicked)]]
													  // Automatically Generate Launch Configuration
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox)
																+ SHorizontalBox::Slot().FillWidth(1.0f)
																	  [SNew(STextBlock)
																		   .Text(LOCTEXT("AutoGenerateCloudLaunchConfig_Label",
																						 "Automatically Generate Launch Configuration"))
																		   .ToolTipText(LOCTEXT("AutoGenerateCloudLaunchConfig_Tooltip",
																								"Whether to automatically generate the "
																								"launch configuration from the current map "
																								"when a cloud deployment is started."))]
																+ SHorizontalBox::Slot().FillWidth(
																	1.0f)[SNew(SCheckBox)
																			  .IsChecked(this, &SSpatialGDKCloudDeploymentConfiguration::
																								   IsAutoGenerateCloudLaunchConfigEnabled)
																			  .OnCheckStateChanged(
																				  this, &SSpatialGDKCloudDeploymentConfiguration::
																							OnCheckedAutoGenerateCloudLaunchConfig)]]
													  // Primary Launch Config + File Picker
													  + SVerticalBox::Slot().AutoHeight().Padding(2.0f)
															[SNew(SHorizontalBox)
															 + SHorizontalBox::Slot().FillWidth(1.0f)[AddRequiredFieldAsterisk(
																 SNew(STextBlock)
																	 .Text(LOCTEXT("LaunchConfigFile_Label", "Launch Config File"))
																	 .ToolTipText(LOCTEXT("LaunchConfigFile_Tooltip",
																						  "The relative path to the launch configuration "
																						  "file.")))]
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(SFilePathPicker)
																		.BrowseButtonImage(FEditorStyle::GetBrush("PropertyWindow.Button_"
																												  "Ellipsis"))
																		.BrowseButtonStyle(FEditorStyle::Get(), "HoverHintOnly")
																		.BrowseButtonToolTip(LOCTEXT("LaunchConfigFilePicker_Tooltip",
																									 "Path to the launch configuration "
																									 "file."))
																		.BrowseDirectory(SpatialGDKServicesConstants::SpatialOSDirectory)
																		.BrowseTitle(
																			LOCTEXT("LaunchConfigFilePicker_Title", "File picker..."))
																		.FilePath_UObject(
																			SpatialGDKSettings,
																			&USpatialGDKEditorSettings::GetPrimaryLaunchConfigPath)
																		.FileTypeFilter(TEXT("Launch configuration files (*.json)|*.json"))
																		.OnPathPicked(this, &SSpatialGDKCloudDeploymentConfiguration::
																								OnPrimaryLaunchConfigPathPicked)
																		.IsEnabled(this, &SSpatialGDKCloudDeploymentConfiguration::
																							 CanPickOrEditCloudLaunchConfig)]]
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox) + SHorizontalBox::Slot().FillWidth(1.0f)
																+ SHorizontalBox::Slot().FillWidth(
																	1.0f)[SNew(SButton)
																			  .Text(LOCTEXT("OpenLaunchConfig_Label",
																							"Open Launch Configuration editor"))
																			  .OnClicked(this, &SSpatialGDKCloudDeploymentConfiguration::
																								   OnOpenLaunchConfigEditor)
																			  .IsEnabled(this, &SSpatialGDKCloudDeploymentConfiguration::
																								   CanPickOrEditCloudLaunchConfig)]]
													  // Primary Deployment Region Picker
													  + SVerticalBox::Slot().AutoHeight().Padding(2.0f)
															[SNew(SHorizontalBox)
																 .Visibility(this, &SSpatialGDKCloudDeploymentConfiguration::
																					   GetRegionPickerVisibility)
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(STextBlock)
																		.Text(LOCTEXT("PrimaryDeploymentRegion_Label", "Region"))
																		.ToolTipText(LOCTEXT("PrimaryDeploymentRegion_Tooltip",
																							 "The region in which the deployment will be "
																							 "deployed."))]
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(SComboButton)
																		.OnGetMenuContent(this, &SSpatialGDKCloudDeploymentConfiguration::
																									OnGetPrimaryDeploymentRegionCode)
																		.ContentPadding(FMargin(2.0f, 2.0f))
																		.IsEnabled(this, &SSpatialGDKCloudDeploymentConfiguration::
																							 IsPrimaryRegionPickerEnabled)
																		.ButtonContent()[SNew(STextBlock)
																							 .Text_UObject(SpatialGDKSettings,
																										   &USpatialGDKEditorSettings::
																											   GetPrimaryRegionCodeText)]]]
													  // Main Deployment Cluster
													  + SVerticalBox::Slot().AutoHeight().Padding(2.0f)
															[SNew(SHorizontalBox)
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(STextBlock)
																		.Text(
																			LOCTEXT("PrimaryDeploymentCluster_Label", "Deployment Cluster"))
																		.ToolTipText(LOCTEXT("PrimaryDeploymentCluster_Tooltip",
																							 "The name of the cluster to deploy to. Region "
																							 "code will be ignored if this is specified."))]
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(SEditableTextBox)
																		.Text(FText::FromString(
																			SpatialGDKSettings->GetMainDeploymentCluster()))
																		.ToolTipText(LOCTEXT("PrimaryDeploymentCluster_Tooltip",
																							 "The name of the cluster to deploy to. Region "
																							 "code will be ignored if this is specified."))
																		.OnTextCommitted(this, &SSpatialGDKCloudDeploymentConfiguration::
																								   OnDeploymentClusterCommited)
																		.OnTextChanged(this,
																					   &SSpatialGDKCloudDeploymentConfiguration::
																						   OnDeploymentClusterCommited,
																					   ETextCommit::Default)]]
													  // Deployment Tags
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox)
																+ SHorizontalBox::Slot().FillWidth(
																	1.0f)[SNew(STextBlock)
																			  .Text(LOCTEXT("DeploymentTags_Label", "Deployment Tags"))
																			  .ToolTipText(LOCTEXT("DeploymentTags_Tooltip",
																								   "Tags for the deployment (separated by "
																								   "spaces)."))]
																+ SHorizontalBox::Slot().FillWidth(1.0f)
																	  [SNew(SEditableTextBox)
																		   .Text(FText::FromString(SpatialGDKSettings->GetDeploymentTags()))
																		   .ToolTipText(LOCTEXT("DeploymentTags_Tooltip",
																								"Tags for the deployment (separated by "
																								"spaces)."))
																		   .OnTextCommitted(this, &SSpatialGDKCloudDeploymentConfiguration::
																									  OnDeploymentTagsCommitted)
																		   .OnTextChanged(this,
																						  &SSpatialGDKCloudDeploymentConfiguration::
																							  OnDeploymentTagsCommitted,
																						  ETextCommit::Default)]]
													  // Separator
													  + SVerticalBox::Slot().AutoHeight().Padding(2.0f).VAlign(
														  VAlign_Center)[SNew(SSeparator)]
													  // Explanation text
													  + SVerticalBox::Slot()
															.AutoHeight()
															.Padding(2.0f)
															.VAlign(VAlign_Center)
															.HAlign(HAlign_Center)[SNew(STextBlock)
																					   .Text(LOCTEXT("SimulatedPlayers_Label",
																									 "Simulated Players"))]
													  // Toggle Simulated Players
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox)
																+ SHorizontalBox::Slot().FillWidth(
																	1.0f)[SNew(STextBlock)
																			  .Text(LOCTEXT("EnableSimulatedPlayers_Label",
																							"Add simulated players"))]
																+ SHorizontalBox::Slot().FillWidth(
																	1.0f)[SNew(SCheckBox)
																			  .IsChecked(this, &SSpatialGDKCloudDeploymentConfiguration::
																								   IsSimulatedPlayersEnabled)
																			  .OnCheckStateChanged(
																				  this, &SSpatialGDKCloudDeploymentConfiguration::
																							OnCheckedSimulatedPlayers)]]
													  // Simulated Players Deployment Name
													  + SVerticalBox::Slot().AutoHeight().Padding(2.0f)
															[SNew(SHorizontalBox)
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(STextBlock)
																		.Text(LOCTEXT("SimPlayerDeploymentName_Label", "Deployment Name"))
																		.ToolTipText(LOCTEXT("SimPlayerDeploymentName_Tooltip",
																							 "The name of the simulated player "
																							 "deployment."))]
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(SEditableTextBox)
																		.Text(FText::FromString(
																			SpatialGDKSettings->GetSimulatedPlayerDeploymentName()))
																		.ToolTipText(LOCTEXT("SimPlayerDeploymentName_Tooltip",
																							 "The name of the simulated player "
																							 "deployment."))
																		.OnTextCommitted(this, &SSpatialGDKCloudDeploymentConfiguration::
																								   OnSimulatedPlayerDeploymentNameCommited)
																		.OnTextChanged(this,
																					   &SSpatialGDKCloudDeploymentConfiguration::
																						   OnSimulatedPlayerDeploymentNameCommited,
																					   ETextCommit::Default)
																		.IsEnabled_UObject(
																			SpatialGDKSettings,
																			&USpatialGDKEditorSettings::IsSimulatedPlayersEnabled)]]
													  // Simulated Players Number
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox)
																+ SHorizontalBox::Slot().FillWidth(
																	1.0f)[SNew(STextBlock)
																			  .Text(LOCTEXT("NumberOfSimulatedPlayers_Label",
																							"Number of Simulated Players"))
																			  .ToolTipText(LOCTEXT("NumberOfSimulatedPlayers_Tooltip",
																								   "The number of Simulated Players to "
																								   "launch and connect to the game."))]
																+ SHorizontalBox::Slot().FillWidth(1.0f)
																	  [SNew(SSpinBox<uint32>)
																		   .ToolTipText(LOCTEXT("NumberOfSimulatedPlayers_Tooltip",
																								"The number of Simulated Players to launch "
																								"and connect to the game."))
																		   .MinValue(1)
																		   .MaxValue(8192)
																		   .Value(SpatialGDKSettings->GetNumberOfSimulatedPlayers())
																		   .OnValueChanged(this, &SSpatialGDKCloudDeploymentConfiguration::
																									 OnNumberOfSimulatedPlayersCommited)
																		   .IsEnabled_UObject(
																			   SpatialGDKSettings,
																			   &USpatialGDKEditorSettings::IsSimulatedPlayersEnabled)]]
													  // Simulated Players Deployment Region Picker
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox)
																	.Visibility(this, &SSpatialGDKCloudDeploymentConfiguration::
																						  GetRegionPickerVisibility)
																+ SHorizontalBox::Slot().FillWidth(
																	1.0f)[SNew(STextBlock)
																			  .Text(LOCTEXT("SimPlayerRegion_Label", "Region"))
																			  .ToolTipText(LOCTEXT("SimPlayerRegion_Tooltip",
																								   "The region in which the simulated "
																								   "player deployment will be deployed."))]
																+ SHorizontalBox::Slot().FillWidth(
																	1.0f)[SNew(SComboButton)
																			  .OnGetMenuContent(
																				  this, &SSpatialGDKCloudDeploymentConfiguration::
																							OnGetSimulatedPlayerDeploymentRegionCode)
																			  .ContentPadding(FMargin(2.0f, 2.0f))
																			  .IsEnabled(this, &SSpatialGDKCloudDeploymentConfiguration::
																								   IsSimulatedPlayerRegionPickerEnabled)
																			  .ButtonContent()[SNew(STextBlock)
																								   .Text_UObject(
																									   SpatialGDKSettings,
																									   &USpatialGDKEditorSettings::
																										   GetSimulatedPlayerRegionCode)]]]
													  // Simulated Player Cluster
													  + SVerticalBox::Slot().AutoHeight().Padding(2.0f)
															[SNew(SHorizontalBox)
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(STextBlock)
																		.Text(LOCTEXT("SimPlayerCluster_Label", "Deployment Cluster"))
																		.ToolTipText(LOCTEXT("SimPlayerCluster_Tooltip",
																							 "The name of the cluster to deploy to. Region "
																							 "code will be ignored if this is specified."))]
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(SEditableTextBox)
																		.Text(FText::FromString(
																			SpatialGDKSettings->GetSimulatedPlayerCluster()))
																		.ToolTipText(LOCTEXT("SimPlayerCluster_Tooltip",
																							 "The name of the cluster to deploy to. Region "
																							 "code will be ignored if this is specified."))
																		.OnTextCommitted(this, &SSpatialGDKCloudDeploymentConfiguration::
																								   OnSimulatedPlayerClusterCommited)
																		.OnTextChanged(this,
																					   &SSpatialGDKCloudDeploymentConfiguration::
																						   OnSimulatedPlayerClusterCommited,
																					   ETextCommit::Default)
																		.IsEnabled_UObject(
																			SpatialGDKSettings,
																			&USpatialGDKEditorSettings::IsSimulatedPlayersEnabled)]]
													  // Separator
													  + SVerticalBox::Slot().AutoHeight().Padding(2.0f).VAlign(
														  VAlign_Center)[SNew(SSeparator)]
													  // Explanation text
													  + SVerticalBox::Slot()
															.AutoHeight()
															.Padding(2.0f)
															.VAlign(VAlign_Center)
															.HAlign(HAlign_Center)[SNew(STextBlock)
																					   .Text(LOCTEXT("AssemblyConfiguration_Label",
																									 "Assembly Configuration"))]
													  // Build and Upload Assembly
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox)
																+ SHorizontalBox::Slot().FillWidth(1.0f)
																	  [SNew(STextBlock)
																		   .Text(LOCTEXT("BuildAndUploadAssembly_Label",
																						 "Build and Upload Assembly"))
																		   .ToolTipText(LOCTEXT("BuildAndUploadAssembly_Tooltip",
																								"Whether to build and upload the assembly "
																								"when starting the cloud deployment."))]
																+ SHorizontalBox::Slot().FillWidth(
																	1.0f)[SNew(SCheckBox)
																			  .IsChecked(this, &SSpatialGDKCloudDeploymentConfiguration::
																								   IsBuildAndUploadAssemblyEnabled)
																			  .OnCheckStateChanged(
																				  this, &SSpatialGDKCloudDeploymentConfiguration::
																							OnCheckedBuildAndUploadAssembly)]]
													  // Generate Schema
													  + SVerticalBox::Slot().AutoHeight().Padding(2.0f)
															[SNew(SHorizontalBox)
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(STextBlock)
																		.Text(LOCTEXT("GenerateSchema_Label", "Generate Schema"))
																		.ToolTipText(LOCTEXT("GenerateSchema_Tooltip",
																							 "Whether to generate the schema automatically "
																							 "when building the assembly."))]
															 + SHorizontalBox::Slot().FillWidth(
																 1.0f)[SNew(SCheckBox)
																		   .IsChecked(this, &SSpatialGDKCloudDeploymentConfiguration::
																								IsGenerateSchemaEnabled)
																		   .OnCheckStateChanged(this,
																								&SSpatialGDKCloudDeploymentConfiguration::
																									OnCheckedGenerateSchema)
																		   .IsEnabled_UObject(
																			   SpatialGDKSettings,
																			   &USpatialGDKEditorSettings::ShouldBuildAndUploadAssembly)]]
													  // Generate Snapshot
													  + SVerticalBox::Slot().AutoHeight().Padding(2.0f)
															[SNew(SHorizontalBox)
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(STextBlock)
																		.Text(LOCTEXT("GenerateSnapshot_Label", "Generate Snapshot"))
																		.ToolTipText(LOCTEXT("GenerateSnapshot_Tooltip",
																							 "Whether to generate the snapshot "
																							 "automatically when building the assembly."))]
															 + SHorizontalBox::Slot().FillWidth(
																 1.0f)[SNew(SCheckBox)
																		   .IsChecked(this, &SSpatialGDKCloudDeploymentConfiguration::
																								IsGenerateSnapshotEnabled)
																		   .OnCheckStateChanged(this,
																								&SSpatialGDKCloudDeploymentConfiguration::
																									OnCheckedGenerateSnapshot)
																		   .IsEnabled_UObject(
																			   SpatialGDKSettings,
																			   &USpatialGDKEditorSettings::ShouldBuildAndUploadAssembly)]]
													  // Build Configuration
													  + SVerticalBox::Slot().AutoHeight().Padding(2.0f)
															[SNew(SHorizontalBox)
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(STextBlock)
																		.Text(LOCTEXT("BuildConfiguration_Label", "Build Configuration"))
																		.ToolTipText(LOCTEXT("BuildConfiguration_Tooltip",
																							 "The configuration to build."))]
															 + SHorizontalBox::Slot().FillWidth(1.0f)
																   [SNew(SComboButton)
																		.OnGetMenuContent(this, &SSpatialGDKCloudDeploymentConfiguration::
																									OnGetBuildConfiguration)
																		.ContentPadding(FMargin(2.0f, 2.0f))
																		.ButtonContent()[SNew(STextBlock)
																							 .Text_UObject(
																								 SpatialGDKSettings,
																								 &USpatialGDKEditorSettings::
																									 GetAssemblyBuildConfiguration)]
																		.IsEnabled_UObject(
																			SpatialGDKSettings,
																			&USpatialGDKEditorSettings::ShouldBuildAndUploadAssembly)]]
													  // Enable/Disable Build Client Worker
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox)
																+ SHorizontalBox::Slot().FillWidth(1.0f)
																	  [SNew(STextBlock)
																		   .Text(LOCTEXT("BuildClientWorker_Label", "Build Client Worker"))
																		   .ToolTipText(LOCTEXT("BuildClientWorker_Tooltip",
																								"Whether to build the client worker as "
																								"part of the assembly."))]
																+ SHorizontalBox::Slot().FillWidth(1.0f)
																	  [SNew(SCheckBox)
																		   .IsChecked(this, &SSpatialGDKCloudDeploymentConfiguration::
																								IsBuildClientWorkerEnabled)
																		   .OnCheckStateChanged(this,
																								&SSpatialGDKCloudDeploymentConfiguration::
																									OnCheckedBuildClientWorker)
																		   .IsEnabled_UObject(
																			   SpatialGDKSettings,
																			   &USpatialGDKEditorSettings::ShouldBuildAndUploadAssembly)]]
													  // Force Overwrite on Upload
													  + SVerticalBox::Slot().AutoHeight().Padding(
														  2.0f)[SNew(SHorizontalBox)
																+ SHorizontalBox::Slot().FillWidth(
																	1.0f)[SNew(STextBlock)
																			  .Text(LOCTEXT("ForceOverwriteAssembly_Label",
																							"Force Overwrite on Upload"))
																			  .ToolTipText(LOCTEXT("ForceOverwriteAssembly_Tooltip",
																								   "Whether to overwrite an existing "
																								   "assembly when uploading."))]
																+ SHorizontalBox::Slot().FillWidth(1.0f)
																	  [SNew(SCheckBox)
																		   .IsChecked(this, &SSpatialGDKCloudDeploymentConfiguration::
																								ForceAssemblyOverwrite)
																		   .OnCheckStateChanged(this,
																								&SSpatialGDKCloudDeploymentConfiguration::
																									OnCheckedForceAssemblyOverwrite)
																		   .IsEnabled_UObject(
																			   SpatialGDKSettings,
																			   &USpatialGDKEditorSettings::ShouldBuildAndUploadAssembly)]]
													  // Separator
													  + SVerticalBox::Slot().AutoHeight().Padding(2.0f).VAlign(
														  VAlign_Center)[SNew(SSeparator)]
													  // Buttons
													  + SVerticalBox::Slot().FillHeight(1.0f).Padding(2.0f)
															[SNew(SHorizontalBox)
															 + SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Left)[
																 // Open Deployment Page
																 SNew(SUniformGridPanel).SlotPadding(FMargin(2.0f, 20.0f, 0.0f, 0.0f))
																 + SUniformGridPanel::Slot(0, 0)
																	 [SNew(SButton)
																		  .HAlign(HAlign_Center)
																		  .Text(LOCTEXT("OpenDeploymentPage_Label", "Open Deployment Page"))
																		  .OnClicked(this, &SSpatialGDKCloudDeploymentConfiguration::
																							   OnOpenCloudDeploymentPageClicked)
																		  .IsEnabled(this, &SSpatialGDKCloudDeploymentConfiguration::
																							   CanOpenCloudDeploymentPage)]]
															 + SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Right)[
																 // Start Deployment Button
																 SNew(SUniformGridPanel).SlotPadding(FMargin(2.0f, 20.0f, 0.0f, 0.0f))
																 + SUniformGridPanel::Slot(1, 0)
																	 [SNew(SButton)
																		  .HAlign(HAlign_Center)
																		  .Text(LOCTEXT("StartDeployment_Label", "Start Deployment"))
																		  .OnClicked_Raw(ToolbarPtr, &FSpatialGDKEditorToolbarModule::
																										 OnStartCloudDeployment)
																		  .IsEnabled_Raw(ToolbarPtr, &FSpatialGDKEditorToolbarModule::
																										 CanStartCloudDeployment)]]]]]]]];
}

void SSpatialGDKCloudDeploymentConfiguration::OnDeploymentAssemblyCommited(const FText& InText, ETextCommit::Type InCommitType)
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

FText SSpatialGDKCloudDeploymentConfiguration::GetPrimaryDeploymentNameText() const
{
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	return FText::FromString(SpatialGDKEditorSettings->GetPrimaryDeploymentName());
}

void SSpatialGDKCloudDeploymentConfiguration::OnProjectNameCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	FString NewProjectName = InText.ToString();
	if (!USpatialGDKEditorSettings::IsProjectNameValid(NewProjectName))
	{
		ProjectNameInputErrorReporting->SetError(SpatialConstants::ProjectPatternHint);
		return;
	}
	ProjectNameInputErrorReporting->SetError(TEXT(""));

	if (SpatialGDKEditorPtr.IsValid())
	{
		SpatialGDKEditorPtr.Pin()->SetProjectName(NewProjectName);
	}
}

void SSpatialGDKCloudDeploymentConfiguration::OnPrimaryDeploymentNameCommited(const FText& InText, ETextCommit::Type InCommitType)
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

void SSpatialGDKCloudDeploymentConfiguration::OnCheckedUsePinnedVersion(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetUseGDKPinnedRuntimeVersionForCloud(SpatialGDKSettings->RuntimeVariant,
															  NewCheckedState == ECheckBoxState::Checked);
}

void SSpatialGDKCloudDeploymentConfiguration::OnRuntimeCustomVersionCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetCustomCloudSpatialOSRuntimeVersion(SpatialGDKSettings->RuntimeVariant, InText.ToString());
}

void SSpatialGDKCloudDeploymentConfiguration::OnSnapshotPathPicked(const FString& PickedPath)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetSnapshotPath(PickedPath);
}

void SSpatialGDKCloudDeploymentConfiguration::OnPrimaryLaunchConfigPathPicked(const FString& PickedPath)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetPrimaryLaunchConfigPath(PickedPath);
}

void SSpatialGDKCloudDeploymentConfiguration::OnDeploymentTagsCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetDeploymentTags(InText.ToString());
}

TSharedRef<SWidget> SSpatialGDKCloudDeploymentConfiguration::OnGetPrimaryDeploymentRegionCode()
{
	FMenuBuilder MenuBuilder(true, NULL);
	UEnum* pEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ERegionCode"), true);

	if (pEnum != nullptr)
	{
		for (int32 EnumIdx = 0; EnumIdx < pEnum->NumEnums() - 1; EnumIdx++)
		{
			if (!pEnum->HasMetaData(TEXT("Hidden"), EnumIdx))
			{
				int64 CurrentEnumValue = pEnum->GetValueByIndex(EnumIdx);
				FUIAction ItemAction(FExecuteAction::CreateSP(
					this, &SSpatialGDKCloudDeploymentConfiguration::OnPrimaryDeploymentRegionCodePicked, CurrentEnumValue));
				MenuBuilder.AddMenuEntry(pEnum->GetDisplayNameTextByValue(CurrentEnumValue), TAttribute<FText>(), FSlateIcon(), ItemAction);
			}
		}
	}

	return MenuBuilder.MakeWidget();
}

EVisibility SSpatialGDKCloudDeploymentConfiguration::GetRegionPickerVisibility() const
{
	const USpatialGDKSettings* SpatialGDKSettings = GetDefault<USpatialGDKSettings>();
	return SpatialGDKSettings->IsRunningInChina() ? EVisibility::Collapsed : EVisibility::SelfHitTestInvisible;
}

bool SSpatialGDKCloudDeploymentConfiguration::IsPrimaryRegionPickerEnabled() const
{
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKEditorSettings->GetMainDeploymentCluster().IsEmpty();
}

bool SSpatialGDKCloudDeploymentConfiguration::IsSimulatedPlayerRegionPickerEnabled() const
{
	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKEditorSettings->IsSimulatedPlayersEnabled() && SpatialGDKEditorSettings->GetSimulatedPlayerCluster().IsEmpty();
}

void SSpatialGDKCloudDeploymentConfiguration::OnDeploymentClusterCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetMainDeploymentCluster(InText.ToString());
}

TSharedRef<SWidget> SSpatialGDKCloudDeploymentConfiguration::OnGetSimulatedPlayerDeploymentRegionCode()
{
	FMenuBuilder MenuBuilder(true, NULL);
	UEnum* pEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ERegionCode"), true);

	if (pEnum != nullptr)
	{
		for (int32 EnumIdx = 0; EnumIdx < pEnum->NumEnums() - 1; EnumIdx++)
		{
			if (!pEnum->HasMetaData(TEXT("Hidden"), EnumIdx))
			{
				int64 CurrentEnumValue = pEnum->GetValueByIndex(EnumIdx);
				FUIAction ItemAction(FExecuteAction::CreateSP(
					this, &SSpatialGDKCloudDeploymentConfiguration::OnSimulatedPlayerDeploymentRegionCodePicked, CurrentEnumValue));
				MenuBuilder.AddMenuEntry(pEnum->GetDisplayNameTextByValue(CurrentEnumValue), TAttribute<FText>(), FSlateIcon(), ItemAction);
			}
		}
	}

	return MenuBuilder.MakeWidget();
}

void SSpatialGDKCloudDeploymentConfiguration::OnSimulatedPlayerClusterCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetSimulatedPlayerCluster(InText.ToString());
}

void SSpatialGDKCloudDeploymentConfiguration::OnPrimaryDeploymentRegionCodePicked(const int64 RegionCodeEnumValue)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetPrimaryRegionCode((ERegionCode::Type)RegionCodeEnumValue);
}

void SSpatialGDKCloudDeploymentConfiguration::OnSimulatedPlayerDeploymentRegionCodePicked(const int64 RegionCodeEnumValue)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetSimulatedPlayerRegionCode((ERegionCode::Type)RegionCodeEnumValue);
}

void SSpatialGDKCloudDeploymentConfiguration::OnSimulatedPlayerDeploymentNameCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetSimulatedPlayerDeploymentName(InText.ToString());
}

void SSpatialGDKCloudDeploymentConfiguration::OnNumberOfSimulatedPlayersCommited(uint32 NewValue)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetNumberOfSimulatedPlayers(NewValue);
}

FReply SSpatialGDKCloudDeploymentConfiguration::OnRefreshClicked()
{
	// TODO (UNR-1193): Invoke the Deployment Launcher script to list the deployments
	return FReply::Handled();
}

FReply SSpatialGDKCloudDeploymentConfiguration::OnStopClicked()
{
	if (TSharedPtr<FSpatialGDKEditor> SpatialGDKEditorSharedPtr = SpatialGDKEditorPtr.Pin())
	{
		if (FSpatialGDKEditorToolbarModule* ToolbarPtr =
				FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar"))
		{
			ToolbarPtr->OnShowTaskStartNotification("Stopping cloud deployment ...");
		}

		SpatialGDKEditorSharedPtr->StopCloudDeployment(
			FSimpleDelegate::CreateLambda([]() {
				if (FSpatialGDKEditorToolbarModule* ToolbarPtr =
						FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar"))
				{
					ToolbarPtr->OnShowSuccessNotification("Successfully stopped cloud deployment.");
				}
			}),

			FSimpleDelegate::CreateLambda([]() {
				if (FSpatialGDKEditorToolbarModule* ToolbarPtr =
						FModuleManager::GetModulePtr<FSpatialGDKEditorToolbarModule>("SpatialGDKEditorToolbar"))
				{
					ToolbarPtr->OnShowFailedNotification("Failed to stop cloud deployment.");
				}
			}));
	}
	return FReply::Handled();
}

void SSpatialGDKCloudDeploymentConfiguration::OnCloudDocumentationClicked()
{
	FString WebError;
	FPlatformProcess::LaunchURL(
		TEXT("https://documentation.improbable.io/gdk-for-unreal/docs/cloud-deployment-workflow#section-build-server-worker-assembly"),
		TEXT(""), &WebError);
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

void SSpatialGDKCloudDeploymentConfiguration::OnCheckedSimulatedPlayers(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetSimulatedPlayersEnabledState(NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSpatialGDKCloudDeploymentConfiguration::IsBuildAndUploadAssemblyEnabled() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKSettings->ShouldBuildAndUploadAssembly() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSpatialGDKCloudDeploymentConfiguration::OnCheckedBuildAndUploadAssembly(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetBuildAndUploadAssembly(NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSpatialGDKCloudDeploymentConfiguration::IsSimulatedPlayersEnabled() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKSettings->IsSimulatedPlayersEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

ECheckBoxState SSpatialGDKCloudDeploymentConfiguration::IsUsingGDKPinnedRuntimeVersion() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	const FRuntimeVariantVersion& RuntimeVersion = SpatialGDKSettings->GetSelectedRuntimeVariantVersion();
	return RuntimeVersion.GetUseGDKPinnedRuntimeVersionForCloud() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

bool SSpatialGDKCloudDeploymentConfiguration::IsUsingCustomRuntimeVersion() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	const FRuntimeVariantVersion& RuntimeVersion = SpatialGDKSettings->GetSelectedRuntimeVariantVersion();
	return !RuntimeVersion.GetUseGDKPinnedRuntimeVersionForCloud();
}

FText SSpatialGDKCloudDeploymentConfiguration::GetSpatialOSRuntimeVersionToUseText() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	const FRuntimeVariantVersion& RuntimeVersion = SpatialGDKSettings->GetSelectedRuntimeVariantVersion();
	const FString& RuntimeVersionString = RuntimeVersion.GetVersionForCloud();
	return FText::FromString(RuntimeVersionString);
}

bool SSpatialGDKCloudDeploymentConfiguration::CanPickOrEditCloudLaunchConfig() const
{
	const USpatialGDKEditorSettings* SpatialGKDSettings = GetDefault<USpatialGDKEditorSettings>();
	return !SpatialGKDSettings->ShouldAutoGenerateCloudLaunchConfig();
}

ECheckBoxState SSpatialGDKCloudDeploymentConfiguration::IsAutoGenerateCloudLaunchConfigEnabled() const
{
	const USpatialGDKEditorSettings* SpatialGKDSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGKDSettings->ShouldAutoGenerateCloudLaunchConfig() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSpatialGDKCloudDeploymentConfiguration::OnCheckedAutoGenerateCloudLaunchConfig(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetAutoGenerateCloudLaunchConfigEnabledState(NewCheckedState == ECheckBoxState::Checked);
}

FReply SSpatialGDKCloudDeploymentConfiguration::OnOpenLaunchConfigEditor()
{
	ULaunchConfigurationEditor::OpenModalWindow(ParentWindowPtr.Pin(), [](const FString& FilePath) {
		USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
		SpatialGDKSettings->SetPrimaryLaunchConfigPath(FilePath);
	});

	return FReply::Handled();
}

TSharedRef<SWidget> SSpatialGDKCloudDeploymentConfiguration::OnGetBuildConfiguration()
{
	FMenuBuilder MenuBuilder(true, nullptr);

	MenuBuilder.AddMenuEntry(FText::FromString(DebugConfiguration), TAttribute<FText>(), FSlateIcon(),
							 FUIAction(FExecuteAction::CreateSP(this, &SSpatialGDKCloudDeploymentConfiguration::OnBuildConfigurationPicked,
																DebugConfiguration)));

	MenuBuilder.AddMenuEntry(FText::FromString(DebugGameConfiguration), TAttribute<FText>(), FSlateIcon(),
							 FUIAction(FExecuteAction::CreateSP(this, &SSpatialGDKCloudDeploymentConfiguration::OnBuildConfigurationPicked,
																DebugGameConfiguration)));

	MenuBuilder.AddMenuEntry(FText::FromString(DevelopmentConfiguration), TAttribute<FText>(), FSlateIcon(),
							 FUIAction(FExecuteAction::CreateSP(this, &SSpatialGDKCloudDeploymentConfiguration::OnBuildConfigurationPicked,
																DevelopmentConfiguration)));

	MenuBuilder.AddMenuEntry(
		FText::FromString(TestConfiguration), TAttribute<FText>(), FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &SSpatialGDKCloudDeploymentConfiguration::OnBuildConfigurationPicked, TestConfiguration)));

	MenuBuilder.AddMenuEntry(FText::FromString(ShippingConfiguration), TAttribute<FText>(), FSlateIcon(),
							 FUIAction(FExecuteAction::CreateSP(this, &SSpatialGDKCloudDeploymentConfiguration::OnBuildConfigurationPicked,
																ShippingConfiguration)));

	return MenuBuilder.MakeWidget();
}

void SSpatialGDKCloudDeploymentConfiguration::OnBuildConfigurationPicked(FString Configuration)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetAssemblyBuildConfiguration(Configuration);
}

ECheckBoxState SSpatialGDKCloudDeploymentConfiguration::ForceAssemblyOverwrite() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKSettings->IsForceAssemblyOverwriteEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSpatialGDKCloudDeploymentConfiguration::OnCheckedForceAssemblyOverwrite(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetForceAssemblyOverwrite(NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSpatialGDKCloudDeploymentConfiguration::IsBuildClientWorkerEnabled() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKSettings->IsBuildClientWorkerEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSpatialGDKCloudDeploymentConfiguration::OnCheckedBuildClientWorker(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetBuildClientWorker(NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSpatialGDKCloudDeploymentConfiguration::IsGenerateSchemaEnabled() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKSettings->IsGenerateSchemaEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSpatialGDKCloudDeploymentConfiguration::OnCheckedGenerateSchema(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetGenerateSchema(NewCheckedState == ECheckBoxState::Checked);
}

ECheckBoxState SSpatialGDKCloudDeploymentConfiguration::IsGenerateSnapshotEnabled() const
{
	const USpatialGDKEditorSettings* SpatialGDKSettings = GetDefault<USpatialGDKEditorSettings>();
	return SpatialGDKSettings->IsGenerateSnapshotEnabled() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void SSpatialGDKCloudDeploymentConfiguration::OnCheckedGenerateSnapshot(ECheckBoxState NewCheckedState)
{
	USpatialGDKEditorSettings* SpatialGDKSettings = GetMutableDefault<USpatialGDKEditorSettings>();
	SpatialGDKSettings->SetGenerateSnapshot(NewCheckedState == ECheckBoxState::Checked);
}

FReply SSpatialGDKCloudDeploymentConfiguration::OnOpenCloudDeploymentPageClicked()
{
	FString ProjectName = FSpatialGDKServicesModule::GetProjectName();
	FString ConsoleHost =
		GetDefault<USpatialGDKSettings>()->IsRunningInChina() ? SpatialConstants::CONSOLE_HOST_CN : SpatialConstants::CONSOLE_HOST;
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

bool SSpatialGDKCloudDeploymentConfiguration::CanOpenCloudDeploymentPage() const
{
	return !FSpatialGDKServicesModule::GetProjectName().IsEmpty();
}

#undef LOCTEXT_NAMESPACE
