#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct cache_node  //cache node 
{   unsigned long key;
    unsigned long value;
    struct cache_node * pre; 	//cache的双向链表
    struct cache_node * next; 	//  
    struct cache_node * nexthash;  //hash table
};


struct cache_data
{
    int freelist_count;
    int cache_size;
    int hash_table_size;
    int cache_length;           // 当前条目数
#define HASH_TABLE_SIZE  cache_size
#define CACHE_SIZE      cache_size
    unsigned long miss;
    unsigned long hit;
    int (* hash_func)(struct cache_data * cache, unsigned long key);
    struct cache_node * cache_head;
    struct cache_node * cache_tail;
    struct cache_node * freelist;
    struct cache_node * hash_table[0];                                                                               
};

int default_hash(struct cache_data * cache, unsigned long key)
{
    return key % cache->HASH_TABLE_SIZE;
}

struct cache_data *  create_cache(int cache_size, int (* hash)(struct cache_data * cache, unsigned long key)) {
    struct cache_data * cache = malloc(sizeof(struct cache_data) + cache_size * sizeof(void *));
    if(!cache){
	printf("out of memory!\n");
	exit(-1);
    }
    memset(cache, 0, sizeof(struct cache_data) + cache_size * sizeof(void *));
    cache->cache_size = cache_size;
    if(hash != NULL)
	cache->hash_func = hash;
    else
	cache->hash_func = default_hash;
    return cache;
}

//lookup a key in the hash table 
struct cache_node* hash_table_lookup(struct cache_data * cache, unsigned long key) {
    unsigned int pos = cache->hash_func(cache, key);//key % cache->HASH_TABLE_SIZE;    //hash funtion
    if(cache->hash_table[pos]) {
	struct cache_node * pHead = cache->hash_table[pos]; 
	while(pHead) {
	    if(key == pHead->key)
		return pHead;
	    pHead = pHead->nexthash;
	}    
    }      
    return NULL;
}

//insert cache_node into hash table
void hash_table_insert(struct cache_data * cache, struct cache_node *cur)
{
    unsigned int pos = cache->hash_func(cache, cur->key);//cur->key % cache->HASH_TABLE_SIZE;
    cur->nexthash = cache->hash_table[pos];
    cache->hash_table[pos] = cur;
} 

//remove key-value from the hash table 
void hash_table_remove(struct cache_data * cache, unsigned long key) {
    unsigned int pos = cache->hash_func(cache, key);//key % cache->HASH_TABLE_SIZE;
    printf("%s: key=%ld, pos=%d\n", __func__, key, pos);
    if(cache->hash_table[pos]) {
	struct cache_node* pHead = cache->hash_table[pos];
	struct cache_node* pLast = NULL;
	struct cache_node* pRemove = NULL;
	while(pHead) {
	    if(key == pHead->key) {
		pRemove = pHead;
		break;
	    }
	    pLast = pHead;
	    pHead = pHead->nexthash;
	}
	if(pRemove) {
	    if(pLast) 
		pLast->nexthash = pRemove->nexthash;
	    else
		cache->hash_table[pos] = pRemove->nexthash;
	}     
    } 
}  

void push_front(struct cache_data * cache, struct cache_node *cur)   //双向链表删除节点,并将节点移动链表头部，O(1) 
{      
    if(cache->cache_length == 1)
	return;
    if(cur == cache->cache_head)
	return;
    if(cur == cache->cache_tail){ 
	cache->cache_tail = cur->pre;
    }     
    cur->pre->next = cur->next; //删除节点
    if(cur->next != NULL)
	cur->next->pre = cur->pre; 
    cur->next = cache->cache_head;    //移到链表头部 
    cur->pre = NULL;
    cache->cache_head->pre = cur; 
    cache->cache_head = cur;
}  

// retval: -1: miss
int cache_search(struct cache_data * cache, unsigned long key) 
{
#if 0
    if(cache->cache_head == NULL) 
	return -1;
#endif
    struct cache_node *p = hash_table_lookup(cache, key);
    if(p == NULL) {// 不存在该key
	cache->miss++;
	return -1; 
    }
    else 
	push_front(cache, p);    //将节点p置于链表头部
    cache->hit++;
    return cache->cache_head->value;   
}  

