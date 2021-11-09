//Autor: Daniel Paulovic <xpaulo04>
//TODO fix something with escaping in arguments
//TODO vypis response
//TODO! redo code from man
//TODO doc

#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

//1 to enable debug messages, 0 to turn them off
#define DEBUG 1

//exit codes
#define ERROR_SUCCESS    0
#define ERROR_FAILURE    1
#define ERROR_PARAMS     2

//default settings
#define DEFAULT_PORT "32323"
#define DEFAULT_IP "127.0.0.1"

#define MAXDATASIZE 1024
#define MAXSIZEPASSWORD 1000

typedef struct args{
    string ip;
    string port;
    string command;
    string arg1;
    string arg2;
    string arg3;
} Args;

int parse_arguents(int argc, char* argv[], Args *args)
{
    //set default values
    args->ip = DEFAULT_IP;
    args->port = DEFAULT_PORT;
    args->command = args->arg1 = args->arg2 = args->arg3 = "";

    int c;

    while(1)
    {
        static struct option long_options[] = 
        {
            {"address", required_argument,  0,  'a'},
            {"help",    no_argument,        0,  'h'},
            {"port",    required_argument,  0,  'p'},
            {0, 0, 0, 0}
        };
        int option_index = 0;
        c = getopt_long (argc, argv, "a:hp:", long_options, &option_index);
        if(c == -1) break;  //end of options

        switch (c)
        {
            case 0:
                if(long_options[option_index].flag != 0)
                {
                    break;
                }
                printf ("option %s", long_options[option_index].name);
                if (optarg) printf (" with arg %s", optarg);
                printf ("\n");
                break;
            case 'a':
                args->ip = optarg;
                break;
            case 'h':
                cout << "HELP INFO" << endl;
                exit(0);
                break;
            case 'p':
                args->port = optarg;
                break;
        }
    }

    //commands handling
    if (optind < argc) {
        while (optind < argc)
        {
            string tmpstr(argv[optind]);
            if(tmpstr == "register"){
                if(args->command != "")
                {
                    cerr << "ERROR: Pokus o zadanie viac commandov." << endl;
                    return ERROR_PARAMS;
                }
                args->command = "register";
            }
            else if(tmpstr == "login")
            {
                if(args->command != "")
                {
                    cerr << "ERROR: Pokus o zadanie viac commandov." << endl;
                    return ERROR_PARAMS;
                }
                args->command = "login";
            }
            else if(tmpstr == "list")
            {
                if(args->command != "")
                {
                    cerr << "ERROR: Pokus o zadanie viac commandov." << endl;
                    return ERROR_PARAMS;
                }
                args->command = "list";
            }
            else if(tmpstr == "send")
            {
                if(args->command != "")
                {
                    cerr << "ERROR: Pokus o zadanie viac commandov." << endl;
                    return ERROR_PARAMS;
                }
                args->command = "send";
            }
            else if(tmpstr == "fetch")
            {
                if(args->command != "")
                {
                    cerr << "ERROR: Pokus o zadanie viac commandov." << endl;
                    return ERROR_PARAMS;
                }
                args->command = "fetch";
            }
            else if(tmpstr == "logout")
            {
                if(args->command != "")
                {
                    cerr << "ERROR: Pokus o zadanie viac commandov." << endl;
                    return ERROR_PARAMS;
                }
                args->command = "logout";
            }
            else    //should be argument
            {
                if(args->command == "")
                {
                    cerr << "ERROR: Neznamy command." << endl;
                    return ERROR_PARAMS;
                }
                if(args->arg1 == "")
                {
                    args->arg1 = tmpstr;
                }
                else if(args->arg2 == "")
                {
                    args->arg2 = tmpstr;
                }
                else if(args->arg3 == "")
                {
                    args->arg3 = tmpstr;
                }
                else
                {
                    cerr << "ERROR: Prilis vela argumentov." << endl;
                    return ERROR_PARAMS;
                }
            }
            optind++;
        }
    }

    //check if we have good amount of arguments
    if(args->command == "")
    {
        cerr << "ERROR: Ocakavany vstup: <command> [<args>] ... bolo zadanych 0 argumentov." << endl;
        return ERROR_PARAMS;
    }

    if(args->command == "register")
    {
        if((args->arg1 == "") || (args->arg2 == "") || (args->arg3 != ""))
        {
            cerr << "ERROR: Nespravne zadanie prikazu register, ocakavane je register <username> <password>." << endl;
            return ERROR_PARAMS;
        }
    }

    if(args->command == "login")
    {
        if((args->arg1 == "") || (args->arg2 == "") || (args->arg3 != ""))
        {
            cerr << "ERROR: Nespravne zadanie prikazu login, ocakavane je login <username> <password>." << endl;
            return ERROR_PARAMS;
        }
    }

    if(args->command == "list")
    {
        if((args->arg1 != "") || (args->arg2 != "") || (args->arg3 != ""))
        {
            cerr << "ERROR: Nespravne zadanie prikazu list, ocakavane je list." << endl;
            return ERROR_PARAMS;
        }
    }

    if(args->command == "send")
    {
        if((args->arg1 == "") || (args->arg2 == "") || (args->arg3 == ""))
        {
            cerr << "ERROR: Nespravne zadanie prikazu send, ocakavane je send <recipient> <subject> <body>." << endl;
            return ERROR_PARAMS;
        }
    }

    if(args->command == "fetch")
    {
        if((args->arg1 == "") || (args->arg2 != "") || (args->arg3 != ""))
        {
            cerr << "ERROR: Nespravne zadanie prikazu fetch, ocakavane je fetch <id>." << endl;
            return ERROR_PARAMS;
        }
    }

    if(args->command == "logout")
    {
        if((args->arg1 != "") || (args->arg2 != "") || (args->arg3 != ""))
        {
            cerr << "ERROR: Nespravne zadanie prikazu logout, ocakavane je logout." << endl;
            return ERROR_PARAMS;
        }
    }

    //TODO overit validnost IP adresy a portu

    //vypis informacii o argumentoch v debug mode
    if(DEBUG)
    {
        cout << "IP: \"" << args->ip << "\"" << endl;
        cout << "PORT: \"" << args->port << "\"" << endl;
        cout << "COMMAND: \"" << args->command << "\"" << endl;
        cout << "ARG1: \"" << args->arg1 << "\"" << endl;
        cout << "ARG2: \"" << args->arg2 << "\"" << endl;
        cout << "ARG3: \"" << args->arg3 << "\"" << endl;
    }

    return ERROR_SUCCESS;
}

