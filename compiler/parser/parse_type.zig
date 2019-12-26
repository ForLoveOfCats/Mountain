usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_type(self: *TokenIterator, child: ?*pType) anyerror!*pType {
    expect_kind(self.token(), .Word);

    var maybe_ptype: ?*pType = null;
    if(self.has_next() and self.peek().kind == .Period) {
        var reach_module = self.token().string;

        self.next();
        expect_kind(self.token(), .Period); //Sanity check

        self.next();
        expect_kind(self.token(), .Word);
        maybe_ptype = try pType.init(
            self.token().string,
            reach_module,
            child,
        );
    }
    else {
        maybe_ptype = try pType.init(
            self.token().string,
            [_]u8 {},
            child,
        );
    }

    if(maybe_ptype) |ptype| {
        self.next();
        if(self.token().kind == .Colon and !(self.has_next() and self.peek().kind == .Colon)) {
            self.next();
            return try parse_type(self, ptype);
        }
        self.index -= 1;

        return ptype;
    }
    else {
        internal_error("`parse_type` failed to init the pType value");
    }
}
