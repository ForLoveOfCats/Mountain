usingnamespace @import("imports.zig");

const File = fs.File;



pub var logstream: ?*File.OutStream.Stream = null;
var log_mutex = std.Mutex.init();

pub fn print(comptime fmt: []const u8, args: var) void {
    const held = log_mutex.acquire();
    defer held.release();

    logstream.?.print(fmt, args) catch return;
}

pub fn println(comptime fmt: []const u8, args: var) void {
    const held = log_mutex.acquire();
    defer held.release();

    logstream.?.print(fmt, args) catch return;
    logstream.?.print("\n", .{}) catch return;
}


pub fn warnln(comptime fmt: []const u8, args: var) void {
    warn(fmt, args);
    warn("\n", .{});
}


pub fn print_many(count: usize, comptime fmt: []const u8, args: var) void {
    var index: usize = 0;
    while(index < count) {
        index += 1;
        print(fmt, args);
    }
}


pub fn internal_error(comptime fmt: []const u8, args: var) noreturn {
    warn("Internal Error: ", .{});
    warnln(fmt, args);
    std.process.exit(1);
}


pub fn warn_line_error(file: *FileInfo, start: usize, end: usize) void {
    const spacer = "        ";
    const source = file.contents.?;

    var line_start = start;
    while(true) { //Rewind until start of actual line
        if(source[line_start] == '\n') {
            line_start += 1;
            break;
        }
        else if(line_start == 0) {
            break;
        }

        line_start -= 1;
    }
    while(true) { //Proceed until first non-whitespace character
        switch(source[line_start]) {
            ' ', '\t' => line_start += 1,
            else => break,
        }
    }

    var line_end = end;
    while(true) {
        if(line_end == source.len or source[line_end] == '\n') {
            line_end -= 1;
            break;
        }

        line_end += 1;
    }

    warnln("{}{}", .{spacer, source[line_start..line_end+1]});

    var iterator = unicode.Utf8Iterator {
        .bytes = source,
        .i = line_start,
    };

    var underline_start_spaces: usize = 0;
    while(true) {
        if(iterator.i == start) {
            break;
        }

        var point = iterator.nextCodepoint() orelse unreachable;
        if(point == '\t') {
            underline_start_spaces += 4;
        }
        else {
            underline_start_spaces += 1;
        }
    }

    var underline_length: usize = 0;
    while(true) {
        if(iterator.i == end) {
            break;
        }

        _ = iterator.nextCodepoint();
        underline_length += 1;
    }

    warn("{}", .{spacer});
    var i: usize = 0;
    while(i < underline_start_spaces) {
        i += 1;
        warn(" ", .{});
    }
    warn("^", .{});

    if(underline_length > 0) {
        i = 0;
        while(i < underline_length-1) {
            i += 1;
            warn("^", .{});
        }
    }

    warn("\n", .{});
}


pub const LineNumber = struct {
    number: usize,

    pub fn init(number: usize) LineNumber {
        return LineNumber {
            .number = number,
        };
    }
};


pub const CharNumber = struct { //TODO: Rename to ColumnNumber
    number: usize,

    pub fn init(number: usize) CharNumber {
        return CharNumber {
            .number = number,
        };
    }
};
