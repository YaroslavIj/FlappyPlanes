// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FlappyPlanesInstance.generated.h"

/**
 * 
 */
UCLASS()
class FLAPPYPLANES_API UFlappyPlanesInstance : public UGameInstance
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int32 MaxPlayers = 1;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool bIsOnline = false;

public:

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetMaxPlayers(int32 InMaxPlayers) { MaxPlayers = InMaxPlayers; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetMaxPlayers() { return MaxPlayers; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetIsOnline() { return bIsOnline; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetIsOnline(bool InbIsOnline) { bIsOnline = InbIsOnline; }
};
