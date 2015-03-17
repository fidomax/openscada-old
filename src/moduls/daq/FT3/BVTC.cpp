//OpenSCADA system module DAQ.FT3 file: BVTC.cpp
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

#include "mod_FT3.h"
#include "BVTC.h"

using namespace FT3;
//******************************************************
//* B_BVTC                                             *
//******************************************************
B_BVTC::B_BVTC(TMdPrm *prm, uint16_t id, uint16_t n, bool has_params) :
	DA(prm), count_n(n), ID(id << 12), with_params(has_params)
{
    TFld * fld;
    mPrm->p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    for(int i = 0; i < count_n; i++) {
	data.push_back(STCchannel(i));
	mPrm->p_el.fldAdd(fld = new TFld(data[i].Value.lnk.prmName.c_str(), data[i].Value.lnk.prmDesc.c_str(), TFld::Boolean, TFld::NoWrite));
	data[i].Value.vl = EVAL_BOOL;
	fld->setReserve("1:" + TSYS::int2str((i) / 8));
	if(with_params) {
	    data[i].Mask.vl = EVAL_BOOL;
	    mPrm->p_el.fldAdd(fld = new TFld(data[i].Mask.lnk.prmName.c_str(), data[i].Mask.lnk.prmDesc.c_str(), TFld::Boolean, TVal::DirWrite));
	    fld->setReserve("2:" + TSYS::int2str((i) / 8));
	}
    }
    loadIO(true);
}

B_BVTC::~B_BVTC()
{
    data.clear();
}

string B_BVTC::getStatus(void)
{
    string rez;
    if(NeedInit) {
	rez = "20: Опрос";
    } else {
	rez = "0: Норма";
    }
    return rez;

}

