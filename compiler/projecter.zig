usingnamespace @import("imports.zig");



pub var projects: std.ArrayList(Project) = undefined;


pub const FileInfo = struct {
    path: []const u8,
    basename: []const u8,
    contents: ?[]u8,

    pub fn init(path: []const u8, basename: []const u8) !*FileInfo {
        var instance = FileInfo {
            .path = try mem.dupe(allocator, u8, path),
            .basename = try mem.dupe(allocator, u8, basename),
            .contents = null,
        };

        var pointer = try allocator.create(FileInfo);
        pointer.* = instance;

        return pointer;
    }

    pub fn deinit(self: *FileInfo) void {
        allocator.free(self.path);
        allocator.free(self.basename);

        if(self.contents != null) {
            allocator.free(self.contents.?);
        }

        allocator.destroy(self);
    }
};


fn invalid_project(dir_path: []const u8, comptime message: []const u8, args: var) noreturn {
    print("Project '{}' is invalid: ", .{dir_path});
    println(message, args);
    std.process.exit(1);
}


fn check_project_type(definition: json.ObjectMap, dir_path: []const u8) void {
    if(!definition.contains("Type")) {
        invalid_project(
            dir_path,
            "Expected key 'Type' with value 'MountainProject'",
            .{}
        );
    }

    var type_key = definition.get("Type").?;
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


fn project_name_alloc(definition: json.ObjectMap, dir_path: []const u8) ![]u8 {
    if(!definition.contains("Name")) {
        invalid_project(
            dir_path,
            "Expected key 'Name' with string value",
            .{}
        );
    }

    var name_key = definition.get("Name").?;
    if(name_key.value != .String) {
        invalid_project(
            dir_path,
            "Value for 'Name' is not a string",
            .{}
        );
    }

    return try mem.dupe(allocator, u8, name_key.value.String);
}


fn project_source_path_alloc(definition: json.ObjectMap, dir_path: []const u8) ![]u8 {
    if(!definition.contains("Source")) {
        invalid_project(
            dir_path,
            "Expected key 'Source' with source code directory path as value",
            .{}
        );
    }

    var source_key = definition.get("Source").?;
    if(source_key.value != .String) {
        invalid_project(
            dir_path,
            "Value for 'Source' is not a string, expected source code directory path",
            .{}
        );
    }

    var raw_source_path = try mem.concat(
        allocator,
        u8,
        &[_][]const u8 {
            dir_path,
            fs.path.sep_str,
            source_key.value.String
        }
    );
    defer allocator.free(raw_source_path);

    return try fs.realpathAlloc(allocator, raw_source_path);
}


pub const Project = struct {
    name: []u8,
    files: std.ArrayList(*FileInfo),
    rootmod: parser.pModule,

    pub fn load(dir_path: []const u8) !Project {
        var project_dir = try fs.cwd().openDirList(dir_path);

        var definition_source = try project_dir.readFileAlloc(
            allocator,
            "project.json",
            1000 * 1000 //1,000 kb
        );
        defer allocator.free(definition_source);

        var project_parser = json.Parser.init(allocator, false);
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
        check_project_type(definition, dir_path);

        var project_name = try project_name_alloc(definition, dir_path);

        var files = std.ArrayList(*FileInfo).init(allocator);

        var cwd_buffer: [std.fs.MAX_PATH_BYTES]u8 = undefined;
        var cwd = try os.getcwd(&cwd_buffer);

        var source_path = try project_source_path_alloc(definition, dir_path);
        defer allocator.free(source_path);

        var walker = try fs.walkPath(allocator, source_path);
        defer walker.deinit();

        while(try walker.next()) |file| {
            if(file.kind == .File and mem.endsWith(u8, file.basename, ".mtn")) {
                var path: []u8 = try fs.path.relative(allocator, cwd, file.path);
                defer allocator.free(path);

                try files.append(try FileInfo.init(path, file.basename));
            }
        }

        var rootmod = parser.pModule {
            .name = "RootModule",
            .block = parser.pBlock.init(),
            .children = std.StringHashMap(parser.pModule).init(allocator),
        };

        return Project {
            .name = project_name,
            .files = files,
            .rootmod = rootmod,
        };
    }


    pub fn deinit(self: Project) void {
        allocator.free(self.name);

        for(self.files.toSlice()) |file| {
            file.deinit();
        }
        self.files.deinit();

        self.rootmod.deinit();
    }
};
