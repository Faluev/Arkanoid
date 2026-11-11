#pragma once

#include "World/BonusParent.h"
#include "LifeBonus.generated.h"

UCLASS()
class ARKANOID_API ALifeBonus : public ABonusParent
{
    GENERATED_BODY()

public:
    ALifeBonus();

protected:
    virtual void ApplyTo(APaddle& Paddle) const override;
};