//LRU
void cache_update(struct cache_data * cache, unsigned long key, unsigned long value) 
{
    struct cache_node * p = NULL;
    p = hash_table_lookup(cache, key);
    if (NULL != p)
	push_front(cache, p);
    else { 
	if(cache->cache_length == cache->CACHE_SIZE) { 
	    struct cache_node * p = cache->cache_tail;
	    hash_table_remove(cache, cache->cache_tail->key);
	    push_front(cache, p);
	    p->value=value;
	    p->key=key;
	    hash_table_insert(cache, p);
	} else {
	    printf("%s: key=%ld, value=%ld, freelist=%p\n", __func__, key, value, cache->freelist);
	    if(cache->freelist){
		p = cache->freelist;
		cache->freelist = p->next;
		cache->freelist_count--;
		printf("cache->freelist_count = %d\n", cache->freelist_count);
	    }
	    if (!p) {
		p = (struct cache_node *) malloc(sizeof(struct cache_node));
	    }
	    p->key = key;
	    p->value = value; 
	    p->pre = NULL;
	    p->next = cache->cache_head;
	    if(cache->cache_head != NULL) 
		cache->cache_head->pre = p;
	    else
		cache->cache_tail = p;
	    cache->cache_head = p; 
	    hash_table_insert(cache, cache->cache_head);
	    cache->cache_length++; 
	}
    }
}

void cache_flush(struct cache_data * cache, unsigned long key)
{
    struct cache_node * p = NULL;
    p = hash_table_lookup(cache, key);
    if (NULL == p)
	return;
    if (p == cache->cache_head){
	cache->cache_head = p->next;
	cache->cache_head->pre = NULL;
	goto out;
    }
    if (p == cache->cache_tail)
	cache->cache_tail = p->pre;
    p->pre->next = p->next; //删除节点
    if(p->next != NULL)
	p->next->pre = p->pre;
out:
    hash_table_remove(cache, p->key);
    p->next = cache->freelist;
    cache->freelist = p;
    cache->freelist_count++;
    cache->cache_length--;
}

void cache_flush_all(struct cache_data * cache)
{
    cache->freelist = cache->cache_head;
    cache->freelist_count = cache->cache_length;
    cache->cache_length = 0;
    cache->cache_head = cache->cache_tail = NULL;
    memset(cache->hash_table, 0, cache->cache_size * sizeof(void *));
}

void cache_print(struct cache_data * cache, int verbose){
    int i;
    struct cache_node * p = cache->cache_head; 
    while(p) {
	printf("%ld ", p->value); 
	p = p->next; 
    }
    printf("\n");
    if(verbose) {
	for (i = 0; i < cache->HASH_TABLE_SIZE; i++){
	    printf("hash %d:", i);
	    struct cache_node * p = cache->hash_table[i];
	    while(p) {
		printf("%ld", p->value); 
		p = p->nexthash;
	    }
	    printf("\n");
	}
    }
    printf("cache_length: %d, freelist_count: %d\n",
	    cache->cache_length, cache->freelist_count);
    printf("miss: %ld, hit: %ld\n", cache->miss, cache->hit);
}


int main(int argc, char ** argv) 
{ 
    struct cache_data * cache = create_cache(4, NULL); 
    if(-1==cache_search(cache, 1))
	cache_update(cache, 1, 1);
    cache_print(cache, 1);
    if(-1==cache_search(cache, 2)) 
	cache_update(cache, 2, 2); 
    cache_print(cache, 1);
    if(-1==cache_search(cache, 1)) 
	cache_update(cache, 1, 1); 
    cache_print(cache, 1);
    if(-1==cache_search(cache, 4))
	cache_update(cache, 4, 4); 
    cache_print(cache, 1);
    if(-1==cache_search(cache, 2)) 
	cache_update(cache, 2,2); 
    cache_print(cache, 1);
    if(-1==cache_search(cache, 5)) 
	cache_update(cache, 5,5); 
    cache_print(cache, 1);
    if(-1==cache_search(cache, 9)) 
	cache_update(cache, 9,9); 
    cache_print(cache, 1);
    if(-1==cache_search(cache, 3)) 
	cache_update(cache, 3, 3);
    cache_print(cache, 1);
    cache_flush(cache, 3);
    cache_print(cache, 1);
    if(-1==cache_search(cache, 3)) 
	cache_update(cache, 3, 3);
    cache_print(cache, 1);
    cache_flush_all(cache);
    cache_print(cache, 1);
    if(-1==cache_search(cache, 9)) 
	cache_update(cache, 9,9); 
    cache_print(cache, 1);
    return 0;
}
