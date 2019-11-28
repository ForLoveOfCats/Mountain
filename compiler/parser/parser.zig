usingnamespace @import("../imports.zig");

pub usingnamespace @import("parse_file.zig");
pub usingnamespace @import("parse_block.zig");
pub usingnamespace @import("parse_func.zig");
pub usingnamespace @import("parser_nodes.zig");
pub usingnamespace @import("expect.zig");



pub var modules: std.StringHashMap(parser.pModule) = undefined;


pub const TokenIterator = struct {
    tokens: []tokenizer.Token,
    index: usize,

    pub fn token(self: TokenIterator) tokenizer.Token {
        return self.tokens[self.index];
    }

    pub fn has_next(self: *TokenIterator) bool {
        return !(self.index+1 >= self.tokens.len);
    }

    pub fn next(self: *TokenIterator) void {
        if(!has_next(self)) {
            parse_error_file_line_column_start_end(
                self.tokens[self.index].file,
                self.tokens[self.index].line,
                self.tokens[self.index].column_end,
                self.tokens[self.index].start,
                self.tokens[self.index].end,
                "Unexpectedly encountered end of file"
            );
        }

        self.index += 1;
    }
};
