#pragma once

#include "World/BonusParent.h"
#include "DestroyBlocksBonus.generated.h"

UCLASS()
class ARKANOID_API ADestroyBlocksBonus : public ABonusParent
{
    GENERATED_BODY()

public:
    ADestroyBlocksBonus();

protected:
    virtual void ApplyTo(APaddle& Paddle) const override;
};
