#include <cstdio>
#include <cstring>
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

extern void mycpy(const char* d,const char* s,int len);

int main(int argc,char** argv){
	char* s1 ="cabb";
	char* s2="aacc";
	unsigned int i1=*(unsigned int*)s1;
	unsigned int i2=*(unsigned int*)s2;
	printf("i1=%x,i2=%x,swap i1=%x,swap i2=%x\n",i1,i2,__builtin_bswap32(i1),__builtin_bswap32(i2));

	char* s3="dzuiwmqq";
	char* s4="dgsmyzic";

	long l1=*(long*)s3;
	long l2=*(long*)s4;
	long diff=l1-l2;
	printf("l1=%lx,l2=%lx,swap l1=%lx,swap l2=%lx,diff=%lx\n",l1,l2,__builtin_bswap64(l1),__builtin_bswap64(l2),diff);
	long t=-16;
	printf("t=%lx\n",t);
	for(int i=0;i<2;++i){
		if(i==1)
			break;
	}
	int len=strlen(s3);	
	char* pos=strchr(s3,'\n');

	struct timeval tv3,tv4;	
	gettimeofday(&tv3,0);
	string input="demo/test1024m";
	int ifd=open(input.c_str(),O_RDONLY);
	struct stat sb;
        if (fstat(ifd, &sb) == -1){
                fprintf(stderr,"fstat erro,err=%s\n",strerror(errno));
                exit(-1);
        }

        size_t length=sb.st_size;
	char* addr=(char*)malloc(length);
	printf("lenght=%d\n",length);
	read(ifd,addr,length);
	readahead(ifd,0,length);
	gettimeofday(&tv4,0);
        fprintf(stderr,"read interval2=%d\n",(tv4.tv_sec-tv3.tv_sec)*1000+(tv4.tv_usec-tv3.tv_usec)/1000);

	char tmp[20];

	mycpy(tmp,s3,strlen(s3)+1);
	fprintf(stderr,"dst str=%s\n",tmp);	
	return 0;
}
