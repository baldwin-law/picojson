/**
 * PicoJSON - Implementation file
 */

#include "../include/pico_json.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Internal functions declarations */
static int skip_whitespace(const char *s, int i);
static int parse_value(struct pj_str json, int i, int depth);
static int parse_string(struct pj_str json, int i);
static int parse_number(struct pj_str json, int i);
static int parse_object(struct pj_str json, int i, int depth);
static int parse_array(struct pj_str json, int i, int depth);
static int find_path_token(struct pj_str json, const char *path);

/* Create string from C string */
struct pj_str pj_str_s(const char *str)
{
    struct pj_str result = {str, str == NULL ? 0 : strlen(str)};
    return result;
}

/* Create string with specified length */
struct pj_str pj_str_n(const char *str, size_t len)
{
    struct pj_str result = {str, len};
    return result;
}

/* Validate if given string is a valid JSON */
bool pj_is_valid(struct pj_str json)
{
    int dummy;
    int result = parse_value(json, skip_whitespace(json.str, 0), 0);
    if (result < 0) {
        return false;
    }
    int end_pos = skip_whitespace(json.str, result);
    return end_pos == json.length;
}

/* Skip whitespace characters, return new position */
static int skip_whitespace(const char *s, int i)
{
    while (s[i] && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r'))
        i++;
    return i;
}

/* Parse JSON value at position i, return new position */
static int parse_value(struct pj_str json, int i, int depth)
{
    if (depth > PICO_JSON_MAX_DEPTH)
        return PICO_JSON_TOO_DEEP;
    if (i >= (int)json.length)
        return PICO_JSON_INVALID;

    i = skip_whitespace(json.str, i);
    if (i >= (int)json.length)
        return PICO_JSON_INVALID;

    switch (json.str[i]) {
        case '{':
            return parse_object(json, i, depth + 1);
        case '[':
            return parse_array(json, i, depth + 1);
        case '"':
            return parse_string(json, i);
        case 't':
            if (i + 3 < (int)json.length && strncmp(json.str + i, "true", 4) == 0) {
                if (i + 4 >= (int)json.length || !isalnum(json.str[i + 4]))
                    return i + 4;
            }
            return PICO_JSON_INVALID;
        case 'f':
            if (i + 4 < (int)json.length && strncmp(json.str + i, "false", 5) == 0) {
                if (i + 5 >= (int)json.length || !isalnum(json.str[i + 5]))
                    return i + 5;
            }
            return PICO_JSON_INVALID;
        case 'n':
            if (i + 3 < (int)json.length && strncmp(json.str + i, "null", 4) == 0) {
                if (i + 4 >= (int)json.length || !isalnum(json.str[i + 4]))
                    return i + 4;
            }
            return PICO_JSON_INVALID;

        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return parse_number(json, i);
        default:
            return PICO_JSON_INVALID;
    }
}

/* Parse JSON string at position i, return new position */
static int parse_string(struct pj_str json, int i)
{
    if (json.str[i] != '"')
        return PICO_JSON_INVALID;

    for (i++; i < (int)json.length; i++) {
        if (json.str[i] == '\\') {
            if (i + 1 >= (int)json.length)
                return PICO_JSON_INVALID;
            i++; // Skip the escaped character
        } else if (json.str[i] == '"') {
            return i + 1;
        }
    }

    return PICO_JSON_INVALID; // Unterminated string
}

/* Parse JSON number at position i, return new position */
static int parse_number(struct pj_str json, int i)
{
    int start = i;

    // Optional minus sign
    if (json.str[i] == '-')
        i++;

    // Integer part
    if (i < (int)json.length && json.str[i] == '0') {
        i++;
    } else if (i < (int)json.length && json.str[i] >= '1' && json.str[i] <= '9') {
        i++;
        while (i < (int)json.length && isdigit(json.str[i]))
            i++;
    } else {
        return PICO_JSON_INVALID;
    }

    // Fractional part
    if (i < (int)json.length && json.str[i] == '.') {
        i++;
        if (i >= (int)json.length || !isdigit(json.str[i]))
            return PICO_JSON_INVALID;
        while (i < (int)json.length && isdigit(json.str[i]))
            i++;
    }

    // Exponent
    if (i < (int)json.length && (json.str[i] == 'e' || json.str[i] == 'E')) {
        i++;
        if (i < (int)json.length && (json.str[i] == '+' || json.str[i] == '-'))
            i++;
        if (i >= (int)json.length || !isdigit(json.str[i]))
            return PICO_JSON_INVALID;
        while (i < (int)json.length && isdigit(json.str[i]))
            i++;
    }

    return i;
}

/* Parse JSON object at position i, return new position */
static int parse_object(struct pj_str json, int i, int depth)
{
    if (json.str[i] != '{')
        return PICO_JSON_INVALID;

    i = skip_whitespace(json.str, i + 1);
    if (i >= (int)json.length)
        return PICO_JSON_INVALID;

    if (json.str[i] == '}')
        return i + 1; // Empty object

    while (i < (int)json.length) {
        // Parse key
        i = skip_whitespace(json.str, i);
        if (i >= (int)json.length || json.str[i] != '"')
            return PICO_JSON_INVALID;

        i = parse_string(json, i);
        if (i < 0)
            return i;

        // Parse colon
        i = skip_whitespace(json.str, i);
        if (i >= (int)json.length || json.str[i] != ':')
            return PICO_JSON_INVALID;

        // Parse value
        i = parse_value(json, i + 1, depth);
        if (i < 0)
            return i;

        // Parse comma or closing brace
        i = skip_whitespace(json.str, i);
        if (i >= (int)json.length)
            return PICO_JSON_INVALID;

        if (json.str[i] == '}')
            return i + 1;
        if (json.str[i] != ',')
            return PICO_JSON_INVALID;

        i++;
    }

    return PICO_JSON_INVALID;
}

/* Parse JSON array at position i, return new position */
static int parse_array(struct pj_str json, int i, int depth)
{
    if (json.str[i] != '[')
        return PICO_JSON_INVALID;

    i = skip_whitespace(json.str, i + 1);
    if (i >= (int)json.length)
        return PICO_JSON_INVALID;

    if (json.str[i] == ']')
        return i + 1; // Empty array

    while (i < (int)json.length) {
        // Parse value
        i = parse_value(json, i, depth);
        if (i < 0)
            return i;

        // Parse comma or closing bracket
        i = skip_whitespace(json.str, i);
        if (i >= (int)json.length)
            return PICO_JSON_INVALID;

        if (json.str[i] == ']')
            return i + 1;
        if (json.str[i] != ',')
            return PICO_JSON_INVALID;
        i++;
    }

    return PICO_JSON_INVALID;
}

/* Find token at path, return position, set token_len */
int pj_get(struct pj_str json, const char *path, int *token_len)
{
    int pos = find_path_token(json, path);
    if (pos < 0)
        return pos;

    // Find token length
    int end = parse_value(json, pos, 0);
    if (end < 0)
        return end;

    if (token_len)
        *token_len = end - pos;
    return pos;
}

/* Internal path parsing */
static int find_path_token(struct pj_str json, const char *path)
{
    if (!path || !path[0])
        return 0; // Empty path means root

    struct pj_str current = json;
    int pos = 0, next;

    // Skip leading $ if present (JSONPath root indicator)
    if (path[0] == '$')
        path++;

    while (path[0]) {
        next = parse_value(current, pos, 0);
        if (next < 0)
            return next;

        // Handle object access
        if (path[0] == '.') {
            path++;
            if (!path[0])
                return PICO_JSON_INVALID;

            // Make sure current token is an object
            pos = skip_whitespace(current.str, pos);
            if (pos >= (int)current.length || current.str[pos] != '{')
                return PICO_JSON_NOT_FOUND;

            // Find key in object
            const char *key_end = strchr(path, '.');
            const char *bracket = strchr(path, '[');
            if (!key_end)
                key_end = path + strlen(path);
            if (bracket && bracket < key_end)
                key_end = bracket;

            size_t key_len = key_end - path;
            pos++; // Skip '{'

            while (pos < (int)current.length) {
                pos = skip_whitespace(current.str, pos);
                if (pos >= (int)current.length)
                    return PICO_JSON_INVALID;

                if (current.str[pos] == '}')
                    return PICO_JSON_NOT_FOUND;

                // Parse key
                if (current.str[pos] != '"')
                    return PICO_JSON_INVALID;
                int key_start = pos + 1;
                pos = parse_string(current, pos);
                if (pos < 0)
                    return pos;

                // Check if key matches
                if ((pos - key_start - 1) == key_len && strncmp(current.str + key_start, path, key_len) == 0) {
                    // Found the key, get value
                    pos = skip_whitespace(current.str, pos);
                    if (pos >= (int)current.length || current.str[pos] != ':')
                        return PICO_JSON_INVALID;

                    pos = skip_whitespace(current.str, pos + 1);
                    path = key_end;
                    break;
                }

                // Skip colon and value
                pos = skip_whitespace(current.str, pos);
                if (pos >= (int)current.length || current.str[pos] != ':')
                    return PICO_JSON_INVALID;

                pos = parse_value(current, pos + 1, 0);
                if (pos < 0)
                    return pos;

                // Skip comma or end object
                pos = skip_whitespace(current.str, pos);
                if (pos >= (int)current.length)
                    return PICO_JSON_INVALID;

                if (current.str[pos] == '}')
                    return PICO_JSON_NOT_FOUND;
                if (current.str[pos] != ',')
                    return PICO_JSON_INVALID;

                pos++;
            }
        }
        // Handle array access
        else if (path[0] == '[') {
            path++;
            char *end;
            long idx = strtol(path, &end, 10);
            if (end == path || *end != ']')
                return PICO_JSON_INVALID;

            path = end + 1; // Skip ']'

            // Make sure current token is an array
            pos = skip_whitespace(current.str, pos);
            if (pos >= (int)current.length || current.str[pos] != '[')
                return PICO_JSON_NOT_FOUND;

            // Find index in array
            long current_idx = 0;
            pos++; // Skip '['

            if (idx < 0)
                return PICO_JSON_NOT_FOUND;

            while (current_idx < idx && pos < (int)current.length) {
                pos = parse_value(current, pos, 0);
                if (pos < 0)
                    return pos;

                // Skip comma or end array
                pos = skip_whitespace(current.str, pos);
                if (pos >= (int)current.length)
                    return PICO_JSON_INVALID;

                if (current.str[pos] == ']')
                    return PICO_JSON_NOT_FOUND;
                if (current.str[pos] != ',')
                    return PICO_JSON_INVALID;

                pos = skip_whitespace(current.str, pos + 1);
                current_idx++;
            }

            if (current_idx != idx)
                return PICO_JSON_NOT_FOUND;
        } else {
            return PICO_JSON_INVALID; // Invalid path syntax
        }
    }

    return pos;
}

/* Get token as string */
struct pj_str pj_get_token(struct pj_str json, const char *path)
{
    struct pj_str result = {NULL, 0};
    int token_len;
    int pos = pj_get(json, path, &token_len);

    if (pos >= 0) {
        result.str = json.str + pos;
        result.length = token_len;
    }

    return result;
}

int pj_get_type(struct pj_str j, const char *path)
{
    struct pj_str token = pj_get_token(j, path);
    if (token.str == NULL || token.length == 0) {
        return PJ_TYPE_INVALID;
    }

    int pos = skip_whitespace(token.str, 0);
    if (pos >= token.length) {
        return PJ_TYPE_INVALID;
    }

    char c = token.str[pos];
    switch (c) {
        case '{':
            return PJ_TYPE_OBJECT;
        case '[':
            return PJ_TYPE_ARRAY;
        case '"':
            return PJ_TYPE_STRING;
        case 't':
        case 'f':
            if ((token.length - pos >= 4 && strncmp(token.str + pos, "true", 4) == 0) ||
                (token.length - pos >= 5 && strncmp(token.str + pos, "false", 5) == 0)) {
                return PJ_TYPE_BOOLEAN;
            }
            return PJ_TYPE_INVALID;
        case 'n':
            if (token.length - pos >= 4 && strncmp(token.str + pos, "null", 4) == 0) {
                return PJ_TYPE_NULL;
            }
            return PJ_TYPE_INVALID;
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return PJ_TYPE_NUMBER;
        default:
            return PJ_TYPE_INVALID;
    }
}

/* Get boolean value */
bool pj_get_bool(struct pj_str json, const char *path, bool *value)
{
    int token_len;
    int pos = pj_get(json, path, &token_len);

    if (pos < 0)
        return false;

    pos = skip_whitespace(json.str, pos);
    if (pos >= (int)json.length)
        return false;

    if (token_len == 4 && strncmp(json.str + pos, "true", 4) == 0) {
        if (value)
            *value = true;
        return true;
    } else if (token_len == 5 && strncmp(json.str + pos, "false", 5) == 0) {
        if (value)
            *value = false;
        return true;
    }

    return false;
}

/* Get numeric value */
bool pj_get_num(struct pj_str json, const char *path, double *value)
{
    int token_len;
    int pos = pj_get(json, path, &token_len);

    if (pos < 0)
        return false;

    pos = skip_whitespace(json.str, pos);
    if (pos >= (int)json.length)
        return false;

    // Check if token is a number
    char c = json.str[pos];
    if ((c >= '0' && c <= '9') || c == '-') {
        char buf[token_len + 1];
        strncpy(buf, json.str + pos, token_len);
        buf[token_len] = '\0';

        char *end;
        double result = strtod(buf, &end);
        bool success = (end != buf && *end == '\0');
        if (success) {
            if (value)
                *value = result;
            return true;
        }
    }
    return false;
}

/* Get integer value with default */
int pj_get_int(struct pj_str json, const char *path, int default_value)
{
    double value;
    if (pj_get_num(json, path, &value)) {
        return (int)value;
    }
    return default_value;
}

/* Get string value */
char *pj_get_str(struct pj_str json, const char *path)
{
    int token_len;
    int pos = pj_get(json, path, &token_len);

    if (pos < 0)
        return NULL;

    pos = skip_whitespace(json.str, pos);
    if (pos >= (int)json.length || json.str[pos] != '"')
        return NULL;

    // Find string content (without quotes)
    int content_start = pos + 1;
    int content_end = pos + token_len - 1;

    // Allocate and copy string content (including handling escapes)
    struct pj_str str = {json.str + content_start, content_end - content_start};
    char *buf = malloc(str.length + 1);
    if (!buf)
        return NULL;

    size_t j = 0;
    for (size_t i = 0; i < str.length; i++) {
        if (str.str[i] == '\\' && i + 1 < str.length) {
            i++;
            switch (str.str[i]) {
                case 'n':
                    buf[j++] = '\n';
                    break;
                case 'r':
                    buf[j++] = '\r';
                    break;
                case 't':
                    buf[j++] = '\t';
                    break;
                case 'b':
                    buf[j++] = '\b';
                    break;
                case 'f':
                    buf[j++] = '\f';
                    break;
                default:
                    buf[j++] = str.str[i];
                    break;
            }
        } else {
            buf[j++] = str.str[i];
        }
    }

    buf[j] = '\0';
    return buf;
}

bool pj_get_str_n(struct pj_str str, const char *path, char *buf, size_t buf_size)
{
    int token_len;
    int pos = pj_get(str, path, &token_len);

    if (pos < 0)
        return false;

    pos = skip_whitespace(str.str, pos);
    if (pos >= (int)str.length || str.str[pos] != '"')
        return false;

    // Find string content (without quotes)
    int content_start = pos + 1;
    int content_end = pos + token_len - 1;

    // Copy string content to buffer
    size_t j = 0;
    for (size_t i = content_start; i < content_end && j < buf_size - 1; i++) {
        if (str.str[i] == '\\' && i + 1 < content_end) {
            i++;
            switch (str.str[i]) {
                case 'n':
                    buf[j++] = '\n';
                    break;
                case 'r':
                    buf[j++] = '\r';
                    break;
                case 't':
                    buf[j++] = '\t';
                    break;
                case 'b':
                    buf[j++] = '\b';
                    break;
                case 'f':
                    buf[j++] = '\f';
                    break;
                default:
                    buf[j++] = str.str[i];
                    break;
            }
        } else {
            buf[j++] = str.str[i];
        }
    }

    buf[j] = '\0';
    return true;
}

/* Unescape a JSON string */
bool pj_unescape(struct pj_str str, char *buf, size_t buf_len)
{
    size_t j = 0;

    for (size_t i = 0; i < str.length && j < buf_len - 1; i++) {
        if (str.str[i] == '\\' && i + 1 < str.length) {
            i++;
            switch (str.str[i]) {
                case 'n':
                    buf[j++] = '\n';
                    break;
                case 'r':
                    buf[j++] = '\r';
                    break;
                case 't':
                    buf[j++] = '\t';
                    break;
                case 'b':
                    buf[j++] = '\b';
                    break;
                case 'f':
                    buf[j++] = '\f';
                    break;
                case 'u': // Unicode escape
                    if (i + 4 < str.length) {
                        // Simplified handling (just copies the Unicode escaped sequence)
                        buf[j++] = '\\';
                        buf[j++] = 'u';
                        for (int k = 0; k < 4 && j < buf_len - 1; k++) {
                            buf[j++] = str.str[++i];
                        }
                    } else {
                        return false;
                    }
                    break;
                default:
                    buf[j++] = str.str[i];
                    break;
            }
        } else {
            buf[j++] = str.str[i];
        }
    }

    buf[j] = '\0';
    return j < buf_len;
}

/* Iterate through object key-value pairs */
size_t pj_next(struct pj_str obj, size_t offset, struct pj_str *key, struct pj_str *val)
{
    if (offset == 0) {
        // Check if it's an object and skip '{'
        offset = skip_whitespace(obj.str, offset);
        if (offset >= obj.length || obj.str[offset] != '{')
            return 0;
        offset++;
    }

    offset = skip_whitespace(obj.str, offset);
    if (offset >= obj.length)
        return 0;

    // End of object
    if (obj.str[offset] == '}')
        return 0;

    // Skip comma if present
    if (obj.str[offset] == ',') {
        offset++;
        offset = skip_whitespace(obj.str, offset);
    }

    // Parse key
    if (offset >= obj.length || obj.str[offset] != '"')
        return 0;

    int key_start = offset + 1;                  // Skip opening quote
    int key_end = parse_string(obj, offset) - 1; // Skip closing quote
    if (key_end < 0)
        return 0;

    if (key) {
        key->str = obj.str + key_start;
        key->length = key_end - key_start;
    }

    // Skip colon
    offset = skip_whitespace(obj.str, key_end + 1);
    if (offset >= obj.length || obj.str[offset] != ':')
        return 0;
    offset++;

    // Parse value
    offset = skip_whitespace(obj.str, offset);
    if (offset >= obj.length)
        return 0;

    int val_start = offset;
    int val_end = parse_value(obj, offset, 0);
    if (val_end < 0)
        return 0;

    if (val) {
        val->str = obj.str + val_start;
        val->length = val_end - val_start;
    }

    return val_end;
}

int pj_vstreamf(const struct pj_stream_out *out, const char *fmt, va_list ap)
{
    if (out && out->on_write) {
        char buf[1024];
        int len = vsnprintf(buf, sizeof(buf), fmt, ap);
        if (len > 0 && len < (int)sizeof(buf)) {
            out->on_write(buf, len, out->user);
            return len;
        }
    }
    return -1;
}

int pj_streamf(const struct pj_stream_out *out, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int result = pj_vstreamf(out, fmt, ap);
    va_end(ap);
    return result;
}

static void stream_to_file(const char *buf, size_t len, void *user)
{
    FILE *fp = (FILE *)user;
    fwrite(buf, 1, len, fp);
}

void pj_fstream(FILE *fp, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    struct pj_stream_out out = {stream_to_file, fp};
    pj_vstreamf(&out, fmt, ap);
    va_end(ap);
}