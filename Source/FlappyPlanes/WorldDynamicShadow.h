// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
//
#include "WorldDynamicShadow.generated.h"

class AFlappyPlane;

UCLASS()
class FLAPPYPLANES_API AWorldDynamicShadow : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWorldDynamicShadow();

	AFlappyPlane* Plane = nullptr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FTimerHandle UpdateTimer;
	UPROPERTY(EditDefaultsOnly)
	float UpdateRate = 0.1f;
	UPROPERTY(EditDefaultsOnly)
	float MeshResolution = 1.f;
	UPROPERTY(EditDefaultsOnly)
	float MeshResolutionStepUp = 5.f;
	UPROPERTY(EditDefaultsOnly)
	float MeshZDistance = 50000.f;
	UPROPERTY(EditDefaultsOnly)
	float TracesAngle = -90.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UProceduralMeshComponent* ProclMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECollisionChannel::ECC_GameTraceChannel1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UMaterialInterface* MainMaterial = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UMaterialInterface* ShadowMaterial = nullptr;
	UPROPERTY(BlueprintReadWrite)
	FVector TraceDirection;

	TArray<int32> ShadowVerticesInSectionsCount;
	int32 MainVerticesCount;

	UFUNCTION()
	void Update();


	UPROPERTY(BlueprintReadWrite)
	int32 CountOfUpdates = 0;
	UPROPERTY(BlueprintReadWrite)
	int32 CountOfDestroys = 0;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
