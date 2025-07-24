// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldDynamicShadow.h"
#include "Kismet/GameplayStatics.h"
#include "FLappyPlane.h"
#include "GamePawn.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AWorldDynamicShadow::AWorldDynamicShadow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProclMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = ProclMesh;
}

// Called when the game starts or when spawned
void AWorldDynamicShadow::BeginPlay()
{
	Super::BeginPlay();

	AActor* Camera = Cast<AActor>(Plane->PawnOwner);
	if(Camera)
	{
		float DistanceToCamara = FMath::Abs(Plane->GetActorLocation().X - Camera->GetActorLocation().X);
		FVector CameraDirection = Camera->GetActorForwardVector();
		FVector CenterDeprojected = Camera->GetActorLocation() + CameraDirection * DistanceToCamara;
		SetActorLocation(CenterDeprojected);
	}

	/*if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(UpdateTimer, this, &AWorldDynamicShadow::Update, UpdateRate, true);
	}*/
	//Update();
}

void AWorldDynamicShadow::Update()
{
	if(ProclMesh)
	{
		UGameViewportClient* Viewport = GEngine->GameViewport;
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC && Viewport && Plane)
		{
			AActor* Camera = Cast<AActor>(Plane->PawnOwner);
			if (Camera)
			{
				float DistanceToCamara = FMath::Abs(Plane->GetActorLocation().X - Camera->GetActorLocation().X);
				/*FVector2D ScreenSize;
				Viewport->GetViewportSize(ScreenSize);*/
				FVector WorldLocation;
				FVector WorldDirection;
				PC->DeprojectScreenPositionToWorld(0, 0, WorldLocation, WorldDirection);
				//DrawDebugLine(GetWorld(), WorldLocation, WorldLocation + WorldDirection * 1000, FColor::Red, false, 0.f);
				//PC->DeprojectScreenPositionToWorld(ScreenSize.X, ScreenSize.Y, WorldLocation, WorldDirection);
				FVector CameraDirection = Camera->GetActorForwardVector();

				float cos = FVector::DotProduct(CameraDirection, WorldDirection);
				float tg = FMath::Tan(FMath::Acos(cos));
				float HalfDiagonalLenght = tg * DistanceToCamara;
				FVector CenterDeprojected = Camera->GetActorLocation() + CameraDirection * DistanceToCamara;
				FVector DiagonaleDirection = WorldLocation - Camera->GetActorLocation();
				DiagonaleDirection.X = 0;
				DiagonaleDirection.Normalize();
				//FVector CornerPoint = CenterDeprojected + DiagonaleDirection * HalfDiagonalLenght;
				FVector Diagonale = DiagonaleDirection * HalfDiagonalLenght;
				//DrawDebugLine(GetWorld(), CenterDeprojected, CenterDeprojected + Diagonale, FColor::Red, false, 0.f);
				float YDistance = FMath::Abs(Diagonale.Y);
				float Angle = TracesAngle + 90;
				float Offset = MeshZDistance * FMath::Tan(Angle);
				//YDistance -= Offset / 2;
				float ZDistance = FMath::Abs(Diagonale.Z);
				int32 TracesCount = int((YDistance - Offset / 2) * 2 / MeshResolution);
				FVector TracesLocalStartLocation = FVector(0, -YDistance, MeshZDistance);
				//FVector TracesStartLocation = CenterDeprojected + TracesLocalStartLocation;
				TArray<FVector> MainVertices;
				TArray<int32> MainTriangles;

				//FVector TraceDirection;
				TraceDirection.X = 0;
				TraceDirection.Y = FMath::Cos(FMath::DegreesToRadians(TracesAngle));
				TraceDirection.Z = FMath::Sin(FMath::DegreesToRadians(TracesAngle));

				ProclMesh->ClearAllMeshSections();

				float ZOffset = CenterDeprojected.Z - GetActorLocation().Z;
				float YOffset = CenterDeprojected.Y - GetActorLocation().Y;
				int32 ResolutionsCountY = YOffset / MeshResolution;
				YOffset = ResolutionsCountY * MeshResolution;
				UE_LOG(LogTemp, Warning, TEXT("YOffset = %f"), YOffset);
				AddActorWorldOffset(FVector(0, YOffset, ZOffset));

				for (int32 i = 0; i < TracesCount; i++)
				{
					FVector LocalStart = TracesLocalStartLocation + FVector(0, MeshResolution * i, 0);
					FVector Start = GetActorLocation() + LocalStart;

					MainVertices.Add(LocalStart);


					FVector LocalEnd = LocalStart + TraceDirection * MeshZDistance / FMath::Abs(TraceDirection.Z) + TraceDirection * ZDistance / FMath::Abs(TraceDirection.Z);
					FVector End = GetActorLocation() + LocalEnd;

					FHitResult Hit;
					GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TraceChannel);
					//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.f);
					if (Hit.bBlockingHit)
					{
						FVector MainWorldHitDirection = Hit.ImpactPoint - End;
						MainVertices.Add(LocalEnd + MainWorldHitDirection);
					}
					else
					{
						MainVertices.Add(LocalEnd);
					}
				}
				UE_LOG(LogTemp, Warning, TEXT("MainVertices before optimization: %d"), MainVertices.Num());
				/*for (int32 i = MainVertices.Num(); i > 0; i -= 2)
				{
					if (MainVertices.IsValidIndex(i - 1) && MainVertices.IsValidIndex(i + 1) && MainVertices.IsValidIndex(i + 3))
					{
						TArray<FVector> VerticesToCheck = { MainVertices[i - 1], MainVertices[i + 3] };

						bool bNeedToLeave = false;
						for (FVector OtherVertex : VerticesToCheck)
						{
							if (!FMath::IsNearlyEqual(OtherVertex.Z, MainVertices[i + 1].Z, MeshResolutionStepUp))
							{
								bNeedToLeave = true;
							}

						}
						if (!bNeedToLeave)
						{
							MainVertices.RemoveAt(i + 1);
							MainVertices.RemoveAt(i);
						}
					}
				}*/
				UE_LOG(LogTemp, Warning, TEXT("MainVertices after optimization: %d"), MainVertices.Num());

				for (int32 i = 0; i < MainVertices.Num(); i++)
				{
					if (i % 2 == 0)
					{
						MainTriangles.Add(i);
						MainTriangles.Add(i + 1);
						MainTriangles.Add(i + 2);
					}
					else if (i % 2 == 1)
					{
						MainTriangles.Add(i);
						MainTriangles.Add(i + 2);
						MainTriangles.Add(i + 1);
					}
				}
				TArray<FVector> Normals;
				TArray<FVector2D> UV0;
				TArray<FLinearColor> VertexColors;
				TArray<FProcMeshTangent>Tangents;
				
				ProclMesh->CreateMeshSection_LinearColor(0, MainVertices, MainTriangles, Normals, UV0, VertexColors, Tangents, false);
				//ProceduralMesh->UpdateMeshSection_LinearColor(0, MainVertices, Normals, UV0, VertexColors, Tangents, false);
				if (MainMaterial)
				{
					ProclMesh->SetMaterial(0, MainMaterial);
				}

				TArray<int32> Sections;
				for (int32 i = 1; i < MainVertices.Num(); i += 2)
				{
					if (MainVertices.IsValidIndex(i + 2) && FMath::IsNearlyEqual(FMath::Abs(MainVertices[i].Z), ZDistance, MeshResolutionStepUp) && !FMath::IsNearlyEqual(FMath::Abs(MainVertices[i + 2].Z), ZDistance, MeshResolutionStepUp))
					{
						Sections.Add(i);
					}
					else if (!FMath::IsNearlyEqual(FMath::Abs(MainVertices[i].Z), ZDistance, MeshResolutionStepUp) && !MainVertices.IsValidIndex(i + 2) || !FMath::IsNearlyEqual(FMath::Abs(MainVertices[i].Z), ZDistance, MeshResolutionStepUp) && !MainVertices.IsValidIndex(i - 2))				
					{
						UE_LOG(LogTemp, Warning, TEXT("MainVertices[i].Z = %f, ZDistance = %f"), MainVertices[i].Z, ZDistance);
						Sections.Add(i);
					}
					else if (!FMath::IsNearlyEqual(FMath::Abs(MainVertices[i].Z), ZDistance, MeshResolutionStepUp) && FMath::IsNearlyEqual(FMath::Abs(MainVertices[i + 2].Z), ZDistance, MeshResolutionStepUp))
					{
						Sections.Add(i + 4);
					}
				}
				for (int32 i = 0; i < Sections.Num(); i += 2)
				{
					TArray<FVector> ShadowVertices;
					TArray<int32> ShadowTriangles;
					if (Sections.IsValidIndex(i + 1))
					{
						for (int32 j = Sections[i]; j < Sections[i + 1]; j += 2)
						{
							FVector LocalStart = MainVertices[j];
							//FVector Start = CenterDeprojected + LocalStart;

							float Length = FMath::Abs((LocalStart.Z + ZDistance) / FMath::Cos(FMath::DegreesToRadians(TracesAngle)));

							FVector LocalEnd = LocalStart + TraceDirection * Length;
							//DrawDebugSphere(GetWorld(), CenterDeprojected + LocalEnd, 10, 10, FColor::Red);
							//DrawDebugLine(GetWorld(), CenterDeprojected + LocalStart, CenterDeprojected + LocalEnd, FColor::Red, false, 0.f);
							//ShadowVertices.Add(FVector(LocalStart.X, LocalStart.Y, -ZDistance));
							ShadowVertices.Add(LocalEnd);
							ShadowVertices.Add(LocalStart);

						}
						/*for (int32 j = ShadowVertices.Num(); j > 0; j -= 2)
						{
							if (ShadowVertices.IsValidIndex(j - 1) && ShadowVertices.IsValidIndex(j + 1) && ShadowVertices.IsValidIndex(j + 3))
							{
								TArray<FVector> VerticesToCheck = { ShadowVertices[j - 1], ShadowVertices[j + 3] };

								bool bNeedToLeave = false;
								for (FVector OtherVertex : VerticesToCheck)
								{
									if (!FMath::IsNearlyEqual(OtherVertex.Z, ShadowVertices[j + 1].Z, MeshResolutionStepUp))
									{
										bNeedToLeave = true;
									}

								}
								if (!bNeedToLeave)
								{
									ShadowVertices.RemoveAt(j + 1);
									ShadowVertices.RemoveAt(j);
								}
							}
						}*/
						for (int32 j = 0; j < ShadowVertices.Num(); j++)
						{
							if (j % 2 == 1)
							{
								ShadowTriangles.Add(j);
								ShadowTriangles.Add(j + 1);
								ShadowTriangles.Add(j + 2);
							}
							else if (j % 2 == 0)
							{
								ShadowTriangles.Add(j);
								ShadowTriangles.Add(j + 2);
								ShadowTriangles.Add(j + 1);
							}
						}
						ProclMesh->CreateMeshSection_LinearColor(i / 2 + 1, ShadowVertices, ShadowTriangles, Normals, UV0, VertexColors, Tangents, false);
						if (ShadowMaterial)
						{
							ProclMesh->SetMaterial(i / 2 + 1, ShadowMaterial);
						}
					}
				}

				
				//SetActorLocation(CenterDeprojected);
			}
		}
	}
}

