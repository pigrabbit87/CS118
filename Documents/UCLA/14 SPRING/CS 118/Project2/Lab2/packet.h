
/* Packet contains:
-packetType:
	0: request from client
	1: ACK 
	2: data
	3: FIN-ACK
	4: Error in request
*/

#define PACKETSIZE 1024 //bytes

struct packet{
	int packetType;
	int size; //data size
	char file[PACKETSIZE]; //client: filename; server: data
	int seqNum;
	int ackNum;
	int fin;
	
};

int selectFunc(struct timeval timeout, fd_set rset, int sockfd){
	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	timeout.tv_sec = 1; //2 second timeout
	timeout.tv_usec = 0;
	return select(sockfd+1, &rset, NULL, NULL, &timeout);
}
