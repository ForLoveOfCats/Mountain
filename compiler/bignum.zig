usingnamespace @import("./imports.zig");

const math = std.math;
const Allocator = mem.Allocator;
const ArrayList = std.ArrayList;
const maxInt = std.math.maxInt;

usingnamespace std.math.big;



//Modified version of `math.big.Int.toString` to fix memory leaks
pub fn toString(self: Int, allocator: *Allocator, base: u8) ![]const u8 {
    if (base < 2 or base > 16) {
        return error.InvalidBase;
    }

    var digits = ArrayList(u8).init(allocator);
    try digits.ensureCapacity(self.sizeInBase(base) + 1);
    defer digits.deinit();

    if (self.eqZero()) {
        try digits.append('0');
        return digits.toOwnedSlice();
    }

    // Power of two: can do a single pass and use masks to extract digits.
    if (math.isPowerOfTwo(base)) {
        const base_shift = math.log2_int(Limb, base);

        for (self.limbs[0..self.len()]) |limb| {
            var shift: usize = 0;
            while (shift < Limb.bit_count) : (shift += base_shift) {
                const r = @intCast(u8, (limb >> @intCast(Log2Limb, shift)) & Limb(base - 1));
                const ch = try digitToChar(r, base);
                try digits.append(ch);
            }
        }

        while (true) {
            // always will have a non-zero digit somewhere
            const c = digits.pop();
            if (c != '0') {
                digits.append(c) catch unreachable;
                break;
            }
        }
    } // Non power-of-two: batch divisions per word size.
    else {
        const digits_per_limb = math.log(Limb, base, maxInt(Limb));
        var limb_base: Limb = 1;
        var j: usize = 0;
        while (j < digits_per_limb) : (j += 1) {
            limb_base *= base;
        }

        var q = try self.clone();
        defer q.deinit();
        q.abs();
        var r = try Int.init(allocator);
        defer r.deinit();
        var b = try Int.initSet(allocator, limb_base);
        defer b.deinit();

        while (q.len() >= 2) {
            try Int.divTrunc(&q, &r, q, b);

            var r_word = r.limbs[0];
            var i: usize = 0;
            while (i < digits_per_limb) : (i += 1) {
                const ch = try digitToChar(@intCast(u8, r_word % base), base);
                r_word /= base;
                try digits.append(ch);
            }
        }

        {
            debug.assert(q.len() == 1);

            var r_word = q.limbs[0];
            while (r_word != 0) {
                const ch = try digitToChar(@intCast(u8, r_word % base), base);
                r_word /= base;
                try digits.append(ch);
            }
        }
    }

    if (!self.isPositive()) {
        try digits.append('-');
    }

    var s = digits.toOwnedSlice();
    mem.reverse(u8, s);
    return s;
}


fn digitToChar(d: u8, base: u8) !u8 {
    if (d >= base) {
        return error.DigitTooLargeForBase;
    }

    return switch (d) {
        0...9 => '0' + d,
        0xa...0xf => ('a' - 0xa) + d,
        else => unreachable,
    };
}
