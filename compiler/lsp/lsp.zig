usingnamespace @import("../imports.zig");

pub usingnamespace @import("server.zig");



pub const Id = union(enum) {
    String: []const u8,
    Integer: i64,
};


pub const Request = struct {
    id: Id,
    method: []const u8,
    params: *json.ObjectMap,
};


pub const Notification = struct {
    method: []const u8,
    params: *json.ObjectMap,
};


pub const Rpc = union(enum) {
    Request: Request,
    Notification: Notification,
};
