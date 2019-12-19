usingnamespace @import("../imports.zig");



pub fn debug_print_level(level: usize) void {
    print_many(level, "    ");
}


pub const pType = struct {
    name: []u8,
    reach_module: []u8,
    child: ?*pType,

    pub fn deinit(self: *pType) void {
        if(self.child) |actual| {
            actual.deinit();
        }
        heap.c_allocator.destroy(self);
    }

    pub fn debug_print(self: *pType) void {
        if(self.child) |actual| {
            actual.debug_print();
            print(":");
        }

        print("{}", self.name);
    }
};


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

    pub const InBlock = union(enum) {
        Func: pFunc,
        Expression: *pExpression,
    };

    pub fn init() pBlock {
        return pBlock {
            .contents = std.ArrayList(InBlock).init(heap.c_allocator),
        };
    }

    pub fn deinit(self: pBlock) void {
        for(self.contents.toSlice()) |*item| {
            switch(item.*) {
                .Func => |*func| {
                    func.deinit();
                },

                .Expression => |expression| {
                    expression.deinit();
                },
            }
        }

        self.contents.deinit();
    }

    pub fn debug_print(self: pBlock, level: usize) void {
        debug_print_level(level);
        println("Block:");

        for(self.contents.toSlice()) |item| {
            switch(item) {
                .Func => |func| func.debug_print(level+1),
                .Expression => |expression| expression.debug_print(level+1),
            }
        }
    }
};


pub const pExpression = union(enum) {
    Let: pLet,

    pub fn deinit(self: *pExpression) void {
        switch(self.*) {
            .Let => |*let| {
                let.deinit();
            },
        }
    }

    pub fn debug_print(self: pExpression, level: usize) void {
        debug_print_level(level);
        println("Expression:");

        switch(self) {
            .Let => |let| let.debug_print(level+1),
        }
    }
};


pub const pLet = struct {
    name: []u8,
    ptype: *pType,
    expression: ?*pExpression,

    pub fn deinit(self: *pLet) void {
        self.ptype.deinit();
    }

    pub fn debug_print(self: pLet, level: usize) void {
        debug_print_level(level);

        print("Let '{}' with type '", self.name);
        self.ptype.debug_print();
        println("'");

        if(self.expression) |actual| {
            actual.debug_print(level+1);
        }
    }
};


pub const pFunc = struct {
    name: []u8,
    ptype: *pType,
    block: pBlock,

    pub fn deinit(self: *pFunc) void {
        self.block.deinit();
        self.ptype.deinit();
    }

    pub fn debug_print(self: pFunc, level: usize) void {
        debug_print_level(level);

        print("Func '{}' with return type '", self.name);
        self.ptype.debug_print();
        println("':");

        self.block.debug_print(level+1);
    }
};
