usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_block(self: *TokenIterator, block: *pBlock, global: bool) anyerror!void {
    if(!global) {
        expect_kind(self.token, .OpenBrace);
        self.next();
    }

    while(true) {
        //Short circut if reached the end of the block
        if(global and !self.more_left()) {
            return;
        }
        else if(!global and self.token.kind == .CloseBrace) {
            expect_kind(self.token, .CloseBrace); //Sanity check
            return;
        }

        //Parse the next item in block
        if(self.token.kind == .OpenBrace) {
            var child_block = pBlock.init();
            try parse_block(self, &child_block, false);
            try block.contents.append(pBlock.InBlock {.Block = child_block});
        }
        else if(mem.eql(u8, self.token.string, "func")) {
            var func = try parse_func(self);
            try block.contents.append(pBlock.InBlock {.Func = func});
        }
        else if(mem.eql(u8, self.token.string, "let")) {
            var let = try parse_let(self);
            try block.contents.append(pBlock.InBlock {.Let = let});
        }
        else {
            var expression = try parse_expression(self);
            try block.contents.append(pBlock.InBlock {.Expression = expression});

            self.next();
            expect_kind(self.token, .Semicolon);
        }

        if(self.more_left()) {
            self.next();
        }
        else if(!global) {
            parse_error(self.token, "Unexpected end of file", .{});
        }
    }
}
