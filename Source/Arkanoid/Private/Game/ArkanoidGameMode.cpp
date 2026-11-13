#include "Game/ArkanoidGameMode.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Game/ArkanoidArcadeController.h"
#include "Game/ArkanoidGameInstance.h"
#include "Game/ArkanoidHUD.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"

namespace
{
constexpr int32 PointsPerBlock = 100;

bool UsesOrbitalPresentation(const UWorld* World)
{
    const auto* Player = Cast<AArkanoidArcadeController>(World->GetFirstPlayerController());
    return Player && Player->IsOrbitalGame();
}

template <typename AssetType>
AssetType* LoadCourseAsset(const TCHAR* Path)
{
    const FSoftObjectPath AssetPath(Path);
    if (FPackageName::DoesPackageExist(AssetPath.GetLongPackageName()))
    {
        return Cast<AssetType>(AssetPath.TryLoad());
    }
    return nullptr;
}
}

AArkanoidGameMode::AArkanoidGameMode()
{
    // The arena contains a player-owned paddle at its authored starting position.
    DefaultPawnClass = nullptr;
    HUDClass = AArkanoidHUD::StaticClass();
}

void AArkanoidGameMode::StartPlay()
{
    Super::StartPlay();
    if (!bArcadePresentation)
    {
        return;
    }

    const bool bOrbital = UsesOrbitalPresentation(GetWorld());
    MenuClick = LoadCourseAsset<USoundBase>(bOrbital
        ? TEXT("/Game/Arkanoid/Orbital/Audio/S_OrbitalClick.S_OrbitalClick")
        : TEXT("/Game/Art/Sounds/Cues/Futuristic_Click_01_Cue.Futuristic_Click_01_Cue"));
    if (bOrbital)
    {
        return;
    }
    UMaterialInterface* FloorMaterial = LoadCourseAsset<UMaterialInterface>(
        TEXT("/Game/Art/BasicMeshes/Materials/MI_Floor.MI_Floor"));
    UStaticMesh* FabRock = LoadCourseAsset<UStaticMesh>(
        TEXT("/Game/Rock_Collection_04/Meshes/Rock_01/StaticMeshes/SM_Rock_01.SM_Rock_01"));
    for (TActorIterator<AStaticMeshActor> MeshActor(GetWorld()); MeshActor; ++MeshActor)
    {
        UStaticMeshComponent* Mesh = MeshActor->GetStaticMeshComponent();
        if (FloorMaterial && MeshActor->ActorHasTag(TEXT("ArcadePackFloor")))
        {
            Mesh->SetMaterial(0, FloorMaterial);
        }
        if (FabRock && MeshActor->ActorHasTag(TEXT("ArcadeFabRock")) && Mesh->GetStaticMesh())
        {
            const FBox OldBounds = Mesh->GetStaticMesh()->GetBoundingBox();
            const FBox NewBounds = FabRock->GetBoundingBox();
            const FVector OldCenter = Mesh->GetComponentTransform().TransformPosition(OldBounds.GetCenter());
            const FVector FootprintScale = Mesh->GetRelativeScale3D() * OldBounds.GetSize() / NewBounds.GetSize();
            Mesh->SetMobility(EComponentMobility::Movable);
            if (Mesh->SetStaticMesh(FabRock))
            {
                Mesh->EmptyOverrideMaterials();
                Mesh->SetRelativeScale3D(FootprintScale);
                Mesh->AddWorldOffset(OldCenter - Mesh->GetComponentTransform().TransformPosition(NewBounds.GetCenter()));
                Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
    }
}

void AArkanoidGameMode::PlayMenuClick() const
{
    if (MenuClick)
    {
        UGameplayStatics::PlaySound2D(this, MenuClick);
    }
}

AArkanoidArcadeMode::AArkanoidArcadeMode()
{
    bArcadePresentation = true;
    PlayerControllerClass = AArkanoidArcadeController::StaticClass();
}

void AArkanoidArcadeMode::StartPlay()
{
    Super::StartPlay();
    StartSeconds = GetWorld()->GetTimeSeconds();
    bOrbitalPresentation = UsesOrbitalPresentation(GetWorld());
    BlockSound = LoadCourseAsset<USoundBase>(bOrbitalPresentation
        ? TEXT("/Game/Arkanoid/Orbital/Audio/S_OrbitalBlock.S_OrbitalBlock")
        : TEXT("/Game/Art/Sounds/Cues/Abstract_Pop_01_Cue.Abstract_Pop_01_Cue"));
    PickupSound = LoadCourseAsset<USoundBase>(bOrbitalPresentation
        ? TEXT("/Game/Arkanoid/Orbital/Audio/S_OrbitalPickup.S_OrbitalPickup")
        : TEXT("/Game/Art/Sounds/Cues/Coins_01_Cue.Coins_01_Cue"));
    if (!bOrbitalPresentation)
    {
        BlockEffect = LoadCourseAsset<UParticleSystem>(TEXT("/Game/Art/Starter/VFX/P_Explosion.P_Explosion"));
    }
}

int32 AArkanoidArcadeMode::GetBestScore() const
{
    const UArkanoidGameInstance* Instance = GetGameInstance<UArkanoidGameInstance>();
    return Instance ? Instance->GetBestScore() : 0;
}

float AArkanoidArcadeMode::GetRunTime() const
{
    return bFinished ? FinalRunTime : GetWorld()->GetTimeSeconds() - StartSeconds;
}

float AArkanoidArcadeMode::GetBonusFlash() const
{
    return FMath::Clamp(1.0f - (GetWorld()->GetTimeSeconds() - LastBonusTime) / 1.25f, 0.0f, 1.0f);
}

void AArkanoidArcadeMode::OnBlockDestroyed(const FVector& Location, bool bBoardCleared, const FLinearColor& Color)
{
    if (bFinished)
    {
        return;
    }

    Score += PointsPerBlock;
    if (UArkanoidGameInstance* Instance = GetGameInstance<UArkanoidGameInstance>())
    {
        Instance->RecordScore(Score);
    }
    UE_LOG(LogTemp, Log, TEXT("Arkanoid score %d, best %d"), Score, GetBestScore());

    AddOrbitalFeedback(EOrbitalFeedbackKind::BlockDestroyed, Location, FVector::ZeroVector, Color);
    if (BlockSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BlockSound, Location);
    }
    if (BlockEffect)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(), BlockEffect, Location, FRotator::ZeroRotator, FVector(0.32));
    }
    if (bBoardCleared)
    {
        FinishRound(true);
    }
}

