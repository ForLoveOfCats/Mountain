usingnamespace @import("../imports.zig");



pub fn debug_print_level(level: usize) void {
    print_many(level, "    ");
}


pub const pType = struct {
    name: []u8,
    reach_module: []u8,
    child: ?*pType,

    pub fn init(name: []u8, reach_module: []u8, child: ?*pType) !*pType {
        var self = try heap.c_allocator.create(pType);
        self.* = pType {
            .name = name,
            .reach_module = reach_module,
            .child = child,
        };
        return self;
    }

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
    children: std.StringHashMap(pModule),

    pub fn deinit(self: pModule) void {
        self.block.deinit();

        var children_iterator = self.children.iterator();
        while(children_iterator.next()) |child| {
            child.value.deinit();
        }
        self.children.deinit();
    }

    pub fn debug_print(self: pModule, level: usize) void {
        debug_print_level(level);
        println("Module '{}':", self.name);

        self.block.debug_print(level+1);

        var children_iterator = self.children.iterator();
        while(children_iterator.next()) |child| {
            child.value.debug_print(level+1);
        }
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

        if(self.contents.len == 0) {
            println("Block: (Empty)");
        }
        else {
            println("Block:");

            for(self.contents.toSlice()) |item| {
                switch(item) {
                    .Func => |func| func.debug_print(level+1),
                    .Expression => |expression| expression.debug_print(level+1),
                }
            }
        }
    }
};


pub const pMathKind = enum {
    Add,
    Subtract,
    Multiply,
    Divide,
};


pub const pMath = struct {
    kind: pMathKind,
    left: *pExpression,
    right: *pExpression,

    pub fn deinit(self: *pMath) void {
        self.left.deinit();
        self.right.deinit();
    }

    pub fn debug_print(self: pMath, level: usize) void {
        debug_print_level(level);
        println("{}:", @tagName(self.kind));
        self.left.debug_print(level+1);
        self.right.debug_print(level+1);
    }
};


pub const pNegate = struct {
    expression: *pExpression,

    pub fn deinit(self: *pNegate) void {
        self.expression.deinit();
    }

    pub fn debug_print(self: pNegate, level: usize) void {
        debug_print_level(level);
        println("Negate:");
        self.expression.debug_print(level+1);
    }
};


pub const pExpression = union(enum) {
    Let: pLet,
    Int: big.Int,
    Negate: pNegate,
    Math: pMath,

    //This feels like a hack but allows the allocation to be done in
    //the init which is consistant and less surprising in the long
    //run. This consistency allows for directly destroying in the
    //deinit functions when the need arises
    pub fn init(exp: pExpression) !*pExpression {
        var self = try heap.c_allocator.create(pExpression);
        self.* = exp;
        return self;
    }

    pub fn deinit(self: *pExpression) void {
        switch(self.*) {
            .Let => |*let| let.deinit(),
            .Int => |*int| int.deinit(),
            .Negate => |*negate| negate.deinit(),
            .Math => |*math| math.deinit(),
        }

        heap.c_allocator.destroy(self);
    }

    pub fn is_math(self: pExpression) bool {
        return switch(self) {
            Math => true,
            else => false,
        };
    }

    pub fn debug_print(self: pExpression, level: usize) void {
        debug_print_level(level);
        println("Expression:");

        switch(self) {
            .Let => |let| let.debug_print(level+1),
            .Int => |int| {
                debug_print_level(level+1);

                var str = bignum.toString(int, heap.c_allocator, 10) catch unreachable;
                defer heap.c_allocator.free(str);
                println("Int: {}", str);
            },
            .Negate => |negate| negate.debug_print(level+1),
            .Math => |math| math.debug_print(level+1),
        }
    }
};


pub const pLet = struct {
    name: []u8,
    ptype: *pType,
    expression: ?*pExpression,

    pub fn deinit(self: *pLet) void {
        self.ptype.deinit();

        if(self.expression) |actual| {
            actual.deinit();
        }
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
