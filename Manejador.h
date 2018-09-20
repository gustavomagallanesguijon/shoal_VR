/*=================================================
* FileName: Manejador.h
* 
* Created by: Gustavo Magallanes Guijón
* Project name: Cardumen
* Unreal Engine version: 4.19.1
* Created on: 2018/04/17
*
* =================================================*/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Manejador.generated.h"


public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	TArray<UClass*> flockTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	TArray<float> numInFlock;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	float minZ = -9000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	float maxZ = -950.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	float limitesPecera = 10000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	bool attachToPlayer = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	bool DebugMode = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Config)
	UClass* playerType;

	AActor* player;

	float getMinZ();

	float getMaxZ();

	AManejador(const FObjectInitializer& ObjectInitializer);

protected:

	virtual void Tick(float val) override;

	void setup();

	void moveToPlayer();

	float maxX;
	float maxY;
	float minX;
	float minY;

	bool isSetup = false;

	bool PezSpawned = false;
};
