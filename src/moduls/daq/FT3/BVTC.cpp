//OpenSCADA system module DAQ.AMRDevs file: da_Ergomera.cpp
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
//*************************************************
//* BVI                                           *
//*************************************************

B_BVTC::B_BVTC(TMdPrm *prm, uint16_t id, uint16_t n, bool has_params) :
	DA(prm), count_n(n), ID(id << 12), with_params(has_params)
{
    TFld * fld;
    mPrm->p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    for(int i = 0; i < count_n; i++) {
	data.push_back(STCchannel(i));
	mPrm->p_el.fldAdd(fld = new TFld(data[i].ValueLink.prmName.c_str(), data[i].ValueLink.prmDesc.c_str(), TFld::Boolean, TFld::NoWrite));
//	mlnk.push_back(data[i].ValueLink);
	fld->setReserve("1:" + TSYS::int2str((i) / 8));
	if(with_params) {
	    data[i].Mask = EVAL_BOOL;
//	    mlnk.push_back(data[i].MaskLink);
	    mPrm->p_el.fldAdd(fld = new TFld(data[i].MaskLink.prmName.c_str(), data[i].MaskLink.prmDesc.c_str(), TFld::Boolean, TVal::DirWrite));
	    fld->setReserve("2:" + TSYS::int2str((i) / 8));
	}
    }
    loadIO();

}

