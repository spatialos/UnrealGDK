// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorToolbarStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"
#include "Slate/SlateGameResources.h"
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

#define IMAGE_BRUSH(RelativePath, ...) FSlateImageBrush(Style->RootToContentDir(RelativePath, TEXT(".png")), __VA_ARGS__)

namespace
{
const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);
const FVector2D Icon100x22(100.0f, 22.0f);
} // namespace

TSharedRef<FSlateStyleSet> FSpatialGDKEditorToolbarStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("SpatialGDKEditorToolbarStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("SpatialGDK")->GetBaseDir() / TEXT("Resources"));

	Style->Set("SpatialGDKEditorToolbar.CreateSpatialGDKSnapshot", new IMAGE_BRUSH(TEXT("Snapshot"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.CreateSpatialGDKSnapshot.Small", new IMAGE_BRUSH(TEXT("Snapshot@0.5x"), Icon20x20));

	Style->Set("SpatialGDKEditorToolbar.CreateSpatialGDKSchema", new IMAGE_BRUSH(TEXT("Schema"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.CreateSpatialGDKSchema.Small", new IMAGE_BRUSH(TEXT("Schema@0.5x"), Icon20x20));

	Style->Set("SpatialGDKEditorToolbar.StartNative", new IMAGE_BRUSH(TEXT("None"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.StartNative.Small", new IMAGE_BRUSH(TEXT("None@0.5x"), Icon20x20));

	Style->Set("SpatialGDKEditorToolbar.StartLocalSpatialDeployment", new IMAGE_BRUSH(TEXT("StartLocal"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.StartLocalSpatialDeployment.Small", new IMAGE_BRUSH(TEXT("StartLocal@0.5x"), Icon20x20));

	Style->Set("SpatialGDKEditorToolbar.StartCloudSpatialDeployment", new IMAGE_BRUSH(TEXT("StartCloud"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.StartCloudSpatialDeployment.Small", new IMAGE_BRUSH(TEXT("StartCloud@0.5x"), Icon20x20));

	Style->Set("SpatialGDKEditorToolbar.StopSpatialDeployment", new IMAGE_BRUSH(TEXT("StopLocal"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.StopSpatialDeployment.Small", new IMAGE_BRUSH(TEXT("StopLocal@0.5x"), Icon20x20));

	Style->Set("SpatialGDKEditorToolbar.LaunchInspectorWebPageAction", new IMAGE_BRUSH(TEXT("Inspector"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.LaunchInspectorWebPageAction.Small", new IMAGE_BRUSH(TEXT("Inspector@0.5x"), Icon20x20));

	Style->Set("SpatialGDKEditorToolbar.OpenCloudDeploymentWindowAction", new IMAGE_BRUSH(TEXT("Cloud"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.OpenCloudDeploymentWindowAction.Small", new IMAGE_BRUSH(TEXT("Cloud@0.5x"), Icon20x20));

	Style->Set("SpatialGDKEditorToolbar.StartSpatialService", new IMAGE_BRUSH(TEXT("StartLocal"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.StartSpatialService.Small", new IMAGE_BRUSH(TEXT("StartLocal@0.5x"), Icon20x20));

	Style->Set("SpatialGDKEditorToolbar.StopSpatialService", new IMAGE_BRUSH(TEXT("StopLocal"), Icon40x40));

	Style->Set("SpatialGDKEditorToolbar.StopSpatialService.Small", new IMAGE_BRUSH(TEXT("StopLocal@0.5x"), Icon20x20));

	Style->Set("SpatialGDKEditorToolbar.SpatialOSLogo", new IMAGE_BRUSH(TEXT("SPATIALOS_LOGO_WHITE"), Icon100x22));

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
