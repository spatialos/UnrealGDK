#pragma once

#include "ComponentIdentifier.h"
#include "CoreMinimal.h"
#include "SpatialGDKWorkerTypes.h"

#include "RequestId.generated.h"

/**
* FRequestId is a USTRUCT wrapper for a uint32_t request id.
* FRequestId has a bValidId field to indicate whether the underlying request id
* is set properly.
* When a RequestId is converted to a FRequestId, its request type is lost.
* FRequestIds are mostly used as unique identifiers to link up requests sent
* with their request ids.
*/
USTRUCT(BlueprintType)
struct SPATIALGDK_API FRequestId
{
	GENERATED_USTRUCT_BODY()

  public:
	FORCEINLINE FRequestId()
	{
		Underlying = 0;
		bValidId = false;
	}

	FORCEINLINE FRequestId(const FRequestId& Other)
	{
		Underlying = Other.Underlying;
		bValidId = Other.bValidId;
	}

	FORCEINLINE FRequestId(const std::uint32_t UnderlyingRequestId, const bool bIsValidId)
	{
		Underlying = UnderlyingRequestId;
		bValidId = bIsValidId;
	}

	FORCEINLINE FRequestId& operator=(const FRequestId& Other)
	{
		Underlying = Other.Underlying;
		bValidId = Other.bValidId;
		return *this;
	}

	FORCEINLINE bool operator==(const FRequestId& Other) const
	{
		return Underlying == Other.Underlying && bValidId == Other.bValidId;
	}

	template <typename T>
	FORCEINLINE bool operator==(const worker::RequestId<T>& Other) const
	{
		return Underlying == Other.Id && bValidId;
	}

	FORCEINLINE bool operator!=(const FRequestId& Other) const
	{
		return Underlying != Other.Underlying || bValidId != Other.bValidId;
	}

	/**
  * This method returns the wrapped uint32_t request id
  */
	std::uint32_t GetUnderlying() const
	{
		return Underlying;
	}

	/**
  * return if the request id is a valid request id or not
  */
	bool IsValid() const
	{
		return bValidId;
	}

  private:
	std::uint32_t Underlying;
	bool bValidId;

	friend uint32 GetTypeHash(const FRequestId& Rhs)
	{
		return FComponentIdentifier::HashRequestId(Rhs.Underlying);
	}
};