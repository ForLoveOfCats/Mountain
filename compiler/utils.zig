const std = @import("std");
const io = std.io;
const fs = std.fs;
const File = fs.File;



var stdout_file: File = undefined;
var stdout_file_out_stream: File.OutStream = undefined;

var stdout_stream: ?*io.OutStream(File.WriteError) = null;
var stdout_mutex = std.Mutex.init();

pub fn println(comptime fmt: []const u8, args: ...) void {
    const held = stdout_mutex.acquire();
    defer held.release();
    const stdout = getStdoutStream() catch return;
    stdout.print(fmt, args) catch return;
    stdout.print("\n") catch return;
}

pub fn print(comptime fmt: []const u8, args: ...) void {
    const held = stdout_mutex.acquire();
    defer held.release();
    const stdout = getStdoutStream() catch return;
    stdout.print(fmt, args) catch return;
}

fn getStdoutStream() !*io.OutStream(File.WriteError) {
    if (stdout_stream) |st| {
        return st;
    } else {
        stdout_file = try io.getStdOut();
        stdout_file_out_stream = stdout_file.outStream();
        const st = &stdout_file_out_stream.stream;
        stdout_stream = st;
        return st;
    }
}


pub const LineNumber = struct {
    number: i32,
};

pub fn newLineNumber(number: i32) LineNumber {
    return LineNumber {
        .number = number,
    };
}


pub const CharNumber = struct {
    number: i32,
};

pub fn newCharNumber(number: i32) CharNumber {
    return CharNumber {
        .number = number,
    };
}