// Called every frame
void AWorldDynamicShadow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//AActor* Camera = Cast<AActor>(Plane->PawnOwner);
	//if(Camera)
	//{
	//	float DistanceToCamara = FMath::Abs(Plane->GetActorLocation().X - Camera->GetActorLocation().X);
	//	FVector CameraDirection = Camera->GetActorForwardVector();
	//	FVector CenterDeprojected = Camera->GetActorLocation() + CameraDirection * DistanceToCamara;
	//	SetActorLocation(CenterDeprojected);
	//}
	//
	//if()
	//UGameViewportClient* Viewport = GEngine->GameViewport;
	//APlayerController* PC = GetWorld()->GetFirstPlayerController();
	//if (PC && Viewport && Plane)
	//{
	//	AActor* Camera = Cast<AActor>(Plane->PawnOwner);
	//	if (Camera)
	//	{
	//		float DistanceToCamara = FMath::Abs(Plane->GetActorLocation().X - Camera->GetActorLocation().X);
	//		/*FVector2D ScreenSize;
	//		Viewport->GetViewportSize(ScreenSize);*/
	//		FVector WorldLocation;
	//		FVector WorldDirection;
	//		PC->DeprojectScreenPositionToWorld(0, 0, WorldLocation, WorldDirection);
	//		//DrawDebugLine(GetWorld(), WorldLocation, WorldLocation + WorldDirection * 1000, FColor::Red, false, 0.f);
	//		//PC->DeprojectScreenPositionToWorld(ScreenSize.X, ScreenSize.Y, WorldLocation, WorldDirection);
	//		FVector CameraDirection = Camera->GetActorForwardVector();
	//
	//		float cos = FVector::DotProduct(CameraDirection, WorldDirection);
	//		float tg = FMath::Tan(FMath::Acos(cos));
	//		float HalfDiagonalLenght = tg * DistanceToCamara;
	//		FVector CenterDeprojected = Camera->GetActorLocation() + CameraDirection * DistanceToCamara;
	//		FVector DiagonaleDirection = WorldLocation - Camera->GetActorLocation();
	//		DiagonaleDirection.X = 0;
	//		DiagonaleDirection.Normalize();
	//		//FVector CornerPoint = CenterDeprojected + DiagonaleDirection * HalfDiagonalLenght;
	//		FVector Diagonale = DiagonaleDirection * HalfDiagonalLenght;
	//		//DrawDebugLine(GetWorld(), CenterDeprojected, CenterDeprojected + Diagonale, FColor::Red, false, 0.f);
	//		float YDistance = FMath::Abs(Diagonale.Y);
	//		float Angle = TracesAngle + 90;
	//		float Offset = MeshZDistance * FMath::Tan(Angle);
	//		//YDistance -= Offset / 2;
	//		float ZDistance = FMath::Abs(Diagonale.Z);
	//		int32 TracesCount = int((YDistance - Offset / 2) * 2 / MeshResolution);
	//		FVector TracesLocalStartLocation = FVector(0, -YDistance, MeshZDistance);
	//		//FVector TracesStartLocation = CenterDeprojected + TracesLocalStartLocation;
	//		TArray<FVector> MainVertices;
	//
	//		//FVector TraceDirection;
	//		TraceDirection.X = 0;
	//		TraceDirection.Y = FMath::Cos(FMath::DegreesToRadians(TracesAngle));
	//		TraceDirection.Z = FMath::Sin(FMath::DegreesToRadians(TracesAngle));
	//
	//		for (int32 i = 0; i < TracesCount; i++)
	//		{
	//			FVector LocalStart = TracesLocalStartLocation + FVector(0, MeshResolution * i, 0);
	//			FVector Start = CenterDeprojected + LocalStart;
	//
	//			MainVertices.Add(LocalStart);
	//
	//
	//			FVector LocalEnd = LocalStart + TraceDirection * MeshZDistance / FMath::Abs(TraceDirection.Z) + TraceDirection * ZDistance / FMath::Abs(TraceDirection.Z);
	//			FVector End = CenterDeprojected + LocalEnd;
	//
	//			FHitResult Hit;
	//			GetWorld()->LineTraceSingleByChannel(Hit, Start, End, TraceChannel);
	//			//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 0.f);
	//			if (Hit.bBlockingHit)
	//			{
	//				FVector MainWorldHitDirection = Hit.ImpactPoint - End;
	//				MainVertices.Add(LocalEnd + MainWorldHitDirection);
	//			}
	//			else
	//			{
	//				MainVertices.Add(LocalEnd);
	//			}
	//		}
	//		UE_LOG(LogTemp, Warning, TEXT("MainVertices before optimization: %d"), MainVertices.Num());
	//		
	//		/*for (int32 i = MainVertices.Num(); i > 0; i -= 2)
	//		{
	//			if (MainVertices.IsValidIndex(i - 1) && MainVertices.IsValidIndex(i + 1) && MainVertices.IsValidIndex(i + 3))
	//			{
	//				TArray<FVector> VerticesToCheck = { MainVertices[i - 1], MainVertices[i + 3] };
	//
	//				bool bNeedToLeave = false;
	//				for (FVector OtherVertex : VerticesToCheck)
	//				{
	//					if (!FMath::IsNearlyEqual(OtherVertex.Z, MainVertices[i + 1].Z, MeshResolutionStepUp))
	//					{
	//						bNeedToLeave = true;
	//					}
	//
	//				}
	//				if (!bNeedToLeave)
	//				{
	//					MainVertices.RemoveAt(i + 1);
	//					MainVertices.RemoveAt(i);
	//				}
	//			}
	//		}*/
	//		UE_LOG(LogTemp, Warning, TEXT("MainVertices after optimization: %d"), MainVertices.Num());
