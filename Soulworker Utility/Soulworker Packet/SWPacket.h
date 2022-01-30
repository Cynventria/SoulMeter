#pragma once
#include ".\Language\Region.h"


#ifdef SERVER_KOREA
#define SWMAGIC 3
#endif
#ifdef SERVER_STEAM
#define SWMAGIC 3
#endif
#ifdef SERVER_JAPAN
#define SWMAGIC 2
#endif

//SWMAGIC WILL BE SET BY KEY FILE

#define SWCONSTVALUE 1

#pragma pack(push, 1)

typedef struct _SWHEADER {
	USHORT _magic;
	USHORT _size;
	UCHAR _const_value01;
	USHORT _op;
} SWHEADER;

#pragma pack(pop)

class SWPacket : public MemoryPool<SWPacket, 10> {
protected:
	SWHEADER* _swheader;
	BYTE* _data;

	SWPacket() {}
	SWPacket(SWPacket& other) {}

public:
	SWPacket(SWHEADER* swheader, BYTE* data);
	virtual ~SWPacket() {}

	virtual VOID Debug();
	virtual VOID Log();
	virtual VOID Do();
};