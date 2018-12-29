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

using namespace std;

struct ads{
	char* data;
	int len;
};

#define LIKELY(x) __builtin_expect(!!(x),1)
#define UNLIKELY(x) __builtin_expect(!!(x),0)
#define sg_align(d, a)     (((d) + (a - 1)) & ~(a - 1))

inline int my_memcmp( const void *ads1, const void *ads2)
{
   ads* a1=*(ads**)ads1;
   ads* a2=*(ads**)ads2;
   void* s1=a1->data;
   void* s2=a2->data;
    __builtin_prefetch(a1->data);
        __builtin_prefetch(a2->data);
   int len=a1->len>a2->len?a2->len:a1->len;
    len+=1;
    const int *p1, *q1;
    const char *p2, *q2;
    int off, mod;

    off = len >> 2;
    mod = len - (off << 2);

    if (mod > 0) {
        p2 = (const char *)s1;
        q2 = (const char *)s2;
        while (mod --) {
            if (*p2 ++ != *q2 ++) {
                return p2[-1] > q2[-1] ? 1 : -1;
            }
        }
    }

    /* if p1 & q1 address don't align with 4 bytes,
 *        how about the efficiency ? */
    mod = len - (off << 2);
    p1 = (const int *)(s1 + mod);
    q1 = (const int *)(s2 + mod);

    while (off --) {
        if (*p1 ++ != *q1 ++) {
            return p1[-1] > q1[-1] ? 1 : -1;
        }
    }

    return 0;
}

inline int my_memcmp1( const void *ads1, const void *ads2)
{
   ads* a1=*(ads**)ads1;
   ads* a2=*(ads**)ads2;
   __builtin_prefetch(a1->data);
        __builtin_prefetch(a2->data);
  if(a1->data[0]!=a2->data[0])
       return a1->data[0]-a2->data[0];

   void* s1=a1->data;
   void* s2=a2->data;
   int len=a1->len>a2->len?a2->len:a1->len;
   len++;

   const __int128_t* t1,*t2;
   t1=(const __int128_t*)s1;
   t2=(const __int128_t*)s2;
   if(len>=16){
	int off,mod;
	off=len>>4;
	mod=len-len>>4;
	len=mod;
	while(off--){
		if(*t1^*t2){
			len=16;
			break;
		}
		t1++;t2++;
	}
   }
 

   const long *r1,*r2;
   int off1,mod1;
   off1=len>>3;
   mod1=len-(off1<<3);
   len=mod1;
   r1=(const long*)(t1);
   r2=(const long*)(t2);
   while(off1--){
	if(*r1++^*r2++){
		//return *(r1-1)-*(r2-1);
		r1--;
		r2--;
		len=8;
	}
   }

    const int *p1, *q1;

    p1 = (const int *)(r1);
    q1 = (const int *)(r2);
    int off=len>>2;
    int mod=len-off<<2;
    len=mod;
    while (off --) {
        if (*p1 ++^ *q1 ++) {
            p1--;q1--;
            len=4;
        }
    } 	

    //s1=(char*)p1;
    //s2=(char*)p2;
     
    if (len > 0) {
        const char* p2 = (const char *)p1;
        const char* q2 = (const char *)q1;
        while (len --) {
            if (*p2 ++ ^ *q2 ++) {
                return *(p2-1)-*(q2-1);
            }
        }
    }


    return 0;
}



inline int my_memcmp2( const void *ads1, const void *ads2)
{
   ads* a1=*(ads**)ads1;
   ads* a2=*(ads**)ads2;

   if(a1->data[0]!=a2->data[0])
       return a1->data[0]-a2->data[0];
   void* s1=a1->data;
   void* s2=a2->data;
   int len=a1->len>a2->len?a2->len:a1->len;
    len+=1;
    const int *p1, *q1;
    const char *p2, *q2;
    int off, mod;

    off = len >> 2;
    mod = len - (off << 2);

     p1=(const int*)s1;
     q1=(const int*)s2;
     while (off --) {
        if (*p1 ^*q1) {
	    return __builtin_bswap32(*p1)-__builtin_bswap32(*q1);
        }
	p1++;q1++;
    }


    if (mod>0) {
        p2 = (const char *)p1;
        q2 = (const char *)q1;
        while (mod --) {
            if (*p2 ++ != *q2 ++) {
                return p2[-1] > q2[-1] ? 1 : -1;
            }
        }
    }


    return 0;
}

