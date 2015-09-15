//OpenSCADA system module DAQ.MSO file: MSOparam.cpp
/***************************************************************************
 *   Copyright (C) 2015 by Maxim Kochetkov                            *
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
#include "MSOparam.h"

using namespace MSO;
//*************************************************
//* MSOparam                                    *
//*************************************************

MSOparam::MSOparam( TMdPrm *prm, uint16_t id ) : DA(prm), ID(id)
{
	TFld * fld;
	mPrm->p_el.fldAdd(fld = new TFld(TSYS::strMess("communication").c_str(),TSYS::strMess(_("Communication")).c_str(),TFld::Integer,TFld::NoWrite));
}

MSOparam::~MSOparam( )
{

}

uint16_t MSOparam::Refresh()
{
    mPrm->vlAt("communication").at().setI(mPrm->owner().communication,0,true);
	return true;
}

string  MSOparam::getStatus(void )
{
	string rez;
	rez = _("0: Normal.");
	return rez;

}

uint16_t MSOparam::Task(uint16_t uc)
{
	Refresh();
	return 0;
}

