import sys
import re

base='''
case %s:
 __asm__ __volatile__ (
    %s
    ".p2align 4;"
    :
    :"S"(s),"D"(d)
    :"%%eax","%%edx","%%ecx"
    );
        break;
'''


input_file=sys.argv[1]

input_handle=open(input_file,"r")

output_file=sys.argv[2]
output_handle=open(output_file,"w")


code=""
l=0
for line in input_handle.readlines():
	line=line.strip("\n")
	g=re.search("write_(\d+)bytes",line)
	if g:
		code=""
		l=g.group(1)
		print "len:",l
	else:
		if line.find("ret")==-1:
			line=line.strip(" \n\r\t")
			n="\t\""+line+";\"" +" \\\n"
			n=n.replace("%","%%");
			code+=n
		else:
			print code
			block=base %(l,code)
			print block
			output_handle.write(block)	
	
