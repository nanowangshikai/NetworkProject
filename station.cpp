// g++ station.cpp -o station
// ./station East_Station 4003 4004 4002 4008 4006
// Kill current program lsof -ti:4003 | xargs kill -9

#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <typeinfo>
#include <iterator>
#include <fstream>
#include <time.h>
#include <ctime>

#define host "localhost"
#define MAXPACK 4800
using namespace std;

int max(int x, int y){
    if(x > y)
        return x;
    else
        return y;
}

// Read current station text file

void printI(vector<int> const &input){
    copy(input.begin(),input.end(), ostream_iterator<int>(cout, " "));
}

void printS(vector<string> const &input){
    copy(input.begin(),input.end(), ostream_iterator<string>(cout, " "));
}




int main(int argc, char const *argv[]){
    // Initial and store command-line argument 
    vector<string> station_data;
    string orginal_station = argv[1];
    int TCP_port = stoi(argv[2]);
    int UDP_port = stoi(argv[3]);
    vector<int> Neighbour;
    for(int i=4; i<argc; i++){
        Neighbour.push_back(stoi(argv[i] ));
    }
    // for(int i=0; i<Neighbour.size(); i++){
    //     cout << "Neighb are" << Neighbour.at(i) <<"\n";
    // }

    // Current time
    time_t rawtime;
    struct tm *timeinfo;
    char time_buffer[80];
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buffer, sizeof(time_buffer), "%H:%M", timeinfo);
    string current_time(time_buffer);
    // cout << "current time ----->" << current_time;

    // Read txt file
    string eachline;
    ifstream in("tt-"+orginal_station+".txt");
    while(getline(in,eachline)){
        if(eachline.size()>0){
            station_data.push_back(eachline);
        }
    }
    in.close();


    int listenfd, connfd, udpfd, nready, maxfdp1;
    char buffer[MAXPACK];
    pid_t childpid;
    fd_set rset;
    ssize_t n;
    socklen_t len;
    const int on = 1;
    struct sockaddr_in cliaddr, servaddr;

    void sig_chld(int);
    int opt = 1;
    char message[MAXPACK] = "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=ISO-8859-4 \r\n\r\n<html><body><div class='img_center'><img src = 'data:image/png;base64,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'><hr><h1>Welcome to TransPerth!</h1><hr></div><h2>The Route of the journey shows below:</h2></body></html>";
    // Create socket fd
    if(( listenfd = socket(AF_INET, SOCK_STREAM,0)) == 0 ){
        perror(" socket failed ");
        exit(EXIT_FAILURE);
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr  = htonl(INADDR_ANY);
    servaddr.sin_port = htons(TCP_port);

    // Each if there is a bus available in current time
    vector<string> time_table;
    for(int i=1; i<station_data.size(); ++i){
        string eachline;
        eachline = station_data.at(i);
        // cout << station_data.at(i) << "\n";
        string time = eachline.substr(0,eachline.find(","));
        if(time > current_time){
            time_table.push_back(eachline);

        }

    }
    // No bus after 10PM
    if(time_table.empty()){
        strcat(message, "<h4>-------------------------------------------------</h4><h4>Today is not bus available after 10pm.</h4><h4>-------------------------------------------------</h4>");
        cout << "No bus after 10PM";
    }

    // printS(time_table);

    // Attaching socket to the TCP port
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))==-1){
        perror("setssockopt");
        exit(EXIT_FAILURE);
    }

    // Attaching socket to the TCP
    if(bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0 ){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // Listen
    if(listen(listenfd, 10) < 0){
        perror("Listen");
        exit(EXIT_FAILURE);
    }
    // Create UDP socket and attaching socket to the UDP
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    bind(udpfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    memset(&servaddr, 0, sizeof(servaddr));
    // Clear the descriptor set
    FD_ZERO(&rset);

    // Get maxfd
    maxfdp1 = max(listenfd, udpfd) + 1;
    for(;;){
        // Set listenfd and udpfd inreadset
        FD_SET(listenfd, &rset);
        FD_SET(udpfd, &rset);

        // TCP socket is readable receive the message
        if(FD_ISSET(listenfd, &rset)){
            len = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len );
            if((childpid == fork()) == 0){
                close(listenfd);
                bzero(buffer, sizeof(buffer));
                read(connfd, buffer, sizeof(buffer));
                cout << "Message From TCP client: " << "\n";
                // puts(buffer);

                // Get station name from URL
                string s;
                s = s+buffer;
                string header = s.substr(0, s.find("\n"));
                string header_nohttp = s.substr(0,header.find(" HTTP"));
                string name_and_time = header_nohttp.substr(header_nohttp.find("=")+1 );
                string station_name = name_and_time.substr(0,name_and_time.find("&"));
                bool direct = false;
                vector<string> final;
                if(!time_table.empty()){
                    for(int i=0; i<time_table.size(); ++i){
                        string eachline;
                        eachline = time_table.at(i);
                        reverse(eachline.begin(),eachline.end());
                        string station = eachline.substr(0, eachline.find(","));

                        reverse(station.begin(), station.end());
                        if(station == station_name && sizeof(station_name)){
                            reverse(eachline.begin(), eachline.end());
                            final.push_back(eachline);
                            direct = true;

                        }                        
                    }        
                }

                if(direct){
                    int n = final[0].length();
                    char station_info[n+1];
                    strcpy(station_info, final[0].c_str());
                    string data_list[5];
                    int count = 0;
                    char delim[] = ",";
                    // cout << "The tokens are: " <<endl;
                    char *token = strtok(station_info, delim);
                    while(token){
                        data_list[count++] = token;
                        token = strtok(NULL, delim);
                    }

                    string departure_time = data_list[0];
                    string vechicle_no = data_list[1];
                    string platform = data_list[2];
                    string arrive_time = data_list[3];
                    
                    string assemble = "<h4>From " + orginal_station + ", " + "Vehicle "+ vechicle_no + " at Platform "+ platform +" ,"+ " Departure Time : "+ departure_time+" . " + "Arrive Time at " + station_name+ ": " + arrive_time +"</h4>";
                    const char *assemble_char = assemble.c_str();
                    strcat(message, assemble_char);

                    // Send the message to the browser(client)
                    send(connfd, message, strlen(message), 0);
                    close(connfd);
                    exit(0);
                }else{
                    
                    strcat(message, "<h4>There is no direct route</h4>");
                    send(connfd, message, strlen(message), 0);
                    char msg[] = "Hello from neighbour";
                    // servaddr.sin_port = htons(4004);

                    sendto(udpfd, msg, strlen(msg), 0, (const struct sockaddr*)&servaddr, sizeof(servaddr));
                    cout << "Message sent to UDP\n ";
  
                    // exit(0)
                }


                
            }
            // close(connfd);

        }

        // UDP socket is readable receive the message
        if(FD_ISSET(udpfd, &rset)){
            len = sizeof(cliaddr);
            bzero(buffer, sizeof(buffer));
            cout << "Message From UDP client: " << "\n";
            n = recvfrom(udpfd, (char *)buffer, sizeof(buffer),0, (struct sockaddr*)&cliaddr, &len);
            puts(buffer);
            sendto(udpfd, (const char*)message, sizeof(buffer), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
        }

    }




    return 0;

}