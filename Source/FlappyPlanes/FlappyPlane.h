// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Projectile.h"
//
#include "FlappyPlane.generated.h"

UCLASS()
class FLAPPYPLANES_API AFlappyPlane : public AActor 
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFlappyPlane();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* PlaneMesh = nullptr;

	bool bIsSpeedUp = false;
	bool bIsFiring = false;

	FTimerHandle FireTimer;

	//Movement
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MinForwardSpeed = 800;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MaxForwardSpeed = 2000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float AccelerationForce = 200000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float EnginePower = 100000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float StartImpulse = 100000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float DragCoefficient = 0.1;
	bool bIsFalling = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float AirDensity = 1.225f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float LiftCoefficient = 5.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float SpeedUpOffsetAngle = 30.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float SpeedUpOffsetForce = 20000;
	UPROPERTY(BlueprintReadWrite)
	FVector UpwardOffsetDirection;
	//Rotation
	UPROPERTY(BlueprintReadWrite)
	float RotationRate = 20;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MinRotationRate = 20;	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MaxRotationRate = 45;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float AlignSpeed = 90.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float FlipSpeed = 90.f;
	bool bNeedToFlip = false;
	UPROPERTY(BlueprintReadWrite)
	float CurrentFlipRotation = 0.f;
	//Fuel
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fuel")
	float Fuel = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fuel")
	float FuelConsumption = 0.05;
	//Fire
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
	float FireRate = 0.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
	TSubclassOf<AProjectile> ProjectileClass;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
	FVector FireLocation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
	int32 MaxProjectilesAmount = 30;
	UPROPERTY(BlueprintReadWrite, Category = "Fire")
	int32 ProjectilesAmount;
public:
	UPROPERTY(BlueprintReadWrite)
	bool bIsMovingForward = true;

	virtual void Tick(float DeltaTime) override;

	void SetIsSpeedUp(bool InbIsSpeedUp);
	void FillFuel();
	void SetIsFiring(bool InbIsFiring);
	void MovementTick(float DeltaTime);
	UFUNCTION()
	void Fire();
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCurrentFuel() { return Fuel; };
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCurrentProjectilesAmount() { return ProjectilesAmount; };
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float SetProjectilesAmount() { return ProjectilesAmount; };
};
