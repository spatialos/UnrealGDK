// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialRuntimeVersionCustomization.h"

#include "SpatialGDKEditorSettings.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "PropertyHandle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IPropertyTypeCustomization> FSpatialRuntimeVersionCustomization::MakeInstance()
{
	return MakeShareable(new FSpatialRuntimeVersionCustomization);
}

void FSpatialRuntimeVersionCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> StructPropertyHandle, FDetailWidgetRow& HeaderRow,
														  IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	HeaderRow.NameContent()[StructPropertyHandle->CreatePropertyNameWidget()];
}

void FSpatialRuntimeVersionCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> StructPropertyHandle,
															IDetailChildrenBuilder& StructBuilder,
															IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	const FName& PinnedGDKRuntimeLocalPropertyName = GET_MEMBER_NAME_CHECKED(FRuntimeVariantVersion, bUseGDKPinnedRuntimeVersionForLocal);
	const FName& PinnedGDKRuntimeCloudPropertyName = GET_MEMBER_NAME_CHECKED(FRuntimeVariantVersion, bUseGDKPinnedRuntimeVersionForCloud);

	uint32 NumChildren;
	StructPropertyHandle->GetNumChildren(NumChildren);

	for (uint32 ChildIdx = 0; ChildIdx < NumChildren; ++ChildIdx)
	{
		TSharedPtr<IPropertyHandle> ChildProperty = StructPropertyHandle->GetChildHandle(ChildIdx);

		// Layout other properties as usual.
		if (ChildProperty->GetProperty()->GetFName() != PinnedGDKRuntimeLocalPropertyName
			&& ChildProperty->GetProperty()->GetFName() != PinnedGDKRuntimeCloudPropertyName)
		{
			StructBuilder.AddProperty(ChildProperty.ToSharedRef());
			continue;
		}

		void* StructPtr;
		check(StructPropertyHandle->GetValueData(StructPtr) == FPropertyAccess::Success);

		const FRuntimeVariantVersion* VariantVersion = reinterpret_cast<const FRuntimeVariantVersion*>(StructPtr);

		IDetailPropertyRow& CustomRow = StructBuilder.AddProperty(ChildProperty.ToSharedRef());

		FString PinnedVersionDisplay = FString::Printf(TEXT("GDK Pinned Version : %s"), *VariantVersion->GetPinnedVersion());

		CustomRow.CustomWidget()
			.NameContent()[ChildProperty->CreatePropertyNameWidget()]
			.ValueContent()[SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().HAlign(HAlign_Left).AutoWidth()[ChildProperty->CreatePropertyValueWidget()]
							+ SHorizontalBox::Slot()
								  .Padding(5)
								  .HAlign(HAlign_Center)
								  .AutoWidth()[SNew(STextBlock).Text(FText::FromString(PinnedVersionDisplay))]];
	}
}
