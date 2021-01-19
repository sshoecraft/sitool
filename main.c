
/*
Copyright (c) 2021, Stephen P. Shoecraft
All rights reserved.

This source code is licensed under the BSD-style license found in the
LICENSE file in the root directory of this source tree.
*/

struct mybmm_config;
typedef struct mybmm_config mybmm_config_t;

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <ctype.h>
#include <dlfcn.h>
//#include "util.h"
#include "parson.h"
#include "mybmm.h"
#include "si_info.h"

int debug = 1;

int outfmt = 0;
FILE *outfp;
char sepch;
char *sepstr;

JSON_Value *root_value;
JSON_Object *root_object;
char *serialized_string = NULL;

#define _getshort(p) (short)(*(p) | (*((p)+1) << 8))
#define _getint(p) (int)(*(p) | (*((p)+1) << 8) | (*((p)+2) << 16) | (*((p)+3) << 24))

enum JBD_PARM_DT {
	JBD_PARM_DT_UNK,
	JBD_PARM_DT_INT,		/* Std int/number */
	JBD_PARM_DT_FLOAT,		/* floating pt */
	JBD_PARM_DT_STR,		/* string */
	JBD_PARM_DT_TEMP,		/* temp */
	JBD_PARM_DT_DATE,		/* date */
	JBD_PARM_DT_PCT,		/* % */
	JBD_PARM_DT_FUNC,		/* function bits */
	JBD_PARM_DT_NTC,		/* ntc bits */
	JBD_PARM_DT_B0,			/* byte 0 */
	JBD_PARM_DT_B1,			/* byte 1 */
};

struct jk_params {
	uint8_t reg;
	char *label;
	int dt;
} params[] = {
	{ 0,0,0 }
};
typedef struct jk_params jk_params_t;

struct jk_params *_getp(char *label) {
	register struct jk_params *pp;

	dprintf(3,"label: %s\n", label);
	for(pp = params; pp->label; pp++) {
		dprintf(3,"pp->label: %s\n", pp->label);
		if (strcmp(pp->label,label)==0) {
			return pp;
		}
	}
	return 0;
}

void dint(char *label, char *format, int val) {
	char temp[128];

	dprintf(3,"dint: label: %s, val: %d\n", label, val);
	switch(outfmt) {
	case 2:
		json_object_set_number(root_object, label, val);
		break;
	case 1:
		sprintf(temp,"%%s,%s\n",format);
		dprintf(3,"temp: %s\n", temp);
		fprintf(outfp,temp,label,val);
		break;
	default:
		sprintf(temp,"%%-25s %s\n",format);
		dprintf(3,"temp: %s\n", temp);
		fprintf(outfp,temp,label,val);
		break;
	}
}
#define _dint(l,v) dint(l,"%d",v)

void dfloat(char *label, char *format, float val) {
	char temp[128];

	dprintf(3,"dint: label: %s, val: %f\n", label, val);
	switch(outfmt) {
	case 2:
		json_object_set_number(root_object, label, val);
		break;
	case 1:
		sprintf(temp,"%%s,%s\n",format);
		dprintf(3,"temp: %s\n", temp);
		fprintf(outfp,temp,label,val);
		break;
	default:
		sprintf(temp,"%%-25s %s\n",format);
		dprintf(3,"temp: %s\n", temp);
		fprintf(outfp,temp,label,val);
		break;
	}
}
#define _dfloat(l,v) dfloat(l,"%f",v)

void dstr(char *label, char *format, char *val) {
	char temp[128];

	dprintf(3,"dint: label: %s, val: %s\n", label, val);
	switch(outfmt) {
	case 2:
		json_object_set_string(root_object, label, val);
		break;
	case 1:
		sprintf(temp,"%%s,%s\n",format);
		dprintf(3,"temp: %s\n", temp);
		fprintf(outfp,temp,label,val);
		break;
	default:
		sprintf(temp,"%%-25s %s\n",format);
		dprintf(3,"temp: %s\n", temp);
		fprintf(outfp,temp,label,val);
		break;
	}
}
#define _dstr(l,v) dstr(l,"%s",v)

