// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialLaunchConfigCustomization.h"

#include "SpatialGDKEditorSettings.h"
#include "SpatialGDKSettings.h"

#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyHandle.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SpatialLaunchConfigCustomization"

TSharedRef<IPropertyTypeCustomization> FSpatialLaunchConfigCustomization::MakeInstance()
{
	return MakeShared<FSpatialLaunchConfigCustomization>();
}

void FSpatialLaunchConfigCustomization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle,
														class FDetailWidgetRow& HeaderRow,
														IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

void FSpatialLaunchConfigCustomization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle,
														  class IDetailChildrenBuilder& StructBuilder,
														  IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TArray<UObject*> EditedObject;
	StructPropertyHandle->GetOuterObjects(EditedObject);

	const FName& PinnedGDKRuntimeLocalPropertyName =
		GET_MEMBER_NAME_CHECKED(FSpatialLaunchConfigDescription, bUseDefaultTemplateForRuntimeVariant);

	if (EditedObject.Num() == 0)
	{
		return;
	}

	const bool bIsInSettings = Cast<USpatialGDKEditorSettings>(EditedObject[0]) != nullptr;

	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);
	for (uint32 ChildIdx = 0; ChildIdx < NumChildren; ++ChildIdx)
	{
		TSharedPtr<IPropertyHandle> ChildProperty = StructPropertyHandle->GetChildHandle(ChildIdx);

		if (ChildProperty->GetProperty()->GetFName() == PinnedGDKRuntimeLocalPropertyName)
		{
			// Place the pinned template name for this runtime variant in the pinned template field.

			void* StructPtr;
			check(StructPropertyHandle->GetValueData(StructPtr) == FPropertyAccess::Success);

			const FSpatialLaunchConfigDescription* LaunchConfigDesc = reinterpret_cast<const FSpatialLaunchConfigDescription*>(StructPtr);

			FText PinnedTemplateDisplay = FText::Format(LOCTEXT("DefaultTemplate", "Default: {0}"),
														FText::FromString(LaunchConfigDesc->GetDefaultTemplateForRuntimeVariant()));

			IDetailPropertyRow& CustomRow = StructBuilder.AddProperty(ChildProperty.ToSharedRef());

			CustomRow.CustomWidget().NameContent()[ChildProperty->CreatePropertyNameWidget()].ValueContent()
				[SNew(SHorizontalBox) + SHorizontalBox::Slot().HAlign(HAlign_Left).AutoWidth()[ChildProperty->CreatePropertyValueWidget()]
				 + SHorizontalBox::Slot().Padding(5).HAlign(HAlign_Center).AutoWidth()[SNew(STextBlock).Text(PinnedTemplateDisplay)]];
		}
		else
		{
			// Layout regular properties as usual.
			StructBuilder.AddProperty(ChildProperty.ToSharedRef());
			continue;
		}
	}
}

#undef LOCTEXT_NAMESPACE
