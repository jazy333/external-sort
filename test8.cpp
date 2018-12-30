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
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include<algorithm>
#include <time.h>
#include <math.h>
using namespace std;

struct ads {
	char* data;
	int len;
};

#define LIKELY(x) __builtin_expect(!!(x),1)
#define UNLIKELY(x) __builtin_expect(!!(x),0)
#define sg_align(d, a)     (((d) + (a - 1)) & ~(a - 1))
extern void __qsort_r (void *b, size_t n, size_t s, __compar_d_fn_t cmp, void *arg);
#define QSORT(void *b, size_t n, size_t s, __compar_d_fn_t cmp) __qsort_r(b,n,s,cmp,0)

inline int my_memcmp(const void *ads1, const void *ads2) {
	ads* a1 = *(ads**) ads1;
	ads* a2 = *(ads**) ads2;
	void* s1 = a1->data;
	void* s2 = a2->data;
	__builtin_prefetch(a1->data);
	__builtin_prefetch(a2->data);
	int len = a1->len > a2->len ? a2->len : a1->len;
	len += 1;
	const int *p1, *q1;
	const char *p2, *q2;
	int off, mod;

	off = len >> 2;
	mod = len - (off << 2);

	if (mod > 0) {
		p2 = (const char *) s1;
		q2 = (const char *) s2;
		while (mod--) {
			if (*p2++ != *q2++) {
				return p2[-1] > q2[-1] ? 1 : -1;
			}
		}
	}

	/* if p1 & q1 address don't align with 4 bytes,
	 *        how about the efficiency ? */
	mod = len - (off << 2);
	p1 = (const int *) (s1 + mod);
	q1 = (const int *) (s2 + mod);

	while (off--) {
		if (*p1++ != *q1++) {
			return p1[-1] > q1[-1] ? 1 : -1;
		}
	}

	return 0;
}

inline int my_memcmp1(const void *ads1, const void *ads2) {
	ads* a1 = *(ads**) ads1;
	ads* a2 = *(ads**) ads2;
	__builtin_prefetch(a1->data);
	__builtin_prefetch(a2->data);
	if (a1->data[0] != a2->data[0])
		return a1->data[0] - a2->data[0];

	void* s1 = a1->data;
	void* s2 = a2->data;
	int len = a1->len > a2->len ? a2->len : a1->len;
	len++;

	const __int128_t* t1,*t2;
	t1 = (const __int128_t *) s1;
	t2 = (const __int128_t *) s2;
	if (len >= 16) {
		int off, mod;
		off = len >> 4;
		mod = len - len >> 4;
		len = mod;
		while (off--) {
			if (*t1 ^ *t2) {
				len = 16;
				break;
			}
			t1++;
			t2++;
		}
	}

	const long *r1, *r2;
	int off1, mod1;
	off1 = len >> 3;
	mod1 = len - (off1 << 3);
	len = mod1;
	r1 = (const long*) (t1);
	r2 = (const long*) (t2);
	while (off1--) {
		if (*r1++ ^ *r2++) {
			//return *(r1-1)-*(r2-1);
			r1--;
			r2--;
			len = 8;
		}
	}

	const int *p1, *q1;

	p1 = (const int *) (r1);
	q1 = (const int *) (r2);
	int off = len >> 2;
	int mod = len - off << 2;
	len = mod;
	while (off--) {
		if (*p1++ ^ *q1++) {
			p1--;
			q1--;
			len = 4;
		}
	}

	//s1=(char*)p1;
	//s2=(char*)p2;

	if (len > 0) {
		const char* p2 = (const char *) p1;
		const char* q2 = (const char *) q1;
		while (len--) {
			if (*p2++ ^ *q2++) {
				return *(p2 - 1) - *(q2 - 1);
			}
		}
	}

	return 0;
}

inline int my_memcmp2(const void *ads1, const void *ads2) {
	ads* a1 = *(ads**) ads1;
	ads* a2 = *(ads**) ads2;

	if (a1->data[0] != a2->data[0])
		return a1->data[0] - a2->data[0];
	void* s1 = a1->data;
	void* s2 = a2->data;
	int len = a1->len > a2->len ? a2->len : a1->len;
	len += 1;
	const int *p1, *q1;
	const char *p2, *q2;
	int off, mod;

	off = len >> 2;
	mod = len - (off << 2);

	p1 = (const int*) s1;
	q1 = (const int*) s2;
	while (off--) {
		if (*p1 ^ *q1) {
			return __builtin_bswap32(*p1) - __builtin_bswap32(*q1);
		}
		p1++;
		q1++;
	}

	if (mod > 0) {
		p2 = (const char *) p1;
		q2 = (const char *) q1;
		while (mod--) {
			if (*p2++ != *q2++) {
				return p2[-1] > q2[-1] ? 1 : -1;
			}
		}
	}

	return 0;
}

