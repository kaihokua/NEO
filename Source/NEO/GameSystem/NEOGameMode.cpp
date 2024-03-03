// Fill out your copyright notice in the Description page of Project Settings.


#include "NEOGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "ProceduralMeshComponent.h"
#include "NEO/GameSystem/SplineCamera.h"
#include "NEO/GameSystem/GameSystem_BattleArea.h"
#include "NEO/GameSystem/SpawnPoint.h"
#include "NEO/CharacterSystem/BossSystem/OdaBase.h"
#include "NEO/CharacterSystem/Enemy/EnamyBase.h"
#include "NEO/CharacterSystem/PlayerSystem/PlayerBase.h"
#include "NEO/CharacterSystem/PlayerSystem/NEOPlayerController.h"

ANEOGameMode::ANEOGameMode()
{

}


void ANEOGameMode::BeginPlay()
{
	Super::BeginPlay();

	// �Q�[���X�e�[�g���擾
	pGameState = Cast<ANEOGameState>(UGameplayStatics::GetGameState(GetWorld()));

	// �v���C���[�R���g���[���[���擾
	PlayerController = Cast<ANEOPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
}

void ANEOGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	// �Q�[���̏�Ԃ��X�V
	if (pGameState) 
	{
		// �Q�[���̏�ԍX�V
		pGameState->UpdateGameState(DeltaTime);
	}
	else 
	{
		UE_LOG(LogTemp, Error, TEXT("Game State is not found"));
		pGameState = Cast<ANEOGameState>(UGameplayStatics::GetGameState(GetWorld()));
	}
}


/*
 * �֐����@�@�@�@�FSetViewTargetWithBlend()
 * �������e�@�@�@�F���݂̃J�����̏���Ԃ�
 * �����P�@�@�@�@�FAActor* _newViewTarget�E�E�E�V�����J�����̏��
 * �����Q�@�@�@�@�Ffloat _blendTime�E�E�E�E�E�E�؂�ւ��ɂ����鎞��
 * �����R�@�@�@�@�FEViewTargetBlendFunction _blendFunc
 * �����S�@�@�@�@�Ffloat _blendExp
 * �����T�@�@�@�@�Fbool _bLockOutgoing
 * �߂�l�@�@�@�@�F���݂̃J�����̏��
 */
void ANEOGameMode::SetViewTargetWithBlend(AActor* _newViewTarget, float _blendTime, EViewTargetBlendFunction _blendFunc, float _blendExp, bool _bLockOutgoing)
{
	// �v���C���[�R���g���[���[���擾
	PlayerController = Cast<ANEOPlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));

	// �R���g���[���[�����鎞�J�������u�����h
	if (PlayerController)
	{
		PlayerController->SetViewTargetWithBlend(_newViewTarget, _blendTime);
	}

	pCamera = _newViewTarget;
}

// �J�����̏����ݒ�
void ANEOGameMode::InitCameraOnPlayer()
{
	SetViewTargetWithBlend(PlayerCamera);
}

/*
 * �֐����@�@�@�@�FGetNowPlayerCamera()
 * �������e�@�@�@�F���݂̃J�����̏���Ԃ�
 * �߂�l�@�@�@�@�F���݂̃J�����̏��
 */
AActor* ANEOGameMode::GetNowPlayerCamera()const
{
	// �o�g���G���A���ɂ��邩�ǂ����ŃJ������I��
	AActor* NowCamera = (bIsOnBattleArea) ? (pCamera) : (PlayerCamera);

	if (NowCamera)
	{
		return NowCamera;
	}
	
	return nullptr;
}


/*
 * �֐����@�@�@�@�FSetIsOnBattleArea()
 * �������e�@�@�@�F�o�g���G���A���N������
 * �����P�@�@�@�@�Fbool _IsBattleArea�E�E�E�E�E�E�E�E�E�E�E�E�E�o�g���G���A�𔭓����邩
 * �����Q�@�@�@�@�FTArray<class ASpawnPoint*> SpawnPoints�E�E�E�o���ꏊ�̔z����
 * �����R�@�@�@�@�FAActor* Camera�E�E�E�E�E�E�E�E�E�E�E�E�E�E�E�o�g���G���A�̃J�������
 * �����S�@�@�@�@�FUProceduralMeshComponent* LeftMesh�E�E�E�E�E�o�g���G���A�̕ǂ̏��(��)
 * �����T�@�@�@�@�FUProceduralMeshComponent* RightMesh �E�E�E�E�o�g���G���A�̕ǂ̏��(�E)
 * �����U�@�@�@�@�FUProceduralMeshComponent* NearMesh�E�E�E�E�E�o�g���G���A�̕ǂ̏��(��O)
 * �߂�l�@�@�@�@�F�Ȃ�
 */
