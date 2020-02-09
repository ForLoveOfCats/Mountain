usingnamespace @import("imports.zig");



fn read_rpc(instream: *fs.File.InStream.Stream) ![]u8 {
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

    var content = try allocator.alloc(u8, content_length.?);
    const read_length = try instream.read(content);

    if(content_length.? != read_length) {
        return error.LspContentSizeMismatch;
    }

    return content;
}


pub fn serve() !void {
    var instream = &io.getStdIn().inStream().stream;

    while(true) {
        var content = try read_rpc(instream);
        defer allocator.free(content);

        println("{}", .{content});
        println("", .{});
    }
}
