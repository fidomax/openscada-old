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
#ifndef DA_GNS_H
#define DA_GNS_H

#include "da.h"

namespace FT3
{

enum eKA_GNS_NAS {
	NAS_VAG = 0,    // неопределено
	NAS_OFF = 1,    // выкл.
	NAS_ON = 2,     // вкл.
	NAS_REP = 3,    // ремонт
	NAS_AWR = 4     // авария
};
enum eKA_GNS_State {
	KA_GNS_Error = 0x0, KA_GNS_Normal = 0x1
};
struct KANSTUParams      // FT3 ID
{
	uint16_t TUOn;
	uint16_t TimeOn;
	uint16_t TUOff;
	uint16_t TimeOff;
	uint16_t TUStop;
	uint16_t TimeStop;
	uint16_t TURemote;
	uint16_t TimeRemote;
	uint16_t TUManual;
	uint16_t TimeManual;
} __attribute__((packed));
struct KANSTCParams      // FT3 ID
{
	uint16_t TCOn;
	uint16_t TCOff;
	uint16_t TCMode;
} __attribute__((packed));


class KA_GNS : public DA
{
public:
//Methods
KA_GNS(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params);
~KA_GNS();
uint16_t count_n;
bool with_params;
uint16_t max_count_data;
uint16_t GetState(void);
uint16_t HandleEvent(int64_t, uint8_t *);
uint8_t cmdGet(uint16_t prmID, uint8_t * out);
uint8_t cmdSet(uint8_t * req, uint8_t addr);
uint16_t setVal(TVal &val);
string getStatus(void);
uint16_t config;
};
class KA_NS : public DA
{
public:
KA_NS(TMdPrm& prm, DA &parent, uint16_t id, bool has_params);
~KA_NS();
bool with_params;
DA &parentDA;
uint16_t SetParams(void);
uint16_t RefreshParams(void);
uint16_t RefreshData(void);
uint16_t HandleEvent(int64_t, uint8_t *);
uint8_t cmdGet(uint16_t prmID, uint8_t * out);
uint8_t cmdSet(uint8_t * req, uint8_t addr);
uint16_t setVal(TVal &val);
void saveIO(void);
void loadIO(bool force = false);
void saveParam(void);
void loadParam(void);
void tmHandler(void);

ui8Data State, Function;
ui16Data TUOn, TUOff, TUStop, TURemote, TUManual;
ui16Data TimeOn, TimeOff, TimeStop, TimeRemote, TimeManual;
ui16Data TCOn, TCOff, TCMode;
ui32Data Time;
int lnkSize()
{
	if (with_params)
		return 16;
	else
		return 2;
}
int lnkId(const string &id)
{
	if (State.lnk.prmName == id) return 0;
	if (Function.lnk.prmName == id) return 1;
	if (with_params) {
		if (TUOn.lnk.prmName == id) return 2;
		if (TUOff.lnk.prmName == id) return 3;
		if (TUStop.lnk.prmName == id) return 4;
		if (TURemote.lnk.prmName == id) return 5;
		if (TUManual.lnk.prmName == id) return 6;
		if (TimeOn.lnk.prmName == id) return 7;
		if (TimeOff.lnk.prmName == id) return 8;
		if (TimeStop.lnk.prmName == id) return 9;
		if (TimeRemote.lnk.prmName == id) return 10;
		if (TimeManual.lnk.prmName == id) return 11;
		if (TCOn.lnk.prmName == id) return 12;
		if (TCOff.lnk.prmName == id) return 13;
		if (TCMode.lnk.prmName == id) return 14;
		if (Time.lnk.prmName == id) return 15;
	}
	return -1;
}
SLnk &lnk(int num)
{
	switch (num) {
	case 0:
		return State.lnk;
	case 1:
		return Function.lnk;
	case 2:
		return TUOn.lnk;
	case 3:
		return TUOff.lnk;
	case 4:
		return TUStop.lnk;
	case 5:
		return TURemote.lnk;
	case 6:
		return TUManual.lnk;
	case 7:
		return TimeOn.lnk;
	case 8:
		return TimeOff.lnk;
	case 9:
		return TimeStop.lnk;
	case 10:
		return TimeRemote.lnk;
	case 11:
		return TimeManual.lnk;
	case 12:
		return TCOn.lnk;
	case 13:
		return TCOff.lnk;
	case 14:
		return TCMode.lnk;
	case 15:
		return Time.lnk;
	}
}

protected:
void UpdateState(uint16_t ID, uint8_t cl);
void UpdateTUParam(uint16_t ID, uint8_t cl);
void UpdateTCParam(uint16_t ID, uint8_t cl);
void UpdateTime(uint16_t ID, uint8_t cl);
bool IsTUParamChanged();
bool IsTCParamChanged();
uint8_t SetNewTUParam(uint8_t addr, uint16_t prmID, uint8_t *val);
uint8_t SetNewTCParam(uint8_t addr, uint16_t prmID, uint8_t *val);
uint8_t SetNewState(uint8_t addr, uint16_t prmID, uint8_t *val);
uint8_t SetNewFunction(uint8_t addr, uint16_t prmID, uint8_t *val);
};
} //End namespace

#endif //DA_GNS_H
