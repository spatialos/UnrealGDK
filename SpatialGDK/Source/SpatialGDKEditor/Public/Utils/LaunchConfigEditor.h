// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "SpatialGDKEditorSettings.h"
#include "Utils/TransientUObjectEditor.h"

#include "LaunchConfigEditor.generated.h"

UCLASS()
class SPATIALGDKEDITOR_API ULaunchConfigurationEditor : public UTransientUObjectEditor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FSpatialLaunchConfigDescription LaunchConfig;

	UFUNCTION(Exec)
	void SaveConfiguration();
};
