//OpenSCADA system module DAQ.MSO file: MezTI.cpp
/***************************************************************************
 *   Copyright (C) 2014-2015 by Maxim Kochetkov                            *
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

#include "mso_daq.h"
#include "MezTI.h"

using namespace MSO;
//*************************************************
//* MezTI                                           *
//*************************************************

MezTI::MezTI( TMdPrm *prm, uint16_t id ) : DA(prm), ID(id)
{
	TFld * fld;
	for (int i = 1; i <= 4; i++)	{
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("count_%d",i).c_str(),TSYS::strMess(_("Count %d"),i).c_str(),TFld::Integer,TVal::DirWrite));
		fld->setReserve(TSYS::strMess("12:%d:0",i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("value_%d",i).c_str(),TSYS::strMess(_("Value %d"),i).c_str(),TFld::Real,TVal::DirWrite));
		fld->setReserve(TSYS::strMess("12:%d:1",i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("coeff_%d",i).c_str(),TSYS::strMess(_("Coeff TI %d"),i).c_str(),TFld::Real,TVal::DirWrite));
		fld->setReserve(TSYS::strMess("12:%d:2",i + ID * 4));
	}
}

MezTI::~MezTI( )
{

}

uint16_t MezTI::Refresh()
{
	string pdu;
    bool bInit = false;
	for (int i=0;i<4;i++){
		if (mPrm->vlAt(TSYS::strMess("count_%d", i+1).c_str()).at().getI(0, true) == EVAL_INT){
            bInit = true;
			if (!mPrm->owner().MSOReq(i + ID * 4, 12, 0, pdu))
				return false;
		}
		if (mPrm->vlAt(TSYS::strMess("value_%d", i+1).c_str()).at().getR(0, true) == EVAL_REAL){
            bInit = true;
			if (!mPrm->owner().MSOReq(i + ID * 4, 12, 1, pdu))
				return false;
		}
		if (mPrm->vlAt(TSYS::strMess("coeff_%d", i+1).c_str()).at().getR(0, true) == EVAL_REAL){
            bInit = true;
			if (!mPrm->owner().MSOReq(i + ID * 4, 12, 2, pdu))
				return false;
		}
	}
	NeedInit = bInit;
	return true;
}

string  MezTI::getStatus(void )
{
	string rez;
	rez = _("0: Normal.");
	return rez;

}

uint16_t MezTI::Task(uint16_t uc)
{
	if (NeedInit) {
		Refresh();
	}
	return 0;
}

uint16_t MezTI::HandleEvent(unsigned int channel, unsigned int type, unsigned int param, unsigned int flag, const string &ireqst)
{
    uint16_t rc = 1;
    if (channel / 4 != ID)
        return rc = 0;
    switch (type) {
        case 12:
            switch (param) {
                case 0:
                    mPrm->vlAt(TSYS::strMess("count_%d", channel % 4 + 1).c_str()).at().setI(TSYS::getUnalign32(ireqst.data()), 0, true);
                    break;
                case 1:
                    mPrm->vlAt(TSYS::strMess("value_%d", channel % 4 + 1).c_str()).at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
                    break;
                case 2:
                    mPrm->vlAt(TSYS::strMess("coeff_%d", channel % 4 + 1).c_str()).at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
                    break;
                default:
                    rc = 0;
            }
            break;
        default:
            rc = 0;
    }
    return rc;
}

uint16_t MezTI::setVal(TVal &val)
{
	int off = 0;
	uint16_t type = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(),NULL,0); // тип данных
	uint16_t channel = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(),NULL,0); // канал
	uint16_t param = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(),NULL,0); // параметр

	mess_info(mPrm->nodePath().c_str(),_("setVal: type %d channel %d param %d"),type,channel,param);
	string pdu;
	uint8_t f[4];

	switch (type){
		case 12:
			switch (param){
				case 0:
					*(uint32_t *)(f)= (uint32_t)val.get(NULL,true).getI();
					pdu = f[0];
					pdu += f[1];
					pdu += f[2];
					pdu += f[3];
					mPrm->owner().MSOSet(channel-1, type, param, pdu);
					break;
				case 2:
					*(float *)(f)= (float)val.get(NULL,true).getR();
					pdu = f[0];
					pdu += f[1];
					pdu += f[2];
					pdu += f[3];
					mPrm->owner().MSOSet(channel-1, type, param, pdu);
					break;
			}
			break;
	}
	return 0;
}

