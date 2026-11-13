#include "Game/ArkanoidHUD.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Game/ArkanoidArcadeController.h"
#include "Game/ArkanoidGameInstance.h"
#include "Game/ArkanoidGameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "World/Ball.h"
#include "World/Paddle.h"
#include "World/PlayingBoard.h"

namespace
{
const FName PlayButton(TEXT("Play"));
const FName MenuButton(TEXT("Menu"));
const FName ExitButton(TEXT("Exit"));
const FName ResumeButton(TEXT("Resume"));
const FName NextButton(TEXT("Next"));
const FName ArenaButtons[] = { TEXT("Dock"), TEXT("Reactor"), TEXT("Core") };
const TCHAR* ArenaNames[] = { TEXT("ORBITAL DOCK"), TEXT("PLASMA REACTOR"), TEXT("GRAVITY CORE") };
const TCHAR* ArenaDetails[] = { TEXT("Breakthrough lanes"), TEXT("Bank off reflectors"), TEXT("Split the formation") };
const FLinearColor ArenaColors[] = {
    FLinearColor(0.32f, 0.93f, 0.94f),
    FLinearColor(1.0f, 0.65f, 0.27f),
    FLinearColor(0.73f, 0.58f, 1.0f),
};

const FLinearColor OrbitalWhite(0.91f, 0.96f, 1.0f);
const FLinearColor OrbitalMuted(0.46f, 0.57f, 0.69f);
const FLinearColor OrbitalPanel(0.012f, 0.021f, 0.035f, 0.90f);
const FLinearColor OrbitalEdge(0.052f, 0.09f, 0.14f, 0.95f);

class FOrbitalCanvas
{
public:
    FOrbitalCanvas(AHUD& InHUD, UCanvas& InCanvas, const APlayerController& Controller, const UFont* InFont)
        : HUD(InHUD), Target(InCanvas), Font(InFont)
    {
        Unit = FMath::Min(Target.ClipX / 1280.0f, Target.ClipY / 720.0f);
        Origin = FVector2D((Target.ClipX - 1280.0f * Unit) * 0.5f,
            (Target.ClipY - 720.0f * Unit) * 0.5f);
        float X;
        float Y;
        if (Controller.bShowMouseCursor && Controller.GetMousePosition(X, Y))
        {
            Pointer = (FVector2D(X, Y) - Origin) / Unit;
        }
    }

    void Rect(const FLinearColor& Color, float X, float Y, float W, float H) const
    {
        if (W > 0.0f && H > 0.0f)
        {
            HUD.DrawRect(Color, Origin.X + X * Unit, Origin.Y + Y * Unit, W * Unit, H * Unit);
        }
    }

    void Text(const FString& Value, const FLinearColor& Color, float X, float Y, float Size,
        FName Face = TEXT("Regular"), bool bCentered = false) const
    {
        const FVector2D Position = Origin + FVector2D(X, Y) * Unit;
        FCanvasTextItem Item(FVector2D(FMath::RoundToDouble(Position.X), FMath::RoundToDouble(Position.Y)),
            FText::FromString(Value), FSlateFontInfo(Font, Size * Unit, Face), Color);
        Item.bCentreX = bCentered;
        Item.bDontCorrectStereoscopic = true;
        Target.DrawItem(Item);
    }

    void Line(const FLinearColor& Color, FVector2D A, FVector2D B, float Width = 1.0f) const
    {
        const FVector2D Start = Origin + A * Unit;
        const FVector2D End = Origin + B * Unit;
        HUD.DrawLine(Start.X, Start.Y, End.X, End.Y, Color, Width * Unit);
    }

    void Fill(const FLinearColor& Color) const
    {
        HUD.DrawRect(Color, 0.0f, 0.0f, Target.ClipX, Target.ClipY);
    }

