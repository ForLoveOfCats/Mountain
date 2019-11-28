usingnamespace @import("../imports.zig");



pub fn debug_print_level(level: usize) void {
    print_many(level, "    ");
}


pub const pModule = struct {
    name: []u8,
    block: pBlock,

    pub fn deinit(self: pModule) void {
        self.block.deinit();
    }

    pub fn debug_print(self: pModule, level: usize) void {
        debug_print_level(level);
        println("Module '{}':", self.name);
        self.block.debug_print(level+1);
    }
};


pub const pBlock = struct {
    contents: std.ArrayList(InBlock),

    pub const InBlockEnum = enum {
        Func,
    };

    pub const InBlock = union(InBlockEnum) {
        Func: pFunc,
    };

    pub fn init() pBlock {
        return pBlock {
            .contents = std.ArrayList(InBlock).init(heap.c_allocator),
        };
    }

    pub fn deinit(self: pBlock) void {
        self.contents.deinit();
    }

    pub fn debug_print(self: pBlock, level: usize) void {
        debug_print_level(level);
        println("Block:");

        for(self.contents.toSlice()) |item| {
            switch(item) {
                .Func => |func| func.debug_print(level+1),
            }
        }
    }
};


pub const pFunc = struct {
    name: []u8,

    pub fn debug_print(self: pFunc, level: usize) void {
        debug_print_level(level);
        println("Func '{}':", self.name);
    }
};
