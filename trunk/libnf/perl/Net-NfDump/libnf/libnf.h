
/* 
* This .h file is also used to create some parts of libnf documentation 
* the text after the macro pod<double dot> will be placed into separate file libnf.h.pod 
* and then is included into basic libnf documentation 
*/

#include <stdio.h>
#include <stdlib.h>
#include <nffile.h>
#include <nfx.h>
#include "bit_array.h"


/* multiple use of version for both perl and nfdump so we redefine it */
#define NFL_VERSION VRESION
#undef VERSION


/* string prefix for error and warning outputs */
#define NFL_LOG			"Net::NfDump: "

/* type of fields */
/* note: if the fields type allows two kind of data type  */
/* for example UINT32 and UINT64 libnf always uses the biggest one */
#define LNF_UINT8			0x08
#define LNF_UINT16			0x16
#define LNF_UINT32			0x32
#define LNF_UINT64			0x64
#define LNF_ADDR 			0xA1	/* 128 bit addr */
#define LNF_MAC				0xA2
#define LNF_STRING			0xAA	/* null terminated string */
#define LNF_MPLS			0xAB	/* mpls labels */
#define LNF_BASIC_RECORD	0xB1


#define LNF_MASK_TYPE  		0x000000FF
#define LNF_GET_TYPE(x) 	(x & LNF_MASK_TYPE)

/* top two bytes of field identifies data type LNF_UINT8, ... */

#define NFL_FLD_ZERO		0x00
#define LNF_FLD_FIRST		0x010064
#define LNF_FLD_LAST		0x020064
#define LNF_FLD_RECEIVED	0x030064
#define LNF_FLD_DOCTETS		0x040064
#define LNF_FLD_DPKTS		0x050064
#define LNF_FLD_OUT_BYTES	0x060064
#define LNF_FLD_OUT_PKTS	0x070064
#define LNF_AGGR_FLOWS		0x080064
#define LNF_FLD_SRCPORT 	0x090016
#define LNF_FLD_DSTPORT		0x0a0016
#define LNF_FLD_TCP_FLAGS	0x0b0008
#define LNF_FLD_SRCADDR 	0x0c00a1
#define LNF_FLD_DSTADDR		0x0d00a1
#define LNF_FLD_IP_NEXTHOP	0x0e00a1
#define LNF_FLD_SRC_MASK	0x0f0008
#define LNF_FLD_DST_MASK	0x100008
#define LNF_FLD_TOS			0x110008
#define LNF_FLD_DST_TOS		0x130008
#define LNF_FLD_SRCAS		0x140032
#define LNF_FLD_DSTAS		0x150032
#define LNF_FLD_BGPNEXTADJACENTAS	0x160032
#define LNF_FLD_BGPPREVADJACENTAS	0x170032
#define LNF_FLD_BGP_NEXTHOP		0x1800a1
#define LNF_FLD_PROT	 	0x190008
#define LNF_FLD_SRC_VLAN	0x200016
#define LNF_FLD_DST_VLAN	0x210016
#define LNF_FLD_IN_SRC_MAC	0x2200a2
#define LNF_FLD_OUT_SRC_MAC	0x2300a2
#define LNF_FLD_IN_DST_MAC	0x2400a2
#define LNF_FLD_OUT_DST_MAC	0x2500a2
#define LNF_FLD_MPLS_LABEL	0x2600ab
#define LNF_FLD_INPUT		0x270016
#define LNF_FLD_OUTPUT		0x280017
#define LNF_FLD_DIR			0x290008
#define LNF_FLD_FWD_STATUS	0x300008
#define LNF_FLD_IP_ROUTER	0x3100a1
#define LNF_FLD_ENGINE_TYPE	0x320008
#define LNF_FLD_ENGINE_ID	0x330008
#define LNF_FLD_EVENT_TIME	0x340064
#define LNF_FLD_CONN_ID		0x350032
#define LNF_FLD_ICMP_CODE	0x360008
#define LNF_FLD_ICMP_TYPE	0x370008
#define LNF_FLD_FW_XEVENT	0x380016
#define LNF_FLD_XLATE_SRC_IP	0x3900a1
#define LNF_FLD_XLATE_DST_IP	0x4000a1
#define LNF_FLD_XLATE_SRC_PORT	0x410016
#define LNF_FLD_XLATE_DST_PORT	0x420016
#define LNF_FLD_INGRESS_ACL_ID	0x43
#define LNF_FLD_INGRESS_ACE_ID	0x44
#define LNF_FLD_INGRESS_XACE_ID	0x45
#define LNF_FLD_EGRESS_ACL_ID	0x46
#define LNF_FLD_EGRESS_ACE_ID	0x47
#define LNF_FLD_EGRESS_XACE_ID	0x48
#define LNF_FLD_USERNAME		0x49
#define LNF_FLD_INGRESS_VRFID	0x50
#define LNF_FLD_EVENT_FLAG		0x51
#define LNF_FLD_EGRESS_VRFID	0x52
#define LNF_FLD_BLOCK_START		0x530016
#define LNF_FLD_BLOCK_END		0x540016
#define LNF_FLD_BLOCK_STEP		0x550016
#define LFN_FLD_BLOCK_SIZE		0x560016
#define LNF_FLD_CLIENT_NW_DELAY_USEC	0x570064
#define LNF_FLD_SERVER_NW_DELAY_USEC	0x580064
#define LNF_FLD_APPL_LATENCY_USEC		0x590064



