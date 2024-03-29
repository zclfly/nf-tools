

#include <libnf.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <limits.h>
#include <flist.h>
#include <time.h>
#include <pthread.h>
#include "screen.h"

#define MAX_STR 100	/* max length of format string */
#define MAX_STR_LONG 1000	/* max length of format string */

#define UNIT_1K (double)(1000.0)
#define UNIT_1M (double)(1000.0 * 1000.0)
#define UNIT_1G (double)(1000.0 * 1000.0 * 1000.0)
#define UNIT_1T (double)(1000.0 * 1000.0 * 1000.0 * 1000.0)

/* format big bumber with units */
static void num_unit(char *buff, double num) {

	if (num >= UNIT_1T) {
		sprintf(buff, "%.1f T", num / UNIT_1T);
	} else if (num >= UNIT_1G) {
		sprintf(buff, "%.1f G", num / UNIT_1G);
	} else if (num >= UNIT_1M) {
		sprintf(buff, "%.1f M", num / UNIT_1M);
	} else {
		sprintf(buff, "%.1f", num);
	}
}

/* prototype of formating function */
typedef void (*format_func_t)(char *buff, char *data);

/* function for print number */
static void format_uint64_unit(char *buff, char *data) {
	num_unit(buff, *((uint64_t *)data));
}

/* basic functions for print number */
static void format_uint64(char *buff, char *data) {
	sprintf(buff, " %llu", *((uint64_t *)data));
}

static void format_uint32(char *buff, char *data) {
	sprintf(buff, " %u", *((uint32_t *)data));
}

static void format_uint16(char *buff, char *data) {
	sprintf(buff, " %hu", *((uint16_t *)data));
}

static void format_uint8(char *buff, char *data) {
	sprintf(buff, " %hhu", *((uint8_t *)data));
}

/* function for print float */
static void format_double(char *buff, char *data) {
	num_unit(buff, *((double *)data));
}

/* function for print duration */
static void format_duration(char *buff, char *data) {
	sprintf(buff, " %1.3f", *((double *)data) / 1000);
}

/* format date/time */
static void format_date(char *buff, char *data) {
	time_t sec;
	int msec;
	struct tm *ts;
	char buff2[MAX_STR];

	sec = *((uint64_t *)data) / 1000LL;	
	msec = *((uint64_t *)data) - sec * 1000LL;
	ts = localtime(&sec);
	strftime(buff, MAX_STR, "%Y-%m-%d %H:%M:%S", ts);
	sprintf(buff2, ".%03d", msec);
	strcat(buff, buff2);
}

/* function for IPv4/IPv6 address */
static void format_addr(char *buff, char *data) {
	lnf_ip_t *addr = (lnf_ip_t *)data;

	if (IN6_IS_ADDR_V4COMPAT(addr)) {
		inet_ntop(AF_INET, (char *)&(addr->data[3]), buff, MAX_STR);
	} else {
		inet_ntop(AF_INET6, addr, buff, MAX_STR);
	}
}


/* list of the fields to be displayed */
typedef struct field_ent_s {
	int field;
	int type;
	int numbits;
	int numbits6;
	char format[MAX_STR];
	char hdr_format[MAX_STR];
	format_func_t format_func;
} field_ent_t;

field_ent_t fields[LNF_FLD_TERM_] = { };
int numfields = 0;

/* defines format ptions for types and fields */
/* type 0 and field 0 defines default value  */
typedef struct format_ent_s {
	int type;
	int field;
	char format[MAX_STR];		/* column width */
	format_func_t format_func;
} format_ent_t;

