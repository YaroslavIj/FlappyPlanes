
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
			FVector CurrentPlaneLocation = Plane->GetActorLocation();
			CurrentPlaneLocation.X = 0;
			FVector CameraLocation = GetActorLocation();
			CameraLocation.X = 0;
			float Distance = FVector::Dist(CurrentPlaneLocation, CameraLocation);
			if (Distance > PlaneMoveRadius)
			{
				FVector Direction = FVector(CurrentPlaneLocation - CameraLocation);
				Direction.Normalize();
				FVector CameraMove = Direction * (Distance - PlaneMoveRadius);
				SetActorLocation(GetActorLocation() + CameraMove);
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

void AGamePawn::Fire_Server_Implementation(bool bIsSpeedUp)
{
	if (Plane)
	{
		Plane->SetIsFiring_Server(bIsSpeedUp);
	}
}