static inline void _addstr(char *str,char *newstr) {
	dprintf(4,"str: %s, newstr: %s\n", str, newstr);
	if (strlen(str)) strcat(str,sepstr);
	dprintf(4,"str: %s\n", str);
	if (outfmt == 2) strcat(str,"\"");
	strcat(str,newstr);
	if (outfmt == 2) strcat(str,"\"");
	dprintf(4,"str: %s\n", str);
}

#if 0
void disp(char *label, int dt, ...) {
	va_list ap;

	dprintf(3,"label: %s, dt: %d\n", label, dt);

	va_start(ap,dt);
	switch(dt) {
	default:
	case JBD_PARM_DT_INT:
		_dint(label,va_arg(ap,int));
		break;
	case JBD_PARM_DT_FLOAT:
		_dfloat(label,va_arg(ap,double));
		break;
	case JBD_PARM_DT_STR:
		_dstr(label,va_arg(ap,char *));
		break;
	case JBD_PARM_DT_TEMP:
		_dint(label,va_arg(ap,int));
		break;
	case JBD_PARM_DT_DATE:
		_dint(label,va_arg(ap,int));
		break;
	case JBD_PARM_DT_FUNC:
		_dint(label,va_arg(ap,int));
		break;
	case JBD_PARM_DT_NTC:
		_dint(label,va_arg(ap,int));
		break;
        case JBD_PARM_DT_B0:
		_dint(label,va_arg(ap,int));
		break;
        case JBD_PARM_DT_B1:
		_dint(label,va_arg(ap,int));
		break;
	}
}

void pdisp(char *label, int dt, uint8_t *data, int len) {
	dprintf(3,"label: %s, dt: %d\n", label, dt);
	switch(dt) {
	case JBD_PARM_DT_INT:
	case JBD_PARM_DT_TEMP:
	case JBD_PARM_DT_DATE:
	case JBD_PARM_DT_PCT:
	case JBD_PARM_DT_FUNC:
	case JBD_PARM_DT_NTC:
		_dint(label,(int)_getshort(data));
		break;
	case JBD_PARM_DT_B0:
		_dint(label,data[0]);
		break;
	case JBD_PARM_DT_B1:
		_dint(label,data[1]);
		break;
	case JBD_PARM_DT_FLOAT:
		_dfloat(label,(float)_getshort(data));
		break;
	case JBD_PARM_DT_STR:
		data[len] = 0;
		trim((char *)data);
		_dstr(label,(char *)data);
		break;
	}
}
#endif

void _outpower(char *label, si_power_t *p) {
	char str[32],temp[36];

	str[0] = 0;
	sprintf(temp,"%.2f",p->l1);
	_addstr(str,temp);
	sprintf(temp,"%.2f",p->l2);
	_addstr(str,temp);
	sprintf(temp,"%.2f",p->l3);
	_addstr(str,temp);
	switch(outfmt) {
	case 2:
		sprintf(temp,"[ %s ]",str);
		dprintf(1,"temp: %s\n", temp);
		json_object_dotset_value(root_object, label, json_parse_string(temp));
		break;
	case 1:
		printf("%s,%s\n",label,str);
		break;
	default:
		printf("%-25s %s\n",label,str);
		break;
	}
}

