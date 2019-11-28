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

        if(self.index >= self.tokens.len) {
            var previous_index = self.index - 1;

            parse_error_file_line_column_start_end(
                self.tokens[previous_index].file,
                self.tokens[previous_index].line,
                self.tokens[previous_index].column_end,
                self.tokens[previous_index].start,
                self.tokens[previous_index].end,
                "Unexpectedly encountered end of file"
            );
        }
    }
};
