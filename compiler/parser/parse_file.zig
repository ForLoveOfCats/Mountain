usingnamespace @import("../imports.zig");

usingnamespace tokenizer;



pub fn parse_file(tokens: []Token) anyerror!void {
    for(tokens) |token| {
        println("'{}' of {}", token.string, token.kind);
    }
}

