 /* Client code in C++ */
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
#include <vector>
#include <sstream>
#include <fstream>
#include <math.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <atomic>
#include <map>
using namespace std;
mutex mutexListUsers;
pthread_mutex_t lock;

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

string messageParserServer(string message_server){
    string inputCode=message_server.substr(0,1);//codigo caracteres =1
    if(inputCode=="0") return message_server.substr(1);
    message_server=message_server.substr(1);          //input bash sin el caracter de codigo 
    string copybash=message_server;
    map<int,int> parameters=parametersParse(message_server); //parametros parseados
    
    message_server=message_server.substr(2); 
    message_server=message_server.substr((parameters.end()->first)*2); //elimina el header solo deja las palabras
    
    map<int,string> messageWords=wordsParse(parameters,message_server);

    string messageResponse;

    if(inputCode=="L"){
        messageResponse="Login: ";
        messageResponse.append(messageWords[1]);
    }
    if(inputCode=="I"){
        messageResponse="Lista de Usuarios\n";
        for(int i=1;i<=messageWords.end()->first;i++){
            messageResponse.append("-"+messageWords[i]+"\n");
        }
    }
    if(inputCode=="M"){
        messageResponse=messageWords[1];
        messageResponse.append(" te envio un mensaje:");
        for(int i=2;i<=messageWords.end()->first;i++){
            messageResponse.append(" "+messageWords[i]);
        }
    }
    if(inputCode=="B"){
         messageResponse=messageWords[1];
        messageResponse.append(" envio un mensaje broadcast:");
        for(int i=2;i<=messageWords.end()->first;i++){
            messageResponse.append(" "+messageWords[i]);
        }
    }

    return messageResponse;


}