//
	//		TArray<FVector> Normals;
	//		TArray<FVector2D> UV0;
	//		TArray<FLinearColor> VertexColors;
	//		TArray<FProcMeshTangent>Tangents;
	//		//ProceduralMesh->ClearAllMeshSections();
	//		//ProceduralMesh->CreateMeshSection_LinearColor(0, MainVertices, MainTriangles, Normals, UV0, VertexColors, Tangents, false);
	//		bool bNeedToCreateAllShadows = false;
	//		if(MainVertices.Num() == MainVerticesCount)
	//		{
	//			CountOfUpdates++;
	//			ProceduralMesh->UpdateMeshSection_LinearColor(0, MainVertices, Normals, UV0, VertexColors, Tangents, false);
	//			
	//		}
	//		else if (MainMaterial)
	//		{
	//			CountOfDestroys++;
	//			bNeedToCreateAllShadows = true;
	//			TArray<int32> MainTriangles;
	//			for (int32 i = 0; i < MainVertices.Num(); i++)
	//			{
	//				if (i % 2 == 0)
	//				{
	//					MainTriangles.Add(i);
	//					MainTriangles.Add(i + 1);
	//					MainTriangles.Add(i + 2);
	//				}
	//				else if (i % 2 == 1)
	//				{
	//					MainTriangles.Add(i);
	//					MainTriangles.Add(i + 2);
	//					MainTriangles.Add(i + 1);
	//				}
	//			}
	//			
	//			ProceduralMesh->ClearAllMeshSections();
	//			ProceduralMesh->CreateMeshSection_LinearColor(0, MainVertices, MainTriangles, Normals, UV0, VertexColors, Tangents, false);
	//			ProceduralMesh->SetMaterial(0, MainMaterial);
	//			MainVerticesCount = MainVertices.Num();
	//		}
