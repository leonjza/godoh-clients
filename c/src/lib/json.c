#include <sys/types.h>
#include <stdlib.h>

#include "parson/parson.h"
#include "utils.h"

#define JSON_MAX_BUF 1024

size_t parse_json_command(char *json_data, char *out)
{

    JSON_Value *root_value;
    const char *command;
    size_t command_len;

    root_value = json_parse_string(json_data);
    command = json_value_get_string(root_value);
    command_len = json_value_get_string_len(root_value);

    for(int i = 0; i < (int) command_len; i++) {
        out[i] = command[i];
    }

    json_value_free(root_value);

    return command_len;
}