    void Cover(UTexture2D& Texture) const
    {
        const float Scale = FMath::Max(Target.ClipX / Texture.GetSizeX(), Target.ClipY / Texture.GetSizeY());
        const FVector2D Size(Texture.GetSizeX() * Scale, Texture.GetSizeY() * Scale);
        const FVector2D Position = (FVector2D(Target.ClipX, Target.ClipY) - Size) * 0.5f;
        FCanvasTileItem Tile(Position, Texture.GetResource(), Size, FLinearColor::White);
        Tile.BlendMode = SE_BLEND_Opaque;
        Target.DrawItem(Tile);
    }

    bool Hover(float X, float Y, float W, float H) const
    {
        return Pointer.X >= X && Pointer.X < X + W && Pointer.Y >= Y && Pointer.Y < Y + H;
    }

    void HitBox(FName Name, float X, float Y, float W, float H) const
    {
        HUD.AddHitBox(Origin + FVector2D(X, Y) * Unit, FVector2D(W, H) * Unit, Name, true);
    }

    void Button(FName Name, const TCHAR* Label, const FLinearColor& Accent, bool bPrimary,
        float X, float Y, float W, float H) const
    {
        const bool bHover = Hover(X, Y, W, H);
        Rect(bHover ? Accent : OrbitalEdge, X, Y, W, H);
        Rect(bPrimary ? (bHover ? OrbitalWhite : Accent) : OrbitalPanel, X + 1, Y + 1, W - 2, H - 2);
        Text(Label, bPrimary ? OrbitalPanel : OrbitalWhite, X + W * 0.5f, Y + (H - 22.0f) * 0.5f,
            14, TEXT("Bold"), true);
        HitBox(Name, X, Y, W, H);
    }

private:
    AHUD& HUD;
    UCanvas& Target;
    const UFont* Font;
    float Unit = 1.0f;
    FVector2D Origin;
    FVector2D Pointer = FVector2D(-1, -1);
};

void DrawArenaMark(const FOrbitalCanvas& View, int32 Index, float X, float Y, const FLinearColor& Color)
{
    for (int32 Row = 0; Row < 3; ++Row)
    {
        const int32 Count = Index == 1 ? 3 - FMath::Abs(Row - 1) : 3;
        for (int32 Column = 0; Column < Count; ++Column)
        {
            const float U = X + Column * 16 + (3 - Count) * 8;
            const float V = Y + Row * 11 + (Index == 2 && Column == 1 ? 5 : 0);
            View.Rect(Color, U, V, Index == 0 ? 13 : 10, 6);
        }
    }
}
}

AArkanoidHUD::AArkanoidHUD()
{
    static ConstructorHelpers::FObjectFinder<UFont> Font(TEXT("/Engine/EngineFonts/Roboto.Roboto"));
    OrbitalFont = Font.Object;
}

