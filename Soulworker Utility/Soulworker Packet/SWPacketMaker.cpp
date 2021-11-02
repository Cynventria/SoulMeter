#include "pch.h"
#include ".\Soulworker Packet\PacketType.h"
#include ".\Soulworker Packet\SWPacketMaker.h"


SWPacketMaker::SWPacketMaker() {
	_isSegmentation = FALSE;

	//read ketTable from file
	//key file format:
	//	[USHORT		SWMAGIC]
	//	[int		keyLength]
	//	[BYTE[]		keyTable] 
	//	[char[]		description]
	//	EOF
	char buffer[64];
	errno_t err;
	FILE *keyFile;
	if ((err = fopen_s(&keyFile, KEY_FILE_NAME, "rb")) != 0) {
		sprintf_s(_keyInfo, "Invalid_KeyFile");
	}
	else{

		//check file size
		fseek(keyFile, 0, SEEK_END);
		int fileSize = ftell(keyFile);
		rewind(keyFile);
		
		//read SWMAGIC
		fread(&_SWMAGIC, sizeof(USHORT), 1, keyFile);

		//read key length
		int tmpKeyLen = 0;
		fread(&tmpKeyLen, sizeof(int), 1, keyFile);

		if (tmpKeyLen > 0 && tmpKeyLen < 64) {

			if (fileSize - sizeof(int) - tmpKeyLen < 0) {
				sprintf_s(_keyInfo, "Invalid_KeySize");
				fclose(keyFile);
				return;
			}
			fread(&buffer, sizeof(char), tmpKeyLen, keyFile);
			//read key
			//decrypt if needed
			_keyLength = tmpKeyLen;
			memcpy(_keyTable, buffer, _keyLength);
			memset(buffer, 0, sizeof(buffer));



			fread(&buffer, sizeof(char), (fileSize - sizeof(int) - tmpKeyLen) > 64 ? 64: (fileSize - sizeof(int) - tmpKeyLen), keyFile);
			//read description
			sprintf_s(_keyInfo, "%s", buffer);

		}
		fclose(keyFile);
	}
}

SWPacketMaker::~SWPacketMaker() {

}

SWHEADER* SWPacketMaker::GetSWHeader(IPv4Packet* packet) {

	if (packet == nullptr || packet->_datalength < sizeof(SWHEADER)) {
		return nullptr;
	}
		

	SWHEADER* swheader = (SWHEADER*)(packet->_data);

	if (swheader->_magic != _SWMAGIC || swheader->_const_value01 != SWCONSTVALUE) {
		return nullptr;
	}
		

	return swheader;

}

BYTE* SWPacketMaker::GetSWData(IPv4Packet* packet) {

	return (BYTE*)(packet->_data);
}

VOID SWPacketMaker::Decrypt(BYTE* data, const UINT size, const UINT start) {

	if (data == nullptr || size < 0 || start < 0)
		return;

	for (UINT i = 0; i < size; i++) 
		data[i + start] ^= _keyTable[i % (_keyLength)];  //data[i + start] ^= _keyTable[i % (sizeof(_keyTable) / sizeof(BYTE))];
}

VOID SWPacketMaker::ResizePacket(IPv4Packet* packet) {

	SWHEADER* swheader = GetSWHeader(packet);

#if DEBUG_RESIZEPACKET == 1
	Log::WriteLogA(const_cast<CHAR*>("[ResizePacket Before] [Packet Length = %d] [SWPacket Size = %u]"), packet->_datalength, swheader->_size);
#endif

	if (packet->_datalength >= swheader->_size) {
		packet->_datalength -= swheader->_size;
		packet->_data += swheader->_size;
	}
	else {
		packet->_datalength = 0;
	}

#if DEBUG_RESIZEPACKET == 1
	Log::WriteLogA(const_cast<CHAR*>("[ResizePacket After] [Packet Length = %d] [SWPacket Size = %u]"), packet->_datalength, swheader->_size);
#endif
}