//TODO MONSTROCITY https://www.geeksforgeeks.org/encode-ascii-string-base-64-format/
string encode_base64(string password)
{
    char base64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char *result = (char *) malloc(MAXSIZEPASSWORD * sizeof(char));

    int index = 0;
    int val = 0;
    int count = 0;
    int temp = 0;
    int no_of_bits = 0;
    int padding = 0;

    int i, j, k = 0;

    int length = password.length();

    for (i = 0; i < length; i += 3)
    {
        val = 0, count = 0, no_of_bits = 0;
        for(j = i; j < length && j <= i + 2; j++)
        {
            val = val << 8;
            val = val | password.c_str()[j];
            count++;
        }
        no_of_bits = count * 8;
        padding = no_of_bits % 3;

        while(no_of_bits != 0)
        {
            if(no_of_bits >= 6)
            {
                temp = no_of_bits - 6;
                index = (val >> temp) & 63;
                no_of_bits -= 6;
            }
            else
            {
                temp = 6 - no_of_bits;
                index = (val << temp) & 63;
                no_of_bits = 0;
            }
            result[k++] = base64chars[index];
        }
    }
    for(i =1; i <= padding; i++)
    {
        result[k++] = '=';
    }
    result[k] = '\0';
    cout << string(result) << endl;
    return string(result);
}

