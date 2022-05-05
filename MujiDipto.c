#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <pthread.h>
#include <netdb.h>


#define CONNECT_LENGTH 7 // length of "Connect"
#define IM_LENGTH 2 // length of "IM"
#define TRANSFER_LENGTH 8 // length of "Transfer"
#define EXECUTE_LENGTH 7 // length of "Execute"
#define DELIVER_LENGTH 7 // length of "Deliver"
#define WITHDRAW_LENGTH 8 // length of "Withdraw"
#define DEFER_LENGTH 5 // length of "Defer"

// stores a deferred message 
typedef struct {
    unsigned int key;
    char* message;
} Defer;

// Enum for all depot exit statuses
typedef enum {
    OK = 0,
    BAD_ARG_NUM = 1,
    BAD_NAME = 2,
    BAD_QUANTITY = 3
} Status;

// stores a neighbour of the depot
typedef struct {
    char* name;
    char* port;
} Neighbour;

// stores a resources
typedef struct {
    int quantity;
    char* name;
} Goods;

// stores a connection of the depot
// i.e. when connected to another depot
// using "Connect:p" message
typedef struct {
    char* name;
    char* port;
    FILE* send;
    FILE* receive;
} Connection;

// Properties associated with the depot
typedef struct {
    int numOfConnections;
    Connection** connections;
    int portNo;
    Defer** deferredItems;
    char* name;
    int deferCount;
    Goods** goods;
    int numOfGoods;
    Neighbour** neighbours;
    int numOfNeighbours;
} Depot;

// global variable for handling sighup
Depot depot;

/*
 * Reads a line from a given file
 *
 * @param file: file to be read
 *
 * @return: line - line read from the file
 *
 */
char* read_line(FILE* file) {
    int position = 0;
    char* line = malloc(sizeof(char));
    char c;
    while (1) {
        c = fgetc(file);
        line = realloc(line, sizeof(char) * position + 1);
        if (c == EOF || c == '\n') {
            line[position] = '\0';
            return line;
        } else {
            line[position++] = (char) c;
        }

    }

}
/*
 * Connects to a given port number when a
 * "Connect:p" message arrives
 *
 * @param message: the connect message
 * @param depot:   the depot that will connect
 *
 */
void connect_to_port(char* message, Depot* depot);


/*
 * Looks for banned chars in a given string
 * According to the spec, the banned chars are:
 * space, color, \n and \r
 *
 * @param str: the string to be checked
 *
 * @return: true if given string is empty
 *          or a banned char is found
 *          false otherwise
 *
 */
bool has_banned_char(char* str) {
    if (strlen(str) == 0) {
        return true;

    }
    for (int i = 0; i < strlen(str); ++i) {
        if (str[i] == ' ' || str[i] == '\n' ||
                str[i] == ':' || str[i] == '\r') {
            return true;
        }
        /* double check with ascii values
         * 32 is space, 10 is line feed, 13 is carriage return and 58 is colon
         */
        int asciiValue = (int) str[i];
        if (asciiValue == 32 || asciiValue == 10 ||
                asciiValue == 13 || asciiValue == 58) {
            return true;
        }
    }
    return false;

}

/*
 * Stores the goods given by the delivery message
 *
 * @param message: the delivery message containing good type and quantity
 * @param depot:   the depot where the goods will be stored
 *
 */
void store_delivery(char* message, Depot* depot) {
    // deliver message should contain three items- Deliver:qty:name
    char* deliver = strtok(message, ":");
    char* quantity = strtok(NULL, ":");
    char* type = strtok(NULL, ":");

    // if message contains any extra characters, it is invalid
    char* extra = strtok(NULL, ":");
    char* err;
    // ignore bad message
    if (deliver == NULL || quantity == NULL || type == NULL || extra != NULL) {
        return;

    }
    // remove newline
    type[strcspn(type, "\n")] = 0;
    if (has_banned_char(deliver) || has_banned_char(type)) {
        return;
    }
    int amount = strtoul(quantity, &err, 10);
    if (*err != '\0' || amount <= 0) {
        return;
    }
    int counter = depot->numOfGoods;
    // check if we already have the item
    for (int i = 0; i < counter; ++i) {
        if (!strcmp(type, depot->goods[i]->name)) {
            depot->goods[i]->quantity += amount;
            return;
        }
    }
    // create and store the new item
    depot->numOfGoods = counter + 1;
    depot->goods[counter] = malloc(sizeof(Goods));
    depot->goods[counter]->name = type;
    depot->goods[counter]->quantity = amount;
    counter++;
    depot->goods = realloc(depot->goods,
            (sizeof(Goods*) * (depot->numOfGoods + 1)));

}

