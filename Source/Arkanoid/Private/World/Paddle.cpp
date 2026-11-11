#include "World/Paddle.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Game/ArkanoidGameMode.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "World/Ball.h"

namespace
{
constexpr float PaddleHalfDepth = 18.0f;
constexpr float PaddleHalfWidth = 75.0f;
constexpr float PaddleHalfHeight = 15.0f;
constexpr float BallHoldOffset = 48.0f;
}

APaddle::APaddle()
{
    PrimaryActorTick.bCanEverTick = true;

    Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->InitBoxExtent(FVector(PaddleHalfDepth, PaddleHalfWidth, PaddleHalfHeight));
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionObjectType(ECC_Pawn);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
    Collision->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
    Collision->SetGenerateOverlapEvents(true);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Collision);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->SetRelativeScale3D(FVector(PaddleHalfDepth, PaddleHalfWidth, PaddleHalfHeight) / 50.0);

    ShieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShieldMesh"));
    ShieldMesh->SetupAttachment(Collision);
    ShieldMesh->SetAbsolute(true, true, true);
    ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ShieldMesh->SetCastShadow(false);
    ShieldMesh->SetVisibility(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded())
    {
        Mesh->SetStaticMesh(Cube.Object);
        ShieldMesh->SetStaticMesh(Cube.Object);
    }

    BallClass = ABall::StaticClass();
}

void APaddle::BeginPlay()
{
    Super::BeginPlay();
    Lives = StartingLives;

    // The shield spans the bottom lane independently of the paddle's movement.
    ShieldMesh->SetWorldLocation(FVector(GetActorLocation().X - 104.0, 0.0, GetActorLocation().Z));
    ShieldMesh->SetWorldScale3D(FVector(0.08, LaneHalfWidth / 50.0, 0.12));
    PrepareBall();
}

void APaddle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (InputSubsystem.IsValid() && InputMapping)
    {
        InputSubsystem->RemoveMappingContext(InputMapping);
    }
    GetWorldTimerManager().ClearTimer(PrepareBallTimer);
    GetWorldTimerManager().ClearTimer(SizeBonusTimer);
    if (CurrentBall)
    {
        CurrentBall->OnDestroyed.RemoveDynamic(this, &APaddle::HandleBallDestroyed);
    }
    Super::EndPlay(EndPlayReason);
}

void APaddle::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    ULocalPlayer* LocalPlayer = PlayerController ? PlayerController->GetLocalPlayer() : nullptr;
    if (!EnhancedInput || !LocalPlayer)
    {
        return;
    }

    if (InputSubsystem.IsValid() && InputMapping)
    {
        InputSubsystem->RemoveMappingContext(InputMapping);
    }

    InputMapping = NewObject<UInputMappingContext>(this);
    UInputAction* MoveAction = NewObject<UInputAction>(InputMapping);
    MoveAction->ValueType = EInputActionValueType::Axis1D;
    MoveAction->AccumulationBehavior = EInputActionAccumulationBehavior::Cumulative;
    InputMapping->MapKey(MoveAction, EKeys::D);
    InputMapping->MapKey(MoveAction, EKeys::Right);
    InputMapping->MapKey(MoveAction, EKeys::A).Modifiers.Add(NewObject<UInputModifierNegate>(InputMapping));
    InputMapping->MapKey(MoveAction, EKeys::Left).Modifiers.Add(NewObject<UInputModifierNegate>(InputMapping));
    EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APaddle::Move);
    EnhancedInput->BindAction(MoveAction, ETriggerEvent::Completed, this, &APaddle::StopMoving);
    EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this, &APaddle::StopMoving);

    UInputAction* LaunchAction = NewObject<UInputAction>(InputMapping);
    InputMapping->MapKey(LaunchAction, EKeys::SpaceBar);
    EnhancedInput->BindAction(LaunchAction, ETriggerEvent::Started, this, &APaddle::LaunchBall);

    if (!GetWorld()->GetAuthGameMode<AArkanoidArcadeMode>())
    {
        UInputAction* RestartAction = NewObject<UInputAction>(InputMapping);
        InputMapping->MapKey(RestartAction, EKeys::R);
        EnhancedInput->BindAction(RestartAction, ETriggerEvent::Started, this, &APaddle::RestartLevel);
    }

    InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    if (InputSubsystem.IsValid())
    {
        InputSubsystem->AddMappingContext(InputMapping, 0);
    }
}

