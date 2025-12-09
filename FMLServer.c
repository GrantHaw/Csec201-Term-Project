#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <process.h>
#include <winsock2.h>
#include <time.h>
#include "ll.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 8421

// DLL function pointers
typedef void (*CanonicalizeFunc)(char*, char*);
typedef unsigned int (*Rotl32Func)(unsigned int, int);
typedef unsigned int (*NextHashFunc)(unsigned int, const unsigned char*, int);

HMODULE hDLL = NULL;
CanonicalizeFunc canonicalize = NULL;
Rotl32Func rotl32 = NULL;
NextHashFunc nextHash = NULL;

// Thread arguments structure
struct threadArgs {
    SOCKET* clientConn;
    struct LinkedList* CommandHistory;
};

int loadBlockchainDLL() {
    hDLL = LoadLibrary(L"BlockchainDLL.dll");
    if (hDLL == NULL) {
        printf("ERROR: Could not load BlockchainDLL.dll\n");
        return 0;
    }

    canonicalize = (CanonicalizeFunc)GetProcAddress(hDLL, "canonicalize");
    rotl32 = (Rotl32Func)GetProcAddress(hDLL, "rotl32");
    nextHash = (NextHashFunc)GetProcAddress(hDLL, "nextHash");

    if (canonicalize == NULL || rotl32 == NULL || nextHash == NULL) {
        printf("ERROR: Could not find DLL functions\n");
        FreeLibrary(hDLL);
        return 0;
    }

    printf("Blockchain DLL loaded successfully.\n");
    return 1;
}

/*
 * BUFFER OVERFLOW VULNERABILITY
 * 
 * This function checks if all characters in the input are printable ASCII.
 * 
 * VULNERABILITY: The local buffer 'checked' is only 64 bytes, but we copy
 * the entire input string into it without bounds checking. If input exceeds
 * 64 bytes, this overflows the stack buffer.
 */
int isValidInput(char* input) {
    char checked[64];  // vulnerability: small fixed-size buffer
    int i;
    int len = strlen(input);
    
    //strcpy with no bounds checking
    // If input > 64 bytes, this overflows 'checked' buffer
    strcpy(checked, input);
    
    // Check each character is printable ASCII (0x20-0x7E) or newline/carriage return
    for (i = 0; i < len; i++) {
        if ((checked[i] < 0x20 || checked[i] > 0x7E) && 
            checked[i] != '\n' && checked[i] != '\r') {
            return 0;  // Invalid character found
        }
    }
    return 1;  // All characters valid
}

/*
 * Parse and execute a command
 * Returns:
 *   1  = Valid command (add to history)
 *   0  = Quit command
 *  -1  = Command takes 2 arguments error
 *  -2  = First arg must be 'local' or 'remote'
 *  -3  = Second arg error (files/path/folders)
 *  -4  = Command takes no arguments
 *  -5  = Invalid command
 *  -6  = Invalid character detected
 */
int parseCommand(char* rawCommand, struct LinkedList* CommandHistory, SOCKET* clientSock) {
    char workingCopy[256];
    char* first;
    char* second;
    char* third;
    char response[2048];
    SOCKET clientConn = *clientSock;
    
    // Check for valid ASCII input (VULNERABLE FUNCTION)
    if (!isValidInput(rawCommand)) {
        return -6;
    }
    
    // Remove trailing newline/carriage return
    int len = strlen(rawCommand);
    while (len > 0 && (rawCommand[len-1] == '\n' || rawCommand[len-1] == '\r')) {
        rawCommand[len-1] = '\0';
        len--;
    }
    
    // Make working copy for tokenization
    strcpy(workingCopy, rawCommand);
    
    // Tokenize
    first = strtok(workingCopy, " ");
    second = strtok(NULL, " ");
    third = strtok(NULL, " ");
    
    // Empty command
    if (first == NULL) {
        return -5;
    }
    
    // quit
    if (strcmp(first, "quit") == 0) {
        sprintf(response, "Goodbye.\n");
        send(clientConn, response, strlen(response), 0);
        return 0;
    }
    
    // history
    if (strcmp(first, "history") == 0) {
        if (second != NULL) {
            return -4;  // Takes no arguments
        }
        printHistory(CommandHistory, clientSock);
        return 1;
    }
    
    // validate
    if (strcmp(first, "validate") == 0) {
        if (second != NULL) {
            return -4;  // Takes no arguments
        }
        if (validateBlockchain(CommandHistory, clientSock)) {
            sprintf(response, "SUCCESS> Blockchain integrity verified.\n");
        } else {
            sprintf(response, "ERROR> Blockchain integrity check failed.\n");
        }
        send(clientConn, response, strlen(response), 0);
        return 1;
    }
    
    // upload <local> <remote>
    if (strcmp(first, "upload") == 0) {
        if (second == NULL || third == NULL) {
            return -1;  //takes 2 arguments
        }
        sprintf(response, "SUCCESS> upload %s %s\n", second, third);
        send(clientConn, response, strlen(response), 0);
        return 1;
    }
    
    // download <remote> <local>
    if (strcmp(first, "download") == 0) {
        if (second == NULL || third == NULL) {
            return -1;  // Takes 2 arguments
        }
        sprintf(response, "SUCCESS> download %s %s\n", second, third);
        send(clientConn, response, strlen(response), 0);
        return 1;
    }
    
    // delete local|remote <filename>
    if (strcmp(first, "delete") == 0) {
        if (second == NULL || third == NULL) {
            return -1;  // Takes 2 arguments
        }
        if (strcmp(second, "local") != 0 && strcmp(second, "remote") != 0) {
            return -2;  // First arg must be local or remote
        }
        sprintf(response, "SUCCESS> delete %s %s\n", second, third);
        send(clientConn, response, strlen(response), 0);
        return 1;
    }
    
    // change local|remote <filepath>
    if (strcmp(first, "change") == 0) {
        if (second == NULL || third == NULL) {
            return -1;  // Takes 2 arguments
        }
        if (strcmp(second, "local") != 0 && strcmp(second, "remote") != 0) {
            return -2;  // First arg must be local or remote
        }
        sprintf(response, "SUCCESS> change %s %s\n", second, third);
        send(clientConn, response, strlen(response), 0);
        return 1;
    }
    
    // show local|remote path|files|folders
    if (strcmp(first, "show") == 0) {
        if (second == NULL || third == NULL) {
            return -1;  // Takes 2 arguments
        }
        if (strcmp(second, "local") != 0 && strcmp(second, "remote") != 0) {
            return -2;  // First arg must be local or remote
        }
        if (strcmp(third, "path") != 0 && strcmp(third, "files") != 0 && strcmp(third, "folders") != 0) {
            return -3;  // Second arg must be path/files/folders
        }
        sprintf(response, "SUCCESS> show %s %s\n", second, third);
        send(clientConn, response, strlen(response), 0);
        return 1;
    }
    
    // Unknown command
    return -5;
}

