/* A threaded server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define BUFFERLENGTH 3

int toBeUsedIndex = -1; /* buffer initially empty */
int freeIndex = 0;
char *globalBuffer[BUFFERLENGTH];

/* displays error messages from system calls */
void error(char *msg) {
    perror(msg);
    exit(1);
};

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER; /* the lock used for processing */

int writeResult (int sockfd, char *buffer, size_t bufsize) {
    int n;
   
    n = write(sockfd, &bufsize, sizeof(size_t));
    if (n < 0) {
		fprintf (stderr, "ERROR writing to result\n");
		return -1;
    }
    
    n = write(sockfd, buffer, bufsize);
    if (n != bufsize) {
		fprintf (stderr, "Couldn't write %ld bytes, wrote %d bytes\n", bufsize, n);
		return -1;
    }
    return 0;
}

char *readRes(int sockfd) {
    size_t bufsize;
    int res;
    char *buffer;

    res = read(sockfd, &bufsize, sizeof(size_t));
    if (res != sizeof(size_t)) {
		fprintf (stderr, "Reading number of bytes from socket failed\n");
		return NULL;
    }

    buffer = malloc(bufsize+1);
    if (buffer) {
		buffer[bufsize]  = '\0';
		res = read(sockfd, buffer, bufsize);
		if (res != bufsize) {
			fprintf (stderr, "Reading reply from socket\n");
			free(buffer);
			return NULL;
		}
    }
    return buffer;
}    

char *addEntry(char *entry) {
    char *result;

    pthread_mutex_lock (&mut); /* lock access */
    if (freeIndex == -1) {
	/* buffer full */
	pthread_mutex_unlock (&mut); /* unlock access */
	free(entry);
	result = strdup("Buffer full");
    }
    else {
	globalBuffer[freeIndex] = entry;
	if (toBeUsedIndex == -1) {
	    toBeUsedIndex = freeIndex; /* buffer no longer empty */
	}
	freeIndex = (freeIndex + 1) % BUFFERLENGTH;
	if (freeIndex == toBeUsedIndex) {
	    freeIndex = -1; /* buffer full */
	}
	pthread_mutex_unlock (&mut); /* unlock access */
	result = strdup("Entry added");
    }
    return result;
}

char *removeEntry() {
    char *result;
    pthread_mutex_lock (&mut); /* lock access */
    if (toBeUsedIndex == -1) {
	/* buffer empty */
	pthread_mutex_unlock (&mut); /* unlock access */
	result = strdup("Buffer empty");
    }
    else {
	result = globalBuffer[toBeUsedIndex];
	if (freeIndex == -1) {
	    freeIndex = toBeUsedIndex; /* have space now */
	}
	toBeUsedIndex = (toBeUsedIndex + 1) % BUFFERLENGTH;
	if (freeIndex == toBeUsedIndex) {
	    toBeUsedIndex = -1; /* buffer now empty */
	}
	pthread_mutex_unlock (&mut); /* unlock access */
    }
    return result;
}

/* For each connection, this function is called in a separate thread. */
void *processRequest (void *args) {
    int *newsockfd =  (int *) args;   
    char *result;
    char *buffer;

    buffer = readRes (*newsockfd);
    if (!buffer)  {
		fprintf (stderr, "ERROR reading from socket\n");
    }
    else {
	if (strcmp(buffer, "R\n") == 0) {
	    result = removeEntry();
	    free(buffer);
	}
	else
	    result = addEntry(buffer);
	writeResult(*newsockfd, result, strlen(result) +1);
	free(result);
    }

    close(*newsockfd); /* important to avoid memory leak */
    free (newsockfd);
	  
    pthread_exit (NULL); /*exit value not used */
}

int main(int argc, char *argv[]) {
    int sockfd, portno;
    struct sockaddr_in6 serv_addr;
    int result;

    if (argc < 2) {
		fprintf (stderr,"ERROR, no port provided\n");
		exit(1);
    }
	     
    /* create socket */
    sockfd = socket (AF_INET6, SOCK_STREAM, 0);
    if (sockfd < 0) 
	error("ERROR opening socket");
    bzero ((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_port = htons (portno);

    /* bind it */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR on binding");
	}

    /* ready to accept connections */
    listen (sockfd,5);

    /* now wait in an endless loop for connections and process them */
    while(1) {
		pthread_t server_thread; /* thread information */
		pthread_attr_t pthread_attr; /* attributes for newly created thread */
		int *newsockfd;
		struct sockaddr_in6 cli_addr;
		socklen_t clilen;

		clilen = sizeof(cli_addr);
		newsockfd  = malloc(sizeof (int));
		if (!newsockfd) {
			fprintf (stderr, "Memory allocation failed!\n");
			exit(1);
		}
		
		/* waiting for connections */
		*newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (*newsockfd < 0) {
			error ("ERROR on accept");
		}

		/* create thread for processing of connection */
		if (pthread_attr_init (&pthread_attr)) {
			fprintf (stderr, "Creating initial thread attributes failed!\n");
			exit (1);
		}

		if (pthread_attr_setdetachstate (&pthread_attr, PTHREAD_CREATE_DETACHED)) {
			fprintf (stderr, "setting thread attributes failed!\n");
			exit (1);
		}
			
		result = pthread_create (&server_thread, &pthread_attr, processRequest, (void *) newsockfd);
		if (result != 0) {
			fprintf (stderr, "Thread creation failed!\n");
			exit (1);
		}
    }
}





