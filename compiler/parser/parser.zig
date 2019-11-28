usingnamespace @import("../imports.zig");

pub usingnamespace @import("parse_file.zig");
pub usingnamespace @import("parser_nodes.zig");
pub usingnamespace @import("expect.zig");



pub var modules: std.StringHashMap(parser.pModule) = undefined;


pub const TokenIterator = struct {
    tokens: []tokenizer.Token,
    index: usize,

    pub fn token(self: TokenIterator) tokenizer.Token {
        return self.tokens[self.index];
    }

    pub fn next(self: *TokenIterator) void {
        self.index += 1;
    }
};
