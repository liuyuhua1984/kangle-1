/*
 * Copyright (c) 2010, NanChang BangTeng Inc
 *
 * kangle web server              http://www.kanglesoft.com/
 * ---------------------------------------------------------------------
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *  See COPYING file for detail.
 *
 *  Author: KangHongjiu <keengo99@gmail.com>  2011-07-18
 */
#ifndef KCONFIG_H
#define KCONFIG_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef _WIN32
#include <unistd.h>
#endif
#include <vector>
#include <list>
#include <string>
#include <map>
#include "KSocket.h"
#include "global.h"
#include "KMutex.h"
#include "KTimeMatch.h"
#include "malloc_debug.h"
#include "KUserManageInterface.h"
#include "server.h"
#include "KAsyncWorker.h"
#ifdef KSOCKET_SSL
#include "KSSLSocket.h"
#endif
#define AUTOUPDATE_OFF    0
#define AUTOUPDATE_ON   1
#define AUTOUPDATE_DOWN   2

#define REFRESH_AUTO	0
#define	REFRESH_ANY		1	
#define REFRESH_NEVER	2

#define USE_PROXY	1
#define USE_LOG		(1<<1)
#define IGNORE_CASE	(1<<2)
#define NO_FILTER	(1<<2)
#define SAFE_STRCPY(s,s1)   do {memset(s,0,sizeof(s));strncpy(s,s1,sizeof(s)-1);}while(0)
class KAcserverManager;
class KVirtualHostManage;
class KVirtualHost;
struct KPerIpConnect;
class KListenHost {
public:
	KListenHost()
	{
		ext = cur_config_ext;
		event_driven = false;
	}
	std::string name;
	std::string ip;
	int port;
	int model;
	bool ext;
	bool event_driven;
#ifdef KSOCKET_SSL
	bool sni;
	std::string certificate;
	std::string certificate_key;
#endif
};
struct WorkerProcess
{
#ifdef _WIN32
#ifdef ENABLE_DETECT_WORKER_LOCK
	HANDLE active_event;
#endif
	HANDLE shutdown_event;
	HANDLE notice_event;
	HANDLE hProcess;
	int pid;
	int pending_count;
#else
	pid_t pid;
#endif
	bool closed;
	int worker_index;
};
class KConfigBase 
{
public:
	KConfigBase()
	{
		memset(this,0,sizeof(KConfigBase));
	}
	unsigned int worker;
	unsigned int max; //����߳�
	unsigned int max_per_ip; //ÿ��IP���������
	unsigned int per_ip_deny;
	unsigned min_free_thread;
	unsigned time_out;
	unsigned keep_alive;
	char hostname[32];
	int log_level;
	INT64 log_rotate_size;
	INT64 error_rotate_size;
	//Ĭ���Ƿ񻺴�,1=��,����=��
	int default_cache;
	unsigned max_cache_size;
	INT64 mem_cache;
#ifdef ENABLE_DISK_CACHE
	INT64 disk_cache;
#ifdef ENABLE_BIG_OBJECT
	INT64 max_bigobj_size;
#endif
#endif
	int select_count;
	int refresh;
	int refresh_time;
	
