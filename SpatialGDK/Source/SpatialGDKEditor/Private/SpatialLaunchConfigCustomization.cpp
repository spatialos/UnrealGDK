// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "SpatialLaunchConfigCustomization.h"

#include "SpatialGDKSettings.h"
#include "SpatialGDKEditorSettings.h"

#include "IDetailChildrenBuilder.h"
#include "IDetailGroup.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyHandle.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IPropertyTypeCustomization> FSpatialLaunchConfigCustomization::MakeInstance()
{
	return MakeShared<FSpatialLaunchConfigCustomization>();
}

void FSpatialLaunchConfigCustomization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	
}

void FSpatialLaunchConfigCustomization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TArray<UObject*> EditedObject;
	StructPropertyHandle->GetOuterObjects(EditedObject);

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

		// Layout regular properties as usual.
		if (ChildProperty->GetProperty()->GetName() != "ServerWorkersMap")
		{
			StructBuilder.AddProperty(ChildProperty.ToSharedRef());
			continue;
		}

		// Layout ServerWorkers map in a way that does not allow resizing and key edition.
		uint32 NumEntries;
		ChildProperty->GetNumChildren(NumEntries);

		IDetailGroup& NewGroup = StructBuilder.AddGroup("ServerWorkersMap", ChildProperty->GetPropertyDisplayName());
		NewGroup.HeaderRow()
		.NameContent()
		[
			SNew(STextBlock).Text(FText::FromString(TEXT("Server Workers")))
		]
		.ValueContent()
		[
			SNew(STextBlock).Text(FText::FromString(FString::Printf(TEXT("%i Elements"), NumEntries)))
		];

		const TMap<FName, FWorkerTypeLaunchSection>* CustomizedMap = reinterpret_cast<TMap<FName, FWorkerTypeLaunchSection>*>(ChildProperty->GetValueBaseAddress(reinterpret_cast<uint8*>(EditedObject[0])));

		// Assume that the properties as listed in the same order as the map's iterator.
		auto Iterator = CustomizedMap->CreateConstIterator();

		for (uint32 EntryIdx = 0; EntryIdx < NumEntries; ++EntryIdx, ++Iterator)
		{
			TSharedPtr<IPropertyHandle> EntryProp = ChildProperty->GetChildHandle(EntryIdx);

			IDetailGroup& Entry = NewGroup.AddGroup(Iterator->Key, FText::FromName(Iterator->Key));
			uint32 NumEntryFields;
			EntryProp->GetNumChildren(NumEntryFields);

			for (uint32 EntryField = 0; EntryField < NumEntryFields; ++EntryField)
			{
				TSharedPtr<IPropertyHandle> EntryFieldProp = EntryProp->GetChildHandle(EntryField);

				// Skip the load balancing property in the Editor settings.
				if (bIsInSettings && EntryFieldProp->GetProperty()->GetFName() == GET_MEMBER_NAME_CHECKED(FWorkerTypeLaunchSection, WorkerLoadBalancing))
				{
					continue;
				}

				Entry.AddPropertyRow(EntryFieldProp.ToSharedRef()).CustomWidget(true).NameContent()
				[
					EntryFieldProp->CreatePropertyNameWidget()
				]
				.ValueContent()
				[
					EntryFieldProp->CreatePropertyValueWidget()
				];
			}
		}		
	}
}
