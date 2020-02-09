usingnamespace @import("imports.zig");



pub fn serve() !void {
    var instream = &io.getStdIn().inStream().stream;

    var buffer: [1024 * 4]u8 = undefined;

    message: while(true) {
        var content_length: ?usize = null;

        //First process the header
        while(true) {
            var line = io.readLineSliceFrom(instream, buffer[0..]) catch continue :message;

            if (line.len == 0) {
                break; //Blank line between header and content, end of header
            }

            const seperator = mem.indexOf(u8, line, ": ") orelse return error.LspMissingSeperator;
            if(content_length == null and mem.eql(u8, line[0..seperator], "Content-Length")) {
                content_length = try std.fmt.parseInt(usize, line[seperator+2..], 10);
            }
        }

        println("Length in bytes: {}", .{content_length.?});

        var content = try allocator.alloc(u8, content_length.?);
        defer allocator.free(content);

        const read_length = try instream.read(content);

        if(content_length.? != read_length) {
            return error.LspContentSizeMismatch;
        }

        println("{}", .{content});
        println("", .{});
    }
}
