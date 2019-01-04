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
#include <algorithm>

using namespace std;


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


		void sort(vector<vector<char*>>& input,vector<char*>&output,int (*compar)(const void *, const void *),int thr,int step){
                        cmp=(__compar_d_fn_t)compar;
                        k=input.size();
                        ls.reserve(k);
                        b.reserve(k+1);
			index.reserve(k);
			int start=thr*step;
                        for(int i=0;i<k;++i){
                                if(input[i].size()>start){
                                        b.push_back(input[i][start]);
                                }else
                                        b.push_back(0);
				index.push_back(start);

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
		vector<int> index;
		__compar_d_fn_t cmp;	
		void adjust(int i){
			int t=(i+k)/2;
			while(t>0){
				if(ls[t]==k||!b[i]||(i!=k&&b[ls[t]]&&cmp(&b[i],&b[ls[t]],0)>0)){
					int tmp=i;
					i=ls[t];
					ls[t]=tmp;
				}
				t/=2;
			}

			ls[0]=i;
			
		}

	

		void create_loser_tree(){
			b.push_back("");
			for(int i=0;i<k;++i){
				ls.push_back(k);
			}
			for(int i=k-1;i>=0;--i){
				adjust(i);
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
	char* pre;
	char* data;
	vector<char*>* output;
	int thr;
	int len;
	int real_len;
};


void* sort_handle(void* arg){
          //vector<char*> lines;
        //lines.reserve(10000000 * 2);
        fprintf(stderr,"in sort thread\n");
        thread_args* ta=(thread_args*)arg;
        char* addr=ta->data;
        char* end=addr+ta->len;
        if(ta->thr!=0){
                while(addr-1>=ta->pre&&*(addr-1)!='\n'&&*(addr-1)!=0)addr--;
                //addr++;
        }
        char* start, *pre;
        start = addr;
        pre = addr;
        while ((start = strchr(start, '\n')) != 0&&start<end) {
                int len = start - pre + 1;
                pre[len - 1] = 0;
                //fprintf(stderr,"pre=%s,thr=%d\n",pre,ta->thr);
                ta->output->push_back(pre);
                pre = start + 1;
                start++;
		real_len+=len;
        }

        char** data = ta->output->data();

        struct timeval tv5, tv6;
        gettimeofday(&tv5, 0);
        qsort(data, ta->output->size(), sizeof(char*), cmp4);
        //sort(data,data+lines.size(),cmpads);
        //QSORT(data, lines.size(), sizeof(char*), cmp4);
        //mysort(data,lines.size(),sizeof(char*),(__compar_d_fn_t)cmp4,0);
        //heap_sort(data,ta->output->size(),cmp5,ta->fifo);
        gettimeofday(&tv6, 0);

        fprintf(stderr, "thread %d,lines num=%d,len=%d,sort interval3=%d\n",ta->thr,ta->output->size(),ta->len,
                        (tv6.tv_sec - tv5.tv_sec) * 1000
                                      + (tv6.tv_usec - tv5.tv_usec) / 1000);
        return 0;
}


struct merge_thread_args{
	int thr;
	vector<char*>* sort_lines;
	vector<vector<char*>>* input;
	int step;
};

void* merge_handle(void* arg){
	fprintf(stderr,"in merge thread\n");
	struct timeval tv1, tv2;
	gettimeofday(&tv1, 0);
        merge_sort ms;
	merge_thread_args* ta=(merge_thread_args*)arg;
        ms.sort(input,*(ta->sort_lines),cmp4,ta->thr,ta->step);
	gettimeofday(&tv2, 0);
	fprintf(stderr, "merge thread,output size=%d,sort interval=%d\n",ta->sort_lines->size(),
                        (tv2.tv_sec - tv1.tv_sec) * 1000
                                      + (tv2.tv_usec - tv1.tv_usec) / 1000);
        return 0;
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


	struct timeval t3, t4;
	gettimeofday(&t3, 0);
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

	const int thread_num=6;
	
	char* addr = (char*) malloc(length);
	int i=0;
	int block=length/thread_num;
	thread_args args[thread_num];
	vector<vector<char*>> output_lines;
	vector<pthread_t> tids;
	tids.reserve(thread_num);
	output_lines.reserve(thread_num);
	kfifo* fifo[thread_num];


	for(int i=0;i<thread_num;++i){
		vector<char*> tmp;
		output_lines.push_back(tmp);
	}
	int beg=0;
	while(i<thread_num){
		pthread_t tid;
		if(i==thread_num-1){
			read(ifd,addr+beg,length-block*(thread_num-1));
			args[i].len=length-block*(thread_num-1);
		}
		else{
			read(ifd, addr+beg, block);
			args[i].len=block;
		}
		if(i==0)
			args[i].pre=0;
		else
			args[i].pre=args[i-1].data;
		args[i].data=addr+beg;
		args[i].output=&output_lines[i];
		args[i].thr=i;
		beg+=block;
		pthread_create(&tid,0,sort_handle,&args[i]);
		tids.push_back(tid);
		++i;
	}

	 for(int i=0;i<tids.size();++i){
                pthread_join(tids[i],0);
        }


	i=0;
	tids.clear();
	vector<merge_thread_args> mtas
	vector<vector<char*>> sort_lines;

	int lines_num=0;	
        for(int i=0;i<thread_num;++i){
                vector<char*> tmp;
                sort_lines.push_back(tmp);
		lines_num+=output_lines[i].size();
        }
	
	int step=lines_num/(thread_num*thread_num);
	while(i<thread_num){
		pthread_t tid;
		mtas[i].sort_lines=&sort_lines[i];
		mtas[i].thr=i;
		mtas[i].input=&output_lines;
		mtas[i].step=step;
		pthread_create(&tid,0,merge_handle,&mtas[i]);
		tids.push_back(tid);
	}
	for(int i=0;i<tids.size();++i){
		pthread_join(tids[i],0);
	}


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