string messageParse(string inputBash){
    if(inputBash.find(" ")>inputBash.size()){
        return "";
    }
    inputBash=inputBash.substr(inputBash.find(" ")+1);
    vector<string> substrMessage;
    size_t pos = inputBash.find( " " );
    size_t initialPos = 0;
    string messageFormat;
    // Descomponer el string por espacios
    while( pos != std::string::npos ) {
        substrMessage.push_back( inputBash.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;

        pos = inputBash.find( " ", initialPos );
    }
    // Add the last one
    substrMessage.push_back( inputBash.substr( initialPos, min( pos, inputBash.size() ) - initialPos + 1 ) );
    messageFormat.append(2-(to_string(substrMessage.size()).length()),'0');
    messageFormat.append(to_string(substrMessage.size()));
    for (vector<string>::const_iterator i = substrMessage.begin(); i != substrMessage.end(); ++i){
        messageFormat.append(2-(to_string((*i).size()).length()),'0');
        messageFormat.append(to_string((*i).size()));
    }
    
    for (vector<string>::const_iterator i = substrMessage.begin(); i != substrMessage.end(); ++i){
        messageFormat.append(*i);
    }

    return messageFormat ;

}

string FileParser(string inputBash){
    string messageFile;
    inputBash=inputBash.substr(inputBash.find(" ")+1);
    string user=inputBash.substr(0,inputBash.find(' '));
    inputBash=inputBash.substr(inputBash.find(" ")+1);
    string name_file=inputBash;
    cout<<"user->"<<user<<" "<<"name_file->"<<name_file<<endl;

    ifstream file{name_file};
    string file_contents = static_cast<ostringstream&>(ostringstream{} << file.rdbuf()).str();
    map<int,int> count_blocks;
    int counter=0;
    int indice=1;
    for (string::iterator it=file_contents.begin(); it!=file_contents.end(); ++it){
        if(counter++==98){
            count_blocks[indice++]=99;
            counter=0;
        }
    }
    if(counter!=0) count_blocks[indice]=counter;
    int parameter=2+count_blocks.end()->first;
    messageFile.append(2-to_string(parameter).length(),'0');
    messageFile.append(to_string(parameter));
    parameter=user.length();
    messageFile.append(2-to_string(parameter).length(),'0');
    messageFile.append(to_string(parameter));
    parameter=name_file.length();
    messageFile.append(2-to_string(parameter).length(),'0');
    messageFile.append(to_string(parameter));
    for(map<int,int>::iterator it=count_blocks.begin();it!=count_blocks.end();++it){
        parameter=it->second;
        messageFile.append(2-to_string(parameter).length(),'0');
        messageFile.append(to_string(parameter));
    }
    messageFile.append(user);
    messageFile.append(name_file);
    messageFile.append(file_contents);
    return messageFile;
}
string inputParser(string inputBash){
    if(inputBash=="exit")
        return "x";
    string messageComplete;
    string inputCode=inputBash.substr(0,inputBash.find(' '));
    string inputMessage;
    if(inputCode=="login"){
        messageComplete.append("l");
    }    
    else if(inputCode=="msg-user")
        messageComplete.append("m");
    else if(inputCode=="msg-bc")
        messageComplete.append("b");
    else if(inputCode=="uploadfile"){
        messageComplete.append("u");
        messageComplete.append( FileParser(inputBash) );
        return messageComplete;
    }
        //uploadfile tinyhos externo.txt
    else if(inputCode=="file_AN")
        messageComplete.append("f");
    else if(inputCode=="list")
        messageComplete.append("i");
    else
        messageComplete.append("e");
    messageComplete.append(messageParse(inputBash)); 
    
    return messageComplete;
}

void thread_write(int n,int SocketFD,atomic<bool>& run,atomic<bool>& run_write){
    string message_client="";
    size_t message_client_size;
    string message_server(256,0);
    string inputBash;
    do{
        inputBash="";
        cout<<"client@Desktop:~$ ";
        run_write.store(false);
        getline(cin,inputBash);
        run_write.store(true);
        if(inputBash=="")
            continue;
        message_client=inputParser(inputBash);
        message_client_size=message_client.length();
        n = write(SocketFD,message_client.c_str(),message_client_size);
        run.store(false);
        while(!run.load())
            std::this_thread::sleep_for(1ms);
        //n=write(SocketFD,"Hi, this is Sanchez",19);
        /* perform read write operations ... */
       
    }while(true);

}

int main(int argc, char** argv){
    int port;
    stringstream geek(argv[1]);
    geek>>port;
   
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n;

    if (-1 == SocketFD){
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    Res = inet_pton(AF_INET,"127.0.0.1", &stSockAddr.sin_addr);

    if (0 > Res){
        perror("error: first parameter is not a valid address family");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }
    else if (0 == Res){
        perror("char string (second parameter does not contain valid ipaddress");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))){
        perror("connect failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    //add message console
    atomic<bool>run(true);
    atomic<bool>run_write(true);

    thread (thread_write,n,SocketFD,ref(run),ref(run_write)).detach();
    while(true){
        string message_server(256,0);    
        n = read(SocketFD, &message_server[0],255);
        if(message_server[0]=='X') {
            cout<<"Cerrando Sesion"<<endl;
            break;
        }
        message_server=messageParserServer(message_server);
        if(n==1)continue;
        run.store(false);

        if(!run_write.load()){
            cout<<"\r-----server say----------"<<endl;
                cout<<message_server<<endl;

        }
        else{
            cout<<"-----server say----------"<<endl;
            cout<<message_server<<endl;
        }
        
        run.store(true);
        
    }
    //string message_client="";
    //size_t message_client_size;
    //string message_server(256,0);
    //string inputBash;
    

    //while(message_client!="0"){
         
    //    cout<<"client@Desktop:~$ ";
    //    getline(cin,inputBash);
    //    message_client=inputParser(inputBash);
    //    message_client_size=message_client.length();
    //    n = write(SocketFD,message_client.c_str(),message_client_size);

    //    n = read(SocketFD, &message_server[0],255);
    //    cout<<"Server say: "<<message_server<<endl;
    //}
    
    //thread(thread_write,n,SocketFD).detach();
    
    close(SocketFD);
    return 0;
}
