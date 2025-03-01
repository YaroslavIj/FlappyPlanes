// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UStaticMeshComponent* PlaneMesh = nullptr;

	bool bIsSpeedUp = false;


	//Movement
	UPROPERTY(BlueprintReadWrite)
	float ForwardSpeed = 800;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MinForwardSpeed = 800;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MaxForwardSpeed = 2000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float AccelerationSpeed = 500;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float FallingAcceleration = 500;
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
	bool bIsAligningFromUpsideDown = false;
	bool bNeedToFlip = false;
	FQuat InitialRotationForFlip;
	bool bIsFalling = false;
	UPROPERTY(BlueprintReadWrite)
	float CurrentFlipRotation = 0.f;

public:

	bool bIsMovingForward = true;

	virtual void Tick(float DeltaTime) override;

	void SetIsSpeedUp(bool InbIsSpeedUp);
};
