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
/**
   This function invokes the received power calculation
   and selects the path loss model based on the channel characteristics
 
*/
int fn_NetSim_Zigbee_CalculateReceivedPower(IEEE802_15_4_PHY_VAR *pstruPhyVar, double dDistance,double* dReceivedPower, NETSIM_ID nLinkID,NetSim_EVENTDETAILS* pstruEventDetails)
{

	double fpi=3.1417f; // TO GET THE PI VALUE
	double dGainTxr=0;		// TO GET THE TRANSMITTER GAIN
	double dGainRver=0;	// TO GET THE RECEIVER GAIN
	int nDefaultDistance=1; // TO GET THE DEFULT DISTANCE
	double fA1,fWaveLength=0.0; // TO GET THE WAVELENGTH VALUE
	double fA1dB, fA2dB;
	double fTXPowerdBm; // TO GET THE TRANSMITTER PWOER IN dbm
	double dDefaultExponent;
	double dTransmitterPower_mw;
	
	/* TO CHECK THE CHANNEL CHARACTERISTICS
	 * 0 AS NO PATH LOSS
	 * 1 AS LINE OF SIGHT
	 * 2 AS FADING ONLY
	 * 3 AS FADING AND SHADOWING
	 */
	dTransmitterPower_mw = pstruPhyVar->dTransmitterPower_mw;
//check this path loss
	if(NETWORK->ppstruNetSimLinks[nLinkID-1]->puniMedProp.pstruWirelessLink.dPathLossExponent != 0
		&& NETWORK->ppstruNetSimLinks[nLinkID-1]->puniMedProp.pstruWirelessLink.dFadingFigure == 0)
	{
		if(dDistance < 8)
		{
			nDefaultDistance = 1;
			dDefaultExponent = 2;
		}
		else
		{
			nDefaultDistance = 8;
			dDefaultExponent = 3.3;
		}
		//TO CALCULATE LEMDA
		fWaveLength=(double)(300.0/(pstruPhyVar->dFrequencyBand_MHz*1.0));//pathloss

		// TO CALCULATE (4*3.14*do)
		fA1=fWaveLength/(4*(double) fpi * nDefaultDistance);

		// TO CALCULATE 20log10[ lemda/(4*3.14*do) ]
		fA1dB =  10 * dDefaultExponent * log10(1.0/fA1);

		// TO CALCULATE 10 * n *log10 (d/do)
		fA2dB =  10 * NETWORK->ppstruNetSimLinks[nLinkID-1]->puniMedProp.pstruWirelessLink.dPathLossExponent * log10(dDistance/nDefaultDistance);

		fTXPowerdBm = 10 * log10(dTransmitterPower_mw);

		//TO CALCULATE [Pt]  +  [Gt]  +  [Gr]  +  20log10[ Lemda/(4*3.14*do) ] + ( 10 * n *log10 (do/d) )
		*dReceivedPower = fTXPowerdBm + dGainTxr + dGainRver - fA1dB - fA2dB;

	}
	else
	{
		if(dDistance < 8)
		{
			nDefaultDistance = 1;
			dDefaultExponent = 2;
		}
		else
		{
			nDefaultDistance = 8;
			dDefaultExponent = 3.3;
		}
		// TO GET THE GAIN OF TRANSMITTER
		dGainTxr=5.5;

		// TO GET THE GAIN OF RECEIVER
		dGainRver=5.5;

		// TO CALCULATE THE WAVELENGTH
		fWaveLength=(double)(300.0/(pstruPhyVar->dFrequencyBand_MHz * 1.0));//pathloss

		fA1=fWaveLength/(4*(double) fpi * nDefaultDistance );

		//20 * log10(fA1);
		fA1dB =  10 * dDefaultExponent * log10(1.0/fA1);
		fA2dB =  10 * NETWORK->ppstruNetSimLinks[nLinkID-1]->\
			puniMedProp.pstruWirelessLink.dPathLossExponent * \
			log10(dDistance/nDefaultDistance);
		fTXPowerdBm = 10 * log10(dTransmitterPower_mw);

		// TO CALCULATE THE RECEIVED POWER
		*dReceivedPower = fTXPowerdBm + dGainTxr + dGainRver - fA1dB - fA2dB;
	}
	return 0;

}
