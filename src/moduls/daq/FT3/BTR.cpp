//OpenSCADA system module DAQ.FT3 file: BTR.cpp
/***************************************************************************
*   Copyright (C) 2011-2016 by Maxim Kochetkov                            *
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
#include "BTR.h"

using namespace FT3;

KA_BTU::KA_BTU(TMdPrm& prm, uint16_t id, uint16_t nu, bool has_params) :
    DA(prm, id), count_nu(nu), with_params(has_params), config(4 | (nu << 4) | (16 << 10))
{
    mTypeFT3 = KA;
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    if (mPrm.owner().cfg("PRTTYPE").getS() == "KA") {
	mPrm.p_el.fldAdd(fld = new TFld("execution", _("Execution"), TFld::Integer, TVal::DirWrite));
	fld->setReserve("0:2");
    }
    loadIO(true);
}

KA_BTU::~KA_BTU()
{
}

string KA_BTU::getStatus(void)
{
    string rez;

    if (NeedInit)
	rez = "20: Опрос";
    else
	rez = "0: Норма";
    return rez;
}

uint16_t KA_BTU::GetState()
{
    tagMsg Msg;
    uint16_t rc = BlckStateUnknown;

    Msg.L = 5;
    Msg.C = AddrReq;
    *((uint16_t*)Msg.D) = PackID(ID, 0, 0);   //state
    if (mPrm.owner().DoCmd(&Msg) == GOOD3) {
	switch (mPrm.vlAt("state").at().getI(0, true)) {
	case KA_BTU_Error:
	    rc = BlckStateError;
	    break;
	case KA_BTU_Normal:
	    rc = BlckStateNormal;
	    break;
	}
    }
    return rc;
}

void KA_BTU::tmHandler(void)
{
    NeedInit = false;
}

uint16_t KA_BTU::HandleEvent(int64_t tm, uint8_t * D)
{
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(D));

    if (ft3ID.g != ID) return 0;
    uint16_t l = 0;
    if (ft3ID.k == 0) {
	switch (ft3ID.n) {
	case 0:
	    mPrm.vlAt("state").at().setI(D[2], tm, true);
	    l = 3;
	    break;
	case 1:
	    l = 4;
	    break;
	case 2:
	    mPrm.vlAt(TSYS::strMess("execution")).at().setI(D[3], tm, true);
	    l = 4;
	    break;
	}
    }
    return l;
}

uint8_t KA_BTU::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);

    if (ft3ID.g != ID) return 0;
    uint l = 0;
    if (ft3ID.k == 0) {
	switch (ft3ID.n) {
	case 0:
	    out[0] = 1;
	    l = 1;
	    break;
	case 1:
	    out[0] = config & 0xFF;
	    out[1] = (config >> 8) & 0xFF;
	    l = 2;
	    break;
	case 2:
	    out[0] = 0;
	    out[1] = 0;
	    l = 2;
	    break;
	}
    }
    return l;
}

uint8_t KA_BTU::runTU(uint8_t tu)
{
    uint8_t rc = 0;

    vector<string> lst;
    mPrm.list(lst);
    for (int i_l = 0; i_l < lst.size(); i_l++) {
	AutoHD<TMdPrm> t = mPrm.at(lst[i_l]);
        DA *tDA = t.at().mDA;
        if (tDA){
          if (tDA->runTU(tu)) rc = 3;
        }

    }
/*    if (tu == 0x55) {
        for (int i = 0; i < count_nu; i++) {
            if (TUdata[i].iTY) {
                if (TUdata[i].Line.lnk.Connected()) {
                    TUdata[i].Line.lnk.aprm.at().setI(TUdata[i].Line.vl);
                    TUdata[i].Line.vl = 0;
                    TUdata[i].iTY = 0;
                    rc = 3;
                }
            }
        }
    }
    if (tu == 0x0) {
        for (int i = 0; i < count_nu; i++) {
            if (TUdata[i].Line.lnk.Connected()) {
                TUdata[i].Line.vl = 0;
                TUdata[i].iTY = 0;
                TUdata[i].Line.lnk.aprm.at().setI(TUdata[i].Line.vl);
                rc = 3;
            }
        }
   }*/
    return rc;
}

uint8_t KA_BTU::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);

    if (ft3ID.g != ID) return 0;
    uint l = 0;
    uint8_t E[2];
    if (ft3ID.k == 0) {
	switch (ft3ID.n) {
	case 2:
	    l = runTU(req[2]);
	    if (l) {
		uint8_t E[2] = { addr, req[2] };
		PushInBE(1, sizeof(E), prmID, E);
	    }
	    break;
	}
    }
    return l;
}

