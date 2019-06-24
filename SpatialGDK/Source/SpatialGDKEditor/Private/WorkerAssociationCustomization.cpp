#include "WorkerAssociationCustomization.h"

#include "SpatialGDK/public/SpatialGDKSettings.h"

#include "PropertyEditor/Public/DetailLayoutBuilder.h"
#include "PropertyEditor/Public/DetailWidgetRow.h"
#include "PropertyEditor/Public/IDetailChildrenBuilder.h"
#include "PropertyEditor/Public/PropertyCustomizationHelpers.h"
#include "PropertyEditor/Public/PropertyHandle.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IPropertyTypeCustomization> FWorkerAssociationCustomization::MakeInstance()
{
	return MakeShareable(new FWorkerAssociationCustomization());
}

void FWorkerAssociationCustomization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	TSharedPtr<IPropertyHandle> WorkerTypeNameProperty = StructPropertyHandle->GetChildHandle("WorkerTypeName");

	if (WorkerTypeNameProperty->IsValidHandle())
	{
		HeaderRow.NameContent()
			[
				StructPropertyHandle->CreatePropertyNameWidget()
			]
		.ValueContent()
			[
				PropertyCustomizationHelpers::MakePropertyComboBox(WorkerTypeNameProperty,
				FOnGetPropertyComboBoxStrings::CreateStatic(&FWorkerAssociationCustomization::OnGetStrings),
				FOnGetPropertyComboBoxValue::CreateStatic(&FWorkerAssociationCustomization::OnGetValue, WorkerTypeNameProperty))
			];
	}
}

void FWorkerAssociationCustomization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	
}

void FWorkerAssociationCustomization::OnGetStrings(TArray<TSharedPtr<FString>>& OutComboBoxStrings, TArray<TSharedPtr<class SToolTip>>& OutToolTips, TArray<bool>& OutRestrictedItems)
{
	if (const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>())
	{
		for (FString WorkerType : Settings->WorkerTypes)
		{
			TSharedPtr<FString> WorkerTypePtr = MakeShared<FString>(WorkerType);
			OutComboBoxStrings.Add(WorkerTypePtr);
			OutToolTips.Add(SNew(SToolTip).Text(FText::FromString(WorkerType)));
			OutRestrictedItems.Add(false);
		}
	}
}

FString FWorkerAssociationCustomization::OnGetValue(TSharedPtr<IPropertyHandle> WorkerTypeNameHandle)
{
	FString WorkerTypeValue;

	if (const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>())
	{
		WorkerTypeNameHandle->GetValue(WorkerTypeValue);

		if (Settings->WorkerTypes.Contains(WorkerTypeValue))
		{
			return WorkerTypeValue;
		}
		else
		{
			return TEXT("INVALID");
		}
	}

	return WorkerTypeValue;
}
