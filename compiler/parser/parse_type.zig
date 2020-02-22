usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_type(self: *TokenIterator, child: ?*pType) anyerror!*pType {
    expect_kind(self.token, .Word);

    var maybe_ptype: ?*pType = null;
    if(self.peek()) |peeked| {
        if(peeked.kind == .Period) {
            var reach_module = self.token.string;

            self.next();
            expect_kind(self.token, .Period); //Sanity check

            self.next();
            expect_kind(self.token, .Word);
            maybe_ptype = try pType.init(
                self.token.string,
                reach_module,
                child,
            );
        }
    }

    if(maybe_ptype == null) {
        maybe_ptype = try pType.init(
            self.token.string,
            "",
            child,
        );
    }

    if(maybe_ptype) |ptype| {
        if(self.peek()) |peeked| {
            if(peeked.kind == .Colon) {
                self.next(); //The colon
                self.next();
                return try parse_type(self, ptype);
            }
        }

        return ptype;
    }
    else {
        internal_error("`parse_type` failed to init the pType value", .{});
    }
}
