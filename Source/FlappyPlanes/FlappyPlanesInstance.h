// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FindSessionsCallbackProxy.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
//
#include "FlappyPlanesInstance.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionSearchCompleted, bool, bWasSuccessful, const TArray<FBlueprintSessionResult>&, FoundSessions);

UCLASS()
class FLAPPYPLANES_API UFlappyPlanesInstance : public UGameInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FOnSessionSearchCompleted OnSessionSearchCompleted;

protected:

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	int32 MaxPlayers = 1;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly)
	bool bIsOnline = false;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UPROPERTY(EditDefaultsOnly)
	bool bNeedLogin = false;
public:

	virtual void Init() override;
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetMaxPlayers(int32 InMaxPlayers) { MaxPlayers = InMaxPlayers; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetMaxPlayers() { return MaxPlayers; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetIsOnline() { return bIsOnline; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetIsOnline(bool InbIsOnline) { bIsOnline = InbIsOnline; }
	UFUNCTION(BlueprintCallable)
	void EOSLoginWithDeviceID();
	UFUNCTION(BlueprintNativeEvent)
	void OnEOSLoginCompleted(bool bWasSuccessful, FName Error);
	UFUNCTION(BlueprintCallable)
	void CreateSession();
	UFUNCTION(BlueprintNativeEvent)
	void OnCreateSessionCompleted(FName SessionName, bool bWasSuccessful);

	UFUNCTION(BlueprintCallable)
	void FindSessions();
	void OnFindSessionsCompleted(bool bWasSuccessful);

	UFUNCTION(BlueprintCallable)
	void JoinSession(FBlueprintSessionResult Session);
	void OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult);
	UFUNCTION(BlueprintNativeEvent)
	void OnJoinSessionCompleted_BP(bool bWasSuccessful, FName Message);
};
