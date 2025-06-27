// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Projectile.h"
#include "NiagaraComponent.h"
//
#include "FlappyPlane.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlaneDied, AFlappyPlane*, Plane);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHealthChanged, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFuelChanged, float, NewFuel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProjectilesAmountChanged, float, NewProjectilesAmount);

class AGamePawn;
class AWorldDynamicShadow;

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
	UStaticMeshComponent* Mesh = nullptr;
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//UStaticMeshComponent* OverlapMesh = nullptr;

	UPROPERTY(Replicated)
	bool bIsSpeedUp = false;
	bool bIsFiring = false;

	FTimerHandle FireTimer;
	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;
	UPROPERTY(BlueprintAssignable)
	FOnFuelChanged OnFuelChanged;
	UPROPERTY(BlueprintAssignable)
	FOnProjectilesAmountChanged OnProjectilesAmountChanged;

	//Movement
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MinForwardSpeed = 800;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MaxForwardSpeed = 2000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float SpeedUpForce = 6000000.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float CurrentForceWhileSpeedUp = 6000000.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MinSpeedUpForce = 6000000.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float AccelerationForce = 4000000.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float EnginePower = 100000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float StartImpulse = 100000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float DragCoefficient = 0.1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float AirDensity = 1.225f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float LiftCoefficient = 5.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float LiftDragCoef = 0.1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float SpeedUpOffsetAngle = 30.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float SpeedUpOffsetForce = 20000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float CollisionImpulceForSelf = 20000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float CollisionImpulceForOther = 50000;
	UPROPERTY(BlueprintReadWrite)
	FVector UpwardOffsetDirection;
	//Rotation
	UPROPERTY(BlueprintReadWrite)
	float RotationRate = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MinRotationRate = 20;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MaxRotationRate = 45;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float RotationAcceleration = 90.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float FlipSpeed = 90.f;	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MaxFallRotationSpeed = 180.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MinFallRotationSpeed = 10.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float FallRotationAcceleration = 180.f;
	UPROPERTY(BlueprintReadWrite)
	float CurrentFallRotationSpeed = 0;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float AngularDistanceToSlowFallRoation = 30.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float MovementForceWhileFlip = 1000000;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement")
	float PlaneXPosition = -920;
	UPROPERTY(BlueprintReadWrite)
	bool bNeedToFlip = false;
	UPROPERTY(BlueprintReadWrite)
	bool bReturnFlip = false;
	float DegreeceToReturnFlip;
	UPROPERTY(BlueprintReadWrite)
	float CurrentFlipRotation = 0.f;
	//Fuel
	UPROPERTY(ReplicatedUsing = OnRep_Fuel , EditDefaultsOnly, BlueprintReadWrite, Category = "Fuel")
	float Fuel = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fuel")
	float FuelConsumption = 0.05;
	//Fire
	UPROPERTY(Replicated, BlueprintReadWrite)
	FProjectilesSettings CurrentProjectilesType;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
	FVector FireLocation;
	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
	//int32 MaxProjectilesAmount = 30;
	UPROPERTY(ReplicatedUsing = OnRep_ProjectilesAmount, BlueprintReadWrite, Category = "Fire")
	int32 ProjectilesAmount;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
	float CollisionDamageForSelf = 0.008f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
	float CollisionDamageForOther = 0.016f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Fire")
	TArray<FProjectilesSettings> ProjectileTypes;

	//Sounds
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sounds")
	USoundBase* FlightSound = nullptr;
	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sounds")
	//USoundBase* SpeedUpSound = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sounds")
	float FlightSoundVolume;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sounds")
	float SpeedUpSoundVolume;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sounds")
	float FlightSoundVolumeCangeRate = 0.4;
	UPROPERTY(BlueprintReadWrite)
	UAudioComponent* CurrentFlightSound = nullptr;
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health = 100.f;
	bool bIsGameStarted = false;

	UFUNCTION(BlueprintCallable)
	void Dead();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FX")
	UNiagaraSystem* SpeedUpStartNiagara = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FX")
	UNiagaraSystem* SpeedUpNiagara = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "FX")
	FTransform SpeedUpFXSpawnTransform;
	UNiagaraComponent* SpeedUpNiagaraComponent = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TArray<FVector> LiftForcePoints;
	UPROPERTY(BlueprintReadWrite)
	AWorldDynamicShadow* DynamicShadow = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AWorldDynamicShadow> WorldDynamicShadowClass;

	FVector LastVelocity;
	FQuat LastRotation;

public:

	bool bIsFalling = false;
	bool bHasInfiniteAmmo = false;
	UPROPERTY(Replicated, BlueprintReadWrite)
	AGamePawn* PawnOwner = nullptr;

	FOnPlaneDied OnPlaneDied;

	UPROPERTY(BlueprintReadWrite)
	bool bIsMovingForward = true;

	virtual void Tick(float DeltaTime) override;

	UFUNCTION(Server, Reliable)
	void SetIsSpeedUp_Server(bool InbIsSpeedUp);
	void FillFuel();
	UFUNCTION(Server, Reliable)
	void SetIsFiring_Server(bool InbIsFiring);
	void MovementTick(float DeltaTime);
	UFUNCTION()
	void Fire();
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCurrentFuel() { return Fuel; };
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCurrentProjectilesAmount() { return ProjectilesAmount; };
	/*UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetMaxProjectilesAmount() { return MaxProjectilesAmount; };*/
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetProjectilesAmount(float NewProjectilesAmount) { ProjectilesAmount = NewProjectilesAmount; };
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCurrentHealth() { return Health; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetHealth(float NewHealth) { Health = NewHealth; }
	UFUNCTION(BlueprintCallable)
	void ReceiveDamage(float Damage);

	UFUNCTION(NetMulticast, Unreliable)
	void ChangeFlightSoundVolumeTick_Multicast(float DeltaTime);
	void StartGame();

	UFUNCTION(BlueprintNativeEvent)
	void OnRep_Health();
	UFUNCTION(BlueprintNativeEvent)
	void OnRep_Fuel();
	UFUNCTION(BlueprintNativeEvent)
	void OnRep_ProjectilesAmount();

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(NetMulticast, Reliable)
	void SpawnNiagaraAtLocation_Multicast(UNiagaraSystem* NiagaraFX, FTransform SpawnTransform);
	UFUNCTION(NetMulticast, Reliable)
	void SpawnSpeedUpNiagaraAttached_Multicast(UNiagaraSystem* NiagaraFX, FTransform SpawnTransform);
	UFUNCTION(NetMulticast, Reliable)
	void StopSpeedUpNiagara_Multicast();
	UFUNCTION(NetMulticast, Reliable)
	void CreateDynamicShadow_Multicast();
	UFUNCTION(Server, Reliable)
	void NextProjectilesType_Server();
	UFUNCTION(NetMulticast, Reliable)
	void SetMaxProjectilesByTypeOnWidget_Multicast(float MaxProjectiles, TSubclassOf<AProjectile> Type);
	UFUNCTION(BlueprintCallable)
	void FillProjectilesAmount();
	UFUNCTION()
	void OnHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);
};
