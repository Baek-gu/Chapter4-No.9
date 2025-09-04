#include "Game/BBGameModeBase.h"
#include "Player/BBPlayerState.h"
#include "Game/BBGameStateBase.h"
#include "Player/BBPlayerController.h"
#include "EngineUtils.h"


void ABBGameModeBase::OnPostLogin(AController* NewPlayer)
{
	Super::OnPostLogin(NewPlayer);

	ABBPlayerController* BBPlayerController = Cast<ABBPlayerController>(NewPlayer);
	if (IsValid(BBPlayerController) == true)
	{
		BBPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));
		
		AllPlayerControllers.Add(BBPlayerController);

		ABBPlayerState* BBPS = BBPlayerController->GetPlayerState<ABBPlayerState>();
		if (IsValid(BBPS) == true)
		{
			BBPS->PlayerNameString = TEXT("Player") + FString::FromInt(AllPlayerControllers.Num());
		}

		ABBGameStateBase* BBGameStateBase =  GetGameState<ABBGameStateBase>();
		if (IsValid(BBGameStateBase) == true)
		{
			BBGameStateBase->MulticastRPCBroadcastLoginMessage(BBPS->PlayerNameString);
		}
	}
}

void ABBGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	SecretNumberString = GenerateSecretNumber();
}


FString ABBGameModeBase::GenerateSecretNumber()
{
	TArray<int32> Numbers;
	for (int32 i = 1; i <= 9; ++i)
	{
		Numbers.Add(i);
	}

	FMath::RandInit(FDateTime::Now().GetTicks());
	
	FString Result;
	for (int32 i = 0; i < 3; ++i)
	{
		int32 Index = FMath::RandRange(0, Numbers.Num() - 1);
		Result.Append(FString::FromInt(Numbers[Index]));
		Numbers.RemoveAt(Index);
	}
	return Result;
}

bool ABBGameModeBase::IsGuessNumberString(const FString& InNumberString)
{
	if (InNumberString.Len() != 3)
	{
		return false;
	}

	TSet<TCHAR> UniqueDigits;
	for (TCHAR C : InNumberString)
	{
		if (FChar::IsDigit(C) == false || C == '0')
		{
			return false;
		}
		UniqueDigits.Add(C);
	}

	if (UniqueDigits.Num() != 3)
	{
		return false;
	}

	return true;
}

FString ABBGameModeBase::JudgeResult(const FString& InSecretNumberString, const FString& InGuessNumberString)
{
	int32 StrikeCount = 0, BallCount = 0;

	for (int32 i = 0; i < 3; ++i)
	{
		if (InSecretNumberString[i] == InGuessNumberString[i])
		{
			StrikeCount++;
		}
		
		else 
		{
			FString PlayerGuessChar = FString::Printf(TEXT("%c"), InGuessNumberString[i]);
			if (InSecretNumberString.Contains(PlayerGuessChar))
			{
				BallCount++;				
			}
		}
	}

	if (StrikeCount == 3)
	{
		return TEXT("3S");
	}
	
	if (StrikeCount == 0 && BallCount == 0)
	{
		return TEXT("OUT");
	}

	return FString::Printf(TEXT("%dS%dB"), StrikeCount, BallCount);
}

void ABBGameModeBase::ProcessPlayerChat(ABBPlayerController* InChattingPlayerController, const FString& InChatMessage)
{
	ABBPlayerState* BBPS = InChattingPlayerController->GetPlayerState<ABBPlayerState>();
	if (IsValid(BBPS) == false)
	{
		return;
	}
	
	FString MessageToBroadcast;
	int32 StrikeCount = 0;
	bool bWasGuess = false;

	if (IsGuessNumberString(InChatMessage) == true)
	{
		if (BBPS->CurrentGuessCount < BBPS->MaxGuessCount)
		{
			bWasGuess = true;
			IncreaseGuessCount(InChattingPlayerController);
			
			FString JudgeResultString = JudgeResult(SecretNumberString, InChatMessage);
			if (JudgeResultString.Contains(TEXT("S")))
			{
				StrikeCount = FCString::Atoi(&JudgeResultString[0]);
			}
			
			FString PlayerInfo = BBPS->GetPlayerInfoString();
			MessageToBroadcast = PlayerInfo + TEXT(": ") + InChatMessage + TEXT(" -> ") + JudgeResultString;
		}
		
		else
		{
			FString PlayerInfo = BBPS->GetPlayerInfoString();
			MessageToBroadcast = PlayerInfo + TEXT(": ") + InChatMessage;
		}
	}
	
	else
	{
		FString PlayerInfo = BBPS->GetPlayerInfoString();
		MessageToBroadcast = PlayerInfo + TEXT(": ") + InChatMessage;
	}
	
	for (TActorIterator<ABBPlayerController> It(GetWorld()); It; ++It)
	{
		ABBPlayerController* BBPlayerController = *It;
		if (IsValid(BBPlayerController) == true)
		{
			BBPlayerController->ClientRPCPrintChatMessageString(MessageToBroadcast);
		}
	}
	
	if (bWasGuess)
	{
		JudgeGame(InChattingPlayerController, StrikeCount);
	}
}


void ABBGameModeBase::IncreaseGuessCount(ABBPlayerController* InPlayerController)
{
	ABBPlayerState* BBPS = InPlayerController->GetPlayerState<ABBPlayerState>();
	if (IsValid(BBPS) == true)
	{
		BBPS->CurrentGuessCount++;
	}
}

void ABBGameModeBase::ResetGame()
{
	SecretNumberString = GenerateSecretNumber();

	for (const auto& BBPlayerController : AllPlayerControllers)
	{
		ABBPlayerState* BBPS = BBPlayerController->GetPlayerState<ABBPlayerState>();
		if (IsValid(BBPS) == true)
		{
			BBPS->CurrentGuessCount = 0;
		}
	}
}

void ABBGameModeBase::JudgeGame(ABBPlayerController* InChattingPlayerController, int InStrikeCount)
{
	if (3 == InStrikeCount)
	{
		ABBPlayerState* BBPS = InChattingPlayerController->GetPlayerState<ABBPlayerState>();
		if (IsValid(BBPS) == true)
		{
			FString WinMessage = BBPS->PlayerNameString + TEXT(" has won the game.");
			for (const auto& BBPlayerController : AllPlayerControllers)
			{
				BBPlayerController->NotificationText = FText::FromString(WinMessage);
			}
			ResetGame();
		}
	}
	
	else
	{
		bool bIsDraw = true;
		for (const auto& BBPlayerController : AllPlayerControllers)
		{
			ABBPlayerState* BBPS = BBPlayerController->GetPlayerState<ABBPlayerState>();
			if (IsValid(BBPS) == true)
			{
				if (BBPS->CurrentGuessCount < BBPS->MaxGuessCount)
				{
					bIsDraw = false;
					break;
				}
			}
		}

		if (true == bIsDraw)
		{
			for (const auto& BBPlayerController : AllPlayerControllers)
			{
				BBPlayerController->NotificationText = FText::FromString(TEXT("Draw..."));
			}
			ResetGame();
		}
	}
}