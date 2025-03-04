#include "pch.h"
#include ".\Soulworker Packet\SWPacket.h"
#include ".\Damage Meter\Damage Meter.h"
#include ".\Soulworker Packet\SWPacketWorldChange.h"

SWPacketWorldChange::SWPacketWorldChange(SWHEADER* swheader, BYTE* data) : SWPacket(swheader, data) {

}

VOID SWPacketWorldChange::Do() {
	
	// 재시작시 자동 리셋
	DAMAGEMETER.Clear();

	SWPACKETWORLDCHANGE* world_change = (SWPACKETWORLDCHANGE*)(_data + sizeof(SWHEADER));

	DAMAGEMETER.SetMyID(world_change->_id);
	DAMAGEMETER.SetWorldID(world_change->_worldID);

	DAMAGEMETER.SetMazeState(FALSE);
}

VOID SWPacketWorldChange::Log() {

}

VOID SWPacketWorldChange::Debug() {
	SWPACKETWORLDCHANGE* world_change = (SWPACKETWORLDCHANGE*)(_data + sizeof(SWHEADER));

	//Log::WriteLogA(const_cast<CHAR*>("[DEBUG] [World Change = %04x] [MyID = %08x]"), world_change->_worldID, world_change->_id);
}