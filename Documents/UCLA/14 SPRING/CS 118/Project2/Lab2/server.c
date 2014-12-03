#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>   /* for the waitpid() system call */
#include <signal.h>     /* signal name macros, and the kill() prototype */
#include <fcntl.h>   //read oflag constants
#include <string.h>
#include <errno.h>
#include "packet.h"
#include <sys/stat.h>
#include <sys/time.h>
#include <ctype.h>

//#define PORT_NUM 5560
//#define WINDOW_SIZE 5
#define TIMEOUT 5000


int main(int argc, char *argv[]){
	struct sockaddr_in serv_addr, cli_addr;
	int sockfd, newsockfd;
	int PORT_NUM, WINDOW_SIZE;
	socklen_t clientlen;
	int reqSize;
	struct packet reqPacket, respPacket, finPack, ackPack;
	struct stat fileStat;
	int fileSize;
	FILE *fp; 
	struct packet* packetQ;
	int packetQsize;

	
	int base;
	time_t timer;
	double PROB_LOSS;
	double PROB_CORR;
	
	if(argc < 4) {
		fprintf(stderr, "invalid argument: port_number congestion_window_size prob_loss prob_corruption\n");
		exit(1);
	}
	//packet queue
	
	PORT_NUM = atoi(argv[1]);
	WINDOW_SIZE = atoi(argv[2]);
	if(WINDOW_SIZE < 1){
		fprintf(stderr, "invalid argument: congestion_window_size must be greater than 1\n");
		exit(1);
	}
	PROB_LOSS = atof(argv[3]);
	PROB_CORR = atof(argv[4]);

	//create a socket	
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);	
	if (sockfd < 0){
		fprintf(stderr, "ERROR in opening socket");
		exit(1);
	}
	
	//bind the socket descriptor
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; //type ofsocket
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(PORT_NUM); //port used
	
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))< 0)
	{
		fprintf(stderr, "ERROR in binding\n");
		exit(1);
	}

	clientlen = sizeof(cli_addr);
	
	while(1){
		int lostRequest = 0;
		int corruptReq = 0;
		printf("Server:		waiting for file request\n");
		//receive request packet ==> saving to reqPacket
		reqSize = recvfrom(sockfd, &reqPacket, sizeof(reqPacket), 0, (struct sockaddr *) &cli_addr, &clientlen);
		if(reqSize < 0 || rand() < RAND_MAX*PROB_LOSS ){
			printf("Server:		Packet Loss \n");
			lostRequest = 1;
		} else if(reqPacket.file == NULL || rand() < RAND_MAX*PROB_CORR ){
			printf("Server:		Corrupt Packet \n");
			corruptReq = 1;
		}
		//only continue if request not lost, otherwise, hit end of while and keep waiting
		if(lostRequest == 0 && corruptReq == 0){
			printf("Server:		DATA received seq#%d, ACK=#%d, FIN %d, content-length: %d\n", reqPacket.seqNum, reqPacket.ackNum, reqPacket.fin, reqPacket.size); 

			//send ACK for request
			struct packet ackRequest;
			ackRequest.packetType = 1;
			int ackSize;
			ackSize = sendto(sockfd, &ackRequest, sizeof(struct packet), 0, (struct sockaddr *) &cli_addr, clientlen);
			if(ackSize < 0){
				fprintf(stderr, "Couldn't send ACK packet for request");
			}

			//intialize window 
			base = 0;

			//create file descriptor to file named in reqPacket
			char clientFile[sizeof(reqPacket.file)];
			int j;
			for(j = 0; reqPacket.file[j] != '\0'; j++){
				clientFile[j] = reqPacket.file[j];
			}
			clientFile[j] = '\0';
		
			printf("Server:		File requested is \"%s\"\n", clientFile);
			fp = fopen(clientFile, "rb");
			
			//sending FILE dne packet
			struct packet fileDNE;
			fileDNE.packetType = 4;
			fileDNE.ackNum = reqPacket.size + 1;
			fileDNE.seqNum = 0;
			int dneSize;
			int filedneFlag = 0;
			if( fp == NULL){ //file doesn't exist
				printf("Server:		ERROR File does not exist\n");
				dneSize = sendto(sockfd, &fileDNE, sizeof(struct packet), 0, (struct sockaddr *) &cli_addr, clientlen);	
				filedneFlag = 1;
			}
			int numBytesRead = 0;
			int serverAck = reqPacket.size + 1;
			
			//if file exists
			if(filedneFlag == 0){
				//get fileSize, divide file into packets, put in packetQ
				stat(reqPacket.file, &fileStat);
				fileSize = fileStat.st_size;
				
				//make packet queue
				packetQsize = fileSize % PACKETSIZE ? (fileSize/PACKETSIZE) + 1 : (fileSize/PACKETSIZE);
				packetQ = (struct packet*) malloc (packetQsize*sizeof(struct packet));
				
				int i; //how many packets sent
				for(i = 0; i < packetQsize; i++){
					//create response packet 

					packetQ[i].size = fread(packetQ[i].file, 1, PACKETSIZE, fp);
					packetQ[i].packetType = 2;
					packetQ[i].ackNum = serverAck;	
					packetQ[i].seqNum = numBytesRead;
					packetQ[i].fin = 0;
			
					numBytesRead += packetQ[i].size;
				}
		
				int nextSeqNum = base + WINDOW_SIZE;
				int sentSize;
				for(i = base; i < packetQsize && i < base + WINDOW_SIZE; i++){ //send first WINDOW_SIZE packets
						//acknum
						packetQ[i].ackNum = serverAck;
						sentSize = sendto(sockfd, &packetQ[i], sizeof(struct packet), 0, (struct sockaddr *) &cli_addr, clientlen);	
				
						printf("Server:		DATA sent seq#%d, ACK=%d, FIN %d, content-length: %d\n", packetQ[i].seqNum, packetQ[i].ackNum, packetQ[i].fin, packetQ[i].size);
						if (sentSize < 0){
							fprintf(stderr, "something wrong with sending packet");
						}
					}

					struct timeval timeout;
					fd_set rset;
					int notTimeout;

					int lostPacket = 0;
					int corruptPacket = 0;
					while(base < packetQsize){ // finished sending entire file
					
						//listen for ACK		
						/*FD_ZERO(&rset);
						FD_SET(sockfd, &rset);
						timeout.tv_sec = 1; //5 second timeout
						timeout.tv_usec = 0;*/
						
						//If packet lost, receive, discard and keep listening
						do{
							lostPacket = 0;
							corruptPacket = 0;
							notTimeout = selectFunc(timeout, rset, sockfd);
							if (rand() < RAND_MAX*PROB_LOSS){
								printf("Server:		Packet lost \n");
								lostPacket = 1;
								if (notTimeout > 0){
									reqSize = recvfrom(sockfd, &reqPacket, sizeof(reqPacket), 0, (struct sockaddr *) &cli_addr, &clientlen);	
								}
							} else if(rand() < RAND_MAX*PROB_CORR){
								printf("Server:		Packet corrupt \n");
								corruptPacket = 1;
								if (notTimeout > 0){
									reqSize = recvfrom(sockfd, &reqPacket, sizeof(reqPacket), 0, (struct sockaddr *) &cli_addr, &clientlen);	
								}
							}
						} while(lostPacket || corruptPacket);
					
						if(notTimeout < 0){
							fprintf(stderr, "Error with select");				
						} else if(notTimeout == 0) {//timed out
							printf("Server:		(ACK lost or corrupted) Timeout\n");
							//resend everything in window	
							int sentSize;
							for(i = base; i < packetQsize && i < base + WINDOW_SIZE; i++){ //send everything in window
								packetQ[i].ackNum = serverAck;
								sentSize = sendto(sockfd, &packetQ[i], sizeof(struct packet), 0, (struct sockaddr *) &cli_addr, clientlen);	 
								if (sentSize < 0){
									fprintf(stderr, "something wrong with sending packet\n");

								}
								printf("Server:		DATA sent seq#%d, ACK=#%d, FIN %d, content-length: %d \n", packetQ[i].seqNum, packetQ[i].ackNum, packetQ[i].fin, packetQ[i].size); 
							}			
						} else { //received ACK
							reqSize = recvfrom(sockfd, &reqPacket, sizeof(reqPacket), 0, (struct sockaddr *) &cli_addr, &clientlen);	
							if(reqPacket.packetType != 1){
								fprintf(stderr, "it wasn't an ACK packet that got sent\n");			
							}
							printf("Server:		ACK received seq#%d, ACK=#%d, FIN %d, content-length: %d \n", reqPacket.seqNum, reqPacket.ackNum, reqPacket.fin, reqPacket.size); 
							if (packetQsize > WINDOW_SIZE){ //keep going if more than WINDOW_SIZE packets
								if(reqPacket.seqNum == serverAck){ //if base == ack num
									serverAck = reqPacket.seqNum + 1;
									if(nextSeqNum != packetQsize){ //if haven't finished sending all packets
										packetQ[nextSeqNum].ackNum = serverAck;
										sentSize = sendto(sockfd, &packetQ[nextSeqNum], sizeof(struct packet), 0, (struct sockaddr *) &cli_addr, clientlen);
										printf("Server:		DATA sent seq#%d, ACK=#%d, FIN %d, content-length: %d \n", packetQ[nextSeqNum].seqNum, packetQ[nextSeqNum].ackNum, packetQ[nextSeqNum].fin, packetQ[nextSeqNum].size); 
										nextSeqNum++;
									}
									base++;
									printf("Server:		sliding window\n");
								} else if (reqPacket.seqNum >= serverAck) { //out of order ACK bu cummulative
									serverAck = reqPacket.seqNum + 1;
									int extra = (reqPacket.ackNum/PACKETSIZE) - base;
									base = base + extra;
									int sentSize;
									if(nextSeqNum < packetQsize){ //haven't sent everything yet
										for(i = nextSeqNum; i < extra+nextSeqNum && i < packetQsize; i++){ //send extra

											packetQ[i].ackNum = serverAck;
											sentSize = sendto(sockfd, &packetQ[i], sizeof(struct packet), 0, (struct sockaddr *) &cli_addr, clientlen);	 
											if (sentSize < 0){
												fprintf(stderr, "something wrong with sending packet\n");
											}
											printf("Server:		DATA sent seq#%d, ACK=#%d, FIN %d, content-length: %d i is %d \n", packetQ[i].seqNum, packetQ[i].ackNum, packetQ[i].fin, packetQ[i].size, i); 
										}
										if( i< packetQsize){	
											nextSeqNum += extra;
										} else if (i == packetQsize){
											nextSeqNum = packetQsize;								
										}
									} 
								} else{ //get ACK that is less than Window
									if(nextSeqNum != packetQsize){
										packetQ[nextSeqNum-1].ackNum = serverAck-1;
										sentSize = sendto(sockfd, &packetQ[nextSeqNum-1], sizeof(struct packet), 0, (struct sockaddr *) &cli_addr, clientlen);
										printf("Server:		DATA sent seq#%d, ACK=#%d, FIN %d, content-length: %d \n", packetQ[nextSeqNum-1].seqNum, packetQ[nextSeqNum-1].ackNum, packetQ[nextSeqNum-1].fin, packetQ[nextSeqNum-1].size); 
									}
								}
							} else { //get ACK for first WINDOW_SIZE packets
								base++;
							}
						}
					} //finished sending everything
					printf("Server:		final transfer complete \n");
				}
				//create FIN packet
				finPack.fin = 1;
				finPack.seqNum = numBytesRead;
				finPack.ackNum = serverAck;
				finPack.size = 0;
				finPack.packetType = 3;

				//wait on FIN-ACK
				int timed = 0;
				int sentSize;
				struct timeval timeout;
				fd_set rset;
					
				int finackLost;
				
				//send it once
				sentSize = sendto(sockfd, &finPack, sizeof(struct packet), 0, (struct sockaddr *) &cli_addr, clientlen);	
				printf("Server:		DATA sent seq#%d, ACK=#%d, FIN %d, content-length: %d \n", finPack.seqNum, finPack.ackNum, finPack.fin, finPack.size); 
				do{ //listen
					finackLost = 0;
					FD_ZERO(&rset);
					FD_SET(sockfd, &rset);
					timeout.tv_sec = 1; //5 second timeout
					timeout.tv_usec = 0;

					timed = select(sockfd+1, &rset, NULL, NULL, &timeout);	

					if (timed > 0 && rand()<RAND_MAX*PROB_LOSS ){
						printf("Server:		FINACK lost\n");
						reqSize = recvfrom(sockfd, &reqPacket, sizeof(reqPacket), 0, (struct sockaddr *) &cli_addr, &clientlen);	
						finackLost = 1;
					} else if ( timed == 0){
						sentSize = sendto(sockfd, &finPack, sizeof(struct packet), 0, (struct sockaddr *) &cli_addr, clientlen);	
						printf("Server:		DATA sent seq#%d, ACK=#%d, FIN %d, content-length: %d \n", finPack.seqNum, finPack.ackNum, finPack.fin, finPack.size); 
					}
				} while (finackLost == 1 || timed == 0);	
				
				reqSize = recvfrom(sockfd, &reqPacket, sizeof(reqPacket), 0, (struct sockaddr *) &cli_addr, &clientlen);	

				printf("Server:		FINACK received seq#%d, ACK=#%d, FIN %d, content-length: %d \n", reqPacket.seqNum, reqPacket.ackNum, reqPacket.fin, reqPacket.size);  

				ackPack.packetType = 3;
				ackPack.seqNum = reqPacket.ackNum;
				ackPack.ackNum = reqPacket.seqNum + 1;
				ackPack.size=0;
				ackPack.fin = 1;
				//send ACK
				int getPacket;
				do{
					getPacket = 0;
					sentSize = sendto(sockfd, &ackPack, sizeof(struct packet), 0, (struct sockaddr *) &cli_addr, clientlen);	
					printf("Server:		FINACK sent seq#%d, ACK=#%d, FIN %d, content-length: %d \n", ackPack.seqNum,ackPack.ackNum, ackPack.fin, ackPack.size); 

					FD_ZERO(&rset);
					FD_SET(sockfd, &rset);
					timeout.tv_sec = 1; //5 second timeout
					timeout.tv_usec = 0;

					timed = select(sockfd+1, &rset, NULL, NULL, &timeout);	

					if(timed == 0){
						break;
					} else if (timed > 0){
						reqSize = recvfrom(sockfd, &reqPacket, sizeof(reqPacket), 0, (struct sockaddr *) &cli_addr, &clientlen);	
						getPacket = 1;
					}
				} while (getPacket == 1);		
			
				//timed out --- good!
				 if(timed == 0){
					close(sockfd); //close socket
					printf("Server:		close connection\n");
					break;
				}
		}
	}	
		
	
}

/*int selectFunc(struct timeval timeout, fd_set rset, int sockfd){
	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	timeout.tv_sec = 1; //5 second timeout
	timeout.tv_usec = 0;
	return select(sockfd+1, &rset, NULL, NULL, &timeout);
}*/