/* the maximim number of fields requested from the client */
#define NFL_MAX_FIELDS 256

/* the maxumim naumber of instances (objects) that can be used in code */
#define NFL_MAX_INSTANCES 512

/* return eroror codes */
#define NFL_NO_FREE_INSTANCES -1;

/* extend NF_XX codes with code idicates thet we already to read the next record */
#define NF_OK      1


/* C interface */

/* list of maps used in file taht we create */
typedef struct lnf_map_list_s {
	bit_array_t				 bit_array;
	extension_map_t			*map;
	struct lnf_map_list_s	*next;
} lnf_map_list_t;


/* structure representing single record */
/* it contains two fields. Nfdump'ps master record */
/* and bit array representing set of fields that are activated in the master ricord */
/* master record or bit array SHOULD NEVER be used in the direct form */
/* the structure of the field might change across versions and there is */
/* a big probability that any future version wouldn't be compadible */
typedef struct lnf_rec_s {
	master_record_t *master_record;		/* reference to master record */
	bit_array_t *extensions_arr;		/* list of extensions available in the record */
} lnf_rec_t;


/* structure representing single nfdump file */
/* every nfdump file which is open for either read or write is identified by this handle */
typedef struct lnf_file_s {
	nffile_t 				*nffile;				/* ptr to nfdump's nffile structure */
	int 					flags;
#define LNF_READ	0x0		
#define LNF_WRITE	0x1		/* file is open for writing */
#define LNF_ANON	0x2		/* set anon flag on the file */
#define LNF_COMP	0x4		/* the file is compressed */
#define LNF_WEAKERR	0x8		/* return weak erros $(unknow block, record) */
	int 					blk_record_remains;		/* records to be processed in the current block */
	extension_map_list_t 	*extension_map_list;	/* ptr to extmap structure */
	lnf_map_list_t			*lnf_map_list;			/* internal list of maps (used by lnf) */
	int						max_num_extensions;		/* the max number of extensions */
	common_record_t 		*flow_record;			/* ptr to buffer/next record */
	master_record_t			*master_record;
	lnf_rec_t				*lnf_rec;				/* temporary */
	bit_array_t				extensions_arr;			/* structure initialised at the beginging to store bitt array of extensions from last record */
	uint64_t                processed_blocks;
	uint64_t                skipped_blocks;
	uint64_t                processed_records;
	uint64_t                current_processed_blocks;
	uint64_t                processed_bytes;
} lnf_file_t;


 
/* functions 
lnf_open(hnd, file, flags, ident);
LNF_MEM
lnf_close(hnd);
*/

#define LNF_OK				0x0001	/* OK status */
#define LNF_EOF 			0x0000	/* end of file */

#define LNF_ERR_UNKBLOCK	-0x0001	/* weak error: unknown block type */
#define LNF_ERR_UNKREC		-0x0002	/* weak error: unknown record type */
#define LNF_ERR_COMPAT15	-0x0004	/* weak error: old blok type suppoerted by nfdump 1.5 */
#define LNF_ERR_WEAK		-0x000F	/* all weak errors (errors to skip) */

#define LNF_ERR_READ		-0x0010	/* read error (IO) */
#define LNF_ERR_CORRUPT		-0x0020	/* coruprted file */
#define LNF_ERR_EXTMAPB		-0x0040	/* too big extension map */
#define LNF_ERR_EXTMAPM		-0x0080	/* missing extension map */
#define LNF_ERR_WRITE		-0x00F0	/* missing extension map */

#define LNF_ERR_NOTSET		-0x0100	/* item is not set  */

//lnf_read(hnd, rechnd);

/*
lnf_write(hnd, rechnd);

lnf_rclone(rec2, rec1);
lnf_dget(rec, FIELD, &data);
lnf_dset(rec, FIELD, &data);

lnf_finit(fhnd, "filter");
lnf_fmatch(fhnd, rec);
lnf_fdestroy(fhnd);

lnf_agrset(ahnd, num fields, fields???);
structure { int field, bitmask } 

lnf_sortset(ahnd, numfields, bitmask);

*/

lnf_file_t * lnf_open(char * filename, unsigned int flags, char * ident);
void lnf_close(lnf_file_t *lnf_file);
int lnf_read(lnf_file_t *lnf_file, lnf_rec_t *lnf_rec);
int lnf_write(lnf_file_t *lnf_file, lnf_rec_t *lnf_rec);

/*
int lnf_item_init(lnf_rec_t *rec);
int lnf_item_get(lnf_rec_t *rec, int field, void ** ptr); 
int lnf_item_set(lnf_rec_t *rec, int field, void * ptr); 
int lnf_item_datat(int field);
*/

