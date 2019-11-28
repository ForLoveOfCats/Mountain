usingnamespace @import("../imports.zig");

usingnamespace parser;
usingnamespace tokenizer;



pub fn parse_file(self: *TokenIterator) !void {
    if(self.tokens.len == 0) {
        return; //Empty file
    }

    expect_word(self.token(), "module");

    self.next();
    expect_kind(self.token(), .Word); //Module name
    var module: pModule = undefined;
    if(modules.contains(self.token().string)) {
        var kv = modules.get(self.token().string) orelse unreachable;
        module = kv.value;
    }
    else {
        module = pModule {
            .name = self.token().string,
        };
        _ = try modules.put(self.token().string, module);
    }

    self.next();
    expect_kind(self.token(), .Semicolon);
}
