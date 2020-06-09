// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/LaunchConfigEditorLayoutDetails.h"

#include "SpatialGDKSettings.h"
#include "Utils/LaunchConfigurationEditor.h"
#include "DetailLayoutBuilder.h"

TSharedRef<IDetailCustomization> FLaunchConfigEditorLayoutDetails::MakeInstance()
{
	return MakeShareable(new FLaunchConfigEditorLayoutDetails);
}

void FLaunchConfigEditorLayoutDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	MyLayout = &DetailBuilder;
	const USpatialGDKSettings* GDKSettings = GetDefault<USpatialGDKSettings>();
}
