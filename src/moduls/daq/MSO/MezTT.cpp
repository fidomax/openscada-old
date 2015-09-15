//OpenSCADA system module DAQ.MSO file: MezTT.cpp
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
#include "MezTT.h"

using namespace MSO;
//*************************************************
//* MezTT                                           *
//*************************************************

MezTT::MezTT(TMdPrm *prm, uint16_t id) :
		DA(prm), ID(id)
{
	TFld * fld;
	for (int i = 1; i <= 4; i++) {
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("state_%d", i).c_str(), TSYS::strMess(_("State %d"), i).c_str(), TFld::Integer, TFld::NoWrite));
		fld->setReserve(TSYS::strMess("11:%d:0", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("value_%d", i).c_str(), TSYS::strMess(_("Value %d"), i).c_str(), TFld::Real, TFld::NoWrite));
		fld->setReserve(TSYS::strMess("11:%d:0", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("mode_%d", i).c_str(), TSYS::strMess(_("Mode %d"), i).c_str(), TFld::Integer, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("17:%d:1", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("period_%d", i).c_str(), TSYS::strMess(_("Measure period %d"), i).c_str(), TFld::Real, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("17:%d:2", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("sensor_min_%d", i).c_str(), TSYS::strMess(_("Sensor min %d"), i).c_str(), TFld::Real, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("17:%d:3", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("sensor_max_%d", i).c_str(), TSYS::strMess(_("Sensor max %d"), i).c_str(), TFld::Real, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("17:%d:4", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("PV_min_%d", i).c_str(), TSYS::strMess(_("PV min %d"), i).c_str(), TFld::Real, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("17:%d:5", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("PV_max_%d", i).c_str(), TSYS::strMess(_("PV max %d"), i).c_str(), TFld::Real, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("17:%d:6", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("coeff_%d", i).c_str(), TSYS::strMess(_("CoeffExt %d"), i).c_str(), TFld::Real, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("17:%d:7", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("save_params_%d", i).c_str(), TSYS::strMess(_("Save params %d"), i).c_str(), TFld::Boolean, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("17:%d:0", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("calibrate_0_%d", i).c_str(), TSYS::strMess(_("Calibrate 0 %d"), i).c_str(), TFld::Boolean, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("16:%d:5", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("calibrate_20_%d", i).c_str(), TSYS::strMess(_("Calibrate 20 %d"), i).c_str(), TFld::Boolean, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("16:%d:6", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("stop_calibrate_%d", i).c_str(), TSYS::strMess(_("Stop calibration %d"), i).c_str(), TFld::Boolean,TVal::DirWrite));
		fld->setReserve(TSYS::strMess("16:%d:7", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("save_calibrate_%d", i).c_str(), TSYS::strMess(_("Save calibration %d"), i).c_str(), TFld::Boolean,TVal::DirWrite));
		fld->setReserve(TSYS::strMess("16:%d:0", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("warn_min_%d", i).c_str(), TSYS::strMess(_("Warning min %d"), i).c_str(), TFld::Real, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("15:%d:1", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("warn_max_%d", i).c_str(), TSYS::strMess(_("Warning max %d"), i).c_str(), TFld::Real, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("15:%d:2", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("alarm_min_%d", i).c_str(), TSYS::strMess(_("Alarm min %d"), i).c_str(), TFld::Real, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("15:%d:3", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("alarm_max_%d", i).c_str(), TSYS::strMess(_("Alarm max %d"), i).c_str(), TFld::Real, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("15:%d:4", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("sens_%d", i).c_str(), TSYS::strMess(_("Sensitivity %d"), i).c_str(), TFld::Real, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("15:%d:5", i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("save_alarms_%d", i).c_str(), TSYS::strMess(_("Save alarms %d"), i).c_str(), TFld::Boolean, TVal::DirWrite));
		fld->setReserve(TSYS::strMess("15:%d:0", i + ID * 4));
	}
}

MezTT::~MezTT()
{

}

uint16_t MezTT::Refresh()
{

    string pdu;
    bool bInit = false;
    for (int i = 0; i < 4; i++) {
        if (mPrm->vlAt(TSYS::strMess("state_%d", i + 1).c_str()).at().getI(0, true) == EVAL_INT) {
            bInit = true;
            if (!mPrm->owner().MSOReq(i + ID * 4, 11, 0, pdu))
                return false;
        }
        if (mPrm->vlAt(TSYS::strMess("warn_min_%d", i + 1).c_str()).at().getR(0, true) == EVAL_REAL) {
            bInit = true;
            if (!mPrm->owner().MSOReq(i + ID * 4, 15, 1, pdu))
                return false;
        }
        if (mPrm->vlAt(TSYS::strMess("warn_max_%d", i + 1).c_str()).at().getR(0, true) == EVAL_REAL) {
            bInit = true;
            if (!mPrm->owner().MSOReq(i + ID * 4, 15, 2, pdu))
                return false;
        }
        if (mPrm->vlAt(TSYS::strMess("alarm_min_%d", i + 1).c_str()).at().getR(0, true) == EVAL_REAL) {
            bInit = true;
            if (!mPrm->owner().MSOReq(i + ID * 4, 15, 3, pdu))
                return false;
        }
        if (mPrm->vlAt(TSYS::strMess("alarm_max_%d", i + 1).c_str()).at().getR(0, true) == EVAL_REAL) {
            bInit = true;
            if (!mPrm->owner().MSOReq(i + ID * 4, 15, 4, pdu))
                return false;
        }
        if (mPrm->vlAt(TSYS::strMess("sens_%d", i + 1).c_str()).at().getR(0, true) == EVAL_REAL) {
            bInit = true;
            if (!mPrm->owner().MSOReq(i + ID * 4, 15, 5, pdu))
                return false;
        }
        if (mPrm->vlAt(TSYS::strMess("mode_%d", i + 1).c_str()).at().getI(0, true) == EVAL_INT) {
            bInit = true;
            if (!mPrm->owner().MSOReq(i + ID * 4, 17, 1, pdu))
                return false;
        }
        if (mPrm->vlAt(TSYS::strMess("period_%d", i + 1).c_str()).at().getR(0, true) == EVAL_REAL) {
            bInit = true;
            if (!mPrm->owner().MSOReq(i + ID * 4, 17, 2, pdu))
                return false;
        }
        if (mPrm->vlAt(TSYS::strMess("sensor_min_%d", i + 1).c_str()).at().getR(0, true) == EVAL_REAL) {
            bInit = true;
            if (!mPrm->owner().MSOReq(i + ID * 4, 17, 3, pdu))
                return false;
        }
        if (mPrm->vlAt(TSYS::strMess("sensor_max_%d", i + 1).c_str()).at().getR(0, true) == EVAL_REAL) {
            bInit = true;
            if (!mPrm->owner().MSOReq(i + ID * 4, 17, 4, pdu))
                return false;
        }
        if (mPrm->vlAt(TSYS::strMess("PV_min_%d", i + 1).c_str()).at().getR(0, true) == EVAL_REAL) {
            bInit = true;
            if (!mPrm->owner().MSOReq(i + ID * 4, 17, 5, pdu))
                return false;
        }
        if (mPrm->vlAt(TSYS::strMess("PV_max_%d", i + 1).c_str()).at().getR(0, true) == EVAL_REAL) {
            bInit = true;
            if (!mPrm->owner().MSOReq(i + ID * 4, 17, 6, pdu))
                return false;
        }
        if (mPrm->vlAt(TSYS::strMess("coeff_%d", i + 1).c_str()).at().getR(0, true) == EVAL_REAL) {
            bInit = true;
            if (!mPrm->owner().MSOReq(i + ID * 4, 17, 7, pdu))
                return false;
        }
    }
    NeedInit = bInit;
    return true;

}

string MezTT::getStatus(void)
{
	string rez;
	rez = _("0: Normal.");
	return rez;

}

uint16_t MezTT::Task(uint16_t uc)
{
	if (NeedInit) {
		Refresh();
	}
	return 0;
}

uint16_t MezTT::HandleEvent(unsigned int channel, unsigned int type, unsigned int param, unsigned int flag, const string &ireqst)
{
	uint16_t rc = 1;
	if (channel / 4 != ID)
		return rc = 0;
	switch (type) {
		case 11:
			switch (channel % 4) {
				case 0:
					mPrm->vlAt("value_1").at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
					mPrm->vlAt("state_1").at().setI(TSYS::getUnalign32(ireqst.data() + 4), 0, true);
					break;
				case 1:
					mPrm->vlAt("value_2").at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
					mPrm->vlAt("state_2").at().setI(TSYS::getUnalign32(ireqst.data() + 4), 0, true);
					break;
				case 2:
					mPrm->vlAt("value_3").at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
					mPrm->vlAt("state_3").at().setI(TSYS::getUnalign32(ireqst.data() + 4), 0, true);
					break;
				case 3:
					mPrm->vlAt("value_4").at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
					mPrm->vlAt("state_4").at().setI(TSYS::getUnalign32(ireqst.data() + 4), 0, true);
					break;
				default:
					rc = 0;
			}
			break;
		case 15:
			switch (param) {
				case 1:
					mPrm->vlAt(TSYS::strMess("warn_min_%d", channel % 4 + 1).c_str()).at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
					break;
				case 2:
					mPrm->vlAt(TSYS::strMess("warn_max_%d", channel % 4 + 1).c_str()).at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
					break;
				case 3:
					mPrm->vlAt(TSYS::strMess("alarm_min_%d", channel % 4 + 1).c_str()).at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
					break;
				case 4:
					mPrm->vlAt(TSYS::strMess("alarm_max_%d", channel % 4 + 1).c_str()).at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
					break;
				case 5:
					mPrm->vlAt(TSYS::strMess("sens_%d", channel % 4 + 1).c_str()).at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
					break;
				default:
					rc = 0;
			}
			break;
		case 17:
			switch (param) {
				case 1:
					mPrm->vlAt(TSYS::strMess("mode_%d", channel % 4 + 1).c_str()).at().setI(TSYS::getUnalign32(ireqst.data()), 0, true);
					break;
				case 2:
					mPrm->vlAt(TSYS::strMess("period_%d", channel % 4 + 1).c_str()).at().setI(TSYS::getUnalign32(ireqst.data()), 0, true);
					break;
				case 3:
					mPrm->vlAt(TSYS::strMess("sensor_min_%d", channel % 4 + 1).c_str()).at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
					break;
				case 4:
					mPrm->vlAt(TSYS::strMess("sensor_max_%d", channel % 4 + 1).c_str()).at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
					break;
				case 5:
					mPrm->vlAt(TSYS::strMess("PV_min_%d", channel % 4 + 1).c_str()).at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
					break;
				case 6:
					mPrm->vlAt(TSYS::strMess("PV_max_%d", channel % 4 + 1).c_str()).at().setR(TSYS::getUnalignFloat(ireqst.data()), 0, true);
					break;
				case 7:
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

uint16_t MezTT::setVal(TVal &val)
{
	int off = 0;
	uint16_t type = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // тип данных
	uint16_t channel = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // канал
	uint16_t param = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(), NULL, 0); // параметр

	mess_info(mPrm->nodePath().c_str(), _("setVal: type %d channel %d param %d"), type, channel, param);
	string pdu;
	uint8_t f[4];
	switch (type) {
		case 17:
			switch (param) {
				case 0:
					mPrm->owner().MSOSet(channel - 1, type, param, pdu);
					break;
				case 1:
				case 2:
					*(uint32_t *) (f) = (uint32_t) val.get(NULL, true).getI();
					pdu = f[0];
					pdu += f[1];
					pdu += f[2];
					pdu += f[3];
					mPrm->owner().MSOSet(channel - 1, type, param, pdu);
					break;
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
					*(float *) (f) = (float) val.get(NULL, true).getR();
					pdu = f[0];
					pdu += f[1];
					pdu += f[2];
					pdu += f[3];
					mPrm->owner().MSOSet(channel - 1, type, param, pdu);
					break;

			}
			break;
		case 15:
			switch (param) {
				case 0:
					mPrm->owner().MSOSet(channel - 1, type, param, pdu);
					break;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					*(float *) (f) = (float) val.get(NULL, true).getR();
					pdu = f[0];
					pdu += f[1];
					pdu += f[2];
					pdu += f[3];
					mPrm->owner().MSOSet(channel - 1, type, param, pdu);
					break;
			}
			break;
		case 16:
			switch (param) {
				case 0:
				case 5:
				case 6:
				case 7:
					mPrm->owner().MSOSetV(channel - 1, type, param, pdu);
					break;

			}
			break;
	}
	return 0;
}

