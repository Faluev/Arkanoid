#include "Game/ArkanoidGameInstance.h"

#include "Game/ArkanoidSaveGame.h"
#include "Kismet/GameplayStatics.h"

namespace
{
const FString ScoreSlot(TEXT("ArkanoidHighScore"));
constexpr int32 SaveUser = 0;
}

void UArkanoidGameInstance::Init()
{
    Super::Init();

    TArray<uint8> SavedBytes;
    if (!UGameplayStatics::LoadDataFromSlot(SavedBytes, ScoreSlot, SaveUser))
    {
        return;
    }

    // Reject a truncated file or unrelated data before loading a UE SaveGame object.
    constexpr int32 MinSaveBytes = 32;
    constexpr int32 MaxSaveBytes = 64 * 1024;
    if (SavedBytes.Num() < MinSaveBytes || SavedBytes.Num() > MaxSaveBytes
        || SavedBytes[0] != 'G' || SavedBytes[1] != 'V' || SavedBytes[2] != 'A' || SavedBytes[3] != 'S')
    {
        UE_LOG(LogTemp, Warning, TEXT("Arkanoid ignored an invalid score save"));
        return;
    }

    if (const UArkanoidSaveGame* Save = Cast<UArkanoidSaveGame>(UGameplayStatics::LoadGameFromMemory(SavedBytes)))
    {
        BestScore = FMath::Max(0, Save->BestScore);
    }
}

bool UArkanoidGameInstance::RecordScore(int32 Score)
{
    if (Score <= BestScore)
    {
        return false;
    }

    UArkanoidSaveGame* Save = Cast<UArkanoidSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UArkanoidSaveGame::StaticClass()));
    if (!Save)
    {
        return false;
    }

    Save->BestScore = Score;
    if (!UGameplayStatics::SaveGameToSlot(Save, ScoreSlot, SaveUser))
    {
        UE_LOG(LogTemp, Warning, TEXT("Arkanoid could not save the new record %d"), Score);
        return false;
    }

    BestScore = Score;
    UE_LOG(LogTemp, Log, TEXT("Arkanoid best score saved: %d"), BestScore);
    return true;
}
