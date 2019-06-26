#pragma once

#include "CoreMinimal.h"
#include "PropertyEditor/Public/IPropertyTypeCustomization.h"
#include "PropertyEditor/Public/IDetailCustomNodeBuilder.h"
#include "PropertyEditor/Public/PropertyHandle.h"
#include "SpatialGDK/Public/Utils/ActorGroupManager.h"

class FWorkerTypeCustomization : public IPropertyTypeCustomization
{
public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

private:
	static void OnGetStrings(TArray<TSharedPtr<FString>>& OutComboBoxStrings, TArray<TSharedPtr<class SToolTip>>& OutToolTips, TArray<bool>& OutRestrictedItems);
	static FString OnGetValue(TSharedPtr<IPropertyHandle> WorkerTypeNameHandle);
	static void OnValueSelected(const FString& SelectedValue, TSharedPtr<IPropertyHandle> WorkerTypeNameHandle);
};
