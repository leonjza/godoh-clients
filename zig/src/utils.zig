const std = @import("std");

const rand = std.rand;
const time = std.time;
const log = std.log;
const expect = std.testing.expect;


pub const JitterSleep = struct {

    rng: rand.DefaultPrng,
    max: u64,

    pub fn init(max: u64) JitterSleep {
        return JitterSleep {
            .rng = rand.DefaultPrng.init(@intCast(u64, time.timestamp())) ,
            .max = max,
        };
    }

    pub fn randInt(j: *JitterSleep) u64 {
        var t: u64 = 0;
        while (t <= 0) {
            t = j.rng.random.uintLessThanBiased(u64, j.max);
        }
        return t;
    }

    pub fn sleep(j: *JitterSleep) void {
        var t = j.randInt();
        log.debug("sleeping for {}s", .{ t });
        time.sleep(t * time.ns_per_s);
    }
};

test "inits jitter sleep" {
    var j = JitterSleep.init(0);
    expect(@TypeOf(j) == JitterSleep);
}

test "inits jitter with correct max value" {
    var j = JitterSleep.init(5);
    expect(j.max == 5);
}

test "produces sleep seconds less than max value" {
    var j = JitterSleep.init(20);
    expect(j.randInt() < 20);
    expect(j.randInt() < 20);
    expect(j.randInt() < 20);
}