void APaddle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!FMath::IsNearlyZero(MovementInput) && Lives > 0)
    {
        FVector Location = GetActorLocation();
        const double Limit = LaneHalfWidth - GetHalfWidth();
        Location.Y = FMath::Clamp(Location.Y + MovementInput * MoveSpeed * DeltaTime, -Limit, Limit);
        SetActorLocation(Location, true);
    }
}

void APaddle::Move(const FInputActionValue& Value)
{
    MovementInput = FMath::Clamp(Value.Get<float>(), -1.0f, 1.0f);
}

void APaddle::StopMoving()
{
    MovementInput = 0.0f;
}

void APaddle::RestartLevel()
{
    UGameplayStatics::OpenLevel(this, FName(*UGameplayStatics::GetCurrentLevelName(this)));
}

void APaddle::PrepareBall()
{
    if (Lives <= 0 || CurrentBall || !BallClass)
    {
        return;
    }

    FActorSpawnParameters Parameters;
    Parameters.Owner = this;
    Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    const FVector Position = GetActorLocation() + FVector(BallHoldOffset, 0.0, 0.0);
    CurrentBall = GetWorld()->SpawnActor<ABall>(BallClass, Position, FRotator::ZeroRotator, Parameters);
    if (CurrentBall)
    {
        CurrentBall->HoldForLaunch();
        CurrentBall->AttachToActor(this, FAttachmentTransformRules::KeepWorldTransform);
        CurrentBall->OnDestroyed.AddDynamic(this, &APaddle::HandleBallDestroyed);
    }
}

void APaddle::LaunchBall()
{
    if (CurrentBall && !CurrentBall->IsLaunched())
    {
        CurrentBall->Launch(FVector::ForwardVector);
        UE_LOG(LogTemp, Log, TEXT("Arkanoid Ball %s launched from paddle"), *CurrentBall->GetName());
    }
}

void APaddle::HandleBallDestroyed(AActor* DestroyedActor)
{
    if (DestroyedActor == CurrentBall)
    {
        CurrentBall = nullptr;
        Lives = FMath::Max(0, Lives - 1);
        if (Lives > 0)
        {
            PrepareBallTimer = GetWorldTimerManager().SetTimerForNextTick(this, &APaddle::PrepareBall);
        }
        else if (AArkanoidArcadeMode* Mode = GetWorld()->GetAuthGameMode<AArkanoidArcadeMode>())
        {
            Mode->OnLivesDepleted();
        }
    }
}

void APaddle::AddLife(int32 Amount)
{
    if (Amount > 0)
    {
        Lives += Amount;
        PrepareBall();
    }
}

float APaddle::GetHalfWidth() const
{
    return PaddleHalfWidth * SizeMultiplier;
}

void APaddle::SetPaddleSize(float Multiplier)
{
    SizeMultiplier = Multiplier;
    const double Limit = LaneHalfWidth - GetHalfWidth();
    FVector Location = GetActorLocation();
    Location.Y = FMath::Clamp(Location.Y, -Limit, Limit);
    SetActorLocation(Location, true);
    const FVector HalfExtent(PaddleHalfDepth, GetHalfWidth(), PaddleHalfHeight);
    Collision->SetBoxExtent(HalfExtent);
    Mesh->SetRelativeScale3D(HalfExtent / 50.0);
}

void APaddle::ApplySizeBonus(float Multiplier, float Duration)
{
    if (Multiplier > 0.0f && Duration > 0.0f)
    {
        SetPaddleSize(FMath::Clamp(Multiplier, 0.25f, (LaneHalfWidth - 5.0f) / PaddleHalfWidth));
        GetWorldTimerManager().SetTimer(SizeBonusTimer, this, &APaddle::ResetSizeBonus, Duration, false);
    }
}

void APaddle::ResetSizeBonus()
{
    SetPaddleSize(1.0f);
}

void APaddle::ActivateShield()
{
    bShieldActive = true;
    ShieldMesh->SetVisibility(true);
}

bool APaddle::TrySaveBall(ABall* Ball)
{
    if (!bShieldActive || !Ball || Ball != CurrentBall || !Ball->IsLaunched())
    {
        return false;
    }

    bShieldActive = false;
    ShieldMesh->SetVisibility(false);
    Ball->BounceFromShield();
    return true;
}
