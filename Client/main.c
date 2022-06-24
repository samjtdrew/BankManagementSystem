#include <winsock2.h>
#include <stdio.h>
#include <string.h>

//Linking with windows socket libraries
#pragma comment(lib, "ws2_32.lib")

char hexKey[100];
char userClass[100];
char inusername[30];
char homeopt[250] = "Options:\n0 - Login\n1 - Terminate Program\n";

void header() {
    //Simple ASCII art header
    printf("* * * * * * * * *\n");
    printf("*  Welcome To   *\n");
    printf("*  The Itergen  *\n");
    printf("*    Banking    *\n");
    printf("*  Management   *\n");
    printf("*    System     *\n");
    printf("* * * * * * * * *\n\n");
}

void createSocket(SOCKET *newsock) {
    *newsock = socket(AF_INET, SOCK_STREAM, 0);
    if (*newsock == INVALID_SOCKET) {
        printf("Socket Error >> Please Check Firewall Settings\n");
    }
}

void connectServer(SOCKET *targetsock) {
    int iResult;
    struct sockaddr_in server;
    server.sin_addr.s_addr = inet_addr("86.15.126.152");
    server.sin_family = AF_INET;
    server.sin_port= htons(30003);
    iResult = connect(*targetsock, (const struct sockaddr *) &server, sizeof(server));
    if (iResult == SOCKET_ERROR) {
        closesocket(*targetsock);
        *targetsock = INVALID_SOCKET;
        printf("Socket Error >> Please Check Firewall Settings\n");
    }
}

