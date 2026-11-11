#include "Game/ArkanoidHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "World/Ball.h"
#include "World/Paddle.h"
#include "World/PlayingBoard.h"

void AArkanoidHUD::DrawHUD()
{
    Super::DrawHUD();

    const APaddle* Paddle = Cast<APaddle>(GetOwningPawn());
    if (!Canvas || !Paddle)
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

    Text(TEXT("ARKANOID"), Ink, 38, 50, Large, 2.45f);
    Text(TEXT("BONUS ARENA"), Accent, 40, 104, Medium, 1.2f);
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

    Text(TEXT("A / D  or  arrows     Move"), Muted, 40, 496, Small, 1.3f);
    Text(TEXT("SPACE     Launch ball"), Muted, 40, 524, Small, 1.3f);
    Text(TEXT("R     Restart arena"), Muted, 40, 552, Small, 1.3f);

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
