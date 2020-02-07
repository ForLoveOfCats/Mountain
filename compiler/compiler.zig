usingnamespace @import("imports.zig");



const Mode = enum {
        Build,
        Server,
};


pub var files: std.ArrayList(FileInfo) = undefined;
pub var sources: std.ArrayList([]u8) = undefined;


fn help_message(comptime fmt: []const u8, args: var) void {
    print("    ", .{});
    println(fmt, args);
}

fn print_help() void {
    println("Mountain programming language compiler:", .{});
    help_message("Usage: `mountain --build ./Path/To/Project`", .{});

    println("", .{});

    println("Flags:", .{});
    help_message("--help", .{});
    help_message("--build", .{});
    help_message("--server", .{});
}


fn process_cli(args: [][]const u8) Mode {
    if(args.len < 2) { //Need at least one arg
        print_help();
        std.process.exit(1);
    }

    if(mem.eql(u8, args[1], "--help")) {
        print_help();
        std.process.exit(0);
    }
    else if(mem.eql(u8, args[1], "--server")) {
        return .Server;
    }
    else if(!mem.eql(u8, args[1], "--build")) {
        println(
            "Expected '--build' or '--server' flag but found '{}' instead",
            .{args[1]}
        );
        std.process.exit(1);
    }

    if(args.len != 3) {
        println("Expected project folder path following '--build' flag", .{});
        std.process.exit(1);
    }

    return .Build;
}


pub fn main() anyerror!void {
    const allocator = heap.c_allocator;

    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    var mode = process_cli(args);

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

    projects = std.ArrayList(Project).init(allocator);
    defer {
        for(projects.toSlice()) |project| {
            project.deinit();
        }
        projects.deinit();
    }

    switch(mode) {
        .Server => {
        },

        .Build => {
            const dir_path = try fs.realpathAlloc(allocator, args[2]);
            defer allocator.free(dir_path);

            var project = try Project.load(dir_path);

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
                    var path: []u8 = try fs.path.relative(allocator, cwd, file.path);
                    defer allocator.free(path);

                    try files.append(try FileInfo.init(path, file.basename));
                }
            }

            for(files.toSlice()) |file| {
                println("{}", .{file.path});
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
        },
    }

    println("The following parse tree was built", .{});
    parser.rootmod.debug_print(0);
}