	//ֻѹ������cache�����
	int only_gzip_cache;
	//��Сѹ������
	unsigned min_gzip_length;
	//ѹ������(1-9)
	int gzip_level;
	//bool x_forwarded_for;
	//bool x_real_ip;
	//bool insert_via;
	//bool x_cache;
	bool error_ip;
	bool path_info;
	//�Ƿ��첽io
	bool async_io;
	int worker_io;
	int worker_dns;
	int auth_type;
	int passwd_crypt;
	//int configVersion;
	int anti_fatboy;
	//������ʱ��
	int wl_time;
#ifdef ENABLE_ADPP
	int process_cpu_usage;
#endif
#ifdef MALLOCDEBUG
	bool mallocdebug;
#endif
	unsigned maxLogHandle;
	unsigned logs_day;
	bool log_sub_request;
	bool log_handle;
	INT64 logs_size;
	int autoupdate;
	int autoupdate_time;
	unsigned buffer;
#ifdef ENABLE_TF_EXCHANGE
	int tmpfile;
	INT64 max_post_size;
#endif
#ifdef KSOCKET_UNIX	
	bool unix_socket;
#endif
	int errorTryCount;
	bool removeAcceptEncoding;
#ifdef ENABLE_VH_FLOW
	//�Զ�ˢ������ʱ��(��)
	int flush_flow_time;
#endif
	char lang[8];
	char disk_cache_dir2[512];
	/////////[314]
	char log_rotate[32];
	char access_log[128];
	char logHandle[512];
	char cookie_stick_name[16];
	char server_software[32];
	char disk_work_time[32];
	/////////[315]
};
class KConfig : public KConfigBase
{
public:
	KConfig()
	{
		memset(disk_cache_dir,0,sizeof(disk_cache_dir));
		per_ip_head = NULL;
		per_ip_last = NULL;
	}
	~KConfig();
	KAcserverManager *am;
	KVirtualHostManage *vm;
	std::string admin_passwd;
	std::string admin_user;
	std::vector<std::string> admin_ips;
	std::vector<KListenHost *> service;	
	//run_user,run_groupΪһ����ʹ�ã����԰�ȫ��string
	std::string run_user;
	std::string run_group;
	//disk_cache_dir2�ǿ����޸ĵġ����������õġ�
	//ʵ���õ�ʱ����disk_cahce_dir.
	char disk_cache_dir[512];
	KPerIpConnect *per_ip_head;
	KPerIpConnect *per_ip_last;
	/////////[316]
	void copy(KConfig *c);
	void set_time_out(unsigned val)
	{
		time_out = val;
		if(time_out>0 && time_out < 5){
			//��С��ʱΪ5��
			time_out = 5;
		}
	}
	void setErrorTryCount(int errorTryCount)
	{
		if(errorTryCount<0) {
			errorTryCount = 0;
		} else if(errorTryCount>5) {
			errorTryCount = 5;
		}
		this->errorTryCount = errorTryCount;
	}
	void setHostname(const char *hostname) {
		SAFE_STRCPY(this->hostname,hostname);
	}
};
//���ò����ͳ���������Ϣ
class KGlobalConfig : public KConfig {
public:
	KGlobalConfig();
	//////////////////////////////////////////////////////
	KMutex admin_lock;
	/////////////////////////////////////////////////////////
	//���������õı�������ÿ�������ļ����ģ���Ҫ��������
	KTimeMatch diskWorkTime;
	KTimeMatch autoupdate_install_time;
	void set_autoupdate_time(int autoupdate_time);
	/////////////////////////////////////////////////////////
	//�����ǳ���������Ϣ,ֻ�ڳ�������ǰ��ʼ��һ��...
	std::list<std::string> mergeFiles;
	std::string path;
	std::string tmppath;
	std::string program;
	std::string extworker;
	KAsyncWorker *ioWorker;
	KAsyncWorker *dnsWorker;
	int serverNameLength;
	char serverName[32];
#ifdef _WIN32
		//kangle�������ڵ��̷�
	std::string diskName;
#endif
	/////////[317]
	KAcserverManager *gam;
	KVirtualHostManage *gvm;
	//3311��������������
	KVirtualHost *sysHost;
};
extern KGlobalConfig conf;
extern int m_debug;
extern bool need_reboot_flag;
extern KConfig *cconf;
int merge_apache_config(const char *file);
void LoadDefaultConfig();
void do_config(bool firstTime=true);
//��ȡ�����ļ���������
void load_config(KConfig *cconf,bool firstTime);
//���������ļ�,������load_config֮�󣬶�һЩ��Ŀ������������ʵʩ
void parse_config(bool firstTime);
//��������ļ��������ڴ�й©���ʱ���ŵ���
void clean_config();

bool saveConfig();
INT64 get_size(const char *size);
std::string get_size(INT64 size);
#define CONFIG_FILE_SIGN  "<!--configfileisok-->"
#ifndef CONFIG_FILE
#ifndef _WIN32
#define CONFIG_FILE "/etc/config.xml"
#else
#define CONFIG_FILE "\\etc\\config.xml"
#endif
#endif
#endif