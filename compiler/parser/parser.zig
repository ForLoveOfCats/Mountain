usingnamespace @import("../imports.zig");

pub usingnamespace @import("tokenizer.zig");
pub usingnamespace @import("parse_file.zig");
pub usingnamespace @import("parse_block.zig");
pub usingnamespace @import("parse_expression.zig");
pub usingnamespace @import("parse_let.zig");
pub usingnamespace @import("parse_func.zig");
pub usingnamespace @import("parse_type.zig");
pub usingnamespace @import("ptypes.zig");
pub usingnamespace @import("expect.zig");



pub var rootmod: pModule = undefined;


pub fn parse_error(token: parser.Token, comptime fmt: []const u8, args: var) noreturn {
    parse_error_file_line_column_start_end(token.file, token.line, token.column_start, token.start, token.end, fmt, args);
}


pub fn parse_error_file_line_column_start_end(
    file: usize,
    line: LineNumber,
    column_start: CharNumber,
    start: usize,
    end: usize,
    comptime fmt: []const u8,
    args: var
) noreturn {
    const file_entry = compiler.files.toSlice()[file];

    warn("  Parse error in '{}' @ line {}, column {}: ", .{file_entry.path, line.number, column_start.number});
    warnln(fmt, args);
    warn_line_error(file, start, end);

    std.process.exit(1);
}
