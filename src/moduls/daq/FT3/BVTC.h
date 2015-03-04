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
#ifndef DA_BVTC_H
#define DA_BVTC_H

#include "da.h"

namespace FT3
{
    class B_BVTC: public DA
    {
    public:
	//Methods
	B_BVTC(TMdPrm *prm, uint16_t id, uint16_t n, bool has_params);
	~B_BVTC();
	uint16_t ID;
	uint16_t count_n;
	bool with_params;
	uint16_t Task(uint16_t);
	uint16_t HandleEvent(uint8_t *);
	uint8_t GetData(uint16_t prmID, uint8_t * out);
	uint16_t setVal(TVal &val);
	string getStatus(void);
	void tmHandler(void);
	class STCchannel
	{
	public:
	    STCchannel(uint8_t iid) :
		    id(iid), Mask(0), Value(EVAL_BOOL),
		    ValueLink(SLnk(TSYS::strMess("TC_%d", id+1).c_str(), TSYS::strMess(_("TC %d"), id+1).c_str())),
		    MaskLink(SLnk(TSYS::strMess("Mask_%d", id+1).c_str(), TSYS::strMess(_("Mask %d"), id+1).c_str()))
	    {
	    }
	    uint8_t id;
	    uint8_t Value;
	    SLnk ValueLink;
	    uint8_t Mask;
	    SLnk MaskLink;
	};
	vector<STCchannel> data;
	int lnkSize( ){
	    if(with_params) {
		return data.size() * 2;
	    } else {
		return data.size();
	    }
	}
	int lnkId( const string &id ){
	    if(with_params) {
		for(int i_l = 0; i_l < data.size(); i_l++) {
		    if(data[i_l].ValueLink.prmName == id) {
			return i_l * 2;
		    }
		    if(data[i_l].MaskLink.prmName == id) {
			return i_l * 2 + 1;
		    }
		}
	    } else {
		for(int i_l = 0; i_l < data.size(); i_l++) {
		    if(data[i_l].ValueLink.prmName == id) {
			return i_l;
		    }
		}
	    }
	    return -1;
	}
	SLnk &lnk(int num)
	{
	    if(with_params) {
		switch(num % 2) {
		case 0:
		    return data[num / 2].ValueLink;
		case 1:
		    return data[num / 2].MaskLink;
		}
	    } else
		return data[num].ValueLink;
	}
    };

} //End namespace

#endif //DA_BVTC_H
