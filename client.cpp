//David Lassalle
#include <sstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <pthread.h>


using namespace std;
string fin = " quizzaciously";
int peaceout = 0;
string s = " ";
string at = "@";
int sockfd;
int port = 5000;
struct sockaddr_in serv_addr;
struct hostent *server;
string premessage;

void *split(void *ID);
void sender(int sock, char* message);
char* reciever(int sock);
int finished(string rec);



int main(int argc, char *argv[]) {
    if(argc != 3){
	cout << "Error: Wrong number of arguments passed to client" << endl;
	cout << "Please Type: ./client IP username" << endl;
	exit(-1);
    }
    premessage = argv[2];
    pthread_t threads[2];

    // Create a TCP socket for the client
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // The commented code below would create a UDP socket
    // sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
    if (sockfd < 0) {
        cout << "Error: could not create socket for connecting to server" << endl;
	exit(-1);
    }

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        cout << "Error: error getting hostname of server" << endl;
        exit(-1);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cout << "Error: could not connect to server's socket" << endl;
        exit(-1);
    }
	for(int i=0; i<2; i++){
		pthread_create(&threads[i], NULL, &split, (void *)i);
	}
	for(int i=0; i<2; i++){
		pthread_join(threads[i], NULL);
	}
	close(sockfd);

    return 0;
}


void *split(void *ID){
	char* buffer;
        long ii;
        ii = (long)ID;
        if(ii==0){//RECIEVE
                while(!peaceout){
                        buffer = reciever(sockfd);
			cout << buffer << endl;
                }
                return NULL;
        }//end recieve
        else{//SEND
                premessage.insert(0,at);
                char *name = new char[premessage.length()+1];
                strcpy(name,premessage.c_str());
                sender(sockfd,name);

                while(true){
                        cout << "> ";
                        string message;
                        getline(cin,message);
                        if(message != "quit"){
                                char *cmd = new char[message.length()+1];
                                strcpy(cmd,message.c_str());
                                sender(sockfd,cmd);
                        }
                        else{ //quit
                                peaceout = 1;
                                char *cmd = new char[message.length()+1];
                                strcpy(cmd,message.c_str());
                                sender(sockfd,cmd);
				exit(1);
                        }
                }//end while
        } //end send
}




void sender(int sock, char* message){ //send messages with FIN
        int temp;
        string len = message;
        len.append(fin);
        char* write_buffer = new char[len.length()+1];
        strcpy(write_buffer,len.c_str());
        int remaining_length = len.length();
        char* temp_buffer = write_buffer;
        do {
                temp = send(sock, temp_buffer, remaining_length, 0);
                if (temp < 0) {
                        cout << "Error: error sending data to the client" << endl;
                        exit(-1);
                }
                temp_buffer = temp_buffer + temp;
                remaining_length -= temp;
        } while (remaining_length != 0);

}

char* reciever(int sock){ //recieves messages and returns them as char*
        char buf[256];
bzero(buf,256);
        string rec;
        int n;
        do{
                n = recv(sock,buf,255,0);
                if (n < 0) {
                        cout << "Error: error reading from socket; socket may be broken" << endl;
                        exit(-1);
                }
                rec.append(buf);
                bzero(buf,256);
        } while(!finished(rec));

		//remove FIN from incoming message
                string t;
                string bufs; // Have a buffer string
                char* buffer = new char[rec.length()+1];
                strcpy(buffer,rec.c_str());
                stringstream ss(buffer); // Insert the string into a stream
                vector<char*> tokens; // Create vector to hold our words
                while (ss >> bufs){
                        char *temps = new char[bufs.length()+1];
                        strcpy(temps,bufs.c_str());
                        tokens.push_back(temps);
                }
                for ( int k = 0; k < tokens.size()-1; k++ ){
                        t.append(tokens[k]);
                        if(!((tokens.size() == 1)||(k == tokens.size()-2))){
                                t.append(s);
                        }
                }

        char* returnVal = new char[t.length()+1];
        strcpy(returnVal,t.c_str());
        return returnVal;

}

int finished(string rec){ //returns true if we found the end of the message
                char* buffer = new char[rec.length()+1];
                strcpy(buffer,rec.c_str());
                string t;
                string buf; // Have a buffer string
                stringstream ss(buffer); // Insert the string into a stream
                vector<char*> tokens; // Create vector to hold our words
                while (ss >> buf){
                        char *temps = new char[buf.length()+1];
                        strcpy(temps,buf.c_str());
                        tokens.push_back(temps);
                }
                t.append(tokens[tokens.size()-1]);
                if(t == "quizzaciously"){
                        return 1;
                }
                else{
                        return 0;
                }
}