/*
 * Scans a message to look for "Connect:"
 * if found, it calls connect_to_port() that
 * connects to the given port if the message is valid
 *
 * @param message: the message to be scanned
 * @param depot:   the depot that will connect
 *
 * returns: line - line read from the file
 *
 */
void accept_connections(char* message, Depot* depot) {
    if (strncmp(message, "Connect", CONNECT_LENGTH) != 0) {
        return;
    }
    connect_to_port(message, depot);
}

/*
 * Adds a neighbour to a depot
 *
 * @param message: string containing neighbour's name and port
 * @param depot:   depot to which the neighbour will be added
 *
 */
void add_neighbour(char* message, Depot* depot) {
    // IM messages should contain three items- imMessage:p:name
    char* imMessage = strtok(message, ":");
    char* port = strtok(NULL, ":");
    char* name = strtok(NULL, ":");
    // ignore bad message
    if (imMessage == NULL || port == NULL || name == NULL) {
        return;

    }
    // remove newline
    name[strcspn(name, "\n")] = 0;
    if (has_banned_char(name)) {
        return;
    }
    // add to the neighbours of the depot
    int counter = depot->numOfNeighbours;
    depot->numOfNeighbours = counter + 1;
    depot->neighbours[counter] = malloc(sizeof(Neighbour));
    depot->neighbours[counter]->name = name;
    depot->neighbours[counter]->port = port;
    counter++;
    depot->neighbours = realloc(depot->neighbours,
            (sizeof(Neighbour*) * (depot->numOfNeighbours + 1)));
    // if port matches, store the name of the connection from IM message
    for (int i = 0; i < depot->numOfNeighbours; ++i) {
        for (int j = 0; j < depot->numOfConnections; ++j) {
            if (strcmp(depot->neighbours[i]->port,
                    depot->connections[j]->port) == 0) {
                depot->connections[j]->name = depot->neighbours[i]->name;
            }
        }
    }
}

/*
 * Withdraws goods from a depot
 *
 * @param message: string containing type and quantity of goods
 * @param depot:   depot from which the good will be withdrawn
 *
 */
void withdraw_goods(char* message, Depot* depot) {
    // withdraw message should contain three items- Deliver:name:qty
    char* withdraw = strtok(message, ":");
    char* quantity = strtok(NULL, ":");
    char* type = strtok(NULL, ":");

    // if message contains extra information, it is invalid
    char* extra = strtok(NULL, ":");
    char* err;
    // ignore bad message
    if (withdraw == NULL || quantity == NULL ||
            type == NULL || extra != NULL) {
        return;

    }
    // remove newline
    type[strcspn(type, "\n")] = 0;
    if (has_banned_char(withdraw) || has_banned_char(type)) {
        return;
    }
    int amount = strtoul(quantity, &err, 10);
    if (*err != '\0' || amount <= 0) {
        return;
    }

    int counter = depot->numOfGoods;
    // check if we already have it and try to withdraw
    for (int i = 0; i < counter; ++i) {
        if (!strcmp(type, depot->goods[i]->name)) {
            depot->goods[i]->quantity -= amount;
            return;
        }
    }
    // store the new type of good
    depot->numOfGoods = counter + 1;
    depot->goods[counter] = malloc(sizeof(Goods));
    depot->goods[counter]->name = type;
    depot->goods[counter]->quantity = 0 - amount;
    counter++;
    depot->goods = realloc(depot->goods,
            (sizeof(Goods*) * (depot->numOfGoods + 1)));

}