void ANEOGameMode::SetIsOnBattleArea(bool _IsBattleArea,TArray<class ASpawnPoint*> SpawnPoints,
	AActor* Camera = nullptr,
	class UProceduralMeshComponent* LeftMesh = nullptr,
	class UProceduralMeshComponent* RightMesh = nullptr,
	class UProceduralMeshComponent* NearMesh = nullptr,
	float SetWallDealy)
{
	// NULL Check
	if (Camera && LeftMesh && RightMesh && NearMesh) {
		BattleAreaMeshs.Reset();

		BattleAreaMeshs.Add(LeftMesh);
		BattleAreaMeshs.Add(RightMesh);
		BattleAreaMeshs.Add(NearMesh);

		BattleAreaCamera = Camera;

		//�X�|�[���|�C���g���Q�[���X�e�[�g�ɓo�^����
		BattleAreaSpawnPoints.Reset();
		for (auto spawnPoint : SpawnPoints) {
			BattleAreaSpawnPoints.Add(spawnPoint);
		}
	}

	if (SetWallDealy != 0.f)
	{
		// ��莞�Ԍ�ɕǐ���
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		TimerManager.SetTimer(TimerHandle, this, &ANEOGameMode::SetWall, SetWallDealy, false);
	}
	else
	{
		SetWall();
	}

	// �v���C���[�̃J�������Œ�J�����ɕύX
	if (Camera)
	{
		SetViewTargetWithBlend(Camera);
	}

	// �o�g���G���A�ɓG���X�|�[��������.
	SpawnEnemyInBattleArea();

	// �o�g���G���A�̃t���O�I��
	bIsOnBattleArea = _IsBattleArea;

	// �C�x���g�i�s
	++EventIndex;
}

void ANEOGameMode::SetWall()
{
	//�o�g���G���A��L����
	for (auto Mesh : BattleAreaMeshs) {
		if (Mesh) {
			//-------------------�R���W������L����----------------------
			Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("MeshWall is not found"));
		}
	}
}


/*
 * �֐����@�@�@�@�FSpawnEnemy()
 * �������e�@�@�@�F�G�̏o������
 * �����P�@�@�@�@�F�o���ꏊ�̏��
 * �߂�l�@�@�@�@�F�Ȃ�
 */
AActor* ANEOGameMode::SpawnEnemy(ASpawnPoint* spawnPoint)
{
	//NULL Check
	if (!spawnPoint) {
		UE_LOG(LogTemp, Error, TEXT("SpawnPoint is not found"));
		return nullptr;
	}

	//Transform���擾����
	FTransform spawnTransform = spawnPoint->GetTransform();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// �G���X�|�[��
	AActor* spawn_Actor = GetWorld()->SpawnActor<AActor>(spawnPoint->GetSpawnActorClass(), spawnPoint->GetActorLocation(), spawnPoint->GetActorRotation(), SpawnParams);


	// �|�C���^�擾
	AOdaBase* Boss = Cast<AOdaBase>(spawn_Actor);
	AEnamyBase* Enemy = Cast<AEnamyBase>(spawn_Actor);

	if (Enemy) {
		Enemy->SetActorTransform(spawnTransform);
		Enemy->IsAreaEnemy = true;		//Flag Set 

		// �G�l�~�[��ǉ�
		++BattleAreaEnemyCount;

		return Enemy;
	}
	else if (Boss) {
		Boss->SetActorTransform(spawnTransform);
		Boss->IsAreaEnemy = true;		//Flag Set 

		// �G�l�~�[��ǉ�
		++BattleAreaEnemyCount;

		return Boss;
	}

	return nullptr;
}


/*
 * �֐����@�@�@�@�FSetCameraOnPlayer()
 * �������e�@�@�@�F�v���C���[�̃J�����ݒ�
 * �߂�l�@�@�@�@�F�Ȃ�
 */
