#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <time.h>
#include "ll.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 8421

//dll stuff
typedef void (*CanonicalizeFunc)(char*, char*);
typedef unsigned int (*Rotl32Func)(unsigned int, int);
typedef unsigned int (*NextHashFunc)(unsigned int, const unsigned char*, int);

HMODULE hDLL = NULL;
CanonicalizeFunc canonicalize = NULL;
Rotl32Func rotl32 = NULL;
NextHashFunc nextHash = NULL;

struct threadArgs {
    SOCKET* clientConn;
    struct LinkedList* CommandHistory;
};

int loadDLL() {
    hDLL = LoadLibrary(L"BlockchainDLL.dll");
    if (hDLL == NULL) {
        printf("ERROR: cant load BlockchainDLL.dll\n");
        return 0;
    }

    canonicalize = (CanonicalizeFunc)GetProcAddress(hDLL, "canonicalize");
    rotl32 = (Rotl32Func)GetProcAddress(hDLL, "rotl32");
    nextHash = (NextHashFunc)GetProcAddress(hDLL, "nextHash");

    if (canonicalize == NULL || rotl32 == NULL || nextHash == NULL) {
        printf("ERROR: cant find DLL functions\n");
        FreeLibrary(hDLL);
        return 0;
    }

    printf("DLL loaded\n");
    return 1;
}


// checks if input is valid ascii characters
// need this for the assignment
int checkInput(char* input) {
    char buf[64];
    int i;
    int len = strlen(input);
    
    //copy to buffer to check
    strcpy(buf, input);
    
    for (i = 0; i < len; i++) {
        // check printable ascii range
        if ((buf[i] < 0x20 || buf[i] > 0x7E) && buf[i] != '\n' && buf[i] != '\r') {
            return 0;
        }
    }
    return 1;
}

// parse command and return result code
// 1 = good, 0 = quit, negative = error
int parseCommand(char* rawCommand, struct LinkedList* hist, SOCKET* clientSock) {
    char temp[256];
    char* first;
    char* second; 
    char* third;
    char resp[2048];
    SOCKET sock = *clientSock;
    int len;
    
    //check input first
    if (!checkInput(rawCommand)) {
        return -6;
    }
    
    // get rid of newline
    len = strlen(rawCommand);
    while (len > 0 && (rawCommand[len-1] == '\n' || rawCommand[len-1] == '\r')) {
        rawCommand[len-1] = '\0';
        len--;
    }
    
    strcpy(temp, rawCommand);
    
    first = strtok(temp, " ");
    second = strtok(NULL, " ");
    third = strtok(NULL, " ");
    
    if (first == NULL) {
        return -5;
    }
    
    //quit
    if (strcmp(first, "quit") == 0) {
        sprintf(resp, "Goodbye\n");
        send(sock, resp, strlen(resp), 0);
        return 0;
    }
    
    //history
    if (strcmp(first, "history") == 0) {
        if (second != NULL) {
            return -4;
        }
        printHistory(hist, clientSock);
        return 1;
    }
    
    //validate 
    if (strcmp(first, "validate") == 0) {
        if (second != NULL) {
            return -4;
        }
        if (validateBlockchain(hist, clientSock)) {
            sprintf(resp, "SUCCESS> blockchain ok\n");
        } else {
            sprintf(resp, "ERROR> blockchain failed check\n");
        }
        send(sock, resp, strlen(resp), 0);
        return 1;
    }
    
    //upload
    if (strcmp(first, "upload") == 0) {
        if (second == NULL || third == NULL) {
            return -1;
        }
        sprintf(resp, "SUCCESS> upload %s %s\n", second, third);
        send(sock, resp, strlen(resp), 0);
        return 1;
    }
    
    //download
    if (strcmp(first, "download") == 0) {
        if (second == NULL || third == NULL) {
            return -1;
        }
        sprintf(resp, "SUCCESS> download %s %s\n", second, third);
        send(sock, resp, strlen(resp), 0);
        return 1;
    }
    
    // delete
    if (strcmp(first, "delete") == 0) {
        if (second == NULL || third == NULL) {
            return -1;
        }
        if (strcmp(second, "local") != 0 && strcmp(second, "remote") != 0) {
            return -2;
        }
        sprintf(resp, "SUCCESS> delete %s %s\n", second, third);
        send(sock, resp, strlen(resp), 0);
        return 1;
    }
    
    //change
    if (strcmp(first, "change") == 0) {
        if (second == NULL || third == NULL) {
            return -1; 
        }
        if (strcmp(second, "local") != 0 && strcmp(second, "remote") != 0) {
            return -2;
        }
        sprintf(resp, "SUCCESS> change %s %s\n", second, third);
        send(sock, resp, strlen(resp), 0);
        return 1;
    }
    
    //show
    if (strcmp(first, "show") == 0) {
        if (second == NULL || third == NULL) {
            return -1;
        }
        if (strcmp(second, "local") != 0 && strcmp(second, "remote") != 0) {
            return -2;
        }
        if (strcmp(third, "path") != 0 && strcmp(third, "files") != 0 && strcmp(third, "folders") != 0) {
            return -3;
        }
        sprintf(resp, "SUCCESS> show %s %s\n", second, third);
        send(sock, resp, strlen(resp), 0);
        return 1;
    }
    
    return -5; //invalid cmd
}

