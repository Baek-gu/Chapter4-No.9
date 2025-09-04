#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BBGameModeBase.generated.h"

class ABBPlayerController;

UCLASS()
class BASEBALL_API ABBGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void OnPostLogin(AController* NewPlayer) override;

	virtual void BeginPlay() override;

	FString GenerateSecretNumber();

	bool IsGuessNumberString(const FString& InNumberString);

	FString JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString);
	
	void ProcessPlayerChat(ABBPlayerController* InChattingPlayerController, const FString& InChatMessage);

	void IncreaseGuessCount(ABBPlayerController* InChattingPlayerController);

	void ResetGame();

	void JudgeGame(ABBPlayerController* InChattingPlayerController, int InStrikeCount);
	
protected:
	FString SecretNumberString;

	TArray<TObjectPtr<ABBPlayerController>> AllPlayerControllers;
};