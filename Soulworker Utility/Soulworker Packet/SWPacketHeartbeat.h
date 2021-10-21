#pragma once
#include ".\Soulworker Packet\SWPacket.h"

/*
* ���� ���� �𸣰��� ����
* 1�ʸ��� ��� ���ƿ�
*/

#pragma pack(push, 1)
typedef struct _SWPACKETHEARTBEAT {
	UINT32 _millisecond; // 1000�� 1���� ����
	BYTE _unknown01[4];
}SWPACKETHEARTBEAT;

#pragma pack(pop)

class SWPacketHeartbeat : public SWPacket {
protected:
	SWPacketHeartbeat() {}

public:
	SWPacketHeartbeat(SWHEADER* swheader, BYTE* data);
	~SWPacketHeartbeat() {}

	VOID Do();
	VOID Log();
	VOID Debug();
};