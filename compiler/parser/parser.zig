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



pub var modules: std.StringHashMap(parser.pModule) = undefined;
