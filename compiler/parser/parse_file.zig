usingnamespace @import("../imports.zig");

usingnamespace parser;



pub fn parse_file(self: *TokenIterator) !void {
    if(self.tokens.len == 0) {
        return; //Empty file
    }

    //Purposely don't check token kind to improve error UX
    if(!mem.eql(u8, self.token().string, "module")) {
        parse_error(self.token(), "File must first declare a module");
    }

    self.next();
    expect_kind(self.token(), .Word); //Module name
    var module: *pModule = undefined;
    if(modules.contains(self.token().string)) {
        var kv = modules.get(self.token().string) orelse unreachable;
        module = &kv.value;
    }
    else {
        var stack_module = pModule {
            .name = self.token().string,
            .block = pBlock.init(),
        };
        _ = try modules.put(self.token().string, stack_module);
        var kv = modules.get(self.token().string) orelse unreachable;
        module = &kv.value;
    }

    self.next();
    expect_kind(self.token(), .Semicolon);

    if(self.has_next()) {
        self.next();
        try parse_block(self, &module.block, true);
    }
}