int __stdcall HandleConnection(struct threadArgs* args) {
    SOCKET clientConn = *(args->clientConn);
    struct LinkedList* hist = args->CommandHistory;
    int bytesRead;
    char* rawCmd;
    int result;
    
    char banner[] = "WELCOME TO FML SERVER\n";
    char prompt[] = "FML> ";
    char err1[] = "ERROR> command takes 2 arguments\n";
    char err2[] = "ERROR> 1st arg must be 'local' or 'remote'\n";
    char err3[] = "ERROR> 2nd arg must be 'files', 'path', or 'folders'\n";
    char err4[] = "ERROR> command takes no arguments\n";
    char err5[] = "ERROR> invalid command\n";
    char err6[] = "ERROR> invalid character\n";
    char errOther[] = "ERROR> something went wrong\n";
    
    rawCmd = (char*)malloc(sizeof(char) * 1000);
    
    send(clientConn, banner, sizeof(banner), 0);
    
    while (1) {
        //printf("sending prompt\n");
        send(clientConn, prompt, sizeof(prompt), 0);
        //printf("waiting...\n");
        
        bytesRead = recv(clientConn, rawCmd, 1000, 0);
        if (bytesRead == -1) break;
        rawCmd[bytesRead] = '\0';
        printf("got: %s\n", rawCmd);
        
        result = parseCommand(rawCmd, hist, &clientConn);
        
        if (result == 1) {
            addCommand(hist, rawCmd);
            printf("added to history: %s\n", rawCmd);
        }
        else if (result == 0) {
            printf("client quit\n");
            break;
        }
        else if (result == -1) {
            send(clientConn, err1, sizeof(err1), 0);
        }
        else if (result == -2) {
            send(clientConn, err2, sizeof(err2), 0);
        }
        else if (result == -3) {
            send(clientConn, err3, sizeof(err3), 0);
        }
        else if (result == -4) {
            send(clientConn, err4, sizeof(err4), 0);
        }
        else if (result == -5) {
            send(clientConn, err5, sizeof(err5), 0);
        }
        else if (result == -6) {
            send(clientConn, err6, sizeof(err6), 0);
        }
        else {
            send(clientConn, errOther, sizeof(errOther), 0);
        }
    }
    
    free(rawCmd);
    closesocket(clientConn);
    _endthreadex(0);
    return 0;
}

void BeginServer() {
    WSADATA wsa;
    SOCKET serverSock, clientSock;
    struct sockaddr_in serverAddr, clientAddr;
    int addrLen = sizeof(clientAddr);
    HANDLE thread;
    int res;
    int connNum = 0;
    
    struct LinkedList* hist = (struct LinkedList*)malloc(sizeof(struct LinkedList));
    initialize(hist);
    
    if (!loadDLL()) {
        printf("dll failed\n");
        return;
    }
    
    res = WSAStartup(MAKEWORD(2, 2), &wsa);
    if (res != 0) {
        printf("winsock failed\n");
        exit(-1);
    }
    
    serverSock = socket(AF_INET, SOCK_STREAM, 0);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    
    bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSock, 5);
    
    printf("server running on port %d\n", PORT);
    
    while (1) {
        clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &addrLen);
        
        struct threadArgs* args = (struct threadArgs*)malloc(sizeof(struct threadArgs));
        args->clientConn = &clientSock;
        args->CommandHistory = hist;
        connNum++;
        printf("connection %d\n", connNum);
        thread = (HANDLE)_beginthreadex(NULL, 0, (unsigned int(__stdcall*)(void*))HandleConnection, args, 0, NULL);
    }
}

int main(void) {
    BeginServer();
    return 0;
}