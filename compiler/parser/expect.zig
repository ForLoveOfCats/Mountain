usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn expect_kind(token: Token, expected: tKind) void {
    if(token.kind != expected) {
        parse_error(token, "Expected {} but found {}", .{@tagName(expected), @tagName(token.kind)});
    }
}


pub fn expect_word(token: Token, expected: []const u8) void {
    if(!mem.eql(u8, token.string, expected)) {
        parse_error(token, "Expected '{}' but found '{}'", .{expected, token.string});
    }
}
