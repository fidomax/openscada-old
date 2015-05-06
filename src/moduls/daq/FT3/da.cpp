//OpenSCADA system module DAQ.FT3 file: da.cpp
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

#include <sys/times.h>

#include <tsys.h>

#include "mod_FT3.h"
#include "da.h"

using namespace FT3;

void DA::loadLnk(SLnk& lnk, const string& io_bd, TConfig& cfg)
{
    cfg.cfg("ID").setS(lnk.prmName);
    if(SYS->db().at().dataGet(io_bd, mPrm.owner().owner().nodePath() + mPrm.typeDBName() + "_io", cfg, false, true)) {
	lnk.prmAttr = cfg.cfg("VALUE").getS();
	lnk.aprm = SYS->daq().at().attrAt(lnk.prmAttr, '.', true);
    }
}

void DA::saveLnk(SLnk& lnk, const string& io_bd, TConfig& cfg)
{
    cfg.cfg("ID").setS(lnk.prmName);
    cfg.cfg("VALUE").setS(lnk.prmAttr);
    SYS->db().at().dataSet(io_bd, mPrm.owner().owner().nodePath() + mPrm.typeDBName() + "_io", cfg);
}

uint8_t DA::SetNew8Val(ui8Data &d, uint8_t addr, uint16_t prmID, uint8_t val)
{
    if(!d.lnk.aprm.freeStat()) {
	d.s = addr;
	d.vl = val;
	d.lnk.aprm.at().setI(d.vl);
	mPrm.vlAt(d.lnk.prmName.c_str()).at().setI(d.vl, 0, true);
	uint8_t E[2] = { addr, d.vl };
	mPrm.owner().PushInBE(1, sizeof(E), prmID, E);
	return 2 + 1;
    } else {
	return 0;
    }
}

uint8_t DA::SetNewflVal(flData &d, uint8_t addr, uint16_t prmID, float val)
{
    if(!d.lnk.aprm.freeStat()) {
	d.s = addr;
	d.vl = val;
	d.lnk.aprm.at().setR(d.vl);
	mPrm.vlAt(d.lnk.prmName.c_str()).at().setR(d.vl, 0, true);
	uint8_t E[5] = { addr, d.b_vl[0], d.b_vl[1], d.b_vl[2], d.b_vl[3] };
	mPrm.owner().PushInBE(1, sizeof(E), prmID, E);
	return 2 + 4;
    } else {
	return 0;
    }
}

uint8_t DA::SetNew2flVal(flData &d1, flData& d2, uint8_t addr, uint16_t prmID, float val1, float val2)
{
    if((!d1.lnk.aprm.freeStat()) && (!d2.lnk.aprm.freeStat())) {
	d1.s = addr;
	d1.vl = val1;
	d1.lnk.aprm.at().setR(d1.vl);
	mPrm.vlAt(d1.lnk.prmName.c_str()).at().setR(d1.vl, 0, true);
	d2.s = addr;
	d2.vl = val2;
	d2.lnk.aprm.at().setR(d2.vl);
	mPrm.vlAt(d2.lnk.prmName.c_str()).at().setR(d2.vl, 0, true);
	uint8_t E[9] = { addr, d1.b_vl[0], d1.b_vl[1], d1.b_vl[2], d1.b_vl[3], d2.b_vl[0], d2.b_vl[1], d2.b_vl[2], d2.b_vl[3] };
	mPrm.owner().PushInBE(1, sizeof(E), prmID, E);
	return 2 + 4 + 4;
    } else {
	return 0;
    }
}
