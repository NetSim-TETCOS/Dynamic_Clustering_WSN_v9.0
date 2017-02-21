/************************************************************************************
 * Copyright (C) 2013                                                               *
 * TETCOS, Bangalore. India                                                         *
 *                                                                                  *
 * Tetcos owns the intellectual property rights in the Product and its content.     *
 * The copying, redistribution, reselling or publication of any or all of the       *
 * Product or its content without express prior written consent of Tetcos is        *
 * prohibited. Ownership and / or any other right relating to the software and all *
 * intellectual property rights therein shall remain at all times with Tetcos.      *
 *                                                                                  *
 * Author:    Shashi Kant Suman                                                       *
 *                                                                                  *
 * ---------------------------------------------------------------------------------*/
#include "main.h"
#include "802_15_4.h"

long double lf_NN_RandomNo(long double,unsigned long*,unsigned long*);
/** 
	This function is used to calculate the packet error based on the error rate
*/
int fn_NetSim_Zigbee_ErrorPathloss(int nErrorRate,double ldPacketSize,double dEpsilon,NetSim_EVENTDETAILS* pstruEventDetails)
{
	//int nRet = 0;
	double nPacketBits = 0.0;
	double dPacketnoerror ;
	double dPEP;
	double ldRandomNumber;
	double dBER=0;
	double fRand;
	static unsigned long ulSeed1 = 12345678;
	static unsigned long ulSeed2 = 23456789;
	switch (nErrorRate)
	{
	case -1: // No error
		return 0;
	case 0: // Very high error, and therefore the packet will always be errored
		return 1;
	case 1:
		dBER = 10;
		break;
	case 2:
		dBER = 100;
		break;
	case 3:
		dBER = 1000;
		break;
	case 4:
		dBER = 10000;
		break;
	case 5:
		dBER = 100000;
		break;
	case 6:
		dBER = 1000000;
		break;
	case 7:
		dBER = 10000000;
		break;
	case 8:
		dBER = 100000000;
		break;
	case 9:
		dBER = 1000000000;
		break;
	default:
		return 0;
	}
	nPacketBits = ldPacketSize * 8;
	dPacketnoerror = (double) (pow((1-dEpsilon),nPacketBits));
	dPEP = (double) (1-dPacketnoerror);
	ldRandomNumber = (double) lf_NN_RandomNo(dBER,&ulSeed1,&ulSeed2);
	fRand = (double) (ldRandomNumber /(1.0*dBER));
	if (fRand <= dPEP)
	{
		return 1;//	Error Case
	}
	else
	{
		return 0;//	No Error Case
	}
}
/** This function is used to generate the Random Number */
long double lf_NN_RandomNo(long double ldBER,unsigned long* ulnErrorSeed1,unsigned long* ulnErrorSeed2)
{

  long lx = 0;
  long double ldy = 0;
  long double ltemp;

  *ulnErrorSeed1 = (unsigned long) ((40014 * (*ulnErrorSeed1))  % (unsigned long) (2147483563));
  *ulnErrorSeed2 =  (unsigned long) ((40692 * (*ulnErrorSeed2)) % (unsigned long) (2147483399));

  lx = (long) ((*ulnErrorSeed1 - *ulnErrorSeed2) % (long) (2147483562));

  if (lx != 0)
  {
     ldy = (long double) ((long double) (lx) / (long double) (2147483563));
  }
  else
  {
     ldy = (long double) ((long double) (2147483562) / (long double) (2147483563));
  }

  ldy = ldy * 100000000;
  ltemp = (long double) (fmod((double)ldy, (double)(ldBER+1)));
  return ltemp;
}
