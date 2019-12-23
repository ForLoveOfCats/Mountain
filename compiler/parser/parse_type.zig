usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_type(self: *TokenIterator, child: ?*pType) anyerror!*pType {
    expect_kind(self.token(), .Word);

    var ptype = try heap.c_allocator.create(pType);
    if(self.has_next() and self.peek().kind == .Period) {
        var reach_module = self.token().string;

        self.next();
        expect_kind(self.token(), .Period); //Sanity check

        self.next();
        expect_kind(self.token(), .Word);
        ptype.* = pType {
            .name = self.token().string,
            .reach_module = reach_module,
            .child = child,
        };
    }
    else {
        ptype.* = pType {
            .name = self.token().string,
            .reach_module = [_]u8 {},
            .child = child,
        };
    }

    self.next();
    if(self.token().kind == .Colon and !(self.has_next() and self.peek().kind == .Colon)) {
        self.next();
        return try parse_type(self, ptype);
    }
    self.index -= 1;

    return ptype;
}
