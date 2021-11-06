//Autor: Daniel Paulovic <xpaulo04>

#include <string>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <getopt.h>

using namespace std;

//1 to enable debug messages, 0 to turn them off
#define DEBUG 1

//exit codes
#define ERROR_SUCCESS    0
#define ERROR_FAILURE    1
#define ERROR_PARAMS     2

//default settings
#define DEFAULT_PORT 32323
#define DEFAULT_IP "127.0.0.1"

    typedef struct args{
        string ip;
        int port;
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
                    args->port = atoi(optarg);
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

    // int encrypt()
    // {
    //     return ERROR_SUCCESS;
    // }
    
    // int reg(string user, string password)
    // {
    //     return ERROR_SUCCESS;
    // }

    // int login(string user, string password)
    // {
    //     return ERROR_SUCCESS;
    // }

    // int list()
    // {
    //     return ERROR_SUCCESS;
    // }

    // int send(string recipient, string subject, string body)
    // {
    //     return ERROR_SUCCESS;
    // }

    // int fetch(string id)
    // {
    //     return ERROR_SUCCESS;
    // }

    // int logout()
    // {
    //     return ERROR_SUCCESS;
    // }

int main(int argc, char *argv[])
{
    Args* args = new Args;
    if(parse_arguents(argc, argv, args)) return ERROR_PARAMS;

    return ERROR_SUCCESS;
}