void AArkanoidArcadeMode::OnBonusCollected(const FVector& Location, const FText& Name, const FLinearColor& Color)
{
    LastBonusTime = GetWorld()->GetTimeSeconds();
    LastBonusName = Name;
    AddOrbitalFeedback(EOrbitalFeedbackKind::BonusPickedUp, Location, FVector::ZeroVector, Color);
    if (PickupSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, Location);
    }
}

void AArkanoidArcadeMode::AddOrbitalFeedback(EOrbitalFeedbackKind Kind, const FVector& Position,
    const FVector& Direction, const FLinearColor& Color, float Extent)
{
    if (!bOrbitalPresentation || bFinished)
    {
        return;
    }

    // Expired entries cannot hold up a frame even during a long round.
    constexpr float FeedbackLifetime = 1.25f;
    constexpr int32 MaxFeedbackEvents = 32;
    const float Now = GetWorld()->GetTimeSeconds();
    OrbitalFeedback.RemoveAll([Now](const FOrbitalFeedbackEvent& Event)
    {
        return Now - Event.StartSeconds >= FeedbackLifetime;
    });
    if (OrbitalFeedback.Num() == MaxFeedbackEvents)
    {
        OrbitalFeedback.RemoveAt(0, 1, EAllowShrinking::No);
    }
    OrbitalFeedback.Add({Position, Direction, Color, Now, Kind, Extent});

    switch (Kind)
    {
    case EOrbitalFeedbackKind::Launch:
        if (BlockSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, BlockSound, Position, 0.64f, 1.42f);
        }
        break;
    case EOrbitalFeedbackKind::PaddleBounce:
    case EOrbitalFeedbackKind::BlockHit:
        if (BlockSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, BlockSound, Position, 0.36f,
                Kind == EOrbitalFeedbackKind::PaddleBounce ? 1.20f : 0.82f);
        }
        break;
    case EOrbitalFeedbackKind::StickyCatch:
    case EOrbitalFeedbackKind::ShieldSave:
        if (PickupSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, PickupSound, Position, 0.48f,
                Kind == EOrbitalFeedbackKind::StickyCatch ? 0.87f : 1.14f);
        }
        break;
    default:
        break;
    }
}

void AArkanoidArcadeMode::OnLivesDepleted()
{
    FinishRound(false);
}

void AArkanoidArcadeMode::FinishRound(bool bVictory)
{
    if (bFinished)
    {
        return;
    }
    FinalRunTime = GetRunTime();
    bFinished = true;
    bWon = bVictory;
    UE_LOG(LogTemp, Log, TEXT("Arkanoid round %s with score %d"), bWon ? TEXT("won") : TEXT("lost"), Score);
    if (AArkanoidArcadeController* Player = Cast<AArkanoidArcadeController>(GetWorld()->GetFirstPlayerController()))
    {
        Player->ShowMenuInput();
        Player->SetPause(true);
    }
}

AArkanoidMenuMode::AArkanoidMenuMode()
{
    bArcadePresentation = true;
    PlayerControllerClass = AArkanoidArcadeController::StaticClass();
}

void AArkanoidMenuMode::StartPlay()
{
    Super::StartPlay();
    const bool bOrbital = UsesOrbitalPresentation(GetWorld());
    if (USoundBase* Music = LoadCourseAsset<USoundBase>(bOrbital
        ? TEXT("/Game/Arkanoid/Orbital/Audio/S_OrbitalMenu.S_OrbitalMenu")
        : TEXT("/Game/Art/Music/itty-bitty-8-bit_Cue.itty-bitty-8-bit_Cue")))
    {
        UGameplayStatics::SpawnSound2D(this, Music, bOrbital ? 0.60f : 0.24f);
    }
}
