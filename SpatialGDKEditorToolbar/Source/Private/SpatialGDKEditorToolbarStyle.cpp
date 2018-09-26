// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorToolbarStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "IPluginManager.h"
#include "SlateGameResources.h"
#include "SpatialGDKEditorToolbar.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FSpatialGDKEditorToolbarStyle::StyleInstance = NULL;

void FSpatialGDKEditorToolbarStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FSpatialGDKEditorToolbarStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FSpatialGDKEditorToolbarStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("SpatialGDKEditorToolbarStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH(RelativePath, ...) \
  FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

namespace
{
const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
}

TSharedRef<FSlateStyleSet> FSpatialGDKEditorToolbarStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style =
		MakeShareable(new FSlateStyleSet("SpatialGDKEditorToolbarStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("SpatialGDKEditorToolbar")->GetBaseDir() /
		TEXT("Resources"));

	Style->Set("SpatialGDKEditorToolbar.CreateSpatialGDKSnapshot",
		new IMAGE_BRUSH(TEXT("Snapshot"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.CreateSpatialGDKSnapshot.Small",
		new IMAGE_BRUSH(TEXT("Snapshot@0.5x"), Icon20x20));

	Style->Set("SpatialGDKEditorToolbar.GenerateSchema",
		new IMAGE_BRUSH(TEXT("Schema"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.GenerateSchema.Small",
		new IMAGE_BRUSH(TEXT("Schema@0.5x"), Icon20x20));

	Style->Set("SpatialGDKEditorToolbar.StartSpatialOSStackAction",
		new IMAGE_BRUSH(TEXT("Launch"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.StartSpatialOSStackAction.Small",
		new IMAGE_BRUSH(TEXT("Launch@0.5x"), Icon20x20));
	
	Style->Set("SpatialGDKEditorToolbar.StopSpatialOSStackAction",
		new IMAGE_BRUSH(TEXT("Stop"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.StopSpatialOSStackAction.Small",
		new IMAGE_BRUSH(TEXT("Stop@0.5x"), Icon20x20));
	
	Style->Set("SpatialGDKEditorToolbar.LaunchInspectorWebPageAction",
		new IMAGE_BRUSH(TEXT("Inspector"), Icon40x40));
	
	Style->Set("SpatialGDKEditorToolbar.LaunchInspectorWebPageAction.Small",
		new IMAGE_BRUSH(TEXT("Inspector@0.5x"), Icon20x20));

	return Style;
}

#undef IMAGE_BRUSH

void FSpatialGDKEditorToolbarStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FSpatialGDKEditorToolbarStyle::Get()
{
	return *StyleInstance;
}
