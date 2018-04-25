// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "TestFieldTypesData.h"

UTestFieldTypesData::UTestFieldTypesData()
{
	Underlying.Reset(new test::TestFieldTypesData(0, static_cast<test::TestEnum>(0), worker::List<std::string>(), worker::Map<worker::EntityId, test::TestType1>(), worker::Option<test::TestType2>(), test::ListMapOptionUserTypeData(worker::List<worker::EntityId>(), worker::Map<std::string, improbable::Coordinates>(), worker::Option<bool>(), test::BuiltInTypesData(false, 0, 0, 0, 0, "", "", improbable::Coordinates(0, 0, 0), improbable::Vector3d(0, 0, 0), improbable::Vector3f(0, 0, 0), worker::EntityId(0)))));
}

UTestFieldTypesData* UTestFieldTypesData::Init(const test::TestFieldTypesData& underlying)
{
    Underlying.Reset(new test::TestFieldTypesData(underlying));
	return this;
}

int UTestFieldTypesData::GetBuiltInProperty()
{
    return static_cast<int>(Underlying->built_in_property());
}
UTestFieldTypesData* UTestFieldTypesData::SetBuiltInProperty(int built_in_property)
{
    Underlying->set_built_in_property(static_cast<std::int32_t>(built_in_property));
	return this;
}

ETestEnum UTestFieldTypesData::GetEnumProperty()
{
    return static_cast<ETestEnum>(Underlying->enum_property());
}
UTestFieldTypesData* UTestFieldTypesData::SetEnumProperty(ETestEnum enum_property)
{
    Underlying->set_enum_property(static_cast<test::TestEnum>(enum_property));
	return this;
}

UStdStringList* UTestFieldTypesData::GetListProperty()
{
    return NewObject<UStdStringList>()->Init(Underlying->list_property());
}
UTestFieldTypesData* UTestFieldTypesData::SetListProperty(UStdStringList* list_property)
{
    Underlying->set_list_property(list_property->GetUnderlying());
	return this;
}

UWorkerEntityIdToTestTestType1Map* UTestFieldTypesData::GetMapProperty()
{
    return NewObject<UWorkerEntityIdToTestTestType1Map>()->Init(Underlying->map_property());
}
UTestFieldTypesData* UTestFieldTypesData::SetMapProperty(UWorkerEntityIdToTestTestType1Map* map_property)
{
    Underlying->set_map_property(map_property->GetUnderlying());
	return this;
}

UTestTestType2Option* UTestFieldTypesData::GetOptionProperty()
{
    return NewObject<UTestTestType2Option>()->Init(Underlying->option_property());
}
UTestFieldTypesData* UTestFieldTypesData::SetOptionProperty(UTestTestType2Option* option_property)
{
    Underlying->set_option_property(option_property->GetUnderlying());
	return this;
}

UListMapOptionUserTypeData* UTestFieldTypesData::GetUserTypeProperty()
{
    return NewObject<UListMapOptionUserTypeData>()->Init(Underlying->user_type_property());
}
UTestFieldTypesData* UTestFieldTypesData::SetUserTypeProperty(UListMapOptionUserTypeData* user_type_property)
{
    Underlying->set_user_type_property(user_type_property->GetUnderlying());
	return this;
}


test::TestFieldTypesData UTestFieldTypesData::GetUnderlying()
{
	return *Underlying.Get();
}
