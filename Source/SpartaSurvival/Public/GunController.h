
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GunController.generated.h"

class UInputMappingContext;
class UInputAction;


UCLASS()
class SPARTASURVIVAL_API AGunController : public APlayerController
{
	GENERATED_BODY()
public:
	AGunController();

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	UInputMappingContext* WeaponMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	UInputAction* FireAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	UInputAction* ReloadAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun")
	UInputAction* ZoomAction;

};
