#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ArkanoidArcadeController.generated.h"

UCLASS()
class ARKANOID_API AArkanoidArcadeController : public APlayerController
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Arkanoid|Menu")
    void StartGame();

    UFUNCTION(BlueprintCallable, Category = "Arkanoid|Menu")
    void ReturnToMenu();

    void QuitGame();
    void ShowMenuInput();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    void ActivatePrimaryAction();
    void TravelTo(FName MapName);

    bool bTravelPending = false;
};