void display_info(si_info_t *info) {
	_outpower("Active Grid",&info->active.grid);
	_outpower("Active SI",&info->active.si);
	_outpower("ReActive Grid",&info->reactive.grid);
	_outpower("ReActive SI",&info->reactive.si);
#if 0
	struct {
		power_t grid;
		power_t si;
	} active;
	struct {
		power_t grid;
		power_t si;
	} reactive;
	power_t voltage;
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
	power_t load;
	uint8_t charging_proc;
	uint8_t state;
	uint16_t errmsg;
	power_t ac2;
	float ac2_frequency;
	float PVPwrAt;
	float GdCsmpPwrAt;
	float GdFeedPwr;

	dfloat("Battery Voltage","%.3f",info->battery_voltage);
	dfloat("battCurrent","%.3f",info->current);
	dfloat("DesignCapacity","%.3f",info->fullcap);
	dfloat("RemainingCapacity","%.3f",info->capacity);
	_dint("PercentCapacity",info->pctcap);
	_dint("CycleCount",info->cycles);
	_dint("Probes",info->probes);
	switch(outfmt) {
	case 2:
		p = temp;
		p += sprintf(p,"[ ");
		for(i=0; i < info->probes; i++) {
			if (i) p += sprintf(p,",");
			p += sprintf(p, "%.1f",info->temps[i]);
		}
		strcat(temp," ]");
                dprintf(1,"temp: %s\n", temp);
                json_object_dotset_value(root_object, "Temps", json_parse_string(temp));
                break;
	default:
		p = temp;
		for(i=0; i < info->probes; i++) {
			if (i) p += sprintf(p,"%c",sepch);
			p += sprintf(p, "%.1f",info->temps[i]);
		}
		dstr("Temps","%s",temp);
                break;
	}
	_dint("Strings",info->strings);
	switch(outfmt) {
	case 2:
		p = temp;
		p += sprintf(p,"[ ");
		for(i=0; i < info->strings; i++) {
			if (i) p += sprintf(p,",");
			p += sprintf(p, "%.3f",info->cellvolt[i]);
		}
		strcat(temp," ]");
                dprintf(1,"temp: %s\n", temp);
                json_object_dotset_value(root_object, "Cells", json_parse_string(temp));
                break;
	default:
		p = temp;
		for(i=0; i < info->strings; i++) {
			if (i) p += sprintf(p,"%c",sepch);
			p += sprintf(p, "%.3f",info->cellvolt[i]);
		}
		dstr("Cells","%s",temp);
                break;
	}
	dfloat("CellTotal","%.3f",info->cell_total);
	dfloat("CellMin","%.3f",info->cell_min);
	dfloat("CellMax","%.3f",info->cell_max);
	dfloat("CellDiff","%.3f",info->cell_diff);
	dfloat("CellAvg","%.3f",info->cell_avg);
	_dstr("DeviceName",info->model);
	_dstr("ManufactureDate",info->mfgdate);
	dfloat("Version","%.1f",info->version);
	temp[0] = 0;
	p = temp;
	if (info->fetstate & JBD_MOS_CHARGE) p += sprintf(p,"Charge");
	if (info->fetstate & JBD_MOS_DISCHARGE) {
		if (info->fetstate & JBD_MOS_CHARGE) p += sprintf(p,sepstr);
		p += sprintf(p,"Discharge");
	}
	dstr("FET","%s",temp);
#endif
}

int init_inv(mybmm_inverter_t *inv, mybmm_config_t *c, char *type, char *transport, char *target, char *opts, mybmm_module_t *cp, mybmm_module_t *tp) {
	memset(inv,0,sizeof(*inv));
	strcpy(inv->type,type);
	if (transport) strcpy(inv->transport,transport);
	if (target) strcpy(inv->target,target);
//	if (opts) strcpy(inv->opts,opts);
        inv->open = cp->open;
        inv->read = cp->read;
        inv->close = cp->close;
        inv->handle = cp->new(c,inv,tp);
	dprintf(1,"handle: %p\n", inv->handle);
        return 0;
}

int si_start(si_session_t *s) {
	unsigned char b;
	int retries;
	uint8_t data[8];

	b = 0x01;
	retries=5;
	while(retries--) {
		if (s->tp->write(s->tp_handle,0x35c,&b,1)) return 1;
		if (s->get_data(s,0x307,data,8) == 0) {
			bindump("data",data,8);
			if (data[3] & 0x0002) return 0;
		}
		sleep(1);
	}
	return (retries < 0 ? 1 : 0);
}

void si_stop(si_session_t *s) {
}

void usage(char *name) {
	printf("usage: %s [-acjJrwlh] [-f filename] [-b <bluetooth mac addr | -i <ip addr>] [-o output file]\n",name);
	printf("arguments:\n");
#ifdef DEBUG
	printf("  -d <#>		debug output\n");
#endif
	printf("  -j		JSON output\n");
	printf("  -J		JSON output pretty print\n");
	printf("  -s		Start SI\n");
	printf("  -S		Stop SI\n");
	printf("  -h		this output\n");
	printf("  -o <filename>	output filename\n");
	printf("  -t <transport:target> transport & target\n");
	printf("  -e <opts>	transport-specific opts\n");
}