/*
 * Scan for messages beginning with "IM:"
 * If found, calls add_neighbour() which further processes
 * the message and adds neighbour if the message is valid
 *
 * @param message: the string to scan
 * @param depot:   the depot where the neighbour would be added
 *
 */
bool accept_neighbours(char* message, Depot* depot) {
    bool found = false;
    if (message == NULL) {
        return false;
    }
    // +2 to allow for extra characters (checked later)
    char* neighbourMessage = malloc(strlen(message) + 2);
    if (strncmp(message, "IM", IM_LENGTH) == 0) {
        found = true;
        strcpy(neighbourMessage, message);
    }
    if (!found) {
        free(neighbourMessage);
        return false;
    } else {
        add_neighbour(neighbourMessage, depot);
        return true;
    }

}

/*
 * Transfers goods from one depot to another
 * This involves withdrawing goods from the depot that
 * is transferring and delivering it to the destination depot
 *
 * @param quantity: the amount to be transferred
 * @param type:     the name of goods to be transferred
 * @param port:     the destination port
 * @param depot:    depot that will perform the transfer
 *
 */
void transfer_goods(int quantity, char* type, char* port, Depot* depot) {
    // withdraw locally
    bool weHaveIt = false;
    int counter = depot->numOfGoods;
    for (int i = 0; i < counter; ++i) {
        if (!strcmp(type, depot->goods[i]->name)) {
            depot->goods[i]->quantity -= quantity;
            weHaveIt = true;
            break;
        }
    }
    if (!weHaveIt) {
        depot->numOfGoods = counter + 1;
        depot->goods[counter] = malloc(sizeof(Goods));
        depot->goods[counter]->name = type;
        depot->goods[counter]->quantity = 0 - quantity;
        counter++;
        depot->goods = realloc(depot->goods,
                (sizeof(Goods*) * (depot->numOfGoods + 2)));
    }
    // send delivery
    bool isConnected = false;
    FILE* to;
    int connectionNum = depot->numOfConnections;
    for (int i = 0; i < connectionNum; ++i) {
        if (!strcmp(port, depot->connections[i]->name)) {
            to = depot->connections[i]->send;
            isConnected = true;
        }
    }
    fflush(stdout);
    if (!isConnected) {
        return;
    }
    fprintf(to, "Deliver:%d:%s\n", quantity, type);
    fflush(to);
}

/*
 * Scans a message to look for "Transfer:"
 * if found, it calls transfer_goods() that carries
 * out the transfer process if the message is valid
 *
 * @param str:   the message to be scanned
 * @param depot: depot that will carry out the transfer
 *
 */
void check_for_transfer(char* str, Depot* depot) {
    char* message = malloc(sizeof(char) * strlen(str));
    message = strcpy(message, str);
    if (message == NULL) {
        return;
    }
    if (strncmp(message, "Transfer", TRANSFER_LENGTH) != 0) {
        return;
    }
    char* transfer = strtok(message, ":");
    char* givenQuantity = strtok(NULL, ":");
    char* name = strtok(NULL, ":");
    char* port = strtok(NULL, ":");
    char* extra = strtok(NULL, ":"), * err;
    if (transfer == NULL || givenQuantity == NULL || name == NULL ||
            port == NULL || extra != NULL) {
        return;

    }
    // remove newline
    port[strcspn(port, "\n")] = 0;
    int quantity = strtoul(givenQuantity, &err, 10);
    if (*err != '\0' || quantity <= 0) {
        return;

    }

    if (has_banned_char(name)) {
        return;
    }
    transfer_goods(quantity, name, port, depot);

}

/*
 * Executes a deffered message if key matches
 *
 * @param message: message containing the key and the
 *                 command to be executed
 * @param depot:  depot on which the command will be executed
 *
 */
