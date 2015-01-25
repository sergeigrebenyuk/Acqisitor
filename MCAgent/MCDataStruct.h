
#pragma once
#include "MultiClampBroadcastMsg.hpp"
#include "AxMultiClampMsg.h"
#define STAT_ONLINE			1
#define STAT_OFFLINE		0
#define STAT_PENDING		-1

#define STAGE_NO_COMMAND	0
#define STAGE_CONNECT		1
							
#define MC_NO_COMMAND		0
#define MC_EXIT				1


#define MAX_STAGE 8
typedef struct
{
	MC_TELEGRAPH_DATA	MCPacket;
	UINT				SN;
	UINT				StageID;
	char				Status;
	char				stage_cmd;
	char				Connected;
} 
STAGE_UNIT;

typedef struct
{
	char ActiveStages;
	char MCOnline;
	char MCAgentOnline;
	char ShutDown;
	char mc_cmd;
	STAGE_UNIT * stage_arr;
	UINT ReqPeriod;
} 
MC_COMMAND;
