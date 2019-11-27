usingnamespace @import("../imports.zig");

usingnamespace tokenizer;


pub fn expect_kind(token: Token, expected: tKind) void {
    if(token.kind != expected) {
        parse_error(token, "Expected '{}' but found '{}' instead", expected, token.kind);
    }
}

