// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialGDKEditorSettings.h"
#include "Utils/TransientUObjectEditor.h"

#include "LaunchConfigEditor.generated.h"

class ULaunchConfigurationEditor;

DECLARE_DELEGATE_TwoParams(FOnSpatialOSLaunchConfigurationSaved, ULaunchConfigurationEditor*, const FString&)

class UAbstractRuntimeLoadBalancingStrategy;

UCLASS(Transient, CollapseCategories)
class SPATIALGDKEDITOR_API ULaunchConfigurationEditor : public UTransientUObjectEditor
{
	GENERATED_BODY()

public:
	FOnSpatialOSLaunchConfigurationSaved OnConfigurationSaved;

	void OnWorkerTypesChanged();

protected:
	void PostInitProperties() override;

	UPROPERTY(EditAnywhere, Category = "Launch Configuration")
	FSpatialLaunchConfigDescription LaunchConfiguration;

	UFUNCTION(Exec)
	void SaveConfiguration();
};
