#include "SingleEplServer.h"

SingleEplServer::SingleEplServer()
{
	m_argc = 0;
	memset(m_argv_0,0,EXE_FILE_NAME_LENGTH);
	memset(m_serverPort,0,8);

	m_brokerPort = 10001;
	memset(m_brokerIP,0,16);
}

SingleEplServer::~SingleEplServer()
{

}

/************************************************************************/
/* print info about SingleEplServer                                                                     */
/************************************************************************/

void SingleEplServer::description()
{
	printf("\nSingleEplServer is implemented by epoll which is only used in Linux 2.6.\n\n");
}

void SingleEplServer::run()
{
#ifdef WIN32
	printf("You are using Windows system. We suggest that use SingleEplServer in Linux system.");
	return;
#else
	int sfd, s;
	int efd;
	struct epoll_event event;
	struct epoll_event *events;

	if(m_argc != 2)
    {
		fprintf(stderr, "Usage: %s [port]\n", m_argv_0);
		exit(EXIT_FAILURE);
    }

	sfd = createAndBind(m_serverPort);//the port
	if(sfd == -1)
	{
		abort ();
	}

	s = makeSocketNonBlocking(sfd);
	if(s == -1)
	{
		abort();
	}
	s = listen(sfd, SOMAXCONN);
	if (s == -1)
	{
		perror("listen");
		abort();
    }

	efd = epoll_create1(0);
	if(efd == -1)
	{
		perror("epoll_create");
		abort();
    }
	event.data.fd = sfd;
	event.events = EPOLLIN | EPOLLET;
	s = epoll_ctl(efd, EPOLL_CTL_ADD, sfd, &event);
	if(s == -1)
	{
		perror("epoll_ctl");
		abort();
    }

	/* Buffer where events are returned */
	events = (struct epoll_event *)calloc(MAXEVENTS, sizeof event);
	
	/* The event loop */
	while(1)
	{
		int n, i;
		n = epoll_wait(efd, events, MAXEVENTS, -1);
		for (i = 0; i < n; i++)
		{
			if ((events[i].events & EPOLLERR) ||
				(events[i].events & EPOLLHUP) ||
				(!(events[i].events & EPOLLIN)))
			{
				/* An error has occured on this fd, or the socket is not
				ready for reading (why were we notified then?) */
				fprintf(stderr, "epoll error\n");
				close(events[i].data.fd);
				continue;
			}
			else if(sfd == events[i].data.fd)
			{
				/* We have a notification on the listening socket, which
				means one or more incoming connections. */
				while(1)
				{
					struct sockaddr in_addr;
					socklen_t in_len;
					int infd;
					char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
					in_len = sizeof in_addr;
					infd = accept(sfd, &in_addr, &in_len);
					if(infd == -1)
					{
						if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
						{
							/* We have processed all incoming
							connections. */
							break;
						}
						else
						{
							perror("accept");
							break;
						}
					}
					s = getnameinfo(&in_addr, in_len,hbuf, sizeof hbuf,sbuf, sizeof sbuf,NI_NUMERICHOST | NI_NUMERICSERV);
					if (s == 0)
					{
						printf("Accepted connection on descriptor %d "
                             "(host=%s, port=%s)\n", infd, hbuf, sbuf);
					}
					/* Make the incoming socket non-blocking and add it to the
					list of fds to monitor. */
					s = makeSocketNonBlocking(infd);
					if (s == -1)
					{
						abort();
					}
					event.data.fd = infd;
					event.events = EPOLLIN | EPOLLET;
					s = epoll_ctl(efd, EPOLL_CTL_ADD, infd, &event);
					if (s == -1)
					{
						perror("epoll_ctl");
						abort();
					}
				}
				continue;
			}
			else
			{
				/* We have data on the fd waiting to be read. Read and
				display it. We must read whatever data is available
				completely, as we are running in edge-triggered mode
				and won't get a notification again for the same
				data. */
				int done = 0;
				while (1)
				{
					ssize_t count;
					char buf[512];
					count = read(events[i].data.fd, buf, sizeof buf);
					if (count == -1)
					{
						/* If errno == EAGAIN, that means we have read all
						data. So go back to the main loop. */
						if (errno != EAGAIN)
						{
							perror ("read");
							done = 1;
						}
						break;
					}
					else if(count == 0)
					{
						/* End of file. The remote has closed the
						connection. */
						done = 1;
						break;
					}
					/* Write the buffer to standard output */
					s = write(1, buf, count);
					if (s == -1)
					{
						perror("write");
						abort();
					}
				}
				if(done)
				{
					printf ("Closed connection on descriptor %d\n",events[i].data.fd);
		   		 
					/* Closing the descriptor will make epoll remove it
					from the set of descriptors which are monitored. */
					close (events[i].data.fd);
				}
			}
		}
	}

	free (events);
	close (sfd);
#endif
	
	return;
}

