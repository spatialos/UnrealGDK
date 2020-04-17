// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Input/Reply.h"
#include "Layout/Visibility.h"
#include "SpatialGDKEditor.h"
#include "SpatialGDKEditorSettings.h"
#include "Templates/SharedPointer.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SCompoundWidget.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKSimulatedPlayerDeployment, Log, All);

class SWindow;

enum class ECheckBoxState : uint8;

class SSpatialGDKSimulatedPlayerDeployment : public SCompoundWidget
{
public:

	SLATE_BEGIN_ARGS(SSpatialGDKSimulatedPlayerDeployment) {}

	/** A reference to the parent window */
	SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)
	SLATE_ARGUMENT(TSharedPtr<FSpatialGDKEditor>, SpatialGDKEditor)

	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs);

private:

	/** The parent window of this widget */
	TWeakPtr<SWindow> ParentWindowPtr;

	/** Pointer to the SpatialGDK editor */
	TWeakPtr<FSpatialGDKEditor> SpatialGDKEditorPtr;

	TFuture<bool> AttemptSpatialAuthResult;

	/** Delegate to commit project name */
	void OnProjectNameCommitted(const FText& InText, ETextCommit::Type InCommitType);

	/** Delegate to commit assembly name */
	void OnDeploymentAssemblyCommited(const FText& InText, ETextCommit::Type InCommitType);

	/** Delegate to commit primary deployment name */
	void OnPrimaryDeploymentNameCommited(const FText& InText, ETextCommit::Type InCommitType);

	/** Delegate called when the user clicks the GDK Pinned Version checkbox */
	void OnCheckedUsePinnedVersion(ECheckBoxState NewCheckedState);

	/** Delegate to commit runtime version */
	void OnRuntimeCustomVersionCommited(const FText& InText, ETextCommit::Type InCommitType);

	/** Delegate called when the user has picked a path for the snapshot file */
	void OnSnapshotPathPicked(const FString& PickedPath);

	/** Delegate called when the user has picked a path for the primary launch configuration file */
	void OnPrimaryLaunchConfigPathPicked(const FString& PickedPath);

	/** Delegate to commit deployment tags */
	void OnDeploymentTagsCommitted(const FText& InText, ETextCommit::Type InCommitType);

	/** Delegate called to populate the region codes for the primary deployment */
	TSharedRef<SWidget> OnGetPrimaryDeploymentRegionCode();

	/** Delegate called to populate the region codes for the simulated player deployment */
	TSharedRef<SWidget> OnGetSimulatedPlayerDeploymentRegionCode();

	/** Delegate called when the user selects a region code from the dropdown for the primary deployment */
	void OnPrimaryDeploymentRegionCodePicked(const int64 RegionCodeEnumValue);

	/** Delegate called when the user selects a region code from the dropdown for the simulated player deployment */
	void OnSimulatedPlayerDeploymentRegionCodePicked(const int64 RegionCodeEnumValue);

	/** Delegate to commit simulated player deployment name */
	void OnSimulatedPlayerDeploymentNameCommited(const FText& InText, ETextCommit::Type InCommitType);

	/** Delegate to commit the number of Simulated Players */
	void OnNumberOfSimulatedPlayersCommited(uint32 NewValue);

	/** Delegate called when the user clicks the 'Launch Simulated Player Deployment' button */
	FReply OnLaunchClicked();

	/** Delegate called when the user clicks the 'Refresh' button */
	FReply OnRefreshClicked();

	/** Delegate called when the user clicks the 'Stop Deployment' button */
	FReply OnStopClicked();

	/** Delegate called when the user clicks the cloud deployment documentation */
	void OnCloudDocumentationClicked();

	/** Delegate called when the user either clicks the simulated players checkbox */
	void OnCheckedSimulatedPlayers(ECheckBoxState NewCheckedState);

	TSharedRef<SWidget> OnGetBuildWindowsPlatform();
	void OnWindowsPlatformPicked(FString WindowsPlatform);
	TSharedRef<SWidget> OnGetBuildConfiguration();
	void OnBuildConfigurationPicked(FString Configuration);

	FReply OnBuildAndUploadClicked();
	bool CanBuildAndUpload() const;

	ECheckBoxState ForceAssemblyOverwrite() const;
	void OnCheckedForceAssemblyOverwrite(ECheckBoxState NewCheckedState);

	ECheckBoxState IsSimulatedPlayersEnabled() const;
	ECheckBoxState IsUsingGDKPinnedRuntimeVersion() const;
	bool IsUsingCustomRuntimeVersion() const;
	FText GetSpatialOSRuntimeVersionToUseText() const;

	/** Delegate to determine the 'Launch Deployment' button enabled state */
	bool IsDeploymentConfigurationValid() const;

	ECheckBoxState IsBuildClientWorkerEnabled() const;
	void OnCheckedBuildClientWorker(ECheckBoxState NewCheckedState);

	ECheckBoxState IsGenerateSchemaEnabled() const;
	void OnCheckedGenerateSchema(ECheckBoxState NewCheckedState);

	ECheckBoxState IsGenerateSnapshotEnabled() const;
	void OnCheckedGenerateSnapshot(ECheckBoxState NewCheckedState);

	void OnBuildSuccess();

	bool CanLaunchDeployment() const;
};
