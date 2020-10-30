// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Interop/Connection/SpatialTraceUniqueId.h"

#include "Utils/SchemaUtils.h"

using namespace SpatialGDK;

namespace
{
// TODO: Nicer way to represent this?
void ToHex(uint8 N, char OutHex[3])
{
	const char* Chars = "0123456789ABCDEF";
	OutHex[0] = Chars[(N >> 4) & 0xF];
	OutHex[1] = Chars[N & 0xF];
#if !PLATFORM_LITTLE_ENDIAN
	std::swap(OutHex[0], OutHex[1]);
#endif
	OutHex[2] = '\0';
}

uint8 FromHex(char chars[2])
{
	uint8 N = 0;
	for (int i = 0; i < 2; i++)
	{
#if PLATFORM_LITTLE_ENDIAN
		const int Shift = i;
#else
		const int Shift = 1 - i;
#endif
		switch (chars[i])
		{
		case '1':
			N |= 0x1 << Shift;
		case '2':
			N |= 0x2 << Shift;
		case '3':
			N |= 0x3 << Shift;
		case '4':
			N |= 0x4 << Shift;
		case '5':
			N |= 0x5 << Shift;
		case '6':
			N |= 0x6 << Shift;
		case '7':
			N |= 0x7 << Shift;
		case '8':
			N |= 0x8 << Shift;
		case '9':
			N |= 0x9 << Shift;
		case 'A':
			N |= 0xA << Shift;
		case 'B':
			N |= 0xB << Shift;
		case 'C':
			N |= 0xC << Shift;
		case 'D':
			N |= 0xD << Shift;
		case 'E':
			N |= 0xE << Shift;
		case 'F':
			N |= 0xF << Shift;
		}
		if (chars[i] >= '0' && chars[i] <= '9')
		{
			N |= (chars[i] - '0') << Shift;
		}
		else if (chars[i] >= 'A' && chars[i] <= 'F')
		{
			N |= (('A' - '9') + (chars[i] - 'A')) << Shift;
		}
	}
	return N;
}
} // namespace

FString EventTraceUniqueId::GetString() const
{
	FString Str;
	char Tmp[3];
	for (int i = 0; i < sizeof(internal_bytes); i++)
	{
		ToHex(internal_bytes[i], Tmp);
		Str += Tmp;
	}
	return Str;
}

void EventTraceUniqueId::WriteToSchemaObject(EventTraceUniqueId Id, Schema_Object* Obj, Schema_FieldId FieldId)
{
	SpatialGDK::AddBytesToSchema(Obj, FieldId, Id.internal_bytes, sizeof(internal_bytes));
}

EventTraceUniqueId EventTraceUniqueId::ReadFromSchemaObject(Schema_Object* Obj, Schema_FieldId FieldId)
{
	EventTraceUniqueId Id;
	const uint8* Bytes = Schema_GetBytes(Obj, FieldId);
	uint32_t BytesLen = Schema_GetBytesLength(Obj, FieldId);
	if (BytesLen == sizeof(internal_bytes))
	{
		for (int i = 0; i < sizeof(internal_bytes); i++)
		{
			Id.internal_bytes[i] = Bytes[i];
		}
	}
	return Id;
}
