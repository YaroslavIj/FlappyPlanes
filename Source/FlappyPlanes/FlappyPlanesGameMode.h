// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FlappyPlanesGameMode.generated.h"

class AFlappyPlane;
class AGamePawn;
class AAIGamePawn;
/**
 * 
 */
UCLASS()
class FLAPPYPLANES_API AFlappyPlanesGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float StartGameTime = 5;
	FTimerHandle StartGameTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AFlappyPlane> PlaneClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGamePawn> PawnClass;	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AGamePawn> AIPawnClass;
	UPROPERTY(BlueprintReadWrite)
	TArray<APlayerController*> PlayerControllers;
	virtual void PostLogin(APlayerController* PC) override;
	UPROPERTY(BlueprintReadWrite)
	TArray<AAIGamePawn*> AIGamePawns;
public:

	bool bIsGameStarted = false;
	UFUNCTION(BlueprintNativeEvent)
	void StartGame_BP();
	void StartGame();
	UFUNCTION()
	void OnPlaneDied(AFlappyPlane* Plane);

	UFUNCTION(BlueprintNativeEvent)
	void EndGame_BP(APlayerController* Looser);
	AFlappyPlane* SpawnPlane(AGamePawn* PawnOwner);
};