uint16_t KA_BTU::setVal(TVal &val)
{
    uint16_t rc = 0;
    int off = 0;
    FT3ID ft3ID;

    ft3ID.k = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.n = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.g = ID;

    tagMsg Msg;
    uint8_t run;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
    if (ft3ID.k == 0) {
	switch (ft3ID.n) {
	case 2:
	    run = val.getI(0, true);
	    if (run == 170)
		Msg.L += SerializeB(Msg.D + Msg.L, 0);
	    else {
		if (run == 85)
		    Msg.L += SerializeB(Msg.D + Msg.L, run);
		else
		    Msg.L = 0;
	    }
	    break;
	}
    }
    if (Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return rc;
}

KA_TU::KA_TU(TMdPrm& prm, DA &parent, uint16_t id, bool has_params) :
    DA(prm, id), parentDA(parent), with_params(has_params), iTY(0),
    Line("Line", _("Line"))
{
    mTypeFT3 = KA;
    AddAttr(Line.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("0"));
    if (with_params) {
	for (int i = 0; i < 16; i++) {
	    Time.push_back(ui16Data(TSYS::strMess("Time_%d", i + 1), TSYS::strMess(_("Time %d"), i + 1)));
	    AddAttr(Time[i].lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d", i + 1));
	}
    }
    loadIO(true);
}

KA_TU::~KA_TU()
{
    Time.clear();
}

string KA_TU::getStatus(void)
{
    string rez;

    if (NeedInit)
	rez = "20: Опрос";
    else
	rez = "0: Норма";
    return rez;
}

uint16_t KA_TU::GetState()
{
    uint16_t rc = BlckStateUnknown;

    return rc;
}

uint16_t KA_TU::SetParams(void)
{
    uint16_t rc;
    tagMsg Msg;

    loadParam();
    Msg.L = 0;
    Msg.C = SetData;
    for (int j = 0; j < 16; j++) {
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, j + 1));
	Msg.L += Time[j].SerializeAttr(Msg.D + Msg.L);
	if (Msg.L > mPrm.owner().cfg("MAXREQ").getI()) {
	    Msg.L += 3;
	    rc = mPrm.owner().DoCmd(&Msg);
	    Msg.L = 0;
	    Msg.C = SetData;
	    if ((rc == BAD2) || (rc == BAD3) || (rc == ERROR)) break;
	}
    }
    if (Msg.L) {
	Msg.L += 3;
	rc = mPrm.owner().DoCmd(&Msg);
    }
    if ((rc == BAD2) || (rc == BAD3))
	mPrm.mess_sys(TMess::Error, "Can't set channel %d", ID);
    else if (rc == ERROR)
	mPrm.mess_sys(TMess::Error, "No answer to set channel %d", ID);

    return rc;
}

uint16_t KA_TU::RefreshData(void)
{
    tagMsg Msg;

    Msg.L = 0;
    Msg.C = AddrReq;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, 0));
    Msg.L += 3;
    return mPrm.owner().DoCmd(&Msg);
}

uint16_t KA_TU::RefreshParams(void)
{
    uint16_t rc;
    tagMsg Msg;

    Msg.L = 0;
    Msg.C = AddrReq;
    for (int j = 1; j <= 16; j++)
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(parentDA.ID, ID, j));
    Msg.L += 3;
    rc = mPrm.owner().DoCmd(&Msg);
    if ((rc == BAD2) || (rc == BAD3))
	mPrm.mess_sys(TMess::Error, "Can't refresh channel %d params", ID);
    else if (rc == ERROR)
	mPrm.mess_sys(TMess::Error, "No answer to refresh channel %d params", ID);

    return rc;
}

void KA_TU::loadIO(bool force)
{
    if (mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    } //Load/reload IO context only allow for stopped controllers for prevent throws
    loadLnk(Line.lnk);
    for (int j = 0; j < 16; j++)
	loadLnk(Time[j].lnk);
}

void KA_TU::saveIO()
{
//Save links
    saveLnk(Line.lnk);
    for (int j = 0; j < 16; j++)
	saveLnk(Time[j].lnk);
}

void KA_TU::saveParam(void)
{
    for (int j = 0; j < 16; j++)
	saveVal(Time[j].lnk);
}

void KA_TU::loadParam(void)
{
    for (int j = 0; j < 16; j++)
	loadVal(Time[j].lnk);
}

