usingnamespace @import("../imports.zig");

usingnamespace parser;



fn is_operator(token: Token) bool {
    return switch(token.kind) {
        .Add,
        .Subtract,
        .Multiply,
        .Divide,
        .CompEqual,
        .CompNotEqual,
        .CompGreater,
        .CompGreaterEqual,
        .CompLess,
        .CompLessEqual
            => true,

        else => false
    };
}


fn get_operator_precedence(kind: pOperatorKind) i32 {
    return switch(kind) {
        .Add      => 2,
        .Subtract => 2,

        .Multiply => 3,
        .Divide   => 3,

        .CompEqual => 1,
        .CompNotEqual => 1,

        .CompGreater => 1,
        .CompGreaterEqual => 1,

        .CompLess => 1,
        .CompLessEqual => 1,
    };
}


fn parse_int(string: []u8) !?big.Int {
    var int = try big.Int.init(heap.c_allocator);
    int.setString(10, string) catch {
        int.deinit();
        return null;
    };
    return int;
}


pub fn parse_expression(self: *TokenIterator) anyerror!*pExpression {
    const InRpn = union(enum) {
        Expression: *pExpression,
        Operator: pOperatorKind,
    };

    var rpn = std.ArrayList(InRpn).init(heap.c_allocator);
    defer rpn.deinit();

    var operators = std.ArrayList(pOperatorKind).init(heap.c_allocator);
    defer operators.deinit();

    const ExpressionExpected = enum {
        Value,
        Operator,
    };
    var expected: ExpressionExpected = .Value;

    while(true) {
        if(is_operator(self.token())) { //Parse operator
            if(expected == .Value) {
                parse_error(self.token(), "Expected value but found operator '{}'", self.token().string);
            }

            var operator = switch(self.token().kind) {
                .Add => pOperatorKind.Add,
                .Subtract => pOperatorKind.Subtract,

                .Multiply => pOperatorKind.Multiply,
                .Divide => pOperatorKind.Divide,

                .CompEqual => pOperatorKind.CompEqual,
                .CompNotEqual => pOperatorKind.CompNotEqual,

                .CompGreater => pOperatorKind.CompGreater,
                .CompGreaterEqual => pOperatorKind.CompGreaterEqual,

                .CompLess => pOperatorKind.CompLess,
                .CompLessEqual => pOperatorKind.CompLessEqual,

                else => unreachable,
            };

            var precedence = get_operator_precedence(operator);

            var pop_count: usize = 0;
            for(operators.toSlice()) |in_queue| {
                var in_queue_precedence = get_operator_precedence(in_queue);
                if(precedence == in_queue_precedence or precedence < in_queue_precedence) {
                    pop_count += 1;
                    try rpn.append(InRpn { .Operator = in_queue });
                }
                else {
                    break;
                }
            }

            try operators.resize(operators.len - pop_count);

            try operators.append(operator);

            expected = .Value;
        }
        else { //Parse value
            if(expected == .Operator) {
                parse_error(self.token(), "Expected operator but found value");
            }

            if(try parse_int(self.token().string)) |int| {
                var expression = try pExpression.init(
                    pExpression {
                        .Int = int
                    }
                );
                try rpn.append(InRpn { .Expression = expression });
            }
            else {
                parse_error(self.token(), "Unexpected token '{}'", self.token().string);
            }

            expected = .Operator;
        }

        if(!self.has_next()) {
            break;
        }
        else {
            switch(self.peek().kind) {
                .Semicolon => break,
                else => self.next(),
            }
        }
    }

    if(expected == .Value) {
        parse_error(self.token(), "Operator '{}' has no right side value", self.token().string);
    }

    var operators_slice = operators.toSlice();
    var index: usize = operators_slice.len - 1;
    while(true) { //Add each remaining operator to RPN in reverse order
        try rpn.append(InRpn { .Operator = operators_slice[index] });

        //Because usize has 0 as min we cannot check this in the while condition as it would underflow
        if(index > 0) {
            index -= 1;
        }
        else {
            break;
        }
    }

    println("Operators start: =======================");
    for(operators.toSlice()) |entry| {
        debug_print_level(1);
        println("{}", entry);
    }
    println("Operators end: =========================");
    println("");

    println("RPN start: =============================");
    for(rpn.toSlice()) |entry| {
        switch(entry) {
            .Expression => |expression| expression.debug_print(1),
            .Operator => |operator| {
                debug_print_level(1);
                println("{}", operator);
            }
        }
    }
    println("RPN end: ===============================");
    println("");

    var stack = std.ArrayList(*pExpression).init(heap.c_allocator);
    defer stack.deinit();

    for(rpn.toSlice()) |entry| { //Convert RPN to pExpression tree
        switch(entry) {
            .Expression => |expression| try stack.append(expression),

            .Operator => |operator| {
                var right = stack.pop();
                var left = stack.pop();

                var expression = try pExpression.init(
                    pExpression {
                        .Operator = pOperator {
                            .kind = operator,
                            .left = left,
                            .right = right,
                        }
                    }
                );
                try stack.append(expression);
            },
        }
    }

    //We should now have only one pExpression in the stack
    //It is the root of the expression tree we just built
    if(stack.len != 1) {
        internal_error("Expression parse tree building stack does not have only one remaining entry");
    }
    return stack.pop();
}