VOID SWPacketMaker::ResizePacket(SIZE_T remainsize, IPv4Packet* packet) {

#if DEBUG_RESIZEPACKET == 1
	Log::WriteLogA(const_cast<CHAR*>("[ResizePacket Before] [Packet Length = %d] [remainSize = %d]"), packet->_datalength, remainsize);
#endif

	if (packet->_datalength >= remainsize) {
		packet->_datalength -= remainsize;
		packet->_data += remainsize;
	}
	else {
		packet->_datalength = 0;
	}

#if DEBUG_RESIZEPACKET == 1
	Log::WriteLogA(const_cast<CHAR*>("[ResizePacket After] [Packet Length = %d] [remainSize = %d]"), packet->_datalength, remainsize);
#endif
}

DWORD SWPacketMaker::CheckSegmentation(IPv4Packet* packet) {
	if (_isSegmentation) {
		return REASSAMBLY;
	}
	else {
		SWHEADER* swheader = GetSWHeader(packet);

		if (swheader == nullptr) {
			return NO_SWHEADER;
		}
			

		if (packet->_datalength < swheader->_size) {
			_isSegmentation = TRUE;
			return YES_SEGMENTATION;
		}
	}

	return NO_SEGMENTATION;
}

VOID SWPacketMaker::StartReassambly(IPv4Packet* packet) {

	SWHEADER* swheader = GetSWHeader(packet);
	BYTE* data = GetSWData(packet);

	if (swheader == nullptr || data == nullptr)
		return;

	_segmentationPacket.Init(swheader->_size);
	_segmentationPacket.AppendData(data, packet->_datalength);

	ResizePacket(packet);
}

VOID SWPacketMaker::Reassambly(IPv4Packet* packet) {

	BYTE* data = GetSWData(packet);

#if DEBUG_SEGMENTATION == 1
	Log::WriteLogA(const_cast<CHAR*>("[Reassambly] [datalength = %d]"), packet->_datalength);
#endif

	SIZE_T remainsize = _segmentationPacket.GetRemainSize();

	DWORD result = SEGMENTATION_SUCCESS;

	if (packet->_datalength >= remainsize) {
		result = _segmentationPacket.AppendData(data, remainsize);
	}
	else if (packet->_datalength < remainsize) {
		result = _segmentationPacket.AppendData(data, packet->_datalength);
	}

	ResizePacket(remainsize, packet);

	switch (result) {
	case SEGMENTATION_SUCCESS: {
#if DEBUG_SEGMENTATION == 1
		IPv4Packet* test = _segmentationPacket.GetData();
		Log::WriteLogA(const_cast<CHAR*>("[Reassambly Success] [datalength = %d]"), test->_datalength);

		for (int i = 0; i < test->_datalength; i++)
			printf("%02x ", test->_data[i]);
		printf("\n");
#endif
		CreateSWPacket(_segmentationPacket.GetData());
		_isSegmentation = FALSE;
	}
		break;
	case SEGMENTATION_FAILED:
		break;
	}
}

