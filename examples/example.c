#include "pico_json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Example 1: Stream to memory buffer
char buffer[512] = {0};
size_t buffer_pos = 0;

// Callback function for memory stream
// You can directly use this function to write to a file or other output like stdout or network
void memory_writer(const char *chunk, size_t len, void *user_data)
{
    size_t *pos = (size_t *)user_data;
    if (*pos + len < sizeof(buffer)) {
        memcpy(buffer + *pos, chunk, len);
        *pos += len;
        buffer[*pos] = '\0'; // Ensure null-termination
    }
}

// Demo of JSON streaming output
void json_streaming_demo()
{
    printf("\n=== JSON Streaming Output Example ===\n");

    // Create stream output
    struct pj_stream_out mem_out = {memory_writer, &buffer_pos};

    // Generate a JSON object with different data types
    pj_streamf(&mem_out, "{");
    pj_streamf(&mem_out, "\"name\":\"%s\",", "John Doe");
    pj_streamf(&mem_out, "\"age\":%d,", 30);
    pj_streamf(&mem_out, "\"height\":%.2f,", 1.85);
    pj_streamf(&mem_out, "\"is_active\":%s,", "true");

    // Nested object
    pj_streamf(&mem_out, "\"address\":{");
    pj_streamf(&mem_out, "\"street\":\"%s\",", "123 Main St");
    pj_streamf(&mem_out, "\"city\":\"%s\",", "Anytown");
    pj_streamf(&mem_out, "\"zipcode\":%d", 12345);
    pj_streamf(&mem_out, "},");

    // Array
    pj_streamf(&mem_out, "\"phones\":[");
    pj_streamf(&mem_out, "\"%s\",", "123-456-7890");
    pj_streamf(&mem_out, "\"%s\"", "098-765-4321");
    pj_streamf(&mem_out, "]");

    pj_streamf(&mem_out, "}");

    printf("Generated JSON:\n%s\n\n", buffer);

    // Example 2: Stream to file
    FILE *fp = fopen("output.json", "w");
    if (fp) {
        printf("Streaming JSON to file 'output.json'...\n");

        // Directly stream to file using convenience function
        pj_fstream(fp, "{");
        pj_fstream(fp, "\"device\":\"%s\",", "temperature_sensor");
        pj_fstream(fp, "\"readings\":[");

        // Generate multiple readings
        for (int i = 0; i < 5; i++) {
            pj_fstream(fp,
                       "{\"temp\":%.1f,\"time\":%d}%s",
                       20.0 + (rand() % 100) / 10.0, // Random temperature
                       1600000000 + i * 3600,        // Unix timestamp
                       (i < 4) ? "," : "");          // Comma except for last item
        }

        pj_fstream(fp, "],");
        pj_fstream(fp, "\"status\":\"%s\"", "normal");
        pj_fstream(fp, "}");

        fclose(fp);
        printf("JSON data has been written to output.json\n");
    }

    // Verify generated memory JSON is valid
    struct pj_str json = pj_str(buffer);
    if (pj_is_valid(json)) {
        printf("Generated JSON is valid. Extracting data...\n");

        // Extract data from the generated JSON
        char name[50];
        if (pj_get_str_n(json, "$.name", name, sizeof(name))) {
            printf("Name: %s\n", name);
        }

        int age = pj_get_int(json, "$.age", -1);
        printf("Age: %d\n", age);

        char city[50];
        if (pj_get_str_n(json, "$.address.city", city, sizeof(city))) {
            printf("City: %s\n", city);
        }

        char phone[50];
        if (pj_get_str_n(json, "$.phones[0]", phone, sizeof(phone))) {
            printf("First phone: %s\n", phone);
        }
    } else {
        printf("Generated JSON is invalid!\n");
    }
}

// After line 187, before json_streaming_demo()
void type_checking_demo(void)
{
    printf("\n=== Type Checking Demo ===\n");
    
    const char *json_data = "{"
        "\"object\": {\"nested\": \"value\"},"
        "\"array\": [1, 2, 3],"
        "\"string\": \"hello world\","
        "\"number\": 42.5,"
        "\"integer\": 100,"
        "\"boolean\": true,"
        "\"is_null\": null"
    "}";
    
    struct pj_str json = pj_str(json_data);
    printf("JSON: %s\n\n", json_data);
    
    // Use pj_get_type to get the type of a JSON value
    printf("Type detection using pj_get_type:\n");
    printf("object type: %d\n", pj_get_type(json, "$.object"));
    printf("array type: %d\n", pj_get_type(json, "$.array"));
    printf("string type: %d\n", pj_get_type(json, "$.string"));
    printf("number type: %d\n", pj_get_type(json, "$.number"));
    printf("boolean type: %d\n", pj_get_type(json, "$.boolean"));
    printf("null type: %d\n", pj_get_type(json, "$.is_null"));
    printf("nonexistent type: %d\n", pj_get_type(json, "$.nonexistent"));
    
    // Using type checking macros
    printf("\nType checking using macros:\n");
    printf("Is object an object? %s\n", pj_is_object(json, "$.object") ? "Yes" : "No");
    printf("Is array an array? %s\n", pj_is_array(json, "$.array") ? "Yes" : "No");
    printf("Is string a string? %s\n", pj_is_string(json, "$.string") ? "Yes" : "No");
    printf("Is number a number? %s\n", pj_is_number(json, "$.number") ? "Yes" : "No");
    printf("Is integer a number? %s\n", pj_is_number(json, "$.integer") ? "Yes" : "No");
    printf("Is boolean a boolean? %s\n", pj_is_boolean(json, "$.boolean") ? "Yes" : "No");
    printf("Is null a null? %s\n", pj_is_null(json, "$.is_null") ? "Yes" : "No");
    
    // Branching based on type
    printf("\nConditional processing based on type:\n");
    const char *paths[] = {"$.object", "$.array", "$.string", "$.number", "$.boolean", "$.is_null", "$.nonexistent"};
    
    for (int i = 0; i < sizeof(paths)/sizeof(paths[0]); i++) {
        printf("Processing %s: ", paths[i]);
        
        if (pj_is_object(json, paths[i])) {
            printf("Found an object\n");
        } 
        else if (pj_is_array(json, paths[i])) {
            printf("Found an array with elements\n");
        }
        else if (pj_is_string(json, paths[i])) {
            char *str_value = pj_get_str(json, paths[i]);
            printf("Found a string: \"%s\"\n", str_value);
            free(str_value);
        }
        else if (pj_is_number(json, paths[i])) {
            double num_value;
            pj_get_num(json, paths[i], &num_value);
            printf("Found a number: %g\n", num_value);
        }
        else if (pj_is_boolean(json, paths[i])) {
            bool bool_value;
            pj_get_bool(json, paths[i], &bool_value);
            printf("Found a boolean: %s\n", bool_value ? "true" : "false");
        }
        else if (pj_is_null(json, paths[i])) {
            printf("Found a null value\n");
        }
        else {
            printf("Path not found or invalid type\n");
        }
    }
}

