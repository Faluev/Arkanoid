#include "Game/ArkanoidArcadeController.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Game/ArkanoidGameMode.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
const FName MenuMap(TEXT("/Game/Arkanoid/Maps/L_ArcadeMenu"));
const FName ArenaMap(TEXT("/Game/Arkanoid/Maps/L_ArcadeArena"));
}

void AArkanoidArcadeController::BeginPlay()
{
    Super::BeginPlay();
    if (GetWorld()->GetAuthGameMode<AArkanoidMenuMode>())
    {
        ShowMenuInput();
    }
}

void AArkanoidArcadeController::SetupInputComponent()
{
    Super::SetupInputComponent();

    InputComponent->BindKey(EKeys::Enter, IE_Pressed, this,
        &AArkanoidArcadeController::ActivatePrimaryAction).bExecuteWhenPaused = true;
    InputComponent->BindKey(EKeys::R, IE_Pressed, this,
        &AArkanoidArcadeController::StartGame).bExecuteWhenPaused = true;
    InputComponent->BindKey(EKeys::Escape, IE_Pressed, this,
        &AArkanoidArcadeController::ReturnToMenu).bExecuteWhenPaused = true;
}

void AArkanoidArcadeController::ActivatePrimaryAction()
{
    const AArkanoidArcadeMode* Arcade = GetWorld()->GetAuthGameMode<AArkanoidArcadeMode>();
    if (!Arcade || Arcade->HasFinished())
    {
        StartGame();
    }
}

void AArkanoidArcadeController::ShowMenuInput()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);
}

void AArkanoidArcadeController::StartGame()
{
    TravelTo(ArenaMap);
}

void AArkanoidArcadeController::ReturnToMenu()
{
    if (GetWorld()->GetAuthGameMode<AArkanoidArcadeMode>())
    {
        TravelTo(MenuMap);
    }
}

void AArkanoidArcadeController::TravelTo(FName MapName)
{
    if (bTravelPending)
    {
        return;
    }
    bTravelPending = true;

    if (AArkanoidGameMode* Mode = GetWorld()->GetAuthGameMode<AArkanoidGameMode>())
    {
        Mode->PlayMenuClick();
    }
    SetPause(false);
    UGameplayStatics::OpenLevel(this, MapName);
}

void AArkanoidArcadeController::QuitGame()
{
    UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, false);
}
