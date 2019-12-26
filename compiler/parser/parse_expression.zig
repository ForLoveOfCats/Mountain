usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_expression(self: *TokenIterator) anyerror!*pExpression {
    if(mem.eql(u8, self.token().string, "let")) {
        var let = try parse_let(self);
        return pExpression.init(pExpression{ .Let = let });
    }

    else {
        parse_error(self.token(), "Unexpected token '{}'", self.token().string);
    }
}