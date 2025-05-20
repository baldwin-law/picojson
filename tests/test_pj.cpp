#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "pico_json.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Helper function: verify pj_str content
static bool str_equals(struct pj_str str, const char* expected) {
    return (str.length == strlen(expected) && 
            strncmp(str.str, expected, str.length) == 0);
}

TEST_CASE("pj_str creation functions", "[str]") {
    SECTION("pj_str_s - Normal string") {
        const char* test = "hello";
        struct pj_str s = pj_str_s(test);
        
        REQUIRE(s.str == test);
        REQUIRE(s.length == 5);
    }
    
    SECTION("pj_str_s - Empty string") {
        struct pj_str s = pj_str_s("");
        
        REQUIRE(s.str != NULL);
        REQUIRE(s.length == 0);
    }
    
    SECTION("pj_str_s - NULL") {
        struct pj_str s = pj_str_s(NULL);
        
        REQUIRE(s.str == NULL);
        REQUIRE(s.length == 0);
    }
    
    SECTION("pj_str_n - Specified length") {
        const char* test = "hello world";
        struct pj_str s = pj_str_n(test, 5);
        
        REQUIRE(s.str == test);
        REQUIRE(s.length == 5);
    }
    
    SECTION("pj_str_n - Zero length") {
        const char* test = "hello";
        struct pj_str s = pj_str_n(test, 0);
        
        REQUIRE(s.str == test);
        REQUIRE(s.length == 0);
    }
    
    SECTION("pj_str macro") {
        struct pj_str s = pj_str("test");
        REQUIRE(s.str != NULL);
        REQUIRE(s.length == 4);
    }
}

TEST_CASE("pj_is_valid - JSON validation", "[validation]") {
    SECTION("Basic types") {
        REQUIRE(pj_is_valid(pj_str("null")));
        REQUIRE(pj_is_valid(pj_str("true")));
        REQUIRE(pj_is_valid(pj_str("false")));
        REQUIRE(pj_is_valid(pj_str("123")));
        REQUIRE(pj_is_valid(pj_str("-456")));
        REQUIRE(pj_is_valid(pj_str("0.789")));
        REQUIRE(pj_is_valid(pj_str("\"string\"")));
    }
    
    SECTION("Object validation") {
        REQUIRE(pj_is_valid(pj_str("{}")));
        REQUIRE(pj_is_valid(pj_str("{\"a\":1}")));
        REQUIRE(pj_is_valid(pj_str("{\"a\":1,\"b\":2}")));
        REQUIRE(pj_is_valid(pj_str("{\"a\":{\"b\":2}}")));
        REQUIRE(pj_is_valid(pj_str("{\"a\":[1,2,3]}")));
        REQUIRE(pj_is_valid(pj_str("{\"a\":true,\"b\":false,\"c\":null}")));
    }
    
    SECTION("Array validation") {
        REQUIRE(pj_is_valid(pj_str("[]")));
        REQUIRE(pj_is_valid(pj_str("[1,2,3]")));
        REQUIRE(pj_is_valid(pj_str("[\"a\",\"b\",\"c\"]")));
        REQUIRE(pj_is_valid(pj_str("[{},{}]")));
        REQUIRE(pj_is_valid(pj_str("[[1,2],[3,4]]")));
        REQUIRE(pj_is_valid(pj_str("[true,false,null]")));
    }
    
    SECTION("Whitespace handling") {
        REQUIRE(pj_is_valid(pj_str(" { } ")));
        REQUIRE(pj_is_valid(pj_str("\n\t{\n\t\"key\": \"value\"\n}")));
        REQUIRE(pj_is_valid(pj_str(" [ 1 , 2 ] ")));
    }
    
    SECTION("Invalid JSON") {
        REQUIRE_FALSE(pj_is_valid(pj_str("")));
        REQUIRE_FALSE(pj_is_valid(pj_str("{")));
        REQUIRE_FALSE(pj_is_valid(pj_str("}")));
        REQUIRE_FALSE(pj_is_valid(pj_str("[")));
        REQUIRE_FALSE(pj_is_valid(pj_str("]")));
        REQUIRE_FALSE(pj_is_valid(pj_str("\"Unclosed string")));
        REQUIRE_FALSE(pj_is_valid(pj_str("{\"key\": value}")));
        REQUIRE_FALSE(pj_is_valid(pj_str("{\"key\":}")));
        REQUIRE_FALSE(pj_is_valid(pj_str("{,}")));
        REQUIRE_FALSE(pj_is_valid(pj_str("[,]")));
        REQUIRE_FALSE(pj_is_valid(pj_str("[1,]")));
        REQUIRE_FALSE(pj_is_valid(pj_str("1.")));
        REQUIRE_FALSE(pj_is_valid(pj_str("truee")));
    }
    
    SECTION("Depth limit test") {
        // Create a nested array exceeding the maximum depth
        std::string deep_json = "";
        for (int i = 0; i < PICO_JSON_MAX_DEPTH + 5; i++) {
            deep_json += "[";
        }
        for (int i = 0; i < PICO_JSON_MAX_DEPTH + 5; i++) {
            deep_json += "]";
        }
        REQUIRE_FALSE(pj_is_valid(pj_str_s(deep_json.c_str())));
    }
}

