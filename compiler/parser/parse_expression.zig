usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_expression(self: *TokenIterator) anyerror!*pExpression {
    var expression = try heap.c_allocator.create(pExpression);

    if(mem.eql(u8, self.token().string, "let")) {
        var let = try parse_let(self);
        expression.* = pExpression { .Let = let };
        return expression;
    }

    else {
        parse_error(self.token(), "Unexpected token '{}'", self.token().string);
    }
}
