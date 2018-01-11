/*
 c++ server socket
 */

#include<stdio.h>
#include<string.h>    //strlen
#include<stdlib.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<pthread.h>   //for threading , link with lpthread
#include<string>
#include<iostream>  //for cout
#include<map>
#include<fstream>
using namespace std;
map<string, struct connectState> info;
map<string, struct connectInfo> user;
int onlineNum = 0;

// the thread function
void *connection_handler(void *);
string List(struct connectInfo);
// check if it Register

struct connectInfo {
    struct sockaddr_in addr;
    int connfdp;
} ;

struct connectState {
    struct sockaddr_in addr;
    string port;
};

int main(int argc , char *argv[])
{
    int socket_desc, c;
    struct sockaddr_in server , client;
    connectInfo client_sock;
    
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    //puts("Socket created");
    // Open server
    
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8888 );
    // port number 8888
    
    //Bind
    bind(socket_desc,(struct sockaddr *)&server , sizeof(server));
    /*
     if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
     {
     //print the error message
     perror("bind failed. Error");
     return 1;
     }
     */
    
    //Listen
    listen(socket_desc , 3);
    
    /*
     //Accept and incoming connection
     //puts("Waiting for incoming connections...");
     c = sizeof(struct sockaddr_in);
     */
    
    //Accept and incoming connection
    puts("Open Server");
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    pthread_t thread_id;
    int connfdp = 0;
    
    while( (connfdp = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
    {
        // client connect to the server
        puts("Connection accepted");
        client_sock.addr = client;
        client_sock.connfdp = connfdp;
        if( pthread_create( &thread_id , NULL ,  connection_handler , (void*) &client_sock ) < 0 )
        {
            // now create thread and go to function "connection_handler"
            perror("could not create thread");
            return 1;
        }
        
        //Now join the thread , so that we don't terminate before the thread
        //pthread_join( thread_id , NULL);
        puts("Handler assigned");
    }
    if (connfdp < 0)
    {
        perror("accept failed");
        return 1;
    }
    
    return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    struct connectInfo data = *(connectInfo*) socket_desc;
    long read_size;
    char clientMessage[2000];
    string message;
    string nowAccount;
    
    //Send some messages to the client
    message = "Connection sucess!\n";
    send(data.connfdp , message.c_str(), message.size(), 0);
    
    
    
    //Receive a message from client
    while( (read_size = recv(data.connfdp , clientMessage , 2000 , 0)) > 0 )
    {
        /*
         //end of string marker
         client_message[read_size] = '\0';
         //Send the message back to client
         write(sock , client_message , strlen(client_message));
         */
        
        
        // REGISTER
        if (strspn(clientMessage, "REGISTER#") == 9) {
            string regisName;
            for (int j = 9; j < strlen(clientMessage); j++) {
                regisName += clientMessage[j];
            }
            if (info.find(regisName) != info.end()) {
                // it exist !!
                message = "210 FALL\n";
                send(data.connfdp , message.c_str(), message.size(), 0);
            }
            else {
                info[regisName].port = "-1";
                message = "100 OK\n";
                send(data.connfdp , message.c_str(), message.size(), 0);
            }
            memset(clientMessage, 0, 20*sizeof(char));
            continue;
        }
        
        
        // log-in
        bool logSuccess = false;
        string account;
        string portNum;
        for (int i = 0; i < strlen(clientMessage); i++) {
            if (clientMessage[i] == '#') {
                if (info.find(account) != info.end()) {
                    // log-in succss
                    nowAccount = account;
                    for (int j = i + 1; clientMessage[j] != '\0'; j++) {
                        portNum += clientMessage[j];
                    }
                    logSuccess = true;
                    info[account].port = portNum;
                    user[account].connfdp = data.connfdp;
                    cout << "connfdp : " << user[account].connfdp << endl;
                    cout << "portNum : " << info[account].port << endl;
                    onlineNum++;
                    memset(clientMessage, 0, 20*sizeof(char));
                    break;
                }
                else {
                    message = "220 AUTH_FALL\n";
                    send(data.connfdp , message.c_str(), message.size(), 0);
                    break;
                }
            }
            account += clientMessage[i];
        }
        if (logSuccess) {
            message = "Loggin Successful!!!";
            send(data.connfdp, message.c_str(), message.size(), 0);
            message = List(data);
            message += "What would you like to do ?\n";
            send(data.connfdp, message.c_str(), message.size(), 0);
            int temp_recv, temp_cont;
            cout << "============================" << endl;
            while ( (read_size = recv(data.connfdp , clientMessage , 2000 , 0)) > 0 ) {
                //cout << "Receive from : " << data.connfdp << endl;
                cout << "RCV MSG !!!" << temp_cont << endl;
                message = account;
                message += " : ";
                if (strspn(clientMessage, "Exit") == 4) {
                    message = "Bye";
                    send(data.connfdp , message.c_str(), message.size(), 0);
                    info[account].port = "-1";
                    //onlineNum--;
                    break;
                }
                
                else if (strspn(clientMessage, "List") == 4) {
                    message = List(data);
                    send(data.connfdp , message.c_str(), message.size(), 0);
                    message += "What else would you like to do ?";
                    message += account;
                    message += " : ";
                }

                else if (strspn(clientMessage, "Create") == 6) {
                    message = "Who to contact ? \n";
                    message += List(data);
                    // message += account;
                    // message += " : ";
                    memset(clientMessage, 0, 20*sizeof(char));
                
                    send(data.connfdp , message.c_str(), message.size(), 0);
                    //cout << "to client " << message << endl;
                    recv(data.connfdp , clientMessage , 2000 , 0);
                    //cout << "from client : " << clientMessage << endl;
                    cout << "RCV MSG !!!" << endl;
                    cout << "        Create LOG" << endl;
                    cout << "=======================" << endl;
                    if(info.find(clientMessage) != info.end()){
                        // To be contacted
                        temp_recv = user[clientMessage].connfdp;
                        cout << "RCVer is : " << temp_recv << endl;
                        // Contacter
                        temp_cont = data.connfdp;
                        cout << "CNer is : " << temp_cont << endl;
                        message = "What would you like to say to him/her?";
                        send(temp_cont, message.c_str(), 2000, 0);
                        cout << "SND MSG !!!" << temp_cont << endl;
                        memset(clientMessage, 0, 20*sizeof(char));
                        recv(temp_cont, clientMessage, 2000, 0);
                        cout << "RCV MSG !!!" << temp_cont << endl;
                        send(temp_recv, clientMessage, 2000, 0);
                        cout << "SND MSG !!!" << temp_recv << endl;
                        //cout << "ITS OVER COOOOLLLLL" << endl;
                        // message = "Msg sent...";
                        // send(temp_cont, message.c_str(), 2000, 0);
                    }
                
                //
                    //Wanna find if there exists the ppl to create a group
                    // if(info.find(clientMessage) != info.end()){
                    //     // To be contacted
                    //     temp_recv = user[clientMessage].connfdp;

                    //     // Contacter
                    //     temp_cont = data.connfdp;


                    //     message = "\n ========================== \n";
                    //     message += "Someone want to talk to you \n" ;
                    //     message += "Enter Y# for okay or others to block\n" ;
                    //     // message += account;
                    //     // message += " : ";

                    //     //send(data.connfdp , message.c_str(), message.size(), 0);
                    //     send(user[clientMessage].connfdp, message.c_str(), message.size(), 0);
                        
                    //     //Log info
                    //     //cout << user[clientMessage].connfdp << " msg sent \n" << endl;
                    //     recv(temp_recv, clientMessage, 2000, 0);
                    //     cout << clientMessage << endl;
                    //     cout << "YOYOYOY" << endl;
                    //     if(strspn(clientMessage, "Y#") == 2){
                    //         cout << "The loop Start" << endl;
                    //         message = "Contact Connected !\n";
                    //         message += " ========================== \n";
                    //         // message += account;
                    //         // message += " : ";
                    //         send(temp_recv, message.c_str(), message.size(), 0);
                    //         send(temp_cont, message.c_str(), message.size(), 0);
                    //         bool outLoop = 0;
                    //         while(!outLoop){
                    //             cout << "Round Start !" << endl;
                    //             cout << "From Client1 : " << endl;
                    //             memset(clientMessage, 0, 20*sizeof(char));
                    //             recv(temp_cont, clientMessage, 2000, 0);
                    //             cout << account << " : " << clientMessage << endl;
                    //             if(strspn(clientMessage, "Exit") == 4){
                    //                 outLoop = 1;
                    //                 message += "User has left the room.\n";
                    //                 send(temp_recv, message.c_str(), 2000, 0);
                    //                 break;
                    //             }
                    //             send(temp_recv, clientMessage, 2000, 0);
                    //             cout << "From Client2 : " << endl;
                    //             memset(clientMessage, 0, 20*sizeof(char));
                    //             recv(temp_recv, clientMessage, 2000, 0);
                    //             if(strspn(clientMessage, "Exit") == 4){
                    //                 outLoop = 1;
                    //                 message += "User has left the room.\n";
                    //                 send(temp_recv, message.c_str(), 2000, 0);
                    //                 break;
                    //             }
                    //             send(temp_cont, clientMessage, 2000, 0);
                    //         }                            
                    //     }
                    //     else{
                    //         message = "Contact Denied !\n";
                    //         message += "What else would you like to do ?\n";
                    //         //send(temp_recv, message.c_str(), message.size(), 0);
                    //         send(temp_cont, message.c_str(), message.size(), 0);
                    //     }
                    // }
                //
                    else{
                        message = "User Not Found ! \n";
                        //send(data.connfdp, message.c_str(), message.size(), 0);
                    }
                    //send(data.connfdp , message.c_str(), message.size(), 0);
                }
                else if(strspn(clientMessage, "Wait") == 4){

                }

                else if(strspn(clientMessage, "ChatY") == 5){
                    memset(clientMessage, 0, 20*sizeof(char));
                    recv(data.connfdp , clientMessage , 2000 , 0);
                    //cout << "from client : " << clientMessage << endl;
                    cout << "RCV MSG !!!" << endl;
                    cout << "        Chat LOG" << endl;
                    cout << "=======================" << endl;
                    if(info.find(clientMessage) != info.end()){
                        // To be contacted
                        temp_recv = user[clientMessage].connfdp;
                        cout << "RCVer is : " << temp_recv << endl;
                        // Contacter
                        temp_cont = data.connfdp;
                        cout << "CNer is : " << temp_cont << endl;
                        message = "\n ============Welcome============";
                        message += "\n Say Something...\n";
                        send(temp_recv, message.c_str(), 2000, 0);
                        message = "\n ============Welcome============";
                        message += "\n Press Y to confirm...\n";

                        send(temp_cont, message.c_str(), 2000, 0);
                        memset(clientMessage, 0, 20*sizeof(char));
                        while((read_size = recv(temp_cont, clientMessage, 2000, 0)) > 0 ){
                            //recv(temp_cont, clientMessage, 2000, 0);


                            

                            if(strspn(clientMessage, "Help") == 4){
                                string hotnews;
                                ifstream inFile("hotnews.txt");
                                int countnum=0;
                                message = "";
                                getline(inFile,hotnews);
                                while(getline(inFile,hotnews)){
                                    cout<<hotnews<<endl;
                                    message += "\n";
                                    message += hotnews;
                                    countnum++;
                                    if(countnum>=5){
                                        break;
                                    }
                                }
                                send(temp_cont, message.c_str(), 2000, 0);                                
                            }
                            else if (strspn(clientMessage, "Leave") == 5){
                                message = "\n ==============Bye==============";
                                send(temp_cont, message.c_str(), 2000, 0);
                                message = "\n He/She has left the room ...";
                                message += "\n ==============Bye==============";
                                send(temp_recv, message.c_str(), 2000, 0);
                                break;
                            }
                            else{
                                cout << "RCV MSG !!!" << temp_cont << endl;
                                send(temp_recv, clientMessage, 2000, 0);
                                cout << "SND MSG !!!" << temp_recv << endl;
                                memset(clientMessage, 0, 20*sizeof(char));
                            }


                        }
                        
                    }
                }

                else if(strspn(clientMessage, "Chat") == 4){
                    message = "Who to chat ? \n";
                    message += List(data);
                    // message += account;
                    // message += " : ";
                    memset(clientMessage, 0, 20*sizeof(char));
                
                    send(data.connfdp , message.c_str(), message.size(), 0);
                    //cout << "to client " << message << endl;
                    recv(data.connfdp , clientMessage , 2000 , 0);
                    //cout << "from client : " << clientMessage << endl;
                    cout << "RCV MSG !!!" << endl;
                    cout << "        Chat LOG" << endl;
                    cout << "=======================" << endl;
                    if(info.find(clientMessage) != info.end()){
                        // To be contacted
                        temp_recv = user[clientMessage].connfdp;
                        cout << "RCVer is : " << temp_recv << endl;
                        // Contacter
                        temp_cont = data.connfdp;
                        cout << "CNer is : " << temp_cont << endl;
                        message = "-Would you like to chat with ";
                        message += account;
                        message += " ? Reply \"ChatY\" to chat !";
                        send(temp_recv, message.c_str(), 2000, 0);
                        //send(temp_cont, message.c_str(), 2000, 0);
                        memset(clientMessage, 0, 20*sizeof(char));
                        while((read_size = recv(temp_cont, clientMessage, 2000, 0)) > 0 ){
                            if(strspn(clientMessage, "Help") == 4){
                                string hotnews;
                                ifstream inFile("hotnews.txt");
                                int countnum=0;
                                message = "";
                                getline(inFile,hotnews);
                                while(getline(inFile,hotnews)){
                                    cout<<hotnews<<endl;
                                    message += "\n";
                                    message += hotnews;
                                    countnum++;
                                    if(countnum>=5){
                                        break;
                                    }
                                }
                                send(temp_cont, message.c_str(), 2000, 0);                                
                            }
                            else if (strspn(clientMessage, "Leave") == 5){
                                message = "\n ==============Bye==============";
                                send(temp_cont, message.c_str(), 2000, 0);
                                message = "\n He/She has left the room ...";
                                message += "\n ==============Bye==============";
                                send(temp_recv, message.c_str(), 2000, 0);
                                break;
                            }
                            else{
                                cout << "RCV MSG !!!" << temp_cont << endl;
                                send(temp_recv, clientMessage, 2000, 0);
                                cout << "SND MSG !!!" << temp_recv << endl;
                                memset(clientMessage, 0, 20*sizeof(char));
                            }
                        }
                        //cout << "ITS OVER COOOOLLLLL" << endl;
                        // message = "Msg sent...";
                        // send(temp_cont, message.c_str(), 2000, 0);
                    }
                }
                

            //
                // else if (strspn(clientMessage, "Y#") == 2) {
                //     cout << "Client2 INNNNNNN" << endl;
                //     bool outLoop = 0;
                //         while(!outLoop){
                //             memset(clientMessage, 0, 20*sizeof(char));
                //             recv(temp_recv, clientMessage, 2000, 0);
                //             if(strspn(clientMessage, "Exit") == 4){
                //                 outLoop = 1;
                //                 message += "User has left the room.\n";
                //                 send(temp_recv, message.c_str(), 2000, 0);
                //                 break;
                //             }
                //             send(temp_cont, clientMessage, 2000, 0);
                //         }
                // }
            //
                else {
                    cout << "SHIT ITS BROKEN STATE !!!" << endl;
                    //cout << clientMessage << endl;
                    message = "Ok ... Fuck You ! \n";
                    send(data.connfdp, message.c_str(), message.size(), 0);
                }
                memset(clientMessage, 0, 20*sizeof(char));
            }
        }
        

        
        //clear the message buffer
        memset(clientMessage, 0, 20*sizeof(char));
    }
    
    
    if(read_size == 0)
    {
        info[nowAccount].port = "-1";
        onlineNum--;
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
    
    return 0;
}

string List(struct connectInfo data) {
    string message;
    char onlineNumChar[4];
    //message = "Account Balance: 1000\n";
    message += "==================\n";
    message += "Online Number: ";
    sprintf(onlineNumChar, "%d", onlineNum);
    message += onlineNumChar;
    message += "\n";
    //cout << data.connfdp;
    for (map<string, struct connectState>::iterator it = info.begin(); it != info.end(); it++) {
        if (it->second.port != "-1") {
            message += it->first;
            message += "#";
            message += inet_ntoa(data.addr.sin_addr);
            message += "#";
            message += it->second.port;
            message += "#";
            message += data.connfdp;
            message += "\n";
        }
    }
    return message;
}







