usingnamespace @import("../imports.zig");

usingnamespace parser;
usingnamespace tokenizer;



pub fn parse_file(tokens: []Token) anyerror!void {
    for(tokens) |token| {
        expect_kind(token, .Word);
    }
}

