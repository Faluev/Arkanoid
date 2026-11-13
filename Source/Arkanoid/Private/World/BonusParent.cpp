#include "World/BonusParent.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Game/ArkanoidGameMode.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "World/Paddle.h"

ABonusParent::ABonusParent()
{
    PrimaryActorTick.bCanEverTick = true;

    constexpr float Radius = 22.0f;
    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    SetRootComponent(Collision);
    Collision->InitSphereRadius(Radius);
    Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Collision->SetCollisionObjectType(ECC_WorldDynamic);
    Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
    Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    Collision->SetGenerateOverlapEvents(true);

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(Collision);
    Mesh->SetRelativeScale3D(FVector(Radius / 50.0f));
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    if (Sphere.Succeeded())
    {
        Mesh->SetStaticMesh(Sphere.Object);
    }
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Tint(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    if (Tint.Succeeded())
    {
        Mesh->SetMaterial(0, Tint.Object);
    }

    Label = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Label"));
    Label->SetupAttachment(Collision);
    Label->SetRelativeLocation(FVector(0.0, 0.0, Radius + 2.0f));
    Label->SetRelativeRotation(FRotator(90.0, 180.0, 0.0));
    Label->SetHorizontalAlignment(EHTA_Center);
    Label->SetVerticalAlignment(EVRTA_TextCenter);
    Label->SetWorldSize(16.0f);
    Label->SetTextRenderColor(FColor(16, 22, 36));
}

void ABonusParent::BeginPlay()
{
    Super::BeginPlay();

    if (UMaterialInstanceDynamic* Material = Mesh->CreateDynamicMaterialInstance(0))
    {
        Material->SetVectorParameterValue(TEXT("Color"), Color * 0.20f);
    }
    Label->SetText(FText::FromString(Symbol));
    SetLifeSpan(5.5f);
    UE_LOG(LogTemp, Log, TEXT("Arkanoid Bonus %s dropped"), *GetClass()->GetName());
}

void ABonusParent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // The paddle is at the near end of the board, along negative world X.
    AddActorWorldOffset(FVector(-FallSpeed * DeltaTime, 0.0, 0.0), true);
}

void ABonusParent::SetSourceCell(int32 Row, int32 Column)
{
    SourceRow = Row;
    SourceColumn = Column;
}

void ABonusParent::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    APaddle* Paddle = Cast<APaddle>(OtherActor);
    if (Paddle && !bCollected)
    {
        bCollected = true;
        ApplyTo(*Paddle);
        if (const AArkanoidArcadeMode* Mode = GetWorld()->GetAuthGameMode<AArkanoidArcadeMode>())
        {
            Mode->OnBonusCollected(GetActorLocation());
        }
        UE_LOG(LogTemp, Log, TEXT("Arkanoid Bonus %s collected"), *GetClass()->GetName());
        Destroy();
    }
}
