#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int s, ss; //socket descriptor
struct sockaddr_in serveur, serveur_data; //structure de l'addresse du serveur.
char bla[256], req[256], rep[1024], data[4096];
int ans = 0; //response from the server

void printusage() {
	printf("Here is the usage:\n\nftpdump <ip> <port> <username> <password>\n\nHave a nice day.\n");
}

void clean_pair() {
	memset(rep, 0, strlen(rep));
	memset(req, 0, strlen(req));
}

void get_data() {
	memset(data, 0, strlen(data));
	read(ss, data, 4096);
	printf("\033[0;32m%s\033[0m", data);
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
		printf("\033[0;35mThe banner did not come back. Are you sure this is an FTP server ?\033[0m\n");
		return -1;
	}
	
	strcpy(req, "USER ");
	strcat(req, argv[3]);
	exchange();
	if (ans!=331) {
		printf("\033[0;35mThe username %s is not valid.\033[0m\n", argv[3]);
		return -1;
	}
	
	strcpy(req, "PASS ");
	strcat(req, argv[4]);
	exchange();
	if (ans!=230) {
		printf("\033[0;35mThe credentials %s:%s is not valid.\033[0m\n", argv[3],argv[4]);
		return -1;
	}
	return 0; //Everything should have gone right... Has it ?	
}

int ftp_list(char **argv) {
	strcpy(req, "PASV");
	exchange();
	if (ans!=227) {
		printf("\033[0;35mPassive Mode could not be set.\033[0m\n");
		return -1;
	}
	/*Asks for passive mode*/
	
	int a,b,data_port;
	if (sscanf(rep, "%*[^(](%*d,%*d,%*d,%*d,%d,%d)", &a, &b) != 2) {
		printf("\033[0;35mCould not get the DATA port. Has the PASV command been answered correctly ?\033[0m\n");
		return -1;
	}
	data_port = a*256+b;
	/*Got the data port*/
	
	ss = socket(AF_INET, SOCK_STREAM, 0);
	if (ss<0) {
		printf("\033[0;35mData socket could not be open.\033[0m\n");
		return -1;
	}
	serveur_data.sin_family = AF_INET;
	serveur.sin_port = htons(data_port);
	inet_pton(AF_INET, argv[1], &(serveur_data.sin_addr));
	printf("\033[0;37mTrying to connect to %s on port %d without credentials.\033[0m\n", argv[1], data_port);
	int error = connect(ss, (struct sockaddr *)&serveur_data, sizeof(serveur_data));
	if (error<0) {
		printf("\033[0;35mCould not connect to data server at %d. Error %d\033[0m\n", data_port, error);
		return -1;
	}
	/*Connects to data port*/
	
	/*Sends the request for listing*/
	strcpy(req, "NLST -la /");
	exchange();
	printf("After last exchange, ans has value %d\n", ans);
	get_data();
	close(ss);
}

int main(int argc, char **argv) {
	if (argc != 5) {
		printf("\033[0;35mWrong number of arguments given.\033[0m\n");
		printusage();
		goto close_correctly;
	}
	/*Creates a socket and connects to the server.*/
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0) {
		printf("\033[0;35mSocket could not open.\033[0m\n");
		goto close_correctly;
	}
	serveur.sin_family = AF_INET;
	serveur.sin_port = htons(atoi(argv[2]));
	inet_pton(AF_INET, argv[1], &(serveur.sin_addr));
	inet_ntop(AF_INET, &(serveur.sin_addr), req, INET_ADDRSTRLEN); //debugging
	printf("\033[0;37mTrying to connect to %s on port %d with username %s and password specified.\033[0m\n", req, atoi(argv[2]), argv[3]);
	if (connect(s, (struct sockaddr *)&serveur, sizeof(serveur))<0) {
		printf("\033[0;35mCould not connect to server.\033[0m\n");
		goto close_correctly;
	}
	
	if (ftp_connect(argv) < 0) {
		printf("\033[0;35mGot error during the FTP authentification. See above for more info\033[0m\n");
		goto close_correctly;
	}
	printf("\033[0;37mSuccessfully logged in with %s account\033[0m\n", argv[3]);
	
	if (ftp_list(argv) < 0) {
		printf("\033[0;35mGot error during the FTP Listing. See above for more info\033[0m\n");
		goto close_correctly;
	}
	close_correctly:
	close(s);
	return 0;
}

