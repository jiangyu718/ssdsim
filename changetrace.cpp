#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <io.h>

struct trace_info{
	__int64 time_t;
	int device;
	int lsn;
	int size;
	int op;
};

int main()
{
	char *buf;
	char *filename = "Financial2.ascii";
	char *out_filename = "to_Financial2.ascii";
	char *newfilename = "t_Financial2.ascii";
	FILE *fp=NULL,*fpo=NULL;
	FILE *fpr=NULL,*fp3=NULL;
	struct trace_info ptr,*pptr;
	char buffer[200];
	long long size;
	int i;

	fopen_s(&fp,filename,"r");
	fopen_s(&fpo,out_filename,"wb");
	while(fgets(buffer, 200, fp)) {
		sscanf(buffer, "%I64u %d %d %d %d",&ptr.time_t,&ptr.device,&ptr.lsn,&ptr.size,&ptr.op);
		fwrite(&ptr, sizeof(struct trace_info), 1, fpo);
	} 
	fclose(fp);
	fclose(fpo);
/*
	fopen_s(&fpr,out_filename,"rb");
	fopen_s(&fp3,newfilename,"w");
	while (fread(&ptr, sizeof(struct trace_info), 1, fpr)) {
		fprintf(fp3, "%I64u %d %d %d %d\n", ptr.time_t,ptr.device,ptr.lsn,ptr.size,ptr.op);
	};
	fclose(fpr);
	fclose(fp3);
*/	
//	pptr = (struct trace_info *)mmap(NULL,nFileSize,PROT_READ,MAP_SHARED,ssd->fd_trace,0); 
/*	fpr = fopen(out_filename, "rb");
	size = filelength(fileno(fpr));
	printf("%d ",size/1024);

	HANDLE dumpFileDescriptor = CreateFileA(out_filename,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
	HANDLE fileMappingObject = CreateFileMapping(dumpFileDescriptor,
                      NULL,
                      PAGE_READWRITE,
                      0,
                      0,
                      NULL);
	pptr =(struct trace_info *)MapViewOfFile(fileMappingObject,
                      FILE_MAP_ALL_ACCESS,
                      0,
                      0,
                      size);
	size /= sizeof(struct trace_info);
	printf("%d ",size);
	fopen_s(&fp3,newfilename,"w");
	for (i = 0; i < size; ++i) {
		//printf("%d ",i);
		fprintf(fp3, "%I64u %d %d %d %d\n",pptr[i].time_t,pptr[i].device,pptr[i].lsn,pptr[i].size,pptr[i].op);
	}
	fclose(fpr);
	fclose(fp3);
	*/
}