// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "ListMapOptionUserTypeData.h"

UListMapOptionUserTypeData::UListMapOptionUserTypeData()
{
	Underlying.Reset(new test::ListMapOptionUserTypeData(worker::List<worker::EntityId>(), worker::Map<std::string, improbable::Coordinates>(), worker::Option<bool>(), test::BuiltInTypesData(false, 0, 0, 0, 0, "", "", improbable::Coordinates(0, 0, 0), improbable::Vector3d(0, 0, 0), improbable::Vector3f(0, 0, 0), worker::EntityId(0))));
}

UListMapOptionUserTypeData* UListMapOptionUserTypeData::Init(const test::ListMapOptionUserTypeData& underlying)
{
    Underlying.Reset(new test::ListMapOptionUserTypeData(underlying));
	return this;
}

UWorkerEntityIdList* UListMapOptionUserTypeData::GetListProperty()
{
    return NewObject<UWorkerEntityIdList>()->Init(Underlying->list_property());
}
UListMapOptionUserTypeData* UListMapOptionUserTypeData::SetListProperty(UWorkerEntityIdList* list_property)
{
    Underlying->set_list_property(list_property->GetUnderlying());
	return this;
}

UStdStringToImprobableCoordinatesMap* UListMapOptionUserTypeData::GetMapProperty()
{
    return NewObject<UStdStringToImprobableCoordinatesMap>()->Init(Underlying->map_property());
}
UListMapOptionUserTypeData* UListMapOptionUserTypeData::SetMapProperty(UStdStringToImprobableCoordinatesMap* map_property)
{
    Underlying->set_map_property(map_property->GetUnderlying());
	return this;
}

UBoolOption* UListMapOptionUserTypeData::GetOptionProperty()
{
    return NewObject<UBoolOption>()->Init(Underlying->option_property());
}
UListMapOptionUserTypeData* UListMapOptionUserTypeData::SetOptionProperty(UBoolOption* option_property)
{
    Underlying->set_option_property(option_property->GetUnderlying());
	return this;
}

UBuiltInTypesData* UListMapOptionUserTypeData::GetUserTypeProperty()
{
    return NewObject<UBuiltInTypesData>()->Init(Underlying->user_type_property());
}
UListMapOptionUserTypeData* UListMapOptionUserTypeData::SetUserTypeProperty(UBuiltInTypesData* user_type_property)
{
    Underlying->set_user_type_property(user_type_property->GetUnderlying());
	return this;
}


test::ListMapOptionUserTypeData UListMapOptionUserTypeData::GetUnderlying()
{
	return *Underlying.Get();
}
