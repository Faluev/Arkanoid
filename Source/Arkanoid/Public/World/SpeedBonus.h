#pragma once

#include "World/BonusParent.h"
#include "SpeedBonus.generated.h"

UCLASS()
class ARKANOID_API ASpeedBonus : public ABonusParent
{
    GENERATED_BODY()

public:
    ASpeedBonus();

protected:
    virtual void ApplyTo(APaddle& Paddle) const override;
};
