/* Server code in C++ */
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string>
#include <thread>
#include <map>
#include <sstream>
#include <mutex>
using namespace std;

#define PASSWORDSERVER "ucsp"


//global variables
map<string,int> listUsers;
map<int,int> UserAttemps;
mutex mutexListUsers;


string findUser(int ConnectFD){
    for(map<string,int>::iterator it=listUsers.begin();it!=listUsers.end();++it){
        if(it->second==ConnectFD) return it->first;
    }
    return "";
}
map<int ,int> parametersParse(string inputbash){
    map<int,int> parameters;
    int numParameters; //numero de parametros del inputbash}
    stringstream geek(inputbash.substr(0,2));
    geek>>numParameters;
    inputbash=inputbash.substr(2);  
    //inputbash=inputbash.substr(numParameters*2);              //corta el numero de parametros ya
    //llenar map con el numero de parametros de numParameters
    for(int i=0;i<numParameters;i++){
        stringstream geek(inputbash.substr(i*2,2));
        geek>>parameters[i+1];
    }
    return parameters;

}

map<int,string> wordsParse(map<int,int> &parameters, string inputbash){
    map<int,string> words;
    for(int i=1;i<=parameters.end()->first;i++){
        words[i]=inputbash.substr(0,parameters[i]);
        inputbash=inputbash.substr(parameters[i]);  //corta el inputbash hasta la palabras que aun faltan
    } 
    return words;

}