int SingleEplServer::createAndBind(char *port)
{
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	int s, sfd;

	memset (&hints, 0, sizeof (struct addrinfo));
	hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
	hints.ai_socktype = SOCK_STREAM; /* We want a TCP socket */
	hints.ai_flags = AI_PASSIVE;     /* All interfaces */

	s = getaddrinfo (NULL, port, &hints, &result);
	if (s != 0)
	{
		fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
		return -1;
	}

	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sfd = socket (rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		int opt = 1;  
		setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&opt, sizeof(opt));
		if (sfd == -1)
			continue;

		s = bind (sfd, rp->ai_addr, rp->ai_addrlen);
		if (s == 0)
		{
			/* We managed to bind successfully! */
			break;
		}
		else
		{
			printf("s is %d\n",s);
		}

#ifdef WIN32
		closesocket(sfd);
#else
		close (sfd);
#endif

	}

	if (rp == NULL)
	{
		fprintf (stderr, "Could not bind\n");
		return -1;
	}

	freeaddrinfo (result);

	return sfd;
}

int SingleEplServer::makeSocketNonBlocking(int sfd)
{
	int flags, s;

#ifdef WIN32
	printf("You are using Windows system. We suggest that use SingleEplServer in Linux system.");
	(void)flags;
	ULONG NonBlock = 1;
	if((s=ioctlsocket(sfd, FIONBIO, &NonBlock)) == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError()); 
		return -1;
	}

	
#else
	flags = fcntl (sfd, F_GETFL, 0);
	if (flags == -1)
	{
		perror ("fcntl");
		return -1;
	}

	flags |= O_NONBLOCK;
	s = fcntl (sfd, F_SETFL, flags);
	if (s == -1)
	{
		perror ("fcntl");
		return -1;
	}

#endif
	
	return 0;
}

void SingleEplServer::setArgc(int agc)
{
	m_argc = agc;
}

int SingleEplServer::getArgc()
{
	return m_argc;
}

void SingleEplServer::setArgv_0(char *argv_0)
{
	if (argv_0 == NULL)
	{
		return;
	}
	memcpy(m_argv_0,argv_0,sizeof(argv_0));
}
char* SingleEplServer::getArgv_0()
{
	return m_argv_0;
}

void SingleEplServer::setServerPort(char *port)
{
	if (port == NULL)
	{
		return;
	}
	memcpy(m_serverPort,port,sizeof(port));
}
char* SingleEplServer::getServerPort()
{
	return m_serverPort;
}

void SingleEplServer::setBrokerPort(unsigned int port)
{
	m_brokerPort = port;
}
unsigned int SingleEplServer::getBrokerPort()
{
	return m_brokerPort;
}

void SingleEplServer::setBrokerIP(char *ip)
{
	if (ip == NULL)
	{
		return;
	}

	memcpy(m_brokerIP,ip,16);
}
char* SingleEplServer::getBrokerIp()
{
	return m_brokerIP;
}

void SingleEplServer::setBrokerTopic(char *topic)
{
	if (topic == NULL)
	{
		return;
	}
	memcpy(m_topic,topic,sizeof(topic));
}
char* SingleEplServer::getBrokerTopic()
{
	return m_topic;
}
