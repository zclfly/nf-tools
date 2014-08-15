

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "hash_table.h"
#include "xxhash.h"

#define HASH_ATOMIC_CAS(ptr, comp, exch) (__sync_bool_compare_and_swap(ptr, comp, exch))
#define HASH_ATOMIC_INC(ptr) (__sync_add_and_fetch(ptr, 1))


/* initialise hash table */ 
/* argumets - aggregation key size, sort key size, values size */
hash_table_t * hash_table_init(hash_table_t *t, int numbuckets,
			hash_table_aggr_callback_t acb, 
			hash_table_sort_callback_t scb, 
			void *callback_data) {

	t->buckets = calloc(sizeof(void *), numbuckets);
	if (t->buckets == NULL) {
		return NULL;
	}

	t->numbuckets = numbuckets;
	t->aggr_callback = acb;
	t->sort_callback = scb;
	t->callback_data = callback_data;
	t->entrypoint = NULL;	/* entry point */
	
//	t->rows_used = 0;

	return t;

}

void hash_table_entry_len(hash_table_t *t, int keylen, int vallen) {

	t->keylen = keylen;	
	t->vallen = vallen;	

}

/* insert element into hash table */
char * hash_table_insert(hash_table_t *t, char *key, char *val) {

	unsigned long hash, index;
	char *prow;
	hash_table_row_hdr_t *phdr;
	char *pkey;
	char *pval;

	hash = XXH64(key, t->keylen, 0);	

	index = hash % t->numbuckets;

	prow = t->buckets[index];

	while (prow != NULL) {
		/* collison */
		phdr = (hash_table_row_hdr_t *)prow;
		pkey = prow + sizeof(hash_table_row_hdr_t);
		pval = prow + sizeof(hash_table_row_hdr_t) + t->keylen;

		if (memcmp(pkey, key, t->keylen) == 0) {
			/* same key - aggregate value */
			t->aggr_callback(pkey, pval, val, t->callback_data);
			return prow;
		} else {
			/* keys do not match - try next item in list */
			prow = phdr->next;
		}
	}

	/* new entry */
	prow = malloc(sizeof(hash_table_row_hdr_t) + t->keylen + t->vallen);

	if (prow == NULL) {
		return NULL;
	}

	phdr = (hash_table_row_hdr_t *)prow;
	pkey = prow + sizeof(hash_table_row_hdr_t);
	pval = prow + sizeof(hash_table_row_hdr_t) + t->keylen;

	memcpy(pkey, key, t->keylen);
	memcpy(pval, val, t->vallen);
	phdr->hash = hash;
	phdr->next = t->buckets[index];

	t->buckets[index] = prow;

	return prow;
}

int hash_table_sort_callback(char *prow1, char *prow2, void *p) {

	hash_table_t *t = p;
	char *pkey1, *pval1;
	char *pkey2, *pval2;

	pkey1 = (prow1 + sizeof(hash_table_row_hdr_t));
	pval1 = (prow1 + sizeof(hash_table_row_hdr_t) + t->keylen);
	
	pkey2 = (prow2 + sizeof(hash_table_row_hdr_t));
	pval2 = (prow2 + sizeof(hash_table_row_hdr_t) + t->keylen);

	return t->sort_callback(pkey1, pval1, pkey2, pval2, t->callback_data);

}

/* convert hash table into linked list */
void hash_table_sort(hash_table_t *t) {

	unsigned long index;
	char *prow_tmp;
	hash_table_row_hdr_t *phdr;

	for (index = 0; index < t->numbuckets; index++) {

		if (t->buckets[index] != NULL) {
			
			prow_tmp = t->buckets[index];

			while (prow_tmp != NULL) {
				phdr = (hash_table_row_hdr_t *)prow_tmp;	
				prow_tmp = phdr->next;
			}

			phdr->next = t->entrypoint;
			t->entrypoint = t->buckets[index];

		} 
	}	

	//heap_sort(t->sort_data, t->sort_items, &hash_table_sort_callback, t);
	
}

/* return next field */
char * hash_table_first(hash_table_t *t) {

	return  t->entrypoint;

}

char * hash_table_next(hash_table_t *t, char *prow) {

	hash_table_row_hdr_t *phdr;

	phdr = (hash_table_row_hdr_t *)prow;	

	return phdr->next;
}

void hash_table_fetch(hash_table_t *t, char *prow, char **pkey, char **pval) {

	hash_table_row_hdr_t *phdr;

	if (prow == NULL) {
		prow = t->entrypoint;
	} 

	phdr = (hash_table_row_hdr_t *)prow;	
	*pkey = (prow + sizeof(hash_table_row_hdr_t));
	*pval = (prow + sizeof(hash_table_row_hdr_t) + t->keylen);

}

void hash_table_free(hash_table_t *t) {

	char *prow, *tmp;
	hash_table_row_hdr_t *phdr;

	free(t->buckets);

	prow = t->entrypoint;

	while (prow != NULL) {
		phdr = (hash_table_row_hdr_t *)prow;
		tmp = prow;
		prow = phdr->next;
		free(tmp);
	}	

//	free(t->sort_data);
}
