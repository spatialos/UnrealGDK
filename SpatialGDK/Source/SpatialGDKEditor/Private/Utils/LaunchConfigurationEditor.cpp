// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/LaunchConfigurationEditor.h"

#include "DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"
#include "IDesktopPlatform.h"
#include "MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "PropertyEditor/Public/PropertyEditorModule.h"
#include "SpatialGDKSettings.h"
#include "SpatialGDKDefaultLaunchConfigGenerator.h"
#include "SpatialRuntimeLoadBalancingStrategies.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"

TSharedPtr<ULaunchConfigurationEditor> ULaunchConfigurationEditor::_instance = nullptr;

void ULaunchConfigurationEditor::PostInitProperties()
{
	Super::PostInitProperties();

	const USpatialGDKEditorSettings* SpatialGDKEditorSettings = GetDefault<USpatialGDKEditorSettings>();

	LaunchConfiguration = SpatialGDKEditorSettings->LaunchConfigDesc;
	FillWorkerConfigurationFromCurrentMap(LaunchConfiguration.ServerWorkerConfig, LaunchConfiguration.World.Dimensions);
}

void ULaunchConfigurationEditor::SaveConfiguration()
{
	if (!ValidateGeneratedLaunchConfig(LaunchConfiguration, LaunchConfiguration.ServerWorkerConfig))
	{
		return;
	}

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();

	FString DefaultOutPath = SpatialGDKServicesConstants::SpatialOSDirectory;
	TArray<FString> Filenames;

	bool bSaved = DesktopPlatform->SaveFileDialog(
		FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
		TEXT("Save launch configuration"),
		DefaultOutPath,
		TEXT(""),
		TEXT("JSON Configuration|*.json"),
		EFileDialogFlags::None,
		Filenames);

	if (bSaved && Filenames.Num() > 0)
	{
		if (GenerateLaunchConfig(Filenames[0], &LaunchConfiguration, LaunchConfiguration.ServerWorkerConfig))
		{
			OnConfigurationSaved.ExecuteIfBound(this, Filenames[0]);
		}
	}
}

namespace
{
	// Copied from FPropertyEditorModule::CreateFloatingDetailsView.
	bool ShouldShowProperty(const FPropertyAndParent& PropertyAndParent, bool bHaveTemplate)
	{
		const UProperty& Property = PropertyAndParent.Property;

		if (bHaveTemplate)
		{
			const UClass* PropertyOwnerClass = Cast<const UClass>(Property.GetOuter());
			const bool bDisableEditOnTemplate = PropertyOwnerClass
				&& PropertyOwnerClass->IsNative()
				&& Property.HasAnyPropertyFlags(CPF_DisableEditOnTemplate);

			if (bDisableEditOnTemplate)
			{
				return false;
			}
		}
		return true;
	}

	FReply ExecuteEditorCommand(ULaunchConfigurationEditor* Instance, UFunction* MethodToExecute)
	{
		Instance->CallFunctionByNameWithArguments(*MethodToExecute->GetName(), *GLog, nullptr, true);

		return FReply::Handled();
	}
}

ULaunchConfigurationEditor* ULaunchConfigurationEditor::OpenModalWindow(TSharedPtr<SWindow> InParentWindow)
{
	ULaunchConfigurationEditor* ObjectInstance = NewObject<ULaunchConfigurationEditor>(GetTransientPackage(), ULaunchConfigurationEditor::StaticClass());
	ObjectInstance->AddToRoot();

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	TArray<UObject*> ObjectsToView;
	ObjectsToView.Add(ObjectInstance);

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bLockable = false;
	Args.bAllowSearch = false;
	Args.bShowPropertyMatrixButton = false;

	TSharedRef<IDetailsView> DetailView = PropertyEditorModule.CreateDetailView(Args);

	bool bHaveTemplate = false;
	for (int32 i = 0; i < ObjectsToView.Num(); i++)
	{
		if (ObjectsToView[i] != NULL && ObjectsToView[i]->IsTemplate())
		{
			bHaveTemplate = true;
			break;
		}
	}

	DetailView->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateStatic(&ShouldShowProperty, bHaveTemplate));

	DetailView->SetObjects(ObjectsToView);

	TSharedRef<SVerticalBox> VBoxBuilder = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.FillHeight(1.0)
		[
			DetailView
		];

	// Add UFunction marked Exec as buttons in the editor's window
	for (TFieldIterator<UFunction> FuncIt(ULaunchConfigurationEditor::StaticClass()); FuncIt; ++FuncIt)
	{
		UFunction* Function = *FuncIt;
		if (Function->HasAnyFunctionFlags(FUNC_Exec) && (Function->NumParms == 0))
		{
			const FText ButtonCaption = Function->GetDisplayNameText();

			VBoxBuilder->AddSlot()
				.AutoHeight()
				.VAlign(VAlign_Bottom)
				.HAlign(HAlign_Right)
				.Padding(2.0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2.0)
					[
						SNew(SButton)
						.Text(ButtonCaption)
						.OnClicked(FOnClicked::CreateStatic(&ExecuteEditorCommand, ObjectInstance, Function))
					]
				];
		}
	}

	TSharedRef<SWindow> NewSlateWindow = SNew(SWindow)
		.Title(FText::FromString(TEXT("Launch Configuration Editor")))
		.ClientSize(FVector2D(600, 400))
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush(TEXT("PropertyWindow.WindowBorder")))
			[
				VBoxBuilder
			]
		];

	if (!InParentWindow.IsValid() && FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		// If the main frame exists parent the window to it
		IMainFrameModule& MainFrame = FModuleManager::GetModuleChecked<IMainFrameModule>("MainFrame");
		InParentWindow = MainFrame.GetParentWindow();
	}

	if (InParentWindow.IsValid())
	{
		FSlateApplication::Get().AddModalWindow(NewSlateWindow, InParentWindow.ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddModalWindow(NewSlateWindow, nullptr);
	}
	return ObjectInstance;
}
