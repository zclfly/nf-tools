

#include "config.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <netinet/in.h>

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include "nffile.h"
#include "nfx.h"
#include "nfnet.h"
#include "bookkeeper.h"
#include "nfxstat.h"
#include "nf_common.h"
#include "rbtree.h"
#include "nftree.h"
#include "nfprof.h"
#include "nfdump.h"
#include "nflowcache.h"
#include "nfstat.h"
#include "nfexport.h"
#include "ipconv.h"
#include "flist.h"
#include "util.h"

#include "libnf_internal.h"
#include "libnf.h"
#include "fields.h"




/* function definicion for jump table */
/* TAG for check_items_map.pl: lnf_rec_fset */
/* TAG for check_items_map.pl: lnf_rec_fget */
/* ----------------------- */
static int inline lnf_field_fget_FIRST(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint64_t *)p) = m->first * 1000LL + m->msec_first;
	return LNF_OK;
}
static int inline lnf_field_fset_FIRST(master_record_t *m, void *p, bit_array_t *e) { 
	m->first = *((uint64_t *)p) / 1000LL;
	m->msec_first = *((uint64_t *)p) - m->first * 1000LL;
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_LAST(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint64_t *)p) = m->last * 1000LL + m->msec_last;
	return LNF_OK;
}

static int inline lnf_field_fset_LAST(master_record_t *m, void *p, bit_array_t *e) { 
	m->last = *((uint64_t *)p) / 1000LL;
	m->msec_last = *((uint64_t *)p) - m->last * 1000LL;
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_RECEIVED(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint64_t *)p) = m->received;
	return __bit_array_get(e, EX_RECEIVED) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_RECEIVED(master_record_t *m, void *p, bit_array_t *e) { 
	m->received = *((uint64_t *)p);
	__bit_array_set(e, EX_RECEIVED, 1);
	return LNF_OK;	
}

/* ----------------------- */
static int inline lnf_field_fget_DPKTS(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint64_t *)p) = m->dPkts;
	return LNF_OK;
}
static int inline lnf_field_fset_DPKTS(master_record_t *m, void *p, bit_array_t *e) { 
	m->dPkts = *((uint64_t *)p);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_DOCTETS(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint64_t *)p) = m->dOctets;
	return LNF_OK;
}

static int inline lnf_field_fset_DOCTETS(master_record_t *m, void *p, bit_array_t *e) { 
	m->dOctets = *((uint64_t *)p);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_OUT_PKTS(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint64_t *)p) = m->out_pkts;
	return __bit_array_get(e, EX_OUT_PKG_8) ||  __bit_array_get(e, EX_OUT_PKG_4)  ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_OUT_PKTS(master_record_t *m, void *p, bit_array_t *e) { 
	m->out_pkts = *((uint64_t *)p);
	__bit_array_set(e, EX_OUT_PKG_8, 1);
	/* dummy record for check_items_map.pl EX_OUT_PKG_4 */
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_OUT_BYTES(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint64_t *)p) = m->out_bytes;
	return __bit_array_get(e, EX_OUT_BYTES_8) || __bit_array_get(e, EX_OUT_BYTES_4) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_OUT_BYTES(master_record_t *m, void *p, bit_array_t *e) { 
	m->out_bytes = *((uint64_t *)p);
	__bit_array_set(e, EX_OUT_BYTES_8, 1);
	/* dummy record for check_items_map.pl EX_OUT_BYTES_4 */
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_AGGR_FLOWS(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint64_t *)p) = m->aggr_flows;
	return __bit_array_get(e, EX_AGGR_FLOWS_8) || __bit_array_get(e, EX_AGGR_FLOWS_4) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_AGGR_FLOWS(master_record_t *m, void *p, bit_array_t *e) { 
	m->aggr_flows = *((uint64_t *)p);
	__bit_array_set(e, EX_AGGR_FLOWS_8, 1);
	/* dummy record for check_items_map.pl EX_AGGR_FLOWS_4 */
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_SRCPORT(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint16_t *)p) = m->srcport;
	return LNF_OK;
}

static int inline lnf_field_fset_SRCPORT(master_record_t *m, void *p, bit_array_t *e) { 
	m->srcport = *((uint16_t *)p);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_DSTPORT(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint16_t *)p) = m->dstport;
	return LNF_OK;
}

static int inline lnf_field_fset_DSTPORT(master_record_t *m, void *p, bit_array_t *e) { 
	m->dstport = *((uint16_t *)p);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_TCP_FLAGS(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint16_t *)p) = m->tcp_flags;
	return LNF_OK;
}

static int inline lnf_field_fset_TCP_FLAGS(master_record_t *m, void *p, bit_array_t *e) { 
	m->tcp_flags = *((uint8_t *)p);
	return LNF_OK;
}

/* ----------------------- */
// Required extension 1 - IP addresses 
// NOTE: srcaddr and dst addr do not uses ip_addr_t union/structure 
// however the structures are compatible so we will pretend 
// that v6.srcaddr and v6.dst addr points to same structure 
static int inline lnf_field_fget_SRCADDR(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = (ip_addr_t *)&m->v6.srcaddr;
	
	((ip_addr_t *)p)->v6[0] = htonll(d->v6[0]);
	((ip_addr_t *)p)->v6[1] = htonll(d->v6[1]);

	return LNF_OK;
}

static int inline lnf_field_fset_SRCADDR(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = (ip_addr_t *)&m->v6.srcaddr;

	d->v6[0] = ntohll( ((ip_addr_t *)p)->v6[0] );
	d->v6[1] = ntohll( ((ip_addr_t *)p)->v6[1] );

	if (IN6_IS_ADDR_V4COMPAT((struct in6_addr *)p)) {
		ClearFlag(m->flags, FLAG_IPV6_ADDR);
	} else {
		SetFlag(m->flags, FLAG_IPV6_ADDR);
	}
	return LNF_OK;
}


/* ----------------------- */
static int inline lnf_field_fget_DSTADDR(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = (ip_addr_t *)&m->v6.dstaddr;
	
	((ip_addr_t *)p)->v6[0] = htonll(d->v6[0]);
	((ip_addr_t *)p)->v6[1] = htonll(d->v6[1]);

	return LNF_OK;
}

static int inline lnf_field_fset_DSTADDR(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = (ip_addr_t *)&m->v6.dstaddr;

	d->v6[0] = ntohll( ((ip_addr_t *)p)->v6[0] );
	d->v6[1] = ntohll( ((ip_addr_t *)p)->v6[1] );

	if (IN6_IS_ADDR_V4COMPAT((struct in6_addr *)p)) {
		ClearFlag(m->flags, FLAG_IPV6_ADDR);
	} else {
		SetFlag(m->flags, FLAG_IPV6_ADDR);
	}
	return LNF_OK;
}


