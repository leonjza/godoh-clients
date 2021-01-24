#ifndef GODOH_JSON_H
#define GODOH_JSON_H

#include <sys/types.h>

size_t json_parse_command(char *json_data, char *out);
void json_serialize_cmd_response(char *cmd, char *res, char *out);

#endif //GODOH_JSON_H
