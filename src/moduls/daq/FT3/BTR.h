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
#ifndef DA_BTR_H
#define DA_BTR_H

#include "da.h"

namespace FT3
{
    class B_BTR: public DA
    {
    public:
	//Methods
	B_BTR(TMdPrm& prm, uint16_t id, uint16_t nu, uint16_t nr, bool has_params);
	~B_BTR();
	uint16_t ID;
	uint16_t count_nu;
	uint16_t count_nr;
	bool with_params;
	uint16_t Task(uint16_t);
	uint16_t HandleEvent(uint8_t *);
	uint8_t cmdGet(uint16_t prmID, uint8_t * out);
	uint8_t cmdSet(uint8_t * req, uint8_t addr);
	uint8_t SetNewflVal(flData& d, uint8_t addr, uint16_t prmID, float val);
	uint16_t setVal(TVal &val);
	string getStatus(void);
	void saveIO(void);
	void loadIO(bool force = false );
	void tmHandler(void);
	class STRchannel
	{
	public:
	    STRchannel(uint8_t iid) :
		    id(iid), Value(TSYS::strMess("value_%d", id + 1).c_str(), TSYS::strMess(_("Value %d"), id + 1).c_str())
	    {
	    }
	    uint8_t id;

	    flData Value;
	};
	vector<STRchannel> data;
	int lnkSize()
	{
	    return data.size();
	}
	int lnkId(const string &id)
	{
	    for(int i_l = 0; i_l < data.size(); i_l++) {
		if(data[i_l].Value.lnk.prmName == id) return i_l;
	    }
	    return -1;
	}
	SLnk &lnk(int num)
	{
	    return data[num].Value.lnk;
	}
    };

} //End namespace

#endif //DA_BTR_H
