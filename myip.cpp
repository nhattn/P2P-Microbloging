#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/fcntl.h>

int main(void) {
    int temp_sockfd, check;
    char buffer[100];
    struct sockaddr_in serv;
    temp_sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (temp_sockfd < 0) {
        perror("Error in socket()");
        return 0;
    }
    memset(&serv, 0, sizeof(serv));
    memset(buffer, 0, sizeof(buffer));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("8.8.8.8"); //google Public DNS IP
    serv.sin_port = htons(53);
    check = connect(temp_sockfd, (const struct sockaddr*)&serv, sizeof(serv));
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    check = getsockname(temp_sockfd, (struct sockaddr*)&name, &namelen);
    const char *ip = inet_ntop(AF_INET, &name.sin_addr, buffer, 100);
    if (ip != NULL && check != -1) {
        printf("The IP Address of this system is : %s \n", buffer);
    } else {
        printf("Error %d %s \n", errno, strerror(errno));
    }
    close(temp_sockfd);
    return 0;
}