TEST_CASE("pj_get - JSON path query", "[get]") {
    const char* json_str = "{\"name\":\"John\",\"age\":30,\"active\":true,\"scores\":[10,20,30],\"address\":{\"city\":\"New York\"}}";
    struct pj_str json = pj_str(json_str);
    int token_len;
    
    SECTION("Get root node") {
        int pos = pj_get(json, "", &token_len);
        REQUIRE(pos == 0);
        REQUIRE(token_len == (int)strlen(json_str));
        
        // $ can also be used to get the root node
        pos = pj_get(json, "$", &token_len);
        REQUIRE(pos == 0);
    }
    
    SECTION("Get simple values") {
        int pos = pj_get(json, "$.name", &token_len);
        REQUIRE(pos > 0);
        REQUIRE(token_len == 6); // "John"
        
        pos = pj_get(json, "$.age", &token_len);
        REQUIRE(pos > 0);
        REQUIRE(token_len == 2); // 30
        
        pos = pj_get(json, "$.active", &token_len);
        REQUIRE(pos > 0);
        REQUIRE(token_len == 4); // true
    }
    
    SECTION("Get array values") {
        int pos = pj_get(json, "$.scores", &token_len);
        REQUIRE(pos > 0);
        REQUIRE(token_len > 0);
        
        pos = pj_get(json, "$.scores[0]", &token_len);
        REQUIRE(pos > 0);
        REQUIRE(token_len == 2); // 10
        
        pos = pj_get(json, "$.scores[1]", &token_len);
        REQUIRE(pos > 0);
        REQUIRE(token_len == 2); // 20
        
        pos = pj_get(json, "$.scores[2]", &token_len);
        REQUIRE(pos > 0);
        REQUIRE(token_len == 2); // 30
    }
    
    SECTION("Get nested objects") {
        int pos = pj_get(json, "$.address", &token_len);
        REQUIRE(pos > 0);
        REQUIRE(token_len > 0);
        
        pos = pj_get(json, "$.address.city", &token_len);
        REQUIRE(pos > 0);
        REQUIRE(token_len == 10); // "New York"
    }
    
    SECTION("Non-existent paths") {
        int pos = pj_get(json, "$.nonexistent", &token_len);
        REQUIRE(pos == PICO_JSON_NOT_FOUND);
        
        pos = pj_get(json, "$.scores[10]", &token_len);
        REQUIRE(pos == PICO_JSON_NOT_FOUND);
        
        pos = pj_get(json, "$.address.nonexistent", &token_len);
        REQUIRE(pos == PICO_JSON_NOT_FOUND);
    }
    
    SECTION("Invalid paths") {
        int pos = pj_get(json, ".name", &token_len); // Without $ also works
        REQUIRE(pos > 0);
        
        pos = pj_get(json, "invalid_path", &token_len);
        REQUIRE(pos == PICO_JSON_INVALID);
        
        pos = pj_get(json, "$.name[0]", &token_len); // String can't be accessed as array
        REQUIRE(pos == PICO_JSON_NOT_FOUND);
    }
}