string messageParser(string inputbash,int connectFD){
    string user;
    int n;
    string messageResponse;
    string inputCode=inputbash.substr(0,1);//codigo caracteres =1
    if(inputCode=="x") {
        listUsers.erase(findUser(connectFD));
        return "X";
    }
    inputbash=inputbash.substr(1);          //input bash sin el caracter de codigo 
    string copybash=inputbash;
    map<int,int> parameters=parametersParse(inputbash); //parametros parseados
    
    inputbash=inputbash.substr(2); 
    inputbash=inputbash.substr((parameters.end()->first)*2); //elimina el header solo deja las palabras
    
    map<int,string> messageWords=wordsParse(parameters,inputbash);
    if(inputCode=="l"){
        if(parameters.end()->first==2){        //login debe tener solo dos parametros 
            if(messageWords[2]==PASSWORDSERVER){
                lock_guard<mutex> guard(mutexListUsers);
                listUsers[messageWords[1]]=connectFD;
                UserAttemps[connectFD]==0;
                messageResponse="L0102OK";
                return messageResponse;
            }          
        }
    }
    if(inputCode=="m"){
        user=findUser(connectFD);
        if(listUsers[user]){
            messageResponse="M";
            //rellenar mensaje
            messageResponse.append(2-(to_string(parameters.end()->first)).length(),'0');
            messageResponse.append(to_string(parameters.end()->first));
            //cambiar user
            int user_length=user.length();
            messageResponse.append(2-(to_string(user_length)).length(),'0');
            messageResponse.append(to_string(user_length));

            for(int i=2;i<=parameters.end()->first;i++){
                messageResponse.append(2-(to_string(parameters[i]).length()),'0');
                messageResponse.append(to_string(parameters[i]));
            }
            messageResponse.append(user);
            for(int i=2;i<=messageWords.end()->first;i++){
                messageResponse.append(messageWords[i]);
            }
            user=listUsers.find(messageWords.find(1)->second)->first;    
            n=write(listUsers[user],messageResponse.c_str(),messageResponse.size());
            return "0Mensaje Enviado al usuario";
        }
    }
    if(inputCode=="b"){
        messageResponse="B";
        //rellenar mensaje
        user=findUser(connectFD);
        messageResponse.append(2-to_string((parameters.end()->first)+1).length(),'0');
        messageResponse.append(to_string((parameters.end()->first)+1));
        messageResponse.append(2-to_string(user.length()).length(),'0');
        messageResponse.append(to_string(user.length()));
        for(int i=1;i<=parameters.end()->first;i++){
            messageResponse.append(2-to_string(parameters[i]).length(),'0');
            messageResponse.append(to_string(parameters[i])); 
        }
        messageResponse.append(user);
        for(int i=1;i<=messageWords.end()->first;i++){
            messageResponse.append(messageWords[i]);
        }

        for(map<string,int>::iterator it=listUsers.begin();it!=listUsers.end();++it){
            if(it->second==connectFD) continue;
            n=write(it->second,messageResponse.c_str(),messageResponse.length());
        }
        return messageResponse;

    }
    if(inputCode=="i"){
        user="";
        messageResponse="I";
        messageResponse.append(2-(to_string(listUsers.size()).length()),'0');
        messageResponse.append(to_string(listUsers.size()));
        for(map<string,int>::iterator it=listUsers.begin();it!=listUsers.end();++it){
            int user_length=it->first.length();
            messageResponse.append(2-(to_string(user_length).length()),'0');
            messageResponse.append(to_string(it->first.length()));
            user.append(it->first);
        }
        messageResponse.append(user);
        return messageResponse;
    }
    if(inputCode=="u"){
        user=findUser(connectFD);
        messageResponse="U";
         //rellenar mensaje
        messageResponse.append(2-(to_string(parameters.end()->first)).length(),'0');
        messageResponse.append(to_string(parameters.end()->first));
        //cambiar user
        int user_length=user.length();
        messageResponse.append(2-(to_string(user_length)).length(),'0');
        messageResponse.append(to_string(user_length)); 
        for(int i=2;i<=parameters.end()->first;i++){
            messageResponse.append(2-(to_string(parameters[i]).length()),'0');
            messageResponse.append(to_string(parameters[i]));
        }
        messageResponse.append(user);
        for(int i=2;i<=messageWords.end()->first;i++){
            messageResponse.append(messageWords[i]);
        }
        user=listUsers.find(messageWords.find(1)->second)->first;    
        n=write(listUsers[user],messageResponse.c_str(),messageResponse.size());
        return "0File Send";
    }
    if(inputCode=="f"){
        user=findUser(connectFD);
        messageResponse="F";
        messageResponse.append("01");

        int user_length=user.length();
        messageResponse.append(2-(to_string(user_length)).length(),'0');
        messageResponse.append(to_string(user_length)); 
        messageResponse.append(user);
        n=write(listUsers[messageWords[1]],messageResponse.c_str(),messageResponse.size());
        return "0file_AN enviado";

    }
    lock_guard<mutex> guard(mutexListUsers);
    if(UserAttemps[connectFD]++>5){
        return "X";
    }

    return "0Fallo";

}


void threadConnection(int ConnectFD){
    int n;
    
    string message_server="I got your message";
    size_t message_server_size=message_server.length();
    string message_client(2000,0);
    bool exitCondition=true;
    do{
        bzero(&message_client[0],2000 );
        n = read(ConnectFD, &message_client[0],2000);
 
        cout<<"Message Client: "<<message_client<<endl;
        message_server=messageParser(message_client,ConnectFD); 
        message_server_size=message_server.length();
        cout<<"Status Acction: "<<message_server<<endl;
        n = write(ConnectFD,message_server.c_str(),message_server_size);
        if(message_server=="X") exitCondition=false;
    }while(exitCondition);
    shutdown(ConnectFD, SHUT_RDWR);
        /*perform read write operations ...*/ 
    close(ConnectFD);
    
}


int main(int argc, char** argv){
    int port;
    stringstream geek(argv[1]);
    geek>>port;
   
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    char buffer[256];
    

    if(-1 == SocketFD){
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))){
        perror("error bind failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if(-1 == listen(SocketFD, 10)){
        perror("error listen failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    for(;;){
        int ConnectFD = accept(SocketFD, NULL, NULL);
        thread(threadConnection,ConnectFD).detach();

    //add loop

    }
    cout<<endl<<"end?"<<endl;
    close(SocketFD);
    return 0;
}
