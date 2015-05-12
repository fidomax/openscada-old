//OpenSCADA system module DAQ.MSO file: MezTC.cpp
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
#include "MezTC.h"

using namespace MSO;
//*************************************************
//* MezTC                                           *
//*************************************************

MezTC::MezTC( TMdPrm *prm, uint16_t id ) : DA(prm), ID(id)
{
	TFld * fld;
	for (int i = 1; i <= 4; i++)	{
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("state_%d",i).c_str(),TSYS::strMess(_("State %d"),i).c_str(),TFld::Integer,TFld::NoWrite));
	    fld->setReserve(TSYS::strMess("10:%d:0",i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("value_%d",i).c_str(),TSYS::strMess(_("Value %d"),i).c_str(),TFld::Integer,TFld::NoWrite));
	    fld->setReserve(TSYS::strMess("10:%d:0",i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("mode_%d",i).c_str(),TSYS::strMess(_("Mode %d"),i).c_str(),TFld::Integer,TVal::DirWrite));
		fld->setReserve(TSYS::strMess("18:%d:1",i + ID * 4));
		mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("save_params_%d",i).c_str(),TSYS::strMess(_("Save params %d"),i).c_str(),TFld::Boolean,TVal::DirWrite));
		fld->setReserve(TSYS::strMess("18:%d:0",i + ID * 4));
	}
}

MezTC::~MezTC( )
{

}

uint16_t MezTC::Refresh()
{
	string pdu;
    bool bInit = false;
	for (int i=0;i<4;i++){
		if (mPrm->vlAt(TSYS::strMess("value_%d", i + 1).c_str()).at().getI(0, true) == EVAL_INT){
            bInit = true;
			if (!mPrm->owner().MSOReq(i + ID * 4, 10, 0, pdu))
				return false;
		}
		if (mPrm->vlAt(TSYS::strMess("mode_%d", i + 1).c_str()).at().getI(0, true) == EVAL_INT){
            bInit = true;
			if (!mPrm->owner().MSOReq(i + ID * 4, 18, 1, pdu))
				return false;
		}
	}
	NeedInit = bInit;
	return true;
}

string  MezTC::getStatus(void )
{
	string rez;
	rez = _("0: Normal.");
	return rez;

}

uint16_t MezTC::Task(uint16_t uc)
{
	if (NeedInit) {
		Refresh();
	}
	return 0;
}

uint16_t MezTC::HandleEvent(unsigned int channel,unsigned int type,unsigned int param,unsigned int flag,const string &ireqst)
{
    uint16_t rc = 1;
    if (channel / 4 != ID)
        return rc = 0;
	switch (type){
		case 10:
			switch (channel % 4) {
				case 0:
					mPrm->vlAt("value_1").at().setI(TSYS::getUnalign32(ireqst.data()),0,true);
					mPrm->vlAt("state_1").at().setI(TSYS::getUnalign32(ireqst.data()+4),0,true);
		            break;
				case 1:
					mPrm->vlAt("value_2").at().setI(TSYS::getUnalign32(ireqst.data()),0,true);
					mPrm->vlAt("state_2").at().setI(TSYS::getUnalign32(ireqst.data()+4),0,true);
		            break;
				case 2:
					mPrm->vlAt("value_3").at().setI(TSYS::getUnalign32(ireqst.data()),0,true);
					mPrm->vlAt("state_3").at().setI(TSYS::getUnalign32(ireqst.data()+4),0,true);
		             break;
				case 3:
					mPrm->vlAt("value_4").at().setI(TSYS::getUnalign32(ireqst.data()),0,true);
					mPrm->vlAt("state_4").at().setI(TSYS::getUnalign32(ireqst.data()+4),0,true);
		            break;
                default:
                    rc = 0;
			}
			break;
		case 18:
			switch (param){
				case 1:
					mPrm->vlAt(TSYS::strMess("mode_%d",channel % 4 + 1).c_str()).at().setI(TSYS::getUnalign32(ireqst.data()),0,true);
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

uint16_t MezTC::setVal(TVal &val)
{
	int off = 0;
	uint16_t type = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(),NULL,0); // тип данных
	uint16_t channel = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(),NULL,0); // канал
	uint16_t param = strtol((TSYS::strParse(val.fld().reserve(), 0, ":", &off)).c_str(),NULL,0); // параметр

	mess_info(mPrm->nodePath().c_str(),_("setVal: type %d channel %d param %d"),type,channel,param);
	string pdu;
	uint8_t f[4];
	switch (type){
		case 18:
			switch (param){
				case 0:
					mPrm->owner().MSOSet(channel-1, type, param, pdu);
					break;
				case 1:
					*(uint32_t *)(f)= (uint32_t)val.get(NULL,true).getI();
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