TEST_CASE("pj_get_tok - Get token", "[get_tok]") {
    // Note: Implementation uses pj_get_tok instead of pj_get_token
    const char* json_str = "{\"name\":\"John\",\"scores\":[10,20,30]}";
    struct pj_str json = pj_str(json_str);
    
    SECTION("Get string value") {
        struct pj_str token = pj_get_token(json, "$.name");
        REQUIRE(token.str != NULL);
        REQUIRE(token.length == 6);
        REQUIRE(strncmp(token.str, "\"John\"", 6) == 0);
    }
    
    SECTION("Get array") {
        struct pj_str token = pj_get_token(json, "$.scores");
        REQUIRE(token.str != NULL);
        REQUIRE(token.length > 0);
        REQUIRE(token.str[0] == '[');
    }
    
    SECTION("Non-existent path") {
        struct pj_str token = pj_get_token(json, "$.nonexistent");
        REQUIRE(token.str == NULL);
        REQUIRE(token.length == 0);
    }
}

TEST_CASE("Type-specific getters", "[typed_getters]") {
    const char* json_str = "{\"str\":\"hello\",\"num\":123.456,\"int\":42,\"bool\":true,\"false_val\":false,\"null\":null,\"arr\":[1,2,3],\"obj\":{\"key\":\"value\"}}";
    struct pj_str json = pj_str(json_str);
    
    SECTION("pj_get_bool - Boolean values") {
        bool value;
        
        // Get true
        REQUIRE(pj_get_bool(json, "$.bool", &value));
        REQUIRE(value == true);
        
        // Get false
        REQUIRE(pj_get_bool(json, "$.false_val", &value));
        REQUIRE(value == false);
        
        // Non-boolean values
        REQUIRE_FALSE(pj_get_bool(json, "$.str", &value));
        REQUIRE_FALSE(pj_get_bool(json, "$.num", &value));
        REQUIRE_FALSE(pj_get_bool(json, "$.null", &value));
        REQUIRE_FALSE(pj_get_bool(json, "$.arr", &value));
        REQUIRE_FALSE(pj_get_bool(json, "$.obj", &value));
        REQUIRE_FALSE(pj_get_bool(json, "$.nonexistent", &value));
    }
    
    SECTION("pj_get_num - Numeric values") {
        double value;
        
        // Floating point
        REQUIRE(pj_get_num(json, "$.num", &value));
        REQUIRE(value == Catch::Approx(123.456));
        
        // Integer
        REQUIRE(pj_get_num(json, "$.int", &value));
        REQUIRE(value == Catch::Approx(42.0));
        
        // Non-numeric values
        REQUIRE_FALSE(pj_get_num(json, "$.str", &value));
        REQUIRE_FALSE(pj_get_num(json, "$.bool", &value));
        REQUIRE_FALSE(pj_get_num(json, "$.null", &value));
        REQUIRE_FALSE(pj_get_num(json, "$.arr", &value));
        REQUIRE_FALSE(pj_get_num(json, "$.obj", &value));
        REQUIRE_FALSE(pj_get_num(json, "$.nonexistent", &value));
    }
    
    SECTION("pj_get_int - Integer values") {
        // Integer
        int value = pj_get_int(json, "$.int", -1);
        REQUIRE(value == 42);
        
        // Floating point truncation
        value = pj_get_int(json, "$.num", -1);
        REQUIRE(value == 123);
        
        // Non-numeric values use default
        value = pj_get_int(json, "$.str", -1);
        REQUIRE(value == -1);
        
        value = pj_get_int(json, "$.bool", -1);
        REQUIRE(value == -1);
        
        value = pj_get_int(json, "$.null", -1);
        REQUIRE(value == -1);
        
        value = pj_get_int(json, "$.nonexistent", -1);
        REQUIRE(value == -1);
    }
    
    SECTION("pj_get_str - String values") {
        // Get string
        char* value = pj_get_str(json, "$.str");
        REQUIRE(value != NULL);
        REQUIRE(strcmp(value, "hello") == 0);
        free(value);
        
        // Non-string values
        value = pj_get_str(json, "$.num");
        REQUIRE(value == NULL);
        
        value = pj_get_str(json, "$.bool");
        REQUIRE(value == NULL);
        
        value = pj_get_str(json, "$.null");
        REQUIRE(value == NULL);
        
        value = pj_get_str(json, "$.arr");
        REQUIRE(value == NULL);
        
        value = pj_get_str(json, "$.obj");
        REQUIRE(value == NULL);
        
        value = pj_get_str(json, "$.nonexistent");
        REQUIRE(value == NULL);
    }
}