int main()
{
    // Example JSON data
    const char *json_data = "{"
                            "\"name\": \"John Doe\","
                            "\"age\": 30,"
                            "\"is_active\": true,"
                            "\"height\": 1.85,"
                            "\"address\": {"
                            "\"street\": \"123 Main St\","
                            "\"city\": \"Anytown\","
                            "\"zipcode\": 12345"
                            "},"
                            "\"phones\": ["
                            "\"123-456-7890\","
                            "\"098-765-4321\""
                            "],"
                            "\"empty_array\": [],"
                            "\"escaped_string\": \"Line1\\nLine2\\tTabbed\\\"Quoted\\\"\""
                            "}";

    printf("=== pico_json Usage Example ===\n\n");
    printf("json:%s\n", json_data);

    // 1. Three ways to create a pj_str structure
    struct pj_str json = pj_str(json_data);                       // Using macro

    // 2. Validate JSON validity
    if (pj_is_valid(json)) {
        printf("JSON format is valid\n\n");
    } else {
        printf("JSON format is invalid!\n");
        return 1;
    }

    // 3. Use pj_get to retrieve values of basic types
    int token_len = 0;
    int pos = pj_get(json, "$.name", &token_len);
    if (pos >= 0) {
        printf("name field is at offset %d, length %d\n", pos, token_len);
    }

    // 4. Get boolean value
    bool is_active = false;
    if (pj_get_bool(json, "$.is_active", &is_active)) {
        printf("is_active: %s\n", is_active ? "true" : "false");
    } else {
        printf("Failed to get is_active\n");
    }

    // 5. Get numeric value
    double height = 0.0;
    if (pj_get_num(json, "$.height", &height)) {
        printf("height: %.2f\n", height);
    }

    // 6. Get integer (with default value)
    int age = pj_get_int(json, "$.age", -1);
    printf("age: %d\n", age);

    // Get non-existent field, using default value
    int missing = pj_get_int(json, "$.missing_field", -999);
    printf("missing_field (default value): %d\n", missing);

    // 7. Get string
    char *name = pj_get_str(json, "$.name");
    if (name) {
        printf("name: %s\n", name);
        free(name); // Remember to free memory
    }

    // 8. Get string using a specified buffer
    char city_buf[50];
    if (pj_get_str_n(json, "$.address.city", city_buf, sizeof(city_buf))) {
        printf("city: %s\n", city_buf);
    }

    // 9. Access nested objects
    int zipcode = pj_get_int(json, "$.address.zipcode", -1);
    printf("zipcode: %d\n", zipcode);

    // 10. Access array elements
    char *phone1 = pj_get_str(json, "$.phones[0]");
    if (phone1) {
        printf("First phone: %s\n", phone1);
        free(phone1);
    }

    char *phone2 = pj_get_str(json, "$.phones[1]");
    if (phone2) {
        printf("Second phone: %s\n", phone2);
        free(phone2);
    }

    // 11. Use pj_get_token to get the original token
    struct pj_str address_token = pj_get_token(json, "$.address");
    if (address_token.str) {
        printf("\naddress object: %.*s\n", (int)address_token.length, address_token.str);
    }

    // 12. Use pj_next to iterate through key-value pairs of an object
    printf("\nIterating through address object key-value pairs:\n");
    struct pj_str key, value;
    size_t offset = 0;

    while ((offset = pj_next(address_token, offset, &key, &value)) != 0) {
        printf("Key: %.*s, Value: %.*s\n", (int)key.length, key.str, (int)value.length, value.str);
    }

    // 13. Iterate through array elements
    printf("\nIterating through phones array:\n");
    struct pj_str phones_token = pj_get_token(json, "$.phones");
    offset = 0;
    int index = 0;

    while ((offset = pj_next(phones_token, offset, NULL, &value)) != 0) {
        printf("Index %d: %.*s\n", index++, (int)value.length, value.str);
    }

    // 14. Process escaped strings
    struct pj_str escaped = pj_get_token(json, "$.escaped_string");
    char unescaped[100];
    if (pj_unescape(escaped, unescaped, sizeof(unescaped))) {
        printf("\nUnescaped string:\n%s\n", unescaped);
    }

    // Add type checking demo before streaming demo
    type_checking_demo();

    // Add streaming demo at the end of the main function
    json_streaming_demo();

    return 0;
}