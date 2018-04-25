// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "BuiltInTypesData.h"

UBuiltInTypesData::UBuiltInTypesData()
{
	Underlying.Reset(new test::BuiltInTypesData(false, 0, 0, 0, 0, "", "", improbable::Coordinates(0, 0, 0), improbable::Vector3d(0, 0, 0), improbable::Vector3f(0, 0, 0), worker::EntityId(0)));
}

UBuiltInTypesData* UBuiltInTypesData::Init(const test::BuiltInTypesData& underlying)
{
    Underlying.Reset(new test::BuiltInTypesData(underlying));
	return this;
}

bool UBuiltInTypesData::GetBoolProperty()
{
    return Underlying->bool_property();
}
UBuiltInTypesData* UBuiltInTypesData::SetBoolProperty(bool bool_property)
{
    Underlying->set_bool_property(bool_property);
	return this;
}

int UBuiltInTypesData::GetUint32Property()
{
    return static_cast<int>(Underlying->uint32_property());
}
UBuiltInTypesData* UBuiltInTypesData::SetUint32Property(int uint32_property)
{
    Underlying->set_uint32_property(static_cast<std::uint32_t>(uint32_property));
	return this;
}

int UBuiltInTypesData::GetInt32Property()
{
    return static_cast<int>(Underlying->int32_property());
}
UBuiltInTypesData* UBuiltInTypesData::SetInt32Property(int int32_property)
{
    Underlying->set_int32_property(static_cast<std::int32_t>(int32_property));
	return this;
}

float UBuiltInTypesData::GetFloatProperty()
{
    return Underlying->float_property();
}
UBuiltInTypesData* UBuiltInTypesData::SetFloatProperty(float float_property)
{
    Underlying->set_float_property(float_property);
	return this;
}

float UBuiltInTypesData::GetDoubleProperty()
{
    return static_cast<float>(Underlying->double_property());
}
UBuiltInTypesData* UBuiltInTypesData::SetDoubleProperty(float double_property)
{
    Underlying->set_double_property(static_cast<double>(double_property));
	return this;
}

FString UBuiltInTypesData::GetStringProperty()
{
    return FString(Underlying->string_property().c_str());
}
UBuiltInTypesData* UBuiltInTypesData::SetStringProperty(FString string_property)
{
    Underlying->set_string_property(TCHAR_TO_UTF8(*string_property));
	return this;
}

FString UBuiltInTypesData::GetBytesProperty()
{
    return FString(Underlying->bytes_property().c_str());
}
UBuiltInTypesData* UBuiltInTypesData::SetBytesProperty(FString bytes_property)
{
    Underlying->set_bytes_property(TCHAR_TO_UTF8(*bytes_property));
	return this;
}

FVector UBuiltInTypesData::GetCoordinatesProperty()
{
    return USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(FVector(static_cast<float>(Underlying->coordinates_property().x()), static_cast<float>(Underlying->coordinates_property().y()), static_cast<float>(Underlying->coordinates_property().z())));
}
UBuiltInTypesData* UBuiltInTypesData::SetCoordinatesProperty(FVector coordinates_property)
{
    Underlying->set_coordinates_property(USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(coordinates_property));
	return this;
}

FVector UBuiltInTypesData::GetVector3dProperty()
{
    return FVector(static_cast<float>(Underlying->vector3d_property().x()), static_cast<float>(Underlying->vector3d_property().y()), static_cast<float>(Underlying->vector3d_property().z()));
}
UBuiltInTypesData* UBuiltInTypesData::SetVector3dProperty(FVector vector3d_property)
{
    Underlying->set_vector3d_property(improbable::Vector3d(static_cast<double>(vector3d_property.X), static_cast<double>(vector3d_property.Y), static_cast<double>(vector3d_property.Z)));
	return this;
}

FVector UBuiltInTypesData::GetVector3fProperty()
{
    return FVector(static_cast<float>(Underlying->vector3f_property().x()), static_cast<float>(Underlying->vector3f_property().y()), static_cast<float>(Underlying->vector3f_property().z()));
}
UBuiltInTypesData* UBuiltInTypesData::SetVector3fProperty(FVector vector3f_property)
{
    Underlying->set_vector3f_property(improbable::Vector3f(static_cast<double>(vector3f_property.X), static_cast<double>(vector3f_property.Y), static_cast<double>(vector3f_property.Z)));
	return this;
}

FEntityId UBuiltInTypesData::GetEntityIdProperty()
{
    return FEntityId(Underlying->entity_id_property());
}
UBuiltInTypesData* UBuiltInTypesData::SetEntityIdProperty(FEntityId entity_id_property)
{
    Underlying->set_entity_id_property((entity_id_property).ToSpatialEntityId());
	return this;
}


test::BuiltInTypesData UBuiltInTypesData::GetUnderlying()
{
	return *Underlying.Get();
}
