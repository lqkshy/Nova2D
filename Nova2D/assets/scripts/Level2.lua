-- Level2.lua
-- Put this file in: Nova2D/assets/scripts/Level2.lua
-- Lists are 0-indexed on purpose: LevelLoader.cpp starts reading at index 0.

Level = {
    ----------------------------------------------------------------------------
    -- Assets
    ----------------------------------------------------------------------------
    assets = {
        [0] = { type = "texture", id = "jungle-tilemap",  file = "./assets/tilemaps/jungle.png" },
        { type = "texture", id = "chopper-texture", file = "./assets/images/chopper-spritesheet.png" },
        { type = "texture", id = "tank-texture",    file = "./assets/images/tank-panther-right.png" },
        { type = "texture", id = "bullet-texture",  file = "./assets/images/bullet.png" },
        { type = "font",    id = "charriot-font",   file = "./assets/fonts/charriot.ttf", font_size = 14 },
        -- RenderHealthBarSystem.h asks for this exact id, so keep it
        { type = "font",    id = "pico8-font-5",    file = "./assets/fonts/charriot.ttf", font_size = 10 }
    },

    ----------------------------------------------------------------------------
    -- Tilemap
    -- num_rows / num_cols MUST match your jungle.map file
    ----------------------------------------------------------------------------
    tilemap = {
        map_file = "./assets/tilemaps/jungle.map",
        texture_asset_id = "jungle-tilemap",
        num_rows = 20,
        num_cols = 25,
        tile_size = 32,
        scale = 2.0
    },

    ----------------------------------------------------------------------------
    -- Entities
    ----------------------------------------------------------------------------
    entities = {
        -- Player helicopter
        [0] = {
            tag = "player",
            components = {
                transform = {
                    position = { x = 242, y = 110 },
                    scale = { x = 1.0, y = 1.0 },
                    rotation = 0.0
                },
                rigidbody = {
                    velocity = { x = 0.0, y = 0.0 }
                },
                sprite = {
                    texture_asset_id = "chopper-texture",
                    width = 32,
                    height = 32,
                    z_index = 4
                },
                animation = {
                    num_frames = 2,
                    speed_rate = 10
                },
                boxcollider = {
                    width = 32,
                    height = 25,
                    offset = { x = 0, y = 5 }
                },
                health = {
                    health_percentage = 100
                },
                keyboard_controller = {
                    up_velocity    = { x = 0,   y = -50 },
                    right_velocity = { x = 50,  y = 0 },
                    down_velocity  = { x = 0,   y = 50 },
                    left_velocity  = { x = -50, y = 0 }
                },
                camera_follow = {}
            }
        },

        -- Enemy tank
        {
            group = "enemies",
            components = {
                transform = {
                    position = { x = 400, y = 300 },
                    scale = { x = 1.0, y = 1.0 },
                    rotation = 0.0
                },
                rigidbody = {
                    velocity = { x = 0.0, y = 0.0 }
                },
                sprite = {
                    texture_asset_id = "tank-texture",
                    width = 32,
                    height = 32,
                    z_index = 2
                },
                boxcollider = {
                    width = 32,
                    height = 32
                },
                health = {
                    health_percentage = 100
                },
                projectile_emitter = {
                    projectile_velocity = { x = 100, y = 0 },
                    repeat_frequency = 2,
                    projectile_duration = 5,
                    hit_percentage_damage = 10,
                    friendly = false
                }
            }
        }
    }
}