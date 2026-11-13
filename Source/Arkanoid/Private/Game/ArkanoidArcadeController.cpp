#include "Game/ArkanoidArcadeController.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Game/ArkanoidGameMode.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/PackageName.h"

namespace
{
const FName MenuMap(TEXT("/Game/Arkanoid/Maps/L_ArcadeMenu"));
const FName ArenaMap(TEXT("/Game/Arkanoid/Maps/L_ArcadeArena"));
const FName OrbitalMenu(TEXT("/Game/Arkanoid/Maps/L_OrbitalMenu"));
const FName OrbitalArenas[] = {
    TEXT("/Game/Arkanoid/Maps/L_OrbitalDock"),
    TEXT("/Game/Arkanoid/Maps/L_PlasmaReactor"),
    TEXT("/Game/Arkanoid/Maps/L_GravityCore")
};

bool IsCurrentMap(const APlayerController* Controller, FName Path)
{
    return UGameplayStatics::GetCurrentLevelName(Controller) == FPackageName::GetShortName(Path);
}
}

void AArkanoidArcadeController::BeginPlay()
{
    Super::BeginPlay();
    if (GetWorld()->GetAuthGameMode<AArkanoidMenuMode>())
    {
        ShowMenuInput();
    }
    else if (IsOrbitalGame())
    {
        ShowGameInput();
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
        &AArkanoidArcadeController::TogglePause).bExecuteWhenPaused = true;
    InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AArkanoidArcadeController::SelectDock);
    InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AArkanoidArcadeController::SelectReactor);
    InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AArkanoidArcadeController::SelectCore);
}

void AArkanoidArcadeController::ActivatePrimaryAction()
{
    if (bArcadePaused)
    {
        TogglePause();
        return;
    }
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

void AArkanoidArcadeController::ShowGameInput()
{
    bShowMouseCursor = false;
    bEnableClickEvents = false;
    FInputModeGameOnly InputMode;
    InputMode.SetConsumeCaptureMouseDown(false);
    SetInputMode(InputMode);
}

int32 AArkanoidArcadeController::GetArenaIndex() const
{
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(OrbitalArenas); ++Index)
    {
        if (IsCurrentMap(this, OrbitalArenas[Index]))
        {
            return Index;
        }
    }
    return INDEX_NONE;
}

bool AArkanoidArcadeController::IsOrbitalGame() const
{
    return GetArenaIndex() != INDEX_NONE || IsCurrentMap(this, OrbitalMenu);
}

void AArkanoidArcadeController::SelectDock()
{
    SelectArena(0);
}

void AArkanoidArcadeController::SelectReactor()
{
    SelectArena(1);
}

void AArkanoidArcadeController::SelectCore()
{
    SelectArena(2);
}

void AArkanoidArcadeController::SelectArena(int32 Index)
{
    if (IsCurrentMap(this, OrbitalMenu) && Index >= 0 && Index < UE_ARRAY_COUNT(OrbitalArenas))
    {
        TravelTo(OrbitalArenas[Index]);
    }
}

void AArkanoidArcadeController::NextArena()
{
    const AArkanoidArcadeMode* Arcade = GetWorld()->GetAuthGameMode<AArkanoidArcadeMode>();
    const int32 Index = GetArenaIndex();
    if (Arcade && Arcade->HasWon() && Index >= 0 && Index + 1 < UE_ARRAY_COUNT(OrbitalArenas))
    {
        TravelTo(OrbitalArenas[Index + 1]);
    }
}

void AArkanoidArcadeController::TogglePause()
{
    const AArkanoidArcadeMode* Arcade = GetWorld()->GetAuthGameMode<AArkanoidArcadeMode>();
    if (!Arcade)
    {
        return;
    }
    if (!IsOrbitalGame() || Arcade->HasFinished())
    {
        ReturnToMenu();
        return;
    }

    // The controller stays available while level time and ball motion are paused.
    const bool bPause = !bArcadePaused;
    if (SetPause(bPause))
    {
        bArcadePaused = bPause;
        if (bPause)
        {
            ShowMenuInput();
        }
        else
        {
            ShowGameInput();
        }
    }
}

void AArkanoidArcadeController::StartGame()
{
    const int32 Index = GetArenaIndex();
    if (Index != INDEX_NONE)
    {
        TravelTo(OrbitalArenas[Index]);
    }
    else if (IsCurrentMap(this, OrbitalMenu))
    {
        SelectDock();
    }
    else
    {
        TravelTo(ArenaMap);
    }
}

void AArkanoidArcadeController::ReturnToMenu()
{
    if (GetWorld()->GetAuthGameMode<AArkanoidArcadeMode>())
    {
        TravelTo(IsOrbitalGame() ? OrbitalMenu : MenuMap);
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