void B_BVTC::loadIO( bool force )
{
    //Load links
//    mess_info("B_BVTC::loadIO", "");
    if(mPrm->owner().startStat() && !force) {
	mPrm->modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws

    TConfig cfg(&mPrm->prmIOE());
    cfg.cfg("PRM_ID").setS(mPrm->ownerPath(true));
    string io_bd = mPrm->owner().DB() + "." +mPrm->typeDBName()+  "_io";
    mess_info("B_BVTC::loadIO", "io_bd %s ",io_bd.c_str());
    for(int i = 0; i < count_n; i++) {
	cfg.cfg("ID").setS(data[i].Value.lnk.prmName);
	if(!SYS->db().at().dataGet(io_bd, mPrm->owner().owner().nodePath()+mPrm->typeDBName()+"_io", cfg, false, true)) continue;
	data[i].Value.lnk.prmAttr = cfg.cfg("VALUE").getS();
	data[i].Value.lnk.aprm = SYS->daq().at().attrAt(data[i].Value.lnk.prmAttr, '.', true);
	cfg.cfg("ID").setS(data[i].Mask.lnk.prmName);
	if(!SYS->db().at().dataGet(io_bd, mPrm->owner().owner().nodePath()+mPrm->typeDBName()+"_io", cfg, false, true)) continue;
	data[i].Mask.lnk.prmAttr = cfg.cfg("VALUE").getS();
	data[i].Mask.lnk.aprm = SYS->daq().at().attrAt(data[i].Mask.lnk.prmAttr, '.', true);
    }

}

void B_BVTC::saveIO()
{
    //Save links
    TConfig cfg(&mPrm->prmIOE());
    cfg.cfg("PRM_ID").setS(mPrm->ownerPath(true));
    string io_bd = mPrm->owner().DB() + "." +mPrm->typeDBName()+  "_io";
//    mess_info("B_BVTC::saveIO", "io_bd %s ",io_bd.c_str());
    for(int i = 0; i < count_n; i++) {
	cfg.cfg("ID").setS(data[i].Value.lnk.prmName);
	cfg.cfg("VALUE").setS(data[i].Value.lnk.prmAttr);
	SYS->db().at().dataSet(io_bd, mPrm->owner().owner().nodePath()+mPrm->typeDBName()+"_io", cfg);
	mess_info("B_BVTC::saveIO", "path %s ",(mPrm->owner().owner().nodePath()+mPrm->typeDBName()+"_io").c_str());
	cfg.cfg("ID").setS(data[i].Mask.lnk.prmName);
	cfg.cfg("VALUE").setS(data[i].Mask.lnk.prmAttr);
	SYS->db().at().dataSet(io_bd, mPrm->owner().owner().nodePath()+mPrm->typeDBName()+"_io", cfg);
    }
}

void B_BVTC::tmHandler(void)
{
    NeedInit = false;
    for(int i = 0; i < count_n; i++) {
	uint8_t tmpval;
	if(data[i].Mask.vl == 0) {
	    if(data[i].Value.lnk.aprm.freeStat()) {
		//no connection
		data[i].Value.vl = EVAL_BOOL;
//		mess_info("B_BVTC::tmHandler", "Value no connection %s %s",data[i].Value.lnk.prmName.c_str(), data[i].Value.lnk.prmAttr.c_str());
	    } else {
		tmpval = data[i].Value.lnk.aprm.at().getB();
		if(tmpval != data[i].Value.vl) {
		    data[i].Value.vl = tmpval;
		    mPrm->vlAt(data[i].Value.lnk.prmName.c_str()).at().setB(tmpval, 0, true);
//		    mess_info("B_BVTC::tmHandler", "Value new value %s %s %d",data[i].Value.lnk.prmName.c_str(), data[i].Value.lnk.prmAttr.c_str(), tmpval);
		    uint8_t E[6];
		    uint8_t g = i/8;
		    E[0] = 1;
		    E[1] = 3;
		    E[2] = ID|(1<<6)|(g);
		    E[3] = (ID|(1<<6)|(g))>>8;

		    E[4] = 0; //TC
		    for(int j = 0; j < 8; j++) {
			E[4]|=(data[g*8+j].Value.vl)<<j;
		    }
		    uint8_t DHM[5];
		    time_t rawtime;
		    time(&rawtime);
		    mPrm->owner().Time_tToDateTime(DHM,rawtime);
		    mPrm->owner().PushInBE(E,DHM);
		}
	    }
	}
	if(with_params) {
	    if(data[i].Mask.lnk.aprm.freeStat()) {
		//no connection
		data[i].Mask.vl = EVAL_BOOL;
//		mess_info("B_BVTC::tmHandler", "Mask no connection %s %s",data[i].Mask.lnk.prmName.c_str(), data[i].Mask.lnk.prmAttr.c_str());
	    } else {
		tmpval = data[i].Mask.lnk.aprm.at().getB();
		if(tmpval != data[i].Mask.vl) {
		    data[i].Mask.vl = tmpval;
		    data[i].Mask.s = 0;
		    mPrm->vlAt(data[i].Mask.lnk.prmName.c_str()).at().setB(tmpval, 0, true);
//		    mess_info("B_BVTC::tmHandler", "Mask new mask %s %s %d",data[i].Mask.lnk.prmName.c_str(), data[i].Mask.lnk.prmAttr.c_str(), tmpval);
		}
	    }
	}

    }
}

uint16_t B_BVTC::Task(uint16_t uc)
{
    tagMsg Msg;
    uint16_t rc = 0;
    switch(uc) {
    case TaskRefresh:
	Msg.L = 5;
	Msg.C = AddrReq;
	*((uint16_t *) Msg.D) = ID | (0 << 6) | (0); //state
	if(mPrm->owner().Transact(&Msg)) {
	    if(Msg.C == GOOD3) {
		mPrm->vlAt("state").at().setI(Msg.D[7], 0, true);
		uint16_t nTC = (count_n / 8 + (count_n % 8 ? 1 : 0));
		Msg.L = 3 + nTC * 2;
		Msg.C = AddrReq;
		for(int i = 0; i < nTC; i++) {
		    *((uint16_t *) (Msg.D + i * 2)) = ID | (1 << 6) | (i); //TC Value
		}
		if(mPrm->owner().Transact(&Msg)) {
		    if(Msg.C == GOOD3) {
			for(int i = 1; i <= count_n; i++) {
			    mPrm->vlAt(TSYS::strMess("TC_%d", i).c_str()).at().setB(((Msg.D[7 + ((i - 1) / 8 * 5)]) >> ((i - 1) % 8)) & 0x01, 0, true);
			}
			if(with_params) {
			    Msg.L = 3 + nTC * 2;
			    Msg.C = AddrReq;
			    for(int i = 0; i < nTC; i++) {
				*((uint16_t *) (Msg.D + i * 2)) = ID | (2 << 6) | (i); //маски ТC
			    }
			    if(mPrm->owner().Transact(&Msg)) {
				if(Msg.C == GOOD3) {
				    for(int i = 1; i <= count_n; i++) {
					mPrm->vlAt(TSYS::strMess("Mask_%d", i).c_str()).at().setB(((Msg.D[8 + ((i - 1) / 8 * 6)]) >> ((i - 1) % 8)) & 0x01, 0,
						true);
				    }
				    rc = 1;
				}
			    }
			} else {
			    rc = 1;
			}
		    }
		}
	    }
	}
	if(rc) NeedInit = false;
	break;
    }
    return rc;
}

uint16_t B_BVTC::HandleEvent(uint8_t * D)
{
    if((TSYS::getUnalign16(D) & 0xF000) != ID) return 0;
    uint16_t l = 0;
    uint16_t k = (TSYS::getUnalign16(D) >> 6) & 0x3F; // object
    uint16_t n = TSYS::getUnalign16(D) & 0x3F;  // param
//    mess_info("B_BVTC::HandleEvent", "g k n %d %d %d",ID>>12,k,n);
    switch(k) {
    case 0:
	switch(n) {
	case 0:
	    mPrm->vlAt("state").at().setI(D[2], 0, true);
	    l = 3;
	    break;
	case 1:
	    mPrm->vlAt("state").at().setI(D[2], 0, true);
	    l = 3 + count_n / 4;
	    for(int j = 1; j <= count_n; j++) {

		mPrm->vlAt(TSYS::strMess("TC_%d", j).c_str()).at().setB((D[((j - 1) >> 3) + 3] >> (j % 8)) & 1, 0, true);
		if(with_params) {
		    mPrm->vlAt(TSYS::strMess("Mask_%d", j).c_str()).at().setB((D[((j - 1) >> 3) + 3 + count_n / 8] >> (j % 8)) & 1, 0, true);
		}

	    }
	    break;

	}
	break;
    case 1:
	l = 3;
	for(int i = 0; i < 8; i++) {
	    if((1 + (n << 3) + i) > count_n) break;
	    mPrm->vlAt(TSYS::strMess("TC_%d", 1 + (n << 3) + i).c_str()).at().setB((D[2] >> i) & 1, 0, true);
	}
	break;
    case 2:
	l = 4;
	if(with_params) {
	    for(int i = 0; i < 8; i++) {
		if((1 + (n << 3) + i) > count_n) break;
		mPrm->vlAt(TSYS::strMess("Mask_%d", 1 + (n << 3) + i).c_str()).at().setB((D[3] >> i) & 1, 0, true);
	    }
	}
	break;
    }
    return l;
}

uint8_t B_BVTC::cmdGet(uint16_t prmID, uint8_t * out)
{
    if((prmID & 0xF000) != ID) return 0;
    uint16_t k = (prmID >> 6) & 0x3F; // object
    uint16_t n = prmID & 0x3F;  // param
    uint l = 0;
    uint16_t nTC = (count_n / 8 + (count_n % 8 ? 1 : 0));
    switch(k) {
    case 0:
	switch(n) {
	case 0:
	    //state
	    out[0] = 0;
	    l = 1;
	    break;
	case 1:
	    out[0] = 0;
	    l = 1;
	    //value
	    for(uint8_t i = 0; i < nTC; i++) {
		out[i + 1] = 0;
		for(uint8_t j = i * 8; j < (i + 1) * 8; j++)
		    out[i + 1] |= (data[j].Value.vl & 0x01) << (j % 8);
		l++;
	    }
	    //mask;
	    for(uint8_t i = 0; i < nTC; i++) {
		out[i+nTC+1] = 0;
		for(uint8_t j = i * 8; j < (i + 1) * 8; j++)
		    out[i * 2 + 2] |= (data[j].Mask.vl & 0x01) << (j % 8);
		l++;
	    }
	    break;
	}
	break;

    case 1:
	//value
	out[0] = 0;
	if(n < nTC) {
	    for(uint8_t j = n * 8; j < (n + 1) * 8; j++) {
		out[0] |= (data[j].Value.vl & 0x01) << (j % 8);
	    }
	    l = 1;
	}
	break;
    case 2:
	//mask
	out[0] = 0;
	out[1] = 0;
	if(n < nTC) {
	    for(uint8_t j = n * 8; j < (n + 1) * 8; j++){
		out[0] = data[j].Mask.s;
		out[1] |= (data[j].Mask.vl & 0x01) << (j % 8);
	    }
	    l = 2;
	}
	break;
    }
    return l;
}

uint8_t B_BVTC::cmdSet(uint8_t * req, uint8_t  addr)
{
//    mess_info("BVTC ", _("cmdSet"));
    uint16_t prmID = TSYS::getUnalign16(req);
    if((prmID & 0xF000) != ID) return 0;
//    mess_info("BVTC ", _("prmID %04X"),prmID);
    uint16_t k = (prmID >> 6) & 0x3F; // object
    uint16_t n = prmID & 0x3F;  // param
    uint l = 0;
//    mess_info("BVTC ", _("k %d"),k);
//    mess_info("BVTC ", _("n %d"),n);
    uint16_t nTC = (count_n / 8 + (count_n % 8 ? 1 : 0));
    switch (k){
    case 2:
	if(n < nTC) {
	    uint8_t newMask=req[2];
	    for(uint8_t j = n * 8; j < (n + 1) * 8; j++) {
		mess_info("BVTC ", _("found! %d"), j);

		data[j].Mask.s = addr;
		data[j].Mask.vl = newMask & 0x01;
		if(!data[j].Mask.lnk.aprm.freeStat()) {
		    data[j].Mask.lnk.aprm.at().setB(data[j].Mask.vl);
//		    mess_info("B_BVTC::cmdSet", "Set new mask %s %s %d",data[j].MaskLink.prmName.c_str(), data[j].MaskLink.prmAttr.c_str(),data[j].Mask);
		} else {
//		    mess_info("B_BVTC::cmdSet", "Set new mask EROOR!!! %s %s %d",data[j].MaskLink.prmName.c_str(), data[j].MaskLink.prmAttr.c_str(),data[j].Mask);
		}
		mPrm->vlAt(data[j].Mask.lnk.prmName.c_str()).at().setB(data[j].Mask.vl, 0, true);

		newMask = newMask >> 1;
	    }
	    l = 3;
	}
    }
    return l;
}

uint16_t B_BVTC::setVal(TVal &val)
{
//    mess_info("BVTC ", _("setVal"));
    int off = 0;
    uint16_t k = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // object
    uint16_t n = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // param
    uint16_t addr = ID | (k << 6) | n;

    tagMsg Msg;
    Msg.L = 6;
    Msg.C = SetData;
    Msg.D[0] = addr & 0xFF;
    Msg.D[1] = (addr >> 8) & 0xFF;
    Msg.D[2] = 0;
    uint16_t st = n * 8 + 1;
    uint16_t en = (n + 1) * 8;
    if(en > count_n) en = count_n;
    for(int i = st; i <= en; i++) {
	Msg.D[2] |= ((mPrm->vlAt(TSYS::strMess("Mask_%d", i).c_str()).at().getB(0, true)) << ((i - 1) % 8));
    }
    mPrm->owner().Transact(&Msg);

    return 0;
}

//---------------------------------------------------------------------------