/* ----------------------- */
static int inline lnf_field_fget_IP_NEXTHOP(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = (ip_addr_t *)&m->ip_nexthop;
	
	((ip_addr_t *)p)->v6[0] = htonll(d->v6[0]);
	((ip_addr_t *)p)->v6[1] = htonll(d->v6[1]);

	return __bit_array_get(e, EX_NEXT_HOP_v4) || __bit_array_get(e, EX_NEXT_HOP_v6) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_IP_NEXTHOP(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = &m->ip_nexthop;

	d->v6[0] = ntohll( ((ip_addr_t *)p)->v6[0] );
	d->v6[1] = ntohll( ((ip_addr_t *)p)->v6[1] );

	if (IN6_IS_ADDR_V4COMPAT((struct in6_addr *)p)) {
		ClearFlag(m->flags, FLAG_IPV6_NH);
		__bit_array_set(e, EX_NEXT_HOP_v4, 1);
	} else {
		SetFlag(m->flags, FLAG_IPV6_NH);
		__bit_array_set(e, EX_NEXT_HOP_v6, 1);
	}
	return LNF_OK;
}


/* ----------------------- */
static int inline lnf_field_fget_SRC_MASK(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint8_t *)p) = m->src_mask;
	return __bit_array_get(e, EX_MULIPLE) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_SRC_MASK(master_record_t *m, void *p, bit_array_t *e) { 
	m->src_mask = *((uint8_t *)p);
	__bit_array_set(e, EX_MULIPLE, 1);
	return LNF_OK;
}


