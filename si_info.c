
#include "si_info.h"
#include "si.h"
#include "debug.h"
#include "utils.h"

#define _getshort(v) (short)( ((v)[1]) << 8 | ((v)[0]) )

static void dump_bits(char *label, uint8_t bits) {
	register uint8_t i,mask;
	char bitstr[9];

	i = 0;
	mask = 1;
	while(mask) {
		bitstr[i++] = (bits & mask ? '1' : '0');
		mask <<= 1;
	}
	bitstr[i] = 0;
	dprintf(1,"%s(%02x): %s\n",label,bits,bitstr);
}

int si_get_info(si_session_t *s, si_info_t *info) {
	uint8_t data[8];
	uint8_t bits;

	/* x300 Active power grid/gen */
	if (s->get_data(s,0x300,data,8)) return 1;
	info->active.grid.l1 = _getshort(&data[0]) * 100.0;
	info->active.grid.l2 = _getshort(&data[2]) * 100.0;
	info->active.grid.l3 = _getshort(&data[4]) * 100.0;
	dprintf(1,"active grid: l1: %.1f, l2: %.1f, l3: %.1f\n", info->active.grid.l1, info->active.grid.l2, info->active.grid.l3);

	/* x301 Active power Sunny Island */
	if (s->get_data(s,0x301,data,8)) return 1;
	info->active.si.l1 = _getshort(&data[0]) * 100.0;
	info->active.si.l2 = _getshort(&data[2]) * 100.0;
	info->active.si.l3 = _getshort(&data[4]) * 100.0;
	dprintf(1,"active si: l1: %.1f, l2: %.1f, l3: %.1f\n", info->active.si.l1, info->active.si.l2, info->active.si.l3);

	/* x302 Reactive power grid/gen */
	if (s->get_data(s,0x302,data,8)) return 1;
	info->reactive.grid.l1 = _getshort(&data[0]) * 100.0;
	info->reactive.grid.l2 = _getshort(&data[2]) * 100.0;
	info->reactive.grid.l3 = _getshort(&data[4]) * 100.0;
	dprintf(1,"reactive grid: l1: %.1f, l2: %.1f, l3: %.1f\n", info->reactive.grid.l1, info->reactive.grid.l2, info->reactive.grid.l3);

	/* x303 Reactive power Sunny Island */
	if (s->get_data(s,0x303,data,8)) return 1;
	info->reactive.si.l1 = _getshort(&data[0]) * 100.0;
	info->reactive.si.l2 = _getshort(&data[2]) * 100.0;
	info->reactive.si.l3 = _getshort(&data[4]) * 100.0;
	dprintf(1,"reactive si: l1: %.1f, l2: %.1f, l3: %.1f\n", info->reactive.si.l1, info->reactive.si.l2, info->reactive.si.l3);

	/* 0x304 OutputVoltage - L1 / L2 / Output Freq */
	s->get_data(s,0x304,data,8);
	info->voltage.l1 = _getshort(&data[0]) / 10.0;
	info->voltage.l2 = _getshort(&data[2]) / 10.0;
	info->voltage.l3 = _getshort(&data[4]) / 10.0;
	info->frequency = _getshort(&data[6]) / 100.0;
	dprintf(1,"voltage: l1: %.1f, l2: %.1f, l3: %.1f\n", info->voltage.l1, info->voltage.l2, info->voltage.l3);
	dprintf(1,"frequency: %.1f\n",info->frequency);

	/* 0x305 Battery voltage Battery current Battery temperature SOC battery */
	s->get_data(s,0x305,data,8);
	info->battery_voltage = _getshort(&data[0]) / 10.0;
	info->battery_current = _getshort(&data[2]) / 10.0;
	info->battery_temp = _getshort(&data[4]) / 10.0;
	info->battery_soc = _getshort(&data[6]) / 10.0;
	dprintf(1,"battery_voltage: %.1f\n", info->battery_voltage);
	dprintf(1,"battery_current: %.1f\n", info->battery_current);
	dprintf(1,"battery_temp: %.1f\n", info->battery_temp);
	dprintf(1,"battery_soc: %.1f\n", info->battery_soc);

	/* 0x306 SOH battery / Charging procedure / Operating state / active Error Message / Battery Charge Voltage Set-point */
	s->get_data(s,0x306,data,8);
	info->battery_soh = _getshort(&data[0]);
	info->charging_proc = data[2];
	info->state = data[3];
	info->errmsg = _getshort(&data[4]);
	info->battery_cvsp = _getshort(&data[6]) / 10.0;
	dprintf(1,"battery_soh: %.1f, charging_proc: %d, state: %d, errmsg: %d, battery_cvsp: %.1f\n",
		info->battery_soh, info->charging_proc, info->state, info->errmsg, info->battery_cvsp);

	/* 0x307 Relay state / Relay function bit 1 / Relay function bit 2 / Synch-Bits */
	s->get_data(s,0x307,data,8);
#define SET(m,b) { info->bits.m = ((bits & b) != 0); dprintf(1,"bits.%s: %d\n",#m,info->bits.m); }
	bits = data[0];
	dump_bits("data[0]",bits);
	SET(relay1,   0x0001);
	SET(relay2,   0x0002);
	SET(s1_relay1,0x0004);
	SET(s1_relay2,0x0008);
	SET(s2_relay1,0x0010);
	SET(s2_relay2,0x0020);
	SET(s3_relay1,0x0040);
	SET(s3_relay2,0x0080);
	bits = data[1];
	dump_bits("data[1]",bits);
	SET(GnRn,     0x0001);
	SET(s1_GnRn,  0x0002);
	SET(s2_GnRn,  0x0004);
	SET(s3_GnRn,  0x0008);
	bits = data[2];
	dump_bits("data[2]",bits);
	SET(AutoGn,    0x0001);
	SET(AutoLodExt,0x0002);
	SET(AutoLodSoc,0x0004);
	SET(Tm1,       0x0008);
	SET(Tm2,       0x0010);
	SET(ExtPwrDer, 0x0020);
	SET(ExtVfOk,   0x0040);
	SET(GdOn,      0x0080);
	bits = data[3];
	dump_bits("data[3]",bits);
	SET(Error,     0x0001);
	SET(Run,       0x0002);
	SET(BatFan,    0x0004);
	SET(AcdCir,    0x0008);
	SET(MccBatFan, 0x0010);
	SET(MccAutoLod,0x0020);
	SET(Chp,       0x0040);
	SET(ChpAdd,    0x0080);
	bits = data[4];
	dump_bits("data[4]",bits);
	SET(SiComRemote,0x0001);
	SET(OverLoad,  0x0002);
	bits = data[5];
	dump_bits("data[5]",bits);
	SET(ExtSrcConn,0x0001);
	SET(Silent,    0x0002);
	SET(Current,   0x0004);
	SET(FeedSelfC, 0x0008);

	/* 0x308 TotLodPwr */
	s->get_data(s,0x308,data,8);
	info->load.l1 = _getshort(&data[0]) * 100.0;
	info->load.l2 = _getshort(&data[2]) * 100.0;
	info->load.l3 = _getshort(&data[4]) * 100.0;
	dprintf(1,"load: l1: %.1f, l2: %.1f, l3: %.1f\n",info->load.l1, info->load.l2, info->load.l3);

	/* 0x309 AC2 Voltage L1 / AC2 Voltage L2 / AC2 Voltage L3 / AC2 Frequency */
	s->get_data(s,0x309,data,8);
	info->ac2.l1 = _getshort(&data[0]) / 10.0;
	info->ac2.l2 = _getshort(&data[2]) / 10.0;
	info->ac2.l3 = _getshort(&data[4]) / 10.0;
	info->ac2_frequency = _getshort(&data[6]) / 100.0;
	dprintf(1,"ac2: l1: %.1f, l2: %.1f, l3: %.1f\n",info->ac2.l1, info->ac2.l2, info->ac2.l3);
	dprintf(1,"ac2 frequency: %2.2f\n",info->ac2_frequency);

	/* 0x30A PVPwrAt / GdCsmpPwrAt / GdFeedPwr */
	s->get_data(s,0x30a,data,8);
	info->PVPwrAt = _getshort(&data[0]) / 10.0;
	info->GdCsmpPwrAt = _getshort(&data[0]) / 10.0;
	info->GdFeedPwr = _getshort(&data[0]) / 10.0;
	dprintf(1,"PVPwrAt: %.1f\n", info->PVPwrAt);
	dprintf(1,"GdCsmpPwrAt: %.1f\n", info->GdCsmpPwrAt);
	dprintf(1,"GdFeedPwr: %.1f\n", info->GdFeedPwr);
	return 0;
}
