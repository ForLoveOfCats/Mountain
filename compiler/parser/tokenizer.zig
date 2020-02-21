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
            .start = start,
            .end = start + len,
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
            .line = LineNumber.init(0),
            .column = CharNumber.init(0),
            .token = undefined,
        };

        self.next();

        return self;
    }

    pub fn peek(self: *TokenIterator) ?Token {
        return self.tokens[self.index+1]; //Will crash if not checked with has_next first
    }

    pub fn next(self: *TokenIterator) void {
        if(!has_next(self)) {
            parse_error_file_line_column_start_end(
                self.tokens[self.index].file,
                self.tokens[self.index].line,
                self.tokens[self.index].column_end,
                self.tokens[self.index].start,
                self.tokens[self.index].end,
                "Unexpectedly encountered end of file",
                .{}
            );
        }

        self.index += 1;
    }

    pub fn get_token(self: *TokenIterator) Token {
        var start = self.index;

        var iterator = unicode.Utf8Iterator {
            .bytes = self.source,
            .i = start,
        };

        while(iterator.nextCodepoint()) |codepoint| { //Consume to beginning of next token
            var point = StreamCodePoint8.init(codepoint, iterator.i);
            if(point.bytes[0] == ' ') {
                self.column.number += 1;
            }

            else if(point.bytes[0] == '\t') {
                self.column.number += 4;
            }

            else if(point.bytes[0] == "\n") {
                self.line.number += 1;
                self.column.number = 0;
            }

            else if(point.bytes[0] == '/') {
                if(iterator.nextCodepoint()) |second_codepoint| {
                    var second_point = StreamCodePoint8.init(second_codepoint, iterator.i);
                    if(second_point.bytes[0] == '/') { //Comment!
                        while(iterator.nextCodepoint()) |consumed| {
                            if(consumed.bytes[0] == '\n') {
                                self.line.number += 1;
                                self.column.number = 0;
                                break;
                            }
                        }
                    }
                    else { //Not a comment! RETREAT!!!
                        iterator.i -= second_point.len;
                    }
                }

                //Rewind to before the '/' then break
                iterator.i -= point.len;
                break;
            }

            else {
                break;
            }
        }

        var token = Token {
            .project = project,
            .file = file,
            .line = self.line,
            .column_start = self.column,
            .column_end = self.column,
            .start = self.index,
            .end = self.index,
            .kind = .Word,
            .string = "",
        };

        if(iterator.nextCodepoint()) |codepoint| {
            var point = StreamCodePoint8.init(codepoint, iterator.i);
            self.column.number += 1;

            switch(point.bytes[0]) {
                else => {
                    token.column_end = self.column;
                    
                },

                ' ', '\t', '\n' => {
                    if(point.bytes[0] == '\t') {
                        self.column.number += 3;
                    }
                    else if(point.bytes[0] == '\n') {
                        self.column.number = 0;
                        self.line.number += 1;
                    }

                    if(token.string.len > 0) {
                        token.end = iterator.i - point.len;
                        return token;
                    }
                },

                ':', '.', ';', ',', '#', '=', '>', '<', '+', '-', '*', '/', '!', '&', '$', '(', ')', '{', '}', '[', ']', => {
                    if(token.string.len > 0) {
                        token.end = iterator.i - point.len;
                        self.column.number -= 1;
                        iterator.i -= point.len;
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

                        //Here we add 2 if it is ==, >=, or <= as slicing end is exclusive [start, end)
                        if(char_token.kind == .Equals) fail: {
                            var peeked = try StreamCodePoint8.init(utf8.nextCodepoint() orelse break :fail, iterator.i-1);
                            if(peeked.bytes[0] == '=') {
                                token.kind = .CompEqual;
                                token.column_end.number += 2; //We know we can't have gone to the next line
                                token.end += 2;
                                token.string = source[token.start .. peeked.end+1];
                            }
                            else {
                                iterator.i -= peeked.len;
                            }
                        }
                        else if(token.kind == .CompGreater) fail: {
                            var peeked = try StreamCodePoint8.init(iterator.nextCodepoint() orelse break :fail, iterator.i-1);
                            if(peeked.bytes[0] == '=') {
                                token.kind = .CompGreaterEqual;
                                token.column_end.number += 2; //We know we can't have gone to the next line
                                token.end += 2;
                                token.string = source[token.start .. peeked.end+1];
                            }
                            else {
                                iterator.i -= peeked.len;
                            }
                        }
                        else if(token.kind == .CompLess) fail: {
                            var peeked = try StreamCodePoint8.init(iterator.nextCodepoint() orelse break :fail, iterator.i-1);
                            if(peeked.bytes[0] == '=') {
                                token.kind = .CompLessEqual;
                                token.column_end.number += 2; //We know we can't have gone to the next line
                                token.end += 2;
                                token.string = source[token.start .. peeked.end+1];
                            }
                            else {
                                iterator.i -= peeked.len;
                            }
                        }
                        else if(token.kind == .Exclamation) fail: {
                            var peeked = try StreamCodePoint8.init(iterator.nextCodepoint() orelse break :fail, iterator.i-1);
                            if(peeked.bytes[0] == '=') {
                                token.kind = .CompNotEqual;
                                token.column_end.number += 2; //We know we can't have gone to the next line
                                token.end += 2;
                                token.string = source[token.start .. peeked.end+1];
                            }
                            else {
                                utf8.i -= peeked.len;
                            }
                        }

                        return token;
                    }
                },
            } //End of character switch

            token.string = source[token.start .. point.end+1];
            token.end = last_index;
            last_index = utf8.i;
        }
    }
};
