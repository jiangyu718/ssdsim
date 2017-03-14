ʵ�����������Լ��ܳ�����������ģ�����ԭ�����mmap���ؼ������Ǹ�my_mmap��������������൱��windows�µ�mmap��
��0������changetrace�Ĵ����traceת��һ��
��1����ssd.c�����2��ͷ�ļ� ��
	#include <io.h>
	#include <Windows.h>
��2����initialize.h
	struct ssd_info{...}����Ҫ���4��������
			long long tracesize;		//trace�ļ���С
			long long tracelines;		//trace��Ŀ��
			struct trace_info *ptr; //mmap���ļ�ͷָ��
			long long current_traceline;//��ǰ����
	�����ͷ�ļ����һ���ṹ�壺
	struct trace_info{
		__int64 time_t;		
		int device;
		int lsn;
		int size;
		int op;
	};

��3����ssd.c��main()�У���ssd=initiation(ssd);����������´���:

	ssd->tracefile = fopen(ssd->tracefilename, "rb"); //��fopen_s�ķ�ʽ���ļ���ssd->ptr�Ͳ��ܷ�����
	my_mmap(ssd);
	
	����main()���
		fclose(ssd->tracefile);

��4����ssd.c�����һ������������ssd.h����һ�£�
	void my_mmap(struct ssd_info *ssd)
	{
		HANDLE dumpFileDescriptor;
		HANDLE fileMappingObject;
		ssd->tracesize = filelength(fileno(ssd->tracefile));
		ssd->tracelines = ssd->tracesize / sizeof(struct trace_info);
		dumpFileDescriptor = CreateFileA(ssd->tracefilename,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);
		fileMappingObject = CreateFileMapping(dumpFileDescriptor,
                        NULL,
                        PAGE_READWRITE,
                        0,
                        0,
                        NULL);
		ssd->ptr =(struct trace_info *)MapViewOfFile(fileMappingObject,
                  	FILE_MAP_ALL_ACCESS,
                	0,	
                  	0,
			ssd->tracesize);
	}
(5)��ssd.c��get_requests()�����У������¼��д���ע�͵���
	if(feof(ssd->tracefile)){		//����trace�ļ��Ĵ��붼Ҫȥ��
		return 100; 
	}

	filepoint = ftell(ssd->tracefile);	
	fgets(buffer, 200, ssd->tracefile); 
	sscanf(buffer,"%I64u %d %d %d %d",&time_t,&device,&lsn,&size,&ope);
	
	���⼸�к����ĳɣ�
	if (ssd->current_traceline < ssd->tracelines - 1) {
		time_t = ssd->ptr[ssd->current_traceline].time_t;
		device = ssd->ptr[ssd->current_traceline].device;
		lsn = ssd->ptr[ssd->current_traceline].lsn;
		size = ssd->ptr[ssd->current_traceline].size;
		ope = ssd->ptr[ssd->current_traceline].op;
		ssd->current_traceline++;
	} else {
		return 100;
	}

	��trace�ع�����(��2��)��
	ע�͵�:		fseek(ssd->tracefile,filepoint,0); 
	��Ϊ:		ssd->current_traceline--;

	����������������5�ж�ע�͵�(��������������һ��trace,��ȡʱ�䣬Ȼ��ع��ģ����ڲ���Ҫ��)��
	filepoint = ftell(ssd->tracefile);	
	fgets(buffer, 200, ssd->tracefile);    //Ѱ����һ������ĵ���ʱ��
	sscanf(buffer,"%I64u %d %d %d %d",&time_t,&device,&lsn,&size,&ope);
	ssd->next_request_time=time_t;
	fseek(ssd->tracefile,filepoint,0);

	�����һ�У�
		ssd->next_request_time = ssd->ptr[ssd->current_traceline].time_t;

	ע�͵�
		if(feof(ssd->tracefile)){
			request1 = NULL;
			return 100; 
		}

��6����ssd.c��simulate()�����У�ע�͵��������У�
	if((err=fopen_s(&(ssd->tracefile),ssd->tracefilename,"r"))!=0)
	{  
		printf("the trace file can't open\n");
		return NULL;
	}

	�ĳɣ�
		ssd->current_traceline = 0;		//ԭ���Ǵ��ļ�����ȡ��ɺ�رգ�trace������2�Σ�����ֻ����һ�Σ���Ҫ���¶�ȡ�ļ��Ļ�����һ������Ϳ�����
	
	Ȼ��ע�͵�����
		fclose(ssd->tracefile);

��7��pagemap.c��pre_process_page��������:
	���漸��ע�͵���
	if((err=fopen_s(&(ssd->tracefile),ssd->tracefilename,"r")) != 0 )      /*��trace�ļ����ж�ȡ����*/
	{
		printf("the trace file can't open\n");
		return NULL;
	}
	
	�޸����漸�У�
	while(fgets(buffer_request,200,ssd->tracefile))
	{
		sscanf_s(buffer_request,"%I64u %d %d %d %d",&time,&device,&lsn,&size,&ope);
	
	�ĳɣ�������trace�ļ���صĲ�������Ҫ�޸�һ�£���
	while(ssd->current_traceline < ssd->tracelines)
	{
		//sscanf_s(buffer_request,"%I64u %d %d %d %d",&time,&device,&lsn,&size,&ope);
		time = ssd->ptr[ssd->current_traceline].time_t;
		device = ssd->ptr[ssd->current_traceline].device;
		lsn = ssd->ptr[ssd->current_traceline].lsn;
		size = ssd->ptr[ssd->current_traceline].size;
		ope = ssd->ptr[ssd->current_traceline].op;
		ssd->current_traceline++;
	
	ע�͵�������fclose(ssd->tracefile);
	
Tips:֮ǰ������Ϊ���ļ���ϵͳ���ã������ʱ��ʵ����Ҳ�������trace.out�ļ�����Ҫ�Ļ������԰����Ҳ����
	ע�͵�
	
	