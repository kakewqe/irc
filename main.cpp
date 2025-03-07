#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <string.h>

#define SERVER_PORT 12345

int main(void)
{
    int len, rc, on = 1;
    int listen_sd = -1, new_sd = -1;
    int desc_rdy, end_server = 0;
    int compress_array = 0;
    int close_conn;
    char buffer[80];
    struct sockaddr_in6 addr;
    int timeout;
    struct pollfd fds[200];
    int nfds = 1, current_size = 0, i, j;

    listen_sd = socket(AF_INET6, SOCK_STREAM, 0);
    if (listen_sd < 0)
    {
        perror("socket() failled");
        exit(-1);
    }
    /*
    rc = setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
    if(rc < 0)
    {
        perror("setsocket() failled");
        close(listen_sd);
        exit(-1);
    }
    */

    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    memcpy(&addr.sin6_addr, &in6addr_any, sizeof(in6addr_any));
    addr.sin6_port = htons(SERVER_PORT);
    rc = bind(listen_sd, (struct sockaddr *)&addr, sizeof(addr));
    if (rc < 0)
    {
        perror("bind() failed");
        close(listen_sd);
        exit(-1);
    }

    rc = listen(listen_sd, 32);
    if (rc < 0)
    {
        perror("listen() failed");
        close(listen_sd);
        exit(-1);
    }

    memset(fds, 0, sizeof(fds));
    fds[0].fd = listen_sd;
    fds[0].events = POLLIN;
    // timeout = (3 * 60 * 1000);

    while (!end_server) /* Tant que le serveur doit tourner */
    {
        printf("Waiting on poll()...\n");
        rc = poll(fds, nfds, timeout);

        if (rc < 0)
        {
            perror("  poll() failed");
            break;
        }

        if (rc == 0)
        {
            printf("  poll() timed out.  End program.\n");
            break;
        }

        current_size = nfds;
        for (i = 0; i < current_size; i++)
        {
            if (fds[i].revents == 0)
                continue;

            if (fds[i].revents != POLLIN)
            {
                printf("  Error! revents = %d\n", fds[i].revents);
                end_server = 1;
                break;
            }

            if (fds[i].fd == listen_sd)
            {
                printf("  Listening socket is readable\n");

                while (1) /* Boucle d'acceptation */
                {
                    new_sd = accept(listen_sd, NULL, NULL);
                    if (new_sd < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("  accept() failed");
                            end_server = 1;
                        }
                        break;
                    }

                    printf("  New incoming connection - %d\n", new_sd);
                    fds[nfds].fd = new_sd;
                    fds[nfds].events = POLLIN;
                    nfds++;
                }
            }
            else
            {
                printf("  Descriptor %d is readable\n", fds[i].fd);
                close_conn = 0;

                while (1) /* Boucle de réception */
                {
                    rc = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                    if (rc < 0)
                    {
                        if (errno != EWOULDBLOCK)
                        {
                            perror("  recv() failed");
                            close_conn = 1;
                        }
                        break;
                    }

                    if (rc == 0)
                    {
                        printf("  Connection closed\n");
                        close_conn = 1;
                        break;
                    }

                    printf("  %d bytes received\n", rc);

                    rc = send(fds[i].fd, buffer, rc, 0);
                    if (rc < 0)
                    {
                        perror("  send() failed");
                        close_conn = 1;
                        break;
                    }
                }

                if (close_conn)
                {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    compress_array = 1;
                }
            }
        }

        if (compress_array)
        {
            compress_array = 0;
            for (i = 0; i < nfds; i++)
            {
                if (fds[i].fd == -1)
                {
                    for (j = i; j < nfds - 1; j++)
                    {
                        fds[j] = fds[j + 1];
                    }
                    nfds--;
                    i--;
                }
            }
        }
    }

    /* Fermeture des sockets */
    for (i = 0; i < nfds; i++)
    {
        if (fds[i].fd >= 0)
            close(fds[i].fd);
    }

    return 0;
}