enum SITOOL_ACTION {
	SITOOL_ACTION_INFO=0,
	SITOOL_ACTION_START,
	SITOOL_ACTION_STOP
};

int main(int argc, char **argv) {
	int opt,action,pretty;
	char *transport,*target,*outfile,*opts;
	mybmm_config_t *conf;
	mybmm_module_t *cp,*tp;
	si_info_t info;
	mybmm_inverter_t inv;
	si_session_t *s;

	pretty = outfmt = 0;
	sepch = ',';
	sepstr = ",";
	action = SITOOL_ACTION_INFO;
	transport = target = outfile = opts = 0;
	while ((opt=getopt(argc, argv, "d:sSt:e:jJo:h")) != -1) {
		switch (opt) {
		case 'd':
			debug = atoi(optarg);
			break;
                case 't':
			transport = optarg;
			target = strchr(transport,':');
			if (!target) {
				printf("error: format is transport:target\n");
				usage(argv[0]);
				return 1;
			}
			*target = 0;
			target++;
			break;
		case 'e':
			opts = optarg;
			break;
		case 'j':
			outfmt=2;
			pretty = 0;
			break;
		case 'J':
			outfmt=2;
			pretty = 1;
			break;
		case 's':
			action = SITOOL_ACTION_START;
			break;
		case 'S':
			action = SITOOL_ACTION_STOP;
			break;
		case 'h':
		default:
			usage(argv[0]);
			exit(0);
                }
        }
	dprintf(1,"transport: %p, target: %p\n", transport, target);
	if (!transport || !target) {
		usage(argv[0]);
		return 1;
	}

        argc -= optind;
        argv += optind;
        optind = 0;

#if 0
	if ((action == SITOOL_ACTION_READ || action == SITOOL_ACTION_WRITE) && !filename && !argc && !all && !reg && !dump) {
		printf("error: a filename or parameter name or all (a) must be specified.\n");
		usage(argv[0]);
		return 1;
	}
#endif

	conf = calloc(sizeof(*conf),1);
	if (!conf) {
		perror("calloc conf");
		return 1;
	}
	conf->modules = list_create();

	dprintf(2,"transport: %s\n", transport);

	tp = mybmm_load_module(conf,transport,MYBMM_MODTYPE_TRANSPORT);
	if (!tp) return 1;
	cp = mybmm_load_module(conf,"si",MYBMM_MODTYPE_INVERTER);
	if (!cp) return 1;

	/* Init the inverter */
	if (init_inv(&inv,conf,"si",transport,target,opts,cp,tp)) return 1;
	s = inv.handle;

	if (outfile) {
		dprintf(1,"outfile: %s\n", outfile);
		outfp = fopen(outfile,"w+");
		if (!outfp) {
			perror("fopen outfile");
			return 1;
		}
	} else {
		outfp = fdopen(1,"w");
	}
	dprintf(1,"outfp: %p\n", outfp);

	if (outfmt == 2) {
		root_value = json_value_init_object();
		root_object = json_value_get_object(root_value);
	}

	switch(action) {
	case SITOOL_ACTION_INFO:
		if (inv.open(inv.handle)) return 1;
		si_get_info(s,&info);
		inv.close(inv.handle);
		break;
	case SITOOL_ACTION_START:
		if (inv.open(inv.handle)) return 1;
		si_start(s);
		inv.close(inv.handle);
		break;
	case SITOOL_ACTION_STOP:
		if (inv.open(inv.handle)) return 1;
		si_stop(s);
		inv.close(inv.handle);
		break;
	}
#if 0
	switch(action) {
	case SITOOL_ACTION_INFO:
		if (pack.open(pack.handle)) return 1;
		pack.close(pack.handle);
		break;
	case SITOOL_ACTION_READ:
		if (strcmp(transport,"can")==0) {
			printf("error: reading parameters not possible using CAN bus\n");
			return 1;
		}
		if (reg) {
			char temp[16];

			if (pack.open(pack.handle)) return 1;
			if (jk_eeprom_start(pack.handle)) return 1;
			bytes = jk_rw(pack.handle, JBD_CMD_READ, reg, data, sizeof(data));
			dprintf(3,"bytes: %d\n", bytes);
			if (bytes > 0) {
				sprintf(temp,"Register %02x\n", reg);
				pdisp(temp,JBD_PARM_DT_INT,data,bytes);
			}
			jk_eeprom_end(pack.handle);
			pack.close(pack.handle);
		}
		if (filename) {
			char line[128];
			FILE *fp;

			dprintf(2,"filename: %s\n", filename);

			/* Get param names from .json file? */
			p = strrchr(filename,'.');
			if (p && strcmp(p,".json")==0) {
				JSON_Object *object;
				int count;

				root_value = json_parse_file(filename);
				if (json_value_get_type(root_value) != JSONObject) {
					printf("error: not a valid json file\n");
					return 1;
				}
				if (pack.open(pack.handle)) return 1;
				if (jk_eeprom_start(pack.handle)) return 1;
				object = json_value_get_object(root_value);
				count  = json_object_get_count(object);
				for (i = 0; i < count; i++) {
					p = (char *)json_object_get_name(object, i);
					if (!p) {
						printf("error reading json file\n");
						return 1;
					}
					dprintf(3,"p: %s\n", p);
					pp = _getp(p);
					if (!pp) {
						printf("error: parm in json file not found: %s\n", p);
						return 1;
					}
					memset(data,0,sizeof(data));
					bytes = jk_rw(pack.handle, JBD_CMD_READ, pp->reg, data, sizeof(data));
					if (bytes < 0) continue;
					dprintf(3,"bytes: %d\n", bytes);
					pdisp(pp->label,pp->dt,data,bytes);
				}
				jk_eeprom_end(pack.handle);
				pack.close(pack.handle);
				json_value_free(root_value);
			} else {
				fp = fopen(filename,"r");
				if (!fp) {
					printf("fopen(r) %s: %s\n", filename, strerror(errno));
					return 1;
				}
				if (pack.open(pack.handle)) return 1;
				if (jk_eeprom_start(pack.handle)) return 1;
				while(fgets(line,sizeof(line),fp)) {
					p = line;
					while(*p && isspace(*p)) p++;
					label = p;
					while(*p && !isspace(*p)) p++;
					*p = 0;
					pp = _getp(label);
					dprintf(3,"pp: %p\n", pp);
					if (!pp) continue;
					memset(data,0,sizeof(data));
					bytes = jk_rw(pack.handle, JBD_CMD_READ, pp->reg, data, sizeof(data));
					if (bytes < 0) continue;
					dprintf(3,"bytes: %d\n", bytes);
					pdisp(pp->label,pp->dt,data,bytes);
				}
				jk_eeprom_end(pack.handle);
				pack.close(pack.handle);
				fclose(fp);
			}
		} else {
			dprintf(1,"all: %d\n", all);
			if (pack.open(pack.handle)) return 1;
			if (jk_eeprom_start(pack.handle)) return 1;
			if (all) {
				for(pp = params; pp->label; pp++) {
					dprintf(3,"pp->label: %s\n", pp->label);
					memset(data,0,sizeof(data));
					bytes = jk_rw(pack.handle, JBD_CMD_READ, pp->reg, data, sizeof(data));
					if (bytes < 0) break;
					if (bytes) pdisp(pp->label,pp->dt,data,bytes);
				}
			} else {
				/* Every arg is a parm name */
				for(i=0; i < argc; i++) {
					pp = _getp(argv[i]);
					dprintf(2,"pp: %p\n", pp);
					if (!pp) {
						printf("error: parameter %s not found.\n",argv[i]);
						return 1;
					}
					memset(data,0,sizeof(data));
					bytes = jk_rw(pack.handle, JBD_CMD_READ, pp->reg, data, sizeof(data));
					if (bytes > 0) pdisp(pp->label,pp->dt,data,bytes);
				}
			}
			jk_eeprom_end(pack.handle);
			pack.close(pack.handle);
		}
		break;
	case SITOOL_ACTION_WRITE:
		if (strcmp(transport,"can")==0) {
			printf("error: writing parameters not possible using CAN bus\n");
			return 1;
		}
		if (filename) {
			char line[128],*valp;
			FILE *fp;

			dprintf(3,"filename: %s\n", filename);

			p = strrchr(filename,'.');
			if (p && strcmp(p,".json")==0) {
				char temp[128];
				JSON_Object *object;
				JSON_Value *value;
				int count,type,num;

				root_value = json_parse_file(filename);
				if (json_value_get_type(root_value) != JSONObject) {
					printf("error: not a valid json file\n");
					return 1;
				}
				if (pack.open(pack.handle)) return 1;
				if (jk_eeprom_start(pack.handle)) return 1;
				object = json_value_get_object(root_value);
				count  = json_object_get_count(object);
				for (i = 0; i < count; i++) {
					p = (char *)json_object_get_name(object, i);
					if (!p) {
						printf("error reading json file\n");
						return 1;
					}
					dprintf(3,"p: %s\n", p);
					pp = _getp(p);
					if (!pp) {
						printf("error: parm in json file not found: %s\n", p);
						return 1;
					}
					value = json_object_get_value(object, pp->label);
					type = json_value_get_type(value);
					switch(type) {
					case JSONString:
						p = (char *)json_value_get_string(value);
						break;
					case JSONNumber:
						num = (int)json_value_get_number(value);
						dprintf(3,"value: %d\n", num);
						sprintf(temp,"%d",num);
						p = temp;
						break;
					default:
						printf("error: bad type in json file: %d\n", type);
						break;
					}
					if (write_parm(pack.handle,pp,p)) break;
				}
				jk_eeprom_end(pack.handle);
				pack.close(pack.handle);
				json_value_free(root_value);
			} else {
				fp = fopen(filename,"r");
				if (!fp) {
					printf("fopen(r) %s: %s\n", filename, strerror(errno));
					return 1;
				}
				if (pack.open(pack.handle)) return 1;
				if (jk_eeprom_start(pack.handle)) return 1;
				while(fgets(line,sizeof(line),fp)) {
					/* get parm */
					p = line;
					while(*p && isspace(*p)) p++;
					label = p;
					while(*p && !isspace(*p)) p++;
					*p = 0;
					dprintf(3,"label: %s\n", label);
					pp = _getp(label);
					dprintf(4,"pp: %p\n", pp);
					if (!pp) continue;
					/* get value */
					p++;
					while(*p && isspace(*p)) p++;
					valp = p;
					while(*p && !isspace(*p)) p++;
					*p = 0;
					dprintf(3,"valp: %s\n", valp);
					if (write_parm(pack.handle,pp,valp)) break;
				}
			}
			jk_eeprom_end(pack.handle);
			pack.close(pack.handle);
		} else {
			/* Every arg par is a parm name & value */
			if (pack.open(pack.handle)) return 1;
			if (jk_eeprom_start(pack.handle)) return 1;
			for(i=0; i < argc; i++) {
				/* Ge the parm */
				pp = _getp(argv[i]);
				dprintf(3,"pp: %p\n", pp);
				if (!pp) {
					printf("error: parameter %s not found.\n",argv[i]);
					break;
				}
				/* Get the value */
				if (i+1 == argc) {
					printf("error: no value for parameter %s\n",argv[i]);
					break;
				}
				i++;
				if (write_parm(pack.handle,pp,argv[i])) break;
			}
			jk_eeprom_end(pack.handle);
			pack.close(pack.handle);
		}
		break;
	}
#endif
	if (outfmt == 2) {
		if (pretty)
	    		serialized_string = json_serialize_to_string_pretty(root_value);
		else
    			serialized_string = json_serialize_to_string(root_value);
		fprintf(outfp,"%s",serialized_string);
		json_free_serialized_string(serialized_string);
		json_value_free(root_value);
	}
	fclose(outfp);

	return 0;
}
