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


	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AFlappyPlane> PlaneClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCameraComponent* Camera = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float PlaneMoveRadiusZ = 2000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float PlaneMoveRadiusY = 2000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float PlaneMoveRadius = 2000.f;

	FVector DefaultCameraLocation;
public:	

	UPROPERTY(BlueprintReadWrite)
	AFlappyPlane* Plane = nullptr;

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void SpeedUp();
	void CancelSpeedUp();
	void Fire();
	void CancelFire();
	UFUNCTION(Server, Reliable)
	void SpeedUp_Server(bool bIsSpeedUp);
	UFUNCTION(Server, Reliable)
	void Fire_Server(bool bIsSpeedUp);
};
