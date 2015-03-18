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
#ifndef DA_BVT_H
#define DA_BVT_H

#include "da.h"

namespace FT3
{
    class B_BVT: public DA
    {
    public:
	//Methods
	B_BVT(TMdPrm *prm, uint16_t id, uint16_t n, bool has_params, bool has_k, bool has_rate);
	~B_BVT();
	uint16_t ID;
	uint16_t count_n;

	uint16_t Task(uint16_t);
	uint16_t HandleEvent(uint8_t *);
	uint16_t setVal(TVal &val);
	string getStatus(void);
	class STTchannel
	{
	public:
	    STTchannel(uint8_t iid) :
		    id(iid),
		    State(TSYS::strMess("state_%d", id+1).c_str(), TSYS::strMess(_("State %d"), id+1).c_str()),
		    Value(TSYS::strMess("value_%d", id+1).c_str(), TSYS::strMess(_("Value %d"), id+1).c_str()),
		    Period(TSYS::strMess("period_%d", id+1).c_str(), TSYS::strMess(_("Measure period %d"), id+1).c_str()),
		    Sens(TSYS::strMess("sens_%d", id+1).c_str(), TSYS::strMess(_("Sensitivity %d"), id+1).c_str()),
		    MinS(TSYS::strMess("minS_%d", id+1).c_str(), TSYS::strMess(_("Sensor minimum %d"), id+1).c_str()),
		    MaxS(TSYS::strMess("maxS_%d", id+1).c_str(), TSYS::strMess(_("Senso  r maximum %d"), id+1).c_str()),
		    MinPV(TSYS::strMess("minPV_%d", id+1).c_str(), TSYS::strMess(_("PV minimum %d"), id+1).c_str()),
		    MaxPV(TSYS::strMess("maxPV_%d", id+1).c_str(), TSYS::strMess(_("PV maximum %d"), id+1).c_str()),
		    MinW(TSYS::strMess("minW_%d", id+1).c_str(), TSYS::strMess(_("Warning minimum %d"), id+1).c_str()),
		    MaxW(TSYS::strMess("maxW_%d", id+1).c_str(), TSYS::strMess(_("Warning maximum %d"), id+1).c_str()),
		    MinA(TSYS::strMess("minA_%d", id+1).c_str(), TSYS::strMess(_("Alarm minimum %d"), id+1).c_str()),
		    MaxA(TSYS::strMess("maxA_%d", id+1).c_str(), TSYS::strMess(_("Alarm maximum %d"), id+1).c_str()),
		    Factor(TSYS::strMess("factor_%d", id+1).c_str(), TSYS::strMess(_("Range factor %d"), id+1).c_str()),
		    Dimension(TSYS::strMess("dimens_%d", id+1).c_str(), TSYS::strMess(_("Dimension %d"), id+1).c_str()),
		    CorFactor(TSYS::strMess("corfactor_%d", id+1).c_str(), TSYS::strMess(_("Correcting factor %d"), id+1).c_str()),
		    Rate(TSYS::strMess("rate_%d", id+1).c_str(), TSYS::strMess(_("Rate of change %d"), id+1).c_str()),
		    Calcs(TSYS::strMess("calcs_%d", id+1).c_str(), TSYS::strMess(_("Calcs count %d"), id+1).c_str()),
		    RateSens(TSYS::strMess("ratesens_%d", id+1).c_str(), TSYS::strMess(_("Rate sensitivity %d"), id+1).c_str()),
		    RateLimit(TSYS::strMess("ratelimit_%d", id+1).c_str(), TSYS::strMess(_("Rate limit %d"), id+1).c_str())
	    {
	    }
	    uint8_t id;

	    ui8Data State, Period, Dimension, Calcs;

	    flData Value,Sens,MinS,MaxS,MinPV,MaxPV,MinW,MaxW,MinA,MaxA,Factor,CorFactor,Rate,RateSens,RateLimit;

//	    SLnk MaskLink;
	};
	vector<STTchannel> data;
	int lnkSize()
	{
	    if(with_params) {
		if(with_k) {
		    if(with_rate) {
			return data.size() * 19;
		    } else {
			return data.size() * 15;
		    }
		} else {
		    return data.size() * 14;
		}
	    } else {
		return data.size() * 2;
	    }
	}

    private:
	bool with_params;
	bool with_k;
	bool with_rate;
	vector<SDataRec> chan_err;

    };

} //End namespace

#endif //DA_BVT_H
