const std = @import("std");
const Builder = std.build.Builder;



pub fn build(b: *Builder) void
{
    const mode = b.standardReleaseOptions();
    const exe = b.addExecutable("Mountain", "compiler/compiler.zig");
    exe.setBuildMode(mode);

    exe.linkSystemLibrary("c");

    exe.install();
}
