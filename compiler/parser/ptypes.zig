usingnamespace @import("../imports.zig");



pub fn debug_print_level(level: u64) void {
    print_many(level, "    ", .{});
}


pub const pType = struct {
    name: []u8,
    reach_module: []u8,
    child: ?*pType,

    pub fn init(name: []u8, reach_module: []u8, child: ?*pType) !*pType {
        var self = try allocator.create(pType);
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
        allocator.destroy(self);
    }

    pub fn debug_print(self: *pType) void {
        if(self.child) |actual| {
            actual.debug_print();
            print(":", .{});
        }

        print("{}", .{self.name});
    }
};


pub const pFile = struct {
    file: *FileInfo,
    module_path: std.ArrayList([]u8),
    block: pBlock,

    pub fn init(pfile: pFile) !*pFile {
        var self = try allocator.create(pFile);
        self.* = pfile;
        return self;
    }

    pub fn deinit(self: *
pFile) void {
        self.module_path.deinit();
        self.block.deinit();
        allocator.destroy(self);
    }

    pub fn debug_print(self: pFile, level: u64) void {
        debug_print_level(level);
        println("File '{}':", .{self.file.path});

        self.block.debug_print(level+1);
    }
};


pub const pModule = struct {
    name: []const u8,
    pfiles: std.ArrayList(*pFile),
    children: std.StringHashMap(pModule),

    pub fn deinit(self: pModule) void {
        for(self.pfiles.toSlice()) |pfile| {
            pfile.deinit();
        }
        self.pfiles.deinit();

        var children_iterator = self.children.iterator();
        while(children_iterator.next()) |child| {
            child.value.deinit();
        }
        self.children.deinit();
    }

    pub fn debug_print(self: pModule, level: u64) void {
        debug_print_level(level);
        println("Module '{}':", .{self.name});

        for(self.pfiles.toSlice()) |pfile| {
            pfile.debug_print(level+1);
        }

        var children_iterator = self.children.iterator();
        while(children_iterator.next()) |child| {
            child.value.debug_print(level+1);
        }
    }
};


pub const pBlock = struct {
    contents: std.ArrayList(InBlock),

    pub const InBlock = union(enum) {
        Let: pLet,
        Func: pFunc,
        Expression: *pExpression,
    };

    pub fn init() pBlock {
        return pBlock {
            .contents = std.ArrayList(InBlock).init(allocator),
        };
    }

    pub fn deinit(self: pBlock) void {
        for(self.contents.toSlice()) |*item| {
            switch(item.*) {
                .Let => |*let| {
                    let.deinit();
                },

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

    pub fn debug_print(self: pBlock, level: u64) void {
        debug_print_level(level);

        if(self.contents.len == 0) {
            println("Block: (Empty)", .{});
        }
        else {
            println("Block:", .{});

            for(self.contents.toSlice()) |item| {
                switch(item) {
                    .Let => |let| let.debug_print(level+1),
                    .Func => |func| func.debug_print(level+1),
                    .Expression => |expression| expression.debug_print(level+1),
                }
            }
        }
    }
};


pub const pOperatorKind = enum {
    Add,
    Subtract,

    Multiply,
    Divide,

    CompEqual,
    CompNotEqual,

    CompGreater,
    CompGreaterEqual,

    CompLess,
    CompLessEqual,
};


pub const pOperator = struct {
    kind: pOperatorKind,
    left: *pExpression,
    right: *pExpression,

    pub fn deinit(self: *pOperator) void {
        self.left.deinit();
        self.right.deinit();
    }

    pub fn debug_print(self: pOperator, level: u64) void {
        debug_print_level(level);
        println("{}:", .{@tagName(self.kind)});
        self.left.debug_print(level+1);
        self.right.debug_print(level+1);
    }
};


pub const pNegate = struct {
    expression: *pExpression,

    pub fn deinit(self: *pNegate) void {
        self.expression.deinit();
    }

    pub fn debug_print(self: pNegate, level: u64) void {
        debug_print_level(level);
        println("Negate:", .{});
        self.expression.debug_print(level+1);
    }
};


pub const pExpression = union(enum) {
    Int: big.Int,
    Negate: pNegate,
    Operator: pOperator,

    //This feels like a hack but allows the allocation to be done in
    //the init which is consistant and less surprising in the long
    //run. This consistency allows for directly destroying in the
    //deinit functions when the need arises
    pub fn init(exp: pExpression) !*pExpression {
        var self = try allocator.create(pExpression);
        self.* = exp;
        return self;
    }

    pub fn deinit(self: *pExpression) void {
        switch(self.*) {
            .Int => |*int| int.deinit(),
            .Negate => |*negate| negate.deinit(),
            .Operator => |*math| math.deinit(),
        }

        allocator.destroy(self);
    }

    pub fn is_math(self: pExpression) bool {
        return switch(self) {
            Operator => true,
            else => false,
        };
    }

    pub fn debug_print(self: pExpression, level: u64) void {
        debug_print_level(level);
        println("Expression:", .{});

        switch(self) {
            .Int => |int| {
                debug_print_level(level+1);

                var str = int.toString(allocator, 10) catch unreachable;
                defer allocator.free(str);
                println("Int: {}", .{str});
            },
            .Negate => |negate| negate.debug_print(level+1),
            .Operator => |operator| operator.debug_print(level+1),
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

    pub fn debug_print(self: pLet, level: u64) void {
        debug_print_level(level);

        print("Let '{}' with type '", .{self.name});
        self.ptype.debug_print();
        println("'", .{});

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

    pub fn debug_print(self: pFunc, level: u64) void {
        debug_print_level(level);

        print("Func '{}' with return type '", .{self.name});
        self.ptype.debug_print();
        println("':", .{});

        self.block.debug_print(level+1);
    }
};
