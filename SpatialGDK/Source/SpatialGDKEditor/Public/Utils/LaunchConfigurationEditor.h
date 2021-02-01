// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKEditorSettings.h"

#include "LaunchConfigurationEditor.generated.h"

class ULaunchConfigurationEditor;
class SWindow;

DECLARE_DELEGATE_OneParam(FOnSpatialOSLaunchConfigurationSaved, const FString&)

	UCLASS(Transient, CollapseCategories) class SPATIALGDKEDITOR_API ULaunchConfigurationEditor : public UObject
{
	GENERATED_BODY()
public:
	FOnSpatialOSLaunchConfigurationSaved OnConfigurationSaved;

	UPROPERTY(EditAnywhere, Category = "Launch Configuration")
	FSpatialLaunchConfigDescription LaunchConfiguration;

	/** Tick this if this configuration will be used for cloud deployments. */
	UPROPERTY(EditAnywhere, Category = "Launch Configuration")
	bool bIsCloudConfiguration = true;

	typedef void (*OnLaunchConfigurationSaved)(const FString&);

	static void OpenModalWindow(TSharedPtr<SWindow> InParentWindow, OnLaunchConfigurationSaved InSaved = nullptr);

protected:
	void PostInitProperties() override;

	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	UFUNCTION(Exec)
	void SaveConfiguration();
};
