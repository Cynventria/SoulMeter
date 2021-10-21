#pragma once
#include ".\Soulworker Packet\SWPacket.h"
#include ".\Damage Meter\Damage Meter.h"
#include <unordered_set>

#pragma pack(push, 1)

typedef struct _SWPACKETDAMAGE_MONSTER {
	UINT32 _monsterID;
	BYTE _unknown01;
	BYTE _damageType;
	UINT32 _totalDMG;
	UINT32 _soulstoneDMG;
	UINT32 _remainHP;
	FLOAT _unknownF1;
	FLOAT _unknownF2;
	FLOAT _unknownF3;
#ifdef SERVER_KOREA
	BYTE _unknown02[18];
#endif
#ifdef SERVER_STEAM
	BYTE _unknown02[10];//���� 22
#endif
}SWPACKETDAMAGE_MONSTER;

typedef struct _SWPACKETDAMAGE_PLAYER {
	UINT32 _playerID;
	BYTE _unknown01[20];
	UINT32 _skillID;
	BYTE _unknown02[2];
	USHORT _maxCombo;
	BYTE _unknown03[2];
}SWPACKETDAMAGE_PLAYER;
	
#pragma pack(pop)
static unordered_set<UINT32> pauseIdList({
	31309102, // �糪�� - ���尡
	31301001, // �ٽ� - �÷��� 1��
	31301002, // �ٽ� - �÷��� 2��
	31310101 // �꼼�븻 - �׳׺긮�� 1��
	// 702 // �Ͻ����� �׽�Ʈ�� Ʈ����
	});
static unordered_set<UINT32> endIdList({
	31308001, // ���� - �������
	31309151, // �糪�� - ��ũ ��
	31301003, // �ٽ� - �÷��� 3��
	31310102 // �꼼�븻 - �׳׺긮�� 2��
	});

class SWPacketDamage : public SWPacket {
private:
	VOID AttackOfMonster();

protected:
	SWPacketDamage() {}

public:
	SWPacketDamage(SWHEADER* swheader, BYTE* data);
	~SWPacketDamage() {}

	VOID Do();
	VOID Debug();
	VOID Log() {};
};