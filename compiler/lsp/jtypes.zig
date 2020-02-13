usingnamespace @import("../imports.zig");

usingnamespace lsp;



pub const ParseError: i64           = -32700;
pub const InvalidRequest: i64       = -32600;
pub const MethodNotFound: i64       = -32601;
pub const InvalidParams: i64        = -32602;
pub const InternalError: i64        = -32603;
pub const ServerErrorStart: i64     = -32099;
pub const ServerErrorEnd: i64       = -32000;
pub const ServerNotInitialized: i64 = -32002;
pub const UnknownErrorCode: i64     = -32001;
pub const RequestCancelled: i64     = -32800;
pub const ContentModified: i64      = -32801;


pub const Id = union(enum) {
    String: []const u8,
    Integer: i64,
};


pub const Request = struct {
    id: Id,
    method: []const u8,
    params: ?*json.ObjectMap,
};


pub const ResponseType = enum {
    Result,
    Error,
};


pub const ResultResponse = struct {
    id: Id,
    method: []const u8,
    results: *json.ObjectMap,
};


pub const ErrorResponse = struct {
    id: Id,
    method: []const u8,
    results: *json.ObjectMap,
};


pub const Response = union(ResponseType) {
    Result: ResultResponse,
    Error: ErrorResponse,
};


pub const Notification = struct {
    method: []const u8,
    params: *json.ObjectMap,
};


pub const Rpc = union(enum) {
    Request: Request,
    Notification: Notification,
};


pub fn MakeResponseError(code: i64, message: []const u8) !json.ObjectMap {
    var ResponseError = json.ObjectMap.init(allocator);

    _ = try ResponseError.put("code", json.Value{ .Integer = code });
    _ = try ResponseError.put("message", json.Value{ .String = message });

    return ResponseError;
}
