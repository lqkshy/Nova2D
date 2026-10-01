-- Level3.lua  -  Fortress Assault
-- Lists are 0-indexed on purpose: LevelLoader.cpp starts reading at index 0.
-- Goal of every level: destroy ALL enemies.
--
-- Enemy fields
--   patrol = { min_x, max_x, min_y, max_y }  keeps the enemy driving back and forth on land
--   projectile_velocity: only the SPEED matters, enemies aim at the player by themselves
--   repeat_frequency: seconds between shots (whole numbers)

Level = {
    name = "Fortress Assault",

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
                    hit_percentage_damage = 20,
                    friendly = true
                },
                keyboard_controller = {
                    up_velocity    = { x = 0,    y = -170 },
                    right_velocity = { x = 170,  y = 0 },
                    down_velocity  = { x = 0,    y = 170 },
                    left_velocity  = { x = -170, y = 0 }
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

        {
            group = "enemies",
            components = {
                transform = { position = { x = 790, y = 122 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 0, y = 0.0 } },
                sprite = { texture_asset_id = "tiger-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 170, y = 0 },
                    repeat_frequency = 2,
                    projectile_duration = 4,
                    hit_percentage_damage = 12,
                    friendly = false
                }
            }
        },

        {
            group = "enemies",
            components = {
                transform = { position = { x = 900, y = 170 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 80, y = 0.0 } },
                sprite = { texture_asset_id = "tiger-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 170, y = 0 },
                    repeat_frequency = 2,
                    projectile_duration = 4,
                    hit_percentage_damage = 12,
                    friendly = false
                },
                patrol = { min_x = 790, max_x = 1060, min_y = 170, max_y = 170 }
            }
        },

        {
            group = "enemies",
            components = {
                transform = { position = { x = 1390, y = 230 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 0, y = 0.0 } },
                sprite = { texture_asset_id = "tiger-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 170, y = 0 },
                    repeat_frequency = 2,
                    projectile_duration = 4,
                    hit_percentage_damage = 12,
                    friendly = false
                }
            }
        },

        {
            group = "enemies",
            components = {
                transform = { position = { x = 790, y = 330 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 0, y = 0.0 } },
                sprite = { texture_asset_id = "tank-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 170, y = 0 },
                    repeat_frequency = 2,
                    projectile_duration = 4,
                    hit_percentage_damage = 12,
                    friendly = false
                }
            }
        },

        {
            group = "enemies",
            components = {
                transform = { position = { x = 680, y = 415 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 0, y = 0.0 } },
                sprite = { texture_asset_id = "tank-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 170, y = 0 },
                    repeat_frequency = 2,
                    projectile_duration = 4,
                    hit_percentage_damage = 12,
                    friendly = false
                }
            }
        },

        {
            group = "enemies",
            components = {
                transform = { position = { x = 800, y = 730 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 90, y = 0.0 } },
                sprite = { texture_asset_id = "tiger-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 170, y = 0 },
                    repeat_frequency = 2,
                    projectile_duration = 4,
                    hit_percentage_damage = 12,
                    friendly = false
                },
                patrol = { min_x = 640, max_x = 940, min_y = 730, max_y = 730 }
            }
        },

        {
            group = "enemies",
            components = {
                transform = { position = { x = 1100, y = 790 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 0, y = 0.0 } },
                sprite = { texture_asset_id = "tank-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 170, y = 0 },
                    repeat_frequency = 2,
                    projectile_duration = 4,
                    hit_percentage_damage = 12,
                    friendly = false
                }
            }
        },

        {
            group = "enemies",
            components = {
                transform = { position = { x = 1400, y = 760 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 0, y = 0.0 } },
                sprite = { texture_asset_id = "tiger-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 170, y = 0 },
                    repeat_frequency = 2,
                    projectile_duration = 4,
                    hit_percentage_damage = 12,
                    friendly = false
                }
            }
        },

        {
            group = "enemies",
            components = {
                transform = { position = { x = 1420, y = 870 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 70, y = 0.0 } },
                sprite = { texture_asset_id = "truck-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 170, y = 0 },
                    repeat_frequency = 2,
                    projectile_duration = 4,
                    hit_percentage_damage = 12,
                    friendly = false
                },
                patrol = { min_x = 1340, max_x = 1510, min_y = 870, max_y = 870 }
            }
        },

        {
            group = "enemies",
            components = {
                transform = { position = { x = 1380, y = 500 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 0, y = 0.0 } },
                sprite = { texture_asset_id = "tank-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 170, y = 0 },
                    repeat_frequency = 2,
                    projectile_duration = 4,
                    hit_percentage_damage = 12,
                    friendly = false
                }
            }
        },

        {
            group = "enemies",
            components = {
                transform = { position = { x = 470, y = 420 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 0, y = 0.0 } },
                sprite = { texture_asset_id = "tank-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 170, y = 0 },
                    repeat_frequency = 2,
                    projectile_duration = 4,
                    hit_percentage_damage = 12,
                    friendly = false
                }
            }
        },

        -- Tank on the west road
        {
            group = "enemies",
            components = {
                transform = { position = { x = 250, y = 497 }, scale = { x = 1.0, y = 1.0 }, rotation = 0.0 },
                rigidbody = { velocity = { x = 60, y = 0.0 } },
                sprite = { texture_asset_id = "tank-texture", width = 32, height = 32, z_index = 2 },
                boxcollider = { width = 32, height = 32 },
                health = { health_percentage = 100 },
                projectile_emitter = {
                    projectile_velocity = { x = 170, y = 0 },
                    repeat_frequency = 2,
                    projectile_duration = 4,
                    hit_percentage_damage = 12,
                    friendly = false
                },
                patrol = { min_x = 150, max_x = 380, min_y = 497, max_y = 497 }
            }
        }
    }
}
