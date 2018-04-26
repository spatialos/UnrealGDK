// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialOSEditorToolbarStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "IPluginManager.h"
#include "SlateGameResources.h"
#include "SpatialOSEditorToolbar.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FSpatialOSEditorToolbarStyle::StyleInstance = NULL;

void FSpatialOSEditorToolbarStyle::Initialize()
{
  if (!StyleInstance.IsValid())
  {
	StyleInstance = Create();
	FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
  }
}

void FSpatialOSEditorToolbarStyle::Shutdown()
{
  FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
  ensure(StyleInstance.IsUnique());
  StyleInstance.Reset();
}

FName FSpatialOSEditorToolbarStyle::GetStyleSetName()
{
  static FName StyleSetName(TEXT("SpatialOSEditorToolbarStyle"));
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

TSharedRef<FSlateStyleSet> FSpatialOSEditorToolbarStyle::Create()
{
  TSharedRef<FSlateStyleSet> Style =
	  MakeShareable(new FSlateStyleSet("SpatialOSEditorToolbarStyle"));
  Style->SetContentRoot(IPluginManager::Get().FindPlugin("SpatialOSEditorToolbar")->GetBaseDir() /
						TEXT("Resources"));

  Style->Set("SpatialOSEditorToolbar.StartSpatialOSStackAction",
			 new IMAGE_BRUSH(TEXT("ImprobableIcon_40x"), Icon40x40));
  Style->Set("SpatialOSEditorToolbar.StopSpatialOSStackAction",
			 new IMAGE_BRUSH(TEXT("ImprobableIconStop_40x"), Icon40x40));
  Style->Set("SpatialOSEditorToolbar.LaunchInspectorWebPageAction",
			 new IMAGE_BRUSH(TEXT("BrowserIcon_40x"), Icon40x40));

  Style->Set("SpatialOSEditorToolbar.StartSpatialOSStackAction.Small",
			 new IMAGE_BRUSH(TEXT("ImprobableIcon_40x"), Icon20x20));
  Style->Set("SpatialOSEditorToolbar.StopSpatialOSStackAction.Small",
			 new IMAGE_BRUSH(TEXT("ImprobableIconStop_40x"), Icon20x20));
  Style->Set("SpatialOSEditorToolbar.LaunchInspectorWebPageAction.Small",
			 new IMAGE_BRUSH(TEXT("BrowserIcon_40x"), Icon20x20));

  return Style;
}

#undef IMAGE_BRUSH

void FSpatialOSEditorToolbarStyle::ReloadTextures()
{
  if (FSlateApplication::IsInitialized())
  {
	FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
  }
}

const ISlateStyle& FSpatialOSEditorToolbarStyle::Get()
{
  return *StyleInstance;
}
