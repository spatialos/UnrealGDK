// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "SpatialGDKEditorSettings.h"

#include "LaunchConfigurationEditor.generated.h"

class ULaunchConfigurationEditor;

DECLARE_DELEGATE_TwoParams(FOnSpatialOSLaunchConfigurationSaved, ULaunchConfigurationEditor*, const FString&)

UCLASS(Transient, CollapseCategories)
class SPATIALGDKEDITOR_API ULaunchConfigurationEditor : public UObject
{
	GENERATED_BODY()
public:
	FOnSpatialOSLaunchConfigurationSaved OnConfigurationSaved;

	UPROPERTY(EditAnywhere, Category = "Launch Configuration")
	FSpatialLaunchConfigDescription LaunchConfiguration;

	static ULaunchConfigurationEditor* OpenModalWindow(TSharedPtr<SWindow> InParentWindow);
protected:
	void PostInitProperties() override;

	UFUNCTION(Exec)
	void SaveConfiguration();
};
