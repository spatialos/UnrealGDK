#pragma once

#include "CoreMinimal.h"
#include "LoadBalancing/GridBasedLBStrategy.h"
#include "GameFramework/DefaultPawn.h"
#include "TestLayeredLBStrategy.generated.h"

/**
 * This class is for testing purposes only.
 */
UCLASS(HideDropdown)
class SPATIALGDKTESTS_API UTwoByFourLBGridStrategy : public UGridBasedLBStrategy {
	GENERATED_BODY()

public:
	UTwoByFourLBGridStrategy(): Super() {
		Rows = 2;
		Cols = 4;
	}
};

/**
 * Same as a Default pawn but for testing
 */
UCLASS(config = Game, Blueprintable, BlueprintType)
class SPATIALGDKTESTS_API ASpecialPawn : public ADefaultPawn
{
	GENERATED_BODY()
};
