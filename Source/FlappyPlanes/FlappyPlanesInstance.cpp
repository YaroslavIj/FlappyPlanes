// Fill out your copyright notice in the Description page of Project Settings.


#include "FlappyPlanesInstance.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "OnlineSessionSettings.h"
#include "Online.h"
#include "Kismet/GameplayStatics.h"

void UFlappyPlanesInstance::Init()
{
    Super::Init();
    if(bNeedLogin)
    {
        EOSLoginWithDeviceID();
    }
}

void UFlappyPlanesInstance::EOSLoginWithDeviceID()
{
    IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get(TEXT("EOS"));
    if (OnlineSub)
    {
        IOnlineIdentityPtr Identity = OnlineSub->GetIdentityInterface();
    
        if (Identity.IsValid())
        {
            FOnlineAccountCredentials Credentials;
            Credentials.Type = TEXT("accountportal");
            Credentials.Id = TEXT("");
            Credentials.Token = TEXT("");
    
            Identity->OnLoginCompleteDelegates[0].AddLambda([&](int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
                {
                    if (bWasSuccessful)
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Login successful"));
                    }
                    else
                    {
                        UE_LOG(LogTemp, Error, TEXT("Login failed %s"), *Error);
                    }
                    OnEOSLoginCompleted(bWasSuccessful, *Error);
                });
            if(Identity->GetLoginStatus(0) != ELoginStatus::LoggedIn)
            {
                Identity->Login(0, Credentials);
            }
            else
            {
                OnEOSLoginCompleted(false, TEXT("Player already logged in"));
                UE_LOG(LogTemp, Warning, TEXT("Player already logged in"));
            }
        }
        else
        {
            OnEOSLoginCompleted(false, TEXT("EOS Identity Interface is invalid"));
            UE_LOG(LogTemp, Error, TEXT("EOS Identity Interface is invalid"));
        }
    }
    else
    {
        OnEOSLoginCompleted(false, TEXT("EOS Online Subsystem not found"));
        UE_LOG(LogTemp, Error, TEXT("EOS Online Subsystem not found"));
    }
}

void UFlappyPlanesInstance::CreateSession()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            FOnlineSessionSettings SessionSettings;
            SessionSettings.bIsLANMatch = false;
            SessionSettings.bUsesPresence = true;
            SessionSettings.NumPublicConnections = MaxPlayers;
            SessionSettings.bAllowJoinInProgress = true;
            SessionSettings.bAllowJoinViaPresence = true;
            SessionSettings.bShouldAdvertise = true;
            FName SessionName = FName("GameSession");
            SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UFlappyPlanesInstance::OnCreateSessionCompleted));
            if (SessionInterface->CreateSession(0, SessionName, SessionSettings))
            {
                OnCreateSessionCompleted(FName("CreateSessionStarted"), false);
            }
            else
            {
                OnCreateSessionCompleted(FName("CreateSessionFailed"), false);
            }
        }
        else
        {
            OnCreateSessionCompleted(FName("CreateSessionFailed, SessionInterface is Invalid"), false);
        }
    }
    else
    {
        OnCreateSessionCompleted(FName("CreateSessionFailed, Subsystem in nullptr"), false);
    }
}

void UFlappyPlanesInstance::FindSessions()
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            //SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UFlappyPlanesInstance::OnFindSessionsCompleted));

            SessionSearch = MakeShareable(new FOnlineSessionSearch());
            SessionSearch->MaxSearchResults = 100;
            SessionSearch->bIsLanQuery = false;      
            SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UFlappyPlanesInstance::OnFindSessionsCompleted);
            SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
        }
    }
}

void UFlappyPlanesInstance::OnFindSessionsCompleted(bool bWasSuccessful)
{    
    TArray<FBlueprintSessionResult> FoundSessions;
    if (bWasSuccessful)
    {
        for(FOnlineSessionSearchResult SearchResult : SessionSearch->SearchResults)
        {
            FBlueprintSessionResult SessionResult;
            SessionResult.OnlineResult = SearchResult;
            FoundSessions.Add(SessionResult);
        }
        OnSessionSearchCompleted.Broadcast(true, FoundSessions);
    }
    else
    {
        OnSessionSearchCompleted.Broadcast(false, FoundSessions);
    }
}

void UFlappyPlanesInstance::JoinSession(FBlueprintSessionResult Session)
{
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    if (Subsystem)
    {
        IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UFlappyPlanesInstance::OnJoinSessionCompleted);
            SessionInterface->JoinSession(0, FName("GameSession"), Session.OnlineResult);
        }
    }
}

void UFlappyPlanesInstance::OnJoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult)
{        
    IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
    switch (JoinResult)
    {
    case EOnJoinSessionCompleteResult::Success:
        OnJoinSessionCompleted_BP(true, "Success");
        if (Subsystem)
        {
            IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
            if (SessionInterface.IsValid())
            {
                FString ConnectString;
                if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
                {
                    APlayerController* PlayerController = GetFirstLocalPlayerController();
                    if (PlayerController)
                    {
                        OnJoinSessionCompleted_BP(false, FName(*ConnectString));        
                        OnJoinSessionCompleted_BP(false, "ConnectString: ");
                        UE_LOG(LogTemp, Log, TEXT("Joining session at %s"), *ConnectString);
                        PlayerController->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);

                        
                    }
                    else
                    {
                        OnJoinSessionCompleted_BP(false, "PlayerControllerInvelid");
                    }
                }
            }
        }
        break;
    case EOnJoinSessionCompleteResult::SessionIsFull:
        OnJoinSessionCompleted_BP(false, "SessionIsFull");
        break;
    case EOnJoinSessionCompleteResult::SessionDoesNotExist:
        OnJoinSessionCompleted_BP(false, "SessionDoesNotExist");
        break;
    case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
        OnJoinSessionCompleted_BP(false, "CouldNotRetrieveAddress");
        break;
    case EOnJoinSessionCompleteResult::AlreadyInSession:
        OnJoinSessionCompleted_BP(false, "AlreadyInSession");
        break;
    case EOnJoinSessionCompleteResult::UnknownError:
        OnJoinSessionCompleted_BP(false, "UnknownError");
        break;
    default:
        break;
    } 
}

void UFlappyPlanesInstance::OnJoinSessionCompleted_BP_Implementation(bool bWasSuccessful, FName Message)
{
}

void UFlappyPlanesInstance::OnCreateSessionCompleted_Implementation(FName SessionName, bool bWasSuccessful)
{
    UGameplayStatics::OpenLevel(GetWorld(), "GameMap", true, "listen");
    //BP
}

void UFlappyPlanesInstance::OnEOSLoginCompleted_Implementation(bool bWasSuccessful, FName Error)
{
    //BP
}
