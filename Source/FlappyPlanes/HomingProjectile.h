// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "HomingProjectile.generated.h"

/**
 * 
 */
UCLASS()
class FLAPPYPLANES_API AHomingProjectile : public AProjectile
{
	GENERATED_BODY()
	
public: 
	virtual void Tick(float DeltaTime) override;
protected:

	UPROPERTY(EditDefaultsOnly)
	float FindTargetAngle = 45.f;

	virtual void BeginPlay() override;
};
