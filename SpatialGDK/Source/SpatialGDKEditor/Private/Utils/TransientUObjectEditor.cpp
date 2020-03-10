#include "Utils/TransientUObjectEditor.h"

#include "PropertyEditor/Public/PropertyEditorModule.h"
#include "MainFrame/Public/Interfaces/IMainFrameModule.h"

#include "SlateApplication.h"
#include "SBorder.h"
#include "SButton.h"

//DECLARE_LOG_CATEGORY_EXTERN(LogSpatialGDKTransientUObjectEditor, Log, All);

static void OnTransientUObjectEditorWindowClosed(const TSharedRef<SWindow>& Window, UTransientUObjectEditor* Instance)
{
	Instance->RemoveFromRoot();
}

static bool ShouldShowProperty(const FPropertyAndParent& PropertyAndParent, bool bHaveTemplate)
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

static FReply ExecuteEditorCommand(UTransientUObjectEditor* Instance, UFunction* MethodToExecute)
{
	Instance->CallFunctionByNameWithArguments(*MethodToExecute->GetName(), *GLog, nullptr, true);

	return FReply::Handled();
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
		//UE_LOG(LogSpatialGDKTransientUObjectEditor, Error, TEXT("Class %s is not a class compatible with TransientUObjectEditor"), *ObjectClass->GetName());
		return;
	}

	UTransientUObjectEditor* ObjectInstance = NewObject<UTransientUObjectEditor>(GetTransientPackage(), ObjectClass);
	ObjectInstance->AddToRoot();

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	TArray<UObject*> ObjectsToView;
	ObjectsToView.Add(ObjectInstance);

	TSharedRef<SWindow> NewSlateWindow = SNew(SWindow)
		.Title(FText::FromString(EditorName));
		//.ClientSize(FVector2D(400, 550));

	// If the main frame exists parent the window to it
	TSharedPtr< SWindow > ParentWindow;
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

	FDetailsViewArgs Args;
	Args.bHideSelectionTip = true;
	Args.bLockable = false;
	Args.bAllowSearch = false;
	Args.bShowPropertyMatrixButton = false;
	Args.bShowScrollBar = false;

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
			const FString FunctionName = Function->GetName();
			const FText ButtonCaption = FText::FromString(FunctionName);
			const FString FilterString = FunctionName;

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

	NewSlateWindow->SetContent(
		SNew(SBorder)
		.BorderImage(FEditorStyle::GetBrush(TEXT("PropertyWindow.WindowBorder")))
		[
			VBoxBuilder
		]
	);

	NewSlateWindow->SetOnWindowClosed(FOnWindowClosed::CreateStatic(&OnTransientUObjectEditorWindowClosed, ObjectInstance));
}
