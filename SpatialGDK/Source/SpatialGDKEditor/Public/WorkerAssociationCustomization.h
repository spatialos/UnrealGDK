#pragma once

#include "CoreMinimal.h"
#include "PropertyEditor/Public/IPropertyTypeCustomization.h"
#include "PropertyEditor/Public/IDetailCustomNodeBuilder.h"
#include "PropertyEditor/Public/PropertyHandle.h"
#include "SpatialGDK/Public/Interop/ActorGroupManager.h"

class FDetailAssociationBuilder : public IDetailCustomNodeBuilder
{
public:

	FDetailAssociationBuilder();
	FDetailAssociationBuilder(class USpatialGDKSettings* Settings);

	/** IDetailCustomNodeBuilder interface */
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override { OnRegenerateChildren = InOnRegenerateChildren; }
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override;
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override;

	virtual void Tick(float DeltaTime) {}
	virtual bool RequiresTick() const { return false; }

	virtual bool InitiallyCollapsed() const { return false; }
	virtual FName GetName() const { return FName(); }

	class USpatialGDKSettings* Settings;

	FSimpleDelegate OnRegenerateChildren;

	void OnGetStrings(TArray<TSharedPtr<FString>>& OutComboBoxStrings, TArray<TSharedPtr<class SToolTip>>& OutToolTips, TArray<bool>& OutRestrictedItems);
	FString OnGetValue(FName ActorGroup);
	void OnValueSelected(const FString&, FName ActorGroup);

	void RefreshChildren();

};

class FWorkerAssociationCustomization : public IPropertyTypeCustomization
{
public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** IPropertyTypeCustomization interface */
	virtual void CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils) override;

	FWorkerAssociationCustomization();

	~FWorkerAssociationCustomization();

private:
	class USpatialGDKSettings* Settings;

	TSharedPtr<IPropertyHandle> ActorGroupsHandle;

	FDetailAssociationBuilder* DetailsBuilder;

	void OnActorGroupsChanged();
};
