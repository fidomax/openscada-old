/***************************************************************************
*   Copyright (C) 2011-2017 by Maxim Kochetkov                            *
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
#ifndef DA_SUB_H
#define DA_SUB_H

#include "da.h"

namespace FT3
{

enum eKA_SUB_MA_State {
    MA_ST_UNDEF		= 0,    // неопределено
    MA_ST_OFF		= 1,    // выкл.
    MA_ST_ON		= 2,    // вкл.
    MA_ST_NREADY	= 3,    // не готов
    MA_ST_REPAIR	= 4     // ремонт
};
enum eKA_SUB_MA_Mode {
    MA_MD_AUTO		= 0,    // автомат
    MA_MD_RESERVE	= 1,    // резерв
    MA_MD_TEST		= 2     // тест
};
enum eKA_SUB_MA_Function {
    MA_FN_NONE			= 0,    //
    MA_FN_StartOnOpening	= 1,    // пуск на открывающуюся задвижку
    MA_MD_StartOnClosed		= 2,    // пуск на закрытую задвижку
    MA_MD_QuickStart		= 3,    // быстрый пуск
    MA_MD_NormalStop		= 4,    // останов нормальный
    MA_MD_EmergencyStop		= 5,    // останов аварийный
    MA_MD_SetAutoMode		= 6,    // установить автоматический режим
    MA_MD_SetReserveMode	= 7,    // установить резерв
    MA_MD_SetTestMode		= 8,    // установить тестовый режим
    MA_MD_AutoOnRetry		= 9,    // автоматическое повторное включение
    MA_MD_StopInZD		= 10,   // останов входной задвижки
    MA_MD_StopOutZD		= 11,   // останов выходной задвижки
};



struct KAMAIDParams {
    uint16_t ZDOutID;
    uint16_t ZDInID;
    uint16_t DevID;
} __attribute__((packed));

struct KAMAAlarmSensorParams {
    uint16_t SensorID;
    uint16_t Delay;
    uint8_t Function;
    uint16_t WorkTime;
} __attribute__((packed));

struct KAMADelayParams {
    uint16_t DelayStartOnOpening;
    uint16_t DelayStartOnClosed;
    uint16_t DelayQuickStart;
    uint16_t DelayNormalStop;
    uint16_t DelayEmergencyStop;
} __attribute__((packed));


class KA_MA : public DA
{
    public:
//Methods
	KA_MA(TMdPrm& prm, uint16_t id, uint16_t n);
	~KA_MA();
	uint16_t count_n;
	uint16_t max_count_data;
	uint16_t GetState(void);
	bool IsIDParamChanged();
	void UpdateIDParam(uint16_t, uint8_t);
	bool IsSensorsParamChanged();
	void UpdateSensorsParam(uint16_t, uint8_t);
	bool IsDelayParamChanged();
	void UpdateDelayParam(uint16_t, uint8_t);
	void tmHandler(void);
	uint16_t HandleEvent(int64_t, uint8_t *);
	uint8_t cmdGet(uint16_t prmID, uint8_t * out);
	uint8_t cmdSet(uint8_t * req, uint8_t addr);
	uint16_t setVal(TVal &val);
	uint8_t SetNewIDParam(uint8_t, uint16_t, uint8_t *);
	uint8_t SetNewSensorsParam(uint8_t, uint16_t, uint8_t *);
	uint8_t SetNewDelayParam(uint8_t, uint16_t, uint8_t *);

	void saveIO(void);
	void loadIO(bool force = false);
	void saveParam(void);
	void loadParam(void);
	string getStatus(void);
	uint16_t config;
	ui16Data ZDOutID, ZDInID, DevID;
	ui8Data State, Function;
	ui16Data DelayStartOnOpening, DelayStartOnClosed, DelayQuickStart, DelayNormalStop, DelayEmergencyStop;
	int lnkSize()
	{
	    return 10;
	}
	int lnkId(const string &id)
	{
	    if (State.lnk.prmName == id) return 0;
	    if (ZDOutID.lnk.prmName == id) return 1;
	    if (ZDInID.lnk.prmName == id) return 2;
	    if (DevID.lnk.prmName == id) return 3;
	    if (Function.lnk.prmName == id) return 4;
	    if (DelayStartOnOpening.lnk.prmName == id) return 5;
	    if (DelayStartOnClosed.lnk.prmName == id) return 6;
	    if (DelayQuickStart.lnk.prmName == id) return 7;
	    if (DelayNormalStop.lnk.prmName == id) return 8;
	    if (DelayEmergencyStop.lnk.prmName == id) return 9;
	    return -1;
	}
	SLnk &lnk(int num)
	{
	    switch (num) {
	    case 0:
		return State.lnk;
	    case 1:
		return ZDOutID.lnk;
	    case 2:
		return ZDInID.lnk;
	    case 3:
		return DevID.lnk;
	    case 4:
		return Function.lnk;
	    case 5:
		return DelayStartOnOpening.lnk;
	    case 6:
		return DelayStartOnClosed.lnk;
	    case 7:
		return DelayQuickStart.lnk;
	    case 8:
		return DelayNormalStop.lnk;
	    case 9:
		return DelayEmergencyStop.lnk;
	    }
	}

};
class KA_MASensor : public DA
{
    public:
	KA_MASensor(TMdPrm& prm, DA &parent, uint16_t id);
	~KA_MASensor();
	DA &parentDA;
	bool IsSensorParamChanged();
//	uint16_t SetParams(void);
//	uint16_t RefreshParams(void);
//	uint16_t RefreshData(void);
//	uint16_t HandleEvent(int64_t, uint8_t *);
//	uint8_t cmdGet(uint16_t prmID, uint8_t * out);
//	uint8_t cmdSet(uint8_t * req, uint8_t addr);
	uint16_t setVal(TVal &val);
	void saveIO(void);
	void loadIO(bool force = false);
	void saveParam(void);
	void loadParam(void);
//	void tmHandler(void);

	ui8Data Function;
	ui16Data SensorID, Delay, WorkTime;
	int lnkSize()
	{
	    return 3;
	}
	int lnkId(const string &id)
	{
	    if (SensorID.lnk.prmName == id) return 0;
	    if (Delay.lnk.prmName == id) return 1;
	    if (Function.lnk.prmName == id) return 2;
	    return -1;
	}
	SLnk &lnk(int num)
	{
	    switch (num) {
	    case 0:
		return SensorID.lnk;
	    case 1:
		return Delay.lnk;
	    case 2:
		return Function.lnk;
	    }
	}

    protected:
//	void UpdateState(uint16_t ID, uint8_t cl);
//	void UpdateTUParam(uint16_t ID, uint8_t cl);
//	void UpdateTCParam(uint16_t ID, uint8_t cl);
//	void UpdateTime(uint16_t ID, uint8_t cl);
//	bool IsTUParamChanged();
//	bool IsTCParamChanged();
//	uint8_t SetNewTUParam(uint8_t addr, uint16_t prmID, uint8_t *val);
//	uint8_t SetNewTCParam(uint8_t addr, uint16_t prmID, uint8_t *val);
//	uint8_t SetNewState(uint8_t addr, uint16_t prmID, uint8_t *val);
//	uint8_t SetNewFunction(uint8_t addr, uint16_t prmID, uint8_t *val);
};
} //End namespace

#endif //DA_GNS_H
