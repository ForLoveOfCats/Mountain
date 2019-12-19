usingnamespace @import("../imports.zig");

usingnamespace parser;
usingnamespace tokenizer;



pub fn parse_file(self: *TokenIterator) !void {
    if(self.tokens.len == 0) {
        return; //Empty file
    }

    try parse_block(self, &main_block, true);
}
