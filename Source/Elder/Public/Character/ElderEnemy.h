// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/ElderCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "Interaction/HighlightInterface.h"
#include "ElderEnemy.generated.h"

/**
 * 
 */
UCLASS()
class ELDER_API AElderEnemy : public AElderCharacterBase, public IEnemyInterface, public IHighlightInterface
{
	GENERATED_BODY()
	
};
