usingnamespace @import("../imports.zig");

usingnamespace parser;



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
    Add,
    Subtract,
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
    project: *Project,
    file: *FileInfo,
    line: LineNumber,
    column_start: CharNumber,
    column_end: CharNumber,

    start: u64,
    end: u64,

    kind: tKind,
    string: []u8,

    pub fn println(self: Token) void {
        utils.println("{}", self.string);
    }
};


pub const StreamCodePoint8 = struct {
    bytes: [4]u8,
    len: usize,
    start: usize,
    end: usize,

    pub fn chars(self: StreamCodePoint8) []u8 {
        return self.bytes[0 .. self.len];
    }

    pub fn init(point: u21, end: usize) !StreamCodePoint8 {
        var bytes: [4]u8 = undefined;
        var len = try unicode.utf8Encode(point, bytes[0..]);

        return StreamCodePoint8 {
            .bytes = bytes,
            .len = len,
            .start = end - len,
            .end = end,
        };
    }
};


pub const TokenIterator = struct {
    project: *Project,
    file: *FileInfo,
    source: []const u8,
    index: u64,
    line: LineNumber,
    column: CharNumber,
    token: Token,

    pub fn init(project: *Project, file: *FileInfo, source: []const u8) TokenIterator {
        var self = TokenIterator {
            .project = project,
            .file = file,
            .source = source,
            .index = 0,
            .line = LineNumber.init(1),
            .column = CharNumber.init(1),
            .token = undefined,
        };

        self.next();

        return self;
    }

    pub fn peek(self: *TokenIterator) Token {
        var original = self.*;

        var token = self.get_token() catch unreachable;
        println("Peeking '{}'", .{token.?.string});

        self.* = original;
        return token.?;
    }

    pub fn has_next(self: *TokenIterator) bool {
        return true;
    }

    pub fn next(self: *TokenIterator) void {
        if(self.get_token() catch |err| {
            println("Fatal tokenization error: {}", .{err});
            std.process.exit(1);
        }) |token| {
            self.token = token;
            println("Token '{}' of kind {}", .{token.string, @tagName(token.kind)});
        }
        else {
            parse_error_file_line_column_start_end(
                self.file,
                self.token.line,
                self.token.column_start,
                self.token.start,
                self.token.end,
                "Unexpectedly encountered end of file",
                .{}
            );
        }
    }

    pub fn get_token(self: *TokenIterator) !?Token {
        // println("Started get_token", .{});

        var iterator = unicode.Utf8Iterator {
            .bytes = self.source,
            .i = self.index,
        };

        iteration: while(iterator.nextCodepoint()) |codepoint| { //Consume to beginning of next token
            var point = try StreamCodePoint8.init(codepoint, iterator.i);

            if(point.bytes[0] == ' ') {
                self.column.number += 1;
            }

            else if(point.bytes[0] == '\t') {
                self.column.number += 4;
            }

            else if(point.bytes[0] == '\n') {
                self.line.number += 1;
                self.column.number = 1;
            }

            else if(point.bytes[0] == '/') {
                if(iterator.nextCodepoint()) |second_codepoint| {
                    var second_point = try StreamCodePoint8.init(second_codepoint, iterator.i);
                    if(second_point.bytes[0] == '/') { //Comment!
                        while(iterator.nextCodepoint()) |consumed_codepoint| {
                            var consumed = try StreamCodePoint8.init(consumed_codepoint, iterator.i);
                            if(consumed.bytes[0] == '\n') {
                                self.line.number += 1;
                                self.column.number = 1;
                                continue :iteration;
                            }
                        }
                    }
                    else { //Not a comment! RETREAT!!!
                        iterator.i -= second_point.len;
                    }
                }

                //Rewind to before the '/' then break
                iterator.i -= point.len;
                break :iteration;
            }

            else {
                iterator.i -= point.len;
                break;
            }
        }

        var token = Token {
            .project = self.project,
            .file = self.file,
            .line = self.line,
            .column_start = self.column,
            .column_end = self.column,
            .start = iterator.i,
            .end = iterator.i,
            .kind = .Word,
            .string = "",
        };

        while(iterator.nextCodepoint()) |codepoint| {
            var point = try StreamCodePoint8.init(codepoint, iterator.i);
            self.column.number += 1;

            switch(point.bytes[0]) {
                else => {
                    token.end = iterator.i;
                    token.column_end = self.column;
                    token.string = self.source[token.start .. point.end];
                },

                ' ', '\t', '\n' => {
                    if(point.bytes[0] == '\t') {
                        self.column.number += 3;
                    }
                    else if(point.bytes[0] == '\n') {
                        self.column.number = 1;
                        self.line.number += 1;
                    }

                    // println("Returning due to whitespace", .{});
                    self.index = iterator.i;
                    return token;
                },

                ':', '.', ';', ',', '#', '=', '>', '<', '+', '-', '*', '/', '!', '&', '$', '(', ')', '{', '}', '[', ']', => {
                    if(token.string.len > 0) {
                        self.column.number -= 1;
                        iterator.i -= point.len;
                        token.end = iterator.i;
                        self.index = iterator.i;
                        // println("Returning due to single character", .{});
                        return token;
                    }
                    else {
                        token.kind = switch(point.bytes[0]) {
                            ':' => .Colon,
                            '.' => .Period,
                            ';' => .Semicolon,
                            ',' => .Comma,
                            '#' => .Hash,
                            '=' => .Equals,
                            '>' => .CompGreater,
                            '<' => .CompLess,
                            '+' => .Add,
                            '-' => .Subtract,
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
                        };

                        token.string = self.source[point.start .. point.end];

                        switch(token.kind) {
                            .Equals, .CompGreater, .CompLess, .Exclamation => {
                                if(iterator.nextCodepoint()) |peeked_codepoint| {
                                    var peeked = try StreamCodePoint8.init(peeked_codepoint, iterator.i);

                                    if(peeked.bytes[0] == '=') {
                                        self.column.number += 1;
                                        token.column_end = self.column;
                                        token.end = peeked.end;
                                        token.string = self.source[token.start .. peeked.end];

                                        token.kind = switch(token.kind) {
                                            .Equals => .CompEqual,
                                            .CompGreater => .CompGreaterEqual,
                                            .CompLess => .CompLessEqual,
                                            .Exclamation => .CompNotEqual,
                                            else => unreachable,
                                        };
                                    }
                                    else {
                                        iterator.i -= peeked.len;
                                    }
                                }
                            },

                            else => {},
                        }

                        // println("Returning a single character", .{});
                        self.index = iterator.i;
                        return token;
                    }
                },
            } //End of character switch
        }

        //No next codepoint
        self.index = iterator.i;
        return null;
    }
};
