usingnamespace @import("imports.zig");



pub const FileInfo = struct {
    path: []const u8,
    basename: []const u8,

    pub fn init(path: []const u8, basename: []const u8) !FileInfo {
        return FileInfo {
            .path = try mem.dupe(heap.c_allocator, u8, path),
            .basename = try mem.dupe(heap.c_allocator, u8, basename),
        };
    }
};


pub var projects: std.ArrayList(Project) = undefined;


fn invalid_project(dir_path: []const u8, comptime message: []const u8, args: var) noreturn {
    print("Project '{}' is invalid: ", .{dir_path});
    println(message, args);
    std.process.exit(1);
}


pub const Project = struct {
    name: []u8,
    files: std.ArrayList(FileInfo),

    pub fn load(dir_path: []const u8) !Project {
        var project_dir = try fs.cwd().openDirList(dir_path);

        var definition_source = try project_dir.readFileAlloc(
            heap.c_allocator,
            "project.json",
            1000 * 1000 //1,000 kb
        );
        defer heap.c_allocator.free(definition_source);

        var project_parser = json.Parser.init(heap.c_allocator, false);
        defer project_parser.deinit();

        var tree = project_parser.parse(definition_source) catch |err| {
            invalid_project(
                dir_path,
                "Encountered '{}' when parsing project",
                .{@errorName(err)}
            );
        };
        defer tree.deinit();

        var definition = tree.root.Object;

        if(definition.get("Type")) |type_key| {
            if(type_key.value != .String) {
                invalid_project(
                    dir_path,
                    "Value for 'Type' is not a string, expected 'MountainProject'",
                    .{}
                );
            }

            else if(!mem.eql(u8, type_key.value.String, "MountainProject")) {
                invalid_project(
                    dir_path,
                    "Expected value of 'MountainProject' for key 'Type'",
                    .{}
                );
            }
        }
        else {
            invalid_project(
                dir_path,
                "Expected key 'Type' with value 'MountainProject'",
                .{}
            );
        }

        return Project {
            .name = "",
            .files = std.ArrayList(FileInfo).init(heap.c_allocator),
        };
    }


    pub fn deinit(self: Project) void {
        //TODO
    }
};
