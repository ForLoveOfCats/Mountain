usingnamespace @import("imports.zig");

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


pub fn warnln(comptime fmt: []const u8, args: ...) void {
    warn(fmt, args);
    warn("\n");
}


pub fn parse_error(token: tokenizer.Token, comptime fmt: []const u8, args: ...) noreturn {
    const file = compiler.files.toSlice()[token.file];

    warn("  Parse error in '{}' @ line {}, column {}: ", file.path, token.line.number, token.column_start.number);
    warnln(fmt, args);
    warn_line_error(token.file, token.start, token.end);

    std.process.exit(1);
}


pub fn warn_line_error(file: usize, start: usize, end: usize) void {
    const spacer = "        ";
    var source = compiler.sources.toSlice()[file];

    var line_start = start;
    while(true) {
        if(line_start == 0) {
            break;
        }
        else if(source[line_start] == '\n') {
            line_start += 1;
            break;
        }

        line_start -= 1;
    }

    var line_end = end;
    while(true) {
        if(line_end == source.len or source[line_end] == '\n') {
            line_end -= 1;
            break;
        }

        line_end += 1;
    }

    warnln("{}{}", spacer, source[line_start..line_end+1]);

    var underline_start_spaces: usize = 0;
    while(true) {
        if(line_start+underline_start_spaces == start) {
            break;
        }

        underline_start_spaces += 1;
    }

    var underline_length: usize = 0;
    while(true) {
        if(line_start+underline_start_spaces+underline_length == end) {
            break;
        }

        underline_length += 1;
    }

    warn("{}", spacer);
    var i: usize = 0;
    while(i < underline_start_spaces) {
        i += 1;
        warn(" ");
    }
    warn("^");

    if(underline_length > 0) {
        i = 0;
        while(i < underline_length-1) {
            i += 1;
            warn("^");
        }
    }

    warn("\n");
}


pub const LineNumber = struct {
    number: usize,
};

pub fn newLineNumber(number: usize) LineNumber {
    return LineNumber {
        .number = number,
    };
}


pub const CharNumber = struct {
    number: usize,
};

pub fn newCharNumber(number: usize) CharNumber {
    return CharNumber {
        .number = number,
    };
}