void KA_TU::tmHandler(void)
{
    for (int j = 0; j < 16; j++)
	UpdateParamW(Time[j], PackID(parentDA.ID, ID, j + 1), 1);
}

uint16_t KA_TU::HandleEvent(int64_t tm, uint8_t * D)
{
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(D));

    if (ft3ID.g != parentDA.ID) return 0;
    if (ft3ID.k != ID) return 0;
    uint16_t l = 0;
    switch (ft3ID.n) {
    case 0:
	Line.Update(D[3], tm);
	l = 4;
	break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
	if (with_params)
	    Time[ft3ID.n - 1].Update(TSYS::getUnalign16(D + 3), tm);
	l = 5;
	break;
    case 32:
	if (with_params)
	    Line.Update(TSYS::getUnalign16(D + 3), tm);
	l = 5;
	break;
    }
    return l;
}

uint8_t KA_TU::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);

    if (ft3ID.g != parentDA.ID) return 0;
    if (ft3ID.k != ID) return 0;
    uint8_t l = 0;
    switch (ft3ID.n) {
    case 0:
	l += SerializeB(out + l, Line.s);
	l += SerializeB(out + l, iTY);
	break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
	l += SerializeB(out + l, Time[ft3ID.n - 1].s);
	l += Time[ft3ID.n - 1].Serialize(out + l);
	break;
    case 32:
	l += SerializeB(out + l, Line.s);
	l += Line.Serialize(out + l);
	break;
    }
    return l;
}

uint8_t KA_TU::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(req));
    uint8_t E[2];

    if (ft3ID.g != parentDA.ID) return 0;
    if (ft3ID.k != ID) return 0;
    uint8_t l = 0;
    switch (ft3ID.n) {
    case 0:
	Line.s = addr;
	iTY = req[2];
	Line.vl = 1 << iTY - 1;
	l = 3;
	E[0] = addr;
	E[1] = req[2];
	PushInBE(1, sizeof(E), prmID, E);
	break;
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
	l = SetNewWVal(Time[ft3ID.n - 1], addr, prmID, TSYS::getUnalign16(req + 2));
	break;
    case 32:
	Line.s = addr;
	iTY = 1;
	Line.vl = TSYS::getUnalign16(req + 2);
	l = 3;
	E[0] = addr;
	E[1] = req[2];
	PushInBE(1, sizeof(E), prmID, E);
	break;
    }
    return l;
}

uint8_t KA_TU::runTU(uint8_t tu)
{
    uint8_t rc = 0;

    if (tu == 0x55) {
	if (iTY) {
	    if (Line.lnk.Connected()) {
		Line.lnk.aprm.at().setI(Line.vl);
		Line.vl = 0;
		iTY = 0;
		rc = 3;
	    }
	}
    }
    if (tu == 0x0) {
	if (Line.lnk.Connected()) {
	    Line.vl = 0;
	    iTY = 0;
	    Line.lnk.aprm.at().setI(Line.vl);
	    rc = 3;
	}

    }
    return rc;
}


uint16_t KA_TU::setVal(TVal &val)
{
    uint16_t rc = 0;
    int off = 0;
    FT3ID ft3ID;

    ft3ID.k = ID;
    ft3ID.n = s2i(val.fld().reserve());
    ft3ID.g = parentDA.ID;

    tagMsg Msg;
    uint8_t run;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));

    rc = 1;
    if (ft3ID.n == 0)
	Msg.L += SerializeB(Msg.D + Msg.L, val.getI(0, true));
    else {
	if (ft3ID.n <= 16)
	    Msg.L += SerializeUi16(Msg.D + Msg.L, val.getI(0, true));
	else {
	    Msg.L = 0;
	    rc = 0;
	}
    }

    if (Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return rc;
}



B_BTR::B_BTR(TMdPrm& prm, uint16_t id, uint16_t nu, uint16_t nr, bool has_params) :
    DA(prm), ID(id), count_nu(nu), count_nr(nr), with_params(has_params)

{
    mTypeFT3 = GRS;
    TFld * fld;
    if (count_nr)
	blkID = 70;
    else
	blkID = 30;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    if (count_nu) {
	mPrm.p_el.fldAdd(fld = new TFld("selection", _("Select TU"), TFld::Integer, TVal::DirWrite));
	fld->setReserve("0:1");
	mPrm.p_el.fldAdd(fld = new TFld("execution", _("Execution"), TFld::Integer, TVal::DirWrite));
	fld->setReserve("0:2");
    }

    for (int i = 0; i < count_nu; i++)
	AddTUChannel(i);
    for (int i = 0; i < count_nr; i++)
	AddTRChannel(i);
    loadIO(true);
}

