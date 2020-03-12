// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/TransientUObjectEditor.h"

#include "PropertyEditor/Public/PropertyEditorModule.h"
#include "MainFrame/Public/Interfaces/IMainFrameModule.h"

#include "SBorder.h"
#include "SButton.h"
#include "SlateApplication.h"

namespace
{

	void OnTransientUObjectEditorWindowClosed(const TSharedRef<SWindow>& Window, UTransientUObjectEditor* Instance)
	{
		Instance->RemoveFromRoot();
	}

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

	FReply ExecuteEditorCommand(UTransientUObjectEditor* Instance, UFunction* MethodToExecute)
	{
		Instance->CallFunctionByNameWithArguments(*MethodToExecute->GetName(), *GLog, nullptr, true);

		return FReply::Handled();
	}
}

// Rewrite of FPropertyEditorModule::CreateFloatingDetailsView to use the detail property view in a new window.
void UTransientUObjectEditor::LaunchTransientUObjectEditor(const FString& EditorName, UClass* ObjectClass)
{
	if (!ObjectClass)
	{
		return;
	}

	if (!ObjectClass->IsChildOf<UTransientUObjectEditor>())
	{
		return;
	}

	UTransientUObjectEditor* ObjectInstance = NewObject<UTransientUObjectEditor>(GetTransientPackage(), ObjectClass);
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
	for (TFieldIterator<UFunction> FuncIt(ObjectClass); FuncIt; ++FuncIt)
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
		.Title(FText::FromString(EditorName))
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush(TEXT("PropertyWindow.WindowBorder")))
			[
				VBoxBuilder
			]
		];

	// If the main frame exists parent the window to it
	TSharedPtr<SWindow> ParentWindow;
	if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
	{
		IMainFrameModule& MainFrame = FModuleManager::GetModuleChecked<IMainFrameModule>("MainFrame");
		ParentWindow = MainFrame.GetParentWindow();
	}

	if (ParentWindow.IsValid())
	{
		// Parent the window to the main frame 
		FSlateApplication::Get().AddWindowAsNativeChild(NewSlateWindow, ParentWindow.ToSharedRef());
	}
	else
	{
		FSlateApplication::Get().AddWindow(NewSlateWindow);
	}

	NewSlateWindow->RegisterActiveTimer(0.5, FWidgetActiveTimerDelegate::CreateLambda([NewSlateWindow](double, float)
	{
		NewSlateWindow->Resize(NewSlateWindow->GetDesiredSize());
		return EActiveTimerReturnType::Stop;
	}));
}
