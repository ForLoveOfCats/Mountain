usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_let(self: *TokenIterator) anyerror!pLet {
    expect_word(self.token(), "let"); //Sanity check

    self.next();
    expect_kind(self.token(), .Colon);

    self.next();
    expect_kind(self.token(), .Word);
    const ptype = try parse_type(self, null);

    self.next();
    expect_kind(self.token(), .Word); //Name
    const name = self.token().string;

    self.next();
    expect_kind(self.token(), .Equals);

    self.next();
    var expression: ?*pExpression = null;
    if(!mem.eql(u8, self.token().string, "undefined")) {
        expression = try parse_expression(self);
    }

    self.next();
    expect_kind(self.token(), .Semicolon);

    return pLet {
        .name = name,
        .ptype = ptype,
        .expression = expression,
    };
}