void execute(char* message, Depot* depot) {
    if (strncmp(message, "Execute", EXECUTE_LENGTH) != 0) {
        return;
    }
    char* execute = strtok(message, ":");
    char* givenKey = strtok(NULL, ":");
    char* extra = strtok(NULL, ":");
    char* err;
    if (givenKey == NULL || extra != NULL || execute == NULL) {
        return;
    }
    int key = strtoul(givenKey, &err, 10);
    if (*err != '\0' || key < 0) {
        return;
    }
    // find the command in the deferred message and call functions accordingly
    int n = depot->deferCount;
    for (int i = 0; i < n; ++i) {
        if (depot->deferredItems[i]->key == key) {
            if (strncmp(depot->deferredItems[i]->message, "Deliver",
                    DELIVER_LENGTH) == 0) {
                store_delivery(depot->deferredItems[i]->message, depot);
                depot->deferredItems[i]->key = -1;
            } else if (strncmp(depot->deferredItems[i]->message,
                    "Withdraw", WITHDRAW_LENGTH) == 0) {
                withdraw_goods(depot->deferredItems[i]->message, depot);
                depot->deferredItems[i]->key = -1;
            } else if (strncmp(depot->deferredItems[i]->message,
                    "Transfer", TRANSFER_LENGTH) == 0) {
                check_for_transfer(depot->deferredItems[i]->message, depot);
                depot->deferredItems[i]->key = -1;
            } else {
                // invalid command
                return;
            }
        }
    }
}

/*
 * Scan for messages beginning with "Deliver:"
 * If found, calls store_deliver() which further processes
 * the message and stores the goods if the message is valid
 *
 * @param str:     the string to scan
 * @param depot:   the depot where the goods would be added
 *
 */
bool check_for_delivery(char* str, Depot* depot) {
    bool found = false;
    if (str == NULL) {
        return false;
    }
    // +2 to allow for extra characterts (checked later)
    char* deliverMessage = malloc(strlen(str) + 2);
    if (strncmp(str, "Deliver", DELIVER_LENGTH) == 0) {
        found = true;
        strcpy(deliverMessage, str);
    }
    if (!found) {
        free(deliverMessage);
        return false;
    } else {
        store_delivery(deliverMessage, depot);
        return true;
    }

}

/*
 * Sorts an array of goods in lexicographic order
 * then prints to stdout
 */
void sort_goods() {
    Goods* temp;
    int n = depot.numOfGoods;
    for (int i = 1; i < n; ++i) {
        for (int j = 0; j < n - i; ++j) {
            if (strcmp(depot.goods[j]->name, depot.goods[j + 1]->name) > 0) {
                temp = depot.goods[j];
                depot.goods[j] = depot.goods[j + 1];
                depot.goods[j + 1] = temp;
            }
        }
    }
}

/*
 * Adds a defer message to an array
 * The array can later be iterated through
 * and executed when execute message arrives
 *
 * @param deferMessage: the defer message
 * @param key:          the key of the defer message
 * @param depot:        depot on which the defer message would later
 *                      be executed
 *
 */
void add_defer_message(char* deferMessage, unsigned int key, Depot* depot) {
    int counter = depot->deferCount;
    depot->deferCount = counter + 1;
    depot->deferredItems[counter] = malloc(sizeof(Defer));
    depot->deferredItems[counter]->key = key;
    depot->deferredItems[counter]->message = deferMessage;
    counter++;
    depot->deferredItems = realloc(depot->deferredItems,
            (sizeof(Defer*) * depot->deferCount + 1));
}

/*
 * Scans for message beginning with "Defer"
 * Note - this function only handles deferred transfers
 * for other defers see check_for_deferred_transfers()
 * 
 * @param str:    string to be searched
 * @param depot:  depot on to store the message
 *
 */
void check_for_deferred_transfers(char* str, Depot* depot) {
    char* message = malloc(sizeof(char) * strlen(str));
    message = strcpy(message, str);
    if (message == NULL) {
        return;
    }
    if (strncmp(message, "Defer", DEFER_LENGTH) != 0) {
        return;
    }
    char* defer = strtok(message, ":");
    char* givenKey = strtok(NULL, ":");
    char* transfer = strtok(NULL, ":");
    char* quantity = strtok(NULL, ":");
    char* name = strtok(NULL, ":");
    char* port = strtok(NULL, ":");
    char* extra = strtok(NULL, ":"), * err, * err2;
    if (defer == NULL || givenKey == NULL || transfer == NULL ||
            quantity == NULL || name == NULL || port == NULL ||
            extra != NULL) {
        return;

    }

    // remove newline
    port[strcspn(port, "\n")] = 0;
    unsigned int key = strtoul(givenKey, &err, 10);
    int amount = strtoul(quantity, &err2, 10);
    if (*err != '\0' || key < 0) {
        return;

    }
    if (*err2 != '\0' || amount <= 0) {
        return;
    }

    if (has_banned_char(transfer) || has_banned_char(name)) {
        return;
    }
    int keyLength = floor(log10(key) + 1);
    // 7 for length of "Defer:" and extra ":"
    char* deferMessage = str + 7 + keyLength;
    add_defer_message(deferMessage, key, depot);
}

