// htrdMinionProtector.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <time.h>
#include <io.h>
#include <Windows.h>
#include <crtdbg.h>

time_t systemtimeTotime_t(SYSTEMTIME st)
{
	tm t;
	t.tm_sec = st.wSecond;
	t.tm_min = st.wMinute;
	t.tm_hour = st.wHour;
	t.tm_mday = st.wDay;
	t.tm_mon = st.wMonth - 1;
	t.tm_year = st.wYear - 1900;
	t.tm_isdst = -1;

	return mktime(&t);
}

SYSTEMTIME getFileLastModifyedTime(const char *szAppName)
{
	WIN32_FIND_DATAA ffd;
	HANDLE hFind = FindFirstFileA(szAppName, &ffd);
	SYSTEMTIME st = { 0 };
	char szTimeBuf[64] = { 0 };
	FILETIME LocalFileTimel;
	if (FALSE == FileTimeToLocalFileTime(&ffd.ftLastWriteTime, &LocalFileTimel))
	{
		printf("error FileTimeToLocalFileTime");
		return st;
	}
	FileTimeToSystemTime(&LocalFileTimel, &st);
	
	return st;
}

bool file_exist(const char *szFileName)
{
	if (_access(szFileName, 0) != -1)
	{
		return true;
	}
	return false;
}

SYSTEMTIME gerCurrentTime()
{
	//char szTimeBuf[64] = { 0 };
	SYSTEMTIME st;
	GetLocalTime(&st);

	//sprintf_s(szTimeBuf, 64, "%4d-%02d-%02d %02d:%02d:%2d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

	return st;
}

bool run_cmd(char *cmd, char out[], size_t maxLen)
{
	memset(out, 0, maxLen);
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		return false;
	}
	char command[1024];    //长达1K的命令行，够用了吧  
	strcpy_s(command, 1024, "cmd.exe /C ");
	strcat_s(command, 1024, cmd);
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	si.cb = sizeof(STARTUPINFO);
	GetStartupInfoA(&si);
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;  
	if (!CreateProcessA(NULL, command, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		CloseHandle(hWrite);
		CloseHandle(hRead);
		return false;
	}
	CloseHandle(hWrite); 
	DWORD bytesRead;
	int nLen = 0;
	while (true)
	{
		if (ReadFile(hRead, out + nLen, maxLen - nLen, &bytesRead, NULL) == NULL)
			break;
		nLen += bytesRead;
	}
	CloseHandle(hRead);
	return true;
}

bool is_process_runing_alreaday(const char *szKey)
{
	HANDLE m_hMutex = CreateMutexA(NULL, FALSE, szKey);

	if (GetLastError() == ERROR_ALREADY_EXISTS) 
	{
		CloseHandle(m_hMutex);
		m_hMutex = NULL;

		return true;
	}
	return false;
}

BOOL CtrlHandler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
		/* Handle the CTRL-C signal. */
	case CTRL_C_EVENT:
		printf("CTRL_C_EVENT \n");
		break;
	case CTRL_CLOSE_EVENT:
		printf("CTRL_CLOSE_EVENT \n");
		break;
	case CTRL_BREAK_EVENT:
		printf("CTRL_BREAK_EVENT \n");
		break;
	case CTRL_LOGOFF_EVENT:
		printf("CTRL_LOGOFF_EVENT \n");
		break;
	case CTRL_SHUTDOWN_EVENT:
		printf("CTRL_SHUTDOWN_EVENT \n");
		break;
	default:
		return FALSE;
	}
	exit(0);
	return (TRUE);
}

int main(int argc, char* argv[])
{
	char szStopCommand[1024] = { 0 };
	char szStartCommand[1024] = { 0 };
	char szFileName[512] = { 0 };
	int timeout_to_restart = -1;

	char out[1024] = { 0 };

	if (argc < 5)
	{
		printf("useage<path><file name to check><command to stop server><command to start server><time out if file not modifyed>");
		return -1;
	}

	if (SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE) == FALSE)
	{
		printf("SetConsoleCtrlHandler failed!!!\n");
	}

	strcpy_s(szFileName, 512, argv[1]);
	strcpy_s(szStopCommand, 1024, argv[2]);
	strcpy_s(szStartCommand, 1024, argv[3]);
	timeout_to_restart = atoi(argv[4]);

	printf("to checked file name=%s,command to stop server=%s, command to start server=%s,time out to restart server=%d\n", 
		szFileName, szStopCommand, szStartCommand, timeout_to_restart);

	if (is_process_runing_alreaday(szStartCommand) == true)
	{
		printf("procsess already running,about to exit...\n");
		return -1;
	}

	while (1)
	{
		if (file_exist(szFileName) == false)
		{
			Sleep(30000);
			continue;
		}

		SYSTEMTIME file_modifyed_time = getFileLastModifyedTime(szFileName);
		SYSTEMTIME current_time = gerCurrentTime();

		time_t t1 = systemtimeTotime_t(file_modifyed_time);
		time_t t2 = systemtimeTotime_t(current_time);

		long diff = long(t2 - t1);
		//printf("%lld-%lld=%lld\n", t2, t1, t2 - t1);

		if (diff > timeout_to_restart)
		{
			run_cmd(szStopCommand, out, sizeof(out));
			printf("%s:%s\n", szStopCommand, out);
			Sleep(1);
			run_cmd(szStartCommand, out, sizeof(out));
			printf("%s:%s\n", szStartCommand, out);
			Sleep(60000);
		}
		else
		{
			printf("process is ok!(%s modifyed at %ld)\n", szFileName, t1);
		}
		Sleep(1000);
	}
	_CrtDumpMemoryLeaks();
	return 0;
}

