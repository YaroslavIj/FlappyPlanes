
#include "GamePawn.h"
#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

AGamePawn::AGamePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
}

void AGamePawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AGamePawn::BeginPlay()
{
	Super::BeginPlay();

	DefaultCameraLocation = GetActorLocation();


	//TArray<AActor*> OutActors;
	//UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATargetPoint::StaticClass(), OutActors);
	//if(PlaneClass && GetWorld())
	//{
	//	for (AActor* Point : OutActors)
	//	{
	//		if (Point && Point->Tags.Contains("Plane") && !Point->Tags.Contains("Occupied"))
	//		{
	//			Plane = GetWorld()->SpawnActor<AFlappyPlane>(PlaneClass, Point->GetActorTransform());		
	//			if(Plane)
	//			{
	//				Point->Tags.Add("Occupied");
	//
	//				if (Point->Tags.Contains("Forward"))
	//				{
	//					Plane->bIsMovingForward = true;
	//				}
	//				else if (Point->Tags.Contains("Backward"))
	//				{
	//					Plane->bIsMovingForward = false;
	//				}
	//			}
	//		}
	//	}
	//}
}

void AGamePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(GetLocalRole() == ROLE_Authority)
	{
		if (Plane)
		{
			if(!bMoveCamera)
			{
				FVector CurrentPlaneLocation = Plane->GetActorLocation();
				FVector NewPlaneLocation = CurrentPlaneLocation;
				if (CurrentPlaneLocation.Z - DefaultCameraLocation.Z >= PlaneMoveRadiusZ)
				{
					NewPlaneLocation.Z = DefaultCameraLocation.Z - PlaneMoveRadiusZ + 1;
				}
				else if (CurrentPlaneLocation.Z - DefaultCameraLocation.Z <= -PlaneMoveRadiusZ)
				{
					NewPlaneLocation.Z = DefaultCameraLocation.Z + PlaneMoveRadiusZ - 1;
				}
				if (CurrentPlaneLocation.Y - DefaultCameraLocation.Y >= PlaneMoveRadiusY)
				{
					NewPlaneLocation.Y = DefaultCameraLocation.Y - PlaneMoveRadiusY + 1;
				}
				else if (CurrentPlaneLocation.Y - DefaultCameraLocation.Y <= -PlaneMoveRadiusY)
				{
					NewPlaneLocation.Y = DefaultCameraLocation.Y + PlaneMoveRadiusY - 1;
				}
				Plane->SetActorLocation(NewPlaneLocation);
			}
			else
			{
				FVector CurrentPlaneLocation = Plane->GetActorLocation();
				CurrentPlaneLocation.X = 0;
				FVector CurrentCameraLocation = GetActorLocation();
				CurrentCameraLocation.X = 0;
				float Distance = FVector::Dist(CurrentPlaneLocation, CurrentCameraLocation);
				if (Distance > PlaneMoveRadius)
				{
					FVector Direction = FVector(CurrentPlaneLocation - CurrentCameraLocation);
					Direction.Normalize();
					FVector CameraMove = Direction * (Distance - PlaneMoveRadius);
					SetActorLocation(GetActorLocation() + CameraMove);
				}
			}
		}
	}
}

void AGamePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("SpeedUp", IE_Pressed, this, &AGamePawn::SpeedUp);
	PlayerInputComponent->BindAction("SpeedUp", IE_Released, this, &AGamePawn::CancelSpeedUp);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AGamePawn::Fire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AGamePawn::CancelFire);
	PlayerInputComponent->BindAction("NextProjectilesType", IE_Pressed, this, &AGamePawn::NextProjectilesType);
}

void AGamePawn::SpeedUp()
{
	SpeedUp_Server(true);
}

void AGamePawn::SpeedUp_Server_Implementation(bool bIsSpeedUp)
{
	if (Plane)
	{
		Plane->SetIsSpeedUp_Server(bIsSpeedUp);
	}
}
void AGamePawn::CancelSpeedUp()
{
	SpeedUp_Server(false);
}

void AGamePawn::Fire()
{
	Fire_Server(true);
}

void AGamePawn::CancelFire()
{
	Fire_Server(false);
}

void AGamePawn::NextProjectilesType()
{
	if (Plane)
	{
		Plane->NextProjectilesType_Server();
	}
}

void AGamePawn::Fire_Server_Implementation(bool bIsSpeedUp)
{
	if (Plane)
	{
		Plane->SetIsFiring_Server(bIsSpeedUp);
	}
}
