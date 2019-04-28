#include "source/neslib_asm/neslib.h"
#include "source/menus/title.h"
#include "source/globals.h"
#include "source/configuration/game_states.h"
#include "source/configuration/system_constants.h"
#include "source/menus/text_helpers.h"
#include "source/graphics/palettes.h"
#include "source/configuration/game_info.h"
#include "source/graphics/fade_animation.h"

CODE_BANK(PRG_BANK_TITLE);

#include "graphics/title_1.h"
#include "graphics/prestart.h"

unsigned char titlePhase;


void draw_title_screen(void) {
	titlePhase = 0;
    ppu_off();
	pal_bg(titlePalette);
	pal_spr(titlePalette);

	set_chr_bank_0(CHR_BANK_MENU);
    set_chr_bank_1(CHR_BANK_MENU);
	oam_clear();

	vram_adr(NAMETABLE_A);
	vram_unrle(prestart);
	sfx_play(SFX_CLICKY, SFX_CHANNEL_1);
    
	ppu_on_all();
	gameState = GAME_STATE_TITLE_INPUT;
}

void draw_title_screen_real(void) {
	fade_out();
    ppu_off();
	pal_bg(titlePalette);
	pal_spr(titlePalette);

	set_chr_bank_0(CHR_BANK_MENU);
    set_chr_bank_1(CHR_BANK_MENU);
	clear_screen();
	oam_clear();

    
    put_str(NTADR_A(12, 5), gameName);
	
	put_str(NTADR_A(2, 26), gameAuthorContact);
	
	put_str(NTADR_A(2, 28), "Copyright");
	put_str(NTADR_A(12, 28), currentYear);
	put_str(NTADR_A(17, 28), gameAuthor);

	put_str(NTADR_A(10, 16), "Press Start!");
	ppu_on_all();
	music_play(SONG_TITLE);
	fade_in();
}

void draw_title1(void) {
	fade_out();
    ppu_off();


	vram_adr(NAMETABLE_A);
	vram_unrle(title_1);

	ppu_on_all();
	fade_in();

}

void handle_title_input(void) {

	++tempChara;
	if (titlePhase == 0 && tempChara == 192) {
		++titlePhase;
		draw_title_screen_real();
	}

	if (titlePhase == 1 && pad_trigger(0) & PAD_START) {
		++titlePhase;
		sfx_play(SFX_CLICKY, SFX_CHANNEL_1);

		draw_title1();
		return;
	}
	if (titlePhase == 2 && pad_trigger(0) & PAD_START) {
		gameState = GAME_STATE_POST_TITLE;
		sfx_play(SFX_CLICKY, SFX_CHANNEL_1);

		return;
	}
}