// get sockaddr, IPv4 or IPv6: //TODO
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

//loads content of login-token file
string load_login_token()
{
    ifstream tokenfile("login-token");
    //pokial sa subor podarilo otvorit vrat string
    if(tokenfile.is_open())
    {
        return string((istreambuf_iterator<char>(tokenfile)), istreambuf_iterator<char>());
    }
    else
    {
        cerr << "ERROR: Subor 'login-token' sa nepodarilo otvorit." << endl;
        exit(ERROR_FAILURE);
        return "";
    }
}

// saves login_token to tokin-login file
int save_login_token(string login_token)
{
    ofstream out("login-token");
    out << login_token;
    out.close();
    return ERROR_SUCCESS;
}

//deletes login-token file
int delete_login_token()
{
    if(remove("login-token") != ERROR_SUCCESS)
    {
        cerr << "ERROR: Zmazanie login-token nebolo uspesne." << endl;
        return ERROR_FAILURE;
    }
    else
    {
        return ERROR_SUCCESS;
    }
}

int main(int argc, char *argv[])
{
    Args* args = new Args;
    if(parse_arguents(argc, argv, args)) return ERROR_PARAMS;
    string message = "";
    string login_token = "";

    //create message string based on command and args
    if(args->command == "register")
    {
        message = string("(") + "register " + "\"" + args->arg1 + "\" " + "\"" + args->arg2 + "\"" + ")";
    }
    else if(args->command == "login")
    {
        message = string("(") + "login " + "\"" + args->arg1 + "\" " + "\"" + args->arg2 + "\"" + ")";
    }
    else if(args->command == "list")
    {
        login_token = load_login_token();
        message = string("(") + "list " + "\"" + login_token + "\"" +  ")";
    }
    else if(args->command == "send")
    {
        login_token = load_login_token();
        message = string("(") + "send " + "\"" + login_token + "\" " + "\"" + args->arg1 + "\" " + "\"" + args->arg2 + "\" " + "\"" + args->arg3 + "\"" + ")";
    }
    else if(args->command == "fetch")
    {
        login_token = load_login_token();
        message = string("(") + "fetch " + "\"" + login_token + "\" " + args->arg1 + ")";
    }
    else if(args->command == "logout")
    {
        login_token = load_login_token();
        message = string("(") + "logout " + "\"" + login_token + "\"" +  ")";
    }

    //TODO rework
    //socket settings
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(args->ip.c_str(), args->port.c_str(), &hints, &servinfo)) != 0)
    {
        cerr << "getaddrinfo: " << gai_strerror(rv) << endl;
        return 1;
    }
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    if(DEBUG)printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure

    //sends our message to server
    write(sockfd, message.c_str(), message.length());

    //TODO rework
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1)
    {
        perror("recv");
        exit(1);
    }
    buf[numbytes] = '\0';   //adding \0 to end of the buffer
    close(sockfd);  //close socket

    if(DEBUG)printf("client: received '%s'\n", buf);

    // (ok "something")
    // (err "something")
    string result = string(buf);

    if(DEBUG) cout << result << endl;

    string tmp = "";
    int index = 1; //start after (

    //CHECK status
    while(result[index] != '\"')
    {
        if(DEBUG) cout << result[index] << endl;
        tmp += result[index++];
    }

    if(DEBUG)cout << tmp << endl;
    int result_code;
    if(tmp == "ok ") result_code = 0;
    if(tmp == "err ") result_code = 1;
    tmp = "";
    index++;
    int flag_check = 0;

    while((result[index] != '\"') && (flag_check == 0))
    {
        if(DEBUG) cout << result[index];
        if(result[index] == '\\')
        {
            flag_check = 1;
        }
        else
        {
            flag_check = 0;
        }
        tmp += result[index++];
    }
    if(DEBUG)cout << tmp << endl;

    //if((args->command == "login") && (result == "ok"))

    return EXIT_SUCCESS;
}
