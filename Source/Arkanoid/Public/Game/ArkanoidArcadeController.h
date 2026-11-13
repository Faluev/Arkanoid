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

    UFUNCTION(BlueprintCallable, Category = "Arkanoid|Menu")
    void SelectArena(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Arkanoid|Menu")
    void NextArena();

    UFUNCTION(BlueprintCallable, Category = "Arkanoid|Menu")
    void TogglePause();

    UFUNCTION(BlueprintPure, Category = "Arkanoid|Menu")
    bool IsArcadePaused() const { return bArcadePaused; }

    int32 GetArenaIndex() const;
    bool IsOrbitalGame() const;

    void QuitGame();
    void ShowMenuInput();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

private:
    void ActivatePrimaryAction();
    void SelectDock();
    void SelectReactor();
    void SelectCore();
    void ShowGameInput();
    void TravelTo(FName MapName);

    bool bTravelPending = false;
    bool bArcadePaused = false;
};
