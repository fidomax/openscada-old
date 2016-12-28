/***************************************************************************
 *   Copyright (C) 2016 by Maxim Kochetkov                                 *
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
#ifndef DA_UPZ_H
#define DA_UPZ_H

#include "da.h"

namespace FT3
{
    enum eKA_UPZ_VS
    {
	UPZ_NORMAL = 0, UPZ_OFF = 1, NUM_ALARM_SIGNAL = 8
    };
    enum eKA_UPZ_State
    {
	KA_UPZ_Error = 0x0, KA_UPZ_Normal = 0x1
    };
    class KA_UPZ: public DA
    {
    public:
	//Methods
	KA_UPZ(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params);
	~KA_UPZ();
	uint16_t ID;
	uint16_t count_n;
	uint16_t config;
	void AddChannel(uint8_t iid);
	uint16_t GetState(void);
	uint16_t SetParams(void);
	uint16_t RefreshParams(void);
	uint16_t RefreshData(void);
	uint16_t HandleEvent(int64_t, uint8_t *);
	uint8_t cmdGet(uint16_t prmID, uint8_t * out);
	uint8_t cmdSet(uint8_t * req, uint8_t addr);
	uint16_t setVal(TVal &val);
	string getStatus(void);
	void saveIO(void);
	void loadIO(bool force = false);
	void saveParam(void);
	void loadParam(void);
	void tmHandler(void);
	class SKAUPZchannel
	{
	public:
	    SKAUPZchannel(uint8_t iid, DA* owner) :
		    da(owner), id(iid), State(TSYS::strMess("state_%d", id + 1), TSYS::strMess(_("State %d"), id + 1)),
		    Function(TSYS::strMess("function_%d", id + 1), TSYS::strMess(_("Function %d"), id + 1)),
		    Addr_obj1(TSYS::strMess("addr_obj1_%d", id + 1), TSYS::strMess(_("Address object 1 %d"), id + 1)),
		    Addr_obj2(TSYS::strMess("addr_obj2_%d", id + 1), TSYS::strMess(_("Address object 2 %d"), id + 1)),
		    Alarm_obj1(TSYS::strMess("alarm_obj1_%d", id + 1), TSYS::strMess(_("Alarm object 1 %d"), id + 1)),
		    Time1(TSYS::strMess("time1_%d", id + 1), TSYS::strMess(_("Time1 %d"), id + 1)),
		    Number_function1(TSYS::strMess("numb_func1_%d", id + 1), TSYS::strMess(_("Number function 1 %d"), id + 1)),
		    Number_object1(TSYS::strMess("numb_obj1_%d", id + 1), TSYS::strMess(_("Number object 1 %d"), id + 1)),
		    Alarm_obj2(TSYS::strMess("alarm_obj2_%d", id + 1), TSYS::strMess(_("Alarm object 2 %d"), id + 1)),
		    Time2(TSYS::strMess("time2_%d", id + 1), TSYS::strMess(_("Time2 %d"), id + 1)),
		    Number_function2(TSYS::strMess("numb_func2_%d", id + 1), TSYS::strMess(_("Number function 2 %d"), id + 1)),
		    Number_object2(TSYS::strMess("numb_obj2_%d", id + 1), TSYS::strMess(_("Number object 2 %d"), id + 1))
	    {
	    }
	    DA* da;
	    uint8_t id;

	    ui8Data State, Number_function1, Number_object1, Number_function2, Number_object2, Function;

	    ui16Data Addr_obj1, Addr_obj2, Alarm_obj1, Time1, Alarm_obj2, Time2;
	};
	vector<SKAUPZchannel> data;
	int lnkSize()
	{
	    if(with_params) {
		return data.size() * 12;
	    } else {
		return data.size() * 2;
	    }
	}
	int lnkId(const string &id)
	{

	    if(with_params) {
		for(int i_l = 0; i_l < data.size(); i_l++) {
		    if(data[i_l].State.lnk.prmName == id) return i_l * 12;
		    if(data[i_l].Function.lnk.prmName == id) return i_l * 12 + 1;
		    if(data[i_l].Addr_obj1.lnk.prmName == id) return i_l * 12 + 2;
		    if(data[i_l].Addr_obj2.lnk.prmName == id) return i_l * 12 + 3;

		    if(data[i_l].Alarm_obj1.lnk.prmName == id) return i_l * 12 + 4;
		    if(data[i_l].Time1.lnk.prmName == id) return i_l * 12 + 5;
		    if(data[i_l].Number_function1.lnk.prmName == id) return i_l * 12 + 6;
		    if(data[i_l].Number_object1.lnk.prmName == id) return i_l * 12 + 7;

		    if(data[i_l].Alarm_obj2.lnk.prmName == id) return i_l * 12 + 8;
		    if(data[i_l].Time2.lnk.prmName == id) return i_l * 12 + 9;
		    if(data[i_l].Number_function2.lnk.prmName == id) return i_l * 12 + 10;
		    if(data[i_l].Number_object2.lnk.prmName == id) return i_l * 12 + 11;
		}
	    } else {
		for(int i_l = 0; i_l < data.size(); i_l++) {
		    if(data[i_l].State.lnk.prmName == id) return i_l * 2;
		    if(data[i_l].Function.lnk.prmName == id) return i_l * 2 + 1;
		}
	    }
	    return -1;
	}
	SLnk &lnk(int num)
	{
	    int k;
	    if(with_params) {
		k = 8;
	    } else {
		k = 2;
	    }
	    switch(num % k) {
	    case 0:
		return data[num / k].State.lnk;
	    case 1:
		return data[num / k].Function.lnk;
	    case 2:
		return data[num / k].Addr_obj1.lnk;
	    case 3:
		return data[num / k].Addr_obj2.lnk;
	    case 4:
		return data[num / k].Alarm_obj1.lnk;
	    case 5:
		return data[num / k].Time1.lnk;
	    case 6:
		return data[num / k].Number_function1.lnk;
	    case 7:
		return data[num / k].Number_object1.lnk;
	    case 8:
		return data[num / k].Alarm_obj2.lnk;
	    case 9:
		return data[num / k].Time2.lnk;
	    case 10:
		return data[num / k].Number_function2.lnk;
	    case 11:
		return data[num / k].Number_object2.lnk;
	    }
	}

    private:
	bool with_params;
	vector<SDataRec> chan_err;

    };

} //End namespace

#endif //DA_UPZ_H
