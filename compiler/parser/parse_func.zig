usingnamespace @import("../imports.zig");

usingnamespace parser;
usingnamespace tokenizer;



pub fn parse_func(self: *TokenIterator, func: *pFunc) anyerror!void {
    expect_word(self.token(), "func"); //Sanity check

    self.next();
    expect_kind(self.token(), .Colon);

    self.next();
    expect_kind(self.token(), .Word);

    self.next();
    expect_kind(self.token(), .Word); //Name
    func.name = self.token().string;
}