/*
 * Scans for message beginning with "Defer"
 * Note - this function does not handle deferred transfer
 * for deferred transfers see check_for_deferred_transfers()
 * 
 * @param str:    string to be searched
 * @param depot:  depot on to store the message
 *
 */
void receive_defer_messages(char* str, Depot* depot) {
    char* message = malloc(sizeof(char) * strlen(str));
    message = strcpy(message, str);
    if (message == NULL) {
        return;
    }
    if (strncmp(message, "Defer", DEFER_LENGTH) != 0) {
        return;
    }
    char* defer = strtok(message, ":");
    char* givenKey = strtok(NULL, ":");
    char* command = strtok(NULL, ":");
    char* quantity = strtok(NULL, ":");
    char* name = strtok(NULL, ":");
    char* extra = strtok(NULL, ":"), *err, *errTwo;
    if (defer == NULL || givenKey == NULL || command == NULL ||
            quantity == NULL || name == NULL || extra != NULL) {
        return;

    }

    // remove newline
    name[strcspn(name, "\n")] = 0;
    unsigned int key = strtoul(givenKey, &err, 10);
    int amount = strtoul(quantity, &errTwo, 10);
    if (*err != '\0' || key < 0) {
        return;

    }
    if (*errTwo != '\0' || amount <= 0) {
        return;
    }

    if (has_banned_char(command) || has_banned_char(name)) {
        return;
    }
    int keyLength = floor(log10(key) + 1);
    // 7 for length of "Defer:" and extra ":"
    char* deferMessage = str + 7 + keyLength;
    add_defer_message(deferMessage, key, depot);
}

/*
 * Sorts an array of neighbours in lexicographic order
 * then prints to stdout
 *
 */
void sort_neighbours() {
    Neighbour* temp;
    int n = depot.numOfNeighbours;
    for (int i = 1; i < n; ++i) {
        for (int j = 0; j < n - i; ++j) {
            if (strcmp(depot.neighbours[j]->name,
                    depot.neighbours[j + 1]->name) > 0) {
                temp = depot.neighbours[j];
                depot.neighbours[j] = depot.neighbours[j + 1];
                depot.neighbours[j + 1] = temp;
            }
        }
    }
}

/*
 * If the depot receives SIGHUP, this function is executed and
 * it prints out the current (non-zero) stock of goods in lexicographic
 * order/ It then prints the names of its neighbours in lexicographic otder.
 *
 * singal: signal number- 1 in this case
 */
void got_hupped(int signal) {
    (void) signal;
    sort_goods();
    sort_neighbours();
    printf("Goods:\n");
    fflush(stdout);
    for (int i = 0; i < depot.numOfGoods; ++i) {
        if (depot.goods[i]->quantity != 0) {
            printf("%s %d\n", depot.goods[i]->name, depot.goods[i]->quantity);
            fflush(stdout);
        }
    }
    printf("Neighbours:\n");
    fflush(stdout);
    for (int i = 0; i < depot.numOfNeighbours; ++i) {
        printf("%s\n", depot.neighbours[i]->name);
        fflush(stdout);
    }

}

/*
 * Sets up a signal handler for SIGHUP
 * Calls a function that prints the current
 * stock and goods- see got_hupped()
 *
 */
void setup_sighup(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = got_hupped;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGHUP, &sa, 0);
}

/*
 * Sets up a signal handler for SIGPIPE
 * so that it is blocked and the depot keeps
 * running even if SIGPIPE is received
 *
 */