//
	//		TArray<int32> Sections;
	//		for (int32 i = 1; i < MainVertices.Num(); i += 2)
	//		{
	//			if (MainVertices.IsValidIndex(i + 2) && FMath::IsNearlyEqual(FMath::Abs(MainVertices[i].Z), ZDistance, MeshResolutionStepUp) && !FMath::IsNearlyEqual(FMath::Abs(MainVertices[i + 2].Z), ZDistance, MeshResolutionStepUp))
	//			{
	//				Sections.Add(i + 2);
	//			}
	//			else if (!FMath::IsNearlyEqual(FMath::Abs(MainVertices[i].Z), ZDistance, MeshResolutionStepUp) && !MainVertices.IsValidIndex(i + 2) || !FMath::IsNearlyEqual(FMath::Abs(MainVertices[i].Z), ZDistance, MeshResolutionStepUp) && !MainVertices.IsValidIndex(i - 2)
	//				|| !FMath::IsNearlyEqual(FMath::Abs(MainVertices[i].Z), ZDistance, MeshResolutionStepUp) && FMath::IsNearlyEqual(FMath::Abs(MainVertices[i + 2].Z), ZDistance, MeshResolutionStepUp))
	//			{
	//				UE_LOG(LogTemp, Warning, TEXT("MainVertices[i].Z = %f, ZDistance = %f"), MainVertices[i].Z, ZDistance);
	//				Sections.Add(i);
	//			}
	//		}
	//		if (Sections.Num() != ShadowVerticesInSectionsCount.Num())
	//		{
	//			bNeedToCreateAllShadows = true;
	//		}
	//		TArray<int32> NewShadowVerticesInSectionsCount;
	//		for (int32 i = 0; i < Sections.Num(); i += 2)
	//		{
	//			bool bNeedToCreateSection = false;
	//			if(!bNeedToCreateAllShadows)
	//			{
	//				int32 VerticesInSection = Sections[i + 1] - Sections[i];
	//				if (VerticesInSection != ShadowVerticesInSectionsCount[i])
	//				{
	//					bNeedToCreateSection = true;
	//					ShadowVerticesInSectionsCount[i] = VerticesInSection;
	//				}
	//			}
	//			else
	//			{
	//				bNeedToCreateSection = true;
	//			}
	//			TArray<FVector> ShadowVertices;
	//			if (Sections.IsValidIndex(i + 1))
	//			{
	//				for (int32 j = Sections[i]; j < Sections[i + 1]; j += 2)
	//				{
	//					FVector LocalStart = MainVertices[j];
	//					//FVector Start = CenterDeprojected + LocalStart;
