#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int s; //socket descriptor
struct sockaddr_in serveur; //structure de l'addresse du serveur.
char bla[256], req[256], rep[1024];
int ans = 0; //response from the server

void printusage() {
	printf("Here is the usage:\n\nftpdump <ip> <port> <username> <password>\n\nHave a nice day.\n");
}

void clean_pair() {
	memset(rep, 0, strlen(rep));
	memset(bla, 0, strlen(bla));
	memset(req, 0, strlen(req));
}

void exchange() {
	strcat(req, "\r\n");
	printf("\033[0;34m%s\033[0m", req);
	write(s, req, strlen(req));
	clean_pair();
	read(s, rep, 1024);
	printf("\033[0;31m%s\033[0m", rep);
	sscanf(rep, "%d", &ans); // Gets the number of the response, in order to work with it ^^
}

int ftp_connect(char **argv){
	read(s, rep, 1024);
	printf("\033[0;31m%s\033[0m", rep); //Reads the sever's banner
	sscanf(rep, "%d", &ans);
	if (ans != 220) {
		printf("\033[33mThe banner did not come back. Are you sure this is an FTP server ?\033[0m\n");
		return -1;
	}
	
	strcpy(req, "USER ");
	strcat(req, argv[3]);
	exchange();
	if (ans!=331) {
		printf("\033[33mThe username %s is not valid.", argv[3]);
		return -1;
	}
	
	strcpy(req, "PASS ");
	strcat(req, argv[4]);
	exchange();
	if (ans!=230) {
		printf("\033[33mThe credentials %s:%s is not valid.", argv[3],argv[4]);
		return -1;
	}
	
	strcpy(req, "PASV"); //Sets passive mode
	exchange();
	
}

int main(int argc, char **argv) {
	if (argc != 5) {
		printf("\033[33mWrong number of arguments given.\033[0m\n");
		printusage();
		return 0;
	}
	/*Creates a socket and connects to the server.*/
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		printf("\033[33mSocket could not open.\033[0m\n");
		return 0;
	}
	serveur.sin_family = AF_INET;
	serveur.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &(serveur.sin_addr));
	
	inet_ntop(AF_INET, &(serveur.sin_addr), req, INET_ADDRSTRLEN); //debugging
	printf("\033[0;37mTrying to connect to %s on port %d with username %s and password specified.\033[0m\n", req, atoi(argv[2]), argv[3]);
	if (connect(s, (struct sockaddr *)&serveur, sizeof(serveur))<0) {
		printf("Could not connect to server\n");
		return 0;
	}
	
	if (ftp_connect(argv) < 0) {
		printf("\033[0;31mGot error during the FTP Connection. See above for more info\033[0m\n");
		return 0;
	}
	
	return 0;
}

