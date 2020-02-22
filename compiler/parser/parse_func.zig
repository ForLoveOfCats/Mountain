usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_func(self: *TokenIterator) anyerror!pFunc {
    expect_word(self.token, "func"); //Sanity check

    self.next();
    expect_kind(self.token, .Colon);

    self.next();
    expect_kind(self.token, .Word);
    const ptype = try parse_type(self, null);
    print("Type: ", .{});
    ptype.debug_print();
    println("", .{});

    self.next();
    expect_kind(self.token, .Word); //Name
    const name = self.token.string;
    println("Name: {}", .{name});

    self.next();
    expect_kind(self.token, .OpenParentheses);
    var open_count: usize = 0;
    while(true) {
        if(self.token.kind == .OpenParentheses) {
            open_count += 1;
        }
        else if(self.token.kind == .CloseParentheses) {
            open_count -= 1;
        }

        if(open_count <= 0) {
            break;
        }
        self.next();
    }
    assert(self.token.kind == .CloseParentheses);

    self.next();
    expect_kind(self.token, .OpenBrace);
    var block = pBlock.init();
    try parse_block(self, &block, false);

    return pFunc {
        .name = name,
        .ptype = ptype,
        .block = block,
    };
}
