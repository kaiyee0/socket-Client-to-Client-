#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <string.h>
#include <curses.h> // ncurses
using namespace std;

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }
    // if command go wrong, remind user key in hostname & portno
    
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    // if it cannot find server, indicate error.
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");
    
    char reply[1024];
    recv(sockfd, reply, 1024, 0);
    cout << "\n" << reply;
    memset(reply, 0, 1024);
    // connect message
    
    
    ///////////////
    initscr(); // init
    cbreak(); // 允許讀取字元
    noecho(); // 不讓鍵入字元立刻顯示，相對的就是 echo()
    
    int yMax, xMax;
    getmaxyx(stdscr, yMax, xMax);
    
    WINDOW * menuwin = newwin(6, xMax-12, yMax-8, 5);
    WINDOW * messagewin = newwin(13, xMax-14, yMax-22, 8);
    box(menuwin, 0, 0);
//    box(messagewin, '-', '|');
    box(stdscr, 0, 0);
    refresh();
    wrefresh(messagewin);
    wrefresh(menuwin);
    keypad(menuwin, true); // 啟用讀取特殊按鍵，例如方向鍵，的功能。
    ///////////////
    
    bool outLoop = false;
    int message = 1;
    
    while (!outLoop) {
        string operation;
        char operationChar[100];
        memset(operationChar, 0, 1024);
        bool LogIn = false, SignIn = false;
        
        ///////////////
        string choices[2] = {"sign in", "log in"};
        int choice;
        int highlight = 0;
        
        WINDOW * menuwin = newwin(6, xMax-12, yMax-8, 5);
        box(menuwin, 0, 0);
        wrefresh(menuwin);
        keypad(menuwin, true); // 啟用讀取特殊按鍵，例如方向鍵，的功能。
        
        while(1){
            for(int i = 0; i < 2; i++) {
                if(i == highlight)
                    wattron(menuwin, A_REVERSE);
                mvwprintw(menuwin, i+2, 4, choices[i].c_str());
                wattroff(menuwin, A_REVERSE);
            }
            choice = wgetch(menuwin);
            
            switch(choice) {
                case KEY_UP:
                    highlight--;
                    if(highlight == -1)
                        highlight = 0;
                    break;
                case KEY_DOWN:
                    highlight++;
                    if(highlight == 2)
                        highlight = 1;
                    break;
                default:
                    break;
            }
            if(choice == 10)
                break;
        }
        if (highlight == 0) {
            SignIn = true;
        }
        else {
            LogIn = true;
        }
        getch();
//        endwin();
        ///////////////
        
        if(SignIn) {
            strcat(operationChar, "REGISTER#");
            
            WINDOW * typewin = newwin(6, xMax-12, yMax-8, 5);
            box(typewin, 0, 0); // 畫新的框框
            refresh();
            wrefresh(typewin);
            
            mvprintw(18, 10, "please key in your account name:");
            move(19, 10);
            echo();           /* 開啟 echo 模式, 使輸入立刻顯示在螢幕上 */
            char temp[100];
            getstr(temp);
            strcat(operationChar, temp);
            send(sockfd, operationChar, 1024, 0);
            size_t length = strlen(operationChar);
            mvwprintw(messagewin, message, (xMax-19)-length, "%s", operationChar); // message
            recv(sockfd, reply, 1024, 0);
            mvwprintw(messagewin, message + 1, 0, "%s", reply); // message
            wgetch(messagewin);
            getch();
            refresh();
            memset(operationChar, 0, 1024);
            memset(reply, 0, 1024);

            message += 2;
        }
        else if (LogIn) {
            WINDOW * logwin = newwin(6, xMax-12, yMax-8, 5);
            box(logwin, 0, 0);
            refresh();
            wrefresh(logwin);
            
            mvprintw(18, 10, "please key in your account name:");
            move(19, 10);
            echo();            /* 開啟 echo 模式, 使輸入立刻顯示在螢幕上 */
            getstr(operationChar);
            strcat(operationChar, "#888");
            send(sockfd, operationChar, 1024, 0);
            size_t length = strlen(operationChar);
            mvwprintw(messagewin, message, (xMax-19)-length, "%s", operationChar); // message
            recv(sockfd, reply, 1024, 0);
            mvwprintw(messagewin, message + 1, 0, "%s", reply); // message
            getch();
            wgetch(messagewin);
            message += 2;
            
            if (reply[0] == '2') {
                // 220 log in fail
                continue;
            }
            memset(operationChar, 0, 1024);
            memset(reply, 0, 1024);
            endwin();
            

//            cin >> operation;
//            int i = 0;
//            bool logginSuccess = false;
//            for(; i < operation.size(); i++){
//                if(operation[i] == '#') {
//                    logginSuccess = true;
//                    break;
//                }
//            }
//            if (logginSuccess) {
//                if(i > 9) {
//                    cout << "The name is too long!" << endl;
//                    continue;
//                }
//                send(sockfd, operation.c_str(), operation.size(), 0);
//                recv(sockfd, reply, 1024, 0);
//                cout << reply;
//                memset(reply, 0, 1024);
//                if (reply[0] == '2') {
//                    // 220
//                    continue;
//                }
//
            
            
//              if the program going here, it represent logging in success.
                recv(sockfd, reply, 1024, 0);
                cout << reply << endl;
                memset(reply, 0, 1024);
                bool fst = 1;
                string hisName;
                while (!outLoop) {
                    cout << "ME : ";
                    string operate;
                    cin >> operate;
                    cin.ignore();
                    send(sockfd, operate.c_str(), operate.size(), 0);
                    //cout << " ====< MSG SNT " << endl;
                    if(strcmp(operate.c_str(), "ChatY") == 0){
                        send(sockfd, hisName.c_str(), hisName.size(), 0);
                    }
                    recv(sockfd, reply, 1024, 0);
                    while(strcmp(reply, "") == 0){
                        recv(sockfd, reply, 1024, 0);
                    }
                    //cout << " ====< MSG RECV " << endl;
                    cout << "PC : ";
                    cout << reply << endl;
                    if(reply[0] == '-'){
                      char * pch;
                      //printf ("Splitting string \"%s\" into tokens:\n",reply);
                      pch = strtok(reply," ");
                      int tmp = 6;
                      while (pch != NULL && tmp > 0)
                      {
                        //printf ("%s\n",pch);
                        pch = strtok (NULL, " ");
                        tmp --;
                      }      
                        //cout << pch;
                        string name_tmp(pch);
                        hisName = name_tmp;
                        //cout << hisName << endl;
                    }
                    
                    if (reply[0] == '2') {
                        // 220
                        continue;
                    }
                    if (operate[0] == 'E') {
                        // Exit
                        cout << endl;
                        outLoop = true;
                    }
                    // else if (strcmp(operate.c_str(), "Create") == 0 && !fst){
                    //     recv(sockfd, reply, 1024, 0);
                    //     cout << reply << endl;
                    //}
                    memset(reply, 0, 1024);
                    fst = 0;
                }
//            }
        }
    }
    
    
    
    
    
    
    
    close(sockfd);
    return 0;
}





