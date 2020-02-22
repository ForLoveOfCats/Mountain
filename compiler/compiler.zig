usingnamespace @import("imports.zig");



pub const allocator = heap.c_allocator;


const Mode = enum {
        Build,
        Server,
};


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
    const args = try std.process.argsAlloc(allocator);
    defer std.process.argsFree(allocator, args);

    var mode = process_cli(args);

    projects = std.ArrayList(Project).init(allocator);
    defer {
        for(projects.toSlice()) |project| {
            project.deinit();
        }
        projects.deinit();
    }

    switch(mode) {
        .Build => {
            logstream = &io.getStdOut().outStream().stream;

            const dir_path = try fs.realpathAlloc(allocator, args[2]);
            defer allocator.free(dir_path);

            var project = try Project.load(dir_path);
            defer project.deinit();

            for(project.files.toSlice()) |file| {
                file.contents = try io.readFileAlloc(allocator, file.path);

                var tokens = std.ArrayList(parser.Token).init(allocator);
                defer tokens.deinit();

                var token_iterator = parser.TokenIterator.init(&project, file, file.contents.?);
                if(try parser.parse_file(&token_iterator, file)) |pfile| {
                    try project.add_pfile(pfile);
                }
            }

            println("The following parse tree was built", .{});
            project.rootmod.debug_print(0);
        },

        .Server => {
            logstream = &io.getStdErr().outStream().stream;
            var server = lsp.Server.init();
            try server.serve();
        },
    }
}
