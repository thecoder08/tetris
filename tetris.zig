const drawing = @cImport(@cInclude("xgfx/drawing.h"));
const window = @cImport(@cInclude("xgfx/window.h"));
const stdlib = @cImport(@cInclude("stdlib.h"));
const time = @cImport(@cInclude("time.h")).time;

const Piece = struct { x: [4]i8, y: [4]i8, centerX: i8, centerY: i8, color: u8 };

extern const tileData: [7][20][20][3]u8;
var tiles: [20][10]u8 = undefined;

fn drawTile(x: i32, y: i32, color: u8) void {
    if (color == 0 or color > 7) {
        drawing.rectangle(x * 20, y * 20, 20, 20, 0x00000000);
        return;
    }
    for (0..20) |i| {
        for (0..20) |j| {
            drawing.plot(x * 20 + @as(i32, @intCast(j)), y * 20 + @as(i32, @intCast(i)), @as(c_int, tileData[color - 1][i][j][0]) << 16 | @as(c_int, tileData[color - 1][i][j][1]) << 8 | @as(c_int, tileData[color - 1][i][j][2]));
        }
    }
}

const templatePieces: [7]Piece = .{ .{ .x = .{ 0, 0, 0, 0 }, .y = .{ -1, 0, 1, 2 }, .centerX = 4, .centerY = 1, .color = 1 }, .{ .x = .{ -1, 0, 1, 1 }, .y = .{ 0, 0, 0, -1 }, .centerX = 4, .centerY = 1, .color = 2 }, .{ .x = .{ 1, 0, -1, -1 }, .y = .{ 0, 0, 0, -1 }, .centerX = 4, .centerY = 1, .color = 3 }, .{ .x = .{ 0, 1, 0, 1 }, .y = .{ 0, 0, 1, 1 }, .centerX = 4, .centerY = 1, .color = 4 }, .{ .x = .{ 0, -1, 1, 0 }, .y = .{ 0, 0, 0, 1 }, .centerX = 4, .centerY = 1, .color = 5 }, .{ .x = .{ -1, 0, 0, 1 }, .y = .{ -1, -1, 0, 0 }, .centerX = 4, .centerY = 1, .color = 6 }, .{ .x = .{ 1, 0, 0, -1 }, .y = .{ -1, -1, 0, 0 }, .centerX = 4, .centerY = 1, .color = 7 } };

var fallingPiece: Piece = undefined;
var nextPiece: Piece = undefined;

fn draw() void {
    drawing.rectangle(0, 0, 400, 400, 0x00555555);
    // draw tile grid
    for (0..20) |i| {
        for (0..10) |j| {
            drawTile(@as(i32, @intCast(j)), @as(i32, @intCast(i)), tiles[i][j]);
        }
    }
    // draw falling piece
    for (0..4) |i| {
        drawTile(fallingPiece.centerX + fallingPiece.x[i], fallingPiece.centerY + fallingPiece.y[i], fallingPiece.color);
    }
    drawing.rectangle(260, 80, 60, 80, 0);
    // draw next piece
    for (0..4) |i| {
        drawTile(14 + nextPiece.x[i], 5 + nextPiece.y[i], nextPiece.color);
    }
    window.updateWindow();
}

fn update() void {
    var canFall: bool = true;
    // check if piece can fall
    for (0..4) |i| {
        if (fallingPiece.centerY + fallingPiece.y[i] + 1 == 20 or tiles[@as(usize, @intCast(fallingPiece.centerY + fallingPiece.y[i] + 1))][@as(usize, @intCast(fallingPiece.centerX + fallingPiece.x[i]))] > 0) {
            canFall = false;
            break;
        }
    }
    if (canFall) {
        // if so, move piece down
        fallingPiece.centerY += 1;
    } else {
        // otherwise, snap piece to tile grid
        for (0..4) |i| {
            tiles[@as(usize, @intCast(fallingPiece.centerY + fallingPiece.y[i]))][@as(usize, @intCast(fallingPiece.centerX + fallingPiece.x[i]))] = fallingPiece.color;
        }
        // and pick new piece
        fallingPiece = nextPiece;
        nextPiece = templatePieces[@as(usize, @intCast(@mod(stdlib.rand(), 7)))];
    }
    // check for full lines
    for (0..20) |i| {
        var lineFull: bool = true;
        for (0..10) |j| {
            if (tiles[i][j] == 0) {
                lineFull = false;
                break;
            }
        }
        if (lineFull) {
            var j: usize = i;
            while (j > 0) {
                for (0..10) |k| {
                    tiles[j][k] = tiles[j - 1][k];
                }
                j -= 1;
            }
        }
    }
}

pub fn main() void {
    window.initWindow(400, 400, "Tetris");
    stdlib.srand(@intCast(time(null)));
    fallingPiece = templatePieces[@as(usize, @intCast(@mod(stdlib.rand(), 7)))];
    nextPiece = templatePieces[@as(usize, @intCast(@mod(stdlib.rand(), 7)))];
    var updateTick: i32 = 0;
    while (true) {
        var event: window.Event = undefined;
        while (window.checkWindowEvent(&event) > 0) {
            if (event.type == window.WINDOW_CLOSE) {
                return;
            }
            if (event.type == window.KEY_CHANGE and event.keychange.state == 1) {
                switch (event.keychange.key) {
                    105 => {
                        fallingPiece.centerX -= 1;
                        draw();
                    },
                    106 => {
                        fallingPiece.centerX += 1;
                        draw();
                    },
                    103 => {
                        for (0..4) |i| {
                            const newX: i8 = fallingPiece.y[i];
                            fallingPiece.y[i] = -fallingPiece.x[i];
                            fallingPiece.x[i] = newX;
                        }
                        draw();
                    },
                    108 => {
                        update();
                        draw();
                    },
                    else => {},
                }
            }
        }
        if (updateTick == 0) {
            update();
            draw();
        }
        updateTick += 1;
        if (updateTick == 1000000) {
            updateTick = 0;
        }
    }
}