B_BVTC::~B_BVTC()
{

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
    mess_info("B_BVTC::loadIO", "");
    if(mPrm->owner().startStat() && !force) {
	mPrm->modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws

    TConfig cfg(&mPrm->prmIOE());
    cfg.cfg("PRM_ID").setS(mPrm->ownerPath(true));
    string io_bd = mPrm->owner().DB() + "." +mPrm->typeDBName()+  "_io";
    mess_info("B_BVTC::loadIO", "io_bd %s ",io_bd.c_str());
    for(int i = 0; i < count_n; i++) {
	cfg.cfg("ID").setS(data[i].ValueLink.prmName);
	if(!SYS->db().at().dataGet(io_bd, mPrm->owner().owner().nodePath()+mPrm->typeDBName()+"_io", cfg, false, true)) continue;
	data[i].ValueLink.prmAttr = cfg.cfg("VALUE").getS();
	cfg.cfg("ID").setS(data[i].MaskLink.prmName);
	if(!SYS->db().at().dataGet(io_bd, mPrm->owner().owner().nodePath()+mPrm->typeDBName()+"_io", cfg, false, true)) continue;
	data[i].MaskLink.prmAttr = cfg.cfg("VALUE").getS();
    }

}

void B_BVTC::saveIO()
{
    //Save links
    TConfig cfg(&mPrm->prmIOE());
    cfg.cfg("PRM_ID").setS(mPrm->ownerPath(true));
    string io_bd = mPrm->owner().DB() + "." +mPrm->typeDBName()+  "_io";
    mess_info("B_BVTC::saveIO", "io_bd %s ",io_bd.c_str());
    for(int i = 0; i < count_n; i++) {
	cfg.cfg("ID").setS(data[i].ValueLink.prmName);
	cfg.cfg("VALUE").setS(data[i].ValueLink.prmAttr);
	SYS->db().at().dataSet(io_bd, mPrm->owner().owner().nodePath()+mPrm->typeDBName()+"_io", cfg);
	mess_info("B_BVTC::saveIO", "path %s ",(mPrm->owner().owner().nodePath()+mPrm->typeDBName()+"_io").c_str());
	cfg.cfg("ID").setS(data[i].MaskLink.prmName);
	cfg.cfg("VALUE").setS(data[i].MaskLink.prmAttr);
	SYS->db().at().dataSet(io_bd, mPrm->owner().owner().nodePath()+mPrm->typeDBName()+"_io", cfg);
    }
}

void B_BVTC::tmHandler(void)
{
    for(int i = 0; i < count_n; i++) {
	uint8_t tmpval;
	if(data[i].Mask == 0) {
	    if(data[i].ValueLink.aprm.freeStat()) {
		//no connection
		data[i].Value = EVAL_BOOL;
	    } else {
		tmpval = data[i].ValueLink.aprm.at().getB();
		if(tmpval != data[i].Value) {
		    data[i].Value = tmpval;
		    mPrm->vlAt(data[i].ValueLink.prmName.c_str()).at().setB(tmpval, 0, true);
		    //TODO putinBE;
		}
	    }
	}
	if(with_params) {
	    if(data[i].MaskLink.aprm.freeStat()) {
		//no connection
		data[i].Mask = EVAL_BOOL;
	    } else {
		tmpval = data[i].MaskLink.aprm.at().getB();
		if(tmpval != data[i].Mask) {
		    data[i].Mask = tmpval;
		    data[i].sMask = 0;
		    mPrm->vlAt(data[i].MaskLink.prmName.c_str()).at().setB(tmpval, 0, true);
		    //TODO putinBE;
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
	*((uint16_t *) Msg.D) = ID | (0 << 6) | (0); //состояние
	if(mPrm->owner().Transact(&Msg)) {
	    if(Msg.C == GOOD3) {
		mPrm->vlAt("state").at().setI(Msg.D[7], 0, true);
		uint16_t nTC = (count_n / 8 + (count_n % 8 ? 1 : 0));
		Msg.L = 3 + nTC * 2;
		Msg.C = AddrReq;
		for(int i = 0; i < nTC; i++) {
		    *((uint16_t *) (Msg.D + i * 2)) = ID | (1 << 6) | (i); //Значение ТC
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
    uint16_t k = (TSYS::getUnalign16(D) >> 6) & 0x3F; // номер объекта
    uint16_t n = TSYS::getUnalign16(D) & 0x3F;  // номер параметра
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
    uint16_t k = (prmID >> 6) & 0x3F; // номер объекта
    uint16_t n = prmID & 0x3F;  // номер параметра
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
		    out[i + 1] |= (data[j].Value & 0x01) << (j % 8);
		l++;
	    }
	    //mask;
	    for(uint8_t i = 0; i < nTC; i++) {
		out[i+nTC+1] = 0;
		for(uint8_t j = i * 8; j < (i + 1) * 8; j++)
		    out[i * 2 + 2] |= (data[j].Mask & 0x01) << (j % 8);
		l++;
	    }
	    break;
	}
	break;

    case 1:
	//value
	out[0] = 0;
	mess_info("getData", _("out[0] before %02X "),out[0]);
	if(n < nTC) {
	    for(uint8_t j = n * 8; j < (n + 1) * 8; j++) {
		out[0] |= (data[j].Value & 0x01) << (j % 8);
		mess_info("getData", _("out[0] %d %02X "),j, out[0]);
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
		out[0] = data[j].sMask;
		out[1] |= (data[j].Mask & 0x01) << (j % 8);
	    }
	    l = 2;
	}
	break;
    }
    return l;
}

uint8_t B_BVTC::cmdSet(uint8_t * req, uint8_t  addr)
{
    mess_info("BVTC ", _("cmdSet"));
    uint16_t prmID = TSYS::getUnalign16(req);
    if((prmID & 0xF000) != ID) return 0;
    mess_info("BVTC ", _("prmID %04X"),prmID);
    uint16_t k = (prmID >> 6) & 0x3F; // номер объекта
    uint16_t n = prmID & 0x3F;  // номер параметра
    uint l = 0;
    mess_info("BVTC ", _("k %d"),k);
    mess_info("BVTC ", _("n %d"),n);
    uint16_t nTC = (count_n / 8 + (count_n % 8 ? 1 : 0));
    switch (k){
    case 2:
	if(n < nTC) {
	    uint8_t newMask=req[2];
	    for(uint8_t j = n * 8; j < (n + 1) * 8; j++){
		    mess_info("BVTC ", _("found! %d"),j);

/*		data[j].sMask = addr;
		data[j].Mask = newMask & 0x01;
		if(data[j].MaskLink.aprm.freeStat()) data[j].MaskLink.aprm.at().setB(data[j].Mask,0,true);
		mPrm->vlAt(data[j].MaskLink.prmName.c_str()).at().setB(data[j].Mask, 0, true);
		newMask=newMask>>1;*/
	    }
	    l = 3;
	}
    }
    mess_info("BVTC ", _("l %d"),l);
    return l;
}

uint16_t B_BVTC::setVal(TVal &val)
{
    int off = 0;
    uint16_t k = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // номер объекта
    uint16_t n = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // номер параметра
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