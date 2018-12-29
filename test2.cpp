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

using namespace std;

struct ads{
	char len;	
	char* data;
};

#define LIKELY(x) __builtin_expect(!!(x),1)
#define UNLIKELY(x) __builtin_expect(!!(x),0)


inline int
       cmpads(const void *p1, const void *p2)
       {
           /* The actual arguments to this function are "pointers to
 *               pointers to char", but strcmp(3) arguments are "pointers
 *                             to char", hence the following cast plus dereference */
	ads* a1=*(ads**)p1;
	ads* a2=*(ads**)p2;
	int i=0;
	while(UNLIKELY(a1->data[i]!='\n')&&UNLIKELY(a2->data[i]!='\n')&&UNLIKELY(a1->data[i]==a2->data[i]))++i;
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
	int ret1=memcmp(a1->data,a2->data,len);
	if(UNLIKELY(ret1==0))return ret;
	else return ret1;
	#endif	
		
#if 0
	if(a1->data[i]=='\n'&&a2->data[i]=='\n')
		return 0;
	else if(a1->data[i]=='\n'&&a2->data[i]!='\n'){
		return -1;
	}
	else if(a1->data[i]!='\n'&&a2->data[i]=='\n')
		return 1;
#endif
	return a1->data[i]-a2->data[i];
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
	vector<ads*> lines;
	lines.reserve(10000000*2);
	char* start,*pre;
	start=addr;
	pre=addr;
	char* end=addr+length;
	for(;start<end;++start){
		if(UNLIKELY(*start=='\n')){
			ads*  a=new ads;
			a->len=start-pre;
			a->data=pre;
			pre=start+1;
			lines.push_back(a);
		}
	}	
	gettimeofday(&tv4,0);
	fprintf(stderr,"read interval2=%d\n",(tv4.tv_sec-tv3.tv_sec)*1000+(tv4.tv_usec-tv3.tv_usec)/1000);
	ads** data=lines.data();
	

	struct timeval tv5,tv6;
	gettimeofday(&tv5,0);
	qsort(data,lines.size(),sizeof(ads*),cmpads);	
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
	//addr1[0]='a';
	struct timeval tv7,tv8;
        gettimeofday(&tv7,0);
	int index=0;
	for(int i=0;i<lines.size();++i){
		//write(ofd,lines[i]->data,lines[i]->len+1);
		memcpy(addr1+index,lines[i]->data,lines[i]->len+1);
		index+=lines[i]->len+1;
	}
	gettimeofday(&tv8,0);
	fprintf(stderr,"output interval4=%d\n",(tv8.tv_sec-tv7.tv_sec)*1000+(tv8.tv_usec-tv7.tv_usec)/1000);
	munmap(addr1,length);
	close(ifd);
	close(ofd);
	munmap(addr,length);
	gettimeofday(&t2,0);
	fprintf(stderr,"sum interval=%d\n",(t2.tv_sec-t1.tv_sec)*1000+(t2.tv_usec-t1.tv_usec)/1000);
	return 0;		
	
}
