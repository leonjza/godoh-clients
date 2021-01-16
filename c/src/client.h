typedef enum
{
    Idle,
    Error,
    Command,
} status_t;

typedef struct Client
{
    char *domain;

    char agent_id[6];
    char agent_id_hex[6 * 2 + 1];
    char checkin_domain[256];

    char command[255];

    status_t status;
} client_t;

struct Client *init_client();
void poll(client_t *client);
