#include <iostream>
#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cerrno>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <queue>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include<algorithm>
#include <time.h>
#include <math.h>
#include <algorithm>

using namespace std;


struct ads
{
    char* str;
    int len;
    int next;
};

#define RADIX 26
struct bucket
{
    int head;
    int tail;
};

class wait_list{
	public:
	wait_list(){
		pthread_mutex_init(&_mutex, NULL);
                pthread_cond_init(&_cond, NULL);
	}
	~wait_list(){
		pthread_cond_destroy(&_cond);
                pthread_mutex_destroy(&_mutex);
	}
	void* get(){
		void* ret;
		pthread_mutex_lock(&_mutex);
		while (q.empty())
                	pthread_cond_wait(&_cond, &_mutex);
		ret=q.front();
		q.pop();
		pthread_mutex_unlock(&_mutex);
                return ret;
	}
		
	void put(void* val){
		pthread_mutex_lock(&_mutex);
		q.push(val);
		pthread_cond_signal(&_cond);
		pthread_mutex_unlock(&_mutex);

	}
	
	int size(){
		return q.size();
	}	
	private:
		pthread_mutex_t _mutex;
		pthread_cond_t _cond;
		queue<void*> q;
};


#define LIKELY(x) __builtin_expect(!!(x),1)
#define UNLIKELY(x) __builtin_expect(!!(x),0)
#define sg_align(d, a)     (((d) + (a - 1)) & ~(a - 1))

int cmp4(const void *p1, const void *p2) {
	char* a1 = *(char**) p1;
	char* a2 = *(char**) p2;
#if 1

	__asm__  (
			"sub %%rdi,%%rsi;"
			"sub $16,%%rdi;"
			"strcmpLoop:"
			"add  $16 ,%%rdi;"
			"movdqu (%%rdi),%%xmm0;"
			"pcmpistri $0x1a,(%%rdi,%%rsi),%%xmm0;"
			"jc strcmpDiff;"
			"ja     strcmpLoop;"
			"xor	%%eax, %%eax;"
			"jmp	exitStrcmp;"
			"strcmpDiff:"
			"addq %%rdi,%%rsi;"
			"movzx (%%rsi,%%rcx),%%eax;"
			"movzx (%%rdi,%%rcx),%%edx;"
			"sub %%edx,%%eax;"
			//"sub (%%rdi,%%rcx),%%al;"
			"exitStrcmp:"
			:
			:"S"(a1),"D"(a2)
			:"%eax","%edx"
	);
#endif
//return strcmp(a1->data,a2->data);
}



int cmp5(const void *p1, const void *p2) {
        char* a1 = (char*) p1;
        char* a2 = (char*) p2;
#if 1

        __asm__  (
                        "sub %%rdi,%%rsi;"
                        "sub $16,%%rdi;"
                        "strcmpLoop1:"
                        "add  $16 ,%%rdi;"
                        "movdqu (%%rdi),%%xmm0;"
                        "pcmpistri $0x1a,(%%rdi,%%rsi),%%xmm0;"
                        "jc strcmpDiff1;"
                        "ja     strcmpLoop1;"
                        "xor    %%eax, %%eax;"
                        "jmp    exitStrcmp1;"
                        "strcmpDiff1:"
                        "addq %%rdi,%%rsi;"
                        "movzx (%%rsi,%%rcx),%%eax;"
                        "movzx (%%rdi,%%rcx),%%edx;"
                        "sub %%edx,%%eax;"
                        //"sub (%%rdi,%%rcx),%%al;"
                        "exitStrcmp1:"
                        :
                        :"S"(a1),"D"(a2)
                        :"%eax","%edx"
        );
#endif
//return strcmp(a1->data,a2->data);
}

inline int cmp6(const void *p1, const void *p2) {
	ads* a1 = (ads*) p1;
	ads* a2 = (ads*) p2;

	int len=a1->len<a2->len?a1->len:a2->len;
	return memcmp(a1->str,a2->str,len+1);
}

