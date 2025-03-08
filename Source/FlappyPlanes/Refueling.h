// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
//
#include "Refueling.generated.h"

UCLASS()
class FLAPPYPLANES_API ARefueling : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ARefueling();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	UPROPERTY(EditDefaultsOnly)
	bool bIsRefillable = true;
	UPROPERTY(EditDefaultsOnly)
	float RefillingTime = 5.f;
	UPROPERTY(EditDefaultsOnly)
	bool bIsFilled = true;
	FTimerHandle RefillingTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* Mesh = nullptr;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void Refill();
	UFUNCTION()
	void OnOverlap(AActor* OverlappedActor, AActor* OtherActor);
};
