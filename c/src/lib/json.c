#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "parson/parson.h"
#include "utils.h"
#include "debug.h"

#define JSON_MAX_BUF 1024

size_t json_parse_command(char *json_data, char *out)
{

    JSON_Value *root_value;
    const char *command;
    size_t command_len;

    root_value = json_parse_string(json_data);
    command = json_value_get_string(root_value);
    command_len = json_value_get_string_len(root_value);

    for (int i = 0; i < (int)command_len; i++)
    {
        out[i] = command[i];
    }

    json_value_free(root_value);

    return command_len;
}

void json_serialize_cmd_response(char *cmd, char *res, char *out)
{

    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;

    json_object_set_string(root_object, "exec", cmd);
    json_object_set_string(root_object, "data", res);

    char t_buff[20];
    time_t now = time(NULL);
    strftime(t_buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&now));
    json_object_set_string(root_object, "exectime", t_buff);

    char id[5];
    rand_str(id, 5);
    json_object_set_string(root_object, "identifier", id);

    serialized_string = json_serialize_to_string_pretty(root_value);
    Dprintf("json_encoded response payload:\n%s\n", serialized_string);

    // assignt to out
    strncpy(out, serialized_string, strlen(serialized_string));

    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}
