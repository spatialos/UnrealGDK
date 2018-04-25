// ===========
// DO NOT EDIT - this file is automatically regenerated.
// =========== 

#include "BuiltInTypesComponentUpdate.h"

test::BuiltInTypes::Update UBuiltInTypesComponentUpdate::DefaultUnderlying = test::BuiltInTypes::Update();

UBuiltInTypesComponentUpdate::UBuiltInTypesComponentUpdate()
{
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::Init(const test::BuiltInTypes::Update& underlying)
{
	return InitInternal(underlying);
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::Reset()
{
	return ResetInternal();
}

bool UBuiltInTypesComponentUpdate::HasBoolProperty()
{
    return !Underlying.bool_property().empty();
}

bool UBuiltInTypesComponentUpdate::GetBoolProperty()
{
	BoolProperty = (*(Underlying.bool_property().data()));
    return BoolProperty;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetBoolProperty(bool newValue)
{
    return SetBoolPropertyInternal(newValue);
}

bool UBuiltInTypesComponentUpdate::HasUint32Property()
{
    return !Underlying.uint32_property().empty();
}

int UBuiltInTypesComponentUpdate::GetUint32Property()
{
	Uint32Property = static_cast<int>((*(Underlying.uint32_property().data())));
    return Uint32Property;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetUint32Property(int newValue)
{
    return SetUint32PropertyInternal(newValue);
}

bool UBuiltInTypesComponentUpdate::HasInt32Property()
{
    return !Underlying.int32_property().empty();
}

int UBuiltInTypesComponentUpdate::GetInt32Property()
{
	Int32Property = static_cast<int>((*(Underlying.int32_property().data())));
    return Int32Property;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetInt32Property(int newValue)
{
    return SetInt32PropertyInternal(newValue);
}

bool UBuiltInTypesComponentUpdate::HasFloatProperty()
{
    return !Underlying.float_property().empty();
}

float UBuiltInTypesComponentUpdate::GetFloatProperty()
{
	FloatProperty = (*(Underlying.float_property().data()));
    return FloatProperty;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetFloatProperty(float newValue)
{
    return SetFloatPropertyInternal(newValue);
}

bool UBuiltInTypesComponentUpdate::HasDoubleProperty()
{
    return !Underlying.double_property().empty();
}

float UBuiltInTypesComponentUpdate::GetDoubleProperty()
{
	DoubleProperty = static_cast<float>((*(Underlying.double_property().data())));
    return DoubleProperty;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetDoubleProperty(float newValue)
{
    return SetDoublePropertyInternal(newValue);
}

bool UBuiltInTypesComponentUpdate::HasStringProperty()
{
    return !Underlying.string_property().empty();
}

FString UBuiltInTypesComponentUpdate::GetStringProperty()
{
	StringProperty = FString((*(Underlying.string_property().data())).c_str());
    return StringProperty;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetStringProperty(FString newValue)
{
    return SetStringPropertyInternal(newValue);
}

bool UBuiltInTypesComponentUpdate::HasBytesProperty()
{
    return !Underlying.bytes_property().empty();
}

FString UBuiltInTypesComponentUpdate::GetBytesProperty()
{
	BytesProperty = FString((*(Underlying.bytes_property().data())).c_str());
    return BytesProperty;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetBytesProperty(FString newValue)
{
    return SetBytesPropertyInternal(newValue);
}

bool UBuiltInTypesComponentUpdate::HasCoordinatesProperty()
{
    return !Underlying.coordinates_property().empty();
}

FVector UBuiltInTypesComponentUpdate::GetCoordinatesProperty()
{
	CoordinatesProperty = USpatialOSConversionFunctionLibrary::SpatialOsCoordinatesToUnrealCoordinates(FVector(static_cast<float>((*(Underlying.coordinates_property().data())).x()), static_cast<float>((*(Underlying.coordinates_property().data())).y()), static_cast<float>((*(Underlying.coordinates_property().data())).z())));
    return CoordinatesProperty;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetCoordinatesProperty(FVector newValue)
{
    return SetCoordinatesPropertyInternal(newValue);
}

bool UBuiltInTypesComponentUpdate::HasVector3dProperty()
{
    return !Underlying.vector3d_property().empty();
}

FVector UBuiltInTypesComponentUpdate::GetVector3dProperty()
{
	Vector3dProperty = FVector(static_cast<float>((*(Underlying.vector3d_property().data())).x()), static_cast<float>((*(Underlying.vector3d_property().data())).y()), static_cast<float>((*(Underlying.vector3d_property().data())).z()));
    return Vector3dProperty;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetVector3dProperty(FVector newValue)
{
    return SetVector3dPropertyInternal(newValue);
}

bool UBuiltInTypesComponentUpdate::HasVector3fProperty()
{
    return !Underlying.vector3f_property().empty();
}

FVector UBuiltInTypesComponentUpdate::GetVector3fProperty()
{
	Vector3fProperty = FVector(static_cast<float>((*(Underlying.vector3f_property().data())).x()), static_cast<float>((*(Underlying.vector3f_property().data())).y()), static_cast<float>((*(Underlying.vector3f_property().data())).z()));
    return Vector3fProperty;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetVector3fProperty(FVector newValue)
{
    return SetVector3fPropertyInternal(newValue);
}

bool UBuiltInTypesComponentUpdate::HasEntityIdProperty()
{
    return !Underlying.entity_id_property().empty();
}

FEntityId UBuiltInTypesComponentUpdate::GetEntityIdProperty()
{
	EntityIdProperty = FEntityId((*(Underlying.entity_id_property().data())));
    return EntityIdProperty;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetEntityIdProperty(FEntityId newValue)
{
    return SetEntityIdPropertyInternal(newValue);
}

const test::BuiltInTypes::Update UBuiltInTypesComponentUpdate::GetUnderlying()
{
    return GetUnderlyingInternal();
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::InitInternal(const test::BuiltInTypes::Update& underlying)
{
    Underlying = test::BuiltInTypes::Update(underlying);
	return this;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::ResetInternal()
{
	return InitInternal(DefaultUnderlying);
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetBoolPropertyInternal(bool newValue)
{
    Underlying.set_bool_property(newValue);
	return this;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetUint32PropertyInternal(int newValue)
{
    Underlying.set_uint32_property(static_cast<std::uint32_t>(newValue));
	return this;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetInt32PropertyInternal(int newValue)
{
    Underlying.set_int32_property(static_cast<std::int32_t>(newValue));
	return this;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetFloatPropertyInternal(float newValue)
{
    Underlying.set_float_property(newValue);
	return this;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetDoublePropertyInternal(float newValue)
{
    Underlying.set_double_property(static_cast<double>(newValue));
	return this;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetStringPropertyInternal(FString newValue)
{
    Underlying.set_string_property(TCHAR_TO_UTF8(*newValue));
	return this;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetBytesPropertyInternal(FString newValue)
{
    Underlying.set_bytes_property(TCHAR_TO_UTF8(*newValue));
	return this;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetCoordinatesPropertyInternal(FVector newValue)
{
    Underlying.set_coordinates_property(USpatialOSConversionFunctionLibrary::UnrealCoordinatesToSpatialOsCoordinatesCast(newValue));
	return this;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetVector3dPropertyInternal(FVector newValue)
{
    Underlying.set_vector3d_property(improbable::Vector3d(static_cast<double>(newValue.X), static_cast<double>(newValue.Y), static_cast<double>(newValue.Z)));
	return this;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetVector3fPropertyInternal(FVector newValue)
{
    Underlying.set_vector3f_property(improbable::Vector3f(static_cast<double>(newValue.X), static_cast<double>(newValue.Y), static_cast<double>(newValue.Z)));
	return this;
}

UBuiltInTypesComponentUpdate* UBuiltInTypesComponentUpdate::SetEntityIdPropertyInternal(FEntityId newValue)
{
    Underlying.set_entity_id_property((newValue).ToSpatialEntityId());
	return this;
}

const test::BuiltInTypes::Update UBuiltInTypesComponentUpdate::GetUnderlyingInternal()
{
    return Underlying;
}
