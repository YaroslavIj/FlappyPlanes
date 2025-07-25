// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/ProjectileMovementComponent.h"
//
#include "Projectile.generated.h"

class AProjectile;
USTRUCT(BlueprintType)
struct FProjectilesSettings
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FireRate = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ProjectilesAmount = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaxProjectilesAmount = 50;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* FireSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* FireFX = nullptr;
};

UCLASS()
class FLAPPYPLANES_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* Mesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UProjectileMovementComponent* ProjectileMovement = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* HitSound = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* HitFX = nullptr;
	UFUNCTION()
	void OnProjectileOverlap(AActor* OverlappedActor, AActor* OtherActor);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(NetMulticast, Reliable)
	void Hit_Multicast(USoundBase* Sound, UParticleSystem* FX);

};
