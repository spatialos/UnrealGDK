// Copyright (c) Improbable Worlds Ltd, All Rights Reserved

#include "Utils/InspectionColors.h"

#include "Containers/UnrealString.h"
#include "Math/Color.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/Char.h"

#include "cstring"

namespace SpatialGDK
{
namespace
{
const int32 MIN_HUE = 10;
const int32 MAX_HUE = 350;
const int32 MIN_SATURATION = 60;
const int32 MAX_SATURATION = 100;
const int32 MIN_LIGHTNESS = 25;
const int32 MAX_LIGHTNESS = 60;

int64 GenerateValueFromThresholds(int64 Hash, int32 Min, int32 Max)
{
	return Hash % FMath::Abs(Max - Min) + Min;
}

FColor HSLtoRGB(double Hue, double Saturation, double Lightness)
{
	// const[h, s, l] = hsl;
	// Must be fractions of 1

	const double c = (1 - FMath::Abs(2 * Lightness / 100 - 1)) * Saturation / 100;
	const double x = c * (1 - FMath::Abs(FMath::Fmod((Hue / 60), 2) - 1));
	const double m = Lightness / 100 - c / 2;

	double r = 0;
	double g = 0;
	double b = 0;

	if (0 <= Hue && Hue < 60)
	{
		r = c;
		g = x;
		b = 0;
	}
	else if (60 <= Hue && Hue < 120)
	{
		r = x;
		g = c;
		b = 0;
	}
	else if (120 <= Hue && Hue < 180)
	{
		r = 0;
		g = c;
		b = x;
	}
	else if (180 <= Hue && Hue < 240)
	{
		r = 0;
		g = x;
		b = c;
	}
	else if (240 <= Hue && Hue < 300)
	{
		r = x;
		g = 0;
		b = c;
	}
	else if (300 <= Hue && Hue <= 360)
	{
		r = c;
		g = 0;
		b = x;
	}
	r = (r + m) * 255;
	g = (g + m) * 255;
	b = (b + m) * 255;

	return FColor{ static_cast<uint8>(r), static_cast<uint8>(g), static_cast<uint8>(b) };
}

int64 DJBReverseHash(const PhysicalWorkerName& WorkerName)
{
	const int32 StringLength = WorkerName.Len();
	int64 Hash = 5381;
	for (int32 i = StringLength - 1; i > 0; --i)
	{
		// We're mimicking the Inspector logic which is in JS. In JavaScript,
		// a number is stored as a 64-bit floating point number but the bit-wise
		// operation is performed on a 32-bit integer i.e. to perform a
		// bit-operation JavaScript converts the number into a 32-bit binary
		// number (signed) and perform the operation and convert back the result
		// to a 64-bit number.
		// Ideally, this would just be ((static_cast<int32>(Hash)) << 5) but left
		// shifting a signed int with overflow is undefined so we have to memcpy
		// to an unsigned.
		uint64 BitShiftingScratchRegister;
		std::memcpy(&BitShiftingScratchRegister, &Hash, sizeof(int64));
		int32 BitShiftedHash = static_cast<int32>((BitShiftingScratchRegister << 5) & 0xFFFFFFFF);
		Hash = BitShiftedHash + Hash + static_cast<int32>(WorkerName[i]);
	}
	return FMath::Abs(Hash);
}
} // namespace

FColor GetColorForWorkerName(const PhysicalWorkerName& WorkerName)
{
	int64 Hash = DJBReverseHash(WorkerName);

	const double Lightness = GenerateValueFromThresholds(Hash, MIN_LIGHTNESS, MAX_LIGHTNESS);
	const double Saturation = GenerateValueFromThresholds(Hash, MIN_SATURATION, MAX_SATURATION);
	// Provides additional color variance for potentially sequential hashes
	auto abs = FMath::Abs((double)Hash / Saturation + Lightness);
	Hash = FMath::FloorToInt(abs);
	const double Hue = GenerateValueFromThresholds(Hash, MIN_HUE, MAX_HUE);

	return SpatialGDK::HSLtoRGB(Hue, Saturation, Lightness);
}
} // namespace SpatialGDK
