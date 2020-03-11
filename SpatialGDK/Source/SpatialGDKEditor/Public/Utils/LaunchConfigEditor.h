// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#pragma once

#include "Utils/TransientUObjectEditor.h"
#include "SpatialGDKEditorSettings.h"

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
