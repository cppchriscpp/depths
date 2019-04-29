/*
main.c is the entrypoint of your game. Everything starts from here.
This has the main loop for the game, which is then used to call out to other code.
*/

#include "source/neslib_asm/neslib.h"
#include "source/library/bank_helpers.h"
#include "source/configuration/game_states.h"
#include "source/menus/title.h"
#include "source/globals.h"
#include "source/menus/error.h"
#include "source/menus/credits.h"
#include "source/map/load_map.h"
#include "source/map/map.h"
#include "source/graphics/game_text.h"
#include "source/graphics/hud.h"
#include "source/graphics/fade_animation.h"
#include "source/sprites/player.h"
#include "source/menus/pause.h"
#include "source/sprites/map_sprites.h"
#include "source/sprites/sprite_definitions.h"
#include "source/menus/input_helpers.h"
#include "source/menus/game_over.h"


// Method to set a bunch of variables to default values when the system starts up.
// Note that if variables aren't set in this method, they will start at 0 on NES startup.
void initialize_variables(void) {

    playerOverworldPosition = 27; // Which tile on the overworld to start with; 0-62
    playerHealth = 5; // Player's starting health - how many hearts to show on the HUD.
    playerMaxHealth = 5; // Player's max health - how many hearts to let the player collect before it doesn't count.
    playerXPosition = (128 << PLAYER_POSITION_SHIFT) - 16; // X position on the screen to start (increasing numbers as you go left to right. Just change the number)
    playerYPosition = (128 << PLAYER_POSITION_SHIFT); // Y position on the screen to start (increasing numbers as you go top to bottom. Just change the number)
    playerDirection = SPRITE_DIRECTION_DOWN; // What direction to have the player face to start.

    lastSaveXPosition = playerXPosition;
    lastSaveYPosition = playerYPosition;
    lastSaveOverworldPosition = playerOverworldPosition;

    lastPlayerSpriteCollisionId = NO_SPRITE_HIT;
    lastTurnLastPlayerSpriteCollisionId = NO_SPRITE_HIT;

    currentWorldId = WORLD_OVERWORLD; // The ID of the world to load.
    playerStamina = PLAYER_START_MAX_STAMINA;
    playerMaxStamina = PLAYER_START_MAX_STAMINA;

    //waveDirection = SPRITE_DIRECTION_RIGHT;
    waveDirection = SPRITE_DIRECTION_STATIONARY;
    wavePosition = 0;
    hasGameOvered = 0;
    isStorming = 0;
    isEasyMode = 0;

    chrBankTiles = CHR_BANK_TILES;
    
    // Little bit of generic initialization below this point - we need to set
    // The system up to use a different hardware bank for sprites vs backgrounds.
    bank_spr(1);
}   

