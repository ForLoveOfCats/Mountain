usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_file(self: *TokenIterator, rootmod: *pModule) !void {
    if(self.tokens.len == 0) {
        return; //Empty file
    }

    //Purposely don't check token kind to improve error UX
    if(!mem.eql(u8, self.token().string, "module")) {
        parse_error(self.token(), "File must first declare a module", .{});
    }

    self.next();
    var name_parts = std.ArrayList([]u8).init(heap.c_allocator);
    defer name_parts.deinit();
    while(true) {
        expect_kind(self.token(), .Word);
        try name_parts.append(self.token().string);
        self.next();

        if(self.token().kind == .Period) { //Another part of the name is expected
            self.next(); //Move to what should be a word
            continue; //then continue to handle it
        }
        break; //We've handled the last part of the name
    }

    var module: *pModule = rootmod;
    for(name_parts.toSlice()) |name| {
        if(module.children.contains(name)) {
            var kv = module.children.get(name) orelse unreachable;
            module = &kv.value;
        }
        else {
            var stack_module = pModule {
                .name = name,
                .block = pBlock.init(),
                .children = std.StringHashMap(parser.pModule).init(heap.c_allocator),
            };
            _ = try module.children.put(name, stack_module);
            var kv = module.children.get(name) orelse unreachable;
            module = &kv.value;
        }
    }

    expect_kind(self.token(), .Semicolon);

    if(self.has_next()) {
        self.next();
        try parse_block(self, &module.block, true);
    }
}
