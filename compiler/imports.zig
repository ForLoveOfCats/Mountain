pub const std = @import("std");
pub const big = std.math.big;
pub const debug = std.debug;
pub const warn = debug.warn;
pub const mem = std.mem;
pub const heap = std.heap;
pub const os = std.os;
pub const fs = std.fs;
pub const io = std.io;
pub const unicode = std.unicode;
pub const json = std.json;

pub const assert = std.debug.assert;

pub const compiler = @import("compiler.zig");
pub const projecter = @import("projecter.zig");
pub const parser = @import("parser/parser.zig");
pub const lsp = @import("lsp/lsp.zig");
pub const utils = @import("utils.zig");

pub usingnamespace utils;
pub usingnamespace projecter;
pub usingnamespace compiler;
