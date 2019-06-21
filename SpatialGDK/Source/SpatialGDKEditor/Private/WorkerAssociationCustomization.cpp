#include "WorkerAssociationCustomization.h"

#include "SpatialGDK/public/SpatialGDKSettings.h"

#include "PropertyEditor/Public/DetailLayoutBuilder.h"
#include "PropertyEditor/Public/DetailWidgetRow.h"
#include "PropertyEditor/Public/IDetailChildrenBuilder.h"
#include "PropertyEditor/Public/PropertyCustomizationHelpers.h"
#include "PropertyEditor/Public/PropertyHandle.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "WorkerAssociationCustomization"

FWorkerAssociationCustomization::FWorkerAssociationCustomization() : Settings(GetMutableDefault<USpatialGDKSettings>())
{
}

FWorkerAssociationCustomization::~FWorkerAssociationCustomization()
{
	if (ActorGroupsHandle.IsValid())
	{
		FSimpleDelegate Empty;
		ActorGroupsHandle->SetOnChildPropertyValueChanged(Empty);
		ActorGroupsHandle->SetOnPropertyValueChanged(Empty);
	}
}

TSharedRef<IPropertyTypeCustomization> FWorkerAssociationCustomization::MakeInstance()
{
	return MakeShareable(new FWorkerAssociationCustomization());
}

void FWorkerAssociationCustomization::CustomizeHeader(TSharedRef<class IPropertyHandle> StructPropertyHandle, class FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	HeaderRow.NameContent()
		[
			StructPropertyHandle->CreatePropertyNameWidget()
		];
}

void FWorkerAssociationCustomization::CustomizeChildren(TSharedRef<class IPropertyHandle> StructPropertyHandle, class IDetailChildrenBuilder& StructBuilder, IPropertyTypeCustomizationUtils& StructCustomizationUtils)
{
	if (StructPropertyHandle->IsValidHandle())
	{
		TSharedPtr<IPropertyHandle> ParentHandle = StructPropertyHandle->GetParentHandle();
		if (ParentHandle)
		{
			FSimpleDelegate Refresh = FSimpleDelegate::CreateRaw(this, &FWorkerAssociationCustomization::OnActorGroupsChanged);

			ActorGroupsHandle = ParentHandle->GetChildHandle("ActorGroups");
			if (ActorGroupsHandle.IsValid())
			{
				ActorGroupsHandle->SetOnChildPropertyValueChanged(Refresh);
				ActorGroupsHandle->SetOnPropertyValueChanged(Refresh);
			}
		}
	}

	DetailsBuilder = new FDetailAssociationBuilder(Settings);
	StructBuilder.AddCustomBuilder(MakeShareable(DetailsBuilder));
}

FDetailAssociationBuilder::FDetailAssociationBuilder()
{
}

FDetailAssociationBuilder::FDetailAssociationBuilder(USpatialGDKSettings* Settings) : Settings(Settings)
{
}

void FDetailAssociationBuilder::GenerateHeaderRowContent(FDetailWidgetRow& NodeRow)
{
}

void FDetailAssociationBuilder::RefreshChildren()
{
	OnRegenerateChildren.ExecuteIfBound();
}

void FDetailAssociationBuilder::GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder)
{
	TArray<FName> ActorGroups;
	Settings->ActorGroups.GetKeys(ActorGroups);
	for (FName ActorGroup : ActorGroups)
	{
		ChildrenBuilder.AddCustomRow(LOCTEXT("", ""))
		.NameContent()
			[
				SNew(STextBlock).Font(IDetailLayoutBuilder::GetDetailFont()).Text(FText::FromName(ActorGroup))
			]
		.ValueContent()
			[
				PropertyCustomizationHelpers::MakePropertyComboBox(nullptr,
				FOnGetPropertyComboBoxStrings::CreateRaw(this, &FDetailAssociationBuilder::OnGetStrings),
				FOnGetPropertyComboBoxValue::CreateRaw(this, &FDetailAssociationBuilder::OnGetValue, ActorGroup),
				FOnPropertyComboBoxValueSelected::CreateRaw(this, &FDetailAssociationBuilder::OnValueSelected, ActorGroup))
			];
	}
}

void FWorkerAssociationCustomization::OnActorGroupsChanged()
{
	DetailsBuilder->RefreshChildren();
}

void FDetailAssociationBuilder::OnGetStrings(TArray<TSharedPtr<FString>>& OutComboBoxStrings, TArray<TSharedPtr<class SToolTip>>& OutToolTips, TArray<bool>& OutRestrictedItems)
{
	for (FString WorkerType : Settings->WorkerTypes)
	{
		TSharedPtr<FString> WorkerTypePtr = MakeShared<FString>(WorkerType);
		OutComboBoxStrings.Add(WorkerTypePtr);
		OutToolTips.Add(SNew(SToolTip).Text(FText::FromString(WorkerType)));
		OutRestrictedItems.Add(false);
	}
}

FString FDetailAssociationBuilder::OnGetValue(FName ActorGroup)
{
	if (Settings->WorkerAssociation.ActorGroupToWorker.Contains(ActorGroup))
	{
		return Settings->WorkerAssociation.ActorGroupToWorker.FindRef(ActorGroup);
	}
	else
	{
		return Settings->WorkerTypes.Array()[0];
	}
}

void FDetailAssociationBuilder::OnValueSelected(const FString& Value, FName ActorGroup)
{
	if (Settings->WorkerAssociation.ActorGroupToWorker.FindRef(ActorGroup) != Value)
	{
		Settings->WorkerAssociation.ActorGroupToWorker.Add(ActorGroup, Value);
		Settings->ValidateOffloadingSettings();
	}
}
#undef LOCTEXT_NAMESPACE
