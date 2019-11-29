usingnamespace @import("../imports.zig");

usingnamespace parser;
usingnamespace tokenizer;



pub fn parse_func(self: *TokenIterator) anyerror!pFunc {
    expect_word(self.token(), "func"); //Sanity check

    self.next();
    expect_kind(self.token(), .Colon);

    self.next();
    expect_kind(self.token(), .Word);
    var ptype = try parse_type(self, null);

    self.next();
    expect_kind(self.token(), .Word); //Name
    var name = self.token().string;

    return pFunc {
        .name = name,
        .ptype = ptype,
    };
}
