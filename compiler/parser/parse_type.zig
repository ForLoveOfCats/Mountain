usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_type(self: *TokenIterator, children: ?std.ArrayList(*pType)) anyerror!*pType {
    if(self.token.kind == .OpenBracket) {
        if(children != null) {
            parse_error(self.token, "Type set disallowed when not first element of type", .{});
        }

        self.next();
        expect_kind(self.token, .Word);

        var set = std.ArrayList(*pType).init(allocator);

        while (true) {
            try set.append(try parse_type(self, null));
            self.next();

            if(self.token.kind != .Comma) {
                expect_kind(self.token, .CloseBracket);
                break;
            }
            else {
                self.next();
            }
        }

        self.next();
        expect_kind(self.token, .Colon);

        self.next();
        return try parse_type(self, set);
    }

    expect_kind(self.token, .Word);

    var ptype: *pType = try pType.init(
        self.token.string,
        switch(children == null) {
            true => std.ArrayList(*pType).init(allocator),
            false => children.?,
        },
    );

    if(self.peek()) |peeked| {
        if(peeked.kind == .Colon) {
            self.next(); //The colon
            self.next();

            var parent_children = std.ArrayList(*pType).init(allocator);
            try parent_children.append(ptype);
            return try parse_type(self, parent_children);
        }
    }

    return ptype;
}
