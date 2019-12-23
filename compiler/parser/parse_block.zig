usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_block(self: *TokenIterator, block: *pBlock, global: bool) anyerror!void {
    if(!global) {
        expect_kind(self.token(), .OpenBrace);
        self.next();
    }

    while(true) {
        if(mem.eql(u8, self.token().string, "func")) {
            var func = try parse_func(self);
            try block.contents.append(pBlock.InBlock {.Func = func});
        }

        else {
            var expression = try parse_expression(self);
            try block.contents.append(pBlock.InBlock {.Expression = expression});

            self.next();
            expect_kind(self.token(), .Semicolon);
        }

        if(global and !self.has_next()) {
            return;
        }
        else if(!global and self.peek().kind == .CloseBrace) {
            self.next();
            expect_kind(self.token(), .CloseBrace); //Sanity check
            return;
        }
        self.next();
    }
}
