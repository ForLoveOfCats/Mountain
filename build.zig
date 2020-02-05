const std = @import("std");
const builtin = @import("builtin");
const Builder = std.build.Builder;



pub fn build(b: *Builder) void
{
    const mode = b.standardReleaseOptions();
    const exe = b.addExecutable("Mountain", "compiler/compiler.zig");
    exe.setBuildMode(mode);

    exe.linkSystemLibrary("c");

    const run_cmd = exe.run();
    if(b.args) |args| {
        run_cmd.addArgs(args);
    }
    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);

    exe.install();
}
