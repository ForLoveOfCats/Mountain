usingnamespace @import("imports.zig");



pub const tKind = enum {
    Word,
    Colon,
    Period,
    Semicolon,
    Equals,
    CompEqual,
    CompNotEqual,
    CompGreater,
    CompGreaterEqual,
    CompLess,
    CompLessEqual,
    Plus,
    Minus,
    Multiply,
    Divide,
    Exclamation,
    AddressOf,
    Dereference,
    OpenParentheses,
    CloseParentheses,
    OpenBrace,
    CloseBrace,
    OpenBracket,
    CloseBracket,
    Comma,
    Hash,
};


pub const Token = struct {
    line: LineNumber,
    column_start: CharNumber,
    column_end: CharNumber,

    start: usize,
    end: usize,

    kind: tKind,
    string: []u8,
};


pub const StreamCodePoint8 = struct {
    bytes: [4]u8,
    len: usize,
    start: usize,
    end: usize,

    pub fn chars(self: StreamCodePoint8) []u8 {
        return self.bytes[0 .. self.len];
    }

    pub fn init(point: u32, start: usize) !StreamCodePoint8 {
        var bytes: [4]u8 = undefined;
        var len = try unicode.utf8Encode(point, bytes[0..4]);

        return StreamCodePoint8 {
            .bytes = bytes,
            .len = len,
            .start = start,
            .end = start + (len-1),
        };
    }
};


pub fn tokenize_file(source: []const u8, tokens: *std.ArrayList(Token)) !void {
    if(source.len <= 0) {
        return;
    }

    var iterator = unicode.Utf8Iterator {
        .bytes = source,
        .i = 0,
    };

    var line: usize = 1;
    var column: usize = 0;
    var token = Token {
        .line = newLineNumber(0),
        .column_start = newCharNumber(0),
        .column_end = newCharNumber(0),
        .start = 0,
        .end = 0,
        .kind = .Word,
        .string = [_]u8 {},
    };
    while(iterator.nextCodepoint()) |codepoint32| {
        var point = try StreamCodePoint8.init(codepoint32, iterator.i-1);

        column += 1;
        if(point.bytes[0] == '\n') {
            line += 1;
            column = 0;
        }
        else if(point.bytes[0] == '\t') { //Count a tab as four spaces
            column += 3; //We've already advanced one column
        }

        if(token.string.len <= 0) {
            token = Token {
                .line = newLineNumber(line),
                .column_start = newCharNumber(column),
                .column_end = newCharNumber(column),
                .start = point.start,
                .end = point.end,
                .kind = .Word,
                .string = [_]u8 {},
            };
        }
        token.column_end = newCharNumber(column);

        switch(point.bytes[0]) {
            ' ', '\t', '\n' => {
                if(token.string.len > 0) {
                    try tokens.append(token);
                    token = Token {
                        .line = newLineNumber(line),
                        .column_start = newCharNumber(column),
                        .column_end = newCharNumber(column),
                        .start = point.start,
                        .end = point.end,
                        .kind = .Word,
                        .string = [_]u8 {},
                    };
                }

                continue;
            },

            ':', '.', ';', ',', '#', '=', '>', '<', '+', '-', '*', '/', '!', '&', '$', '(', ')', '{', '}', '[', ']', => {
                if(point.bytes[0] == '/') comment: { //Lets check for a comment
                    var peeked = try StreamCodePoint8.init(iterator.nextCodepoint() orelse break :comment, iterator.i-1);
                    if(peeked.bytes[0] == '/') { //It is a comment, consume to end of line
                        while(iterator.nextCodepoint()) |consuming32| {
                            var consuming = try StreamCodePoint8.init(consuming32, iterator.i-1);
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
                        .column_start = newCharNumber(column),
                        .column_end = newCharNumber(column),
                        .start = point.start,
                        .end = point.end,
                        .kind = .Word,
                        .string = [_]u8 {},
                    };
                    column -= 1;
                    iterator.i -= point.len;
                }
                else {
                    var char_token = Token {
                        .line = newLineNumber(line),
                        .column_start = newCharNumber(column),
                        .column_end = newCharNumber(column+1),
                        .start = point.start,
                        .end = point.end,
                        .kind = switch(point.bytes[0]) {
                            ':' => .Colon,
                            '.' => .Period,
                            ';' => .Semicolon,
                            ',' => .Comma,
                            '#' => .Hash,
                            '=' => .Equals,
                            '>' => .CompGreater,
                            '<' => .CompLess,
                            '+' => .Plus,
                            '-' => .Minus,
                            '*' => .Multiply,
                            '/' => .Divide,
                            '!' => .Exclamation,
                            '&' => .AddressOf,
                            '$' => .Dereference,
                            '(' => .OpenParentheses,
                            ')' => .CloseParentheses,
                            '{' => .OpenBrace,
                            '}' => .CloseBrace,
                            '[' => .OpenBracket,
                            ']' => .CloseBracket,

                            else => unreachable,
                        },
                        .string = source[point.start .. point.end+1],
                    };

                    if(char_token.kind == .Equals) fail: {
                        var peeked = try StreamCodePoint8.init(iterator.nextCodepoint() orelse break :fail, iterator.i-1);
                        if(peeked.bytes[0] == '=') {
                            char_token.kind = .CompEqual;
                            char_token.column_end.number += 1; //We know we can't have gone to the next line
                            char_token.end += 1;
                            char_token.string = source[char_token.start .. peeked.end+1];
                        }
                        else {
                            iterator.i -= peeked.len;
                        }
                    }
                    else if(char_token.kind == .CompGreater) fail: {
                        var peeked = try StreamCodePoint8.init(iterator.nextCodepoint() orelse break :fail, iterator.i-1);
                        if(peeked.bytes[0] == '=') {
                            char_token.kind = .CompGreaterEqual;
                            char_token.column_end.number += 1; //We know we can't have gone to the next line
                            char_token.end += 1;
                            char_token.string = source[char_token.start .. peeked.end+1];
                        }
                        else {
                            iterator.i -= peeked.len;
                        }
                    }
                    else if(char_token.kind == .CompLess) fail: {
                        var peeked = try StreamCodePoint8.init(iterator.nextCodepoint() orelse break :fail, iterator.i-1);
                        if(peeked.bytes[0] == '=') {
                            char_token.kind = .CompLessEqual;
                            char_token.column_end.number += 1; //We know we can't have gone to the next line
                            char_token.end += 1;
                            char_token.string = source[char_token.start .. peeked.end+1];
                        }
                        else {
                            iterator.i -= peeked.len;
                        }
                    }
                    else if(char_token.kind == .Exclamation) fail: {
                        var peeked = try StreamCodePoint8.init(iterator.nextCodepoint() orelse break :fail, iterator.i-1);
                        if(peeked.bytes[0] == '=') {
                            char_token.kind = .CompNotEqual;
                            char_token.column_end.number += 1; //We know we can't have gone to the next line
                            char_token.end += 1;
                            char_token.string = source[char_token.start .. peeked.end+1];
                        }
                        else {
                            iterator.i -= peeked.len;
                        }
                    }

                    try tokens.append(char_token);
                }

                continue;
            },

            else => {},
        }

        token.string = source[token.start .. point.end+1];
    }
}
