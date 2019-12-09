usingnamespace @import("../imports.zig");

usingnamespace parser;
usingnamespace tokenizer;



pub fn parse_block(self: *TokenIterator, block: *pBlock, global: bool) anyerror!void {
    if(!global) {
        expect_kind(self.token(), .OpenBrace);
        self.next();
    }

    if(mem.eql(u8, self.token().string, "func")) {
        var func = try parse_func(self);
        try block.contents.append(pBlock.InBlock {.Func = func});
    }

    else if(mem.eql(u8, self.token().string, "let")) {
        var let = try parse_let(self);
        try block.contents.append(pBlock.InBlock {.Let = let});
    }

    else {
        parse_error(self.token(), "Unexpected token '{}'", self.token().string);
    }
}