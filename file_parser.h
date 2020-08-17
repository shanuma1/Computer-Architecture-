

// parse status
enum status {
    SUCCESS,
    FAILURE,
    END_OF_FILE
};

struct ParseResult {
    enum status parse_status;
    int position;
    int value;
};


struct ParseResult is_digits(char *c, int pos, int len);
struct opcodes *parse_opcodes(char *contents, int len);
