/************************************************************************************
* Copyright (C) 2013                                                               *
* TETCOS, Bangalore. India                                                         *
*                                                                                  *
* Tetcos owns the intellectual property rights in the Product and its content.     *
* The copying, redistribution, reselling or publication of any or all of the       *
* Product or its content without express prior written consent of Tetcos is        *
* prohibited. Ownership and / or any other right relating to the software and all  *
* intellectual property rights therein shall remain at all times with Tetcos.      *
*                                                                                  *
* Author:    Thangarasu.K                                                     *
*                                                                                  *
* ---------------------------------------------------------------------------------*/
#include "main.h"
#include "List.h"
#include "802_15_4.h"
int fn_NetSim_FindAgentPos(double* dXPos, double* dYPos, int nAgentId,double dSensingTime,AGENT** pstruAgent);
double fn_Sensor_CalculateDistance(POS_2D* pstruPos1, POS_2D* pstruPos2);

/** In this function the sensors sense the agent, creates a packet and forwards it to sink node.*/
int fn_NetSim_Zigbee_SensorEvent(int nSensorLoop,NETSIM_ID nGlobalPANCoordinatorId,AGENT** pstruAgent,SENSORS* pstru_Sensor,METRICS** pstruMetrics,NetSim_EVENTDETAILS* pstruEventDetails)
{
	int nFlag = 0;
	static int nPacketId;
	char str[500];
	int nAgentLoop;
	POS_2D* pstruPos;
	double dDistance;
	POS_2D* pstruTemppos;
	NetSim_PACKET *PstruPacket;
	pstruPos = (POS_2D*)fnpAllocateMemory(sizeof(POS_2D),1);
	pstruTemppos = (POS_2D*)fnpAllocateMemory(sizeof(POS_2D),1);
	for(nAgentLoop =0;nAgentLoop<MAXAGENT;nAgentLoop++)
	{
		if(pstruAgent[nAgentLoop] == NULL)
			continue;
		fn_NetSim_FindAgentPos(&(pstruPos->dXPos),&(pstruPos->dYPos),nAgentLoop,pstruEventDetails->dEventTime,pstruAgent);
		pstruTemppos->dXPos = NETWORK->ppstruDeviceList[nSensorLoop-1]->pstruDevicePosition->X;
		pstruTemppos->dYPos = NETWORK->ppstruDeviceList[nSensorLoop-1]->pstruDevicePosition->Y;
		dDistance = fn_Sensor_CalculateDistance(pstruTemppos,pstruPos);
		if(dDistance <= pstru_Sensor->nSensorRange)
		{
			AGENTLOG* agentlog = AGENTLOG_ALLOC();
			agentlog->dTime=pstruEventDetails->dEventTime;
			agentlog->dX = pstruPos->dXPos;
			agentlog->dY = pstruPos->dYPos;
			agentlog->nAgentId = pstruAgent[nAgentLoop]->nAgentID; 
			LIST_ADD_LAST((void**)&(pstruMetrics[nSensorLoop-1]->pstruSensorMetrics->pstruAgentLog),agentlog);
			pstruMetrics[nSensorLoop-1]->pstruSensorMetrics->nAgentSensedCount++;
			pstruMetrics[nSensorLoop-1]->pstruSensorMetrics->nPacketGenerated++;

			//Add application out to tramit the position
			//Generate the packet
			PstruPacket = fn_NetSim_Packet_CreatePacket(5);
			PstruPacket->dEventTime = pstruEventDetails->dEventTime;
			PstruPacket->nDestinationId = nGlobalPANCoordinatorId;
			PstruPacket->nPacketId = nPacketId++;
			PstruPacket->nPacketStatus = PacketStatus_NoError;
			PstruPacket->nPacketType = PacketType_Custom;
			PstruPacket->nPacketPriority = Priority_Low;
			PstruPacket->nQOS =QOS_BE;
			PstruPacket->nSourceId = (NETSIM_ID)nSensorLoop;
			//Update the Transport layer information
			PstruPacket->pstruTransportData->nSourcePort = SOURCEPORT;
			PstruPacket->pstruTransportData->nDestinationPort = DESTINATIONPORT;
			//Update the Network layer information
			PstruPacket->pstruNetworkData->szSourceIP = IP_COPY(fn_NetSim_Stack_GetFirstIPAddressAsId((NETSIM_ID)nSensorLoop,0));
			PstruPacket->pstruNetworkData->szDestIP = IP_COPY(fn_NetSim_Stack_GetFirstIPAddressAsId(PstruPacket->nDestinationId,0));
			PstruPacket->pstruNetworkData->nTTL = MAX_TTL;
			//Update the Application layer information
			sprintf(str,"%d- (%lf,%lf)",pstruAgent[nAgentLoop]->nAgentID,pstruPos->dXPos,pstruPos->dYPos);
			PstruPacket->pstruAppData->dPayload = (double)(sizeof(char)*strlen(str)); 
			PstruPacket->pstruAppData->dOverhead = 0;
			PstruPacket->pstruAppData->dPacketSize = PstruPacket->pstruAppData->dPayload + PstruPacket->pstruAppData->dOverhead;						
			PstruPacket->pstruAppData->dArrivalTime = pstruEventDetails->dEventTime;
			PstruPacket->pstruAppData->dEndTime = pstruEventDetails->dEventTime;
			PstruPacket->pstruAppData->dStartTime =  pstruEventDetails->dEventTime;

			if(NETWORK->ppstruDeviceList[nSensorLoop-1]->pstruTransportLayer->isUDP)
				PstruPacket->pstruTransportData->nTransportProtocol=TX_PROTOCOL_UDP;
			else if(NETWORK->ppstruDeviceList[nSensorLoop-1]->pstruTransportLayer->isTCP)
				PstruPacket->pstruTransportData->nTransportProtocol=TX_PROTOCOL_TCP;
			else
				PstruPacket->pstruTransportData->nTransportProtocol=0;
			if(NETWORK->ppstruDeviceList[nSensorLoop-1]->pstruSocketInterface->pstruSocketBuffer[0]->pstruPacketlist==NULL)
			{
				fn_NetSim_Packet_AddPacketToList((NETWORK->ppstruDeviceList[nSensorLoop-1]->pstruSocketInterface->pstruSocketBuffer[0]),PstruPacket,3);
				pstruEventDetails->dPacketSize=PstruPacket->pstruAppData->dPacketSize;
				pstruEventDetails->nDeviceType = SENSOR;
				pstruEventDetails->nApplicationId=0;
				pstruEventDetails->nProtocolId=PstruPacket->pstruTransportData->nTransportProtocol;
				pstruEventDetails->nDeviceId=(NETSIM_ID)nSensorLoop;
				pstruEventDetails->nInterfaceId=0;
				pstruEventDetails->nEventType=TRANSPORT_OUT_EVENT;
				pstruEventDetails->nSubEventType=0;
				pstruEventDetails->pPacket=NULL;
				fnpAddEvent(pstruEventDetails);	
			}
			else
			{
				fn_NetSim_Packet_AddPacketToList((NETWORK->ppstruDeviceList[nSensorLoop-1]->pstruSocketInterface->pstruSocketBuffer[0]),PstruPacket,3);
			}	
		}
	}
	pstruEventDetails->nDeviceId = (NETSIM_ID)nSensorLoop;
	pstruEventDetails->dEventTime = pstruEventDetails->dEventTime + pstru_Sensor->dSensingInterval;
	pstruEventDetails->nEventType = TIMER_EVENT;
	pstruEventDetails->nProtocolId = MAC_PROTOCOL_IEEE802_15_4;
	pstruEventDetails->nSubEventType = SENSOR_EVENT;
	fnpAddEvent(pstruEventDetails);

	fnpFreeMemory(pstruPos);
	fnpFreeMemory(pstruTemppos);
	return nFlag;
}
/** This function is used to find the Agent's position with respect to the sensing time.*/
int fn_NetSim_FindAgentPos(double* dXPos, double* dYPos, int nAgentId,double dSensingTime,AGENT** pstruAgent)
{
	int nTime;
	double dTime;
	double dTheta;
	double dVx;
	double dVy;
	nTime = (int)(dSensingTime/1000000);
	dTime = (int)dSensingTime%1000000;
	while(pstruAgent[nAgentId]->pstruCurrPos->dTime_sec != nTime)
	{
		pstruAgent[nAgentId]->pstruCurrPos = pstruAgent[nAgentId]->pstruCurrPos->pstruNextPos;
	}
	if(pstruAgent[nAgentId]->pstruCurrPos->dXPos - pstruAgent[nAgentId]->pstruCurrPos->pstruNextPos->dXPos != 0)
	{
		if(pstruAgent[nAgentId]->pstruCurrPos->pstruNextPos != NULL)
			dTheta = atan((pstruAgent[nAgentId]->pstruCurrPos->pstruNextPos->dYPos - pstruAgent[nAgentId]->pstruCurrPos->dYPos)/
			(pstruAgent[nAgentId]->pstruCurrPos->pstruNextPos->dXPos - pstruAgent[nAgentId]->pstruCurrPos->dXPos));
		else
			dTheta =0;
	}
	else
	{
		dTheta = 3.14/2;
	}
	dVx = pstruAgent[nAgentId]->nVelocity*cos(dTheta);
	dVy = pstruAgent[nAgentId]->nVelocity*sin(dTheta);
	*dXPos = pstruAgent[nAgentId]->pstruCurrPos->dXPos + dVx*(dTime/1000000);
	*dYPos = pstruAgent[nAgentId]->pstruCurrPos->dYPos +dVy*(dTime/1000000);
	return 1;
}
/** This function is used to find the distance between the Agent and the Sensor. */
double fn_Sensor_CalculateDistance(POS_2D* pstruPos1, POS_2D* pstruPos2)
{
	double dDistance;
	if(pstruPos1 == NULL || pstruPos2 == NULL)
		return 0;
	dDistance = (pstruPos1->dXPos - pstruPos2->dXPos)*(pstruPos1->dXPos - pstruPos2->dXPos);
	dDistance += (pstruPos1->dYPos - pstruPos2->dYPos)*(pstruPos1->dYPos - pstruPos2->dYPos);
	dDistance = sqrt(dDistance);
	return dDistance;
}
