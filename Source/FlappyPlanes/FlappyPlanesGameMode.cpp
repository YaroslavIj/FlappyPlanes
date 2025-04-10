
#include "FlappyPlanesGameMode.h"
#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"
#include "GamePawn.h"
#include "FlappyPlanesInstance.h"
#include "AIGamePawn.h"

void AFlappyPlanesGameMode::PostLogin(APlayerController* NewPlayer)
{
	if(NewPlayer)
	{
		PlayerControllers.Add(NewPlayer);
		AActor* PlayerStart = FindPlayerStart(NewPlayer);
		if(PawnClass && GetWorld())
		{
			if (AGamePawn* SpawnedPawn = GetWorld()->SpawnActor<AGamePawn>(PawnClass, PlayerStart->GetActorTransform()))
			{
				NewPlayer->Possess(SpawnedPawn);
				SpawnPlane(SpawnedPawn);
			}
		}
	}

	if(UFlappyPlanesInstance* GI = Cast<UFlappyPlanesInstance>(GetGameInstance()))
	{	
		if (!GI->GetIsOnline())
		{
			if (AAIGamePawn* SpawnedPawn = GetWorld()->SpawnActor<AAIGamePawn>(AIPawnClass))
			{
				AIGamePawns.Add(SpawnedPawn);
				AFlappyPlane* Plane = SpawnPlane(SpawnedPawn);
				if (Plane)
				{
					Plane->bHasInfiniteAmmo = true;
				}
			}
		}
		if (PlayerControllers.Num() == GI->GetMaxPlayers() && GI->GetIsOnline() || !GI->GetIsOnline())
		{
			StartGame_BP();
			if (GetWorld())
			{
				GetWorld()->GetTimerManager().SetTimer(StartGameTimer, this, &AFlappyPlanesGameMode::StartGame, StartGameTime, false);
			}
		}
	}
}

void AFlappyPlanesGameMode::StartGame()
{
	for (APlayerController* Controller : PlayerControllers)
	{
		if (AGamePawn* Pawn = Cast<AGamePawn>(Controller->GetPawn()))
		{
			if (AFlappyPlane* Plane = Pawn->Plane)
			{
				Plane->StartGame();		
			}
		}
	}	
	for(AAIGamePawn* AIPawn : AIGamePawns)
	{	
		if(AIPawn->Plane)
		{
			AIPawn->Plane->StartGame();
			AIPawn->StartGame();
		}
	}
}

void AFlappyPlanesGameMode::OnPlaneDied(AFlappyPlane* Plane)
{
	if (Plane && Plane->GetOwner())
	{
		if (AGamePawn* Pawn = Cast<AGamePawn>(Plane->GetOwner()))
		{
			if (APlayerController* Controller = Cast<APlayerController>(Pawn->GetController()))
			{
				Plane->Destroy();
				EndGame_BP(Controller);
				APlayerController* ServerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
				if (ServerController)
				{
					ServerController->ServerPause();
				}
			}
		}
	}
}

AFlappyPlane* AFlappyPlanesGameMode::SpawnPlane(AGamePawn* PawnOwner)
{
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), OutActors);
	if (PlaneClass && GetWorld())
	{
		for (AActor* Point : OutActors)
		{
			if (Point && Point->Tags.Contains("Plane") && !Point->Tags.Contains("Occupied"))
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = PawnOwner;
				if (AFlappyPlane* SpawnedPlane = GetWorld()->SpawnActor<AFlappyPlane>(PlaneClass, Point->GetActorTransform(), SpawnParams))
				{
					SpawnedPlane->PawnOwner = PawnOwner;
					SpawnedPlane->OnPlaneDied.AddDynamic(this, &AFlappyPlanesGameMode::OnPlaneDied);
					PawnOwner->Plane = SpawnedPlane;
					Point->Tags.Add("Occupied");

					if (Point->Tags.Contains("Forward"))
					{
						SpawnedPlane->bIsMovingForward = true;
					}
					else if (Point->Tags.Contains("Backward"))
					{
						SpawnedPlane->bIsMovingForward = false;
					}
					return SpawnedPlane;
				}
			}
		}
	}
	return nullptr;
}

void AFlappyPlanesGameMode::EndGame_BP_Implementation(APlayerController* Looser)
{
	//BP
}

void AFlappyPlanesGameMode::StartGame_BP_Implementation() 
{
	//BP
}