TEST_CASE("pj_unescape - String escape processing", "[unescape]") {
    SECTION("Basic escape sequences") {
        struct pj_str str = pj_str("hello\\nworld");
        char buffer[20];
        
        REQUIRE(pj_unescape(str, buffer, sizeof(buffer)));
        REQUIRE(strcmp(buffer, "hello\nworld") == 0);
    }
    
    SECTION("Multiple escape characters") {
        struct pj_str str = pj_str("\\\"hello\\t\\r\\n\\\\world\\\"");
        char buffer[30];
        
        REQUIRE(pj_unescape(str, buffer, sizeof(buffer)));
        REQUIRE(strcmp(buffer, "\"hello\t\r\n\\world\"") == 0);
    }
    
    SECTION("Unicode escapes") {
        struct pj_str str = pj_str("Unicode \\u2603 snowman");
        char buffer[30];
        
        REQUIRE(pj_unescape(str, buffer, sizeof(buffer)));
        // Simplified processing should include the \u2603 sequence
        REQUIRE(strstr(buffer, "\\u2603") != NULL);
    }
    
    SECTION("Buffer too small") {
        struct pj_str str = pj_str("String too long to fit in small buffer");
        char buffer[5];
        
        REQUIRE(pj_unescape(str, buffer, sizeof(buffer)));
        REQUIRE(strlen(buffer) == 4);  // 4 characters + null terminator
    }
}

TEST_CASE("pj_next - Object iteration", "[next]") {
    const char* json_str = "{\"name\":\"John\",\"age\":30,\"active\":true}";
    struct pj_str json = pj_str(json_str);
    
    SECTION("Iterate all key-value pairs") {
        struct pj_str key, value;
        size_t offset = 0;
        int count = 0;
        bool found_name = false;
        bool found_age = false;
        bool found_active = false;
        
        while ((offset = pj_next(json, offset, &key, &value)) != 0) {
            count++;
            
            if (str_equals(key, "name")) {
                found_name = true;
                REQUIRE(strncmp(value.str, "\"John\"", value.length) == 0);
            }
            
            if (str_equals(key, "age")) {
                found_age = true;
                REQUIRE(strncmp(value.str, "30", value.length) == 0);
            }
            
            if (str_equals(key, "active")) {
                found_active = true;
                REQUIRE(strncmp(value.str, "true", value.length) == 0);
            }
        }
        
        REQUIRE(count == 3);
        REQUIRE(found_name);
        REQUIRE(found_age);
        REQUIRE(found_active);
    }
    
    SECTION("Iterate empty object") {
        struct pj_str empty = pj_str("{}");
        struct pj_str key, value;
        
        REQUIRE(pj_next(empty, 0, &key, &value) == 0);
    }
    
    SECTION("NULL parameters") {
        struct pj_str key;
        size_t offset = 0;
        
        // key is NULL
        offset = pj_next(json, 0, NULL, &key);
        REQUIRE(offset > 0);
        
        // value is NULL
        offset = 0;
        offset = pj_next(json, 0, &key, NULL);
        REQUIRE(offset > 0);
        
        // Both are NULL
        offset = 0;
        offset = pj_next(json, 0, NULL, NULL);
        REQUIRE(offset > 0);
    }
    
    SECTION("Non-object input") {
        struct pj_str arr = pj_str("[1,2,3]");
        struct pj_str key, value;
        
        REQUIRE(pj_next(arr, 0, &key, &value) == 0);
        
        struct pj_str primitive = pj_str("123");
        REQUIRE(pj_next(primitive, 0, &key, &value) == 0);
    }
}

