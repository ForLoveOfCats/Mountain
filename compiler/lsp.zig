usingnamespace @import("imports.zig");



const Id = union(enum) {
    String: []const u8,
    Integer: i64,
};


const Request = struct {
    id: Id,
    method: []const u8,
    params: *json.ObjectMap,
};


const Notification = struct {
    method: []const u8,
    params: *json.ObjectMap,
};


const Rpc = union(enum) {
    Request: Request,
    Notification: Notification,
};


fn read_json(instream: *fs.File.InStream.Stream, arena: *mem.Allocator) ![]u8 {
    var buffer: [1024 * 4]u8 = undefined;
    var content_length: ?usize = null;

    //Process the header
    while(true) {
        var line = io.readLineSliceFrom(instream, buffer[0..]) catch continue;

        if (line.len == 0) {
            break; //Blank line between header and content, end of header
        }

        const seperator = mem.indexOf(u8, line, ": ") orelse return error.LspMissingSeperator;
        if(content_length == null and mem.eql(u8, line[0..seperator], "Content-Length")) {
            content_length = try std.fmt.parseInt(usize, line[seperator+2..], 10);
        }
    }

    var content = try arena.alloc(u8, content_length.?);
    const read_length = try instream.read(content);

    if(content_length.? != read_length) {
        return error.LspContentSizeMismatch;
    }

    return content;
}


fn read_rpc(instream: *fs.File.InStream.Stream, arena: *mem.Allocator) !Rpc {
    var content = try read_json(instream, arena);

    var rpc_parser = json.Parser.init(arena, false);
    defer rpc_parser.deinit();

    var tree = try rpc_parser.parse(content);
    var object = tree.root.Object;

    if(object.get("id")) |id| { //Request
        var actual_id: ?Id = undefined;
        switch(id.value) {
            .String => |str| {
                actual_id = Id {
                    .String = str,
                };
            },

            .Integer => |int| {
                actual_id = Id {
                    .Integer = int,
                };
            },

            else => unreachable,
        }

        return Rpc {
            .Request = Request {
                .id = actual_id.?,
                .method = object.get("method").?.value.String,
                .params = &object.get("params").?.value.Object,
            },
        };
    }
    else { //Notification
        return Rpc {
            .Notification = Notification {
                .method = object.get("method").?.value.String,
                .params = &object.get("params").?.value.Object,
            },
        };
    }
}


pub fn serve() !void {
    var instream = &io.getStdIn().inStream().stream;

    while(true) {
        var arena = heap.ArenaAllocator.init(allocator);
        defer arena.deinit();

        var rpc = try read_rpc(instream, &arena.allocator);

        switch(rpc) {
            .Request => |request| {
                print("Recieved request '{}' of id ", .{request.method});

                switch(request.id) {
                    .String => |id| println("'{}'", .{id}),
                    .Integer => |id| println("'{}'", .{id}),
                }
            },

            .Notification => |notification| {
                print("Recieved notification '{}'", .{notification.method});
            },
        }
    }
}
