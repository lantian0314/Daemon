/*
 * File        : daemon.c
 * Author      : Vincent Cheung
 * Date        : Jan. 20, 2015
 * Description : This is used as process daemon.
 *
 * Copyright (C) Vincent Chueng<coolingfall@gmail.com>
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/system_properties.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include "common.h"

#define	MAXFILE         3
#define SLEEP_INTERVAL  10
#define BUFFER_SIZE 2048

volatile int sig_running = 1;

/* signal term handler */
static void sigterm_handler(int signo)
{
	sig_running = 0;
}

/* start daemon service */
static void start_service(char *package_name, char *service_name)
{
	/* get the sdk version */
	int version = get_version();

	pid_t pid;

	if ((pid = fork()) < 0)
	{
		exit(EXIT_SUCCESS);
	}
	else if (pid == 0)
	{
		if (package_name == NULL || service_name == NULL)
		{
			return;
		}

		char *p_name = str_stitching(package_name, "/");
		char *s_name = str_stitching(p_name, service_name);

		if (version >= 17 || version == 0)
		{
			int ret = execlp("am", "am", "startservice",
						"--user", "0", "-n", s_name, (char *) NULL);
		}
		else
		{
			execlp("am", "am", "startservice", "-n", s_name, (char *) NULL);
		}

		exit(EXIT_SUCCESS);
	}
	else
	{
		waitpid(pid, NULL, 0);
	}
}

int main(int argc, char *argv[])
{
	int i;
	pid_t pid;
	char *package_name = NULL;
	char *service_name = NULL;
	char *daemon_file_dir = NULL;
	int interval = SLEEP_INTERVAL;

	if (argc < 7)
	{
		return;
	}

	for (i = 0; i < argc; i ++)
	{
		if (!strcmp("-p", argv[i]))
		{
			package_name = argv[i + 1];
		}

		if (!strcmp("-s", argv[i]))
		{
			service_name = argv[i + 1];
		}

		if (!strcmp("-t", argv[i]))
		{
			interval = atoi(argv[i + 1]);
		}
	}

	/* package name and service name should not be null */
	if (package_name == NULL || service_name == NULL)
	{
		return;
	}

	if ((pid = fork()) < 0)
	{
		exit(EXIT_SUCCESS);
	}
	else if (pid == 0)
	{
		/* add signal */
		signal(SIGTERM, sigterm_handler);

		/* become session leader */
		setsid();
		/* change work directory */
		chdir("/");

		for (i = 0; i < MAXFILE; i ++)
		{
			close(i);
		}

		/* find pid by name and kill them */
		int pid_list[100];
		int total_num = find_pid_by_name(argv[0], pid_list);
		for (i = 0; i < total_num; i ++)
		{
			int retval = 0;
			int daemon_pid = pid_list[i];
			if (daemon_pid > 1 && daemon_pid != getpid())
			{
				retval = kill(daemon_pid, SIGTERM);
				if (!retval)
				{
				}
				else
				{
					exit(EXIT_SUCCESS);
				}
			}
		}

		while(sig_running)
		{
			interval = interval < SLEEP_INTERVAL ? SLEEP_INTERVAL : interval;
			select_sleep(interval, 0);


			/* start service */
			start_service(package_name, service_name);
		}

		exit(EXIT_SUCCESS);
	}
	else
	{
		/* parent process */
		exit(EXIT_SUCCESS);
	}
}
char *str_stitching(const char *str1, const char *str2)
{
	char *result;
	result = (char *) malloc(strlen(str1) + strlen(str2) + 1);
	if (!result)
	{
		return NULL;
	}

	strcpy(result, str1);
	strcat(result, str2);

    return result;
}

/* open browser with specified url */
void open_browser(char *url)
{
	/* the url cannot be null */
	if (url == NULL || strlen(url) < 4) {
		return;
	}

	/* get the sdk version */
	char value[8] = "";
	__system_property_get("ro.build.version.sdk", value);

	int version = atoi(value);
	/* is the version is greater than 17 */
	if (version >= 17 || version == 0)
	{
		execlp("am", "am", "start", "--user", "0", "-n",
				"com.android.browser/com.android.browser.BrowserActivity",
				"-a", "android.intent.action.VIEW",
				"-d", url, (char *)NULL);
	}
	else
	{
		execlp("am", "am", "start", "-n",
				"com.android.browser/com.android.browser.BrowserActivity",
				"-a", "android.intent.action.VIEW",
				"-d", url, (char *)NULL);
	}
}

/**
 * Get the version of current SDK.
 */
int get_version()
{
	char value[8] = "";
    __system_property_get("ro.build.version.sdk", value);

    return atoi(value);
}

/**
 * Find pid by process name.
 */
int find_pid_by_name(char *pid_name, int *pid_list)
{
	DIR *dir;
	struct dirent *next;
	int i = 0;
	pid_list[0] = 0;

	dir = opendir("/proc");
	if (!dir)
	{
		return 0;
	}

	while ((next = readdir(dir)) != NULL)
	{
		FILE *status;
		char proc_file_name[BUFFER_SIZE];
		char buffer[BUFFER_SIZE];
		char process_name[BUFFER_SIZE];

		/* skip ".." */
		if (strcmp(next->d_name, "..") == 0)
		{
			continue;
		}

		/* pid dir in proc is number */
		if (!isdigit(*next->d_name))
		{
			continue;
		}

		sprintf(proc_file_name, "/proc/%s/cmdline", next->d_name);
		if (!(status = fopen(proc_file_name, "r")))
		{
			continue;
		}

		if (fgets(buffer, BUFFER_SIZE - 1, status) == NULL)
		{
			fclose(status);
			continue;
		}
		fclose(status);

		/* get pid list */
		sscanf(buffer, "%[^-]", process_name);
		if (strcmp(process_name, pid_name) == 0)
		{
			pid_list[i ++] = atoi(next->d_name);
		}
	}

	if (pid_list)
    {
    	pid_list[i] = 0;
    }

    closedir(dir);

    return i;
}

/**
 * Get the process name according to pid.
 */
char *get_name_by_pid(pid_t pid)
{
	char proc_file_path[BUFFER_SIZE];
	char buffer[BUFFER_SIZE];
	char *process_name;

    process_name = (char *) malloc(BUFFER_SIZE);
    if (!process_name)
    {
    	return NULL;
    }

	sprintf(proc_file_path, "/proc/%d/cmdline", pid);
	FILE *fp = fopen(proc_file_path, "r");
	if (fp != NULL)
	{
		if (fgets(buffer, BUFFER_SIZE - 1, fp) != NULL)
		{
			fclose(fp);

			sscanf(buffer, "%[^-]", process_name);
			return process_name;
		}
	}

	return NULL;
}


/**
 * Use `select` to sleep with specidied second and microsecond.
 */
void select_sleep(long sec, long msec)
{
	struct timeval timeout;

	timeout.tv_sec = sec;
	timeout.tv_usec = msec * 1000;

	select(0, NULL, NULL, NULL, &timeout);
}