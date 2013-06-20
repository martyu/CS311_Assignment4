//
//  main.cpp
//  compute
//
//  Created by Marty Ulrich on 3/11/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sstream>
#include <istream>

using namespace std;

#define PORT_NUM "50001"

typedef int sock_fd;

unsigned long long upper_limit;
unsigned long long lower_limit;
string server_name;
sock_fd sock;
vector<string> perfect_numbers;


int check_args(const char * args[], int num_args);
unsigned long long check_timing();


static void handler(int sig)
{
    if(sig == SIGINT)
	{
		cerr << "SIGINT received\n";
		close(sock);
		exit(-1);
	}
	else if(sig == SIGHUP)
	{
		cerr << "SIGHUP received\n";
		close(sock);
		exit(-1);
	}
	else if(sig == SIGKILL)
	{
		cerr << "SIGKILL received\n";
		close(sock);
		exit(-1);
	}
}

static void * threadFunc(void *arg)
{
	//Added a year later to try to send message from cin to server
	char input;
	
	cin >> input;
	if(write(sock, &input, sizeof(char)) == -1)
	{
		cout << "error: " << strerror(errno);
	}


/*
	pthread_detach(pthread_self());
	char buffer;
	while(true)
	{
		if(read(sock, &buffer, sizeof(buffer)) != -1)
		{
			if(buffer == 'k')
			{
				kill(getpid(), SIGKILL);
				return NULL;
			}
			sleep(1);
		}
	}
 */
}

int main (int argc, const char * argv[])
{
	unsigned long long running_total;
	struct addrinfo hints;
    struct addrinfo *result, *rp;
	char iters_in_three_sec_str[20];
	char range[2][20];
	time_t start;
	pthread_t thread_monitor_for_signal;
	struct sigaction sig_act;
	perfect_numbers = vector<string>();
	char buff;
	int index = 0;
	int errNo;
	
	if(check_args(argv, argc) == -1)
		return -1;
	
	sprintf(iters_in_three_sec_str, "%llu\n", check_timing());
	
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if( (errNo = getaddrinfo(server_name.c_str(), PORT_NUM, &hints, &result)) != 0)
	{
		cerr << "getaddrinfo error: " << gai_strerror(errNo) << "\n";
		return -1;
	}
	
	sigemptyset(&sig_act.sa_mask);
	sig_act.sa_flags = 0;
	sig_act.sa_handler = handler;
	
	if(sigaction(SIGINT, &sig_act, NULL) == -1)
	{
		cerr << "sigaction error: " << strerror(errno);
		return -1;
	}
	if(sigaction(SIGHUP, &sig_act, NULL) == -1)
	{
		cerr << "sigaction error: " << strerror(errno);
		return -1;
	}
	if(sigaction(SIGQUIT, &sig_act, NULL) == -1)
	{
		cerr << "sigaction error: " << strerror(errno);
		return -1;
	}
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR)
	{
		cerr << "sigaction error: " << strerror(errno);
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol); 
		if (sock != -1)
		{
			if (connect(sock, rp->ai_addr, rp->ai_addrlen) != -1)
				break;
			close(sock);
		}
	}
	freeaddrinfo(result);
	
	if (rp == NULL)
	{
		cerr << "\ncouldn't connect socket\n";
		return -1;
	}
	cout << "connected to socket\n";
		
	if(write(sock, iters_in_three_sec_str, sizeof(iters_in_three_sec_str)) == -1)
	{
		cerr << "\nwrite failed: " << strerror(errno) << "\n";
		return -1;
	}

	while(true)
	{
		if(read(sock, &buff, 1) == -1)
		{
			cerr << "read error: " << strerror(errno) << "\n";
			return -1;
		}
		if(buff == 'k')
		{
			close(sock);
			kill(getpid(), SIGKILL);
		}
		else if(buff == '\n')
		{
			break;
		}
		else
		{
			range[0][index++] = buff;
		}
	}
	index = 0;
	while(true)
	{
		if(read(sock, &buff, 1) == -1)
		{
			cerr << "read error: " << strerror(errno) << "\n";
			return -1;
		}
		if(buff == 'k')
		{
			close(sock);
			kill(getpid(), SIGKILL);
		}
		else if(buff == '\n')
		{
			break;
		}
		else
		{
			range[1][index++] = buff;
		}
	}
	
	pthread_create(&thread_monitor_for_signal, NULL, threadFunc, NULL);
	
	start = clock();

	lower_limit = atoll(range[0]);
	upper_limit = atoll(range[1]);
	
	cout << "range: " << lower_limit << " - " << upper_limit << "\n";
	
	for(unsigned long long i = lower_limit; i < upper_limit; i++) //find perfect numbers
	{
		if(i % 1000 == 0)
		{
			cout << " at " << i << "\n";
		}
		running_total = 0;
		for(unsigned long long j = i-1; j > 0; j--)
		{
			if(i % j == 0)
			{
				running_total += j;
				if(running_total > i)
					break;
			}
		}		
		if(running_total == i)
		{
			stringstream ss;
			ss << i << "\n";
			perfect_numbers.push_back(ss.str());
		}
	}
	
	cout << "that took " << (double)(clock() - start) / (double)CLOCKS_PER_SEC << " seconds\n";
		
	for(unsigned long long i; i < perfect_numbers.size(); i++)
	{		
		if(write(sock, perfect_numbers[i].c_str(), perfect_numbers[i].size()) == -1)
		{
			cerr << "\nwrite failed: " << strerror(errno) << "\n";
			return -1;
		}
	}
			
	if(close(sock) == -1)
	{
		cerr << "close sock error: " << strerror(errno) << "\n";
		return -1;
	}
	
	cout << "exiting...\n";
	return 1;
	
}
		   
int check_args(const char * args[], int num_args)
{
	if(num_args < 2)
	{
		printf("\tcompute, version 1.0\n\tusage: compute server_name\n");
		return -1;
	}
	for(int i = 1; i < num_args; i++)
	{
		if(strcmp(args[i], "--help") == 0 || strcmp(args[i], "-h") == 0)
		{
			printf("\tcompute, version 1.0\n\tusage: compute server_name\n");
			return -1;
		}
	}
	
	server_name = args[1];
	
	return 0;
}


unsigned long long check_timing()
{
	time_t time_elapsed = 0;
	time_t start = clock();
	unsigned long long running_total;
	unsigned long long number;
	unsigned long long iterations = 0;
	unsigned long long i;
	
	for(i = 1; time_elapsed < 3; i++, time_elapsed = (double)(clock() - start) / (double)CLOCKS_PER_SEC)
	{
		iterations++;
		if(i % 1000 == 0)
		{
			cout << "time elapsed: " << time_elapsed << "\n";
		}
		running_total = 0;
		for(unsigned long long j = i-1; j > 0; j--)
		{
			if(number % j == 0)
			{
				running_total += j;
				if(running_total > number)
					break;
			}
		}
	}
	
	return iterations;
}
