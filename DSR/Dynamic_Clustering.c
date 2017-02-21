/************************************************************************************
 * Copyright (C) 2016                                                               *
 * TETCOS, Bangalore. India                                                         *
 *                                                                                  *
 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 *                                                                                  *
 * Author:    Kanagaraj K                                                      *
 *                                                                                  *
 * ---------------------------------------------------------------------------------*/

/**********************************IMPORTANT NOTES***********************************
1. This file contains the Dynamic Clustering code. 
   To enable Dynamic Clustering, uncomment the line "#define _DYNAMIC_CLUSTERING" in DSR.h
2. For this implementation of Dynamic Clustering, the number of Clusters is set in NUMBEROFCLUSTERS
   varaiable. The user can modify it as per the number of clusters required.
3. The Network scenario can contain any number of sensors which will be divided into number of 
   clusters as specified in the NUMBEROFCLUSTERS variable. The size of each cluster and the 
   sensors in each cluster varies dynamically after every CLUSTER_INTERVAL time. This can 
   be modified in the DSR.h file.
4. Mobility can be set to sensors by setting velocity which is zero by default
************************************************************************************/


#include "main.h"
#include "DSR.h"
#include "List.h"
#include "802_15_4.h"
#define NUMBEROFCLUSTERS 4

int **ClusterElements;
int CH[NUMBEROFCLUSTERS];
int CL_SIZE[NUMBEROFCLUSTERS];
int sensor_count;


int fn_NetSim_dynamic_clustering_CheckDestination(NETSIM_ID nDeviceId, NETSIM_ID nDestinationId)
//Function to check whether the Device ID is same as the Destination ID
{
	if(nDeviceId == nDestinationId)
		return 1;
	else
		return 0;
}

int fn_NetSim_dynamic_clustering_GetNextHop(NetSim_EVENTDETAILS* pstruEventDetails)
//Function to determine the DeviceId of the next hop
{
	int nextHop=0;
	NETSIM_ID nInterface;

	
	int i;
	int ClusterId;
	int cl_flag=0;
	
	int cls=fn_NetSim_dynamic_clustering_IdentifyCluster(pstruEventDetails->nDeviceId);
	
	//Static Routes defined for 4 Clusters.
	//If the sensor is the Cluster Head, it forwards it to the Sink.
	//Otherwise, it forwards the packet to the Cluster Head of its cluster.
	
	if(pstruEventDetails->pPacket->nSourceId == pstruEventDetails->nDeviceId)
	//For the first hop
	{
		
		if(pstruEventDetails->nDeviceId == CH[cls])
		   nextHop = pstruEventDetails->pPacket->nDestinationId;
		else
		{
			ClusterId = fn_NetSim_dynamic_clustering_IdentifyCluster(pstruEventDetails->nDeviceId);
			nextHop = CH[ClusterId];
		}
	}
	else
    {
		
			nextHop = pstruEventDetails->pPacket->nDestinationId;
	}

	
	//Updating the Transmitter ID, Receiver ID and NextHopIP in the pstruEventDetails
	free(pstruEventDetails->pPacket->pstruNetworkData->szNextHopIp);
	pstruEventDetails->pPacket->pstruNetworkData->szNextHopIp = DSR_DEV_IP_COPY(nextHop);
	pstruEventDetails->pPacket->nTransmitterId = pstruEventDetails->nDeviceId;
	pstruEventDetails->pPacket->nReceiverId = nextHop;
	

	return 1;
}

int fn_NetSim_dynamic_clustering_IdentifyCluster(int DeviceId)
//Function to identify the cluster of the sensor.
{
	int i,j;
	for(i=0; i<NUMBEROFCLUSTERS; i++)
	{
		
		for(j=0; j<CL_SIZE[i]; j++)
		{
			if(DeviceId == ClusterElements[i][j])
			return i;
		}
	}
}

int fn_NetSim_dynamic_clustering_run()
{
	double **arr;
	double **DEV_POS;
	char *pos;
	int i=0,j=0;
	sensor_count=NETWORK->nDeviceCount-1;

	arr = (double**)calloc(3,sizeof(double));// To store the values obtained from MATLAB
	pos = (char*)calloc(sensor_count*15,sizeof(char));// To form a matrix containing the coordinates of all the sensros.
	
	ClusterElements=(int**)calloc(NUMBEROFCLUSTERS, sizeof(*ClusterElements));
	
	
	DEV_POS=(double**)calloc(sensor_count, sizeof(*DEV_POS));
	for(i=0;i<sensor_count;i++)
		DEV_POS[i]=(double*)calloc(2,sizeof*(DEV_POS[i]));

	
	for(i=0;i<sensor_count;i++)
	{
		DEV_POS[i][0]=NETWORK->ppstruDeviceList[i]->pstruDevicePosition->X;
		DEV_POS[i][1]=NETWORK->ppstruDeviceList[i]->pstruDevicePosition->Y;
	}

	
	sprintf(pos,"x=[");
	for(i=0;i<sensor_count;i++)
	{
		sprintf(pos,"%s%.2f %.2f",pos,DEV_POS[i][0],DEV_POS[i][1]);
		if(i!=(sensor_count-1))
			sprintf(pos,"%s;",pos);
	}
	sprintf(pos,"%s]",pos);
	fprintf(stderr,"\nPress Enter to continue...\n",pos);
	
	arr[0]=(double*)calloc(NUMBEROFCLUSTERS,sizeof*(arr[0]));//To store the cluster head id's of each cluster
	arr[1]=(double*)calloc(sensor_count-1,sizeof*(arr[1]));//To store the cluster id of each sensor
	arr[2]=(double*)calloc(sensor_count-1,sizeof*(arr[2]));//To store size of each cluster
	
	//call to Function in MATLAB_Interface.c file
	arr=fn_netsim_matlab_run(pos,sensor_count,NUMBEROFCLUSTERS);


	for(i=0;i<NUMBEROFCLUSTERS;i++)
	ClusterElements[i]=(int*)calloc(arr[2][i],sizeof*(ClusterElements[i]));
	
	
	fn_netsim_dynamic_form_clusters(arr[1],arr[2]);//cluster id's and size of each cluster are passed as parameters

	fn_netsim_assign_cluster_heads(arr[0]);//cluster head id's are passed as parameters


	for(i=0;i<(sizeof(arr)/sizeof(double));i++)
		fprintf(stderr,"\n%f",arr[i]);

	getch();
		

}

int fn_netsim_dynamic_form_clusters(double* cl_rand,double* c_size)
//sensors are assigned to respective clusters using the cluster id's obtained from MATLAB
{
	int i=0,j=0,tmp=0;
	sensor_count=NETWORK->nDeviceCount-1;
	
	for(i=0;i<NUMBEROFCLUSTERS;i++)
	{
		CL_SIZE[i]=c_size[i];

		tmp=0;
		for(j=0;j<sensor_count;j++)
		{
			if(cl_rand[j]==i+1)
			{
				ClusterElements[i][tmp]=j+1;
				tmp++;
			}

		}
	}
	
	return 1;

}

int fn_netsim_assign_cluster_heads(double* cl_head)
//Cluster heads are assigned to respective clusters using the data obtained from MATLAB
{
	int i=0;
	for(i=0;i<NUMBEROFCLUSTERS;i++)
		CH[i]=cl_head[i];
	return 1;
}