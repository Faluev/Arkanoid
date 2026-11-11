#pragma once

#include "World/BonusParent.h"
#include "ShieldBonus.generated.h"

UCLASS()
class ARKANOID_API AShieldBonus : public ABonusParent
{
    GENERATED_BODY()

public:
    AShieldBonus();

protected:
    virtual void ApplyTo(APaddle& Paddle) const override;
};
