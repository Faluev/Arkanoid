#include "Game/ArkanoidHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Game/ArkanoidArcadeController.h"
#include "Game/ArkanoidGameInstance.h"
#include "Game/ArkanoidGameMode.h"
#include "World/Ball.h"
#include "World/Paddle.h"
#include "World/PlayingBoard.h"

namespace
{
const FName PlayButton(TEXT("Play"));
const FName MenuButton(TEXT("Menu"));
const FName ExitButton(TEXT("Exit"));
}

void AArkanoidHUD::DrawHUD()
{
    Super::DrawHUD();

    const APaddle* Paddle = Cast<APaddle>(GetOwningPawn());
    if (!Canvas)
    {
        return;
    }

    const float Unit = FMath::Min(Canvas->ClipX / 1280.0f, Canvas->ClipY / 720.0f);
    const float OriginX = (Canvas->ClipX - 1280.0f * Unit) * 0.5f;
    const float OriginY = (Canvas->ClipY - 720.0f * Unit) * 0.5f;
    const FLinearColor Ink(0.93f, 0.96f, 1.0f);
    const FLinearColor Muted(0.52f, 0.64f, 0.78f);
    const FLinearColor Panel(0.025f, 0.045f, 0.09f, 0.94f);
    const FLinearColor Edge(0.16f, 0.27f, 0.43f);
    const FLinearColor Accent(0.26f, 0.91f, 0.80f);
    UFont* const Small = GEngine->GetSmallFont();
    UFont* const Medium = GEngine->GetMediumFont();
    UFont* const Large = GEngine->GetLargeFont();

    const auto Rect = [this, OriginX, OriginY, Unit](const FLinearColor Color, float X, float Y, float Width, float Height)
    {
        DrawRect(Color, OriginX + X * Unit, OriginY + Y * Unit, Width * Unit, Height * Unit);
    };
    const auto Text = [this, OriginX, OriginY, Unit](const FString& Value, const FLinearColor Color, float X, float Y,
        UFont* Font, float Scale = 1.0f)
    {
        DrawText(Value, Color, OriginX + X * Unit, OriginY + Y * Unit, Font, Scale * Unit);
    };
    const auto CenterText = [this, &Text](const FString& Value, const FLinearColor& Color,
        float CenterX, float Y, UFont* Font, float Scale)
    {
        float Width = 0.0f;
        float Height = 0.0f;
        GetTextSize(Value, Width, Height, Font, Scale);
        Text(Value, Color, CenterX - Width * 0.5f, Y, Font, Scale);
    };
    const auto Button = [this, &Rect, &CenterText, OriginX, OriginY, Unit, Medium, Ink, Edge, Accent](
        FName Name, const TCHAR* Label, float X, float Y, float Width, bool bPrimary)
    {
        constexpr float Height = 52.0f;
        Rect(bPrimary ? Accent : Edge, X, Y, Width, Height);
        CenterText(Label, bPrimary ? FLinearColor(0.035f, 0.075f, 0.11f) : Ink,
            X + Width * 0.5f, Y + 16.0f, Medium, 1.45f);
        AddHitBox(FVector2D(OriginX + X * Unit, OriginY + Y * Unit),
            FVector2D(Width * Unit, Height * Unit), Name, true);
    };

    if (GetWorld()->GetAuthGameMode<AArkanoidMenuMode>())
    {
        const UArkanoidGameInstance* Instance = GetGameInstance<UArkanoidGameInstance>();
        const int32 Best = Instance ? Instance->GetBestScore() : 0;
        Rect(Panel, 366, 145, 548, 428);
        Rect(Accent, 366, 145, 548, 3);
        CenterText(TEXT("ARKANOID"), Ink, 640, 191, Large, 4.1f);
        CenterText(TEXT("ARCADE"), Accent, 640, 259, Medium, 1.6f);
        CenterText(FString::Printf(TEXT("BEST SCORE   %d"), Best), Ink, 640, 313, Medium, 1.7f);
        Button(PlayButton, TEXT("PLAY"), 432, 383, 416, true);
        Button(ExitButton, TEXT("EXIT"), 432, 447, 416, false);
        CenterText(TEXT("ENTER TO PLAY"), Muted, 640, 522, Small, 1.2f);
        return;
    }

    if (!Paddle)
    {
        return;
    }
    if (!PlayingBoard.IsValid())
    {
        for (TActorIterator<APlayingBoard> Board(GetWorld()); Board; ++Board)
        {
            PlayingBoard = *Board;
            break;
        }
    }

    const AArkanoidArcadeMode* Arcade = GetWorld()->GetAuthGameMode<AArkanoidArcadeMode>();

    Text(TEXT("ARKANOID"), Ink, 38, 50, Large, 2.45f);
    Text(Arcade ? TEXT("ARCADE RUN") : TEXT("BONUS ARENA"), Accent, 40, 104, Medium, 1.2f);
    Rect(Panel, 38, 157, 274, 282);
    Rect(Edge, 38, 157, 274, 2);

    const ABall* Ball = Paddle->GetCurrentBall();
    const int32 Blocks = PlayingBoard.IsValid() ? PlayingBoard->GetRemainingBlocks() : 0;
    const auto Stat = [&](const FString& Label, const FString& Value, float Y, const FLinearColor Color)
    {
        Text(Label, Muted, 57, Y, Medium, 1.25f);
        Text(Value, Color, 212, Y, Medium, 1.25f);
    };

    Stat(TEXT("Lives"), FString::FromInt(Paddle->GetLives()), 177, Ink);
    Stat(TEXT("Blocks"), FString::FromInt(Blocks), 221, Ink);
    Stat(TEXT("Ball speed"), Ball ? FString::FromInt(FMath::RoundToInt(Ball->GetSpeed())) : TEXT("-"), 265, Ink);
    Stat(TEXT("Ball power"), Ball ? FString::FromInt(Ball->GetPower()) : TEXT("-"), 309, Ink);
    Stat(TEXT("Paddle"), FString::Printf(TEXT("%d%%"), FMath::RoundToInt(Paddle->GetSizeMultiplier() * 100.0f)), 353, Ink);
    Stat(TEXT("Shield"), Paddle->HasShield() ? TEXT("ON") : TEXT("OFF"), 397, Paddle->HasShield() ? Accent : Muted);

    Rect(Panel, 968, 157, 274, 310);
    Rect(Edge, 968, 157, 274, 2);
    Text(TEXT("BONUSES"), Ink, 988, 173, Medium, 1.25f);
    struct FLegendItem
    {
        const TCHAR* Label;
        FLinearColor Color;
    };
    const FLegendItem Legend[] = {
        { TEXT("Wider paddle"), FLinearColor(0.20f, 0.85f, 0.52f) },
        { TEXT("Extra life"), FLinearColor(0.14f, 0.64f, 1.0f) },
        { TEXT("Ball speed"), FLinearColor(1.0f, 0.68f, 0.12f) },
        { TEXT("Ball power"), FLinearColor(0.62f, 0.31f, 1.0f) },
        { TEXT("Destroy blocks"), FLinearColor(1.0f, 0.31f, 0.16f) },
        { TEXT("Safety shield"), FLinearColor(0.96f, 0.26f, 0.61f) },
    };
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(Legend); ++Index)
    {
        const float Y = 222.0f + 36.0f * Index;
        Rect(Legend[Index].Color, 989, Y + 5, 11, 11);
        Text(Legend[Index].Label, Ink, 1011, Y, Medium, 1.15f);
    }

    const float ControlsY = Arcade ? 582.0f : 496.0f;
    if (Arcade)
    {
        Rect(Panel, 38, 452, 274, 109);
        Rect(Accent, 38, 452, 274, 2);
        Stat(TEXT("Score"), FString::FromInt(Arcade->GetScore()), 469, Ink);
        Stat(TEXT("Best score"), FString::FromInt(Arcade->GetBestScore()), 514, Accent);
        Text(TEXT("ESC     Main menu"), Muted, 40, ControlsY + 84, Small, 1.3f);
    }
    Text(TEXT("A / D  or  arrows     Move"), Muted, 40, ControlsY, Small, 1.3f);
    Text(TEXT("SPACE     Launch ball"), Muted, 40, ControlsY + 28, Small, 1.3f);
    Text(TEXT("R     Restart arena"), Muted, 40, ControlsY + 56, Small, 1.3f);

    if (Arcade && Arcade->HasFinished())
    {
        Rect(FLinearColor(0.01f, 0.02f, 0.04f, 0.97f), 390, 211, 500, 300);
        Rect(Accent, 390, 211, 500, 3);
        CenterText(Arcade->HasWon() ? TEXT("FIELD CLEARED") : TEXT("OUT OF LIVES"),
            Ink, 640, 252, Large, 2.75f);
        CenterText(FString::Printf(TEXT("SCORE  %d     BEST  %d"), Arcade->GetScore(), Arcade->GetBestScore()),
            Accent, 640, 328, Medium, 1.45f);
        Button(PlayButton, TEXT("PLAY AGAIN"), 430, 405, 196, true);
        Button(MenuButton, TEXT("MAIN MENU"), 653, 405, 196, false);
        CenterText(TEXT("ENTER / R TO REPLAY     ESC FOR MENU"), Muted, 640, 477, Small, 1.12f);
        return;
    }

    const TCHAR* Message = nullptr;
    if (PlayingBoard.IsValid() && Blocks == 0)
    {
        Message = TEXT("FIELD CLEARED");
    }
    else if (Paddle->GetLives() == 0)
    {
        Message = TEXT("OUT OF LIVES");
    }
    else if (Ball && !Ball->IsLaunched())
    {
        Message = TEXT("SPACE TO LAUNCH");
    }
    if (Message)
    {
        float Width = 0.0f;
        float Height = 0.0f;
        GetTextSize(Message, Width, Height, Medium, 1.0f);
        Rect(Panel, 640 - Width * 0.5f - 18, 666, Width + 36, Height + 14);
        Text(Message, Accent, 640 - Width * 0.5f, 673, Medium);
    }
}

void AArkanoidHUD::NotifyHitBoxClick(FName BoxName)
{
    Super::NotifyHitBoxClick(BoxName);
    if (AArkanoidArcadeController* Player = Cast<AArkanoidArcadeController>(GetOwningPlayerController()))
    {
        if (BoxName == PlayButton)
        {
            Player->StartGame();
        }
        else if (BoxName == MenuButton)
        {
            Player->ReturnToMenu();
        }
        else if (BoxName == ExitButton)
        {
            Player->QuitGame();
        }
    }
}
