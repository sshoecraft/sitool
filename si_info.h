
#ifndef __SI_INFO_H
#define __SI_INFO_H

#include "config.h"
#include "module.h"
#include "si.h"

struct si_power {
	float l1;
	float l2;
	float l3;
};
typedef struct si_power si_power_t;

struct si_info {
	struct {
		si_power_t grid;
		si_power_t si;
	} active;
	struct {
		si_power_t grid;
		si_power_t si;
	} reactive;
	si_power_t voltage;
	float frequency;
	float battery_voltage;
	float battery_current;
	float battery_temp;
	float battery_soc;
	float battery_soh;
	float battery_cvsp;
	struct {
		unsigned relay1: 1;
		unsigned relay2: 1;
		unsigned s1_relay1: 1;
		unsigned s1_relay2: 1;
		unsigned s2_relay1: 1;
		unsigned s2_relay2: 1;
		unsigned s3_relay1: 1;
		unsigned s3_relay2: 1;
		unsigned GnRn: 1;
		unsigned s1_GnRn: 1;
		unsigned s2_GnRn: 1;
		unsigned s3_GnRn: 1;
		unsigned AutoGn: 1;
		unsigned AutoLodExt: 1;
		unsigned AutoLodSoc: 1;
		unsigned Tm1: 1;
		unsigned Tm2: 1;
		unsigned ExtPwrDer: 1;
		unsigned ExtVfOk: 1;
		unsigned GdOn: 1;
		unsigned Error: 1;
		unsigned Run: 1;
		unsigned BatFan: 1;
		unsigned AcdCir: 1;
		unsigned MccBatFan: 1;
		unsigned MccAutoLod: 1;
		unsigned Chp: 1;
		unsigned ChpAdd: 1;
		unsigned SiComRemote: 1;
		unsigned OverLoad: 1;
		unsigned ExtSrcConn: 1;
		unsigned Silent: 1;
		unsigned Current: 1;
		unsigned FeedSelfC: 1;
	} bits;
	si_power_t load;
	uint8_t charging_proc;
	uint8_t state;
	uint16_t errmsg;
	si_power_t ac2;
	float ac2_frequency;
	float PVPwrAt;
	float GdCsmpPwrAt;
	float GdFeedPwr;
};
typedef struct si_info si_info_t;

int si_get_info(si_session_t *s, si_info_t *info);

#endif /* __SI_INFO_H */
