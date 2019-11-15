const std = @import("std");
const mem = std.mem;
const fs = std.fs;
const DirEntry = fs.Walker.Entry;
const warn = std.debug.warn;

usingnamespace @import("utils.zig");



pub fn main() anyerror!void {
    const allocator = std.heap.c_allocator;

    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    if (args.len != 2) {
        warn("Please provide an input folder path to compile\n");
        std.process.exit(1);
    }

    const dir_path = try fs.realpathAlloc(allocator, args[1]);
    defer allocator.free(dir_path);

    var files = std.ArrayList(DirEntry).init(allocator);
    defer files.deinit();

    var walker = try fs.walkPath(allocator, dir_path);
    while(try walker.next()) |entry| {
        if(entry.kind == .File and mem.endsWith(u8, entry.basename, ".mtn")) {
            try files.append(DirEntry {
                .path = try mem.dupe(allocator, u8, entry.path),
                .basename = try mem.dupe(allocator, u8, entry.basename),
                .kind = entry.kind,
            });
        }
    }

    for(files.toSlice()) |file| {
        println("Found file {}", file.basename);
    }
}