inline int my_memcmp3( const void *ads1, const void *ads2)
{
   ads* a1=*(ads**)ads1;
   ads* a2=*(ads**)ads2;
   //__builtin_prefetch(a1->data);
   //__builtin_prefetch(a2->data);
  // if(a1->data[0]!=a2->data[0])
    //   return a1->data[0]-a2->data[0];
   

   //char* s1=a1->data;
   //char* s2=a2->data;
   int len=a1->len>a2->len?a2->len:a1->len;
   
   int off, mod; 

#if 0
   if(len>=16){
#if 0
   const __int128_t* t1,*t2;
   t1=(const __int128_t*)s1;
   t2=(const __int128_t*)s2;
   off=len>>4;
   mod=len-off<<4;
   len=mod;

   while(off--){
	if(*t1^*t2){
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
 
if(len>=16){ 
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
	" pxor	%%xmm0, %%xmm0;" \
	"2:" \
	" movdqu	(%%rdi), %%xmm2;" \
	" pxor	(%%rsi), %%xmm2;" \
 	" ptest	%%xmm2, %%xmm0;" \
        " jnc 1f;" \
	"add $16,%%rsi;" \
	"add $16,%%rdi;" \
	"sub $1,%%edx;" \
        "test %%edx,%%edx;" \
	"jz 3f;"
	"jmp 2b;" \
	"1:" \
	"mov $16,%%eax; " \
        "mov %%eax, %0;"  \
	"3:"
	"movq %%rsi,%1;" \
	"movq %%rdi,%2;"
	:"=m"(len),"=&m"(s1),"=&m"(s2)  \
	:"S"(s1),"D"(s2),"d"(off) \
	:
);
//if(len!=16)
//	printf("after s1=%lx,s2=%lx,t1=%lx,t2=%lx,len=%d,ori_len=%d,mod=%d,off=%d\n",s1,s2,t1,t2,len,ori_len,mod,off);
}

#endif

    const  long *p1, *q1;
    const char *p2, *q2;

    off = len >> 3;
    mod = len - (off << 3);

     p1=(long*)a1->data;
     q1=(long*)a2->data;
    
#if 1 
     while (off --) {
        if (*p1 ^ *q1) {
	    return ((long)__builtin_bswap64(*p1)-(long)__builtin_bswap64(*q1))>0?1:-1;
        }
	__builtin_prefetch(p1+1);
	__builtin_prefetch(q1+1);
	p1++;q1++;
    }
#endif

#if 0
    long ld=0;
    const long* lend=p1+off;
    for(;;){
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
	return ((long)__builtin_bswap64(p1[-1])-(long)__builtin_bswap64(q1[-1]))>0?1:-1 ;
    #endif
    
        p2 = (const char *)p1;
        q2 = (const char *)q1;
	#if 1
	if(mod>=4){
		mod-=4;
		if(LIKELY(*((int*)p2)^*((int*)q2))){
			return __builtin_bswap32(*((int*)p2))-__builtin_bswap32(*((int*)q2));
		}else{
			p2+=4;
			q2+=4;
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
	const char* end=p2+mod;
	int  d = 0;
	for (;;) {
        if (d || p2 >= end) break;
        d = *p2++ - *q2++;
 
        if (d || p2>= end) break;
        d = *p2++ - *q2++;
 
        if (d || p2 >= end) break;
        d = *p2++ - *q2++;
 
        }
    return d;
	#endif

}


inline int
       cmpads(const void *p1, const void *p2)
       {
           /* The actual arguments to this function are "pointers to
 *               pointers to char", but strcmp(3) arguments are "pointers
 *                             to char", hence the following cast plus dereference */
	ads* a1=*(ads**)p1;
	ads* a2=*(ads**)p2;
	//int i=0;
	//while(UNLIKELY(a1->data[i]!='\n')&&UNLIKELY(a2->data[i]!='\n')&&UNLIKELY(a1->data[i]==a2->data[i]))++i;
	#if 0
	int len=0,ret=0;
	if(a1->len<a2->len){
		len=a1->len;
		ret=-1;
	}else{
		len=a2->len;
		ret=1;
		if(a2->len==a1->len)
			ret=0;
	}
	#endif

	int len=a1->len>a2->len?a2->len:a1->len;
	#if 0
	if(len>64)
		printf("len=%d\n",len);
	#endif

	#if 0
	if(len>8){
	char* addr1=(char*)sg_align((long)(a1->data),8);
		printf("orig=%lx,alg=%lx\n",a1->data,addr1);
	}
	#endif

	/*else if(len==2){
		return *((short*)(a1->data[0]))-*((short*)(a2->data[0]));
	}else if(len==4){
		return *((int*)(a1->data[0]))-*((int*)(a2->data[0]));
	}*/
	return memcmp(a1->data,a2->data,len+1);
		
#if 0
	if(a1->data[i]=='\n'&&a2->data[i]=='\n')
		return 0;
	else if(a1->data[i]=='\n'&&a2->data[i]!='\n'){
		return -1;
	}
	else if(a1->data[i]!='\n'&&a2->data[i]=='\n')
		return 1;
#endif
	//return a1->data[i]-a2->data[i];
       }



inline int cmp4(const void *p1, const void *p2){
	char* a1=*(char**)p1;
        char* a2=*(char**)p2;
#if 1

	__asm__ __volatile__ (
		"sub %%rdi,%%rsi;" \
		"sub $16,%%rdi;" \
		"strcmpLoop:"
		"add  $16 ,%%rdi;" \
		"movdqu (%%rdi),%%xmm0;" \
	 	"pcmpistri $0x1a,(%%rdi,%%rsi),%%xmm0;" \
		"ja	strcmpLoop;" \
		"jc strcmpDiff;" \
		"xor	%%eax, %%eax;" \
		"strcmpDiff:" \
		"addq %%rdi,%%rsi;" \
		"movzx (%%rsi,%%rcx),%%eax;" \
		"movzx (%%rdi,%%rcx),%%edx;" \
		"sub %%edx,%%eax;" \
		"exitStrcmp:"
		:
		:"S"(a1),"D"(a2)
		:"%eax","%edx"
	);	
#endif
//return strcmp(a1->data,a2->data);
}

int main(int argc ,char** argv){
	struct timeval t1,t2;
	gettimeofday(&t1,0);
	if(argc<3){
		fprintf(stderr,"not enough args,argc=%d\n",argc);
		exit(-1);
	}


	string input=string(argv[3])+string("/")+string(argv[1]);
	string output=string(argv[3])+string("/")+string(argv[2]);
	int ifd=open(input.c_str(),O_RDONLY);
	if(ifd<0){
		fprintf(stderr,"open file failed,err=%s\n",strerror(errno));
		exit(-1);
	}

	//char* content=malloc(1024*1024*1024);

	//if(!content){
	//	fprintf(stderr,"malloc failed\n");
	//	exit(-1);
	//}

	struct stat sb;
	if (fstat(ifd, &sb) == -1){
		fprintf(stderr,"fstat erro,err=%s\n",strerror(errno));
		exit(-1);
	}

	size_t length=sb.st_size;

	char* addr = (char*)mmap(NULL, length, PROT_READ,
                       MAP_PRIVATE, ifd, 0);
	if (addr == MAP_FAILED){
		fprintf(stderr,"map failed,err=%s\n",strerror(errno));
		exit(-1);
	}
	struct timeval tv3,tv4;
        gettimeofday(&tv3,0);
	vector<char*> lines;
	lines.reserve(10000000*2);
	char* start,*pre;
	start=addr;
	pre=addr;
	char* end=addr+length;
	for(;start<end;++start){
		if(UNLIKELY(*start=='\n')){
			 //ads*  a=new ads;
			//posix_memalign(&a,sizeof(ads),8);
			int len=start-pre+1;
			void* data=0;
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
				posix_memalign(&data,8,len);
		
			memcpy(data,pre,len-1);
			char* tmp=(char*)data;
			tmp[len-1]='\0';
			pre=start+1;
			lines.push_back(tmp);
		}
	}
	        close(ifd);
	        munmap(addr,length);	
	gettimeofday(&tv4,0);
	fprintf(stderr,"read interval2=%d\n",(tv4.tv_sec-tv3.tv_sec)*1000+(tv4.tv_usec-tv3.tv_usec)/1000);
	char** data=lines.data();
	

	struct timeval tv5,tv6;
	gettimeofday(&tv5,0);
	qsort(data,lines.size(),sizeof(char*),cmp4);	
	//sort(data,data+lines.size(),cmpads);
	gettimeofday(&tv6,0);

	fprintf(stderr,"sort interval3=%d\n",(tv6.tv_sec-tv5.tv_sec)*1000+(tv6.tv_usec-tv5.tv_usec)/1000);
	int ofd=open(output.c_str(),O_RDWR|O_CREAT,00644);	

	if(ofd<0){
		fprintf(stderr,"open output error,err=%s\n",strerror(errno));
		exit(-1);
	}
	
	ftruncate(ofd,length);
	char* addr1=(char*)mmap(0,length,PROT_WRITE,MAP_SHARED,ofd,0);


	 if (addr1== MAP_FAILED){
                fprintf(stderr,"write map failed,err=%s\n",strerror(errno));
                exit(-1);
        }
	struct timeval tv7,tv8;
        gettimeofday(&tv7,0);
	int index=0;
	int size=lines.size();
	for(int i=0;i<size;++i){
		//write(ofd,lines[i]->data,lines[i]->len+1);
		//lines[i]->data[lines[i]->len-1]='\n';
		//memcpy(addr1+index,lines[i]->data,lines[i]->len);
		int len=strlen(lines[i]);
		lines[i][len]='\n';
		memcpy(addr1+index,lines[i],len+1);
		index+=len+1;
	}
	gettimeofday(&tv8,0);
	fprintf(stderr,"output interval4=%d\n",(tv8.tv_sec-tv7.tv_sec)*1000+(tv8.tv_usec-tv7.tv_usec)/1000);
	munmap(addr1,length);
//	close(ifd);
	close(ofd);
//	munmap(addr,length);
	gettimeofday(&t2,0);
	fprintf(stderr,"sum interval=%d\n",(t2.tv_sec-t1.tv_sec)*1000+(t2.tv_usec-t1.tv_usec)/1000);
	return 0;		
	
}
