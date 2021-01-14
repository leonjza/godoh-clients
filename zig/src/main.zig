const std = @import("std");
const options = @import("options.zig");
const utils = @import("utils.zig");

const log = std.log;


pub fn main() anyerror!void {

    log.debug("booting for domain: {s}", .{ options.domain });
    log.debug("sleep jitter max is: {}...", .{ options.sleep_time });

    // setup the jitter sleeper
    var j_sleep = utils.JitterSleep.init(options.sleep_time);


    while (true) {
        log.debug("performing jittered sleep", .{});
        j_sleep.sleep(); 
        log.debug("waking up after sleep", .{});

        // TODO: actual lookups and more!
    }

    log.info("All your codebase are belong to us.", .{});
}