TEST_CASE("Integration test - Combined usage", "[integration]") {
    const char* json = "{\"name\":\"John\",\"age\":30,\"active\":true,\"scores\":[10,20,30]}";
    struct pj_str j = pj_str(json);
    
    SECTION("Access various data types") {
        // Get string
        char* name = pj_get_str(j, "$.name");
        REQUIRE(strcmp(name, "John") == 0);
        char buf[64];
        bool r = pj_get_str_n(j, "$.name", buf, sizeof(buf));
        REQUIRE(r);
        REQUIRE(strcmp(buf, "John") == 0);

        // Get number
        int age = pj_get_int(j, "$.age", -1);
        REQUIRE(age == 30);
        
        // Get boolean
        bool active = false;
        REQUIRE(pj_get_bool(j, "$.active", &active));
        REQUIRE(active == true);
        
        // Get array element
        int score = pj_get_int(j, "$.scores[1]", 0);
        REQUIRE(score == 20);
        
        // Free resources
        free(name);
    }
    
    SECTION("Iterate object") {
        struct pj_str obj = pj_get_token(j, "$");
        struct pj_str key, val;
        size_t pos = 0;
        int count = 0;
        
        while ((pos = pj_next(obj, pos, &key, &val)) > 0) {
            count++;
            REQUIRE(key.length > 0);
            REQUIRE(val.length > 0);
        }
        
        REQUIRE(count == 4); // name, age, active, scores
    }
    
    SECTION("Nested paths and error handling") {
        // Valid path
        int score0 = pj_get_int(j, "$.scores[0]", -1);
        REQUIRE(score0 == 10);
        
        // Invalid index
        int invalid_score = pj_get_int(j, "$.scores[5]", -1);
        REQUIRE(invalid_score == -1);
        
        // Invalid path
        char* invalid = pj_get_str(j, "$.nonexistent");
        REQUIRE(invalid == NULL);
    }
}

// Helper for stream tests
typedef struct
{
    char *buf;
    size_t capacity;
    size_t pos;
} test_buffer_t;

static void test_writer(const char *chunk, size_t len, void *user_data)
{
    test_buffer_t *buf = (test_buffer_t *)user_data;
    if (buf->pos + len < buf->capacity) {
        memcpy(buf->buf + buf->pos, chunk, len);
        buf->pos += len;
        buf->buf[buf->pos] = '\0';
    }
}

TEST_CASE("Stream output functions", "[stream]") {
    
    SECTION("pj_streamf - Basic formatting") {
        test_buffer_t buffer;
        buffer.buf = (char*)malloc(256);
        buffer.capacity = 256;
        buffer.pos = 0;
        memset(buffer.buf, 0, buffer.capacity);
        
        struct pj_stream_out out = {test_writer, &buffer};
        
        REQUIRE(pj_streamf(&out, "%s", "hello") == 5);
        REQUIRE(strcmp(buffer.buf, "hello") == 0);
        
        pj_streamf(&out, " %d %4.2f", 42, 3.14);
        REQUIRE(strcmp(buffer.buf, "hello 42 3.14") == 0);
        
        free(buffer.buf);
    }
    
    SECTION("pj_streamf - JSON formatting") {
        test_buffer_t buffer;
        buffer.buf = (char*)malloc(256);
        buffer.capacity = 256;
        buffer.pos = 0;
        memset(buffer.buf, 0, buffer.capacity);
        
        struct pj_stream_out out = {test_writer, &buffer};
        
        // Format a simple JSON object
        pj_streamf(&out, "{");
        pj_streamf(&out, "\"name\":\"%s\",", "John");
        pj_streamf(&out, "\"age\":%d,", 30);
        pj_streamf(&out, "\"active\":%s", "true");
        pj_streamf(&out, "}");
        
        // Verify the JSON is valid
        struct pj_str json = pj_str(buffer.buf);
        REQUIRE(pj_is_valid(json) == true);
        
        // Verify the JSON content
        char name_buf[20];
        REQUIRE(pj_get_str_n(json, "$.name", name_buf, sizeof(name_buf)) == true);
        REQUIRE(strcmp(name_buf, "John") == 0);
        
        int age = pj_get_int(json, "$.age", -1);
        REQUIRE(age == 30);
        
        free(buffer.buf);
    }
    
    SECTION("pj_fstream - File output") {
        // Write to a temporary file
        FILE *fp = tmpfile();
        REQUIRE(fp != NULL);
        
        pj_fstream(fp, "{\"test\":%d}", 123);
        
        // Seek to the beginning of the file
        rewind(fp);
        
        // Read the file content
        char buffer[50] = {0};
        size_t bytes_read = fread(buffer, 1, sizeof(buffer) - 1, fp);
        buffer[bytes_read] = '\0';
        
        // Verify the content
        REQUIRE(strcmp(buffer, "{\"test\":123}") == 0);
        
        fclose(fp);
    }
}

