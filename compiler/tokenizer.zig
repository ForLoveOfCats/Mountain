usingnamespace @import("imports.zig");



pub const Token = struct {
    line: LineNumber,
    start: CharNumber,
    end: CharNumber,
    string: []u8,
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

    var line: i32 = 1;
    var column: i32 = 0;
    var token = Token {
        .line = newLineNumber(0),
        .start = newCharNumber(0),
        .end = newCharNumber(0),
        .string = [_]u8 {},
    };
    while(iterator.nextCodepoint()) |codepoint32| {
        var point = try CodePoint8.init(codepoint32);

        column += 1;
        if(point.bytes[0] == '\n') {
            line += 1;
            column = 0;
        }
        else if(point.bytes[0] == '\t') { //Count a tab as four spaces
            column += 3; //We've already advanced one column
        }

        if(token.string.len <= 0) {
            token = Token  {
                .line = newLineNumber(line),
                .start = newCharNumber(column),
                .end = newCharNumber(column),
                .string = [_]u8 {},
            };
        }
        token.end = newCharNumber(column);

        switch(point.bytes[0]) {
            ' ', '\t', '\n' => {
                if(token.string.len > 0) {
                    try tokens.append(token);
                    token = Token  {
                        .line = newLineNumber(line),
                        .start = newCharNumber(column),
                        .end = newCharNumber(column),
                        .string = [_]u8 {},
                    };
                }

                continue;
            },

            ':', '.', ';', ',', '#', '=', '>', '<', '+', '-', '*', '/', '!', '&', '$', '(', ')', '{', '}', '[', ']', => {
                if(point.bytes[0] == '/') comment: { //Lets check for a comment
                    var peeked = try CodePoint8.init(iterator.nextCodepoint() orelse break :comment);
                    if(peeked.bytes[0] == '/') { //It is a comment, consume to end of line
                        while(iterator.nextCodepoint()) |consuming32| {
                            var consuming = try CodePoint8.init(consuming32);
                            if(consuming.bytes[0] == '\n') { //We've reached the end of the line
                                break;
                            }
                        }
                        line += 1;
                        column = 0;
                        continue;
                    } else { //not a comment, rewind the iterator
                        iterator.i -= peeked.len;
                    }
                }

                if(token.string.len > 0) {
                    try tokens.append(token);
                    token = Token {
                        .line = newLineNumber(line),
                        .start = newCharNumber(column),
                        .end = newCharNumber(column),
                        .string = [_]u8 {},
                    };
                    column -= 1;
                    iterator.i -= point.len;
                }
                else {
                    try tokens.append(Token {
                        .line = newLineNumber(line),
                        .start = newCharNumber(column),
                        .end = newCharNumber(column),
                        .string = try mem.dupe(allocator, u8, point.chars()),
                    });
                }

                continue;
            },

            else => {},
        }

        var new_string = try mem.concat(allocator, u8, [_] []u8 { token.string, point.chars() });
        allocator.free(token.string);
        token.string = new_string;
    }

    return tokens.toOwnedSlice();
}