inline int my_memcmp3(const void *ads1, const void *ads2) {
	ads* a1 = *(ads**) ads1;
	ads* a2 = *(ads**) ads2;
	//__builtin_prefetch(a1->data);
	//__builtin_prefetch(a2->data);
	// if(a1->data[0]!=a2->data[0])
	//   return a1->data[0]-a2->data[0];

	//char* s1=a1->data;
	//char* s2=a2->data;
	int len = a1->len > a2->len ? a2->len : a1->len;

	int off, mod;

#if 0
	if(len>=16) {
#if 0
		const __int128_t* t1,*t2;
		t1=(const __int128_t*)s1;
		t2=(const __int128_t*)s2;
		off=len>>4;
		mod=len-off<<4;
		len=mod;

		while(off--) {
			if(*t1^*t2) {
				len=16;
				s1=(char*)t1;
				s2=(char*)t2;
				break;
			}
			++t1;++t2;
		}
#endif
		return memcmp(a1->data,a2->data,len);
	}
#endif

#if 0

	if(len>=16) {
//printf("ori len=%d\n",len);
		int ori_len=len;
		off=len>>4;
		len=len%16;
//int mod=len;
		//int i=0;
//for(int i=0;i<off;++i,s1+=16,s2+=16){
		//off=len>>4;
		//mod=len-off<<4;
		//len=mod;
//printf("before s1=%lx,s2=%lx,len=%d\n",s1,s2,len); 
//char* t1=s1, *t2=s2;

		__asm__ __volatile__ (
				" pxor	%%xmm0, %%xmm0;"
				"2:"
				" movdqu	(%%rdi), %%xmm2;"
				" pxor	(%%rsi), %%xmm2;"
				" ptest	%%xmm2, %%xmm0;"
				" jnc 1f;"
				"add $16,%%rsi;"
				"add $16,%%rdi;"
				"sub $1,%%edx;"
				"test %%edx,%%edx;"
				"jz 3f;"
				"jmp 2b;"
				"1:"
				"mov $16,%%eax; "
				"mov %%eax, %0;"
				"3:"
				"movq %%rsi,%1;"
				"movq %%rdi,%2;"
				:"=m"(len),"=&m"(s1),"=&m"(s2)
				:"S"(s1),"D"(s2),"d"(off)
				:
		);
//if(len!=16)
//	printf("after s1=%lx,s2=%lx,t1=%lx,t2=%lx,len=%d,ori_len=%d,mod=%d,off=%d\n",s1,s2,t1,t2,len,ori_len,mod,off);
	}

#endif

	const long *p1, *q1;
	const char *p2, *q2;

	off = len >> 3;
	mod = len - (off << 3);

	p1 = (long*) a1->data;
	q1 = (long*) a2->data;

#if 1 
	while (off--) {
		if (*p1 ^ *q1) {
			return ((long) __builtin_bswap64(*p1)
					- (long) __builtin_bswap64(*q1)) > 0 ? 1 : -1;
		}
		__builtin_prefetch(p1 + 1);
		__builtin_prefetch(q1 + 1);
		p1++;
		q1++;
	}
#endif

#if 0
	long ld=0;
	const long* lend=p1+off;
	for(;;) {
		if (p1 >= lend||ld) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;

		if (ld || p1 >= lend) break;
		ld = *p1++ - *q1++;
	}

	if(ld)
	return ((long)__builtin_bswap64(p1[-1])-(long)__builtin_bswap64(q1[-1]))>0?1:-1;
#endif

	p2 = (const char *) p1;
	q2 = (const char *) q1;
#if 1
	if (mod >= 4) {
		mod -= 4;
		if (LIKELY(*((int* )p2) ^ *((int* )q2))) {
			return __builtin_bswap32(*((int*) p2))
					- __builtin_bswap32(*((int*) q2));
		} else {
			p2 += 4;
			q2 += 4;
		}
	}
#endif

#if 0
	while (mod --) {
		if (*p2 ++ ^ *q2 ++) {
			return p2[-1] - q2[-1];
		}
	}
#endif
#if 1
	const char* end = p2 + mod;
	int d = 0;
	for (;;) {
		if (d || p2 >= end)
			break;
		d = *p2++ - *q2++;

		if (d || p2 >= end)
			break;
		d = *p2++ - *q2++;

		if (d || p2 >= end)
			break;
		d = *p2++ - *q2++;

	}
	return d;
#endif

}

inline int cmpads(const void *p1, const void *p2) {
	/* The actual arguments to this function are "pointers to
	 *               pointers to char", but strcmp(3) arguments are "pointers
	 *                             to char", hence the following cast plus dereference */
	ads* a1 = *(ads**) p1;
	ads* a2 = *(ads**) p2;
	//int i=0;
	//while(UNLIKELY(a1->data[i]!='\n')&&UNLIKELY(a2->data[i]!='\n')&&UNLIKELY(a1->data[i]==a2->data[i]))++i;
#if 0
	int len=0,ret=0;
	if(a1->len<a2->len) {
		len=a1->len;
		ret=-1;
	} else {
		len=a2->len;
		ret=1;
		if(a2->len==a1->len)
		ret=0;
	}
#endif

	int len = a1->len > a2->len ? a2->len : a1->len;
#if 0
	if(len>64)
	printf("len=%d\n",len);
#endif

#if 0
	if(len>8) {
		char* addr1=(char*)sg_align((long)(a1->data),8);
		printf("orig=%lx,alg=%lx\n",a1->data,addr1);
	}
#endif

	/*else if(len==2){
	 return *((short*)(a1->data[0]))-*((short*)(a2->data[0]));
	 }else if(len==4){
	 return *((int*)(a1->data[0]))-*((int*)(a2->data[0]));
	 }*/
	return memcmp(a1->data, a2->data, len + 1);

#if 0
	if(a1->data[i]=='\n'&&a2->data[i]=='\n')
	return 0;
	else if(a1->data[i]=='\n'&&a2->data[i]!='\n') {
		return -1;
	}
	else if(a1->data[i]!='\n'&&a2->data[i]=='\n')
	return 1;
#endif
	//return a1->data[i]-a2->data[i];
}