/* ----------------------- */
static int inline lnf_field_fget_DST_MASK(master_record_t *m, void *p, bit_array_t *e) { 
	 *((uint8_t *)p) = m->dst_mask;
	return __bit_array_get(e, EX_MULIPLE) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_DST_MASK(master_record_t *m, void *p, bit_array_t *e) { 
	m->dst_mask = *((uint8_t *)p);
	__bit_array_set(e, EX_MULIPLE, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_TOS(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint8_t *)p) = m->tos;
	return LNF_OK;
}

static int inline lnf_field_fset_TOS(master_record_t *m, void *p, bit_array_t *e) { 
	m->tos = *((uint8_t *)p);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_DST_TOS(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint8_t *)p) = m->dst_tos;
	return __bit_array_get(e, EX_MULIPLE) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_DST_TOS(master_record_t *m, void *p, bit_array_t *e) { 
	m->dst_tos = *((uint8_t *)p);
	__bit_array_set(e, EX_MULIPLE, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_SRCAS(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->srcas;
	return __bit_array_get(e, EX_AS_2) || __bit_array_get(e, EX_AS_4) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_SRCAS(master_record_t *m, void *p, bit_array_t *e) { 
	m->srcas = *((uint32_t *)p);
	__bit_array_set(e, EX_AS_4, 1);
	/* dummy record for check_items_map.pl EX_AS_2 */
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_DSTAS(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->dstas;
	return __bit_array_get(e, EX_AS_2) || __bit_array_get(e, EX_AS_4) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_DSTAS(master_record_t *m, void *p, bit_array_t *e) { 
	m->dstas = *((uint32_t *)p);
	__bit_array_set(e, EX_AS_4, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_BGPNEXTADJACENTAS(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->bgpNextAdjacentAS;
	return __bit_array_get(e, EX_BGPADJ) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_BGPNEXTADJACENTAS(master_record_t *m, void *p, bit_array_t *e) { 
	m->bgpNextAdjacentAS = *((uint32_t *)p);
	__bit_array_set(e, EX_BGPADJ, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_BGPPREVADJACENTAS(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->bgpPrevAdjacentAS;
	return __bit_array_get(e, EX_BGPADJ) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_BGPPREVADJACENTAS(master_record_t *m, void *p, bit_array_t *e) { 
	m->bgpPrevAdjacentAS = *((uint32_t *)p);
	__bit_array_set(e, EX_BGPADJ, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_BGP_NEXTHOP(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = (ip_addr_t *)&m->bgp_nexthop;
	
	((ip_addr_t *)p)->v6[0] = htonll(d->v6[0]);
	((ip_addr_t *)p)->v6[1] = htonll(d->v6[1]);

	return __bit_array_get(e, EX_NEXT_HOP_BGP_v4) || __bit_array_get(e, EX_NEXT_HOP_BGP_v6) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_BGP_NEXTHOP(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = &m->bgp_nexthop;

	d->v6[0] = ntohll( ((ip_addr_t *)p)->v6[0] );
	d->v6[1] = ntohll( ((ip_addr_t *)p)->v6[1] );

	if (IN6_IS_ADDR_V4COMPAT((struct in6_addr *)p)) {
		ClearFlag(m->flags, FLAG_IPV6_NHB);
		__bit_array_set(e, EX_NEXT_HOP_BGP_v4, 1);
	} else {
		SetFlag(m->flags, FLAG_IPV6_NHB);
		__bit_array_set(e, EX_NEXT_HOP_BGP_v6, 1);
	}
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_PROT(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint8_t *)p) = m->prot;
	return LNF_OK;
}

static int inline lnf_field_fset_PROT(master_record_t *m, void *p, bit_array_t *e) { 
	m->prot = *((uint8_t *)p);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_SRC_VLAN(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->src_vlan;
	return __bit_array_get(e, EX_VLAN) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_SRC_VLAN(master_record_t *m, void *p, bit_array_t *e) { 
	m->src_vlan = *((uint32_t *)p);
	__bit_array_set(e, EX_VLAN, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_DST_VLAN(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->dst_vlan;
	return __bit_array_get(e, EX_VLAN) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_DST_VLAN(master_record_t *m, void *p, bit_array_t *e) { 
	m->dst_vlan = *((uint32_t *)p);
	__bit_array_set(e, EX_VLAN, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_IN_SRC_MAC(master_record_t *m, void *p, bit_array_t *e) { 
	int i;
	for (i = 0; i < 6; i++) {
		((uint8_t *)p)[5 - i] = ((uint8_t *)(&m->in_src_mac))[i];
    } 
	return __bit_array_get(e, EX_MAC_1) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_IN_SRC_MAC(master_record_t *m, void *p, bit_array_t *e) { 
	int i;
	m->in_src_mac = 0x0;
	for (i = 0; i < 6; i++) {
		((uint8_t *)(&m->in_src_mac))[5 - i] = ((uint8_t *)p)[i];
	}
	__bit_array_set(e, EX_MAC_1, 1);
	return LNF_OK;
}


/* ----------------------- */
static int inline lnf_field_fget_OUT_DST_MAC(master_record_t *m, void *p, bit_array_t *e) { 
	int i;
	for (i = 0; i < 6; i++) {
		((uint8_t *)p)[5 - i] = ((uint8_t *)(&m->out_dst_mac))[i];
    } 
	return __bit_array_get(e, EX_MAC_1) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_OUT_DST_MAC(master_record_t *m, void *p, bit_array_t *e) { 
	int i;
	m->out_dst_mac = 0x0;
	for (i = 0; i < 6; i++) {
		((uint8_t *)(&m->out_dst_mac))[5 - i] = ((uint8_t *)p)[i];
	}
	__bit_array_set(e, EX_MAC_1, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_OUT_SRC_MAC(master_record_t *m, void *p, bit_array_t *e) { 
	int i;
	for (i = 0; i < 6; i++) {
		((uint8_t *)p)[5 - i] = ((uint8_t *)(&m->out_src_mac))[i];
    } 
	return __bit_array_get(e, EX_MAC_2) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_OUT_SRC_MAC(master_record_t *m, void *p, bit_array_t *e) { 
	int i;
	m->out_src_mac = 0x0;
	for (i = 0; i < 6; i++) {
		((uint8_t *)(&m->out_src_mac))[5 - i] = ((uint8_t *)p)[i];
	}
	__bit_array_set(e, EX_MAC_2, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_IN_DST_MAC(master_record_t *m, void *p, bit_array_t *e) { 
	int i;
	for (i = 0; i < 6; i++) {
		((uint8_t *)p)[5 - i] = ((uint8_t *)(&m->in_dst_mac))[i];
    } 
	return __bit_array_get(e, EX_MAC_2) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_IN_DST_MAC(master_record_t *m, void *p, bit_array_t *e) { 
	int i;
	m->in_dst_mac = 0x0;
	for (i = 0; i < 6; i++) {
		((uint8_t *)(&m->in_dst_mac))[5 - i] = ((uint8_t *)p)[i];
	}
	__bit_array_set(e, EX_MAC_2, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_MPLS_LABEL(master_record_t *m, void *p, bit_array_t *e) { 
	memcpy(p, m->mpls_label, sizeof(lnf_mpls_t));
	return __bit_array_get(e, EX_MPLS) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_MPLS_LABEL(master_record_t *m, void *p, bit_array_t *e) { 
	memcpy(m->mpls_label, p, sizeof(lnf_mpls_t));
	__bit_array_set(e, EX_MPLS, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_INPUT(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->input;
	return __bit_array_get(e, EX_IO_SNMP_2)  || __bit_array_get(e, EX_IO_SNMP_4) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_INPUT(master_record_t *m, void *p, bit_array_t *e) { 
	m->input = *((uint32_t *)p);
	__bit_array_set(e, EX_IO_SNMP_4, 1);
	/* dummy record for check_items_map.pl EX_IO_SNMP_2 */
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_OUTPUT(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) =  m->output;
	return __bit_array_get(e, EX_IO_SNMP_2)  || __bit_array_get(e, EX_IO_SNMP_4) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_OUTPUT(master_record_t *m, void *p, bit_array_t *e) { 
	m->output = *((uint32_t *)p);
	__bit_array_set(e, EX_IO_SNMP_4, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_DIR(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) =  m->dir;
	return __bit_array_get(e, EX_MULIPLE) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_DIR(master_record_t *m, void *p, bit_array_t *e) { 
	m->dir = *((uint32_t *)p);
	__bit_array_set(e, EX_MULIPLE, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_FWD_STATUS(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->fwd_status;
	return LNF_OK;
}

static int inline lnf_field_fset_FWD_STATUS(master_record_t *m, void *p, bit_array_t *e) { 
	m->fwd_status = *((uint32_t *)p);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_IP_ROUTER(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = (ip_addr_t *)&m->ip_router;
	
	((ip_addr_t *)p)->v6[0] = htonll(d->v6[0]);
	((ip_addr_t *)p)->v6[1] = htonll(d->v6[1]);

	return __bit_array_get(e, EX_ROUTER_IP_v4) || __bit_array_get(e, EX_ROUTER_IP_v6) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_IP_ROUTER(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = &m->ip_router;

	d->v6[0] = ntohll( ((ip_addr_t *)p)->v6[0] );
	d->v6[1] = ntohll( ((ip_addr_t *)p)->v6[1] );

	if (IN6_IS_ADDR_V4COMPAT((struct in6_addr *)p)) {
		ClearFlag(m->flags, FLAG_IPV6_EXP);
		__bit_array_set(e,  EX_ROUTER_IP_v4, 1);
	} else {
		SetFlag(m->flags, FLAG_IPV6_EXP);
		__bit_array_set(e,  EX_ROUTER_IP_v6, 1);
	}
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_ENGINE_TYPE(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint8_t *)p) = m->engine_type;
	return __bit_array_get(e, EX_ROUTER_ID) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_ENGINE_TYPE(master_record_t *m, void *p, bit_array_t *e) { 
	m->engine_type = *((uint8_t *)p);
	__bit_array_set(e, EX_ROUTER_ID, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_ENGINE_ID(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint8_t *)p) = m->engine_id;
	return __bit_array_get(e, EX_ROUTER_ID) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_ENGINE_ID(master_record_t *m, void *p, bit_array_t *e) { 
	m->engine_id = *((uint8_t *)p);
	__bit_array_set(e, EX_ROUTER_ID, 1);
	return LNF_OK;
}

/* ----------------------- */
#ifdef NSEL
static int inline lnf_field_fget_EVENT_TIME(master_record_t *m, void *p, bit_array_t *e) { 
		*((uint64_t *)p) = m->event_time;
		return __bit_array_get(e, EX_NSEL_COMMON) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_EVENT_TIME(master_record_t *m, void *p, bit_array_t *e) { 
	m->event_time = *((uint64_t *)p);
	__bit_array_set(e, EX_NSEL_COMMON, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_CONN_ID(master_record_t *m, void *p, bit_array_t *e) { 
		*((uint32_t *)p) = m->conn_id;
		return __bit_array_get(e, EX_NSEL_COMMON) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_CONN_ID(master_record_t *m, void *p, bit_array_t *e) { 
	m->conn_id = *((uint32_t *)p);
	__bit_array_set(e, EX_NSEL_COMMON, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_ICMP_CODE(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint8_t *)p) = m->icmp_code;
	return __bit_array_get(e, EX_NSEL_COMMON) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_ICMP_CODE(master_record_t *m, void *p, bit_array_t *e) { 
	m->icmp_code = *((uint8_t *)p);
	__bit_array_set(e, EX_NSEL_COMMON, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_ICMP_TYPE(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint8_t *)p) = m->icmp_type;
	return __bit_array_get(e, EX_NSEL_COMMON) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_ICMP_TYPE(master_record_t *m, void *p, bit_array_t *e) { 
	m->icmp_type = *((uint8_t *)p);
	__bit_array_set(e, EX_NSEL_COMMON, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_FW_XEVENT(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint16_t *)p) = m->fw_xevent;
	return __bit_array_get(e, EX_NSEL_COMMON) ? LNF_OK : LNF_ERR_NOTSET;
	/* dummy record for check_items_map.pl m->xlate_flags */
}

static int inline lnf_field_fset_FW_XEVENT(master_record_t *m, void *p, bit_array_t *e) { 
	m->fw_xevent = *((uint16_t *)p);
	__bit_array_set(e, EX_NSEL_COMMON, 1);
	return LNF_OK;
	/* dummy record for check_items_map.pl m->xlate_flags */
}

/* ----------------------- */
static int inline lnf_field_fget_XLATE_SRC_IP(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = (ip_addr_t *)&m->xlate_src_ip;
	
	((ip_addr_t *)p)->v6[0] = htonll(d->v6[0]);
	((ip_addr_t *)p)->v6[1] = htonll(d->v6[1]);

	return __bit_array_get(e, EX_NSEL_XLATE_IP_v4) || __bit_array_get(e, EX_NSEL_XLATE_IP_v6) ? LNF_OK : LNF_ERR_NOTSET;
}
		
static int inline lnf_field_fset_XLATE_SRC_IP(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = &m->xlate_src_ip;

	d->v6[0] = ntohll( ((ip_addr_t *)p)->v6[0] );
	d->v6[1] = ntohll( ((ip_addr_t *)p)->v6[1] );

	if (IN6_IS_ADDR_V4COMPAT((struct in6_addr *)p)) {
		__bit_array_set(e,  EX_NSEL_XLATE_IP_v4, 1);
	} else {
		__bit_array_set(e,  EX_NSEL_XLATE_IP_v6, 1);
	}
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_XLATE_DST_IP(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = (ip_addr_t *)&m->xlate_dst_ip;
	
	((ip_addr_t *)p)->v6[0] = htonll(d->v6[0]);
	((ip_addr_t *)p)->v6[1] = htonll(d->v6[1]);

	return __bit_array_get(e, EX_NSEL_XLATE_IP_v4) || __bit_array_get(e, EX_NSEL_XLATE_IP_v6) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_XLATE_DST_IP(master_record_t *m, void *p, bit_array_t *e) { 
	ip_addr_t *d = &m->xlate_dst_ip;

	d->v6[0] = ntohll( ((ip_addr_t *)p)->v6[0] );
	d->v6[1] = ntohll( ((ip_addr_t *)p)->v6[1] );

	if (IN6_IS_ADDR_V4COMPAT((struct in6_addr *)p)) {
		__bit_array_set(e,  EX_NSEL_XLATE_IP_v4, 1);
	} else {
		__bit_array_set(e,  EX_NSEL_XLATE_IP_v6, 1);
	}
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_XLATE_SRC_PORT(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint16_t *)p) = m->xlate_src_port;
	return __bit_array_get(e, EX_NSEL_XLATE_PORTS) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_XLATE_SRC_PORT(master_record_t *m, void *p, bit_array_t *e) { 
	m->xlate_src_port = *((uint16_t *)p);
	__bit_array_set(e, EX_NSEL_XLATE_PORTS, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_XLATE_DST_PORT(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint16_t *)p) = m->xlate_dst_port;
	return __bit_array_get(e, EX_NSEL_XLATE_PORTS) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_XLATE_DST_PORT(master_record_t *m, void *p, bit_array_t *e) { 
	m->xlate_dst_port = *((uint16_t *)p);
	__bit_array_set(e, EX_NSEL_XLATE_PORTS, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_INGRESS_ACL_ID(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->ingress_acl_id[0];
	return __bit_array_get(e, EX_NSEL_ACL) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_INGRESS_ACL_ID(master_record_t *m, void *p, bit_array_t *e) { 
	m->ingress_acl_id[0] = *((uint32_t *)p);
	__bit_array_set(e, EX_NSEL_ACL, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_INGRESS_ACE_ID(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->ingress_acl_id[1];
	return __bit_array_get(e, EX_NSEL_ACL) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_INGRESS_ACE_ID(master_record_t *m, void *p, bit_array_t *e) { 
	m->ingress_acl_id[1] = *((uint32_t *)p);
	__bit_array_set(e, EX_NSEL_ACL, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_INGRESS_XACE_ID(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->ingress_acl_id[2];
	return __bit_array_get(e, EX_NSEL_ACL) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_INGRESS_XACE_ID(master_record_t *m, void *p, bit_array_t *e) { 
	m->ingress_acl_id[2] = *((uint32_t *)p);
	__bit_array_set(e, EX_NSEL_ACL, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_EGRESS_ACL_ID(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->egress_acl_id[0];
	return __bit_array_get(e, EX_NSEL_ACL) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_EGRESS_ACL_ID(master_record_t *m, void *p, bit_array_t *e) { 
	m->egress_acl_id[0] = *((uint32_t *)p);
	__bit_array_set(e, EX_NSEL_ACL, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_EGRESS_ACE_ID(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->egress_acl_id[1];
	return __bit_array_get(e, EX_NSEL_ACL) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_EGRESS_ACE_ID(master_record_t *m, void *p, bit_array_t *e) { 
	m->egress_acl_id[1] = *((uint32_t *)p);
	__bit_array_set(e, EX_NSEL_ACL, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_EGRESS_XACE_ID(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->egress_acl_id[2];
	return __bit_array_get(e, EX_NSEL_ACL) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_EGRESS_XACE_ID(master_record_t *m, void *p, bit_array_t *e) { 
	m->egress_acl_id[2] = *((uint32_t *)p);
	__bit_array_set(e, EX_NSEL_ACL, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_USERNAME(master_record_t *m, void *p, bit_array_t *e) { 
	memcpy(p, m->username, strlen(m->username) + 1);
	return __bit_array_get(e, EX_NSEL_USER) || __bit_array_get(e, EX_NSEL_USER_MAX) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_USERNAME(master_record_t *m, void *p, bit_array_t *e) { 
	int len;

	len = strlen((char *)p);
	if ( len > sizeof(m->username) -  1 ) {
		len = sizeof(m->username) - 1;
	}

	memcpy(m->username, p, len );
	m->username[len] = '\0';

	if ( len < sizeof(((struct tpl_ext_42_s *)0)->username) - 1 ) {
		__bit_array_set(e, EX_NSEL_USER, 1);
	} else {
		__bit_array_set(e, EX_NSEL_USER_MAX, 1);
	}
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_INGRESS_VRFID(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->ingress_vrfid;
	return __bit_array_get(e, EX_NEL_COMMON) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_INGRESS_VRFID(master_record_t *m, void *p, bit_array_t *e) { 
	m->ingress_vrfid = *((uint32_t *)p);
	__bit_array_set(e, EX_NEL_COMMON, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_EVENT_FLAG(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint8_t *)p) = m->event_flag;
	return __bit_array_get(e, EX_NEL_COMMON) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_EVENT_FLAG(master_record_t *m, void *p, bit_array_t *e) { 
	m->event_flag = *((uint8_t *)p);
	__bit_array_set(e, EX_NEL_COMMON, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_EGRESS_VRFID(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint32_t *)p) = m->egress_vrfid;
	return __bit_array_get(e, EX_NEL_COMMON) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_EGRESS_VRFID(master_record_t *m, void *p, bit_array_t *e) { 
	m->egress_vrfid = *((uint32_t *)p);
	__bit_array_set(e, EX_NEL_COMMON, 1);
	return LNF_OK;
}

/* ----------------------- */
// EX_PORT_BLOCK_ALLOC added 2014-04-19
static int inline lnf_field_fget_BLOCK_START(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint16_t *)p) = m->block_start;
	return __bit_array_get(e, EX_PORT_BLOCK_ALLOC) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_BLOCK_START(master_record_t *m, void *p, bit_array_t *e) { 
	m->block_start = *((uint16_t *)p);
	__bit_array_set(e, EX_PORT_BLOCK_ALLOC, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_BLOCK_END(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint16_t *)p) = m->block_end;
	return __bit_array_get(e, EX_PORT_BLOCK_ALLOC) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_BLOCK_END(master_record_t *m, void *p, bit_array_t *e) { 
	m->block_end = *((uint16_t *)p);
	__bit_array_set(e, EX_PORT_BLOCK_ALLOC, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_BLOCK_STEP(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint16_t *)p) = m->block_step;
	return __bit_array_get(e, EX_PORT_BLOCK_ALLOC) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_BLOCK_STEP(master_record_t *m, void *p, bit_array_t *e) { 
	m->block_step = *((uint16_t *)p);
	__bit_array_set(e, EX_PORT_BLOCK_ALLOC, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_BLOCK_SIZE(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint16_t *)p) = m->block_size;
	return __bit_array_get(e, EX_PORT_BLOCK_ALLOC) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_BLOCK_SIZE(master_record_t *m, void *p, bit_array_t *e) { 
	m->block_size = *((uint16_t *)p);
	__bit_array_set(e, EX_PORT_BLOCK_ALLOC, 1);
	return LNF_OK;
}

#endif
/* ----------------------- */
		// extra fields
static int inline lnf_field_fget_CLIENT_NW_DELAY_USEC(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint64_t *)p) = m->client_nw_delay_usec;
	return __bit_array_get(e, EX_LATENCY) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_CLIENT_NW_DELAY_USEC(master_record_t *m, void *p, bit_array_t *e) { 
	m->client_nw_delay_usec = *((uint64_t *)p);
	__bit_array_set(e, EX_LATENCY, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_SERVER_NW_DELAY_USEC(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint64_t *)p) = m->server_nw_delay_usec;
	return __bit_array_get(e, EX_LATENCY) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_SERVER_NW_DELAY_USEC(master_record_t *m, void *p, bit_array_t *e) { 
	m->server_nw_delay_usec = *((uint64_t *)p);
	__bit_array_set(e, EX_LATENCY, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_APPL_LATENCY_USEC(master_record_t *m, void *p, bit_array_t *e) { 
	*((uint64_t *)p) = m->appl_latency_usec;
	return __bit_array_get(e, EX_LATENCY) ? LNF_OK : LNF_ERR_NOTSET;
}

static int inline lnf_field_fset_APPL_LATENCY_USEC(master_record_t *m, void *p, bit_array_t *e) { 
	m->appl_latency_usec = *((uint64_t *)p);
	__bit_array_set(e, EX_LATENCY, 1);
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fset_EMPTY_(master_record_t *m, void *p, bit_array_t *e) { 
	return LNF_OK;
}

#define LNF_DURATION ((m->last * 1000LL + m->msec_last) - (m->first * 1000LL + m->msec_first))

/* ----------------------- */
static int inline lnf_field_fget_CALC_DURATION(master_record_t *m, void *p, bit_array_t *e) { 
	*((double *)p) = LNF_DURATION;
	return LNF_OK;
}

/* ----------------------- */
static int inline lnf_field_fget_CALC_BPS(master_record_t *m, void *p, bit_array_t *e) { 
	if (LNF_DURATION > 0) {
		*((double *)p) = m->dOctets / LNF_DURATION / 1000 * 8;
		return LNF_OK;
	} else {
		*((double *)p) = 0;
		return LNF_ERR_NOTSET;
	}
}

/* ----------------------- */
static int inline lnf_field_fget_CALC_PPS(master_record_t *m, void *p, bit_array_t *e) { 
	if (LNF_DURATION > 0) {
		*((double *)p) = m->dPkts / LNF_DURATION / 1000 * 8;
		return LNF_OK;
	} else {
		*((double *)p) = 0;
		return LNF_ERR_NOTSET;
	}
}

/* ----------------------- */
static int inline lnf_field_fget_CALC_BPP(master_record_t *m, void *p, bit_array_t *e) { 
	if (m->dPkts > 0) {
		*((double *)p) = m->dOctets / m->dPkts;
		return LNF_OK;
	} else {
		return LNF_ERR_NOTSET;
	}
}

/* ----------------------- */
static int inline lnf_field_fget_BREC1(master_record_t *m, void *p, bit_array_t *e) { 
	lnf_brec1_t *brec1 = p;
	lnf_field_fget_FIRST(m, &brec1->first, e);
	lnf_field_fget_LAST(m, &brec1->last, e);
	lnf_field_fget_SRCADDR(m, &brec1->srcaddr, e);
	lnf_field_fget_DSTADDR(m, &brec1->dstaddr, e);
	lnf_field_fget_PROT(m, &brec1->prot, e);
	lnf_field_fget_SRCPORT(m, &brec1->srcport, e);
	lnf_field_fget_DSTPORT(m, &brec1->dstport, e);
	lnf_field_fget_DOCTETS(m, &brec1->bytes, e);
	lnf_field_fget_DPKTS(m, &brec1->pkts, e);
	lnf_field_fget_AGGR_FLOWS(m, &brec1->flows, e);
	return LNF_OK;
}


static int inline lnf_field_fset_BREC1(master_record_t *m, void *p, bit_array_t *e) { 
	lnf_brec1_t *brec1 = p;

	lnf_field_fset_FIRST(m, &brec1->first, e);
	lnf_field_fset_LAST(m, &brec1->last, e);
	lnf_field_fset_SRCADDR(m, &brec1->srcaddr, e);
	lnf_field_fset_DSTADDR(m, &brec1->dstaddr, e);
	lnf_field_fset_PROT(m, &brec1->prot, e);
	lnf_field_fset_SRCPORT(m, &brec1->srcport, e);
	lnf_field_fset_DSTPORT(m, &brec1->dstport, e);
	lnf_field_fset_DOCTETS(m, &brec1->bytes, e);
	lnf_field_fset_DPKTS(m, &brec1->pkts, e);
	lnf_field_fset_AGGR_FLOWS(m, &brec1->flows, e);
	return LNF_OK;

}
/* ----------------------- */

/* text description of the fields */
lnf_field_def_t lnf_fields_def[] = {
// pod:  =====================
	[LNF_FLD_FIRST] = {
		LNF_UINT64,	LNF_AGGR_MIN,	LNF_SORT_ASC,	
		"first",	"Timestamp of the first packet seen (in miliseconds)",
		lnf_field_fget_FIRST,
		lnf_field_fset_FIRST },

	[LNF_FLD_LAST] = {
		LNF_UINT64,	LNF_AGGR_MAX,	LNF_SORT_ASC,	
		"last",		"Timestamp of the last packet seen (in miliseconds)",
		lnf_field_fget_LAST,
		lnf_field_fset_LAST },

	[LNF_FLD_RECEIVED] = {
		LNF_UINT64,	LNF_AGGR_MAX,	LNF_SORT_ASC,	
		"received",	"Timestamp regarding when the packet was received by collector",
		lnf_field_fget_RECEIVED,
		lnf_field_fset_RECEIVED },
		
// pod:
// pod:  Statistical items
// pod:  =====================
	[LNF_FLD_DOCTETS] = {
		LNF_UINT64,		LNF_AGGR_SUM,	LNF_SORT_DESC,	
		"bytes",	"The number of bytes",
		lnf_field_fget_DOCTETS,
		lnf_field_fset_DOCTETS},

	[LNF_FLD_DPKTS] = {
		LNF_UINT64,			LNF_AGGR_SUM,	LNF_SORT_DESC,	
		"pkts",		"The number of packets",
		lnf_field_fget_DPKTS,
		lnf_field_fset_DPKTS},

	[LNF_FLD_OUT_BYTES] = {
		LNF_UINT64,		LNF_AGGR_SUM,	LNF_SORT_DESC,	
		"outbytes",	"The number of output bytes",
		lnf_field_fget_OUT_BYTES,
		lnf_field_fset_OUT_BYTES},

	[LNF_FLD_OUT_PKTS] = { 
		LNF_UINT64,		LNF_AGGR_SUM,	LNF_SORT_DESC,	
		"outpkts",	"The number of output packets",
		lnf_field_fget_OUT_PKTS,
		lnf_field_fset_OUT_PKTS},

	[LNF_FLD_AGGR_FLOWS] = {
		LNF_UINT64,	LNF_AGGR_SUM,	LNF_SORT_DESC,	
		"flows",	"The number of flows (aggregated)",
		lnf_field_fget_AGGR_FLOWS,
		lnf_field_fset_AGGR_FLOWS},

// pod:
// pod:  Layer 4 information
// pod:  =====================
	[LNF_FLD_SRCPORT] = {
		LNF_UINT16,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"srcport",		"Source port",
		lnf_field_fget_SRCPORT,
		lnf_field_fset_SRCPORT},

	[LNF_FLD_DSTPORT] = {
		LNF_UINT16, 		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"dstport",		"Destination port",
		lnf_field_fget_DSTPORT,
		lnf_field_fset_DSTPORT},

	[LNF_FLD_TCP_FLAGS] = {
		LNF_UINT8,		LNF_AGGR_OR,	LNF_SORT_ASC,	
		"tcpflags",		"TCP flags",
		lnf_field_fget_TCP_FLAGS,
		lnf_field_fset_TCP_FLAGS},

// pod}:
// pod:  Layer 3 information
// pod:  =====================
	[LNF_FLD_SRCADDR ] = {
		LNF_ADDR,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"srcip",	"Source IP address",
		lnf_field_fget_SRCADDR,
		lnf_field_fset_SRCADDR},

	[LNF_FLD_DSTADDR] = {
		LNF_ADDR,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"dstip",	"Destination IP address",
		lnf_field_fget_DSTADDR,
		lnf_field_fset_DSTADDR},

	[LNF_FLD_IP_NEXTHOP] = {
		LNF_ADDR,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"nexthop",		"IP next hop",
		lnf_field_fget_IP_NEXTHOP,
		lnf_field_fset_IP_NEXTHOP},

	[LNF_FLD_SRC_MASK] = {
		LNF_UINT8,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"srcmask",		"Source mask",
		lnf_field_fget_SRC_MASK, 
		lnf_field_fset_SRC_MASK}, 

	[LNF_FLD_DST_MASK] = {
		LNF_UINT8,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"dstmask",		"Destination mask",
		lnf_field_fget_DST_MASK, 
		lnf_field_fset_DST_MASK}, 

	[LNF_FLD_TOS] = {
		LNF_UINT8,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"tos",		"Source type of service",
		lnf_field_fget_TOS, 
		lnf_field_fset_TOS}, 

	[LNF_FLD_DST_TOS] = {
		LNF_UINT8,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"dsttos",	"Destination type of service",
		lnf_field_fget_DST_TOS,
		lnf_field_fset_DST_TOS},

	[LNF_FLD_SRCAS] = {
		LNF_UINT32,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"srcas",	"Source AS number",
		lnf_field_fget_SRCAS,
		lnf_field_fset_SRCAS},

	[LNF_FLD_DSTAS] = {
		LNF_UINT32,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"dstas",	"Destination AS number",
		lnf_field_fget_DSTAS,
		lnf_field_fset_DSTAS},

	[LNF_FLD_BGPNEXTADJACENTAS] = {
		LNF_UINT32,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"nextas",	"BGP Next AS",
		lnf_field_fget_BGPNEXTADJACENTAS,
		lnf_field_fset_BGPNEXTADJACENTAS},

	[LNF_FLD_BGPPREVADJACENTAS] = {
		LNF_UINT32,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"prevas",	"BGP Previous AS",
		lnf_field_fget_BGPPREVADJACENTAS,
		lnf_field_fset_BGPPREVADJACENTAS},

	[LNF_FLD_BGP_NEXTHOP] = {
		LNF_ADDR,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"bgpnexthop",	"BGP next hop",
		lnf_field_fget_BGP_NEXTHOP,
		lnf_field_fset_BGP_NEXTHOP},

	[LNF_FLD_PROT] = {
		LNF_UINT8,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"proto",	"IP protocol",
		lnf_field_fget_PROT, 
		lnf_field_fset_PROT}, 

// pod:
// pod:  Layer 2 information
// pod:  =====================
	[LNF_FLD_SRC_VLAN] = {
		LNF_UINT8,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"srcvlan",		"Source vlan label",
		lnf_field_fget_SRC_VLAN,
		lnf_field_fset_SRC_VLAN},

	[LNF_FLD_DST_VLAN] = {
		LNF_UINT8,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"dstvlan",		"Destination vlan label",
		lnf_field_fget_DST_VLAN, 
		lnf_field_fset_DST_VLAN}, 

	[LNF_FLD_IN_SRC_MAC] = {
		LNF_MAC,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"insrcmac",		"In source MAC address",
		lnf_field_fget_IN_SRC_MAC,
		lnf_field_fset_IN_SRC_MAC},

	[LNF_FLD_OUT_SRC_MAC] = {
		LNF_MAC,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"outsrcmac",	"Out destination MAC address",
		lnf_field_fget_OUT_SRC_MAC,
		lnf_field_fset_OUT_SRC_MAC},

	[LNF_FLD_IN_DST_MAC] = {
		LNF_MAC,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"indstmac",	"In destination MAC address",
		lnf_field_fget_IN_DST_MAC, 
		lnf_field_fset_IN_DST_MAC}, 

	[LNF_FLD_OUT_DST_MAC] = {
		LNF_MAC,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"outdstmac",	"Out source MAC address",
		lnf_field_fget_OUT_DST_MAC, 
		lnf_field_fset_OUT_DST_MAC}, 

// pod:
// pod:  MPLS information
// pod:  =====================
	[LNF_FLD_MPLS_LABEL] = {
		LNF_MPLS,	LNF_AGGR_KEY,	LNF_SORT_NONE,	
		"mpls",		"MPLS labels",
		lnf_field_fget_MPLS_LABEL,
		lnf_field_fset_MPLS_LABEL},

// pod:
// pod:  Layer 1 information
// pod:  =====================
	[LNF_FLD_INPUT] = {
		LNF_UINT16,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"inif",		"SNMP input interface number",
		lnf_field_fget_INPUT,
		lnf_field_fset_INPUT},

	[LNF_FLD_OUTPUT] = {
		LNF_UINT16,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"outif",	"SNMP output interface number",
		lnf_field_fget_OUTPUT,
		lnf_field_fset_OUTPUT},

	[LNF_FLD_DIR] = {
		LNF_UINT8,			LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"dir",		"Flow directions ingress/egress",
		lnf_field_fget_DIR, 
		lnf_field_fset_DIR}, 

	[LNF_FLD_FWD_STATUS] = {
		LNF_UINT8,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"fwd",		"Forwarding status",
		lnf_field_fget_FWD_STATUS,
		lnf_field_fset_FWD_STATUS},
// pod:
// pod:  Exporter information
// pod:  =====================
	[LNF_FLD_IP_ROUTER] = {
		LNF_ADDR,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"router",	"Exporting router IP",
		lnf_field_fget_IP_ROUTER, 
		lnf_field_fset_IP_ROUTER}, 

	[LNF_FLD_ENGINE_TYPE] = {
		LNF_UINT8,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"systype",	"Type of exporter",
		lnf_field_fget_ENGINE_TYPE,
		lnf_field_fset_ENGINE_TYPE},

	[LNF_FLD_ENGINE_ID] = {
		LNF_UINT8,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"sysid",	"Internal SysID of exporter",
		lnf_field_fget_ENGINE_ID,
		lnf_field_fset_ENGINE_ID},

// pod:
// pod:  NSEL fields, see: http://www.cisco.com/en/US/docs/security/asa/asa81/netflow/netflow.html
// pod:  =====================
	[LNF_FLD_EVENT_TIME] = {
		LNF_UINT64,		LNF_AGGR_MIN,	LNF_SORT_ASC,	
		"eventtime",	"NSEL The time that the flow was created",
		lnf_field_fget_EVENT_TIME,
		lnf_field_fset_EVENT_TIME},

	[LNF_FLD_CONN_ID] = {
		LNF_UINT32,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"connid",		"NSEL An identifier of a unique flow for the device",
		lnf_field_fget_CONN_ID,
		lnf_field_fset_CONN_ID},

	[LNF_FLD_ICMP_CODE] = {
		LNF_UINT8,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"icmpcode",		"NSEL ICMP code value",
		lnf_field_fget_ICMP_CODE,
		lnf_field_fset_ICMP_CODE},

	[LNF_FLD_ICMP_TYPE] = {
		LNF_UINT8,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"icmptype",		"NSEL ICMP type value",
		lnf_field_fget_ICMP_TYPE,
		lnf_field_fset_ICMP_TYPE},

	[LNF_FLD_FW_XEVENT] = {
		LNF_UINT16,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"xevent",		"NSEL Extended event code",
		lnf_field_fget_FW_XEVENT,
		lnf_field_fset_FW_XEVENT},

	[LNF_FLD_XLATE_SRC_IP] = {
		LNF_ADDR,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"xsrcip",	"NSEL Mapped source IPv4 address",
		lnf_field_fget_XLATE_SRC_IP,
		lnf_field_fset_XLATE_SRC_IP},

	[LNF_FLD_XLATE_DST_IP] = {
		LNF_ADDR,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"xdstip",	"NSEL Mapped destination IPv4 address",
		lnf_field_fget_XLATE_DST_IP,
		lnf_field_fset_XLATE_DST_IP},

	[LNF_FLD_XLATE_SRC_PORT] = {
		LNF_UINT16,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"xsrcport",	"NSEL Mapped source port",
		lnf_field_fget_XLATE_SRC_PORT,
		lnf_field_fset_XLATE_SRC_PORT},

	[LNF_FLD_XLATE_DST_PORT] = {
		LNF_UINT16,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"xdstport",	"NSEL Mapped destination port",
		lnf_field_fget_XLATE_DST_PORT,
		lnf_field_fset_XLATE_DST_PORT},

// pod: NSEL The input ACL that permitted or denied the flow
	[LNF_FLD_INGRESS_ACL_ID] = {
		LNF_UINT32,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"iacl",		"Hash value or ID of the ACL name",
		lnf_field_fget_INGRESS_ACL_ID,
		lnf_field_fset_INGRESS_ACL_ID},

	[LNF_FLD_INGRESS_ACE_ID] = {
		LNF_UINT32,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"iace", 	"Hash value or ID of the ACL name",
		lnf_field_fget_INGRESS_ACE_ID,
		lnf_field_fset_INGRESS_ACE_ID},

	[LNF_FLD_INGRESS_XACE_ID] = {
		LNF_UINT32,	LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"ixace",	"Hash value or ID of an extended ACE configuration",
		lnf_field_fget_INGRESS_XACE_ID,
		lnf_field_fset_INGRESS_XACE_ID},

// pod: NSEL The output ACL that permitted or denied a flow  
	[LNF_FLD_EGRESS_ACL_ID] = {
		LNF_UINT32,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"eacl",		"Hash value or ID of the ACL name",
		lnf_field_fget_EGRESS_ACL_ID,
		lnf_field_fset_EGRESS_ACL_ID},

	[LNF_FLD_EGRESS_ACE_ID] = {
		LNF_UINT32,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"eace",		"Hash value or ID of the ACL name",
		lnf_field_fget_EGRESS_ACE_ID,
		lnf_field_fset_EGRESS_ACE_ID},

	[LNF_FLD_EGRESS_XACE_ID] = {
		LNF_UINT32,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"exace",	"Hash value or ID of an extended ACE configuration",
		lnf_field_fget_EGRESS_XACE_ID,
		lnf_field_fset_EGRESS_XACE_ID},

	[LNF_FLD_USERNAME] = {
		LNF_STRING,			LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"username",	"NSEL username",
		lnf_field_fget_USERNAME,
		lnf_field_fset_USERNAME},

// pod:
// pod:  NEL (NetFlow Event Logging) fields
// pod:  =====================
	[LNF_FLD_INGRESS_VRFID] = {
		LNF_UINT32,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"ingressvrfid",		"NEL NAT ingress vrf id",
		lnf_field_fget_INGRESS_VRFID,
		lnf_field_fset_INGRESS_VRFID},

	[LNF_FLD_EVENT_FLAG] = {
		LNF_UINT8,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"eventflag",		"NAT event flag (always set to 1 by nfdump)",
		lnf_field_fget_EVENT_FLAG,
		lnf_field_fset_EVENT_FLAG},

	[LNF_FLD_EGRESS_VRFID] = {
		LNF_UINT32,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"egressvrfid",		"NAT egress VRF ID",
		lnf_field_fget_EGRESS_VRFID,
		lnf_field_fset_EGRESS_VRFID},

	[LNF_FLD_BLOCK_START] = {
		LNF_UINT16,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"blockstart",		"NAT pool block start",
		lnf_field_fget_BLOCK_START,
		lnf_field_fset_BLOCK_START},

// pod:
// pod:  NEL Port Block Allocation (added 2014-04-19)
// pod:  =====================
	[LNF_FLD_BLOCK_END] = {
		LNF_UINT16,			LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"blockend",			"NAT pool block end",
		lnf_field_fget_BLOCK_END,
		lnf_field_fset_BLOCK_END},

	[LNF_FLD_BLOCK_STEP] = {
		LNF_UINT16,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"blockstep",		"NAT pool block step",
		lnf_field_fget_BLOCK_STEP,
		lnf_field_fset_BLOCK_STEP},

	[LNF_FLD_BLOCK_SIZE] = {
		LNF_UINT16,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"blocksize",		"NAT pool block size",
		lnf_field_fget_BLOCK_SIZE,
		lnf_field_fset_BLOCK_SIZE},
// pod:
// pod:  Extra/special fields
// pod:  =====================
	[LNF_FLD_CLIENT_NW_DELAY_USEC] = {
		LNF_UINT64,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"cl",	"nprobe latency client_nw_delay_usec",
		lnf_field_fget_CLIENT_NW_DELAY_USEC,
		lnf_field_fset_CLIENT_NW_DELAY_USEC},

	[LNF_FLD_SERVER_NW_DELAY_USEC] = {
		LNF_UINT64,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"sl",	"nprobe latency server_nw_delay_usec",
		lnf_field_fget_SERVER_NW_DELAY_USEC,
		lnf_field_fset_SERVER_NW_DELAY_USEC},

	[LNF_FLD_APPL_LATENCY_USEC] = {
		LNF_UINT64,		LNF_AGGR_KEY,	LNF_SORT_ASC,	
		"al",	"nprobe latency appl_latency_usec",
		lnf_field_fget_APPL_LATENCY_USEC,
		lnf_field_fset_APPL_LATENCY_USEC},

// pod:
// pod:  Calculated items
// pod:  =====================
	[LNF_FLD_CALC_DURATION] = {
		LNF_DOUBLE,		LNF_AGGR_SUM,	LNF_SORT_NONE,	
		"duration",	"Flow duration",
		lnf_field_fget_CALC_DURATION,
		lnf_field_fset_EMPTY_},

	[LNF_FLD_CALC_BPS] = {
		LNF_DOUBLE,		LNF_AGGR_SUM,	LNF_SORT_NONE,	
		"bps",	"Bytes per second",
		lnf_field_fget_CALC_BPS,
		lnf_field_fset_EMPTY_},

	[LNF_FLD_CALC_PPS] = {
		LNF_DOUBLE,		LNF_AGGR_SUM,	LNF_SORT_NONE,	
		"pps",	"Packets per second",
		lnf_field_fget_CALC_PPS,
		lnf_field_fset_EMPTY_},

	[LNF_FLD_CALC_BPP] = {
		LNF_DOUBLE,		LNF_AGGR_SUM,	LNF_SORT_NONE,	
		"bpp",	"Bytes per packet",
		lnf_field_fget_CALC_BPP,
		lnf_field_fset_EMPTY_},

// pod:
// pod:  Structure lnf_brec1_t - basic record 
// pod:  =====================
	[LNF_FLD_BREC1] = {
		LNF_BASIC_RECORD1,		LNF_AGGR_KEY,	LNF_SORT_NONE,	
		"brec1",	"basic record 1",
		lnf_field_fget_BREC1,
		lnf_field_fset_BREC1},

	[LNF_FLD_TERM_] = {
		0,				0,				0,				
		NULL,	NULL,
		NULL, 
		NULL}
};


/* return info information for fiels 
*/
int lnf_fld_info(int field, int info, void *data, size_t size) {

	lnf_field_def_t *fe;
	int i;
	size_t reqsize; /* requested minimum size */
	char buf[LNF_INFO_BUFSIZE];

	if (info == LNF_FLD_INFO_FIELDS) {
		int *fld  = buf;
		int x = 0;
		/* find ID for field */
		for (i = LNF_FLD_ZERO_; i < LNF_FLD_TERM_; i++) {
			if (lnf_fields_def[i].name != NULL) {
				*fld = i;
				fld++;
				x++;
			}
		}
		*fld = LNF_FLD_TERM_;
		x++;

		if (size < sizeof(int) * x) {
			return LNF_ERR_NOMEM;
		} else {
			memcpy(data, buf, sizeof(int) * x);
			return LNF_OK;
		}
	}

	if (field < LNF_FLD_ZERO_ || field > LNF_FLD_TERM_) {
		return LNF_ERR_UNKFLD;
	}

	fe = &lnf_fields_def[field];

	switch (info) {
		case LNF_FLD_INFO_TYPE:
			*((int *)buf) = fe->type;
			reqsize = sizeof(fe->type);
			break;
		case LNF_FLD_INFO_NAME:
			strcpy(buf, fe->name);
			reqsize = strlen(fe->name) + 1;
			break;
		case LNF_FLD_INFO_DESCR:
			strcpy(buf, fe->fld_descr);
			reqsize = strlen(fe->fld_descr) + 1;
			break;
		case LNF_FLD_INFO_AGGR:
			*((int *)buf) = fe->default_aggr;
			reqsize = sizeof(fe->default_aggr);
			break;
		case LNF_FLD_INFO_SORT:
			*((int *)buf) = fe->default_sort;
			reqsize = sizeof(fe->default_sort);
			break;
		default:
			return LNF_ERR_OTHER;
	}

	if (reqsize <= size) {
		memcpy(data, buf, reqsize);
		return LNF_OK;
	} else {
		return LNF_ERR_NOMEM;
	}
}

/* parse fields from string 
* accepted format like srcip/24/64 
*/
int lnf_fld_parse(char *str, int *numbits, int *numbits6) {

	char *name, *strbits;
	int field = LNF_FLD_ZERO_;
	char lastch;
	int i;


	/* find first token */
	name = strsep(&str, "/");

	if (name == NULL) {	
		return LNF_ERR_OTHER;
	}

	lastch = name[strlen(name) - 1];

	/* symbol 4 or 6 on last position */
	if (lastch == '4' || lastch == '6') {
		name[strlen(name) - 1] = '\0';
	}

	/* find ID for field */
	for (i = LNF_FLD_ZERO_; i < LNF_FLD_TERM_; i++) {
		if (lnf_fields_def[i].name != NULL) {
			if (strcmp(name, lnf_fields_def[i].name) == 0) {
				field = i;
				break;
			}
		}
	}

	if (field == LNF_FLD_ZERO_) {
		return LNF_FLD_ZERO_;
	}

	if (lnf_fld_type(field) != LNF_ADDR) {
		if (numbits != NULL) { *numbits = 0; }
		if (numbits6 != NULL) { *numbits6 = 0; }
		return field;
	}
		
	if (numbits != NULL) { *numbits = 32; }
	if (numbits6 != NULL) { *numbits6 = 128; }

	/* numbits */
	if (str != NULL) {
		strbits = strsep(&str, "/");
		if (strbits != NULL && numbits != NULL) {
			if (lastch == '6') {
				*numbits6 = strtol(strbits, NULL, 10);
			} else {
				*numbits = strtol(strbits, NULL, 10);
			}
		}
	}
	
	/* numbits6 */
	if (str != NULL && numbits6 != NULL) {
		if (lastch == '6') {
			*numbits = strtol(strbits, NULL, 10);
		} else {
			*numbits6 = strtol(str, NULL, 10);
		}
	}

	return field;
}

