// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialGDKEditorLayoutDetails.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"
#include "HAL/PlatformFilemanager.h"
#include "Input/Reply.h"
#include "IOSRuntimeSettings.h"
#include "Misc/App.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "SpatialCommandUtils.h"
#include "SpatialGDKSettings.h"
#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKEditorModule.h"
#include "SpatialGDKEditorCommandLineArgsManager.h"
#include "SpatialGDKServicesConstants.h"
#include "SpatialGDKServicesModule.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"

TSharedRef<IDetailCustomization> FSpatialGDKEditorLayoutDetails::MakeInstance()
{
	return MakeShareable(new FSpatialGDKEditorLayoutDetails);
}

void FSpatialGDKEditorLayoutDetails::ForceRefreshLayout()
{
	if (CurrentLayout != nullptr)
	{
		TArray<TWeakObjectPtr<UObject>> Objects;
		CurrentLayout->GetObjectsBeingCustomized(Objects);
		USpatialGDKEditorSettings* Settings = Objects.Num() > 0 ? Cast<USpatialGDKEditorSettings>(Objects[0].Get()) : nullptr;
		if (Settings != nullptr)
		{
			// Force layout to happen in the right order, as delegates may not be ordered.
			Settings->OnWorkerTypesChanged();
		}
		CurrentLayout->ForceRefreshDetails();
	}
}

namespace
{
	FSpatialGDKEditorCommandLineArgsManager& GetCommandLineArgsManager()
	{
		FSpatialGDKEditorModule& EditorModule = FModuleManager::GetModuleChecked<FSpatialGDKEditorModule>("SpatialGDKEditor");
		return EditorModule.GetCommandLineArgsManager();
	}
}

void FSpatialGDKEditorLayoutDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	CurrentLayout = &DetailBuilder;
	const USpatialGDKSettings* GDKSettings = GetDefault<USpatialGDKSettings>();
	GDKSettings->OnWorkerTypesChangedDelegate.AddSP(this, &FSpatialGDKEditorLayoutDetails::ForceRefreshLayout);

	TSharedPtr<IPropertyHandle> UsePinnedVersionProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(USpatialGDKEditorSettings, bUseGDKPinnedRuntimeVersion));

	IDetailPropertyRow* CustomRow = DetailBuilder.EditDefaultProperty(UsePinnedVersionProperty);

	FString PinnedVersionDisplay = FString::Printf(TEXT("GDK Pinned Version : %s"), *SpatialGDKServicesConstants::SpatialOSRuntimePinnedVersion);

	CustomRow->CustomWidget()
		.NameContent()
		[
			UsePinnedVersionProperty->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.HAlign(HAlign_Left)
			.AutoWidth()
			[
				UsePinnedVersionProperty->CreatePropertyValueWidget()
			]
			+SHorizontalBox::Slot()
			.Padding(5)
			.HAlign(HAlign_Center)
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(FText::FromString(PinnedVersionDisplay))
			]
		];

	IDetailCategoryBuilder& CloudConnectionCategory = DetailBuilder.EditCategory("Cloud Connection");
	CloudConnectionCategory.AddCustomRow(FText::FromString("Generate Development Authentication Token"))
		.ValueContent()
		.VAlign(VAlign_Center)
		.MinDesiredWidth(250)
		[
			SNew(SButton)
			.VAlign(VAlign_Center)
			.OnClicked(FOnClicked::CreateLambda([this] {
				return GetCommandLineArgsManager().GenerateDevAuthToken();
			}))
			.Content()
			[
				SNew(STextBlock).Text(FText::FromString("Generate Dev Auth Token"))
			]
		];
		
	IDetailCategoryBuilder& MobileCategory = DetailBuilder.EditCategory("Mobile");
	MobileCategory.AddCustomRow(FText::FromString("Push SpatialOS settings to Android device"))
		.ValueContent()
		.VAlign(VAlign_Center)
		.MinDesiredWidth(550)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateLambda([this] {
					return GetCommandLineArgsManager().PushToAndroidDevice();
				}))
				.Content()
				[
					SNew(STextBlock).Text(FText::FromString("Push SpatialOS settings to Android device"))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateLambda([this] {
					return GetCommandLineArgsManager().RemoveFromAndroidDevice();
				}))
				.Content()
				[
					SNew(STextBlock).Text(FText::FromString("Remove SpatialOS settings from Android device"))
				]
			]
		];

	MobileCategory.AddCustomRow(FText::FromString("Push SpatialOS settings to iOS device"))
		.ValueContent()
		.VAlign(VAlign_Center)
		.MinDesiredWidth(550)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateLambda([this] {
					return GetCommandLineArgsManager().PushToIOSDevice();
				}))
				.Content()
				[
					SNew(STextBlock).Text(FText::FromString("Push SpatialOS settings to iOS device"))
				]
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				SNew(SButton)
				.VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateLambda([this] {
					return GetCommandLineArgsManager().RemoveFromIOSDevice();
				}))
				.Content()
				[
					SNew(STextBlock).Text(FText::FromString("Remove SpatialOS settings from iOS device"))
				]
			]
		];
}