void AArkanoidHUD::DrawHUD()
{
    Super::DrawHUD();

    const APaddle* Paddle = Cast<APaddle>(GetOwningPawn());
    const auto* Controller = Cast<AArkanoidArcadeController>(GetOwningPlayerController());
    if (!Canvas)
    {
        return;
    }
    if (Controller && Controller->IsOrbitalGame())
    {
        DrawOrbital(*Controller, Paddle);
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

void AArkanoidHUD::DrawOrbital(const AArkanoidArcadeController& Controller, const APaddle* Paddle)
{
    FOrbitalCanvas View(*this, *Canvas, Controller, OrbitalFont);
    const int32 Index = Controller.GetArenaIndex();
    if (Index == INDEX_NONE)
    {
        if (!bMenuArtLoaded)
        {
            OrbitalMenuArt = LoadObject<UTexture2D>(nullptr,
                TEXT("/Game/Arkanoid/Orbital/UI/T_OrbitalShipyard.T_OrbitalShipyard"));
            bMenuArtLoaded = true;
        }
        if (OrbitalMenuArt && OrbitalMenuArt->GetResource())
        {
            View.Cover(*OrbitalMenuArt);
        }

        const FLinearColor Accent = ArenaColors[0];
        const UArkanoidGameInstance* Instance = GetGameInstance<UArkanoidGameInstance>();
        View.Fill(FLinearColor(0.004f, 0.009f, 0.02f, 0.24f));
        View.Rect(OrbitalPanel, 48, 45, 198, 38);
        View.Rect(Accent, 48, 45, 3, 38);
        View.Text(FString::Printf(TEXT("BEST SCORE  %06d"), Instance ? Instance->GetBestScore() : 0),
            OrbitalWhite, 62, 54, 13, TEXT("Bold"));
        View.Text(TEXT("ARKANOID"), OrbitalWhite, 47, 147, 64, TEXT("Bold"));
        View.Text(TEXT("ORBITAL"), Accent, 47, 225, 86, TEXT("Bold"));

        View.Rect(OrbitalEdge, 52, 454, 1176, 1);
        View.Text(TEXT("SELECT ARENA"), OrbitalWhite, 52, 422, 16, TEXT("Bold"));
        for (int32 Arena = 0; Arena < UE_ARRAY_COUNT(ArenaNames); ++Arena)
        {
            const float X = 52 + 400 * Arena;
            const FLinearColor Tint = ArenaColors[Arena];
            const bool bHover = View.Hover(X, 482, 376, 138);
            const FLinearColor Card = bHover
                ? FLinearColor(0.032f, 0.061f, 0.095f, 0.98f) : FLinearColor(0.013f, 0.024f, 0.042f, 0.94f);
            View.Rect(bHover ? Tint : OrbitalEdge, X, 482, 376, 138);
            View.Rect(Card, X + 1, 483, 374, 136);
            View.Rect(Tint, X, 482, 48, 2);
            View.Text(FString::Printf(TEXT("0%d"), Arena + 1), Tint, X + 18, 498, 14, TEXT("Bold"));
            View.Text(ArenaNames[Arena], OrbitalWhite, X + 18, 527, 23, TEXT("Bold"));
            View.Text(ArenaDetails[Arena], OrbitalMuted, X + 18, 571, 15);
            DrawArenaMark(View, Arena, X + 300, 508, Tint);
            View.Line(Tint, {X + 318, 580}, {X + 342, 580}, 1.4f);
            View.Line(Tint, {X + 336, 574}, {X + 342, 580}, 1.4f);
            View.Line(Tint, {X + 342, 580}, {X + 336, 586}, 1.4f);
            View.HitBox(ArenaButtons[Arena], X, 482, 376, 138);
        }
        View.Text(TEXT("1 / 2 / 3   Select arena       Enter   Start Orbital Dock"), OrbitalMuted, 52, 660, 15);
        View.Button(ExitButton, TEXT("EXIT GAME"), Accent, false, 1080, 650, 148, 42);
        return;
    }

    const AArkanoidArcadeMode* Arcade = GetWorld()->GetAuthGameMode<AArkanoidArcadeMode>();
    if (!Arcade || !Paddle)
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

    DrawOrbitalFeedback(*Arcade);

    const FLinearColor Accent = ArenaColors[Index];
    const int32 Blocks = PlayingBoard.IsValid() ? PlayingBoard->GetRemainingBlocks() : 0;
    StartingBlockCount = FMath::Max(StartingBlockCount, Blocks);
    const float Progress = StartingBlockCount > 0 ? 1.0f - float(Blocks) / StartingBlockCount : 0.0f;
    const int32 Seconds = FMath::FloorToInt(Arcade->GetRunTime());
    View.Rect(OrbitalPanel, 24, 36, 202, 272);
    View.Rect(Accent, 24, 36, 46, 2);
    View.Text(FString::Printf(TEXT("ARENA  0%d"), Index + 1), Accent, 41, 48, 11, TEXT("Bold"));
    View.Text(ArenaNames[Index], OrbitalWhite, 41, 68, 15, TEXT("Bold"));
    View.Rect(OrbitalEdge, 41, 102, 168, 1);
    View.Text(TEXT("SCORE"), OrbitalMuted, 41, 118, 11, TEXT("Bold"));
    View.Text(FString::Printf(TEXT("%06d"), Arcade->GetScore()), OrbitalWhite, 40, 137, 29, TEXT("Bold"));
    View.Text(FString::Printf(TEXT("BEST   %06d"), Arcade->GetBestScore()), Accent, 41, 182, 12, TEXT("Bold"));
    View.Rect(OrbitalEdge, 41, 213, 168, 1);
    View.Text(TEXT("LIVES"), OrbitalMuted, 41, 227, 11, TEXT("Bold"));
    View.Text(FString::FromInt(Paddle->GetLives()), OrbitalWhite, 130, 223, 16, TEXT("Bold"));
    View.Text(TEXT("TIME"), OrbitalMuted, 41, 258, 11, TEXT("Bold"));
    View.Text(FString::Printf(TEXT("%02d:%02d"), Seconds / 60, Seconds % 60), OrbitalWhite, 130, 253, 16, TEXT("Bold"));

    View.Rect(OrbitalPanel, 1054, 36, 202, 109);
    View.Rect(Accent, 1054, 36, 46, 2);
    View.Text(TEXT("BLOCKS LEFT"), OrbitalMuted, 1071, 48, 11, TEXT("Bold"));
    View.Text(FString::Printf(TEXT("%d / %d"), Blocks, StartingBlockCount), OrbitalWhite, 1071, 68, 23, TEXT("Bold"));
    View.Rect(OrbitalEdge, 1071, 125, 168, 4);
    View.Rect(Accent, 1071, 125, 168 * Progress, 4);

    const bool bSticky = Paddle->HasStickyBall();
    const bool bShield = Paddle->HasShield();
    const float BonusFlash = Arcade->GetBonusFlash();
    if (BonusFlash > 0.0f || bSticky || bShield)
    {
        View.Rect(OrbitalPanel, 1054, 158, 202, 72);
        View.Rect(Accent, 1054, 158, 3, 72);
        View.Text(BonusFlash > 0.0f ? TEXT("POWER-UP ACQUIRED") : TEXT("POWER-UP READY"),
            Accent, 1071, 171, 10, TEXT("Bold"));
        const FString Active = BonusFlash > 0.0f ? Arcade->GetLastBonusName().ToString()
            : bSticky ? TEXT("Sticky Ball") : TEXT("Safety Shield");
        View.Text(Active, OrbitalWhite, 1071, 191, 15, TEXT("Bold"));
    }

    View.Rect(OrbitalPanel, 24, 632, 202, 64);
    View.Text(TEXT("A / D    Move"), OrbitalWhite, 41, 641, 12);
    View.Text(TEXT("Esc     Pause"), OrbitalMuted, 41, 667, 12);

    const bool bFinished = Arcade->HasFinished();
    if (bFinished || Controller.IsArcadePaused())
    {
        const bool bWon = Arcade->HasWon();
        const bool bNext = bWon && Index + 1 < UE_ARRAY_COUNT(ArenaNames);
        View.Fill(FLinearColor(0.003f, 0.006f, 0.012f, 0.68f));
        View.Rect(OrbitalEdge, 363, 208, 554, 316);
        View.Rect(OrbitalPanel, 364, 209, 552, 314);
        View.Rect(Accent, 363, 208, 140, 2);
        View.Text(ArenaNames[Index], Accent, 640, 236, 13, TEXT("Bold"), true);
        View.Text(bFinished ? (bWon ? TEXT("ARENA CLEARED") : TEXT("OUT OF LIVES")) : TEXT("PAUSED"),
            OrbitalWhite, 640, 264, 34, TEXT("Bold"), true);
        View.Text(FString::Printf(TEXT("SCORE  %d     BEST  %d"), Arcade->GetScore(), Arcade->GetBestScore()),
            OrbitalMuted, 640, 336, 17, TEXT("Bold"), true);
        View.Button(bNext ? NextButton : bFinished ? PlayButton : ResumeButton,
            bNext ? TEXT("NEXT ARENA") : bFinished ? TEXT("REPLAY") : TEXT("RESUME"),
            Accent, true, 396, 395, 226, 50);
        View.Button(MenuButton, TEXT("MAIN MENU"), Accent, false, 658, 395, 226, 50);
        View.Text(bFinished ? TEXT("R   Replay      Esc   Main menu") : TEXT("Esc   Resume      R   Restart"),
            OrbitalMuted, 640, 478, 13, TEXT("Regular"), true);
    }
    else if (const ABall* Ball = Paddle->GetCurrentBall(); Ball && !Ball->IsLaunched())
    {
        View.Rect(OrbitalPanel, 491, 674, 298, 32);
        View.Text(TEXT("SPACE / LEFT CLICK    LAUNCH"), Accent, 640, 680, 13, TEXT("Bold"), true);
    }
}

void AArkanoidHUD::DrawOrbitalFeedback(const AArkanoidArcadeMode& Arcade)
{
    const float Unit = FMath::Min(Canvas->ClipX / 1280.0f, Canvas->ClipY / 720.0f);
    const float Now = GetWorld()->GetTimeSeconds();
    for (const FOrbitalFeedbackEvent& Event : Arcade.GetOrbitalFeedback())
    {
        const bool bLine = Event.Kind == EOrbitalFeedbackKind::LineClear;
        const bool bHeavy = Event.Kind == EOrbitalFeedbackKind::BlockDestroyed
            || Event.Kind == EOrbitalFeedbackKind::BonusPickedUp
            || Event.Kind == EOrbitalFeedbackKind::StickyCatch
            || Event.Kind == EOrbitalFeedbackKind::ShieldSave;
        const float Age = FMath::Clamp((Now - Event.StartSeconds) / (bLine ? 0.50f : bHeavy ? 0.36f : 0.22f), 0.0f, 1.0f);
        if (Age >= 1.0f)
        {
            continue;
        }
        const FVector Start = Project(Event.Position);
        FLinearColor Color = Event.Color;
        Color.A = (1.0f - Age) * (1.0f - Age);
        if (bLine)
        {
            const FVector End = Project(Event.Position + Event.Direction * Event.Extent);
            FLinearColor Halo = Color;
            Halo.A *= 0.24f;
            DrawLine(Start.X, Start.Y, End.X, End.Y, Halo, (17.0f + Age * 12.0f) * Unit);
            DrawLine(Start.X, Start.Y, End.X, End.Y, Color, 3.0f * Unit);
            continue;
        }

        const FVector2D Center(Start.X, Start.Y);
        const FVector Ray = Project(Event.Position + Event.Direction * 50.0f);
        const FVector2D Travel = FVector2D(Ray.X - Start.X, Ray.Y - Start.Y).GetSafeNormal();
        const float Phase = FMath::Atan2(Travel.Y, Travel.X);
        const float Radius = (5.0f + Age * (bHeavy ? 34.0f : 17.0f)) * Unit;
        const int32 Detail = bHeavy ? 10 : 6;
        for (int32 Side = 0; Side < Detail; ++Side)
        {
            const float Angle = Phase + 2.0f * PI * Side / Detail;
            const FVector2D Offset(FMath::Cos(Angle), FMath::Sin(Angle));
            const FVector2D A = Center + (Offset + Travel * Age * 0.22f) * Radius;
            const FVector2D B = A + Offset * (bHeavy ? 6.0f : 4.0f) * Unit;
            DrawLine(A.X, A.Y, B.X, B.Y, Color, (bHeavy ? 2.2f : 1.7f) * Unit);
        }
    }
}

void AArkanoidHUD::NotifyHitBoxClick(FName BoxName)
{
    Super::NotifyHitBoxClick(BoxName);
    if (AArkanoidArcadeController* Player = Cast<AArkanoidArcadeController>(GetOwningPlayerController()))
    {
        for (int32 Index = 0; Index < UE_ARRAY_COUNT(ArenaButtons); ++Index)
        {
            if (BoxName == ArenaButtons[Index])
            {
                Player->SelectArena(Index);
                return;
            }
        }
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
        else if (BoxName == ResumeButton)
        {
            Player->TogglePause();
        }
        else if (BoxName == NextButton)
        {
            Player->NextArena();
        }
    }
}
