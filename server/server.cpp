#include "clthread.cpp"

void *get_in_addr( struct sockaddr *sa)
{
	if( sa->sa_family == AF_INET )
	{
		return &((( struct sockaddr_in*)sa)->sin_addr );
	}

	return &((( struct sockaddr_in6*)sa)->sin6_addr );
}

int main( int argc, char * argv[] )
{
	//Error check for correct arguments
	if( argc != 4 )
	{
		fprintf( stderr, "Usage: ./server.o seed width height\n" );
		exit( 1 );
	}

	//file descripter for accepting new connections
	int listener;
	//Store address for ipv4/6
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;

	char remoteIP[INET6_ADDRSTRLEN];

	int yes = 1;
	int rv;

	//Shit for address creation
	struct addrinfo hints, *ai, *p;
	
	//Prepare hints
	memset( &hints, 0, sizeof hints );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	//Get the addres info, stick it in ai
	if(( rv = getaddrinfo( NULL, PORT, &hints, &ai )) != 0 )
	{
		fprintf( stderr, "selectserver: %s\n", gai_strerror( rv ) );
		exit( 2 );
	}
	
	//Bind to the first available connection
	for( p = ai; p != NULL; p = p->ai_next )
	{
		listener = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
		if( listener < 0 )
		{
			continue;
		}

		setsockopt( listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) );

		if( bind(listener, p->ai_addr, p->ai_addrlen ) < 0 )
		{
			close( listener );
			continue;
		}
		break;
	}
	if( p == NULL )
	{
		fprintf( stderr, "selectserver: failed to bind\n");
		exit( 3 );

	}
	
	//Free the now not needed address info
	freeaddrinfo( ai );

	//Listen on the new port we will use for accepting connections
	if( listen( listener, 10 ) == -1 )
	{
		perror( "listen" );
		exit( 4 );
	}
	
	//Initialize gamestate
	clthread::initialize_threads( atoi( argv[1] ), atoi(argv[2] ), atoi( argv[3] ) );

	//Accept connections
	for( ;; )
	{
		int newfd;
		addrlen = sizeof( remoteaddr );
		newfd = accept( listener, (struct sockaddr*)&remoteaddr, &addrlen );
		clthread::thread_param input;
		input.sockfd = newfd;
		pthread_create( &clthread::clients[clthread::thread_count++], NULL, &clthread::beginthread, &input );
	}

	return 0;
}
