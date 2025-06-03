// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
//
#include "DynamicShadow.generated.h"

class AFlappyPlane;

UCLASS()
class FLAPPYPLANES_API ADynamicShadow : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADynamicShadow();
	UPROPERTY(BlueprintReadWrite)
	AFlappyPlane* Plane = nullptr;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UProceduralMeshComponent* ProceduralMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 VerticesCount = 720;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Radius = 200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TEnumAsByte<ECollisionChannel> TraceChannel = ECollisionChannel::ECC_GameTraceChannel1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	UMaterialInterface* Material = nullptr;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
