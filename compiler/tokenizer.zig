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

    while(iterator.nextCodepoint()) |point| {
        var chars: [4]u8 = undefined;
        var length = try unicode.utf8Encode(point, chars[0..chars.len]);
        print("{}", chars[0..length]);
    }

    return tokens.toOwnedSlice();
}
