/***************************************************************************
 *   Copyright (C) 2011-2015 by Maxim Kochetkov                            *
 *   fido_max@inbox.ru                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef DA_GKR_H
#define DA_GKR_H

#include "da.h"
#define c_wp 11 //lenght chanell with params
#define c_p 7   //lenght chanell without params
namespace FT3
{
    class B_GKR: public DA
    {
    public:
	//Methods
	B_GKR(TMdPrm& prm, uint16_t id, uint16_t nkr, uint16_t nr, bool has_params);
	~B_GKR();
	uint8_t  state;
	uint16_t ID;
	uint16_t count_kr;
	bool with_params;
	uint16_t Task(uint16_t);
	uint16_t HandleEvent(uint8_t *);
	uint8_t cmdGet(uint16_t prmID, uint8_t * out);
	uint8_t cmdSet(uint8_t * req, uint8_t addr);
	uint16_t setVal(TVal &val);
	//void setTU(uint8_t);
	//void runTU(uint8_t);
	string getStatus(void);
	void saveIO(void);
	void loadIO(bool force = false );
	void tmHandler(void);

	class SKRchannel
	{
	public:
		SKRchannel(uint8_t iid):
			id(iid),
			/*params 1*/State(TSYS::strMess("state_%d", id + 1).c_str(), TSYS::strMess(_("State %d"), id + 1).c_str()),
			/*params 2*/On(TSYS::strMess("on_%d", id + 1).c_str(), TSYS::strMess(_("On %d"), id + 1).c_str()),
			/*params 3*/Off(TSYS::strMess("off_%d", id + 1).c_str(), TSYS::strMess(_("Off %d"), id + 1).c_str()),
			/*params 4*/Run(TSYS::strMess("run_%d", id + 1).c_str(), TSYS::strMess(_("Run %d"), id + 1).c_str()),
			/*params 5*/Reset(TSYS::strMess("reset_%d", id + 1).c_str(), TSYS::strMess(_("Reset %d"), id + 1).c_str()),
			/*params 6*/Ban_MC(TSYS::strMess("run_%d", id + 1).c_str(), TSYS::strMess(_("Ban_MC %d"), id + 1).c_str()),
			/*params 7*/Lubrication(TSYS::strMess("reset_%d", id + 1).c_str(), TSYS::strMess(_("Lubrication %d"), id + 1).c_str()),
			
			/*params 8*/Time(TSYS::strMess("time_%d", id + 1).c_str(), TSYS::strMess(_("Time %d"), id + 1).c_str()),
			/*params 9*/ExTime(TSYS::strMess("extime_%d", id + 1).c_str(), TSYS::strMess(_("ExTime %d"), id + 1).c_str()),
			/*params 10*/Time_Lub(TSYS::strMess("time_%d", id + 1).c_str(), TSYS::strMess(_("Time_Lub %d"), id + 1).c_str()),
			/*params 11*/Timeout_PO(TSYS::strMess("extime_%d", id + 1).c_str(), TSYS::strMess(_("Timeout_PO %d"), id + 1).c_str())
		{
		}
		uint8_t id;
		uint8_t s_currTU;
		uint8_t currTU;
		uint8_t s_run;
		uint8_t run;
		
		ui8Data State, On, Off, Run, Reset, Ban_MC, Lubrication;
		flData  Time, ExTime, Time_Lub, Timeout_PO;
	};
	vector<SKRchannel> KRdata;
	
	int lnkSize()
	{
	    if(with_params) {
		return KRdata.size() * c_wp + TRdata.size();//params SKRchannel
	    } else {
		return KRdata.size() * c_p + TRdata.size();//params SKRchannel
	    }
	}
	int lnkId(const string &id)
	{
	    if(with_params) {
		for(int i_l = 0; i_l < KRdata.size(); i_l++) {
			if(KRdata[i_l].State.lnk.prmName == id) return i_l * c_wp;
			if(KRdata[i_l].On.lnk.prmName == id) return i_l * c_wp + 1;
			if(KRdata[i_l].Off.lnk.prmName == id) return i_l * c_wp + 2;
			if(KRdata[i_l].Run.lnk.prmName == id) return i_l * c_wp + 3;
			if(KRdata[i_l].Reset.lnk.prmName == id) return i_l * c_wp + 4;
			if(KRdata[i_l].Ban_MC.lnk.prmName == id) return i_l * c_wp + 5;
			if(KRdata[i_l].Lubrication.lnk.prmName == id) return i_l * c_wp + 6;
			
			if(KRdata[i_l].Time.lnk.prmName == id) return i_l * c_wp + 7;
			if(KRdata[i_l].ExTime.lnk.prmName == id) return i_l * c_wp + 8;
			if(KRdata[i_l].Time_Lub.lnk.prmName == id) return i_l * c_wp + 9;
			if(KRdata[i_l].Timeout_PO.lnk.prmName == id) return i_l * c_wp + 10;
		}
	    } else {
		for(int i_l = 0; i_l < KRdata.size(); i_l++) {
			if(KRdata[i_l].State.lnk.prmName == id) return i_l * c_p;
			if(KRdata[i_l].On.lnk.prmName == id) return i_l * c_p + 1;
			if(KRdata[i_l].Off.lnk.prmName == id) return i_l * c_p + 2;
			if(KRdata[i_l].Run.lnk.prmName == id) return i_l * c_p + 3;
			if(KRdata[i_l].Reset.lnk.prmName == id) return i_l * c_p + 4;
			if(KRdata[i_l].Ban_MC.lnk.prmName == id) return i_l * c_p + 5;
			if(KRdata[i_l].Lubrication.lnk.prmName == id) return i_l * c_p + 6;
		}
	    }


	    return -1;
	}
	SLnk &lnk(int num)
	{
	    SLnk ret = NULL;
	    if(with_params) {
		if((KRdata.size() > 0) && ((KRdata.size() * c_wp) > num)) {
		    switch(num % c_wp) {
		    case 0:
			ret = KRdata[num / c_wp].State.lnk;
			break;
		    case 1:
			ret = KRdata[num / c_wp].On.lnk;
			break;
		    case 2:
			ret = KRdata[num / c_wp].Off.lnk;
			break;
		    case 3:
			ret = KRdata[num / c_wp].Run.lnk;
			break;
		    case 4:
			ret = KRdata[num / c_wp].Reset.lnk;
			break;
		    case 5:
			ret = KRdata[num / c_wp].Ban_MC.lnk;
			break;
		    case 6:
			ret = KRdata[num / c_wp].Lubrication.lnk;
			break;
		    case 7:
			ret = KRdata[num / c_wp].Time.lnk;
			break;
		    case 8:
			ret = KRdata[num / c_wp].ExTime.lnk;
			break;
		    case 9:
			ret = KRdata[num / c_wp].Time_Lub.lnk;
			break;
		    case 10:
			ret = KRdata[num / c_wp].Timeout_PO.lnk;
			break;
		    }
		}
	    } else {
		if((KRdata.size() > 0) && ((KRdata.size() * c_p) > num)) {
		    switch(num % c_p) {
		    case 0:
			ret = KRdata[num / c_p].State.lnk;
			break;
		    case 1:
			ret = KRdata[num / c_p].On.lnk;
			break;
		    case 2:
			ret = KRdata[num / c_p].Off.lnk;
			break;
		    case 3:
			ret = KRdata[num / c_p].Run.lnk;
			break;
		    case 4:
			ret = KRdata[num / c_p].Reset.lnk;
			break;
		    case 5:
			ret = KRdata[num / c_p].Ban_MC.lnk;
			break;
		    case 6:
			ret = KRdata[num / c_p].Lubrication.lnk;
			break;
		    }
		}
	    }
	    return ret;
	}
    };

} //End namespace

#endif //DA_GKR_H
