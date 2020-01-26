usingnamespace @import("../imports.zig");

usingnamespace parser;



fn parse_int(string: []u8) !?big.Int {
    var int = try big.Int.init(heap.c_allocator);
    int.setString(10, string) catch {
        int.deinit();
        return null;
    };
    return int;
}


pub fn parse_expression(self: *TokenIterator) anyerror!*pExpression {
    if(try parse_int(self.token().string)) |int| {
        return pExpression.init(pExpression{ .Int = int });
    }

    else {
        parse_error(self.token(), "Unexpected token '{}'", self.token().string);
    }
}