inline int cmp4(const void *p1, const void *p2) {
	char* a1 = *(char**) p1;
	char* a2 = *(char**) p2;
#if 1

	__asm__ __volatile__ (
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

inline void mycpy(const char* d, const char* s, int len) {

	d += len;
	s += len;

	switch (len) {

	case 72:
		__asm__ __volatile__ (
				"movdqu  -72(%%rsi), %%xmm0;"
				"movdqu  -56(%%rsi), %%xmm1;"
				"mov     -40(%%rsi), %%r8;"
				"mov     -32(%%rsi), %%r9;"
				"mov     -24(%%rsi), %%r10;"
				"mov     -16(%%rsi), %%r11;"
				"mov     -8(%%rsi), %%rcx;"
				"movdqu   %%xmm0, -72(%%rdi);"
				"movdqu   %%xmm1, -56(%%rdi);"
				"mov      %%r8, -40(%%rdi);"
				"mov      %%r9, -32(%%rdi);"
				"mov      %%r10, -24(%%rdi);"
				"mov      %%r11, -16(%%rdi);"
				"mov      %%rcx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 64:
		__asm__ __volatile__ (
				"movdqu  -64(%%rsi), %%xmm0;"
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
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 56:
		__asm__ __volatile__ (
				"movdqu  -56(%%rsi), %%xmm0;"
				"mov     -40(%%rsi), %%r8;"
				"mov     -32(%%rsi), %%r9;"
				"mov     -24(%%rsi), %%r10;"
				"mov     -16(%%rsi), %%r11;"
				"mov     -8(%%rsi), %%rcx;"
				"movdqu   %%xmm0, -56(%%rdi);"
				"mov      %%r8, -40(%%rdi);"
				"mov      %%r9, -32(%%rdi);"
				"mov      %%r10, -24(%%rdi);"
				"mov      %%r11, -16(%%rdi);"
				"mov      %%rcx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 48:
		__asm__ __volatile__ (
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
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 40:
		__asm__ __volatile__ (
				"mov     -40(%%rsi), %%r8;"
				"mov     -32(%%rsi), %%r9;"
				"mov     -24(%%rsi), %%r10;"
				"mov     -16(%%rsi), %%r11;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r8, -40(%%rdi);"
				"mov      %%r9, -32(%%rdi);"
				"mov      %%r10, -24(%%rdi);"
				"mov      %%r11, -16(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 32:
		__asm__ __volatile__ (
				"mov     -32(%%rsi), %%r9;"
				"mov     -24(%%rsi), %%r10;"
				"mov     -16(%%rsi), %%r11;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r9, -32(%%rdi);"
				"mov      %%r10, -24(%%rdi);"
				"mov      %%r11, -16(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 24:
		__asm__ __volatile__ (
				"mov     -24(%%rsi), %%r10;"
				"mov     -16(%%rsi), %%r11;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r10, -24(%%rdi);"
				"mov      %%r11, -16(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 16:
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

	case 0:
		__asm__ __volatile__ (

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 73:
		__asm__ __volatile__ (
				"movdqu  -73(%%rsi), %%xmm0;"
				"movdqu  -57(%%rsi), %%xmm1;"
				"mov     -41(%%rsi), %%rcx;"
				"mov     -33(%%rsi), %%r9;"
				"mov     -25(%%rsi), %%r10;"
				"mov     -17(%%rsi), %%r11;"
				"mov     -9(%%rsi), %%r8;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -73(%%rdi);"
				"movdqu   %%xmm1, -57(%%rdi);"
				"mov      %%rcx, -41(%%rdi);"
				"mov      %%r9, -33(%%rdi);"
				"mov      %%r10, -25(%%rdi);"
				"mov      %%r11, -17(%%rdi);"
				"mov      %%r8, -9(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 65:
		__asm__ __volatile__ (
				"movdqu  -65(%%rsi), %%xmm0;"
				"movdqu  -49(%%rsi), %%xmm1;"
				"mov     -33(%%rsi), %%r9;"
				"mov     -25(%%rsi), %%r10;"
				"mov     -17(%%rsi), %%r11;"
				"mov     -9(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -65(%%rdi);"
				"movdqu   %%xmm1, -49(%%rdi);"
				"mov      %%r9, -33(%%rdi);"
				"mov      %%r10, -25(%%rdi);"
				"mov      %%r11, -17(%%rdi);"
				"mov      %%rcx, -9(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 57:
		__asm__ __volatile__ (
				"movdqu  -57(%%rsi), %%xmm0;"
				"mov     -41(%%rsi), %%r8;"
				"mov     -33(%%rsi), %%r9;"
				"mov     -25(%%rsi), %%r10;"
				"mov     -17(%%rsi), %%r11;"
				"mov     -9(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -57(%%rdi);"
				"mov      %%r8, -41(%%rdi);"
				"mov      %%r9, -33(%%rdi);"
				"mov      %%r10, -25(%%rdi);"
				"mov      %%r11, -17(%%rdi);"
				"mov      %%rcx, -9(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 49:
		__asm__ __volatile__ (
				"movdqu  -49(%%rsi), %%xmm0;"
				"mov     -33(%%rsi), %%r9;"
				"mov     -25(%%rsi), %%r10;"
				"mov     -17(%%rsi), %%r11;"
				"mov     -9(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -49(%%rdi);"
				"mov      %%r9, -33(%%rdi);"
				"mov      %%r10, -25(%%rdi);"
				"mov      %%r11, -17(%%rdi);"
				"mov      %%rcx, -9(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 41:
		__asm__ __volatile__ (
				"mov     -41(%%rsi), %%r8;"
				"mov     -33(%%rsi), %%r9;"
				"mov     -25(%%rsi), %%r10;"
				"mov     -17(%%rsi), %%r11;"
				"mov     -9(%%rsi), %%rcx;"
				"mov     -1(%%rsi), %%dl;"
				"mov      %%r8, -41(%%rdi);"
				"mov      %%r9, -33(%%rdi);"
				"mov      %%r10, -25(%%rdi);"
				"mov      %%r11, -17(%%rdi);"
				"mov      %%rcx, -9(%%rdi);"
				"mov      %%dl, -1(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 33:
		__asm__ __volatile__ (
				"mov     -33(%%rsi), %%r9;"
				"mov     -25(%%rsi), %%r10;"
				"mov     -17(%%rsi), %%r11;"
				"mov     -9(%%rsi), %%rcx;"
				"mov     -1(%%rsi), %%dl;"
				"mov      %%r9, -33(%%rdi);"
				"mov      %%r10, -25(%%rdi);"
				"mov      %%r11, -17(%%rdi);"
				"mov      %%rcx, -9(%%rdi);"
				"mov      %%dl, -1(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 25:
		__asm__ __volatile__ (
				"mov     -25(%%rsi), %%r10;"
				"mov     -17(%%rsi), %%r11;"
				"mov     -9(%%rsi), %%rcx;"
				"mov     -1(%%rsi), %%dl;"
				"mov      %%r10, -25(%%rdi);"
				"mov      %%r11, -17(%%rdi);"
				"mov      %%rcx, -9(%%rdi);"
				"mov      %%dl, -1(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 17:
		__asm__ __volatile__ (
				"mov     -17(%%rsi), %%r11;"
				"mov     -9(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%r11, -17(%%rdi);"
				"mov      %%rcx, -9(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

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

	case 74:
		__asm__ __volatile__ (
				"movdqu  -74(%%rsi), %%xmm0;"
				"movdqu  -58(%%rsi), %%xmm1;"
				"mov     -42(%%rsi), %%r8;"
				"mov     -34(%%rsi), %%r9;"
				"mov     -26(%%rsi), %%r10;"
				"mov     -18(%%rsi), %%r11;"
				"mov     -10(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -74(%%rdi);"
				"movdqu   %%xmm1, -58(%%rdi);"
				"mov      %%r8, -42(%%rdi);"
				"mov      %%r9, -34(%%rdi);"
				"mov      %%r10, -26(%%rdi);"
				"mov      %%r11, -18(%%rdi);"
				"mov      %%rcx, -10(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 66:
		__asm__ __volatile__ (
				"movdqu  -66(%%rsi), %%xmm0;"
				"movdqu  -50(%%rsi), %%xmm1;"
				"mov     -42(%%rsi), %%r8;"
				"mov     -34(%%rsi), %%r9;"
				"mov     -26(%%rsi), %%r10;"
				"mov     -18(%%rsi), %%r11;"
				"mov     -10(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -66(%%rdi);"
				"movdqu   %%xmm1, -50(%%rdi);"
				"mov      %%r8, -42(%%rdi);"
				"mov      %%r9, -34(%%rdi);"
				"mov      %%r10, -26(%%rdi);"
				"mov      %%r11, -18(%%rdi);"
				"mov      %%rcx, -10(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 58:
		__asm__ __volatile__ (
				"movdqu  -58(%%rsi), %%xmm1;"
				"mov     -42(%%rsi), %%r8;"
				"mov     -34(%%rsi), %%r9;"
				"mov     -26(%%rsi), %%r10;"
				"mov     -18(%%rsi), %%r11;"
				"mov     -10(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm1, -58(%%rdi);"
				"mov      %%r8, -42(%%rdi);"
				"mov      %%r9, -34(%%rdi);"
				"mov      %%r10, -26(%%rdi);"
				"mov      %%r11, -18(%%rdi);"
				"mov      %%rcx, -10(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 50:
		__asm__ __volatile__ (
				"movdqu  -50(%%rsi), %%xmm0;"
				"mov     -34(%%rsi), %%r9;"
				"mov     -26(%%rsi), %%r10;"
				"mov     -18(%%rsi), %%r11;"
				"mov     -10(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -50(%%rdi);"
				"mov      %%r9, -34(%%rdi);"
				"mov      %%r10, -26(%%rdi);"
				"mov      %%r11, -18(%%rdi);"
				"mov      %%rcx, -10(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 42:
		__asm__ __volatile__ (
				"mov     -42(%%rsi), %%r8;"
				"mov     -34(%%rsi), %%r9;"
				"mov     -26(%%rsi), %%r10;"
				"mov     -18(%%rsi), %%r11;"
				"mov     -10(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%r8, -42(%%rdi);"
				"mov      %%r9, -34(%%rdi);"
				"mov      %%r10, -26(%%rdi);"
				"mov      %%r11, -18(%%rdi);"
				"mov      %%rcx, -10(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 34:
		__asm__ __volatile__ (
				"mov     -34(%%rsi), %%r9;"
				"mov     -26(%%rsi), %%r10;"
				"mov     -18(%%rsi), %%r11;"
				"mov     -10(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%r9, -34(%%rdi);"
				"mov      %%r10, -26(%%rdi);"
				"mov      %%r11, -18(%%rdi);"
				"mov      %%rcx, -10(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 26:
		__asm__ __volatile__ (
				"mov     -26(%%rsi), %%r10;"
				"mov     -18(%%rsi), %%r11;"
				"mov     -10(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%r10, -26(%%rdi);"
				"mov      %%r11, -18(%%rdi);"
				"mov      %%rcx, -10(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 18:
		__asm__ __volatile__ (
				"mov     -18(%%rsi), %%r11;"
				"mov     -10(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%r11, -18(%%rdi);"
				"mov      %%rcx, -10(%%rdi);"
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

	case 75:
		__asm__ __volatile__ (
				"movdqu  -75(%%rsi), %%xmm0;"
				"movdqu  -59(%%rsi), %%xmm1;"
				"mov     -43(%%rsi), %%r8;"
				"mov     -35(%%rsi), %%r9;"
				"mov     -27(%%rsi), %%r10;"
				"mov     -19(%%rsi), %%r11;"
				"mov     -11(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -75(%%rdi);"
				"movdqu   %%xmm1, -59(%%rdi);"
				"mov      %%r8, -43(%%rdi);"
				"mov      %%r9, -35(%%rdi);"
				"mov      %%r10, -27(%%rdi);"
				"mov      %%r11, -19(%%rdi);"
				"mov      %%rcx, -11(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 67:
		__asm__ __volatile__ (
				"movdqu  -67(%%rsi), %%xmm0;"
				"movdqu  -59(%%rsi), %%xmm1;"
				"mov     -43(%%rsi), %%r8;"
				"mov     -35(%%rsi), %%r9;"
				"mov     -27(%%rsi), %%r10;"
				"mov     -19(%%rsi), %%r11;"
				"mov     -11(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -67(%%rdi);"
				"movdqu   %%xmm1, -59(%%rdi);"
				"mov      %%r8, -43(%%rdi);"
				"mov      %%r9, -35(%%rdi);"
				"mov      %%r10, -27(%%rdi);"
				"mov      %%r11, -19(%%rdi);"
				"mov      %%rcx, -11(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 59:
		__asm__ __volatile__ (
				"movdqu  -59(%%rsi), %%xmm0;"
				"mov     -43(%%rsi), %%r8;"
				"mov     -35(%%rsi), %%r9;"
				"mov     -27(%%rsi), %%r10;"
				"mov     -19(%%rsi), %%r11;"
				"mov     -11(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -59(%%rdi);"
				"mov      %%r8, -43(%%rdi);"
				"mov      %%r9, -35(%%rdi);"
				"mov      %%r10, -27(%%rdi);"
				"mov      %%r11, -19(%%rdi);"
				"mov      %%rcx, -11(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 51:
		__asm__ __volatile__ (
				"movdqu  -51(%%rsi), %%xmm0;"
				"mov     -35(%%rsi), %%r9;"
				"mov     -27(%%rsi), %%r10;"
				"mov     -19(%%rsi), %%r11;"
				"mov     -11(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -51(%%rdi);"
				"mov      %%r9, -35(%%rdi);"
				"mov      %%r10, -27(%%rdi);"
				"mov      %%r11, -19(%%rdi);"
				"mov      %%rcx, -11(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 43:
		__asm__ __volatile__ (
				"mov     -43(%%rsi), %%r8;"
				"mov     -35(%%rsi), %%r9;"
				"mov     -27(%%rsi), %%r10;"
				"mov     -19(%%rsi), %%r11;"
				"mov     -11(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%r8, -43(%%rdi);"
				"mov      %%r9, -35(%%rdi);"
				"mov      %%r10, -27(%%rdi);"
				"mov      %%r11, -19(%%rdi);"
				"mov      %%rcx, -11(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 35:
		__asm__ __volatile__ (
				"mov     -35(%%rsi), %%r9;"
				"mov     -27(%%rsi), %%r10;"
				"mov     -19(%%rsi), %%r11;"
				"mov     -11(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%r9, -35(%%rdi);"
				"mov      %%r10, -27(%%rdi);"
				"mov      %%r11, -19(%%rdi);"
				"mov      %%rcx, -11(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 27:
		__asm__ __volatile__ (
				"mov     -27(%%rsi), %%r10;"
				"mov     -19(%%rsi), %%r11;"
				"mov     -11(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%r10, -27(%%rdi);"
				"mov      %%r11, -19(%%rdi);"
				"mov      %%rcx, -11(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 19:
		__asm__ __volatile__ (
				"mov     -19(%%rsi), %%r11;"
				"mov     -11(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%r11, -19(%%rdi);"
				"mov      %%rcx, -11(%%rdi);"
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

	case 76:
		__asm__ __volatile__ (
				"movdqu  -76(%%rsi), %%xmm0;"
				"movdqu  -60(%%rsi), %%xmm1;"
				"mov     -44(%%rsi), %%r8;"
				"mov     -36(%%rsi), %%r9;"
				"mov     -28(%%rsi), %%r10;"
				"mov     -20(%%rsi), %%r11;"
				"mov     -12(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -76(%%rdi);"
				"movdqu   %%xmm1, -60(%%rdi);"
				"mov      %%r8, -44(%%rdi);"
				"mov      %%r9, -36(%%rdi);"
				"mov      %%r10, -28(%%rdi);"
				"mov      %%r11, -20(%%rdi);"
				"mov      %%rcx, -12(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 68:
		__asm__ __volatile__ (
				"movdqu  -68(%%rsi), %%xmm0;"
				"movdqu  -52(%%rsi), %%xmm1;"
				"mov     -36(%%rsi), %%r9;"
				"mov     -28(%%rsi), %%r10;"
				"mov     -20(%%rsi), %%r11;"
				"mov     -12(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -68(%%rdi);"
				"movdqu   %%xmm1, -52(%%rdi);"
				"mov      %%r9, -36(%%rdi);"
				"mov      %%r10, -28(%%rdi);"
				"mov      %%r11, -20(%%rdi);"
				"mov      %%rcx, -12(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 60:
		__asm__ __volatile__ (
				"movdqu  -60(%%rsi), %%xmm0;"
				"mov     -44(%%rsi), %%r8;"
				"mov     -36(%%rsi), %%r9;"
				"mov     -28(%%rsi), %%r10;"
				"mov     -20(%%rsi), %%r11;"
				"mov     -12(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -60(%%rdi);"
				"mov      %%r8, -44(%%rdi);"
				"mov      %%r9, -36(%%rdi);"
				"mov      %%r10, -28(%%rdi);"
				"mov      %%r11, -20(%%rdi);"
				"mov      %%rcx, -12(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 52:
		__asm__ __volatile__ (
				"movdqu  -52(%%rsi), %%xmm0;"
				"mov     -36(%%rsi), %%r9;"
				"mov     -28(%%rsi), %%r10;"
				"mov     -20(%%rsi), %%r11;"
				"mov     -12(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"movdqu   %%xmm0, -52(%%rdi);"
				"mov      %%r9, -36(%%rdi);"
				"mov      %%r10, -28(%%rdi);"
				"mov      %%r11, -20(%%rdi);"
				"mov      %%rcx, -12(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 44:
		__asm__ __volatile__ (
				"mov     -44(%%rsi), %%r8;"
				"mov     -36(%%rsi), %%r9;"
				"mov     -28(%%rsi), %%r10;"
				"mov     -20(%%rsi), %%r11;"
				"mov     -12(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%r8, -44(%%rdi);"
				"mov      %%r9, -36(%%rdi);"
				"mov      %%r10, -28(%%rdi);"
				"mov      %%r11, -20(%%rdi);"
				"mov      %%rcx, -12(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 36:
		__asm__ __volatile__ (
				"mov     -36(%%rsi), %%r9;"
				"mov     -28(%%rsi), %%r10;"
				"mov     -20(%%rsi), %%r11;"
				"mov     -12(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%r9, -36(%%rdi);"
				"mov      %%r10, -28(%%rdi);"
				"mov      %%r11, -20(%%rdi);"
				"mov      %%rcx, -12(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 28:
		__asm__ __volatile__ (
				"mov     -28(%%rsi), %%r10;"
				"mov     -20(%%rsi), %%r11;"
				"mov     -12(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%r10, -28(%%rdi);"
				"mov      %%r11, -20(%%rdi);"
				"mov      %%rcx, -12(%%rdi);"
				"mov      %%edx, -4(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 20:
		__asm__ __volatile__ (
				"mov     -20(%%rsi), %%r11;"
				"mov     -12(%%rsi), %%rcx;"
				"mov     -4(%%rsi), %%edx;"
				"mov      %%r11, -20(%%rdi);"
				"mov      %%rcx, -12(%%rdi);"
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

	case 77:
		__asm__ __volatile__ (
				"movdqu  -77(%%rsi), %%xmm0;"
				"movdqu  -61(%%rsi), %%xmm1;"
				"mov     -45(%%rsi), %%r8;"
				"mov     -37(%%rsi), %%r9;"
				"mov     -29(%%rsi), %%r10;"
				"mov     -21(%%rsi), %%r11;"
				"mov     -13(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"movdqu   %%xmm0, -77(%%rdi);"
				"movdqu   %%xmm1, -61(%%rdi);"
				"mov      %%r8, -45(%%rdi);"
				"mov      %%r9, -37(%%rdi);"
				"mov      %%r10, -29(%%rdi);"
				"mov      %%r11, -21(%%rdi);"
				"mov      %%rcx, -13(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 69:
		__asm__ __volatile__ (
				"movdqu  -69(%%rsi), %%xmm0;"
				"movdqu  -53(%%rsi), %%xmm1;"
				"mov     -37(%%rsi), %%r9;"
				"mov     -29(%%rsi), %%r10;"
				"mov     -21(%%rsi), %%r11;"
				"mov     -13(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"movdqu   %%xmm0, -69(%%rdi);"
				"movdqu   %%xmm1, -53(%%rdi);"
				"mov      %%r9, -37(%%rdi);"
				"mov      %%r10, -29(%%rdi);"
				"mov      %%r11, -21(%%rdi);"
				"mov      %%rcx, -13(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 61:
		__asm__ __volatile__ (
				"movdqu  -61(%%rsi), %%xmm0;"
				"mov     -45(%%rsi), %%r8;"
				"mov     -37(%%rsi), %%r9;"
				"mov     -29(%%rsi), %%r10;"
				"mov     -21(%%rsi), %%r11;"
				"mov     -13(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"movdqu   %%xmm0, -61(%%rdi);"
				"mov      %%r8, -45(%%rdi);"
				"mov      %%r9, -37(%%rdi);"
				"mov      %%r10, -29(%%rdi);"
				"mov      %%r11, -21(%%rdi);"
				"mov      %%rcx, -13(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 53:
		__asm__ __volatile__ (
				"movdqu  -53(%%rsi), %%xmm0;"
				"mov     -45(%%rsi), %%r8;"
				"mov     -37(%%rsi), %%r9;"
				"mov     -29(%%rsi), %%r10;"
				"mov     -21(%%rsi), %%r11;"
				"mov     -13(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"movdqu   %%xmm0, -53(%%rdi);"
				"mov      %%r9, -37(%%rdi);"
				"mov      %%r10, -29(%%rdi);"
				"mov      %%r11, -21(%%rdi);"
				"mov      %%rcx, -13(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 45:
		__asm__ __volatile__ (
				"mov     -45(%%rsi), %%r8;"
				"mov     -37(%%rsi), %%r9;"
				"mov     -29(%%rsi), %%r10;"
				"mov     -21(%%rsi), %%r11;"
				"mov     -13(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r8, -45(%%rdi);"
				"mov      %%r9, -37(%%rdi);"
				"mov      %%r10, -29(%%rdi);"
				"mov      %%r11, -21(%%rdi);"
				"mov      %%rcx, -13(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 37:
		__asm__ __volatile__ (
				"mov     -37(%%rsi), %%r9;"
				"mov     -29(%%rsi), %%r10;"
				"mov     -21(%%rsi), %%r11;"
				"mov     -13(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r9, -37(%%rdi);"
				"mov      %%r10, -29(%%rdi);"
				"mov      %%r11, -21(%%rdi);"
				"mov      %%rcx, -13(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 29:
		__asm__ __volatile__ (
				"mov     -29(%%rsi), %%r10;"
				"mov     -21(%%rsi), %%r11;"
				"mov     -13(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r10, -29(%%rdi);"
				"mov      %%r11, -21(%%rdi);"
				"mov      %%rcx, -13(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 21:
		__asm__ __volatile__ (
				"mov     -21(%%rsi), %%r11;"
				"mov     -13(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r11, -21(%%rdi);"
				"mov      %%rcx, -13(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

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

	case 78:
		__asm__ __volatile__ (
				"movdqu  -78(%%rsi), %%xmm0;"
				"movdqu  -62(%%rsi), %%xmm1;"
				"mov     -46(%%rsi), %%r8;"
				"mov     -38(%%rsi), %%r9;"
				"mov     -30(%%rsi), %%r10;"
				"mov     -22(%%rsi), %%r11;"
				"mov     -14(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"movdqu   %%xmm0, -78(%%rdi);"
				"movdqu   %%xmm1, -62(%%rdi);"
				"mov      %%r8, -46(%%rdi);"
				"mov      %%r9, -38(%%rdi);"
				"mov      %%r10, -30(%%rdi);"
				"mov      %%r11, -22(%%rdi);"
				"mov      %%rcx, -14(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 70:
		__asm__ __volatile__ (
				"movdqu  -70(%%rsi), %%xmm0;"
				"movdqu  -54(%%rsi), %%xmm1;"
				"mov     -38(%%rsi), %%r9;"
				"mov     -30(%%rsi), %%r10;"
				"mov     -22(%%rsi), %%r11;"
				"mov     -14(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"movdqu   %%xmm0, -70(%%rdi);"
				"movdqu   %%xmm1, -54(%%rdi);"
				"mov      %%r9, -38(%%rdi);"
				"mov      %%r10, -30(%%rdi);"
				"mov      %%r11, -22(%%rdi);"
				"mov      %%rcx, -14(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 62:
		__asm__ __volatile__ (
				"movdqu  -62(%%rsi), %%xmm0;"
				"mov     -46(%%rsi), %%r8;"
				"mov     -38(%%rsi), %%r9;"
				"mov     -30(%%rsi), %%r10;"
				"mov     -22(%%rsi), %%r11;"
				"mov     -14(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"movdqu   %%xmm0, -62(%%rdi);"
				"mov      %%r8, -46(%%rdi);"
				"mov      %%r9, -38(%%rdi);"
				"mov      %%r10, -30(%%rdi);"
				"mov      %%r11, -22(%%rdi);"
				"mov      %%rcx, -14(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 54:
		__asm__ __volatile__ (
				"movdqu  -54(%%rsi), %%xmm0;"
				"mov     -38(%%rsi), %%r9;"
				"mov     -30(%%rsi), %%r10;"
				"mov     -22(%%rsi), %%r11;"
				"mov     -14(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"movdqu   %%xmm0, -54(%%rdi);"
				"mov      %%r9, -38(%%rdi);"
				"mov      %%r10, -30(%%rdi);"
				"mov      %%r11, -22(%%rdi);"
				"mov      %%rcx, -14(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 46:
		__asm__ __volatile__ (
				"mov     -46(%%rsi), %%r8;"
				"mov     -38(%%rsi), %%r9;"
				"mov     -30(%%rsi), %%r10;"
				"mov     -22(%%rsi), %%r11;"
				"mov     -14(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r8, -46(%%rdi);"
				"mov      %%r9, -38(%%rdi);"
				"mov      %%r10, -30(%%rdi);"
				"mov      %%r11, -22(%%rdi);"
				"mov      %%rcx, -14(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 38:
		__asm__ __volatile__ (
				"mov     -38(%%rsi), %%r9;"
				"mov     -30(%%rsi), %%r10;"
				"mov     -22(%%rsi), %%r11;"
				"mov     -14(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r9, -38(%%rdi);"
				"mov      %%r10, -30(%%rdi);"
				"mov      %%r11, -22(%%rdi);"
				"mov      %%rcx, -14(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 30:
		__asm__ __volatile__ (
				"mov     -30(%%rsi), %%r10;"
				"mov     -22(%%rsi), %%r11;"
				"mov     -14(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r10, -30(%%rdi);"
				"mov      %%r11, -22(%%rdi);"
				"mov      %%rcx, -14(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 22:
		__asm__ __volatile__ (
				"mov     -22(%%rsi), %%r11;"
				"mov     -14(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r11, -22(%%rdi);"
				"mov      %%rcx, -14(%%rdi);"
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

	case 79:
		__asm__ __volatile__ (
				"movdqu  -79(%%rsi), %%xmm0;"
				"movdqu  -63(%%rsi), %%xmm1;"
				"mov     -47(%%rsi), %%r8;"
				"mov     -39(%%rsi), %%r9;"
				"mov     -31(%%rsi), %%r10;"
				"mov     -23(%%rsi), %%r11;"
				"mov     -15(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"movdqu   %%xmm0, -79(%%rdi);"
				"movdqu   %%xmm1, -63(%%rdi);"
				"mov      %%r8, -47(%%rdi);"
				"mov      %%r9, -39(%%rdi);"
				"mov      %%r10, -31(%%rdi);"
				"mov      %%r11, -23(%%rdi);"
				"mov      %%rcx, -15(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 71:
		__asm__ __volatile__ (
				"movdqu  -71(%%rsi), %%xmm0;"
				"movdqu  -55(%%rsi), %%xmm1;"
				"mov     -39(%%rsi), %%r9;"
				"mov     -31(%%rsi), %%r10;"
				"mov     -23(%%rsi), %%r11;"
				"mov     -15(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"movdqu   %%xmm0, -71(%%rdi);"
				"movdqu   %%xmm1, -55(%%rdi);"
				"mov      %%r9, -39(%%rdi);"
				"mov      %%r10, -31(%%rdi);"
				"mov      %%r11, -23(%%rdi);"
				"mov      %%rcx, -15(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 63:
		__asm__ __volatile__ (
				"movdqu  -63(%%rsi), %%xmm0;"
				"mov     -47(%%rsi), %%r8;"
				"mov     -39(%%rsi), %%r9;"
				"mov     -31(%%rsi), %%r10;"
				"mov     -23(%%rsi), %%r11;"
				"mov     -15(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"movdqu   %%xmm0, -63(%%rdi);"
				"mov      %%r8, -47(%%rdi);"
				"mov      %%r9, -39(%%rdi);"
				"mov      %%r10, -31(%%rdi);"
				"mov      %%r11, -23(%%rdi);"
				"mov      %%rcx, -15(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 55:
		__asm__ __volatile__ (
				"movdqu  -55(%%rsi), %%xmm0;"
				"mov     -39(%%rsi), %%r9;"
				"mov     -31(%%rsi), %%r10;"
				"mov     -23(%%rsi), %%r11;"
				"mov     -15(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"movdqu   %%xmm0, -55(%%rdi);"
				"mov      %%r9, -39(%%rdi);"
				"mov      %%r10, -31(%%rdi);"
				"mov      %%r11, -23(%%rdi);"
				"mov      %%rcx, -15(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 47:
		__asm__ __volatile__ (
				"mov     -47(%%rsi), %%r8;"
				"mov     -39(%%rsi), %%r9;"
				"mov     -31(%%rsi), %%r10;"
				"mov     -23(%%rsi), %%r11;"
				"mov     -15(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r8, -47(%%rdi);"
				"mov      %%r9, -39(%%rdi);"
				"mov      %%r10, -31(%%rdi);"
				"mov      %%r11, -23(%%rdi);"
				"mov      %%rcx, -15(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 39:
		__asm__ __volatile__ (
				"mov     -39(%%rsi), %%r9;"
				"mov     -31(%%rsi), %%r10;"
				"mov     -23(%%rsi), %%r11;"
				"mov     -15(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r9, -39(%%rdi);"
				"mov      %%r10, -31(%%rdi);"
				"mov      %%r11, -23(%%rdi);"
				"mov      %%rcx, -15(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 31:
		__asm__ __volatile__ (
				"mov     -31(%%rsi), %%r10;"
				"mov     -23(%%rsi), %%r11;"
				"mov     -15(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r10, -31(%%rdi);"
				"mov      %%r11, -23(%%rdi);"
				"mov      %%rcx, -15(%%rdi);"
				"mov      %%rdx, -8(%%rdi);"

				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"
		);
		break;

	case 23:
		__asm__ __volatile__ (
				"mov     -23(%%rsi), %%r11;"
				"mov     -15(%%rsi), %%rcx;"
				"mov     -8(%%rsi), %%rdx;"
				"mov      %%r11, -23(%%rdi);"
				"mov      %%rcx, -15(%%rdi);"
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
	case 80:
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
	case 81:
		__asm__ __volatile__ (
				"movdqu  -81(%%rsi), %%xmm0;"
				"movdqu  -65(%%rsi), %%xmm1;"
				"movdqu  -49(%%rsi), %%xmm2;"
				"movdqu  -33(%%rsi), %%xmm3;"
				"movdqu  -17(%%rsi), %%xmm4;"
				"mov -1(%%rsi), %%dl;"
				"movdqu   %%xmm0, -81(%%rdi);"
				"movdqu   %%xmm1, -65(%%rdi);"
				"movdqu   %%xmm2, -49(%%rdi);"
				"movdqu   %%xmm3, -33(%%rdi);"
				"movdqu   %%xmm4, -17(%%rdi);"
				"mov %%dl, -1(%%rdi);"
				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"

		);
		break;
	case 82:
		__asm__ __volatile__ (
				"movdqu  -82(%%rsi), %%xmm0;"
				"movdqu  -66(%%rsi), %%xmm1;"
				"movdqu  -50(%%rsi), %%xmm2;"
				"movdqu  -34(%%rsi), %%xmm3;"
				"movdqu  -18(%%rsi), %%xmm4;"
				"mov     -2(%%rsi), %%dx;"
				"movdqu   %%xmm0, -82(%%rdi);"
				"movdqu   %%xmm1, -66(%%rdi);"
				"movdqu   %%xmm2, -50(%%rdi);"
				"movdqu   %%xmm3, -34(%%rdi);"
				"movdqu   %%xmm4, -18(%%rdi);"
				"mov      %%dx, -2(%%rdi);"
				".p2align 4;"
				:
				:"S"(s),"D"(d)
				:"%eax","%edx","%ecx"

		);
		break;
	case 83:
	case 84:
	case 85:
	case 86:
	case 87:
	case 88:
	case 89:
	case 90:
	case 91:
	case 92:
	case 93:
	case 94:
	case 95:
		memcpy(const_cast<char*>(d - len), const_cast<char*>(s - len), len);
		break;
	case 96:
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
	case 97:
	case 98:
	case 99:
	case 100:
	case 101:
	case 102:
	case 103:
	case 104:
	case 105:
	case 106:
	case 107:
	case 108:
	case 109:
	case 110:
	case 111:
		memcpy(const_cast<char*>(d - len), const_cast<char*>(s - len), len);
		break;
	case 112:
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
	case 113:
	case 114:
	case 115:
	case 116:
	case 117:
	case 118:
	case 119:
	case 120:
	case 121:
	case 122:
	case 123:
	case 124:
	case 125:
	case 126:
	case 127:
		memcpy(const_cast<char*>(d - len), const_cast<char*>(s - len), len);
		break;
	case 128:
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
		return;
	}

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

int main(int argc, char** argv) {
	struct timeval t1, t2;
	gettimeofday(&t1, 0);
	if (argc < 3) {
		fprintf(stderr, "not enough args,argc=%d\n", argc);
		exit(-1);
	}

	//test();
	string input = string(argv[3]) + string("/") + string(argv[1]);
	string output = string(argv[3]) + string("/") + string(argv[2]);
	int ifd = open(input.c_str(), O_RDONLY);
	if (ifd < 0) {
		fprintf(stderr, "open file failed,err=%s\n", strerror(errno));
		exit(-1);
	}

	//char* content=malloc(1024*1024*1024);

	//if(!content){
	//	fprintf(stderr,"malloc failed\n");
	//	exit(-1);
	//}

	struct stat sb;
	if (fstat(ifd, &sb) == -1) {
		fprintf(stderr, "fstat erro,err=%s\n", strerror(errno));
		exit(-1);
	}

	size_t length = sb.st_size;

#if 0
	char* addr = (char*)mmap(NULL, length, PROT_READ,
			MAP_PRIVATE, ifd, 0);
	if (addr == MAP_FAILED) {
		fprintf(stderr,"map failed,err=%s\n",strerror(errno));
		exit(-1);
	}
#endif
	char* addr = (char*) malloc(length);
	read(ifd, addr, length);
	struct timeval tv3, tv4;
	gettimeofday(&tv3, 0);
	vector<char*> lines;
	lines.reserve(10000000 * 2);
	char* start, *pre;
	start = addr;
	pre = addr;
	char* end = addr + length;
	while ((start = strchr(start, '\n')) != 0) {
		//if(UNLIKELY(*start=='\n')){
		//ads*  a=new ads;
		//posix_memalign(&a,sizeof(ads),8);
		int len = start - pre + 1;
#if 0
		if(a->len+1<=7)
		posix_memalign(&data,4,a->len+1);
		else if(a->len+1<=15)
		posix_memalign(&data,8,a->len+1);
		else
		posix_memalign(&data,16,a->len+1);

		if(a->len+1>=16)
		posix_memalign(&data,16,a->len+1);
		else
#endif
#if 0
		void* data=0;
		posix_memalign(&data,8,len);

		memcpy(data,pre,len-1);
		char* tmp=(char*)data;
		tmp[len-1]='\0';
#endif
		pre[len - 1] = 0;
		lines.push_back(pre);
		pre = start + 1;
		start++;
		//}
	}
	close(ifd);
	//munmap(addr,length);
	gettimeofday(&tv4, 0);
	fprintf(stderr, "read interval2=%d\n",
			(tv4.tv_sec - tv3.tv_sec) * 1000
					+ (tv4.tv_usec - tv3.tv_usec) / 1000);
	char** data = lines.data();

	struct timeval tv5, tv6;
	gettimeofday(&tv5, 0);
	//qsort(data, lines.size(), sizeof(char*), cmp4);
	//sort(data,data+lines.size(),cmpads);
	//QSORT(data, lines.size(), sizeof(char*), cmp4);
	__qsort_r(data,lines.size(),sizeof(char*),(__compar_d_fn_t)cmp4,0);
	gettimeofday(&tv6, 0);

	fprintf(stderr, "sort interval3=%d\n",
			(tv6.tv_sec - tv5.tv_sec) * 1000
					+ (tv6.tv_usec - tv5.tv_usec) / 1000);
	int ofd = open(output.c_str(), O_RDWR | O_CREAT, 00644);

	if (ofd < 0) {
		fprintf(stderr, "open output error,err=%s\n", strerror(errno));
		exit(-1);
	}

#if 0
	ftruncate(ofd,length);
	char* addr1=(char*)mmap(0,length,PROT_WRITE,MAP_SHARED,ofd,0);

	if (addr1== MAP_FAILED) {
		fprintf(stderr,"write map failed,err=%s\n",strerror(errno));
		exit(-1);
	}
#endif
	struct timeval tv7, tv8;
	gettimeofday(&tv7, 0);
	char* addr1 = (char*) malloc(length);
	build_chunk(lines,addr1);
#if 0
	int index = 0;
	int size = lines.size();
	int count[128] = { 0 };
	for (int i = 0; i < size; ++i) {
		//write(ofd,lines[i]->data,lines[i]->len+1);
		//lines[i]->data[lines[i]->len-1]='\n';
		//memcpy(addr1+index,lines[i]->data,lines[i]->len);
		int len = strlen(lines[i]);
		count[len]++;
		lines[i][len] = '\n';
		//memcpy(addr1+index,lines[i],len+1);
		mycpy1(addr1 + index, lines[i], len + 1);

		index += len + 1;
	}
#endif
	gettimeofday(&tv8, 0);
	/*
	for (int i = 0; i < 128; ++i) {
		fprintf(stderr, "count%d=%d\n", i + 1, count[i]);
	}*/
	fprintf(stderr, "memcpy interval4=%d\n",
			(tv8.tv_sec - tv7.tv_sec) * 1000
					+ (tv8.tv_usec - tv7.tv_usec) / 1000);
	struct timeval tv9, tv10;
	gettimeofday(&tv9, 0);
	write(ofd, addr1, length);
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