void block_sigpipe(void) {
    struct sigaction sa;
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_RESTART;
    sigaction(SIGPIPE, &sa, 0);

}

/*
 * Scans for message beginning with "Withdraw"
 * If found, it calls withdraw_goods() that further
 * processes the message and withdraws the goods from
 * the depot if it is valid
 *
 * @param str:    the message to be scanned
 *                command to be executed
 * @param depot:  depot on which the command will be executed
 *
 */
void check_for_withdrawal(char* str, Depot* depot) {
    bool found = false;
    if (str == NULL) {
        return;
    }
    // +2 to allow extra characters (checked later)
    char* withdrawMessage = malloc(strlen(str) + 2);
    if (strncmp(str, "Withdraw", WITHDRAW_LENGTH) == 0) {
        found = true;
        strcpy(withdrawMessage, str);
    }
    if (!found) {
        free(withdrawMessage);
        return;
    } else {
        withdraw_goods(withdrawMessage, depot);
    }

}

/*
 * Calls all functions responsible to process
 * messages and carry out commands
 *
 */
void process_messages(char* message, Depot* depot) {
    check_for_delivery(message, depot);
    check_for_withdrawal(message, depot);
    accept_neighbours(message, depot);
    receive_defer_messages(message, depot);
    check_for_deferred_transfers(message, depot);
    accept_connections(message, depot);
    check_for_transfer(message, depot);
    execute(message, depot);
}

/*
 * Starts receiving messages
 * This function is designed to be used in a thread

 * @param args: file pointer from which messages will arrive
 *
 * @return: NULL
 *
 */
void* thread_hosting(void* args) {
    FILE* from = (FILE*) args;
    char* buffer;
    while (true) {
        buffer = read_line(from);
        process_messages(buffer, &depot);
    }
    return NULL;
}

/*
 * Attempts to connect to a given port
 * if successful, depot now has a new connection
 *
 * @param message: the connect message
 * @param depot:   depot that will connect
 *
 */
