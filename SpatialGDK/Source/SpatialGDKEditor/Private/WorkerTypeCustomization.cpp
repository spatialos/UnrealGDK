// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "WorkerTypeCustomization.h"

#include "SpatialGDKSettings.h"

#include "PropertyCustomizationHelpers.h"
#include "PropertyHandle.h"
#include "Widgets/SToolTip.h"

TSharedRef<IPropertyTypeCustomization> FWorkerTypeCustomization::MakeInstance()
{
	return MakeShared<FWorkerTypeCustomization>();
}

void FWorkerTypeCustomization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
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
				FOnGetPropertyComboBoxStrings::CreateStatic(&FWorkerTypeCustomization::OnGetStrings),
				FOnGetPropertyComboBoxValue::CreateStatic(&FWorkerTypeCustomization::OnGetValue, WorkerTypeNameProperty),
				FOnPropertyComboBoxValueSelected::CreateStatic(&FWorkerTypeCustomization::OnValueSelected, WorkerTypeNameProperty))
			];
	}
}

void FWorkerTypeCustomization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
}

void FWorkerTypeCustomization::OnGetStrings(TArray<TSharedPtr<FString>>& OutComboBoxStrings, TArray<TSharedPtr<class SToolTip>>& OutToolTips, TArray<bool>& OutRestrictedItems)
{
	if (const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>())
	{
		for (const FName& WorkerType : Settings->ServerWorkerTypes)
		{
			OutComboBoxStrings.Add(MakeShared<FString>(WorkerType.ToString()));
			OutToolTips.Add(SNew(SToolTip).Text(FText::FromName(WorkerType)));
			OutRestrictedItems.Add(false);
		}
	}
}

FString FWorkerTypeCustomization::OnGetValue(TSharedPtr<IPropertyHandle> WorkerTypeNameHandle)
{
	if (!WorkerTypeNameHandle->IsValidHandle())
	{
		return FString();
	}

	FString WorkerTypeValue;

	if (const USpatialGDKSettings* Settings = GetDefault<USpatialGDKSettings>())
	{
		WorkerTypeNameHandle->GetValue(WorkerTypeValue);
		const FName WorkerTypeName = FName(*WorkerTypeValue);

		return Settings->ServerWorkerTypes.Contains(WorkerTypeName) ? WorkerTypeValue : TEXT("INVALID");
	}

	return WorkerTypeValue;
}

void FWorkerTypeCustomization::OnValueSelected(const FString& SelectedValue, TSharedPtr<IPropertyHandle> WorkerTypeNameHandle)
{
	if (WorkerTypeNameHandle->IsValidHandle())
	{
		const FName NewValue = FName(*SelectedValue);
		WorkerTypeNameHandle->SetValue(NewValue);
	}
}