inline int cmp7(const void* p1,const void* p2){
	ads* a1=*(ads**)p1;
        ads* a2=*(ads**)p2;
	int len=a1->len<a2->len?a1->len:a2->len;
	return memcmp(a1->str,a2->str,len+1);
}

 inline void mycpy1(const char* d, const char* s, int len) {
	int off = len >> 4;
	int mod = len % 16;
	s+=len;
	d+=len;
	switch (off) {
	case 0:
		break;
	case 1:
		__asm__ __volatile__ (
				//"mov     -16(%%rsi), %%r11;" \
			"mov     -8(%%rsi), %%rdx;" \
			"mov      %%r11, -16(%%rdi);" \
			"mov      %%rdx, -8(%%rdi);"
				"movdqu  -16(%%rsi), %%xmm0;"
				"movdqu %%xmm0,-16(%%rdi);"
				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;
	case 2:
		__asm__ __volatile__ (
				/*"mov     -32(%%rsi), %%r9;"
				"mov     -24(%%rsi), %%r10;"
				"mov     -16(%%rsi), %%r11;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r9, -32(%%rdi);"
				"mov      %%r10, -24(%%rdi);"
				"mov      %%r11, -16(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"*/
				"movdqu  -32(%%rsi), %%xmm1;"
				 "movdqu  -16(%%rsi), %%xmm0;"
				"movdqu %%xmm1,-32(%%rdi);"
                                "movdqu %%xmm0,-16(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx","%r9","%r10","%r11" 
		);
		break;
	case 3:
		__asm__ __volatile__ (
				/*
				"mov     -48(%%rsi), %%rcx;"
				"mov     -40(%%rsi), %%r8;"
				"mov     -32(%%rsi), %%r9;"
				"mov     -24(%%rsi), %%r10;"
				"mov     -16(%%rsi), %%r11;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%rcx, -48(%%rdi);"
				"mov      %%r8, -40(%%rdi);"
				"mov      %%r9, -32(%%rdi);"
				"mov      %%r10, -24(%%rdi);"
				"mov      %%r11, -16(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"*/
				"movdqu  -48(%%rsi), %%xmm2;"
				 "movdqu  -32(%%rsi), %%xmm1;"
                                 "movdqu  -16(%%rsi), %%xmm0;"
				"movdqu %%xmm2,-48(%%rdi);"
                                "movdqu %%xmm1,-32(%%rdi);"
                                "movdqu %%xmm0,-16(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx","%r8","%r9","%r10","%r11"
		);
		break;
	case 4:
		__asm__ __volatile__ (
				/*"movdqu  -64(%%rsi), %%xmm0;"
				"mov     -48(%%rsi), %%rcx;"
				"mov     -40(%%rsi), %%r8;"
				"mov     -32(%%rsi), %%r9;"
				"mov     -24(%%rsi), %%r10;"
				"mov     -16(%%rsi), %%r11;"
				"mov     -8(%%rsi), %%rdx;"
				"movdqu   %%xmm0, -64(%%rdi);"
				"mov      %%rcx, -48(%%rdi);"
				"mov      %%r8, -40(%%rdi);"
				"mov      %%r9, -32(%%rdi);"
				"mov      %%r10, -24(%%rdi);"
				"mov      %%r11, -16(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"*/
				"movdqu  -64(%%rsi), %%xmm3;"
				"movdqu  -48(%%rsi), %%xmm2;"
                                 "movdqu  -32(%%rsi), %%xmm1;"
                                 "movdqu  -16(%%rsi), %%xmm0;"
				"movdqu %%xmm3,-64(%%rdi);"
                                "movdqu %%xmm2,-48(%%rdi);"
                                "movdqu %%xmm1,-32(%%rdi);"
                                "movdqu %%xmm0,-16(%%rdi);"
				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx","%r8","%r9","%r10","%r11"
		);
		break;
	case 5:
		__asm__ __volatile__ (
				"movdqu  -80(%%rsi), %%xmm0;"
				"movdqu  -64(%%rsi), %%xmm1;"
				"movdqu  -48(%%rsi), %%xmm2;"
				"movdqu  -32(%%rsi), %%xmm3;"
				"movdqu  -16(%%rsi), %%xmm4;"
				"movdqu   %%xmm0, -80(%%rdi);"
				"movdqu   %%xmm1, -64(%%rdi);"
				"movdqu   %%xmm2, -48(%%rdi);"
				"movdqu   %%xmm3, -32(%%rdi);"
				"movdqu   %%xmm4, -16(%%rdi);"
				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"

		);
		break;
	case 6:
		__asm__ __volatile__ (
				"movdqu  -96(%%rsi), %%xmm5;"
				"movdqu  -80(%%rsi), %%xmm0;"
				"movdqu  -64(%%rsi), %%xmm1;"
				"movdqu  -48(%%rsi), %%xmm2;"
				"movdqu  -32(%%rsi), %%xmm3;"
				"movdqu  -16(%%rsi), %%xmm4;"
				"movdqu   %%xmm5, -96(%%rdi);"
				"movdqu   %%xmm0, -80(%%rdi);"
				"movdqu   %%xmm1, -64(%%rdi);"
				"movdqu   %%xmm2, -48(%%rdi);"
				"movdqu   %%xmm3, -32(%%rdi);"
				"movdqu   %%xmm4, -16(%%rdi);"
				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"

		);
		break;
	case 7:
		__asm__ __volatile__ (
				"movdqu  -112(%%rsi), %%xmm6;"
				"movdqu  -96(%%rsi), %%xmm5;"
				"movdqu  -80(%%rsi), %%xmm0;"
				"movdqu  -64(%%rsi), %%xmm1;"
				"movdqu  -48(%%rsi), %%xmm2;"
				"movdqu  -32(%%rsi), %%xmm3;"
				"movdqu  -16(%%rsi), %%xmm4;"
				"movdqu   %%xmm6, -112(%%rdi);"
				"movdqu   %%xmm5, -96(%%rdi);"
				"movdqu   %%xmm0, -80(%%rdi);"
				"movdqu   %%xmm1, -64(%%rdi);"
				"movdqu   %%xmm2, -48(%%rdi);"
				"movdqu   %%xmm3, -32(%%rdi);"
				"movdqu   %%xmm4, -16(%%rdi);"
				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"

		);
		break;
	case 8:
		__asm__ __volatile__ (
				"movdqu  -128(%%rsi), %%xmm7;"
				"movdqu  -112(%%rsi), %%xmm6;"
				"movdqu  -96(%%rsi), %%xmm5;"
				"movdqu  -80(%%rsi), %%xmm0;"
				"movdqu  -64(%%rsi), %%xmm1;"
				"movdqu  -48(%%rsi), %%xmm2;"
				"movdqu  -32(%%rsi), %%xmm3;"
				"movdqu  -16(%%rsi), %%xmm4;"
				"movdqu   %%xmm7, -128(%%rdi);"
				"movdqu   %%xmm6, -112(%%rdi);"
				"movdqu   %%xmm5, -96(%%rdi);"
				"movdqu   %%xmm0, -80(%%rdi);"
				"movdqu   %%xmm1, -64(%%rdi);"
				"movdqu   %%xmm2, -48(%%rdi);"
				"movdqu   %%xmm3, -32(%%rdi);"
				"movdqu   %%xmm4, -16(%%rdi);"
				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"

		);
		break;

	default:
		break;

	}

	s-=(off<<4);
	d-=(off<<4);
	switch (mod) {
	case 0:
		break;
	case 1:
		__asm__ __volatile__ (
				"mov     -1(%%rsi), %%dl;"
				"mov      %%dl, -1(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;
	case 2:
		__asm__ __volatile__ (
				"mov     -2(%%rsi), %%dx;"
				"mov      %%dx, -2(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 3:
		__asm__ __volatile__ (
				"mov     -3(%%rsi), %%dx;"
				"mov     -2(%%rsi), %%cx;"
				"mov      %%dx, -3(%%rdi);"
				"mov      %%cx, -2(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 4:
		__asm__ __volatile__ (
				"mov     -4(%%rsi), %%edx;"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;
	case 5:

		__asm__ __volatile__ (
				"mov     -5(%%rsi), %%edx;"
				"mov     -4(%%rsi), %%ecx;"
				"mov      %%edx, -5(%%rdi);"
				"mov      %%ecx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;
	case 6:
		__asm__ __volatile__ (
				"mov     -6(%%rsi), %%edx;"
				"mov     -4(%%rsi), %%ecx;"
				"mov      %%edx, -6(%%rdi);"
				"mov      %%ecx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;
	case 7:

		__asm__ __volatile__ (
				"mov     -7(%%rsi), %%edx;"
				"mov     -4(%%rsi), %%ecx;"
				"mov      %%edx, -7(%%rdi);"
				"mov      %%ecx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;
	case 8:
		__asm__ __volatile__ (
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%rdx, -8(%%rdi);"
				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;
	case 9:
		__asm__ __volatile__ (
				"mov     -9(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%rcx, -9(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;
	case 10:
		__asm__ __volatile__ (
				"mov     -10(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%rcx, -10(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 11:
		__asm__ __volatile__ (
				"mov     -11(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%rcx, -11(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;
	case 12:
		__asm__ __volatile__ (
				"mov     -12(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%rcx, -12(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 13:
		__asm__ __volatile__ (
				"mov     -13(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%rcx, -13(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;
	case 14:
		__asm__ __volatile__ (
				"mov     -14(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%rcx, -14(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;
	case 15:
		__asm__ __volatile__ (
				"mov     -15(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%rcx, -15(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;
	default:
		break;
	}
}

#if 0

void allocation(ads ** arr, int first_index, int i_th,bucket* bucket_queue)
{
    for(int i = 0; i < RADIX; ++i)     // 置桶为空
    {
        bucket_queue[i].head = -1;
    }
    int index = first_index, key_ith;  // key_ith 串第i_th个字符
    while(index != -1)                 // 分配
    {
        key_ith = 0;                   // 默认串的第i_th字符为空
        if(arr[index]->len > i_th)
        {
            key_ith = arr[index]->str[i_th]-'a'; // 取第i_th个排序码
        }
        if(bucket_queue[key_ith].head == -1)    // 桶为空
        {
            bucket_queue[key_ith].head = index;
        }else
        {
            arr[bucket_queue[key_ith].tail]->next = index;  // 更新子链
        }
        bucket_queue[key_ith].tail = index;     // 更新子链的末指针
        index = arr[index]->next;
    }
}


void collection(ads ** arr, int & first_index,bucket* bucket_queue)
{
    int q_index = 0;  // 桶队列下标
    int last_index;   // 记录已收集子链的末指针
    while(bucket_queue[q_index].head == -1)
    {
        ++q_index;
    }
    first_index = bucket_queue[q_index].head;
    last_index = bucket_queue[q_index].tail;
 
    while(++q_index < RADIX)
    {
        while(q_index < RADIX && bucket_queue[q_index].head == -1)
        {
            ++q_index;
        }
        if(q_index < RADIX && bucket_queue[q_index].head != -1)
        {
            arr[last_index]->next = bucket_queue[q_index].head;
            last_index = bucket_queue[q_index].tail;
        }
    }
    arr[last_index]->next = -1;
}


int radix_sort(ads ** arr, int digits,bucket* bucket_queue)
{
    int first_index = 0;
    for(int i = digits - 1; i >= 0; --i)
    {
        allocation(arr, first_index, i,bucket_queue);
        collection(arr, first_index,bucket_queue);
    }
    return first_index;
}

#endif
void radix_sort(vector<ads*>&  arr, int maxLen )
{
    const int BUCKETS = 26;

    vector<vector<ads*>> wordsByLength( maxLen + 1 );
    vector<vector<ads*>> buckets( BUCKETS );

    for( ads*  s : arr )
        wordsByLength[ s->len ].push_back( s );

    int idx = 0;
    for( auto & wordList : wordsByLength )
        for( ads* s : wordList )
            arr[ idx++ ] = s;

    int startingIndex = arr.size( );
    for( int pos = maxLen - 1; pos >= 0; --pos )
    {
        startingIndex -= wordsByLength[ pos + 1 ].size( );

        for( int i = startingIndex; i < arr.size( ); ++i )
            buckets[ arr[ i ]->str[ pos ]-'a' ].push_back( arr[ i ]  );

        idx = startingIndex;
        for( auto & thisBucket : buckets )
        {
            for( ads* s : thisBucket )
                arr[ idx++ ] = s ;
            thisBucket.clear( );
        }
    }
}

void radix_sort1( vector<ads*> & arr, int stringLen )
{
    const int BUCKETS = 26;

    vector<vector<ads*>> buckets( BUCKETS );

    for( int pos = stringLen - 1; pos >= 0; --pos )
    {
        for( ads* s : arr )
            buckets[ s->str[ pos ]-'a' ].push_back( s );

        int idx = 0;
        for( auto & thisBucket : buckets )
        {
            for( ads* s : thisBucket )
                arr[ idx++ ] = s;

            thisBucket.clear( );
        }
    }
}


void build_chunk(const vector<char*>& lines,char* addr1){
	int index = 0;
		int size = lines.size();
		for (int i = 0; i < size; ++i) {
			//write(ofd,lines[i]->data,lines[i]->len+1);
			//lines[i]->data[lines[i]->len-1]='\n';
			//memcpy(addr1+index,lines[i]->data,lines[i]->len);
			int len = strlen(lines[i]);
			lines[i][len] = '\n';
			//memcpy(addr1+index,lines[i],len+1);
			mycpy1(addr1 + index, lines[i], len + 1);

			index += len + 1;
		}
}


void build_chunk(const vector<vector<char*>>& lines,char* addr1){
        	int index = 0;
                int size = lines.size();
                for (int i = 0; i < size; ++i) {
			int size1=lines[i].size();
			for(int j=0;j<size1;++j){
				char* line=lines[i][j];	
                        	int len = strlen(line);
                        	line[len] = '\n';
                        	mycpy1(addr1 + index, line, len + 1);
                        	index += len + 1;
			}
                }
}


void test() {
	char tmp[129] = { 0 };
	for (int i = 0; i < 129; ++i) {
		srand(time(NULL));
		char dst[129] = { 0 };
		for (int j = 0; j < i; ++j) {
			dst[j] = 'A' + rand() % 26;
		}
		dst[i] = 0;
		mycpy1(tmp, dst, i);
		tmp[i] = 0;
		if (string(dst) == string(tmp)) {
			fprintf(stderr, "test ok len=%d\n", i);
		} else
			fprintf(stderr, "test failed len=%d\n", i);
		fprintf(stderr, "t%d=%s,dst=%s\n", i, tmp,dst);
	}

}


struct kfifo {
    char **buffer;    /* the buffer holding the data */
    unsigned int size;    /* the size of the allocated buffer */
    unsigned int in;    /* data is added at offset (in % size) */
    unsigned int out;    /* data is extracted from off. (out % size) */
};

struct kfifo *kfifo_alloc(unsigned int size)
{
    char **buffer;
    struct kfifo *ret;

    /*   
 *        * round up to the next power of 2, since our 'let the indices   
 *             * wrap' tachnique works only in this case.   
 *                  */

    buffer =(char**)malloc(size*sizeof(char*));
    if (!buffer)
        return 0;

    ret =(kfifo*) malloc(sizeof(kfifo));
    ret->buffer=buffer;
    ret->size=size;
    ret->in=0;
    ret->out=0;


    return ret;
}


unsigned int __kfifo_put(struct kfifo *fifo,
             char *buffer)
{
    unsigned int l;

    //len = min(len, fifo->size - fifo->in + fifo->out); 
    if(fifo->size - fifo->in + fifo->out<=0)
        return -1;//full    

    /*   
 *        * Ensure that we sample the fifo->out index -before- we   
 *             * start putting bytes into the kfifo.   
 *                  */

    //smp_mb();
    __sync_synchronize();

    /* first put the data starting from fifo->in to buffer end */
    //l = min(len, fifo->size - (fifo->in & (fifo->size - 1)));     
    //memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);     

    /* then put the rest (if any) at the beginning of the buffer */
    //memcpy(fifo->buffer, buffer + l, len - l);    
    fifo->buffer[fifo->in%fifo->size]=buffer;

    /*   
 *        * Ensure that we add the bytes to the kfifo -before-   
 *             * we update the fifo->in index.   
 *                  */

    //smp_wmb();
    __sync_synchronize();

    fifo->in +=1;

    return 0;
}



unsigned int __kfifo_get(struct kfifo *fifo, char* &buffer)
{
    unsigned int l;

    //len = min(len, fifo->in - fifo->out);     
    if(fifo->in<=fifo->out)
        return -1;//empty


    //smp_rmb();
    __sync_synchronize();

    /* first get the data from fifo->out until the end of the buffer */
    //l = min(len, fifo->size - (fifo->out & (fifo->size - 1)));     
    //memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), l);     

    /* then get the rest (if any) from the beginning of the buffer */
    //memcpy(buffer + l, fifo->buffer, len - l);     
    buffer=fifo->buffer[fifo->out%fifo->size];
    /*   
 *        * Ensure that we remove the bytes from the kfifo -before-   
 *             * we update the fifo->out index.   
 *                  */

    __sync_synchronize();

    fifo->out += 1;

    return 0;
}

struct merge_thread_args{
        int thr;
        vector<char*>* sort_lines;
        vector<vector<char*>>* input;
        int step;
        vector<int> end;
        vector<int> beg;
        char* output;
        int output_len;
        vector<int>* offsets;
        int ofd;
        pthread_barrier_t* barrier;
        wait_list* wl;
};

vector<wait_list> free_list(6);

char* STR_MAX="{";
ads ADS_MAX={"{",1,-1};
ads ADS_MIN={"`",1,-1};
class merge_sort{
	public:
		
		void sort(vector<vector<char*>>& input,vector<char*>&output,int (*compar)(const void *, const void *)){
			cmp=(__compar_d_fn_t)compar;
			k=input.size();
		        ls.reserve(k);
			b.reserve(k+1);
			index=vector<int>(k,0);

			for(int i=0;i<k;++i){
				if(input[i].size()>0){
					b.push_back(input[i][0]);
				}else
					b.push_back(0);		
				
			}

			create_loser_tree();
			
			while(b[ls[0]]){
				int q=ls[0];
				output.push_back(b[q]);
				index[q]++;
				if(index[q]<input[q].size())
					b[q]=input[q][index[q]];
				else
					b[q]=0;
				adjust(q);
			}
		}


	void sort(vector<vector<char*>>& input,vector<char*>&output,int (*compar)(const void *, const void *),const vector<int>& beg,const vector<int>& end){
                        cmp=(__compar_d_fn_t)compar;
                        k=input.size();
                        ls.reserve(k);
                        b.reserve(k+1);
			index.reserve(k);
                        for(int i=0;i<k;++i){
                                if(beg[i]<end[i]){
                                        b.push_back(input[i][beg[i]]);
                                }else
                                        b.push_back(0);
				index.push_back(beg[i]);

                        }

                        create_loser_tree();

                        while(b[ls[0]]){
                                int q=ls[0];
                                output.push_back(b[q]);
                                index[q]++;
                                if(index[q]<end[q])
                                        b[q]=input[q][index[q]];
                                else
                                        b[q]=0;
                                adjust(q);
                        }
                }


		void sort(vector<vector<char*>>& input,char* output, int& offset,int (*compar)(const void *, const void *),const vector<int>& beg,const vector<int>& end){
                        cmp=(__compar_d_fn_t)compar;
                        k=input.size();
                        ls.reserve(k);
                        b.reserve(k+1);
                        index.reserve(k);
                        for(int i=0;i<k;++i){
                                if(beg[i]<end[i]){
                                        b.push_back(input[i][beg[i]]);
                                }else
                                        b.push_back(STR_MAX);
                                index.push_back(beg[i]);

                        }

                        create_loser_tree();

                        while(compar(&b[ls[0]],&STR_MAX)!=0){
                                int q=ls[0];
                                int len=strlen(b[q]);
				b[q][len]='\n';
                                mycpy1(output+offset,b[q],len+1);
				offset+=len+1;
                                index[q]++;
                                if(index[q]<end[q])
                                        b[q]=input[q][index[q]];
                                else
                                        b[q]="{";
                                adjust(q);
                        }
                }
	
		 void sort(vector<vector<char*>>& input,int ofd, int& offset,int (*compar)(const void *, const void *),const vector<int>& beg,const vector<int>& end, merge_thread_args* ta){
                        cmp=(__compar_d_fn_t)compar;
                        k=input.size();
                        ls.reserve(k);
                        b.reserve(k+1);
                        index.reserve(k);
			struct timeval t1,t2;
			gettimeofday(&t1,0);
			for(int i=0;i<k;++i){
				for(int j=beg[i];j<end[i];++j){
					offset+=strlen(input[i][j])+1;
				}
			}
			gettimeofday(&t2,0);

			int interval=(t2.tv_sec-t1.tv_sec)*1000+(t2.tv_usec-t1.tv_usec)/1000;
			(*(ta->offsets))[ta->thr]=offset;
			fprintf(stderr,"thr=%d,offset=%d,offset time=%d\n",ta->thr,offset,interval);
			pthread_barrier_wait(ta->barrier);
                        for(int i=0;i<k;++i){
                                if(beg[i]<end[i]){
                                        b.push_back(input[i][beg[i]]);
                                }else
                                        b.push_back(STR_MAX);
                                index.push_back(beg[i]);
                        }
			
                        create_loser_tree();
			const int send_block=2*1024;
			char* tmp=(char*)malloc(send_block);
			int tmp_len=0;
			ads* a=(ads*)malloc(sizeof(ads));
			a->str=tmp;
                        while(compar(&b[ls[0]],&STR_MAX)!=0){
				
                                int q=ls[0];
				#if 1
                                int len=strlen(b[q]);
                                b[q][len]='\n';
                                //mycpy1(output+offset,b[q],len+1);
                                //pwrite(ofd,b[q],len,offset);
                               // offset+=len+1;
				if(len+1+tmp_len>send_block){
					a->len=tmp_len;
					a->next=ta->thr;
					ta->wl->put(a);
					tmp_len=0;
                                        if(free_list[ta->thr].size()==0){
                                                a=(ads*)malloc(sizeof(ads));
						tmp=(char*)malloc(send_block);
                                                a->str=tmp;
                                        }else{
                                                a=(ads*)(free_list[ta->thr].get());
						tmp=a->str;
                                        }
				}
				mycpy1(tmp+tmp_len,b[q],len+1);
				tmp_len+=(len+1);
				#endif
                                index[q]++;
                                if(index[q]<end[q])
                                        b[q]=input[q][index[q]];
                                else
                                        b[q]="{";
                                adjust(q);
                        }
			if(tmp_len>0){
				ads* a=(ads*)malloc(sizeof(ads));
                                a->str=tmp;
                                a->len=tmp_len;
                                a->next=ta->thr;
				ta->wl->put(a);
			}
			//ta->wl->put(0);
                }

		  void sort(vector<vector<ads*>>& input,char* output, int& offset,int (*compar)(const void *, const void *),const vector<int>& beg,const vector<int>& end){
                        cmp=(__compar_d_fn_t)compar;
                        k=input.size();
                        ls.reserve(k);
                        b1.reserve(k+1);
                        index.reserve(k);
                        for(int i=0;i<k;++i){
                                if(beg[i]<end[i]){
                                        b1.push_back(input[i][beg[i]]);
                                }else
                                        b1.push_back(&ADS_MAX);
                                index.push_back(beg[i]);

                        }

                        create_loser_tree1();
			int nout=0;
                        while(compar(b1[ls[0]],&ADS_MAX)!=0){
                                int q=ls[0];
                                mycpy1(output+offset,b1[q]->str,b1[q]->len+1);
                                offset+=b1[q]->len+1;
                                index[q]++;
                                if(index[q]<end[q])
                                        b1[q]=input[q][index[q]];
                                else
                                        b1[q]=&ADS_MAX;
                                adjust1(q);
				//++nout;
                        }
			fprintf(stderr,"merge nout lines=%d\n",nout);
                }
			
		void sort(kfifo* input[],vector<char*>&output,int (*compar)(const void *, const void *)){
                        cmp=(__compar_d_fn_t)compar;
                        k=5;
                        ls.reserve(k);
                        b.reserve(k+1);

                        for(int i=0;i<k;++i){
				char* tmp=0;
				while(1){
                                	if(__kfifo_get(input[i],tmp)==0){
                                        	b.push_back(tmp);
						break;
                                	}
					//fprintf(stderr,"fifo empty\n");
				}
                                     

                        }

                        create_loser_tree();

                        while(b[ls[0]]){
                                int q=ls[0];
                                output.push_back(b[q]);
                                char* tmp=0;
				while(1){
                                        if(__kfifo_get(input[q],tmp)==0){
                                                b[q]=tmp;
                                                break;
                                        }
                                }
                                adjust(q);
                        }
                }
	
	private:
		int k;
		vector<int> ls;
		vector<char*> b;
		vector<ads*> b1;
		vector<int> index;
		__compar_d_fn_t cmp;
		 inline void adjust(int i){
			int t=(i+k)/2;
			while(t>0){
				if(cmp(&b[i],&b[ls[t]],0)>0){
					i=i^ls[t];
					ls[t]=i^ls[t];
					i=i^ls[t];
				}
				t=t>>1;
			}

			ls[0]=i;
			
		}


		inline void adjust1(int i){
                        int t=(i+k)/2;
                        while(t>0){
                                if(cmp(b1[i],b1[ls[t]],0)>0){
                                        i=i^ls[t];
                                        ls[t]=i^ls[t];
                                        i=i^ls[t];
                                }
                                t=t>>1;
                        }

                        ls[0]=i;

                }	

	

		void create_loser_tree(){
			b.push_back("`");
			for(int i=0;i<k;++i){
				ls.push_back(k);
			}
			for(int i=k-1;i>=0;--i){
				adjust(i);
			}
		}

		void create_loser_tree1(){
                        b1.push_back(&ADS_MIN);
                        for(int i=0;i<k;++i){
                                ls.push_back(k);
                        }
                        for(int i=k-1;i>=0;--i){
                                adjust1(i);
                        }
                }
};

void swap(char** s1,char** s2){
	char* tmp=*s1;
	*s1=*s2;
	*s2=tmp;
}

void max_heapify(char* arr[], int start, int end,int (*compar)(const void *, const void *)) {
    int dad = start;
    int son = dad * 2 + 1;
    while (son <= end) {
        if (son + 1 <= end && compar(arr[son] ,arr[son + 1])>0) 
            son++;
        if (compar(arr[dad],arr[son])<0) 
            return;
        else { 
            swap(&arr[dad], &arr[son]);
            dad = son;
            son = dad * 2 + 1;
        }
    }
}

void heap_sort(char* arr[], int len,int (*compar)(const void *, const void *),void* arg) {
    kfifo* fifo=(kfifo*)arg;
    for (int i = len / 2 - 1; i >= 0; i--)
        max_heapify(arr, i, len - 1,compar);
    for (int i = len - 1; i > 0; i--) {
	//while(__kfifo_put(fifo,arr[0])!=0);
        swap(&arr[0], &arr[i]);
        max_heapify(arr, 0, i - 1,compar);
    }
    //while(__kfifo_put(fifo,arr[0])!=0);
    //while(__kfifo_put(fifo,0)!=0);
}

struct thread_args{
	char* data;
	vector<vector<char*>>* output;
	int thr;
	int len;
	int fd;
	int offset;
	pthread_mutex_t* mutex;
	wait_list* wl;
	int tn;
};

void* sort_handle1(void* arg){
        fprintf(stderr,"in sort thread\n");
        thread_args* ta=(thread_args*)arg;

	struct timeval tv5, tv6;
	gettimeofday(&tv5, 0);
	ads* a=0;
	//char* adss=(char*)malloc(20000000*sizeof(ads));
	while((a=(ads*)(ta->wl->get()))!=0){
		struct timeval tv1, tv2;
        	gettimeofday(&tv1, 0);
        	char* end=a->str+a->len;
		char* addr=a->str;
        	//if(ta->thr!=0){
                while(*(addr-1)!='\n'&&*(addr-1)!=0)addr--;
        	//}
        	char* start, *pre;
        	start = addr;
        	pre = addr;
		int i=0;
		vector<char*> o;
        	while ((start = strchr(start, '\n')) != 0&&start<end) {

			//ads* a=(ads*)adss;
                	int len = start - pre ;
                	pre[len] = 0;
			//a->len=len;
			//a->str=pre;
			++i;
                	o.push_back(pre);
                	pre = start + 1;
                	start++;
			//adss+=sizeof(ads);
        	}

		gettimeofday(&tv2, 0);
		//fprintf(stderr, "sort thread %d,lines num=%d\n",ta->thr,o.size());
        	char** data = o.data();

		qsort(data, o.size(), sizeof(char**), cmp4);
		pthread_mutex_lock(ta->mutex);
		ta->output->push_back(o);
		pthread_mutex_unlock(ta->mutex);
		free(a);
	}
	gettimeofday(&tv6, 0);
        fprintf(stderr, "sort thread %d,lines num=%d,len=%d,sort interval3=%d\n",ta->thr,ta->output->size(),ta->len,
                        (tv6.tv_sec - tv5.tv_sec) * 1000
                                      + (tv6.tv_usec - tv5.tv_usec) / 1000);
        return 0;
}



void* merge_handle(void* arg){
	fprintf(stderr,"in merge thread\n");
	struct timeval tv1, tv2;
	gettimeofday(&tv1, 0);
        merge_sort ms;
	merge_thread_args* ta=(merge_thread_args*)arg;
        ms.sort(*(ta->input),ta->ofd,ta->output_len,cmp4,ta->beg,ta->end, ta);
	(*(ta->offsets))[ta->thr]=ta->output_len;
	gettimeofday(&tv2, 0);
	fprintf(stderr, "merge thread %d,output line size=%d,offset=%d,sort interval=%d\n",ta->thr,ta->sort_lines->size(),
			ta->output_len,
                        (tv2.tv_sec - tv1.tv_sec) * 1000
                                      + (tv2.tv_usec - tv1.tv_usec) / 1000);
	//pthread_barrier_wait(ta->barrier);
	fprintf(stderr,"all reached\n");
	int offset=0;
	for(int i=0;i<ta->thr;++i){
		offset+=(*(ta->offsets))[i];
	}
	//fprintf(stderr,"merge thread=%d,ouput=%s\n",ta->thr,ta->output);
	//pwrite(ta->ofd,ta->output,ta->output_len,offset);
        return 0;
}

struct io_thread_args{
	int thr;
	int fd;
	vector<wait_list>* wl;
	char* data;
	int size;
	int offset;
	int tn;
	vector<int>* offsets;
	wait_list* iowl;
	int ofd;
	pthread_barrier_t* barrier;	
};


void* io_handle(void* args){
	fprintf(stderr,"io handle\n");
	io_thread_args* ta=(io_thread_args*)args;
	int sum=0;
	int nst=0;
	while(sum<ta->size){
		
		int ret=0;
		ads* a=(ads*)malloc(sizeof(ads));
                a->str=ta->data;
		if(ta->size-sum<2*ta->offset){
			ret=read(ta->fd,ta->data,ta->size-sum);
			}
		else{
			ret=read(ta->fd, ta->data,ta->offset);
		}
		a->len=ret;
		sum+=ret;
		((*(ta->wl))[nst%ta->tn]).put(a);
		ta->data+=ret;
		++nst;
	}
	for(int i=0;i<ta->tn;++i){
		((*(ta->wl))[i]).put(0);
	}

	pthread_barrier_wait(ta->barrier);
	vector<int> offsets;
	for(int i=0;i<ta->tn;++i){
		int sum=0;
		for(int j=0;j<i;++j)
			sum+=(*(ta->offsets))[j];
		offsets.push_back(sum);	
	}	
	ads* a=0;
	int nexit=0;
	int i=0;
	while((a=(ads*)(ta->iowl->get()))!=0){
		#if 0
		if(!a){
			++nexit;
			fprintf(stderr,"get a null,wait list size=%d\n",ta->iowl->size());
			if(nexit==ta->tn)
				break;
			else 
				continue;
		}
		#endif 
		//fprintf(stderr,"thr=%d,str=%s,len=%d\n",a->next,a->str,a->len);
		pwrite(ta->ofd,a->str,a->len,offsets[a->next]);
		offsets[a->next]+=(a->len);
		//fprintf(stderr,"offset[%d]=%d\n",a->next,offsets[a->next]);
		//free(a->str);
		//free(a);
		free_list[a->next].put(a);	
		++i;
	}
	fprintf(stderr,"io handle end,write line=%d\n",i);
	return 0;
}

int bsearch(const char** ptr,int size,char* target,int (*compar)(const void *, const void *)){
	int beg=0;
	int end=size-1;
	while(beg<=end){
		int mid=(beg+end)/2;
		if(compar(target,ptr[mid])>=0&&(size==mid+1||compar(target,ptr[mid+1])<0)){
			return mid+1;
		}else if(compar(target,ptr[mid])>=0){
			beg=mid+1;
		}
		else{
			end=mid-1;
		}
	}
	return 0;
}


int bsearch(const ads** ptr,int size,ads* target,int (*compar)(const void *, const void *)){
        int beg=0;
        int end=size-1;
        while(beg<=end){
                int mid=(beg+end)/2;
                if(compar(target,ptr[mid])>=0&&(size==mid+1||compar(target,ptr[mid+1])<0)){
                        return mid+1;
                }else if(compar(target,ptr[mid])>=0){
                        beg=mid+1;
                }
                else{
                        end=mid-1;
                }
        }
        return 0;
}

void end_adjust(const vector<vector<char*>>& output_lines,int i, int step,int thread_num,vector<int>& end,int (*compar)(const void *, const void *)){
	for(int j=0;j<thread_num;++j){
		int tend=(i+1)*(output_lines[j].size()/step);
                end[j]=min(max(tend,end[j]+1),(int)output_lines[j].size());
        }

	char * smax=0;
        int  imax=-1;
        for(int j=0;j<thread_num;++j){
                if(end[j]>0&&end[j]<=output_lines[j].size()&&(!smax||compar(smax,output_lines[j][end[j]-1])<0)){
                        imax=j;
                        smax=output_lines[j][end[j]-1];
                }

        }
	
	for(int j=0;j<thread_num;++j){
		if(j!=imax){
			int nexti=bsearch((const char**)(output_lines[j].data()+end[j]),output_lines[j].size()-end[j],smax,compar);
			end[j]+=nexti;
		}
	}
}

void end_adjust(const vector<vector<ads*>>& output_lines,int i, int step,int thread_num,vector<int>& end,int (*compar)(const void *, const void *)){
        int tend=((i+1)*step);
        for(int j=0;j<thread_num;++j){
		end[j]=min(max(tend,end[j]+1),(int)output_lines[j].size());
	}
        ads * smax=0;
        int  imax=-1;
        for(int j=0;j<thread_num;++j){
                if(end[j]>0&&end[j]<=output_lines[j].size()&&(!smax||compar(smax,output_lines[j][end[j]-1])<0)){
			imax=j;
                        smax=output_lines[j][end[j]-1];
                }

        }
        for(int j=0;j<thread_num;++j){
                if(j!=imax){
                        int nexti=bsearch((const ads**)(output_lines[j].data()+end[j]),output_lines[j].size()-end[j],smax,compar);
                        end[j]+=nexti;
			//if(nexti!=0)
			//fprintf(stderr,"new end,j=%d,end[%d]=%d\n",j,j,end[j]);
                }
        }
}


void end_adjust(const vector<vector<char*>>& output_lines,int i, int mean_lines,int thread_num,vector<int>& beg,vector<int>& end,int (*compar)(const void *, const void *)){
	
	int now_lines=0;
	vector<char*> check_point;
	int lines_num=output_lines.size();
	for(int j=0;j<lines_num;++j){
                        int size=output_lines[j].size();
                        int tend=(i+1)*(size/thread_num);
                        tend=min(max(tend,end[j]+1),size);
			end[j]=tend;
			//fprintf(stderr,"tend=%d\n",tend);
                        if(tend<size)
                                check_point.push_back(output_lines[j][tend]);
	}


	vector<int> pos=end;
	int sub_lines=99999999;
//	while(true){
		qsort(check_point.data(), check_point.size(), sizeof(char**), cmp4);
		int  mid=0;
		int  start=0;
		int  last=(int)(check_point.size());
		while(start<last){
			mid=(start+last)/2;	
			char* smid=check_point[mid];
			now_lines=0;
			for(int j=0;j<lines_num;++j){
                        	int nexti=bsearch((const char**)(output_lines[j].data()),output_lines[j].size(),smid,compar);
                        	pos[j]=nexti;
				now_lines+=end[j]-beg[j];	
        		}

		
			if(abs(now_lines-mean_lines)<sub_lines){
				end=pos;
				sub_lines=abs(now_lines-mean_lines);
			}
			else if(now_lines-mean_lines>0){
				last=mid-1;
			}else
				start=mid+1;
		//fprintf(stderr,"start=%d,mid=%d,last=%d,now_lines=%d,mean_lines=%d\n",start,mid,last,now_lines,mean_lines);
		}

		#if 0
		check_point.clear();
		if(abs(now_lines-mean_lines)<100000){
			break;
		}else if(now_lines-mean_lines>100000){
			for(int j=0;j<lines_num;++j){
				int size=output_lines[j].size();
				if(end[j]-beg[j]>2){
					int tend=min(max(tend,end[j]-1),size);
					
				}
				
			}
		}
		#endif
//	}
	
}


int main(int argc, char** argv) {

	struct timeval t1, t2;
	gettimeofday(&t1, 0);
	if (argc < 3) {
		fprintf(stderr, "not enough args,argc=%d\n", argc);
		exit(-1);
	}

	//test();
	string input = string(argv[1]);
	string output = string(argv[2]);
	int ifd = open(input.c_str(), O_RDONLY);
	if (ifd < 0) {
		fprintf(stderr, "open file failed,err=%s,input=%s\n", strerror(errno),input.c_str());
		exit(-1);
	}

	readahead(ifd,0,1024*1024*1024);
	struct timeval t3, t4;
	gettimeofday(&t3, 0);
	struct stat sb;
	if (fstat(ifd, &sb) == -1) {
		fprintf(stderr, "fstat erro,err=%s\n", strerror(errno));
		exit(-1);
	}

	int ofd = open(output.c_str(), O_RDWR | O_CREAT, 00644);

        if (ofd < 0) {
                fprintf(stderr, "open output error,err=%s\n", strerror(errno));
                exit(-1);
        }

	const int thread_num=6;
	vector<int> offsets(thread_num,0);
	size_t length = sb.st_size;
	pthread_barrier_t barrier;
        pthread_barrier_init(&barrier,NULL, thread_num+1);
	int read_block=length/384;
	vector<wait_list> wl(thread_num);
	wait_list iowl; 
	char* addr = (char*) malloc(length+1);
	*addr=0;
	io_thread_args ita;
	ita.data=addr+1;
	ita.thr=0;
	ita.wl=&wl;
	ita.offset=read_block;
	ita.fd=ifd;
	ita.tn=thread_num;
	ita.size=length;
	ita.ofd=ofd;
	ita.offsets=&offsets;
	ita.iowl=&iowl;
	ita.barrier=&barrier;
	pthread_t io_tid;
	pthread_create(&io_tid,0,io_handle,&ita);
	
	
	int i=0;
	int block=length/thread_num;
	thread_args args[thread_num];
	vector<vector<char*>> output_lines;
	vector<pthread_t> tids;
	tids.reserve(thread_num);
	output_lines.reserve(thread_num);
	pthread_mutex_t mutex;
	pthread_mutex_init(&mutex,0);


	int beg=0;
	while(i<thread_num){
		pthread_t tid;
		if(i==thread_num-1){
			//read(ifd,addr+beg,length-block*(thread_num-1));
			args[i].len=length-block*(thread_num-1);
		}
		else{
			//read(ifd, addr+beg, block);
			args[i].len=block;
		}
		args[i].data=addr+beg+1;
		args[i].output=&output_lines;
		args[i].thr=i;
		args[i].fd=ifd;
		args[i].offset=read_block;
		args[i].mutex=&mutex;
		args[i].wl=&wl[i];
		args[i].tn=thread_num;
		beg+=block;
		pthread_create(&tid,0,sort_handle1,&args[i]);
		tids.push_back(tid);
		++i;
	}

	 for(int i=0;i<tids.size();++i){
                pthread_join(tids[i],0);
        }
	tids.clear();
#if 0
	for(int i=0;i<output_lines.size();++i){
		for(int j=0;j<output_lines[i].size();++j){
			fprintf(stderr,"output[%d],line=%s\n",i,output_lines[i][j]);
		}
	}
#endif
	merge_thread_args mtas[thread_num];
	vector<vector<char*>> sort_lines(thread_num);

	int lines_num=0;
	//vector<int> offsets(thread_num,0);
	fprintf(stderr,"output lines block=%d\n",output_lines.size());
        for(int i=0;i<output_lines.size();++i){
		lines_num+=output_lines[i].size();
        }
	fprintf(stderr,"output lines num=%d\n",lines_num);
	int step=lines_num/(output_lines.size()*thread_num);
	fprintf(stderr,"step=%d\n",step);
	i=0;
	vector<int> start;
	vector<int> end(output_lines.size(),0);
	while(i<thread_num){
		pthread_t tid;
		mtas[i].sort_lines=&sort_lines[i];
		mtas[i].thr=i;
		mtas[i].input=&output_lines;
		mtas[i].step=step;
		mtas[i].offsets=&offsets;
		start=end;
		mtas[i].ofd=ofd;
		mtas[i].barrier=&barrier;
		mtas[i].wl=&iowl;
		
		if(i==thread_num-1){
			for(int j=0;j<end.size();++j){
				end[j]=output_lines[j].size();
			}
		}
		else{
			end_adjust(output_lines,i,lines_num/thread_num,thread_num,start,end,cmp5);
		}

		mtas[i].beg=start;
		mtas[i].end=end;
		mtas[i].output=(char*)malloc(200*1024*1024);
		mtas[i].output_len=0;
		pthread_create(&tid,0,merge_handle,&mtas[i]);
		tids.push_back(tid);
		++i;
	}
	for(int i=0;i<tids.size();++i){
		pthread_join(tids[i],0);
	}

	iowl.put(0);
	pthread_join(io_tid,0);

	struct timeval tv7, tv8;
	gettimeofday(&tv7, 0);
	gettimeofday(&tv8, 0);
	
	fprintf(stderr, "memcpy interval4=%d\n",
			(tv8.tv_sec - tv7.tv_sec) * 1000
					+ (tv8.tv_usec - tv7.tv_usec) / 1000);
	struct timeval tv9, tv10;
	gettimeofday(&tv9, 0);
#if 0
	//write(ofd, addr1, length);
	for(int i=0;i<thread_num;++i){
		write(ofd, mtas[i].output, mtas[i].output_len);
#endif
	gettimeofday(&tv10, 0);
	fprintf(stderr, "output interval=%d\n",
			(tv10.tv_sec - tv9.tv_sec) * 1000
					+ (tv10.tv_usec - tv9.tv_usec) / 1000);
	//munmap(addr1,length);
//	close(ifd);
	close(ofd);
//	munmap(addr,length);
	gettimeofday(&t2, 0);
	fprintf(stderr, "sum interval=%d\n",
			(t2.tv_sec - t1.tv_sec) * 1000 + (t2.tv_usec - t1.tv_usec) / 1000);
	return 0;

}