TEST_CASE("pj_get_type - Type detection", "[type]") {
    const char* json_str = "{"
        "\"obj\": {\"key\": \"value\"},"
        "\"arr\": [1, 2, 3],"
        "\"str\": \"hello\","
        "\"num\": 123.45,"
        "\"int\": 42,"
        "\"true\": true,"
        "\"false\": false,"
        "\"null\": null"
    "}";
    struct pj_str json = pj_str(json_str);
    
    SECTION("Object type detection") {
        REQUIRE(pj_get_type(json, "$.obj") == PJ_TYPE_OBJECT);
        REQUIRE(pj_is_object(json, "$.obj") == true);
        REQUIRE(pj_is_object(json, "$.arr") == false);
    }
    
    SECTION("Array type detection") {
        REQUIRE(pj_get_type(json, "$.arr") == PJ_TYPE_ARRAY);
        REQUIRE(pj_is_array(json, "$.arr") == true);
        REQUIRE(pj_is_array(json, "$.str") == false);
    }
    
    SECTION("String type detection") {
        REQUIRE(pj_get_type(json, "$.str") == PJ_TYPE_STRING);
        REQUIRE(pj_is_string(json, "$.str") == true);
        REQUIRE(pj_is_string(json, "$.num") == false);
    }
    
    SECTION("Number type detection") {
        REQUIRE(pj_get_type(json, "$.num") == PJ_TYPE_NUMBER);
        REQUIRE(pj_get_type(json, "$.int") == PJ_TYPE_NUMBER);
        REQUIRE(pj_is_number(json, "$.num") == true);
        REQUIRE(pj_is_number(json, "$.int") == true);
        REQUIRE(pj_is_number(json, "$.str") == false);
    }
    
    SECTION("Boolean type detection") {
        REQUIRE(pj_get_type(json, "$.true") == PJ_TYPE_BOOLEAN);
        REQUIRE(pj_get_type(json, "$.false") == PJ_TYPE_BOOLEAN);
        REQUIRE(pj_is_boolean(json, "$.true") == true);
        REQUIRE(pj_is_boolean(json, "$.false") == true);
        REQUIRE(pj_is_boolean(json, "$.num") == false);
    }
    
    SECTION("Null type detection") {
        REQUIRE(pj_get_type(json, "$.null") == PJ_TYPE_NULL);
        REQUIRE(pj_is_null(json, "$.null") == true);
        REQUIRE(pj_is_null(json, "$.num") == false);
    }
    
    SECTION("Invalid paths and types") {
        REQUIRE(pj_get_type(json, "$.nonexistent") == PJ_TYPE_INVALID);
        REQUIRE(pj_get_type(json, "invalid_path") == PJ_TYPE_INVALID);
        
        REQUIRE(pj_is_object(json, "$.nonexistent") == false);
        REQUIRE(pj_is_array(json, "$.nonexistent") == false);
        REQUIRE(pj_is_string(json, "$.nonexistent") == false);
        REQUIRE(pj_is_number(json, "$.nonexistent") == false);
        REQUIRE(pj_is_boolean(json, "$.nonexistent") == false);
        REQUIRE(pj_is_null(json, "$.nonexistent") == false);
    }
    
    SECTION("Empty JSON") {
        struct pj_str empty = pj_str("");
        REQUIRE(pj_get_type(empty, "$.any") == PJ_TYPE_INVALID);
    }
}