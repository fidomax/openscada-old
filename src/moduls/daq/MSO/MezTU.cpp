//OpenSCADA system module DAQ.MSO file: MezTU.cpp
/***************************************************************************
 *   Copyright (C) 2012-2015 by Maxim Kochetkov                            *
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
#include "MezTU.h"

using namespace MSO;
//*************************************************
//* MezTU                                           *
//*************************************************

MezTU::MezTU(TMdPrm *prm, uint16_t id) :
		DA(prm), ID(id)
{
	TFld * fld;
	for (int i = 1; i <= 4; i++) {
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("value_%d", i).c_str(), TSYS::strMess(_("Value %d"), i).c_str(), TFld::Boolean, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("8:%d:0", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("timeTU_%d", i).c_str(), TSYS::strMess(_("TimeTU %d"), i).c_str(), TFld::Integer, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("19:%d:2:0", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("timeTUs_%d", i).c_str(), TSYS::strMess(_("TimeTU (s) %d"), i).c_str(), TFld::Integer, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("19:%d:2:1", i + ID * 4));
	}
}

MezTU::~MezTU()
{

}

uint16_t MezTU::Refresh()
{
	string pdu;
    bool bInit = false;
	for (int i=0;i<4;i++){
		if (mPrm->vlAt(TSYS::strMess("value_%d", i+1).c_str()).at().getB(0, true) == EVAL_BOOL){
            bInit = true;
			if (!mPrm->owner().MSOReq(i + ID * 4, 8, 0, pdu))
				return false;
		}
		if (mPrm->vlAt(TSYS::strMess("timeTU_%d", i+1).c_str()).at().getI(0, true) == EVAL_INT){
            bInit = true;
			if (!mPrm->owner().MSOReq(i + ID * 4, 19, 2, pdu))
				return false;
		}
	}
	NeedInit = bInit;
	return true;
}

string MezTU::getStatus(void)
{
	string rez;
	rez = _("0: Normal.");
	return rez;

}

uint16_t MezTU::Task(uint16_t uc)
{
	if (NeedInit) {
		Refresh();
	}
	return 0;
}

uint16_t MezTU::HandleEvent(unsigned int channel, unsigned int type, unsigned int param, unsigned int flag, const string &ireqst)
{
    uint16_t rc = 1;
    if (channel / 4 != ID)
        return rc = 0;
    switch (type) {
        case 8:
            switch (param) {
                case 0:
                    switch (channel % 4) {
                        case 0:
                            mPrm->vlAt("value_1").at().setB(TSYS::getUnalign32(ireqst.data()), 0, true);
                            break;
                        case 1:
                            mPrm->vlAt("value_2").at().setB(TSYS::getUnalign32(ireqst.data()), 0, true);
                            break;
                        case 2:
                            mPrm->vlAt("value_3").at().setB(TSYS::getUnalign32(ireqst.data()), 0, true);
                            break;
                        case 3:
                            mPrm->vlAt("value_4").at().setB(TSYS::getUnalign32(ireqst.data()), 0, true);
                            break;
                        default:
                            rc = 0;
                    }
                    break;
                default:
                    rc = 0;
            }
            break;
        case 19:
            switch (param) {
                case 2:
                    switch (channel % 4) {
                        case 0:
                            mPrm->vlAt("timeTU_1").at().setI(TSYS::getUnalign32(ireqst.data()), 0, true);
                            mPrm->vlAt("timeTUs_1").at().setI(TSYS::getUnalign32(ireqst.data()) / 1000, 0, true);
                            break;
                        case 1:
                            mPrm->vlAt("timeTU_2").at().setI(TSYS::getUnalign32(ireqst.data()), 0, true);
                            mPrm->vlAt("timeTUs_2").at().setI(TSYS::getUnalign32(ireqst.data()) / 1000, 0, true);
                            break;
                        case 2:
                            mPrm->vlAt("timeTU_3").at().setI(TSYS::getUnalign32(ireqst.data()), 0, true);
                            mPrm->vlAt("timeTUs_3").at().setI(TSYS::getUnalign32(ireqst.data()) / 1000, 0, true);
                            break;
                        case 3:
                            mPrm->vlAt("timeTU_4").at().setI(TSYS::getUnalign32(ireqst.data()), 0, true);
                            mPrm->vlAt("timeTUs_4").at().setI(TSYS::getUnalign32(ireqst.data()) / 1000, 0, true);
                            break;
                        default:
                            rc = 0;
                    }
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

uint16_t MezTU::setVal(TVal &val)
{
	int off = 0;
	uint16_t type = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // тип данных
	uint16_t channel = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // канал
	uint16_t param = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // параметр
	uint16_t subparam = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // параметр
	mess_info(mPrm->nodePath().c_str(), _("setVal: type %d channel %d param %d"), type, channel, param);
	string pdu;

	uint8_t f[4];
	switch (type) {
		case 8:
			switch (param) {
				case 0:
					*(uint32_t *) (f) = (uint32_t) val.get(NULL, true).getB();
					pdu = f[0];
					pdu += f[1];
					pdu += f[2];
					pdu += f[3];
					mPrm->owner().MSOSet(channel - 1, type, param, pdu);
					break;
			}
			break;
		case 19:
			switch (param) {
				case 2:
					if (subparam == 0){
						*(uint32_t *) (f) = (uint32_t) val.get(NULL, true).getI();
					} else {
						*(uint32_t *) (f) = ((uint32_t) val.get(NULL, true).getI()) * 1000;
					}
					pdu = f[0];
					pdu += f[1];
					pdu += f[2];
					pdu += f[3];
					mPrm->owner().MSOSet(channel - 1, type, param, pdu);
					break;
			}
			break;
	}
	return 0;
}

