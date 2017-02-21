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
#include "802_15_4.h"
/* Function prototype declaration for agent mobility */
int fn_NetSim_NoMobility(int nAgentId,double dSimulationTime,AGENT** pstruAgent);
int fn_NetSim_RandomWalk(int nAgentId,double dSimulationTime,AGENT** pstruAgent);
int fn_NetSim_RandomWayPoint(int nAgentId,double dSimulationTime,AGENT** pstruAgent);
int fn_NetSim_RandomPoint(POS_2D* pstruAgentPos, unsigned long *pulSeed1, unsigned long *pulSeed2,int nVelocity,AGENT** pstruAgent);
int fn_NetSim_MobilityRandomNo(unsigned long *uSeed, unsigned long *uSeed1);
double fn_NetSim_CalculateDistance(POS_2D*,POS_2D*);
int fn_NetSim_FindAgentPos(double*,double*,int,double,AGENT**);
double fn_Sensor_CalculateDistance(POS_2D*,POS_2D*);
/** 
In random-based mobility simulation models, the mobile nodes move randomly and freely
without restrictions. To be more specific, the destination, speed and direction are all chosen
randomly and independently of other nodes. 
 */
int fn_NetSim_Zigbee_AgentMobility(AGENT** pstruAgent,NetSim_EVENTDETAILS* pstruEventDetails)
{
	int nLoop,nAgentMobilityModel;
	srand(1);
	for(nLoop=0;nLoop<MAXAGENT;nLoop++)
	{
		if(!pstruAgent[nLoop])
			continue;
		nAgentMobilityModel = pstruAgent[nLoop]->nMobilityModel; 
		switch(nAgentMobilityModel)
		{
		case NOMOBILITY:
			fn_NetSim_NoMobility(nLoop,NETWORK->pstruSimulationParameter->dVal,pstruAgent);
			break;
		case RANDOMWALK:
			fn_NetSim_RandomWalk(nLoop,NETWORK->pstruSimulationParameter->dVal,pstruAgent);
			break;
		case RANDOMWAYPOINT:
			fn_NetSim_RandomWayPoint(nLoop,NETWORK->pstruSimulationParameter->dVal,pstruAgent);
			break;
		default:
			MessageBoxA(NULL,"Unknown Mobilty Type\nTerminating Application......","WSN_Agent_Mobility",0);
			exit(0);
			break;
		}
	}

	return 0;
}

