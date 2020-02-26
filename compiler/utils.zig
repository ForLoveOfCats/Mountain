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


pub fn print_many(count: u64, comptime fmt: []const u8, args: var) void {
    var index: u64 = 0;
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


pub fn warn_line_error(file: *const FileInfo, start: u64, end: u64) void {
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

    var underline_start_spaces: u64 = 0;
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

    var underline_length: u64 = 0;
    while(true) {
        if(iterator.i == end) {
            break;
        }

        _ = iterator.nextCodepoint();
        underline_length += 1;
    }

    warn("{}", .{spacer});
    var i: u64 = 0;
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
    number: u64,

    pub fn init(number: u64) LineNumber {
        return LineNumber {
            .number = number,
        };
    }
};


pub const CharNumber = struct { //TODO: Rename to ColumnNumber
    number: u64,

    pub fn init(number: u64) CharNumber {
        return CharNumber {
            .number = number,
        };
    }
};


/// Reads all characters until the next newline into buf, and returns
/// a slice of the characters read (excluding the newline character(s)).
/// Originally was in std but was removed
fn readLineFrom(stream: var, buf: *std.Buffer) ![]u8 {
    const start = buf.len();
    while (true) {
        const byte = try stream.readByte();
        switch (byte) {
            '\r' => {
                // trash the following \n
                _ = try stream.readByte();
                return buf.toSlice()[start..];
            },
            '\n' => return buf.toSlice()[start..],
            else => try buf.appendByte(byte),
        }
    }
}


/// Reads all characters until the next newline into slice, and returns
/// a slice of the characters read (excluding the newline character(s)).
/// Originally was in std but was removed
pub fn readLineSliceFrom(stream: var, slice: []u8) ![]u8 {
    // We cannot use Buffer.fromOwnedSlice, as it wants to append a null byte
    // after taking ownership, which would always require an allocation.
    var buf = std.Buffer{ .list = std.ArrayList(u8).fromOwnedSlice(std.testing.failing_allocator, slice) };
    try buf.resize(0);
    return try readLineFrom(stream, &buf);
}
