# Pico JSON

A lightweight, fast, and easy-to-use JSON parser and generator for C/C++ applications, written in C99.

## Features

- **Lightweight**: Minimal memory footprint, perfect for embedded systems
- **Fast**: Optimized parsing and serialization algorithms
- **Easy to use**: Simple and intuitive API
- **No dependencies**: Self-contained library
- **Cross-platform**: Works on Windows, Linux, macOS, and various embedded platforms
- **UTF-8 support**: Full support for Unicode characters
- **Compliant**: Passes all JSON standard compliance tests
- **Stream-based output**: Generate JSON directly to files or custom outputs

## Requirements

- C99 compatible compiler
- CMake 3.10+ (for building)

## Installation

### Using CMake

```bash
mkdir build && cd build
cmake ..
make
```

### Manual Integration

Include the header and source files in your project:

```c
#include "pico_json.h"
```

And compile with `pico_json.c`.

## Usage Examples

### Parsing JSON

```c
#include "pico_json.h"

int main() {
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
    struct pj_str json = pj_str(json_data);             // Using macro
    struct pj_str json2 = pj_str_s(json_data);          // Using function
    struct pj_str json3 = pj_str_n(json_data, strlen(json_data)); // Create with specified length
    
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
        free(name);  // Remember to free memory
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
        printf("Key: %.*s, Value: %.*s\n", 
               (int)key.length, key.str, 
               (int)value.length, value.str);
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
    return 0;
}
```

### Creating JSON with Stream Output

Stream-based output allows you to generate JSON directly to a destination (memory buffer, file, network socket, etc.).

```c
#include "pico_json.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // 1. Stream to a memory buffer using a custom callback
    char buffer[1024] = {0};
    size_t buffer_pos = 0;
    
    // Define a callback function to write to the buffer
    void memory_writer(const char *chunk, size_t len, void *user_data) {
        size_t *pos = (size_t*)user_data;
        memcpy(buffer + *pos, chunk, len);
        *pos += len;
        buffer[*pos] = '\0';  // Ensure null-termination
    }
    
    // Create a stream output structure
    struct pj_stream_out mem_out = {memory_writer, &buffer_pos};
    
    // Generate JSON using streaming functions
    pj_streamf(&mem_out, "{");
    pj_streamf(&mem_out, "\"name\":\"%s\",", "John Doe");
    pj_streamf(&mem_out, "\"age\":%d,", 30);
    pj_streamf(&mem_out, "\"is_active\":%s,", "true");
    
    // Nested object
    pj_streamf(&mem_out, "\"address\":{");
    pj_streamf(&mem_out, "\"street\":\"%s\",", "123 Main St");
    pj_streamf(&mem_out, "\"city\":\"%s\"", "Anytown");
    pj_streamf(&mem_out, "}");
    
    pj_streamf(&mem_out, "}");
    
    printf("Generated JSON: %s\n", buffer);
    
    // 2. Stream directly to a file using convenience function
    FILE *fp = fopen("output.json", "w");
    if (fp) {
        // Generate JSON directly to the file
        pj_fstream(fp, "{\"name\":\"%s\",\"age\":%d}", "Jane Smith", 28);
        fclose(fp);
    }
    
    return 0;
}
```

## API Documentation

### Basic Types

```c
struct pj_str {
    const char *str;
    size_t length;
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
```

### String Creation Functions

- `pj_str(s)` - Macro to create a pj_str from a C string
- `pj_str_s(const char *str)` - Create a pj_str from a C string
- `pj_str_n(const char *str, size_t len)` - Create a pj_str with specified length

### JSON Path Navigation

- `pj_get(struct pj_str str, const char *path, int *token_len)` - Get position of a value
- `pj_get_token(struct pj_str str, const char *path)` - Get a token as a pj_str

### Type Detection

- `pj_get_type(struct pj_str j, const char *path)` - Get the type of a JSON value (returns enum pj_type)
- `pj_is_object(json, path)` - Check if the value at path is an object
- `pj_is_array(json, path)` - Check if the value at path is an array
- `pj_is_string(json, path)` - Check if the value at path is a string
- `pj_is_number(json, path)` - Check if the value at path is a number
- `pj_is_boolean(json, path)` - Check if the value at path is a boolean
- `pj_is_null(json, path)` - Check if the value at path is null

### Extracting Values

- `pj_get_bool(struct pj_str str, const char *path, bool *value)` - Get boolean value
- `pj_get_num(struct pj_str str, const char *path, double *value)` - Get numeric value
- `pj_get_int(struct pj_str str, const char *path, int default_value)` - Get integer with default
- `pj_get_str(struct pj_str str, const char *path)` - Get string (caller must free)
- `pj_get_str_n(struct pj_str str, const char *path, char *buf, size_t buf_size)` - Get string into buffer

### Stream Output Functions

```c
struct pj_stream_out {
    void (*on_write)(const char *buf, size_t len, void *user);
    void *user;
};
```

- `pj_streamf(const struct pj_stream_out *out, const char *fmt, ...)` - Format and write to stream
- `pj_vstreamf(const struct pj_stream_out *out, const char *fmt, va_list ap)` - Format with va_list
- `pj_fstream(FILE *fp, const char *fmt, ...)` - Convenience function for file output

### Utilities

- `pj_is_valid(struct pj_str str)` - Check if JSON is valid
- `pj_next(struct pj_str str, size_t ofst, struct pj_str *key, struct pj_str *val)` - Iterate through objects or arrays
- `pj_unescape(struct pj_str str, char *buf, size_t buf_len)` - Unescape JSON string

## Performance

Pico JSON is designed for optimal performance with minimal overhead:

- Parsing: ~200MB/s
- Stream output: Very efficient with minimal memory usage
- Memory usage: Minimal dynamic allocations

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request
