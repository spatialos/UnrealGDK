// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestFieldTypesComponentUpdate.h"

test::TestFieldTypes::Update UTestFieldTypesComponentUpdate::DefaultUnderlying = test::TestFieldTypes::Update();

UTestFieldTypesComponentUpdate::UTestFieldTypesComponentUpdate()
: ListProperty(nullptr)
, MapProperty(nullptr)
, OptionProperty(nullptr)
, UserTypeProperty(nullptr)
{
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::Init(const test::TestFieldTypes::Update& underlying)
{
	return InitInternal(underlying);
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::Reset()
{
	return ResetInternal();
}

bool UTestFieldTypesComponentUpdate::HasBuiltInProperty()
{
    return !Underlying.built_in_property().empty();
}

int UTestFieldTypesComponentUpdate::GetBuiltInProperty()
{
	BuiltInProperty = static_cast<int>((*(Underlying.built_in_property().data())));
    return BuiltInProperty;
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::SetBuiltInProperty(int newValue)
{
    return SetBuiltInPropertyInternal(newValue);
}

bool UTestFieldTypesComponentUpdate::HasEnumProperty()
{
    return !Underlying.enum_property().empty();
}

ETestEnum UTestFieldTypesComponentUpdate::GetEnumProperty()
{
	EnumProperty = static_cast<ETestEnum>((*(Underlying.enum_property().data())));
    return EnumProperty;
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::SetEnumProperty(ETestEnum newValue)
{
    return SetEnumPropertyInternal(newValue);
}

bool UTestFieldTypesComponentUpdate::HasListProperty()
{
    return !Underlying.list_property().empty();
}

UStdStringList* UTestFieldTypesComponentUpdate::GetListProperty()
{
	if (ListProperty == nullptr) { ListProperty = NewObject<UStdStringList>(this); } ListProperty->Init((*(Underlying.list_property().data())));
    return ListProperty;
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::SetListProperty(UStdStringList* newValue)
{
    return SetListPropertyInternal(newValue);
}

bool UTestFieldTypesComponentUpdate::HasMapProperty()
{
    return !Underlying.map_property().empty();
}

UWorkerEntityIdToTestTestType1Map* UTestFieldTypesComponentUpdate::GetMapProperty()
{
	if (MapProperty == nullptr) { MapProperty = NewObject<UWorkerEntityIdToTestTestType1Map>(this); } MapProperty->Init((*(Underlying.map_property().data())));
    return MapProperty;
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::SetMapProperty(UWorkerEntityIdToTestTestType1Map* newValue)
{
    return SetMapPropertyInternal(newValue);
}

bool UTestFieldTypesComponentUpdate::HasOptionProperty()
{
    return !Underlying.option_property().empty();
}

UTestTestType2Option* UTestFieldTypesComponentUpdate::GetOptionProperty()
{
	if (OptionProperty == nullptr) { OptionProperty = NewObject<UTestTestType2Option>(this); } OptionProperty->Init((*(Underlying.option_property().data())));
    return OptionProperty;
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::SetOptionProperty(UTestTestType2Option* newValue)
{
    return SetOptionPropertyInternal(newValue);
}

bool UTestFieldTypesComponentUpdate::HasUserTypeProperty()
{
    return !Underlying.user_type_property().empty();
}

UListMapOptionUserTypeData* UTestFieldTypesComponentUpdate::GetUserTypeProperty()
{
	if (UserTypeProperty == nullptr) { UserTypeProperty = NewObject<UListMapOptionUserTypeData>(this); } UserTypeProperty->Init((*(Underlying.user_type_property().data())));
    return UserTypeProperty;
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::SetUserTypeProperty(UListMapOptionUserTypeData* newValue)
{
    return SetUserTypePropertyInternal(newValue);
}

const test::TestFieldTypes::Update UTestFieldTypesComponentUpdate::GetUnderlying()
{
    return GetUnderlyingInternal();
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::InitInternal(const test::TestFieldTypes::Update& underlying)
{
    Underlying = test::TestFieldTypes::Update(underlying);
	return this;
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::ResetInternal()
{
	return InitInternal(DefaultUnderlying);
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::SetBuiltInPropertyInternal(int newValue)
{
    Underlying.set_built_in_property(static_cast<std::int32_t>(newValue));
	return this;
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::SetEnumPropertyInternal(ETestEnum newValue)
{
    Underlying.set_enum_property(static_cast<test::TestEnum>(newValue));
	return this;
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::SetListPropertyInternal(UStdStringList* newValue)
{
    Underlying.set_list_property(newValue->GetUnderlying());
	return this;
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::SetMapPropertyInternal(UWorkerEntityIdToTestTestType1Map* newValue)
{
    Underlying.set_map_property(newValue->GetUnderlying());
	return this;
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::SetOptionPropertyInternal(UTestTestType2Option* newValue)
{
    Underlying.set_option_property(newValue->GetUnderlying());
	return this;
}

UTestFieldTypesComponentUpdate* UTestFieldTypesComponentUpdate::SetUserTypePropertyInternal(UListMapOptionUserTypeData* newValue)
{
    Underlying.set_user_type_property(newValue->GetUnderlying());
	return this;
}

const test::TestFieldTypes::Update UTestFieldTypesComponentUpdate::GetUnderlyingInternal()
{
    return Underlying;
}
