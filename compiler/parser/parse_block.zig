usingnamespace @import("../imports.zig");

usingnamespace parser;
usingnamespace tokenizer;



pub fn parse_block(self: *TokenIterator, block: *pBlock, global: bool) anyerror!void {
    if(!global) {
        expect_kind(self.token(), .OpenBracket);
        self.next();
    }

    if(mem.eql(u8, self.token().string, "func")) {
        var func = pFunc {
            .name = [_]u8 {},
        };
        try parse_func(self, &func);
        try block.contents.append(pBlock.InBlock {.Func = func});
    }
}
