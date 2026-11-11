#pragma once

#include "World/BonusParent.h"
#include "PowerBonus.generated.h"

UCLASS()
class ARKANOID_API APowerBonus : public ABonusParent
{
    GENERATED_BODY()

public:
    APowerBonus();

protected:
    virtual void ApplyTo(APaddle& Paddle) const override;
};