int __stdcall HandleConnection(struct threadArgs* args) {
    SOCKET clientConn = *(args->clientConn);
    struct LinkedList* CommandHistory = args->CommandHistory;
    int bytesRead;
    char* rawCommand;
    
    char banner[] = "WELCOME TO FML SERVER!\n";
    char prompt[] = "FML> ";
    char errorTwoArgs[] = "ERROR> command takes 2 arguments\n";
    char errorFirstArg[] = "ERROR> 1st arg must be 'local' or 'remote'\n";
    char errorSecondArg[] = "ERROR> 2nd argument must be 'files', 'path', or 'folders'\n";
    char errorNoArgs[] = "ERROR> command takes no arguments\n";
    char errorInvalidCommand[] = "ERROR> Invalid command\n";
    char errorInvalidChar[] = "ERROR> Invalid character detected\n";
    char errorOther[] = "ERROR> OTHER\n";
    
    int parseResult;
    rawCommand = (char*)malloc(sizeof(char) * 1000);
    
    send(clientConn, banner, sizeof(banner), 0);
    
    while (1) {
        printf("Sending prompt\n");
        send(clientConn, prompt, sizeof(prompt), 0);
        printf("Waiting on command:\n");
        
        bytesRead = recv(clientConn, rawCommand, 1000, 0);
        if (bytesRead == -1) break;
        rawCommand[bytesRead] = '\0';
        printf("Received command: %s\n", rawCommand);
        
        parseResult = parseCommand(rawCommand, CommandHistory, &clientConn);
        
        if (parseResult == 1) {
            addCommand(CommandHistory, rawCommand);
            printf("%s added to history\n", rawCommand);
        }
        else if (parseResult == 0) {
            printf("Terminating Connection\n");
            break;
        }
        else if (parseResult == -1) {
            printf("Sending TwoArgs Error\n");
            send(clientConn, errorTwoArgs, sizeof(errorTwoArgs), 0);
        }
        else if (parseResult == -2) {
            printf("Sending FirstArg Error\n");
            send(clientConn, errorFirstArg, sizeof(errorFirstArg), 0);
        }
        else if (parseResult == -3) {
            printf("Sending SecondArg Error\n");
            send(clientConn, errorSecondArg, sizeof(errorSecondArg), 0);
        }
        else if (parseResult == -4) {
            printf("Sending NoArgs Error\n");
            send(clientConn, errorNoArgs, sizeof(errorNoArgs), 0);
        }
        else if (parseResult == -5) {
            printf("Sending Invalid Command Error\n");
            send(clientConn, errorInvalidCommand, sizeof(errorInvalidCommand), 0);
        }
        else if (parseResult == -6) {
            printf("Sending Invalid Character Error\n");
            send(clientConn, errorInvalidChar, sizeof(errorInvalidChar), 0);
        }
        else {
            send(clientConn, errorOther, sizeof(errorOther), 0);
        }
    }
    
    free(rawCommand);
    closesocket(clientConn);
    _endthreadex(0);
    return 0;
}

void BeginServer() {
    WSADATA winSockData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in serverAddress, clientAddress;
    int addrLen = sizeof(clientAddress);
    HANDLE thread;
    int result;
    
    struct LinkedList* CommandHistory = (struct LinkedList*)malloc(sizeof(struct LinkedList));
    initialize(CommandHistory);
    srand(time(NULL));
    int threadnum = 0;
    
    // Load blockchain DLL
    if (!loadBlockchainDLL()) {
        printf("Failed to load blockchain DLL. Exiting.\n");
        return;
    }
    
    result = WSAStartup(MAKEWORD(2, 2), &winSockData);
    if (result != 0) {
        printf("Failed to initialize WinSock\n");
        exit(-1);
    }
    
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(PORT);
    
    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    listen(serverSocket, 5);
    
    printf("FML Server listening on port %d...\n", PORT);
    
    while (1) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &addrLen);
        
        struct threadArgs* args = (struct threadArgs*)malloc(sizeof(struct threadArgs));
        args->clientConn = &clientSocket;
        args->CommandHistory = CommandHistory;
        threadnum++;
        printf("Connection #%d established\n", threadnum);
        thread = (HANDLE)_beginthreadex(NULL, 0, (unsigned int(__stdcall*)(void*))HandleConnection, args, 0, NULL);
    }
}

int main(void) {
    BeginServer();
    return 0;
}