/** This function is called when there is no mobility. i.e., Velocity = 0.  */
int fn_NetSim_NoMobility(int nAgentId,double dSimulationTime,AGENT** pstruAgent)
{
	int nLoop;
	POS_2D* pstruTempPos;
	for(nLoop = 1;nLoop <=(int)(dSimulationTime/1000000)+1;nLoop++)
	{
		pstruTempPos = (POS_2D*)fnpAllocateMemory(sizeof(POS_2D),1);
		pstruTempPos->dTime_sec = nLoop;
		pstruTempPos->dXPos = pstruAgent[nAgentId]->pstruCurrPos->dXPos;
		pstruTempPos->dYPos = pstruAgent[nAgentId]->pstruCurrPos->dYPos;
		pstruTempPos->pstruNextPos = NULL;
		pstruAgent[nAgentId]->pstruCurrPos->pstruNextPos = pstruTempPos;
		pstruAgent[nAgentId]->pstruCurrPos = pstruAgent[nAgentId]->pstruCurrPos->pstruNextPos;
	}
	pstruAgent[nAgentId]->pstruCurrPos = pstruAgent[nAgentId]->pstruPos;
	return 1;
}
/** 
	Random Walk model involves computing the following at each "mobility event" to find the next point
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	 a. Use the predefined velocity value
	 c. Choose a random direction
	 b. No pause time between mobility events
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	 
*/
int fn_NetSim_RandomWalk(int nAgentId,double dSimulationTime,AGENT** pstruAgent)
{
	int nTimeLoop;
	POS_2D* pstruTempPos;
	unsigned long ulSeed1;
	unsigned long ulSeed2;
	ulSeed1 = 12345678*(nAgentId+1);
	ulSeed2 = 23456789*(nAgentId+1);
	pstruTempPos = pstruAgent[nAgentId]->pstruCurrPos;
	for(nTimeLoop = 1;nTimeLoop<=(int)(dSimulationTime/1000000)+1;nTimeLoop++ )
	{
		if(pstruAgent[nAgentId]->nVelocity <= 0)
		{
			pstruTempPos->pstruNextPos = (POS_2D*)fnpAllocateMemory(sizeof(POS_2D),1);
			pstruTempPos->pstruNextPos->dXPos = pstruTempPos->dXPos;
			pstruTempPos->pstruNextPos->dYPos = pstruTempPos->dYPos;
			pstruTempPos = pstruTempPos->pstruNextPos;
			pstruTempPos->dTime_sec = nTimeLoop;
			continue;
		}
		fn_NetSim_RandomPoint(pstruTempPos,&ulSeed1,&ulSeed2,pstruAgent[nAgentId]->nVelocity,pstruAgent);
		fn_NetSim_Anim_MoveNode(NETWORK->nDeviceCount+(NETSIM_ID)nAgentId+1,pstruTempPos->dXPos,pstruTempPos->dYPos,pstruTempPos->dTime_sec*1000000);
		pstruTempPos = pstruTempPos->pstruNextPos;
		pstruTempPos->dTime_sec = nTimeLoop;
	}
	return 1;
}
/** 
	Random Walk model involves computing the following at each "mobility event" to find the next point
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	 a. Use the predefined value of velocity 
	 c. Choose a random direction
	 b. Use the predefined pause time mobility events
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~	 
*/
int fn_NetSim_RandomWayPoint(int nAgentId,double dSimulationTime,AGENT** pstruAgent)
{
	POS_2D* pstruTempPos;
	unsigned long ulSeed1;
	unsigned long ulSeed2;
	int nTimeLoop;
	ulSeed1 = 12345678*(nAgentId+1);
	ulSeed2 = 23456789*(nAgentId+1);
	pstruTempPos = pstruAgent[nAgentId]->pstruCurrPos;
	for(nTimeLoop = 1;nTimeLoop<=(int)(dSimulationTime/1000000)+1;nTimeLoop++)
	{
		if(pstruAgent[nAgentId]->nPauseTime != 0)
		{
			if(nTimeLoop%(pstruAgent[nAgentId]->nPauseTime+1) != 0)
			{
				pstruTempPos->pstruNextPos = (POS_2D*)fnpAllocateMemory(sizeof(POS_2D),1);
				pstruTempPos->pstruNextPos->dXPos = pstruTempPos->dXPos;
				pstruTempPos->pstruNextPos->dYPos = pstruTempPos->dYPos;
				pstruTempPos = pstruTempPos->pstruNextPos;
				pstruTempPos->dTime_sec = nTimeLoop;
				continue;
			}
		}
		if(pstruAgent[nAgentId]->nVelocity <= 0)
		{
			pstruTempPos->pstruNextPos = (POS_2D*)fnpAllocateMemory(sizeof(POS_2D),1);
			pstruTempPos->pstruNextPos->dXPos = pstruTempPos->dXPos;
			pstruTempPos->pstruNextPos->dYPos = pstruTempPos->dYPos;
			pstruTempPos = pstruTempPos->pstruNextPos;
			pstruTempPos->dTime_sec = nTimeLoop;
			continue;
		}
		fn_NetSim_RandomPoint(pstruTempPos,&ulSeed1,&ulSeed2,pstruAgent[nAgentId]->nVelocity,pstruAgent);
		fn_NetSim_Anim_MoveNode((NETSIM_ID)pstruAgent[nAgentId]->id,pstruTempPos->dXPos,pstruTempPos->dYPos,pstruTempPos->dTime_sec*1000000);
		pstruTempPos = pstruTempPos->pstruNextPos;
		pstruTempPos->dTime_sec = nTimeLoop;
	}
	return 1;
}
/** This function is used to generate a random position for the agent */
int fn_NetSim_RandomPoint(POS_2D* pstruAgentPos, unsigned long *pulSeed1, unsigned long *pulSeed2,int nVelocity,AGENT** pstruAgent)
{
	int min;
	int max;
	int ldRandNo;
	double dDistance;
	int nPos = 1;
	POS_2D *pstruTempPos/*, *pstru_NMo_Error*/;
	pstruTempPos = (POS_2D*)fnpAllocateMemory(sizeof(POS_2D),1);

	while(nPos == 1)
	{
		ldRandNo = fn_NetSim_MobilityRandomNo(pulSeed1,pulSeed2);

		min = (int)(pstruAgentPos->dXPos - ldRandNo % (int)(nVelocity +1));

		ldRandNo = fn_NetSim_MobilityRandomNo(pulSeed1,pulSeed2);

		max = (int)(pstruAgentPos->dXPos + ldRandNo % (int)(nVelocity+1));

		if(min > max)
		{
			pstruTempPos->dXPos = max +(int)((min-max+1)*rand()/(RAND_MAX+1.0));
		}
		else
		{
			pstruTempPos->dXPos = min + (int)((max-min+1)*rand()/(RAND_MAX+1.0));
		}
		ldRandNo = fn_NetSim_MobilityRandomNo(pulSeed1,pulSeed2);

		min = (int)(pstruAgentPos->dXPos - ldRandNo % (int)(nVelocity +1));

		ldRandNo = fn_NetSim_MobilityRandomNo(pulSeed1,pulSeed2);

		max = (int)(pstruAgentPos->dYPos + ldRandNo % (int)(nVelocity+1));

		if(min > max)
		{
			pstruTempPos->dYPos = max +(int)((min-max+1)*rand()/(RAND_MAX+1.0));
		}
		else
		{
			pstruTempPos->dYPos = min + (int)((max-min+1)*rand()/(RAND_MAX+1.0));
		}
		if(pstruTempPos->dXPos < 0 || pstruTempPos->dYPos < 0)
			continue;
		if(pstruTempPos->dXPos > pstruAgent[0]->dWorkingArea_X || pstruTempPos->dYPos >pstruAgent[0]->dWorkingArea_Y)
			continue;
		dDistance = fn_NetSim_CalculateDistance(pstruTempPos,pstruAgentPos);
		if((int)dDistance == nVelocity)
		{
			nPos = 0;
			break;
		}
	}
	pstruAgentPos->pstruNextPos = pstruTempPos;
	return 1;
}
/** This Function is used to generate the Random number based on seed values */
int fn_NetSim_MobilityRandomNo(unsigned long *uSeed, unsigned long *uSeed1)
{
	long ldRandNo;
	double ldy = 0;

	*uSeed	= (unsigned long) ((40014 * (*uSeed))
		% (unsigned long) (2147483563));

	*uSeed1 = (unsigned long) ((40692 * (*uSeed1))
		% (unsigned long) (2147483399));

	ldRandNo = (long) ((*uSeed - *uSeed1) % (long) (2147483562));

	if (ldRandNo != 0)
	{
		ldy = (double) ((double) (ldRandNo) / (double) (2147483563));
	}
	else
	{
		ldy = (double) ((double) (2147483562) / (double) (2147483563));
	}

	ldy = ldy * 100000000;
	ldRandNo = (int) (ldy);
	return ldRandNo;
}
/**  This Function is used to calculate the distance between one sensor to another sensor*/
double fn_NetSim_CalculateDistance(POS_2D* pstruPos1, POS_2D* pstruPos2)
{
	double dDistance;
	if(pstruPos1 == NULL || pstruPos2 == NULL)
		return 0;
	dDistance = (pstruPos1->dXPos - pstruPos2->dXPos)*(pstruPos1->dXPos - pstruPos2->dXPos);
	dDistance += (pstruPos1->dYPos - pstruPos2->dYPos)*(pstruPos1->dYPos - pstruPos2->dYPos);
	dDistance = sqrt(dDistance);
	return dDistance;
}
