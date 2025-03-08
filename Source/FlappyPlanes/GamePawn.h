// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "FlappyPlane.h"
//
#include "GamePawn.generated.h"

UCLASS()
class FLAPPYPLANES_API AGamePawn : public APawn
{
	GENERATED_BODY()

public:

	AGamePawn();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadWrite)
	AFlappyPlane* Plane = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AFlappyPlane> PlaneClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCameraComponent* Camera = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float PlaneMoveRadius = 2000.f;
public:	

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SpeedUp();
	void CancelSpeedUp();
	void Fire();
	void CancelFire();
};
