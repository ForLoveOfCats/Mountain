usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_file(self: *TokenIterator, file: *const FileInfo) !?*pFile {
    if(!self.more_left()) {
        return null;
    }

    //Purposely don't check token kind to improve error UX
    if(!mem.eql(u8, self.token.string, "module")) {
        parse_error(self.token, "File must first declare a module", .{});
    }

    self.next();
    var name_parts = std.ArrayList([]u8).init(allocator);
    while(true) {
        expect_kind(self.token, .Word);
        try name_parts.append(self.token.string);
        self.next();

        if(self.token.kind == .Period) { //Another part of the name is expected
            self.next(); //Move to what should be a word
            continue; //then continue to handle it
        }
        break; //We've handled the last part of the name
    }

    var pfile = try pFile.init(
        pFile {
            .file = file,
            .module_path = name_parts,
            .block = pBlock.init(),
        }
    );

    expect_kind(self.token, .Semicolon);

    if(self.more_left()) {
        self.next();
        try parse_block(self, &pfile.block, true);
    }

    return pfile;
}
