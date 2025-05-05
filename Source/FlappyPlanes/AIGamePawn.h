// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GamePawn.h"
#include "AIGamePawn.generated.h"

/**
 * 
 */
UCLASS()
class FLAPPYPLANES_API AAIGamePawn : public AGamePawn
{
	GENERATED_BODY()

public:

	AFlappyPlane* TargetEnemyPlane = nullptr;

	UPROPERTY(BlueprintReadWrite)
	float FinalAngle;

	UPROPERTY(BlueprintReadWrite)
	float SignedAngle;
protected:

	virtual void BeginPlay() override;

public:

	virtual void Tick(float DeltaTime) override;

	void StartGame();

	bool bNeedToCoup = false;
};
