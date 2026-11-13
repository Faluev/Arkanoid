#pragma once

#include "World/BonusParent.h"
#include "StickyBallBonus.generated.h"

UCLASS()
class ARKANOID_API AStickyBallBonus : public ABonusParent
{
    GENERATED_BODY()

public:
    AStickyBallBonus();

protected:
    virtual void ApplyTo(APaddle& Paddle) const override;
};
