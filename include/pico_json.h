#ifndef PICO_JSON_H_HEADER
#define PICO_JSON_H_HEADER

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef PICO_JSON_MAX_DEPTH
#define PICO_JSON_MAX_DEPTH 32 // Maximum nesting depth
#endif

// Error codes for JSON parsing
enum pj_error {
    PICO_JSON_OK = 0,
    PICO_JSON_INVALID = -1,
    PICO_JSON_NOT_FOUND = -2,
    PICO_JSON_TOO_DEEP = -3, // Nesting too deep
};

enum pj_type {
    PJ_TYPE_INVALID = 0,
    PJ_TYPE_OBJECT = 1,
    PJ_TYPE_ARRAY = 2,
    PJ_TYPE_STRING = 3,
    PJ_TYPE_NUMBER = 4,
    PJ_TYPE_BOOLEAN = 5,
    PJ_TYPE_NULL = 6
};

struct pj_str
{
    const char *str;
    size_t length;
};

/**
 * @brief Get the JSON token at the specified path.
 *
 * @param[in] str The JSON string.
 * @param[in] path The JSON path.
 * @param[out] token_len The length of the token.
 * @return int PICO_JSON_OK on success, negative error code on failure.
 */
int pj_get(struct pj_str str, const char *path, int *token_len);

/**
 * @brief Get the JSON token at the specified path.
 *
 * This function is used to retrieve a token from a JSON string based on the specified path.
 *
 * @param[in] str The JSON string.
 * @param[in] path The JSON path. like: "$.store.book[0].title"
 * @return Return the pj_str structure containing the token.
 *         If the token is not found, the str field will be NULL and length will be 0.
 */
struct pj_str pj_get_token(struct pj_str str, const char *path);

/**
 * @brief Get the type of the JSON value at the specified path.
 *
 * @param[in] j The JSON string.
 * @param[in] path The JSON path.
 * @return int The type of the JSON value (enum pj_type), or PJ_TYPE_INVALID on error.
 */
int pj_get_type(struct pj_str j, const char *path);

/**
 * @brief Get the JSON boolean value at the specified path.
 *
 * @param[in] str The JSON string.
 * @param[in] path The JSON path.
 * @param[out] value The boolean value.
 * @return true on success, false on failure.
 */
bool pj_get_bool(struct pj_str str, const char *path, bool *value);

/**
 * @brief Get the JSON number value at the specified path.
 *
 * @param[in] str The JSON string.
 * @param[in] path The JSON path.
 * @param[out] value The number value.
 * @return true on success, false on failure.
 */
bool pj_get_num(struct pj_str str, const char *path, double *value);

/**
 * @brief Get the JSON integer value at the specified path.
 *
 * @param[in] str The JSON string.
 * @param[in] path The JSON path.
 * @param[in] default_value The default value to return if the path is not found.
 * @return int The JSON integer value, or default_value if not found.
 */
int pj_get_int(struct pj_str str, const char *path, int default_value);

/**
 * @brief Get the JSON string value at the specified path.
 *
 * This function retrieves a string value from a JSON string based on the specified path.
 * The result is unescaped and allocated on the heap.
 * The caller is responsible for freeing the returned string.
 * If no heap memory is available, see @pj_get_str_n().
 *
 * @param[in] str The JSON string.
 * @param[in] path The JSON path.
 * @return char* The JSON string value, or NULL if not found.
 * @note The caller is responsible for freeing the returned string.
 */
char *pj_get_str(struct pj_str str, const char *path);

/**
 * @brief Get the JSON string value at the specified path.
 *
 * This function retrieves a string value from a JSON string based on the specified path.
 * The result is unescaped and copied into the provided buffer.
 *
 * @param[in] str The JSON string.
 * @param[in] path The JSON path.
 * @param[out] buf The buffer to store the string value.
 * @param[in] buf_size The size of the buffer.
 * @return true on success, false on failure.
 */
bool pj_get_str_n(struct pj_str str, const char *path, char *buf, size_t buf_size);

/**
 * @brief Get the next JSON token.
 *
 * @param[in] str The JSON string.
 * @param[in] ofst The offset to start searching from.
 * @param[out] key The key of the JSON token.
 * @param[out] val The value of the JSON token.
 * @return size_t The new offset after the token.
 */
size_t pj_next(struct pj_str str, size_t ofst, struct pj_str *key, struct pj_str *val);

/**
 * @brief Unescape a JSON string.
 *
 * @param[in] str The JSON string.
 * @param[out] buf The buffer to store the unescaped string.
 * @param[in] buf_len The length of the buffer.
 * @return true on success, false on failure.
 */
bool pj_unescape(struct pj_str str, char *buf, size_t buf_len);

/**
 * @brief Check if a JSON string is valid.
 *
 * @param[in] str The JSON string.
 * @return true on success, false on failure.
 */
bool pj_is_valid(struct pj_str str);

/**
 * @brief used for avoiding compiler warnings
 */
#define pj_str(s) pj_str_s(s)
struct pj_str pj_str_s(const char *str);
struct pj_str pj_str_n(const char *str, size_t len);

// Serialization functions with custom stream output
struct pj_stream_out
{
    void (*on_write)(const char *buf, size_t len, void *user);
    void *user;
};

/**
 * @brief Function to write formatted output to a stream.
 *
 * @param[in] out The stream output structure.
 * @param[in] fmt The format string.
 * @param[in] ap The variable argument list.
 * @return int The number of characters written or -1 on error.
 */
int pj_vstreamf(const struct pj_stream_out *out, const char *fmt, va_list ap);

/**
 * @brief Function to write formatted output to a stream.
 *
 * @param[in] out The stream output structure.
 * @param[in] fmt The format string.
 * @param[in] ... The values to format.
 * @return int The number of characters written or -1 on error.
 */
int pj_streamf(const struct pj_stream_out *out, const char *fmt, ...);

/**
 * @brief Function to write formatted output to a file stream.
 *
 * @param[in] fp The file pointer.
 * @param[in] fmt The format string.
 * @param[in] ... The values to format.
 */
void pj_fstream(FILE *fp, const char *fmt, ...);

// Useful macros for type checking
#define pj_is_object(json, path)  (pj_get_type(json, path) == PJ_TYPE_OBJECT)
#define pj_is_array(json, path)   (pj_get_type(json, path) == PJ_TYPE_ARRAY)
#define pj_is_string(json, path)  (pj_get_type(json, path) == PJ_TYPE_STRING)
#define pj_is_number(json, path)  (pj_get_type(json, path) == PJ_TYPE_NUMBER)
#define pj_is_boolean(json, path) (pj_get_type(json, path) == PJ_TYPE_BOOLEAN)
#define pj_is_null(json, path)    (pj_get_type(json, path) == PJ_TYPE_NULL)

#if defined(__cplusplus)
}
#endif

#endif // PICO_JSON_H_HEADER