B_BTR::~B_BTR()
{

}

void B_BTR::AddTUChannel(uint8_t iid)
{
    TUdata.push_back(STUchannel(iid, this));
    AddAttr(TUdata.back().On.lnk, TFld::Boolean, TVal::DirWrite, "0:1");
    AddAttr(TUdata.back().Off.lnk, TFld::Boolean, TVal::DirWrite, "0:1");
    AddAttr(TUdata.back().Run.lnk, TFld::Boolean, TVal::DirWrite, "0:2");
    AddAttr(TUdata.back().Reset.lnk, TFld::Boolean, TVal::DirWrite, "0:2");
    if (with_params) {
	AddAttr(TUdata.back().Time.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:0", iid + 1));
	AddAttr(TUdata.back().TC.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	AddAttr(TUdata.back().ExTime.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
    }
}

void B_BTR::AddTRChannel(uint8_t iid)
{
    TRdata.push_back(STRchannel(iid, this));
    AddAttr(TRdata.back().Value.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:0", iid + 1 + count_nu));
}

string B_BTR::getStatus(void)
{
    string rez;

    if (NeedInit)
	rez = "20: Опрос";
    else
	rez = "0: Норма";
    return rez;

}

void B_BTR::loadIO(bool force)
{
    if (mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }   //Load/reload IO context only allow for stopped controllers for prevent throws

    for (int i = 0; i < count_nu; i++) {
	loadLnk(TUdata[i].On.lnk);
	loadLnk(TUdata[i].Off.lnk);
	loadLnk(TUdata[i].Run.lnk);
	loadLnk(TUdata[i].Reset.lnk);
	loadLnk(TUdata[i].Time.lnk);
	loadLnk(TUdata[i].TC.lnk);
	loadLnk(TUdata[i].ExTime.lnk);
    }
    for (int i = 0; i < count_nr; i++)
	loadLnk(TRdata[i].Value.lnk);
}

void B_BTR::saveIO()
{
//Save links
    for (int i = 0; i < count_nu; i++) {
	saveLnk(TUdata[i].On.lnk);
	saveLnk(TUdata[i].Off.lnk);
	saveLnk(TUdata[i].Run.lnk);
	saveLnk(TUdata[i].Reset.lnk);
	saveLnk(TUdata[i].Time.lnk);
	saveLnk(TUdata[i].TC.lnk);
	saveLnk(TUdata[i].ExTime.lnk);
    }
    for (int i = 0; i < count_nr; i++)
	saveLnk(TRdata[i].Value.lnk);
}

void B_BTR::tmHandler(void)
{
    NeedInit = false;
    for (int i = 0; i < count_nu; i++) {
	UpdateParamW(TUdata[i].Time, PackID(ID, i + 1, 0), 1);
	UpdateParamW(TUdata[i].TC, PackID(ID, i + 1, 1), 1);
	UpdateParam8(TUdata[i].ExTime, PackID(ID, i + 1, 2), 1);
    }
    for (int i = 0; i < count_nr; i++)
	UpdateParamFl(TRdata[i].Value, PackID(ID, i + 1 + count_nu, 0), 1);
}

uint16_t B_BTR::Task(uint16_t uc)
{
    tagMsg Msg;
    uint16_t rc = 0;

    switch (uc) {
    case TaskRefresh:
	Msg.L = 5;
	Msg.C = AddrReq;
	*((uint16_t*)Msg.D) = PackID(ID, 0, 0);   //состояние
	if (mPrm.owner().DoCmd(&Msg)) {
	    if (Msg.C == GOOD3) {
		if (count_nu) {
		    Msg.L = 7;
		    Msg.C = AddrReq;
		    *((uint16_t*)(Msg.D)) = PackID(ID, 0, 1);           //выбор ТУ
		    *((uint16_t*)(Msg.D + 2)) = PackID(ID, 0, 2);       //исполнение
		    mPrm.owner().DoCmd(&Msg);
		    if (with_params) {
			for (int i = 1; i <= count_nu; i++) {
			    Msg.L = 9;
			    *((uint16_t*)Msg.D) = PackID(ID, i, 0);             //время выдержки
			    *((uint16_t*)(Msg.D + 2)) = PackID(ID, i, 1);       //ТС
			    *((uint16_t*)(Msg.D + 4)) = PackID(ID, i, 2);       //доп время выдержки
			    if (mPrm.owner().DoCmd(&Msg)) {
				if (Msg.C == GOOD3)
				    rc = 1;
				else {
				    rc = 0;
				    break;
				}
			    } else {
				rc = 0;
				break;
			    }

			}
		    }
		}
		if (count_nr) {
		    for (int i = 1; i <= count_nr; i++) {
			Msg.L = 5;
			Msg.C = AddrReq;
			*((uint16_t*)Msg.D) = PackID(ID, i + count_nu, 0);   //уставка
			if (mPrm.owner().DoCmd(&Msg)) {
			    if (Msg.C == GOOD3)
				rc = 1;
			    else {
				rc = 0;
				break;
			    }
			} else {
			    rc = 0;
			    break;
			}
		    }
		} else
		    rc = 1;
	    }
	}
	if (rc) NeedInit = false;
	break;
    }
    return rc;
}
uint16_t B_BTR::HandleEvent(int64_t tm, uint8_t * D)
{
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(D));

    if (ft3ID.g != ID) return 0;
    uint16_t l = 0;
    if (ft3ID.k == 0) {
	switch (ft3ID.n) {
	case 0:
	    mPrm.vlAt("state").at().setI(D[2], tm, true);
	    l = 3;
	    break;
	case 1:
	    mPrm.vlAt(TSYS::strMess("selection")).at().setI(D[3], tm, true);
	    l = 4;
	    break;
	case 2:
	    mPrm.vlAt(TSYS::strMess("execution")).at().setI(D[3], tm, true);
	    l = 4;
	    break;
	}
    }
    if (count_nu && (ft3ID.k <= count_nu)) {
	switch (ft3ID.n) {
	case 0:
	    mPrm.vlAt(TSYS::strMess("time_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 3), tm, true);
	    l = 5;
	    break;

	case 1:
	    if (with_params)
		mPrm.vlAt(TSYS::strMess("tc_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 3), tm, true);
	    l = 5;
	    break;
	case 2:
	    if (with_params) {
		mPrm.vlAt(TSYS::strMess("extime_%d", ft3ID.k)).at().setI(D[3], tm, true);
		;
	    }
	    l = 4;
	    break;

	}
    }
    if (count_nr && ((ft3ID.k > count_nu) && (ft3ID.k <= count_nr + count_nu))) {
	switch (ft3ID.n) {
	case 0:
	    mPrm.vlAt(TSYS::strMess("value_%d", ft3ID.k - count_nu)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
	    l = 7;
	    break;

	}
    }
    return l;
}

uint8_t B_BTR::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);

    if (ft3ID.g != ID) return 0;
    uint l = 0;
    if (ft3ID.k == 0) {
	switch (ft3ID.n) {
	case 0:
	    out[0] = 0 | blkID;
	    l = 1;
	    break;
	case 1:
	    out[0] = 0;
	    out[1] = 0;
	    l = 2;
	    break;
	case 2:
	    out[0] = 0;
	    out[1] = 0;
	    l = 2;
	    break;
	}
    } else {
	if (count_nu && (ft3ID.k <= count_nu)) {
	    switch (ft3ID.n) {
	    case 0:
		out[0] = TUdata[ft3ID.k - 1].Time.s;
		out[1] = TUdata[ft3ID.k - 1].Time.vl;
		out[2] = TUdata[ft3ID.k - 1].Time.vl >> 8;
		l = 3;
		break;
	    case 1:
		out[0] = TUdata[ft3ID.k - 1].TC.s;
		out[1] = TUdata[ft3ID.k - 1].TC.vl;
		out[2] = TUdata[ft3ID.k - 1].TC.vl >> 8;
		l = 3;
		break;
	    case 2:
		out[0] = TUdata[ft3ID.k - 1].ExTime.s;
		out[1] = TUdata[ft3ID.k - 1].ExTime.vl;
		l = 2;
		break;
	    }
	}
	if (count_nr && ((ft3ID.k > count_nu) && (ft3ID.k <= count_nr + count_nu))) {
	    out[0] = TRdata[ft3ID.k - 1 - count_nu].Value.s;
	    for (uint8_t j = 0; j < 4; j++)
		out[1 + j] = TRdata[ft3ID.k - 1 - count_nu].Value.b_vl[j];
	    l = 5;
	}
    }
    return l;
}

uint8_t B_BTR::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);

    if (ft3ID.g != ID) return 0;
    uint l = 0;
//    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "cmdSet k %d n %d", ft3ID.k, ft3ID.n);
    if (ft3ID.k == 0) {
	switch (ft3ID.n) {
	case 1:
	    setTU(req[2]);
	    l = 3;
	    break;
	case 2:
	    runTU(req[2]);
	    l = 3;
	    break;
	}
    } else {
	if (count_nu && (ft3ID.k <= count_nu)) {
	    switch (ft3ID.n) {
	    case 0:
		l = SetNewWVal(TUdata[ft3ID.k - 1].Time, addr, prmID, TSYS::getUnalign16(req + 2));
		break;
	    case 1:
		l = SetNewWVal(TUdata[ft3ID.k - 1].TC, addr, prmID, TSYS::getUnalign16(req + 2));
		break;
	    case 2:
		l = SetNew8Val(TUdata[ft3ID.k - 1].ExTime, addr, prmID, req[2]);
		break;
	    }
	}
	if (count_nr && ((ft3ID.k > count_nu) && (ft3ID.k <= count_nr + count_nu)))
//	    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "cmdSet Val");
	    l = SetNewflVal(TRdata[ft3ID.k - 1 - count_nu].Value, addr, prmID, TSYS::getUnalignFloat(req + 2));
    }
    return l;
}

void B_BTR::setTU(uint8_t tu)
{
    uint8_t vl = tu >> 7;
    uint8_t n = tu & 0x7F;

    if ((n > 0) && (n <= count_nu)) {
	STUchannel &TU = TUdata[n - 1];
	currTU = n;
	if (vl) {
	    if (TU.On.lnk.Connected()) {
		//on
		TU.On.lnk.aprm.at().setI(1);
		mPrm.vlAt(TU.On.lnk.prmName).at().setI(1, 0, true);
	    }
	} else {
	    if (TUdata[n - 1].Off.lnk.Connected()) {
		//off
		TU.Off.lnk.aprm.at().setI(1);
		mPrm.vlAt(TU.Off.lnk.prmName).at().setI(1, 0, true);
	    }
	}
    }

}
uint8_t B_BTR::runTU(uint8_t tu)
{
    if (currTU) {
	STUchannel &TU = TUdata[currTU - 1];
	if ((tu == 0x55) && (TU.Run.lnk.Connected())) {
	    TU.Run.lnk.aprm.at().setI(1);
	    mPrm.vlAt(TU.Run.lnk.prmName).at().setI(1, 0, true);
	    currTU = 0;
	}
    }
    if (tu == 0x0) {
	for (int i = 0; i < count_nu; i++) {
	    STUchannel & TU = TUdata[i];
	    if ((TU.Reset.lnk.Connected())) {
		TU.Reset.lnk.aprm.at().setI(1);
		mPrm.vlAt(TU.Reset.lnk.prmName).at().setI(1, 0, true);
	    }
	}
	currTU = 0;
    }
    return 0;
}

uint16_t B_BTR::setVal(TVal &val)
{
    int off = 0;
    FT3ID ft3ID;

    ft3ID.k = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.n = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.g = ID;

    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
    uint8_t run;
    if (ft3ID.k == 0) {
	switch (ft3ID.n) {
	case 1:
	    Msg.L += SerializeB(Msg.D + Msg.L, val.getI(0, true));
	    break;
	case 2:
	    run = val.getI(0, true);
	    if (run == 170)
		Msg.L += SerializeB(Msg.D + Msg.L, 0);
	    else {
		if (run == 85)
		    Msg.L += SerializeB(Msg.D + Msg.L, run);
		else
		    Msg.L = 0;
	    }
	    break;
	}
    } else {
	if (count_nu && (ft3ID.k <= count_nu)) {
	    switch (ft3ID.n) {
	    case 0:
		Msg.L += SerializeUi16(Msg.D + Msg.L, val.getI(0, true));
		break;
	    case 1:
		Msg.L += SerializeUi16(Msg.D + Msg.L, val.getI(0, true));
		break;
	    case 2:
		Msg.L += SerializeB(Msg.D + Msg.L, val.getI(0, true));
		break;
	    }
	}
	if (count_nr && ((ft3ID.k > count_nu) && (ft3ID.k <= count_nr + count_nu))) {
	    switch (ft3ID.n) {
	    case 0:
		Msg.L += SerializeF(Msg.D + Msg.L, val.getR(0, true));
		break;
	    }
	}
    }
    if (Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return 0;
}

//---------------------------------------------------------------------------
