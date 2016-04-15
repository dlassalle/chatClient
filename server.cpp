#include <vector>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <pthread.h>
#include <list>
#include <algorithm>
#include <pthread.h>

using namespace std;
list<string> users;
list<int> IDs;
string fin = " quizzaciously";
string s = " ";
pthread_mutex_t mutex_grab;

int add(int sockID, char* user);
int find_user(char* user);
int remove(int sockID, char* user);
void *serve(void *ID);
char* toParse(char* line);
void sender(int sock, char* message);
char* reciever(int sock);
int finished(string rec);

int main() {
     int sockfd, newsockf, portno = 5000;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;

     // Create your TCP socket
     sockfd =  socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) {
        cout << "Error: error creating socket for listening" << endl;
	exit(-1);
     }

     // Setup your server's socket address structure
     // Clear it out, and set its parameters
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;  // set server byte order
     serv_addr.sin_addr.s_addr = INADDR_ANY; // fill in server IP address
     serv_addr.sin_port = htons(portno); // convert port number to network byte order and set port

     // Bind socket to IP address and port on server
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
       cout << "Error: error binding an IP address and port on the server to our socket" << endl;
       exit(-1);
     }

     // Listen on the socket for new connections to be made
     // 20 references the size of the queue of outstanding (non-accepted) requests we will store
     listen(sockfd,20);

     // The accept() call actually accepts an incoming connection
     clilen = sizeof(cli_addr);

     pthread_mutex_init(&mutex_grab, NULL);
	pthread_t threads[10000];
	int thread_num = 0;
	while(1){ //infinite accepting loop
     		// accept a connection from a client, returns a new client socket for communication with client
      		newsockf = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); //new file descriptor, use send and recv to interact with that socket
	      	cout << "Server accepted connection from " << inet_ntoa(cli_addr.sin_addr) << " on port " << ntohs(cli_addr.sin_port) << endl;
		pthread_create(&threads[thread_num],NULL,&serve,(void *) newsockf);
		thread_num++;
	}//end accepting loop

     pthread_mutex_destroy(&mutex_grab);
     close(sockfd);
     return 0;
}// end main



int add(int sockID, char* user){ //Add new connection to table
pthread_mutex_lock(&mutex_grab);
	string person = user;
	users.push_front(person);
	IDs.push_front(sockID);
pthread_mutex_unlock(&mutex_grab);
	return 1;
}
int find_user(char* user){ //find connection in table, return socket descriptor
	int found = 0;
	int id = -1;
	int count = 0;
	string person = user;
	for (list<string>::iterator it=users.begin(); it!=users.end(); ++it){
		if(*it == person){
			found = 1;
			break;
		}
  		count++;
	}
	list<int>::iterator it2=IDs.begin();
	while((count != 0)&&(it2!=IDs.end())){
		++it2;
		count--;
	}
	if(found){
		id = *it2;
	}

	return id; //not found
}
int remove(int sockID, char* user){ //remove connection from table
pthread_mutex_lock(&mutex_grab);
        string person = user;
	list<string>::iterator it;
	it = find (users.begin(), users.end(), person);
	if (it != users.end()){
		users.erase(it);
	}
        list<int>::iterator it2;
        it2 = find (IDs.begin(), IDs.end(), sockID);
        if (it2 != IDs.end()){
                IDs.erase(it2);
        }
pthread_mutex_unlock(&mutex_grab);
	return 1;
}

void *serve(void *ID){
     int newsockfd = (int) ID;
     int temp, n;
     char* buffer;

      // receive name
      buffer = reciever(newsockfd);
      //check that message has been successfully delivered
      string mes = buffer;
      char *me = new char[mes.length()+1];
      strcpy(me,mes.c_str()); //set me to my clients's username
      add(newsockfd,me);
      cout << me << " has logged in." << endl;
    while (1) {
      if (newsockfd < 0) {
        cout << "Error: error creating new socket for communication with client" << endl;
        exit(-1);
      }

	memset(&buffer[0], 0, sizeof(buffer)); //clear the buffer

      // receive data from client using reciever
      buffer = reciever(newsockfd);
      string q = buffer;
      if(q == "quit"){//removes user when they quit
        cout << me << " Logging Off"<< endl;
        remove(newsockfd, me);
        close(newsockfd);
        return NULL;
      }
      else if(q == "list-users"){//list-users command
	string userlist = "Active users: ";
	for (list<string>::iterator it=users.begin(); it!=users.end(); it++){
		userlist = userlist.append(*it);
		userlist = userlist.append(s);
	}
	char* userl = new char[userlist.length()+1];
	strcpy(userl,userlist.c_str());
        sender(newsockfd,userl);
      }
      else{ //regular message
	//get recipient
	char* preparse = new char[q.length()+1];
	strcpy(preparse,q.c_str());
	char* destination = toParse(preparse);

	string message = buffer;
	int dest = find_user(destination);
	if(dest == -1){ //destination user does not exist
		cout << destination << endl;
		char* errmes = (char *)"Sorry, that user does not exist";
		string len = errmes;
		sender(newsockfd,errmes);
	}
	else{// destination user does exists
		string t = me;
    		string buf; // Have a buffer string
    		stringstream ss(buffer); // Insert the string into a stream
    		vector<char*> tokens; // Create vector to hold our words
    		while (ss >> buf){
        		char *temps = new char[buf.length()+1];
        		strcpy(temps,buf.c_str());
        		tokens.push_back(temps);
    		}
    		for ( int k = 1; k < tokens.size(); k++ ){
        		t.append(s);
			t.append(tokens[k]);
    		}
		//SEND MESSAGE
		char* finalmessage = new char[t.length()+1];
		strcpy(finalmessage,t.c_str());
      		sender(dest,finalmessage); //send message with senders usernam in front
	}//end of user existing
      }//end list-users's else
    }//end of while

     return NULL;
} //end serve


char* toParse(char* line){ //returns desitnation of message
        char *p = strtok(line, " ");
        while (p) {
                return p;
                p = strtok(NULL, " ");
        }
}

void sender(int sock, char* message){ //send messages to socket with fin attached at end
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
	}while(!finished(rec));

	//removes the FIN and returns the message that was recieved
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
