// Fill out your copyright notice in the Description page of Project Settings.


#include "ABGameMode.h"
#include "ABCharacter.h"
#include "ABPlayerState.h"
#include "ABPlayerController.h"
#include "ABGameState.h"

AABGameMode::AABGameMode()
{
    DefaultPawnClass = AABCharacter::StaticClass();
    PlayerControllerClass = AABPlayerController::StaticClass();
    PlayerStateClass = AABPlayerState::StaticClass();
    GameStateClass = AABGameState::StaticClass();
}

void AABGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
    auto ABPlayerState = Cast<AABPlayerState>(NewPlayer->PlayerState);
	ABPlayerState->InitPlayerData();
}

void AABGameMode::PostInitializeComponents()
{
	Super::PostInitializeComponents();
    ABGameState = Cast<AABGameState>(GameState);
}

void AABGameMode::AddScore(class AABPlayerController* ScoredPlayer)
{
	for(FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator();It;++It)
    {
        const auto ABPlayerController = Cast<AABPlayerController>(It->Get());
        if((nullptr != ABPlayerController)&&(ScoredPlayer == ABPlayerController))
        {
            ABPlayerController->AddGameScore();
            break;
        }
    }
    ABGameState->AddGameScore();
}