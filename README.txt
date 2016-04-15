David Lassalle

Process of Solving:
First, I created a protocol for sending and recieving smaller sized messages as the server and client.
Then I worked on making the client multi-threaded and developed a quit functionality for 
both it and the server.
Lastly I worked on makind the server multi-threaded and developed the list-users functionality.
Finally, I retro-fitted my small message sending and recieving system to account for the problem of send() not sending all the bytes it needs to in one call.


Problems Encountered:
One of the first problems I encountered was when i tried to make different threads: when i typed "make" the compiler would tell me that my usage of pthread-create and pthread_join were undefined references. I googled the problem and fixed it by changing the -lpthread location in the make file.
The assignment was realitively straight-forward for the most part, the only part that began to create issues was when I tried to retro-fit my messaging code. However, this problem was solved through tracking down each bug by looking at the errors thrown when compiled. I also ran into an issue where random characters were being sent from client to server and server to client; through some testing I realized that I did not clear the char* arrays that I had been using.


Final Results:
The server and client should work for all specifications in the description. I havent been able to test it with a public IP address, but i beleive that it should work as it is required to. 
Quit, list-users, and messaging functionalities are all working, the only issue is that sometimes when you start the server it has an error using bind() originally, but you can just keep trying to execute ./server until it works.
Lastly, clients can be exited by "quit" while the server remains on until killed by the programmer.
