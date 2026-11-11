#ifndef PEER_SERVER_H
#define PEER_SERVER_H

int create_peer_listener(int port);

void run_peer_server(int listener_fd, int stop_fd);

void handle_peer_request(int peer_fd);

#endif