void main(void) {
    fade_out_instant();
    gameState = GAME_STATE_SYSTEM_INIT;

    while (1) {
        /*++everyOtherCycle;
        if (everyOtherCycle > 3) {
            everyOtherCycle = 0;
        }*/
        everyOtherCycle = !everyOtherCycle;
        switch (gameState) {
            case GAME_STATE_SYSTEM_INIT:
                initialize_variables();
                gameState = GAME_STATE_TITLE_DRAW;
                break;

            case GAME_STATE_TITLE_DRAW:
                banked_call(PRG_BANK_TITLE, draw_title_screen);
                fade_in();
                break;
            case GAME_STATE_TITLE_INPUT:
                banked_call(PRG_BANK_TITLE, handle_title_input);
                break;
            case GAME_STATE_POST_TITLE:

                music_stop();
                fade_out();

                playerStamina = playerMaxStamina;
                playerOverworldPosition = lastSaveOverworldPosition;
                playerXPosition = lastSaveXPosition;
                playerYPosition = lastSaveYPosition;


                load_map();

                banked_call(PRG_BANK_MAP_LOGIC, draw_current_map_to_a);
                banked_call(PRG_BANK_MAP_LOGIC, init_map);
                banked_call(PRG_BANK_MAP_LOGIC, load_sprites);
                
                // The draw map methods handle turning the ppu on/off, but we weren't quite done yet. Turn it back off.
                ppu_off();
                banked_call(PRG_BANK_HUD, draw_hud);
                ppu_on_all();

                // Seed the random number generator here, using the time since console power on as a seed
                set_rand(frameCount);
                
                // Map drawing is complete; let the player play the game!
                music_play(SONG_OVERWORLD);
                fade_in();
                gameState = GAME_STATE_RUNNING;
                if (isStorming) {
                    pal_bright(3);
                } else {
                    pal_bright(4);
                }


                banked_call(PRG_BANK_MAP_SPRITES, update_map_sprites);
                banked_call(PRG_BANK_PLAYER_SPRITE, update_player_sprite);
                banked_call(PRG_BANK_PLAYER_SPRITE, trigger_game_start_text);
                break;

            case GAME_STATE_RUNNING:
                // TODO: Might be nice to have this only called when we have something to update, and maybe only update the piece we 
                // care about. (For example, if you get a key, update the key count; not everything!
                banked_call(PRG_BANK_HUD, update_hud);
                banked_call(PRG_BANK_MAP_SPRITES, update_map_sprites);
                banked_call(PRG_BANK_PLAYER_SPRITE, handle_player_movement);
                banked_call(PRG_BANK_PLAYER_SPRITE, update_player_sprite);

                chrBankTiles = 3 + ((frameCount >> 6) & 0x01);

                set_chr_bank_0(chrBankTiles);
                tempChar1 = get_ppu_mask();

                if (isStorming) {
                    tempChar1 |= 0xe0;

                    tempChar2 = frameCount >> 3; // 0-31
                    tempChar3 = frameCount >> 2; // 0-63
                    if (tempChar2 < 4) {
                        if (tempChar2 == 0) {
                            sfx_play(SFX_BOOM, SFX_CHANNEL_1);
                        }
                        if (tempChar2 == 2) {
                            tempChar2 = 1;
                        }
                        if (tempChar2 == 3) {
                            tempChar2 = 0;
                        }
                        pal_bright(5+tempChar2);

                    } else {
                        pal_bright(3);
                    }
                } else {
                    tempChar1 &= 0x1f;
                    pal_bright(4);
                    
                }
                ppu_mask(tempChar1);

                break;
            case GAME_STATE_SCREEN_SCROLL:

                // Hide all non-player sprites in play, so we have an empty screen to add new ones to
                oam_hide_rest(FIRST_ENEMY_SPRITE_OAM_INDEX);

                // If you don't like the screen scrolling transition, you can replace the transition with `do_fade_screen_transition`
                banked_call(PRG_BANK_MAP_LOGIC, do_scroll_screen_transition);
                if (isStorming && playerOverworldPosition == 18) {
                    isStorming = 0;
                    music_play(SONG_OVERWORLD);
                }

                break;
            case GAME_STATE_SHOWING_TEXT:
                banked_call(PRG_BANK_GAME_TEXT, draw_game_text);

                // I don't love that I did this, but meh...
                ppu_off();
                banked_call(PRG_BANK_HUD, draw_hud);
                ppu_on_all();

                gameState = GAME_STATE_RUNNING;
                break;
            case GAME_STATE_PAUSED:
                fade_out();
                banked_call(PRG_BANK_PAUSE_MENU, draw_pause_screen);
                fade_in();
                banked_call(PRG_BANK_PAUSE_MENU, handle_pause_input);

                // When we get here, the player has unpaused. 
                // Pause has its own mini main loop in handle_input to make logic easier.
                fade_out();
                banked_call(PRG_BANK_MAP_LOGIC, draw_current_map_to_a);
                banked_call(PRG_BANK_MAP_LOGIC, init_map);
                
                // The draw map methods handle turning the ppu on/off, but we weren't quite done yet. Turn it back off.
                ppu_off();
                banked_call(PRG_BANK_HUD, draw_hud);
                ppu_on_all();
                fade_in();

                break;
            case GAME_STATE_GAME_OVER:
                fade_out();

                // Draw the "you lose" screen
                banked_call(PRG_BANK_GAME_OVER, draw_game_over_screen);
                fade_in();
                banked_call(PRG_BANK_MENU_INPUT_HELPERS, wait_for_start);
                if (hasGameOvered != 255)
                    ++hasGameOvered;
                oam_clear();
                gameState = GAME_STATE_POST_TITLE;
                break;
            case GAME_STATE_CREDITS:
                //music_stop();
                sfx_play(SFX_WIN, SFX_CHANNEL_1);
                music_play(SONG_WIN);

                fade_out();
                // Draw the "you won" screen

                bank_push(PRG_BANK_CREDITS_MENU);
                // NOTE: Using `j` here is kinda iffy at best. I don't think it's used but if stuff breaks, this could be why

                for (j = 0; j != 2; ++j) {
                    draw_win_screen(j);
                    fade_in();
                    banked_call(PRG_BANK_MENU_INPUT_HELPERS, wait_for_start);
                    fade_out();
                }

                // Folow it up with the credits.
                banked_call(PRG_BANK_CREDITS_MENU, draw_credits_screen);
                fade_in();
                banked_call(PRG_BANK_MENU_INPUT_HELPERS, wait_for_start);
                fade_out();

                for (j = 2; j != 7; ++j) {
                    draw_win_screen(j);
                    fade_in();
                    banked_call(PRG_BANK_MENU_INPUT_HELPERS, wait_for_start);
                    fade_out();
                }


                reset();
                break;
            default:
                crash_error_use_banked_details(ERR_UNKNOWN_GAME_STATE, ERR_UNKNOWN_GAME_STATE_EXPLANATION, "gameState value", gameState);
                
        }
        ppu_wait_nmi();
        
    }
}