//
	//					float Length = FMath::Abs((LocalStart.Z + ZDistance) / FMath::Cos(FMath::DegreesToRadians(TracesAngle)));
//
	//					FVector LocalEnd = LocalStart + TraceDirection * Length;
	//					//DrawDebugSphere(GetWorld(), CenterDeprojected + LocalEnd, 10, 10, FColor::Red);
	//					//DrawDebugLine(GetWorld(), CenterDeprojected + LocalStart, CenterDeprojected + LocalEnd, FColor::Red, false, 0.f);
	//					//ShadowVertices.Add(FVector(LocalStart.X, LocalStart.Y, -ZDistance));
	//					ShadowVertices.Add(LocalEnd);
	//					ShadowVertices.Add(LocalStart);
//
	//				}
	//				/*for (int32 j = ShadowVertices.Num(); j > 0; j -= 2)
	//				{
	//					if (ShadowVertices.IsValidIndex(j - 1) && ShadowVertices.IsValidIndex(j + 1) && ShadowVertices.IsValidIndex(j + 3))
	//					{
	//						TArray<FVector> VerticesToCheck = { ShadowVertices[j - 1], ShadowVertices[j + 3] };
//
	//						bool bNeedToLeave = false;
	//						for (FVector OtherVertex : VerticesToCheck)
	//						{
	//							if (!FMath::IsNearlyEqual(OtherVertex.Z, ShadowVertices[j + 1].Z, MeshResolutionStepUp))
	//							{
	//								bNeedToLeave = true;
	//							}
