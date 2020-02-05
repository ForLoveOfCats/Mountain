usingnamespace @import("imports.zig");



const FileInfo = struct {
    basename: []const u8,
    path: []const u8,
};


pub var files: std.ArrayList(FileInfo) = undefined;
pub var sources: std.ArrayList([]u8) = undefined;


fn help_message(comptime fmt: []const u8, args: var) void {
    print("    ", .{});
    println(fmt, args);
}

fn print_help() void {
    println("Mountain programming language compiler:", .{});
    help_message("Usage: `mountain --build ./Path/To/Input`", .{});

    println("", .{});

    println("Flags:", .{});
    help_message("--help", .{});
    help_message("--build", .{});
}


pub fn main() anyerror!void {
    const allocator = heap.c_allocator;

    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    if(args.len < 2) { //Need at least one arg
        print_help();
        std.process.exit(1);
    }

    const Mode = enum {
        Build,
    };
    var mode: Mode = .Build;

    if(mem.eql(u8, args[1], "--help")) {
        print_help();
        std.process.exit(0);
    }
    else if(!mem.eql(u8, args[1], "--build")) {
        println("Expected '--build' flag but found '{}' instead", .{args[1]});
        std.process.exit(1);
    }

    if(args.len != 3) {
        println("Expected input folder path following '--build' flag", .{});
        std.process.exit(1);
    }

    const dir_path = try fs.realpathAlloc(allocator, args[2]);
    defer allocator.free(dir_path);

    files = std.ArrayList(FileInfo).init(allocator);
    defer {
        for(files.toSlice()) |file| {
            allocator.free(file.path);
            allocator.free(file.basename);
        }
        files.deinit();
    }

    var cwd_buffer: [std.fs.MAX_PATH_BYTES]u8 = undefined;
    var cwd = try os.getcwd(&cwd_buffer);

    var walker = try fs.walkPath(allocator, dir_path);
    defer walker.deinit();

    while(try walker.next()) |file| {
        if(file.kind == .File and mem.endsWith(u8, file.basename, ".mtn")) {
            try files.append(FileInfo {
                .path = try fs.path.relative(allocator, cwd, file.path),//try mem.dupe(allocator, u8, file.path),
                .basename = try mem.dupe(allocator, u8, file.basename),
            });
        }
    }

    var root_module_name = "RootModule";
    parser.rootmod = parser.pModule {
        .name = root_module_name[0..root_module_name.len],
        .block = parser.pBlock.init(),
        .children = std.StringHashMap(parser.pModule).init(allocator),
    };
    defer parser.rootmod.deinit();

    var source_allocator = heap.ArenaAllocator.init(allocator);
    defer source_allocator.deinit();

    sources = std.ArrayList([]u8).init(&source_allocator.allocator);

    for(files.toSlice()) |file| {
        var source = try io.readFileAlloc(&source_allocator.allocator, file.path);
        try sources.append(source);

        var tokens = std.ArrayList(parser.Token).init(&source_allocator.allocator);

        try parser.tokenize_file(source, sources.len-1, &tokens);
        var token_iterator = parser.TokenIterator {
            .tokens = tokens.toSlice(),
            .index = 0,
        };

        try parser.parse_file(&token_iterator);
    }

    println("The following parse tree was built", .{});
    parser.rootmod.debug_print(0);
}
