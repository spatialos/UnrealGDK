// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "NUFEditorToolbarStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "IPluginManager.h"
#include "SlateGameResources.h"
#include "NUFEditorToolbar.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr<FSlateStyleSet> FNUFEditorToolbarStyle::StyleInstance = NULL;

void FNUFEditorToolbarStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FNUFEditorToolbarStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FNUFEditorToolbarStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("NUFEditorToolbarStyle"));
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

TSharedRef<FSlateStyleSet> FNUFEditorToolbarStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style =
		MakeShareable(new FSlateStyleSet("NUFEditorToolbarStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("NUFEditorToolbar")->GetBaseDir() /
		TEXT("Resources"));

	Style->Set("NUFEditorToolbar.CreateNUFSnapshot",
		new IMAGE_BRUSH(TEXT("CreateSnapshotIcon"), Icon40x40));

	Style->Set("NUFEditorToolbar.CreateNUFSnapshot.Small",
		new IMAGE_BRUSH(TEXT("CreateSnapshotIcon"), Icon20x20));

	return Style;
}

#undef IMAGE_BRUSH

void FNUFEditorToolbarStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FNUFEditorToolbarStyle::Get()
{
	return *StyleInstance;
}