//
	//						}
	//						if (!bNeedToLeave)
	//						{
	//							ShadowVertices.RemoveAt(j + 1);
	//							ShadowVertices.RemoveAt(j);
	//						}
	//					}
	//				}*/
	//				//ProceduralMesh->CreateMeshSection_LinearColor(i / 2 + 1, ShadowVertices, ShadowTriangles, Normals, UV0, VertexColors, Tangents, false);
	//		
	//				if(!bNeedToCreateSection)
	//				{
	//					ProceduralMesh->UpdateMeshSection_LinearColor(i / 2 + 1, ShadowVertices, Normals, UV0, VertexColors, Tangents, false);
	//				}
	//				else if (ShadowMaterial)
	//				{
	//					TArray<int32> ShadowTriangles;
	//					for (int32 j = 0; j < ShadowVertices.Num(); j++)
	//					{
	//						if (j % 2 == 1)
	//						{
	//							ShadowTriangles.Add(j);
	//							ShadowTriangles.Add(j + 1);
	//							ShadowTriangles.Add(j + 2);
	//						}
	//						else if (j % 2 == 0)
	//						{
	//							ShadowTriangles.Add(j);
	//							ShadowTriangles.Add(j + 2);
	//							ShadowTriangles.Add(j + 1);
	//						}
	//					}
	//					ProceduralMesh->CreateMeshSection_LinearColor(i / 2 + 1, ShadowVertices, ShadowTriangles, Normals, UV0, VertexColors, Tangents, false);
	//					ProceduralMesh->SetMaterial(i / 2 + 1, ShadowMaterial);
	//				}
	//			}
	//		}
	//		SetActorLocation(CenterDeprojected);
	//	}
	//}
	
	Update();
}

