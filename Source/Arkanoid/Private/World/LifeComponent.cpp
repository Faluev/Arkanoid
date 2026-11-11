#include "World/LifeComponent.h"

ULifeComponent::ULifeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void ULifeComponent::SetLife(int32 Amount)
{
    Life = FMath::Max(1, Amount);
}

void ULifeComponent::ApplyDamage(int32 Amount)
{
    Life = FMath::Max(0, Life - FMath::Max(0, Amount));
}