void connect_to_port(char* message, Depot* depot) {
    char* connectMessage = strtok(message, ":");
    char* givenPort = strtok(NULL, ":");
    char* extra = strtok(NULL, ":");
    // ignore bad messages
    if (connectMessage == NULL || givenPort == NULL || extra != NULL) {
        return;
    }
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    int err;
    if ((err = getaddrinfo("127.0.0.1", givenPort, &hints, &ai))) {
        freeaddrinfo(ai);
        return; // could not work out the address
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
    if (connect(fd, (struct sockaddr*) ai->ai_addr, sizeof(struct sockaddr))) {
        return;
    }
    int fd2 = dup(fd);
    FILE* to = fdopen(fd, "w");
    FILE* from = fdopen(fd2, "r");
    fprintf(to, "IM:%s:%s\n", givenPort, depot->name);
    fflush(to);
    // store the new connection
    int counter = depot->numOfConnections;
    depot->numOfConnections = counter + 1;
    depot->connections[counter] = malloc(sizeof(Connection));
    depot->connections[counter]->port = givenPort;
    depot->connections[counter]->send = to;
    depot->connections[counter]->receive = from;
    depot->connections = realloc(depot->connections,
            (sizeof(Connection*) * (depot->numOfConnections + 1)));
    pthread_t threadId;
    // start communicating in a new thread
    pthread_create(&threadId, NULL, thread_hosting, (void*) from);
}

/*
 * Listens on an ephemeral port and outputs that port
 * number to stdout
 *
 * @param depot: depot that will listen and print
 *
 */
void start_hosting(Depot* depot) {
    struct addrinfo* ai = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // Because we want to bind with it
    int err, connFd;
    if ((err = getaddrinfo("127.0.0.1", 0, &hints, &ai))) {
        freeaddrinfo(ai);
        return; // could not work out the address
    }
    // create a socket and bind it to a port
    int serv = socket(AF_INET, SOCK_STREAM, 0); // 0 == use default protocol
    if (bind(serv, (struct sockaddr*)ai->ai_addr,
            sizeof(struct sockaddr))) {
        return;
    }
    struct sockaddr_in ad;
    memset(&ad, 0, sizeof(struct sockaddr_in));
    socklen_t len = sizeof(struct sockaddr_in);
    if (getsockname(serv, (struct sockaddr*)&ad, &len)) {
        return;
    }
    depot->portNo = (int) ntohs(ad.sin_port);
    printf("%u\n", ntohs(ad.sin_port));
    fflush(stdout);
    if (listen(serv, SOMAXCONN)) {
        return;
    }
    connFd = accept(serv, 0, 0); // change 0, 0 to get info about other end
    int fd2 = dup(connFd);
    FILE* to = fdopen(connFd, "w");
    FILE* from = fdopen(fd2, "r");
    char* buffer;
    fprintf(to, "IM:%d:%s\n", depot->portNo, depot->name);
    fflush(to);
    while (true) {
        buffer = read_line(from);
        process_messages(buffer, depot);
    }
}

/*
 * Initializes a depot.
 * Sets up its name and stock of goods and neighbours
 *
 * argc: number of command line arguments
 * argv: command line arguments
 *
 */
Status init_depot(int argc, char** argv) {
    int numOfGoods;
    int itemNumber = 0;
    depot.numOfNeighbours = 0;
    depot.deferCount = 0;
    depot.numOfConnections = 0;
    if (argc > 2) {
        numOfGoods = (argc - 2) / 2;
        depot.goods = malloc(sizeof(Goods*) * numOfGoods);
    } else {
        // started depot with no resources
        numOfGoods = 0;
        depot.goods = malloc(sizeof(Goods*) * 1);
    }

    depot.name = argv[1];
    depot.numOfGoods = numOfGoods;
    if (argc > 2) {
        int i = 2;
        // store resources from argv
        while (i < argc) {
            depot.goods[itemNumber] = malloc(sizeof(Goods));
            depot.goods[itemNumber]->name = argv[i];
            depot.goods[itemNumber]->quantity = atoi(argv[i + 1]);
            itemNumber++;
            i += 2;
        }
    }
    depot.neighbours = malloc(sizeof(Neighbour*) * 20);
    depot.connections = malloc(sizeof(Connection*));
    depot.deferredItems = malloc(sizeof(Defer*));
    start_hosting(&depot);
    return OK;

}

// output error message for status and return status
Status show_message(Status s) {
    const char* messages[] = {
        "",
        "Usage: 2310depot name {goods qty}\n",
        "Invalid name(s)\n",
        "Invalid quantity\n"
    };
    fputs(messages[s], stderr);
    return s;
}

/*
 * Checks the command line arguments according to the spec
 *
 * @param argc: number of command line arguments
 * @param argv: command line arguments
 *
 */
Status check_args(int argc, char** argv) {
    unsigned int counter = 0;
    if (has_banned_char(argv[1])) {
        return show_message(BAD_NAME);
    }
    for (int i = 2; i < argc; ++i) {
        counter++;
        char* err;
        if ((i % 2)) {
            int qty = strtoul(argv[i], &err, 10);
            if (qty < 0 || *err != '\0' || strlen(argv[i]) == 0) {
                return show_message(BAD_QUANTITY);
            }
        } else {
            if (has_banned_char(argv[i]) || strlen(argv[i]) == 0) {
                return show_message(BAD_NAME);
            }

        }
    }
    if (counter % 2) {
        return show_message(BAD_QUANTITY);

    }
    return init_depot(argc, argv);
}

/*
 * Runs the program
 *
 *  @param argc: number of command line arguments
 *  @param argv: command line arguments
 *
 *  @return: 0 for normal operation
 *           1 for incorrect number of args
 *           2 empty bane or name contains banned characters
 *           3 quantity parameter < 0 or is not a number
 *           4 invalid savefile
 *
 */
int main(int argc, char** argv) {
    // keep program running even if sigpipe is received
    block_sigpipe();
    // print stocks and neighbours on sighup
    setup_sighup();
    if (argc % 2 != 0) {
        return show_message(BAD_ARG_NUM);
    } else {
        return check_args(argc, argv);
    }
    return 0;

}
