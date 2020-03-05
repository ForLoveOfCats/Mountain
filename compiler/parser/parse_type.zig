usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_type(self: *TokenIterator, child: ?*pType) anyerror!*pType {
    expect_kind(self.token, .Word);

    var ptype: *pType = try pType.init(
        self.token.string,
        child,
    );

    if(self.peek()) |peeked| {
        if(peeked.kind == .Colon) {
            self.next(); //The colon
            self.next();
            return try parse_type(self, ptype);
        }
    }

    return ptype;
}
