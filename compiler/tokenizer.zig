const std = @import("std");
const mem = std.mem;
const io = std.io;
const unicode = std.unicode;

usingnamespace @import("utils.zig");



pub const Token = struct {
    line: LineNumber,
    start: CharNumber,
    end: CharNumber,
};


pub fn tokenize_file(allocator: *mem.Allocator, path: []const u8) anyerror![]Token {
    var tokens = std.ArrayList(Token).init(allocator);

    var source = try io.readFileAlloc(allocator, path);
    defer allocator.free(source);
    if(source.len <= 0) {
        return tokens.toOwnedSlice();
    }

    var iterator = unicode.Utf8Iterator {
        .bytes = source,
        .i = 0,
    };

    while(iterator.nextCodepoint()) |codepoint| {
        var point = try CodePoint8.init(codepoint);
        print("{}", point.chars());
    }

    return tokens.toOwnedSlice();
}
