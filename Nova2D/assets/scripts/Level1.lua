-- Level1.lua  -  Jungle Landing
-- Lists are 0-indexed on purpose: LevelLoader.cpp starts reading at index 0.
-- Goal of every level: destroy ALL enemies.
--
-- Enemy fields
--   patrol = { min_x, max_x, min_y, max_y }  keeps the enemy driving back and forth on land
--   projectile_velocity: only the SPEED matters, enemies aim at the player by themselves
--   repeat_frequency: seconds between shots (whole numbers)

Level = {
    name = "Jungle Landing",

    assets = {
        [0] = { type = "texture", id = "jungle-tilemap",  file = "./assets/tilemaps/jungle.png" },
        { type = "texture", id = "chopper-texture",  file = "./assets/images/chopper-spritesheet.png" },
        { type = "texture", id = "tank-texture",     file = "./assets/images/tank-panther-right.png" },
        { type = "texture", id = "tiger-texture",    file = "./assets/images/tank-tiger-right.png" },
        { type = "texture", id = "truck-texture",    file = "./assets/images/truck-ford-right.png" },
        { type = "texture", id = "bullet-texture",   file = "./assets/images/bullet.png" },
        { type = "texture", id = "takeoff-texture",  file = "./assets/images/takeoff-base.png" },
        { type = "font",    id = "charriot-font",    file = "./assets/fonts/charriot.ttf", font_size = 14 },
        -- RenderHealthBarSystem.h asks for this exact id, so keep it
        { type = "font",    id = "pico8-font-5",     file = "./assets/fonts/charriot.ttf", font_size = 10 }
    },

    -- num_rows / num_cols MUST match jungle.map (the map is 1600 x 1280 pixels)
    tilemap = {
        map_file = "./assets/tilemaps/jungle.map",
        texture_asset_id = "jungle-tilemap",
        num_rows = 20,
        num_cols = 25,
        tile_size = 32,
        scale = 2.0
    },

    entities = {
        -- Player helicopter (starts on the small island, top-left)
        [0] = {
            tag = "player",
            components = {
                transform = { position = { x = 242, y = 110 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 0.0, y = 0.0 } },
                sprite = { texture_asset_id = "chopper-texture", width = 32, height = 32, z_index = 4 },
                animation = { num_frames = 2, speed_rate = 10 },
                boxcollider = { width = 32, height = 25, offset = { x = 0, y = 5 } },
                health = { health_percentage = 100 },
                -- repeat_frequency = 0 means "fire when SPACE is pressed"
                projectile_emitter = {
                    projectile_velocity = { x = 320, y = 320 },
                    repeat_frequency = 0,
                    projectile_duration = 2,
                    hit_percentage_damage = 34,
                    friendly = true
                },
                keyboard_controller = {
                    up_velocity    = { x = 0,    y = -160 },
                    right_velocity = { x = 160,  y = 0 },
                    down_velocity  = { x = 0,    y = 160 },
                    left_velocity  = { x = -160, y = 0 }
                },
                camera_follow = {}
            }
        },

        -- Take-off pad under the helicopter
        {
            components = {
                transform = { position = { x = 242, y = 110 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                sprite = { texture_asset_id = "takeoff-texture", width = 32, height = 32, z_index = 1 }
            }
        },

        -- Tank guarding the north coast
        {
            group = "enemies",
            components = {
                transform = { position = { x = 900, y = 122 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 0, y = 0.0 } },
                sprite = { texture_asset_id = "tank-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 110, y = 0 },
                    repeat_frequency = 3,
                    projectile_duration = 4,
                    hit_percentage_damage = 8,
                    friendly = false
                }
            }
        },

        -- Truck patrolling the north coast
        {
            group = "enemies",
            components = {
                transform = { position = { x = 900, y = 170 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 60, y = 0.0 } },
                sprite = { texture_asset_id = "truck-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 110, y = 0 },
                    repeat_frequency = 3,
                    projectile_duration = 4,
                    hit_percentage_damage = 8,
                    friendly = false
                },
                patrol = { min_x = 770, max_x = 1060, min_y = 170, max_y = 170 }
            }
        },

        -- Tank in the north-east base
        {
            group = "enemies",
            components = {
                transform = { position = { x = 1340, y = 220 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 0, y = 0.0 } },
                sprite = { texture_asset_id = "tank-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 110, y = 0 },
                    repeat_frequency = 3,
                    projectile_duration = 4,
                    hit_percentage_damage = 8,
                    friendly = false
                }
            }
        },

        -- Tank in the south jungle
        {
            group = "enemies",
            components = {
                transform = { position = { x = 760, y = 720 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 0, y = 0.0 } },
                sprite = { texture_asset_id = "tank-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 110, y = 0 },
                    repeat_frequency = 3,
                    projectile_duration = 4,
                    hit_percentage_damage = 8,
                    friendly = false
                }
            }
        }
    }
}
