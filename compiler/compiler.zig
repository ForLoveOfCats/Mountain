usingnamespace @import("imports.zig");

const DirEntry = fs.Walker.Entry;



pub fn main() anyerror!void {
    const allocator = heap.c_allocator;

    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    if (args.len != 2) {
        warn("Please provide an input folder path to compile\n");
        std.process.exit(1);
    }

    const dir_path = try fs.realpathAlloc(allocator, args[1]);
    defer allocator.free(dir_path);

    var entries = std.ArrayList(DirEntry).init(allocator);
    defer {
        for(entries.toSlice()) |entry| {
            allocator.free(entry.path);
            allocator.free(entry.basename);
        }
        entries.deinit();
    }

    var walker = try fs.walkPath(allocator, dir_path);
    defer walker.deinit();
    while(try walker.next()) |entry| {
        if(entry.kind == .File and mem.endsWith(u8, entry.basename, ".mtn")) {
            try entries.append(DirEntry {
                .path = try mem.dupe(allocator, u8, entry.path),
                .basename = try mem.dupe(allocator, u8, entry.basename),
                .kind = entry.kind,
            });
        }
    }

    var token_allocator = heap.ArenaAllocator.init(heap.c_allocator);
    defer token_allocator.deinit();

    for(entries.toSlice()) |entry| {
        var source = try io.readFileAlloc(&token_allocator.allocator, entry.path);
        var tokens = std.ArrayList(tokenizer.Token).init(&token_allocator.allocator);

        try tokenizer.tokenize_file(source, &tokens);
        try parser.parse_file(tokens.toSlice());
    }
}