const format_ent_t formats[] = { 
	{ 0,          0, "%s ", NULL }, 				/* default for all types */
	{ LNF_UINT64, 0, "%8s ", format_uint64_unit },	/* default for uint64_t */
	{ LNF_UINT32, 0, "%8s ", format_uint32 },	/* default for uint32_t */
	{ LNF_UINT16, 0, "%8s ", format_uint16 },	/* default for uint16_t */
	{ LNF_UINT8, 0, "%8s ", format_uint8 },	/* default for uint8_t */
	{ LNF_ADDR,   0, "%17s ", format_addr }, 				/* default for all LNF_ADDR */
	{ LNF_MAC,    0, "%10s ", NULL },  				/* default for all LNF_MAC */
	{ LNF_UINT64, LNF_FLD_FIRST, "%23s ", format_date }, /* default for all LNF_MAC */
	{ LNF_UINT64, LNF_FLD_LAST, "%10s ", format_date }, /* default for all LNF_MAC */
	{ LNF_DOUBLE, LNF_FLD_CALC_DURATION, "%9s ", format_duration }, 
	{ LNF_UINT64, LNF_FLD_DPKTS, "%9s ", format_uint64_unit }, 
	{ LNF_DOUBLE, LNF_FLD_CALC_BPS, "%8s ", format_double }, 
//	{ LNF_DOUBLE, LNF_FLD_CALC_PPS, "%6s ", format_double }, 
	{ LNF_DOUBLE, LNF_FLD_CALC_BPP, "%7s ", format_double }, 
	{ LNF_UINT64, LNF_FLD_AGGR_FLOWS, "%5s ", format_uint64 } /* aggr flow withou unit */
};

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void print_header() {
	int i;
	char buf[LNF_INFO_BUFSIZE];

	for (i = 0; i < numfields; i++) {
		lnf_fld_info(fields[i].field, LNF_FLD_INFO_NAME, &buf, LNF_INFO_BUFSIZE);
		printf(fields[i].format, &buf);
	}

	printf("\n");
}

void print_row(lnf_rec_t *rec) {
	int i; 
	char buf[MAX_STR];
	char str[MAX_STR];
	char str2[MAX_STR];
	char row[MAX_STR_LONG];

	row[0] = '\0';

	for (i = 0; i < numfields; i++) {
		lnf_rec_fget(rec, fields[i].field, buf);
		if (fields[i].format_func != NULL) {
			fields[i].format_func(str, buf);
		} else {
			strcpy(str, "<?>");
		}
		sprintf(str2, fields[i].format, str);
		strcat(row, str2);
	}

	strcat(row, "\n");

	pthread_mutex_lock(&print_mutex);
	//printf("%s\n", row);
	fputs(row, stdout);
	pthread_mutex_unlock(&print_mutex);
}

int fields_add(int field) {

	field_ent_t *fe;
	format_ent_t fmte;
	int i;

	for (i = 0; i < sizeof(formats) / sizeof(format_ent_t); i++) {
		/* default for all */
		if ( formats[i].type == 0 ) { 	
			memcpy(&fmte, &formats[i], sizeof(format_ent_t));
		}

		/* default for type */
		if ( formats[i].type == lnf_fld_type(field) && formats[i].field == 0) {	
			memcpy(&fmte, &formats[i], sizeof(format_ent_t));
		}

		/* particular type and field */
		if ( formats[i].type == lnf_fld_type(field) && formats[i].field == field) {	
			memcpy(&fmte, &formats[i], sizeof(format_ent_t));
		}
	}

	fe = &fields[numfields];
	fe->field = field;
	strncpy(fe->format, fmte.format, MAX_STR);
	fe->format_func = fmte.format_func;
	numfields++;
	return 1;
}

/* parse argument given by -A */
int parse_aggreg(lnf_mem_t *memp, char *str) {

	char *token = str;
	int field, numbits, numbits6;

	/* default fields on the begining of the list */
	lnf_mem_fastaggr(memp, LNF_FAST_AGGR_BASIC);
//	fields_add(LNF_FLD_FIRST);
//	fields_add(LNF_FLD_CALC_DURATION);

	while ( (token = strsep(&str, ",")) != NULL ) {
		/* parse field */
		field = lnf_fld_parse(token, &numbits, &numbits6);

		if (field == LNF_FLD_ZERO_) {
			fprintf(stderr, "Cannot parse %s in -A \n", token);
			exit(1);
		}

		if (numbits > 32 || numbits6 > 128) {
			fprintf(stderr, "Invalid bit size (%d/%d) for %s in -A \n", 
			numbits, numbits6, token);
			exit(1);
		}	
		
		lnf_mem_fadd(memp, field, LNF_AGGR_KEY, numbits, numbits6);
		fields_add(field);

		token = NULL;
	}

	/* default fields on the ond of the list */
/*
	fields_add(LNF_FLD_DPKTS);
	fields_add(LNF_FLD_DOCTETS);
	fields_add(LNF_FLD_CALC_BPS);
	fields_add(LNF_FLD_CALC_BPP);
	fields_add(LNF_FLD_AGGR_FLOWS);
*/
	return 1;
}