void ANEOGameMode::SetCameraOnPlayer()
{
	if (PlayerController)
	{
		// �v���C���[�������Ă��邱�Ƃ��m�F
		if (!PlayerController->GetPlayerIsDead())
		{
			// �o�g���G���A���ɂ��邩�ǂ����ŃJ������I��
			AActor* NowCamera = (bIsOnBattleArea)?(pCamera):(PlayerCamera);

			// �v���C���[�̃J�����ɐݒ�
			SetViewTargetWithBlend(NowCamera);
		}
	}
}


/*
 * �֐����@�@�@�@�FRestartGame()
 * �������e�@�@�@�F�Q�[�����Z�b�g
 * �߂�l�@�@�@�@�F�Ȃ�
 */
void ANEOGameMode::RestartGame()
{
	UGameplayStatics::OpenLevel(GetWorld(),"GameMap");
}


/*
 * �֐����@�@�@�@�FSetGamePause()
 * �������e�@�@�@�F�Q�[���|�[�Y
 * �߂�l�@�@�@�@�F�Ȃ�
 */
void ANEOGameMode::SetGamePause(bool _bPaused)
{
	UGameplayStatics::SetGamePaused(GetWorld(), _bPaused);
}

/*
 * �֐����@�@�@�@�FDestroyEnemy()
 * �������e�@�@�@�F�G�̍폜����
 * �����P�@�@�@�@�FAActor* _enemy �E�E�E�E�E�E�E�폜�������G
 * �����Q�@�@�@�@�Fbool _bBattleAreaEnemy�E�E�E�o�g���G���A���̓G���ǂ���
 * �߂�l�@�@�@�@�F�Ȃ�
 */
void ANEOGameMode::DestroyEnemy(AActor* _enemy, bool _bBattleAreaEnemy)
{
	if (!_enemy) { return; }

	// �G���A���̃G�l�~�[��������J�E���g�����炷
	if (_bBattleAreaEnemy)
	{
		Enemies.RemoveSingle(_enemy);
		_enemy->Destroy();
		--BattleAreaEnemyCount;
	}
	else
	{
		_enemy->Destroy();
	}

	// �o�g���G���A�𔲂���
	if (BattleAreaEnemyCount == 0 && bIsOnBattleArea)
	{
		ExitBattleArea();
	}
}


/*
 * �֐����@�@�@�@�FSpawnEnemyInBattleArea()
 * �������e�@�@�@�F�o�g���G���A�ɓG���o��������
 * �߂�l�@�@�@�@�F�Ȃ�
 */
void ANEOGameMode::SpawnEnemyInBattleArea()
{
	//Check SpawnPoints
	if (BattleAreaSpawnPoints.Num() == 0) {
		UE_LOG(LogTemp, Error, TEXT("SpawnPoints is not found"));
		return;
	}

	for (ASpawnPoint* spawnPoint : BattleAreaSpawnPoints) {
		if (!spawnPoint) continue; //Check SpawnPoint

		//�G�𐶐�����
		Enemies.Add(SpawnEnemy(spawnPoint));
	}
}


/*
 * �֐����@�@�@�@�FExitBattleArea()
 * �������e�@�@�@�F�o�g���G���A���甲���鏈��
 * �߂�l�@�@�@�@�F�Ȃ�
 */
void ANEOGameMode::ExitBattleArea()
{
	// ���j��C�x���g
	AGameSystem_BattleArea* Area = Cast<AGameSystem_BattleArea>(GetNowPlayerCamera());
	if (Area) 
	{
		Area->ExitAreaEvent();
	}


	//�o�g���G���A�𖳌���
	bIsOnBattleArea = false;
	for (auto Mesh : BattleAreaMeshs) {
		if (Mesh) {
			Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("MeshWall is not found"));
		}
	}


	//�Œ�J�������v���C���[�̃J�����ɕύX
	if (!PlayerController->GetPlayerIsDead())
	{
		if (PlayerCamera)
		{
			SetViewTargetWithBlend(PlayerCamera, 1.f);
		}
	}
	else
	{
		//Log
		UE_LOG(LogTemp, Warning, TEXT("Player.GetOwner() : <CameraActor> is not found"));
	}
}