//Runs On Startup
int __cdecl main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Socket Error >> Please Check Firewall Settings\n");
    }
    header();
    char mainMenu;
    int loop = 1, userstatus = 0;
    do {
        printf("%s", homeopt);
        scanf_s("%c", &mainMenu, 1);
        getchar(); //Consumes '\n' that may cause formatting errors
        switch (mainMenu) {
            case '0':
                //LOGIN
                if (userstatus == 0) {
                    //Take username and password
                    printf("\n--Login--\n");
                    char password[30];
                    printf("Username -> ");
                    fgets(inusername, sizeof(inusername), stdin);
                    //Strip trailing newline
                    inusername[strcspn(inusername, "\n")] = '\0';
                    printf("Password -> ");
                    fgets(password, sizeof(password), stdin);
                    //Strip trailing newline
                    password[strcspn(password, "\n")] = '\0';
                    //Connecting a windows socket to server
                    SOCKET sock = INVALID_SOCKET;
                    char message[1024], server_reply[2000], *sendmsg;
                    createSocket(&sock);
                    connectServer(&sock);
                    //Send data to server
                    memset(server_reply, '\0', 1);
                    message[0] = '\0';
                    strcpy(message, "LOGIN<>");
                    strcat(message, inusername);
                    strcat(message, ":");
                    strcat(message, password);
                    sendmsg = message;
                    if (send(sock, message, strlen(sendmsg), 0) < 0){
                        printf("Socket Error >> Failed To Send Data\n");
                        break;
                    }
                    //Catch the reply from server
                    if (recv(sock, server_reply, 2000, 0) == SOCKET_ERROR) {
                        printf("Socket Error >> Failed To Receive Data\n");
                        break;
                    }
                    //Store hexKey
                    memset(hexKey, '\0', 1);
                    strncpy(hexKey, server_reply, 39);
                    memset(userClass, '\0', 1);
                    strncpy(userClass, &server_reply[40], 6);
                    //Close socket
                    closesocket(sock);
                    //Edit the home-screen options
                    homeopt[0] = '\0';
                    if (strcmp(userClass, "classV") == 0) {
                        strcpy(homeopt, "Options:\n0 - Logout\n1 - Terminate Program\n2 - Display Balance\n"
                                        "3 - Update Balance\n");
                    } else if (strcmp(userClass, "classI") == 0) {
                        strcpy(homeopt, "Options:\n0 - Logout\n1 - Terminate Program\n2 - Display Balance\n"
                                        "3 - Update Balance\n4 - Create New User\n5 - System Shutdown\n");
                    } else {
                        printf("\nUsername Or Password Was Incorrect, Please Try Again...\n\n");
                        strcpy(homeopt, "Options:\n0 - Login\n1 - Terminate Program\n");
                        break;
                    }
                    //Tell the client that the user is logged in
                    userstatus = 1;
                    printf("\nLogged In As %s\n\n", inusername);
                } else {
                    //LOGOUT
                    SOCKET sock = INVALID_SOCKET;
                    char message[1024], server_reply[20], *sendmsg;
                    message[0] = '\0';
                    server_reply[0] = '\0';
                    //Start the windows socket
                    createSocket(&sock);
                    connectServer(&sock);
                    //Send data to server
                    strcpy(message, "HEXKEY<>");
                    strncat(message, hexKey, 39);
                    strcat(message, "|LOGOUT");
                    sendmsg = message;
                    if (send(sock, message, strlen(sendmsg), 0) < 0) {
                        printf("Socket Error >> Failed To Send Data\n");
                        break;
                    }
                    //Catch the reply from server
                    if (recv(sock, server_reply, 20, 0) == SOCKET_ERROR) {
                        printf("Socket Error >> Failed To Receive Data\n");
                        break;
                    }
                    if (memcmp(server_reply, "Session Terminated", 18) == 0) {
                        userstatus = 0;
                        printf("\nLogged Out...\n\n");
                        homeopt[0] = '\0';
                        strcpy(homeopt, "Options:\n0 - Login\n1 - Terminate Program\n");
                    } else {
                        printf("Error Logging Out, Please Try Again...\n");
                    }
                    closesocket(sock);
                }
                break;
            case '1':
                //Terminate Program
                loop = 0;
                break;
            case '2':
                //Display Balance
                if (userstatus == 1) {
                    SOCKET sock = INVALID_SOCKET;
                    createSocket(&sock);
                    connectServer(&sock);
                    char message[1024], server_reply[2000], balance[25], *sendmsg;
                    //Send data to server
                    memset(server_reply, '\0', 1);
                    message[0] = '\0';
                    strcpy(message, "HEXKEY<>");
                    strcat(message, hexKey);
                    strcat(message, "|FETCHBAL<>");
                    strcat(message, inusername);
                    strcat(message, ":");
                    strcat(message, userClass);
                    sendmsg = message;
                    if (send(sock, message, strlen(sendmsg), 0) < 0){
                        printf("Socket Error >> Failed To Send Data\n");
                        break;
                    }
                    //Catch the reply from server
                    if (recv(sock, server_reply, 2000, 0) == SOCKET_ERROR) {
                        printf("Socket Error >> Failed To Receive Data\n");
                        break;
                    }
                    strcpy(balance, server_reply);
                    closesocket(sock);
                    printf("\nYour Balance Is >> %s\n\n", balance);
                } else {
                    printf("\n[ERR] >> Not A Valid Option\n");
                }
                break;
            case '3':
                //Update Balance
                if (userstatus == 1) {
                    SOCKET sock = INVALID_SOCKET;
                    createSocket(&sock);
                    connectServer(&sock);
                    char message[1024], newbalance[25], *sendmsg;
                    printf("\nPlease Enter The New Balance: ");
                    fgets(newbalance, sizeof(newbalance), stdin);
                    //Strip trailing newline
                    newbalance[strcspn(newbalance, "\n")] = '\0';
                    //Send data to server
                    message[0] = '\0';
                    strcpy(message, "HEXKEY<>");
                    strcat(message, hexKey);
                    strcat(message, "|UPDATEBAL<>");
                    strcat(message, inusername);
                    strcat(message, ":");
                    strcat(message, userClass);
                    strcat(message, ":");
                    strcat(message, newbalance);
                    sendmsg = message;
                    if (send(sock, message, strlen(sendmsg), 0) < 0){
                        printf("Socket Error >> Failed To Send Data\n");
                        break;
                    }
                    closesocket(sock);
                    printf("Your Balance Has Been Updated To >> %s\n\n", newbalance);
                } else {
                    printf("\n[ERR] >> Not A Valid Option\n");
                }
                break;
            case '4':
                //Create New User
                if (userstatus == 1) {
                    if (strcmp(userClass, "classI") == 0) {
                        printf("\n--User Creation Wizard--\n");
                        SOCKET sock = INVALID_SOCKET;
                        createSocket(&sock);
                        connectServer(&sock);
                        char message[1024], newusername[15], newpassword[30], newuserclass[7], *sendmsg;
                        printf("New Username -> ");
                        fgets(newusername, sizeof(newusername), stdin);
                        //Strip trailing newline
                        newusername[strcspn(newusername, "\n")] = '\0';
                        printf("Password Of New User -> ");
                        fgets(newpassword, sizeof(newpassword), stdin);
                        //Strip trailing newline
                        newpassword[strcspn(newpassword, "\n")] = '\0';
                        printf("Class Of New User -> ");
                        fgets(newuserclass, sizeof(newuserclass), stdin);
                        //Strip trailing newline
                        newuserclass[strcspn(newuserclass, "\n")] = '\0';
                        //Send data to server
                        message[0] = '\0';
                        strcpy(message, "HEXKEY<>");
                        strcat(message, hexKey);
                        strcat(message, "|SIGNUP<>");
                        strcat(message, newusername);
                        strcat(message, ":");
                        strcat(message, newpassword);
                        strcat(message, ":");
                        strcat(message, newuserclass);
                        sendmsg = message;
                        if (send(sock, message, strlen(sendmsg), 0) < 0){
                            printf("Socket Error >> Failed To Send Data\n");
                            break;
                        }
                        closesocket(sock);
                        printf("\nNew User Created >> %s\n\n", newusername);
                        break;
                    } else {
                        printf("\n[ERR] >> Not A Valid Option\n");
                        break;
                    }
                } else {
                    printf("\n[ERR] >> Not A Valid Option\n");
                    break;
                }
            case '5':
                //System Shutdown
                if (userstatus == 1) {
                    if (strcmp(userClass, "classI") == 0) {
                        char confirm;
                        printf("Initiating A Total System Shutdown Will Severely Disrupt The Application Network\n"
                               "Are You Sure You Would Like To Do This? (y/n) >> ");
                        scanf("%c", &confirm);
                        if (confirm == *"y") {
                            SOCKET sock = INVALID_SOCKET;
                            createSocket(&sock);
                            connectServer(&sock);
                            char message[1024], *sendmsg;
                            message[0] = '\0';
                            strcpy(message, "HEXKEY<>");
                            strcat(message, hexKey);
                            strcat(message, "|TERMINATE");
                            sendmsg = message;
                            if (send(sock, message, strlen(sendmsg), 0) < 0){
                                printf("Socket Error >> Failed To Send Data\n");
                            }
                            printf("\nNetwork Terminated\n\n");
                            closesocket(sock);
                            break;
                        }
                        printf("\n\nAborting Shutdown...\n\n");
                        break;
                    } else {
                        printf("\n[ERR] >> Not A Valid Option\n");
                    }
                } else {
                    printf("\n[ERR] >> Not A Valid Option\n");
                }
                break;
            default:
                printf("\n[ERR] >> Not A Valid Option\n");
        }
    } while (loop == 1);
    printf("\nExiting...\n");
    if (userstatus == 1) {
        //LOGOUT
        SOCKET sock = INVALID_SOCKET;
        char message[1024], server_reply[20], *sendmsg;
        message[0] = '\0';
        server_reply[0] = '\0';
        //Start the windows socket
        createSocket(&sock);
        connectServer(&sock);
        //Send data to server
        strcpy(message, "HEXKEY<>");
        strncat(message, hexKey, 39);
        strcat(message, "|LOGOUT");
        sendmsg = message;
        if (send(sock, message, strlen(sendmsg), 0) < 0) {
            printf("Socket Error >> Failed To Send Data\n");
        }
        //Catch the reply from server
        if (recv(sock, server_reply, 20, 0) == SOCKET_ERROR) {
            printf("Socket Error >> Failed To Receive Data\n");
        }
        if (memcmp(server_reply, "Session Terminated", 18) == 0) {
            userstatus = 0;
            printf("\nLogged Out...\n\n");
        } else {
            printf("Error Logging Out, Please Try Again...\n");
        }
        closesocket(sock);
    }
    WSACleanup();
    return 0;
}
