// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/LaunchConfigEditorLayoutDetails.h"

#include "SpatialGDKSettings.h"
#include "Utils/LaunchConfigEditor.h"
#include "DetailLayoutBuilder.h"

TSharedRef<IDetailCustomization> FLaunchConfigEditorLayoutDetails::MakeInstance()
{
	return MakeShareable(new FLaunchConfigEditorLayoutDetails);
}

void FLaunchConfigEditorLayoutDetails::ForceRefreshLayout()
{
	if (MyLayout != nullptr)
	{
		TArray<TWeakObjectPtr<UObject>> Objects;
		MyLayout->GetObjectsBeingCustomized(Objects);
		ULaunchConfigurationEditor* Editor = Objects.Num() > 0 ? Cast<ULaunchConfigurationEditor>(Objects[0].Get()) : nullptr;
		if (Editor != nullptr)
		{
			Editor->OnWorkerTypesChanged();
		}
		MyLayout->ForceRefreshDetails();
	}
}

void FLaunchConfigEditorLayoutDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	MyLayout = &DetailBuilder;
	const USpatialGDKSettings* GDKSettings = GetDefault<USpatialGDKSettings>();
	GDKSettings->OnWorkerTypesChangedDelegate.AddSP(this, &FLaunchConfigEditorLayoutDetails::ForceRefreshLayout);
}
