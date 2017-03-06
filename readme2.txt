（1）在ssd.c中添加2个头文件 ：
	#include <io.h>
	#include <Windows.h>
（2）在initialize.h
	struct ssd_info{...}中需要填加4个变量：
			long long filesize;		//trace文件大小
			long long filelines;		//trace条目数
			struct trace_info ptr,*ptr; //mmap的文件头指针
			long long current_traceline;//当前行数
	在这个头文件添加一个结构体：
	struct trace_info{
		__int64 time_t;
		int device;
		int lsn;
		int size;
		int op;
	};

（3）在ssd.c的main()中，在ssd=initiation(ssd);后面添加如下代码:

	ssd->tracefile = fopen(ssd->tracefilename, "rb"); //用fopen_s的方式打开文件，ssd->ptr就不能访问了
	my_mmap(ssd);
	
	并在main()最后：
		fclose(ssd->tracefile);

（4）在ssd.c中添加一个函数，并在ssd.h申明一下：
	void my_mmap(struct ssd_info *ssd)
	{
		ssd->filesize = filelength(fileno(ssd->tracefile));
		ssd->filelines = ssd->filesize / sizeof(struct trace_info);
		HANDLE dumpFileDescriptor = CreateFileA(ssd->tracefilename,
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
		ssd->ptr =(struct trace_info *)MapViewOfFile(fileMappingObject,
                  	FILE_MAP_ALL_ACCESS,
                	0,	
                  	0,
			ssd->filesize);
	}
(5)在ssd.c的get_requests()函数中，将如下几行代码注释掉：
	if(feof(ssd->tracefile)){		//操作trace文件的代码都要去掉
		return 100; 
	}

	filepoint = ftell(ssd->tracefile);	
	fgets(buffer, 200, ssd->tracefile); 
	sscanf(buffer,"%I64u %d %d %d %d",&time_t,&device,&lsn,&size,&ope);
	
	将这几行函数改成：
	if (ssd->current_traceline < ssd->filelines) {
		time_t = ssd->ptr[ssd->current_traceline].time_t;
		device = ssd->ptr[ssd->current_traceline].device;
		lsn = ssd->ptr[ssd->current_traceline].lsn;
		size = ssd->ptr[ssd->current_traceline].size;
		ope = ssd->ptr[ssd->current_traceline].lsn;
		ssd->current_traceline++;
	} else {
		return 100;
	}

	在trace回滚那里(有2处)：
	注释掉:		fseek(ssd->tracefile,filepoint,0); 
	改为:		ssd->current_traceline--;

	这个函数的最后，下面5行都注释掉(本来是用于找下一条trace,获取时间，然后回滚的，现在不需要了)：
	filepoint = ftell(ssd->tracefile);	
	fgets(buffer, 200, ssd->tracefile);    //寻找下一条请求的到达时间
	sscanf(buffer,"%I64u %d %d %d %d",&time_t,&device,&lsn,&size,&ope);
	ssd->next_request_time=time_t;
	fseek(ssd->tracefile,filepoint,0);

	再添加一行：
		ssd->next_request_time = ssd->ptr[ssd->current_traceline].time_t;

（6）在ssd.c的simulate()函数中，注释掉下面四行：
	if((err=fopen_s(&(ssd->tracefile),ssd->tracefilename,"r"))!=0)
	{  
		printf("the trace file can't open\n");
		return NULL;
	}

	改成：
		ssd->current_tracelines = 0;		//原来是打开文件，读取完成后关闭，trace被读过2次，现在只打开了一次，需要重新读取文件的话，加一句这个就可以了

（7）pagemap.c的pre_process_page（）函数:
	下面几行注释掉：
	if((err=fopen_s(&(ssd->tracefile),ssd->tracefilename,"r")) != 0 )      /*打开trace文件从中读取请求*/
	{
		printf("the trace file can't open\n");
		return NULL;
	}
	
	修改下面几行：
	while(fgets(buffer_request,200,ssd->tracefile))
	{
		sscanf_s(buffer_request,"%I64u %d %d %d %d",&time,&device,&lsn,&size,&ope);
	
	改成（反正和trace文件相关的操作都需要修改一下）：
	while(ssd->current_traceline < ssd->tracelines)
	{
		//sscanf_s(buffer_request,"%I64u %d %d %d %d",&time,&device,&lsn,&size,&ope);
		time = ssd->ptr[ssd->current_traceline].time_t;
		device = ssd->ptr[ssd->current_traceline].device;
		lsn = ssd->ptr[ssd->current_traceline].lsn;
		size = ssd->ptr[ssd->current_traceline].size;
		ope = ssd->ptr[ssd->current_traceline].lsn;
		ssd->current_traceline++;
	
	注释掉最后面的fclose(ssd->tracefile);
	
	
	