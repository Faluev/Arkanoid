#pragma once

#include "World/BonusParent.h"
#include "SizeBonus.generated.h"

UCLASS()
class ARKANOID_API ASizeBonus : public ABonusParent
{
    GENERATED_BODY()

public:
    ASizeBonus();

protected:
    virtual void ApplyTo(APaddle& Paddle) const override;
};
