// Fill out your copyright notice in the Description page of Project Settings.


#include "DynamicShadow.h"
#include "FlappyPlane.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

// Sets default values
ADynamicShadow::ADynamicShadow()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
	RootComponent = ProceduralMesh;
}

// Called when the game starts or when spawned
void ADynamicShadow::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADynamicShadow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (Plane)
	{
		FVector CenterLocation = Plane->GetActorLocation();
		SetActorLocation(CenterLocation);
		TArray<int32> Triangles;
		TArray<FVector> Vertices;
		Vertices.Add(FVector(0, 0, 0));
		float AngleBetweenTraces = 360.f / float(VerticesCount);
		for (int32 i = 0; i < VerticesCount; i++)
		{
			float TraceAngle = i * AngleBetweenTraces;
			FRotator TraceRoation = FRotator(TraceAngle, 0, 0);
			FVector TraceDirection;
			TraceDirection.X = 0;
			TraceDirection.Y = FMath::Cos(TraceAngle);
			TraceDirection.Z = FMath::Sin(TraceAngle);
			FVector TraceEnd = TraceDirection * Radius + CenterLocation;
			FHitResult Hit;
			GetWorld()->LineTraceSingleByChannel(Hit, CenterLocation, TraceEnd, TraceChannel);
			FVector Vertix;
			if (Hit.bBlockingHit)
			{
				Vertix = Hit.ImpactPoint - CenterLocation;
			}
			else
			{
				Vertix = TraceDirection * Radius;
			}
			Vertices.Add(Vertix);
			//DrawDebugLine(GetWorld(), CenterLocation, Vertix , FColor::Red, false, 0.f);
			if (i > 0)
			{
				Triangles.Add(0);
				Triangles.Add(i);
				Triangles.Add(i + 1);
			}
		}
		TArray<FVector> Normals;
		TArray<FVector2D> UV0;
		TArray<FLinearColor> VertexColors;
		TArray<FProcMeshTangent>Tangents;
		ProceduralMesh->ClearAllMeshSections();
		ProceduralMesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UV0, VertexColors, Tangents, false);
		if(Material)
		{
			ProceduralMesh->SetMaterial(0, Material);
		}
	}
}