VOID SWPacketMaker::CreateSWPacket(IPv4Packet* packet) {

	SWHEADER* swheader = GetSWHeader(packet);
	BYTE* data = GetSWData(packet);

	if (swheader == nullptr || data == nullptr)
		return;

	Decrypt(data, swheader->_size - sizeof(SWHEADER) + 2, sizeof(SWHEADER) - 2);

	SWPacket* swpacket = nullptr;
	DAMAGEMETER.GetLock();
	{
		switch (_byteswap_ushort(swheader->_op)) {

			/* 0x01*/
		case OPcode::HEARTBEAT:
			//swpacket = new SWPacketHeartbeat(swheader, data);
			break;

			/*0x03*/
		case OPcode::STATCHANGE:
			swpacket = new SWPacketStatChange(swheader, data);
			break;
		case OPcode::DEAD:
			swpacket = new SWPacketDead(swheader, data);
			break;

			/*0x04*/
		case OPcode::WORLDCHANGE:
			swpacket = new SWPacketWorldChange(swheader, data);
			break;
		case OPcode::MAZESTART:
			swpacket = new SWPacketMazeStart(swheader, data);
			break;
		case OPcode::SPAWNED_CHARINFO:
			swpacket = new SWPacketSpawnedCharInfo(swheader, data);
			break;
		case OPcode::OBJECTCREATE:
			swpacket = new SWPacketObjectCreate(swheader, data);
			break;

			/*0x05*/
		case OPcode::STARTMOVE:
			swpacket = new SWPacketStartMove(swheader, data);
			break;
		case OPcode::STOPMOVE:
			break;
		case OPcode::JUMP:
			break;
		case OPcode::CANCEL_WITHMOVE:
			break;

			/*0x06 Combat*/
		//case OPcode::EVADE:
		//	break;
		case OPcode::USESKILL:
			swpacket = new SWPacketUseSkill(swheader, data);
			break;
		case OPcode::OTHER_USESKILL:
			swpacket = new SWPacketOtherUseSkill(swheader, data);
			break;
		case OPcode::DAMAGE:
			swpacket = new SWPacketDamage(swheader, data);
			break;
		case OPcode::BUFFIN:
			swpacket = new SWPacketBuffIn(swheader, data);
			break;
		case OPcode::BUFFOUT:
			swpacket = new SWPacketBuffOut(swheader, data);
			break;
		case OPcode::PROJECTILE:
			break;
		case OPcode::AKASIC:
			swpacket = new SWPacketAkasic(swheader, data);
			break;
		case OPcode::COOLDOWN:
			swpacket = new SWPacketCooldown(swheader, data);
			break;

			/*0x07*/
		case OPcode::CHAT:
			swpacket = new SWPacketChat(swheader, data);
			break;

			/* 0x11 */

		case OPcode::MAZEEND:
			swpacket = new SWPacketMazeEnd(swheader, data);
			break;

			/* 0x12 Party */
		case OPcode::PARTY:
			swpacket = new SWPacketParty(swheader, data);
			break;
		case OPcode::PARTY_LIST_INFO:
			//swpacket = new SWPacketPartyListInfo(swheader, data);
			break;

			/* 0x17 ?? */
		case OPcode::MONSTER_KILLED:
			swpacket = new SWPacketMonsterKilled(swheader, data);
			break;
		case OPcode::AGGRO_CHANGED:
			swpacket = new SWPacketAggroChanged(swheader, data);
			break;

			/* 0x23 Gesture*/
		case OPcode::GESTURE_USED:
			//swpacket = new SWPacketGestureUsed(swheader, data);
			break;

			/* 0x2e Force*/
		case OPcode::POS:
			swpacket = new SWPacketPos(swheader, data);
			break;


		default:
			//swpacket = new SWPacket(swheader, data);
			break;
		}

		if (swpacket != nullptr) {
#if DEBUG_CREATEPACKET == 1 // && defined(_DEBUG)
			swpacket->Debug();
#endif
			// Todo
			swpacket->Do();

			delete swpacket;
		}
	}
	DAMAGEMETER.FreeLock();

	ResizePacket(packet);
}

VOID SWPacketMaker::CheckRemainPacket(IPv4Packet* packet) {
	
	if (packet->_datalength > 0)
		Parse(packet);
}

DWORD SWPacketMaker::Parse(IPv4Packet* packet) {

	if (packet == nullptr)
		return ERROR_INVALID_PARAMETER;

	switch (CheckSegmentation(packet)) {
	case NO_SEGMENTATION:
		CreateSWPacket(packet);
		break;
	case YES_SEGMENTATION:
		StartReassambly(packet);
		break;
	case REASSAMBLY:
		Reassambly(packet);
		break;
	default:
		return ERROR_FUNCTION_FAILED;
	}

	CheckRemainPacket(packet);

	return ERROR_SUCCESS;
}

char* SWPacketMaker::GetKeyInfo() {
	return _keyInfo;

}