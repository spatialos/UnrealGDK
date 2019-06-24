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
				FOnGetPropertyComboBoxValue::CreateStatic(&FWorkerAssociationCustomization::OnGetValue, WorkerTypeNameProperty),
				FOnPropertyComboBoxValueSelected::CreateStatic(&FWorkerAssociationCustomization::OnValueSelected, WorkerTypeNameProperty))
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
		for (FName WorkerType : Settings->WorkerTypes)
		{
			OutComboBoxStrings.Add(MakeShared<FString>(WorkerType.ToString()));
			OutToolTips.Add(SNew(SToolTip).Text(FText::FromName(WorkerType)));
			OutRestrictedItems.Add(false);
		}
	}
}

FString FWorkerAssociationCustomization::OnGetValue(TSharedPtr<IPropertyHandle> WorkerTypeNameHandle)
{
	if (!WorkerTypeNameHandle->IsValidHandle())
	{
		return "";
	}

	FString WorkerTypeValue;

	if (const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>())
	{
		WorkerTypeNameHandle->GetValue(WorkerTypeValue);
		FName WorkerTypeName = FName(*WorkerTypeValue);

		if (Settings->WorkerTypes.Contains(WorkerTypeName))
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

void FWorkerAssociationCustomization::OnValueSelected(const FString& SelectedValue, TSharedPtr<IPropertyHandle> WorkerTypeNameHandle)
{
	if (WorkerTypeNameHandle->IsValidHandle())
	{
		const FName NewValue = FName(*SelectedValue);
		WorkerTypeNameHandle->SetValue(NewValue);
	}
}
