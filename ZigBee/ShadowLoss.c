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
# include "main.h"
# include "802_15_4.h"

int fn_NP_RandomNo1(long,double*,unsigned long*,unsigned long*);
/** 
	This function is used to calculate shadow loss.
*/
int fn_NetSim_Zigbee_CalculateShadowLoss(unsigned long* ulSeed1, unsigned long* ulSeed2, double* dShadowReceivedPower,double dStandardDeviation,NetSim_EVENTDETAILS* pstruEventDetails)
{
	// VARAIBLE DECLARATION
	double ldGaussianDeviate=0.0;
	double ldRandomNumber = 0.0;
	static int nIset=0;
	static double fGset=0;
	double fFac,fRsq,fV1,fV2;

	if(nIset==0)
	{
		do
		{
			//TO CALCULATE THE RANDOM NUMBER FROM THIS FUNCTION
			fn_NP_RandomNo1( (long)1,&ldRandomNumber,ulSeed1,ulSeed2);

			//random no to compute shadow loss
			fV1=(double)(2.0*ldRandomNumber-1.0);

			//calculate the Random number from this function
			fn_NP_RandomNo1( (long)1,&ldRandomNumber,ulSeed1,ulSeed2);

			//random no to compute shadow loss
			fV2=(double)(2.0*ldRandomNumber-1.0);

			fRsq=fV1*fV1+fV2*fV2;
		}
		while(fRsq>=1.0 || fRsq==0.0);


		fFac=(double)(sqrt(-2.0*log(fRsq)/fRsq));

		fGset=fV1*fFac;

		nIset=1;

		ldGaussianDeviate = fV2*fFac;
	}

	else
	{
		nIset=0;

		ldGaussianDeviate = fGset;

	}

	// TO ASSIGN THE SHADOW RECEIVED POWER
	*dShadowReceivedPower = ldGaussianDeviate*(double)sqrt(dStandardDeviation);

	return 0;

}
/**
  This function is used to generate random number using linear congruential random number generator
  Ref Book: Discrete Event Simulation by Jerry Banks
 */

int fn_NP_RandomNo1(long lm,double *ldRandNo,unsigned long *uSeed,unsigned long *uSeed1)
{
	long ldTemp;

	double ldy = 0;

	*uSeed = (unsigned long)((40014 * (*uSeed))  % (unsigned long)(2147483563));

	*uSeed1 =  (unsigned long)((40692 * (*uSeed1)) % (unsigned long)(2147483399));

	ldTemp = (long)((*uSeed - *uSeed1) % (long)(2147483562));

	if (ldTemp != 0)
	{
		ldy = (double)((double)(ldTemp) / (double)(2147483563));
	}
	else
	{
		ldy = (double)((double)(2147483562) / (double)(2147483563));
	}

	*ldRandNo = ldy;

	return 0;
}
