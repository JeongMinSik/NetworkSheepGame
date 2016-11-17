#include "stdafx.h"
#include "UI.h"
#include "SoundPackage.h"
#include "Objects.h"
#include "Sheep.h"

Ui::Ui( int size ) : canvas_size( size ), selected_menu( 0 ), heart_size( 0.5 ), heart_dir( 1 ), presskey( false ), help( 0 ) {

	for( int i = 0; i < 2; ++i )
		key_delay[i] = 0;
}
Ui::~Ui() { }
int Ui::keyboard( unsigned char key )
{
	if( *pGameMode == MAIN_MODE && selected_menu == 1 && key_delay[0] == 0 )
	{
		FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_OK_E], 0, &pSound->Channel[BUTTON_OK_E] );
		key_delay[0] = 3;
		help = ( help + 1 ) % 3;
	}
	//if (key == 27 && key_delay[0] == 0) // ESC
	//{
	//	if (Game_Mode == PLAY_MODE)
	//	{
	//		key_delay[0] = 5;
	//		FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[BUTTON_OK_E], 0, &g_Channel[BUTTON_OK_E]);
	//		FMOD_Channel_SetPaused(g_Channel[GAME_BGM], true);
	//		selected_menu = 0;
	//		Game_Mode = PAUSE_MODE;
	//	}
	//	else if (Game_Mode == PAUSE_MODE)
	//	{
	//		key_delay[0] = 5;
	//		FMOD_System_PlaySound(g_System, FMOD_CHANNEL_FREE, g_Sound[BUTTON_OK_E], 0, &g_Channel[BUTTON_OK_E]);
	//		FMOD_Channel_SetPaused(g_Channel[GAME_BGM], false);
	//		selected_menu = 0;
	//		Game_Mode = PLAY_MODE;
	//	}
	//}
	if( ( key == ' ' || key == 13 ) && key_delay[0] == 0 )
	{
		if( *pGameMode == MAIN_MODE )
		{
			FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_OK_E], 0, &pSound->Channel[BUTTON_OK_E] );
			if( presskey == false )
			{
				FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_OK_E], 0, &pSound->Channel[BUTTON_OK_E] );
				presskey = true;
			}
			else if( selected_menu == 0 )
			{
				// 레디
				FMOD_Channel_Stop( pSound->Channel[MAIN_BGM] );
				return READY_MODE;
			}
			else if( selected_menu == 2 )
			{
				//종료
				exit( 0 );
			}
		}
		// 엔딩메뉴
		else if( ending_screen == 3 )
		{
			FMOD_Channel_Stop( pSound->Channel[CLEAR_BGM] );
			ending_screen = 0;
			FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_OK_E], 0, &pSound->Channel[BUTTON_OK_E] );
			if( selected_menu == 1 )
			{
				//다시하기
				*pGameMode = READY_MODE;
				selected_menu = 0;
				return PLAY_MODE;
			}
			else if( selected_menu == 2 )
			{
				//메인메뉴로
				*pGameMode = MAIN_MODE;
				selected_menu = 0;
				FMOD_Channel_Stop( pSound->Channel[GAME_BGM] );
				FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[MAIN_BGM], 0, &pSound->Channel[MAIN_BGM] );
				return MAIN_MODE;
			}
		}
	}
	if( presskey == false )
	{
		presskey = true;
	}

	return -1;
}
void Ui::special_key( int key )
{
	if( presskey == false )
	{
		FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_OK_E], 0, &pSound->Channel[BUTTON_OK_E] );
		presskey = true;
	}
	else
	{
		if( ( *pGameMode == MAIN_MODE || *pGameMode == PAUSE_MODE ) && ( key_delay[1] == 0 ) )
		{
			key_delay[1] = 3;
			FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_MOVE_E], 0, &pSound->Channel[BUTTON_MOVE_E] );
			if( key == GLUT_KEY_RIGHT ) {
				if( selected_menu == 1 )
				{
					help = ( help + 1 ) % 3;
					if( help < 1 ) help = 1;
				}
				else
				{
					selected_menu = ( selected_menu + 1 ) % 3;
				}
			}
			else if( key == GLUT_KEY_LEFT ) {
				if( selected_menu == 1 )
				{
					--help;
					if( help < 1 ) help = 2;
				}
				else
				{
					--selected_menu;
					if( selected_menu < 0 ) selected_menu = 2;
				}
			}
			else if( key == GLUT_KEY_UP ) {
				help = 0;
				--selected_menu;
				if( selected_menu < 0 ) selected_menu = 2;
			}
			else if( key == GLUT_KEY_DOWN ) {
				help = 0;
				selected_menu = ( selected_menu + 1 ) % 3;
			}
		}
		else if( ( key_delay[1] == 0 ) && ending_screen == 3 )
		{
			key_delay[1] = 3;
			FMOD_System_PlaySound( pSound->System, FMOD_CHANNEL_FREE, pSound->Sound[BUTTON_MOVE_E], 0, &pSound->Channel[BUTTON_MOVE_E] );
			if( key == GLUT_KEY_RIGHT ) {
				++selected_menu;
				if( selected_menu > 2 ) selected_menu = 1;
			}
			else if( key == GLUT_KEY_LEFT ) {
				--selected_menu;
				if( selected_menu < 1 ) selected_menu = 2;
			}
			else if( key == GLUT_KEY_UP ) {
				--selected_menu;
				if( selected_menu < 1 ) selected_menu = 2;
			}
			else if( key == GLUT_KEY_DOWN ) {
				++selected_menu;
				if( selected_menu > 2 ) selected_menu = 1;
			}
		}
	}
}
void Ui::draw( Sheep* sheep )
{
	glPushMatrix();
	glOrtho( -canvas_size, canvas_size, -canvas_size, canvas_size, 1000, -1000 );

	// 메인 메뉴
	if( *pGameMode == MAIN_MODE )
	{
		glPushMatrix();
		glColor3f( 1, 1, 1 );
		int x = -200, y = -200, z = -500;
		int width = 400, height = 400;
		glEnable( GL_TEXTURE_2D );
		if( presskey == false ) { glBindTexture( GL_TEXTURE_2D, pTextures[MAIN_0] ); }
		else if( selected_menu == 0 ) { glBindTexture( GL_TEXTURE_2D, pTextures[MAIN_1] ); }
		else if( selected_menu == 1 ) { glBindTexture( GL_TEXTURE_2D, pTextures[MAIN_2] ); }
		else if( selected_menu == 2 ) { glBindTexture( GL_TEXTURE_2D, pTextures[MAIN_3] ); }
		glBegin( GL_QUADS );
		glTexCoord2i( 0, 0 );
		glVertex3f( x, y, z );
		glTexCoord2i( 1, 0 );
		glVertex3f( x + width, y, z );
		glTexCoord2i( 1, 1 );
		glVertex3f( x + width, y + height, z );
		glTexCoord2i( 0, 1 );
		glVertex3f( x, y + height, z );
		glEnd();
		glDisable( GL_TEXTURE_2D );
		glPopMatrix();

		//도움말
		if( help > 0 )
		{
			glPushMatrix();
			glColor3f( 1, 1, 1 );
			int x = -180, y = -180, z = -550;
			int width = 270, height = 270;
			glEnable( GL_TEXTURE_2D );
			if( help == 1 ) { glBindTexture( GL_TEXTURE_2D, pTextures[HELP_0] ); }
			else if( help == 2 ) { glBindTexture( GL_TEXTURE_2D, pTextures[HELP_1] ); }
			glBegin( GL_QUADS );
			glTexCoord2i( 0, 0 );
			glVertex3f( x, y, z );
			glTexCoord2i( 1, 0 );
			glVertex3f( x + width, y, z );
			glTexCoord2i( 1, 1 );
			glVertex3f( x + width, y + height, z );
			glTexCoord2i( 0, 1 );
			glVertex3f( x, y + height, z );
			glEnd();
			glDisable( GL_TEXTURE_2D );
			glPopMatrix();
		}
	}
	else
	{
		if( *pGameMode == ENDING_MODE )
		{
			// 엔딩 클리어화면
			static int x = 0, y = 0, z = -500;
			static int width = 0, height = 0;
			static int time;
			if( sheep->ending_finished == false ) {
				x = 0, y = 0, z = -500;
				width = 0, height = 0;
				time = 0;
			}
			else
			{
				glPushMatrix();
				glColor3f( 1, 1, 1 );
				if( x > -150 )
				{
					x -= FIXED_FRAME_TIME*0.25;
					y -= FIXED_FRAME_TIME*0.25;
					width += FIXED_FRAME_TIME * 0.5;
					height += FIXED_FRAME_TIME* 0.5;
				}
				else if( ending_screen != 3 )
				{
					time += FIXED_FRAME_TIME;
					if( time >= 2000 )
					{
						ending_screen = 3;
					}
				}
				glEnable( GL_TEXTURE_2D );
				if( ending_screen == 0 ) { glBindTexture( GL_TEXTURE_2D, pTextures[ENDING_0] ); }
				else if( ending_screen == 1 ) { glBindTexture( GL_TEXTURE_2D, pTextures[ENDING_1] ); }
				else if( ending_screen == 3 )
				{
					if( selected_menu == 1 ) { glBindTexture( GL_TEXTURE_2D, pTextures[ENDING_MENU_0] ); }
					else if( selected_menu == 2 ) { glBindTexture( GL_TEXTURE_2D, pTextures[ENDING_MENU_1] ); }
				}
				glBegin( GL_QUADS );
				glTexCoord2i( 0, 0 );
				glVertex3f( x, y, z );
				glTexCoord2i( 1, 0 );
				glVertex3f( x + width, y, z );
				glTexCoord2i( 1, 1 );
				glVertex3f( x + width, y + height, z );
				glTexCoord2i( 0, 1 );
				glVertex3f( x, y + height, z );
				glEnd();
				glDisable( GL_TEXTURE_2D );
				glPopMatrix();
			}
		}
		else if (*pGameMode == GAME_OVER)
		{
			ending_screen = 3;
			glPushMatrix();
			glColor3f(1, 1, 1);
			int x = -70, y = -130, z = -500;
			int width = 155, height = 250;
			glEnable(GL_TEXTURE_2D);
			if (selected_menu == 1) { glBindTexture(GL_TEXTURE_2D, pTextures[DEAD_0]); }
			else if (selected_menu == 2) { glBindTexture(GL_TEXTURE_2D, pTextures[DEAD_1]); }
			glBegin(GL_QUADS);
			glTexCoord2i(0, 0);
			glVertex3f(x, y, z);
			glTexCoord2i(1, 0);
			glVertex3f(x + width, y, z);
			glTexCoord2i(1, 1);
			glVertex3f(x + width, y + height, z);
			glTexCoord2i(0, 1);
			glVertex3f(x, y + height, z);
			glEnd();
			glDisable(GL_TEXTURE_2D);
			glPopMatrix();
		}

		else if( *pGameMode == PLAY_MODE )
		{
			// 목숨
			glPushMatrix();
			glColor3f( 0.9, 0.2, 0.2 ); //하트 색
			glTranslated( -170, 170, -500 ); //화면상의 하트 위치
			for( int i = 0; i < sheep->life; ++i )
			{
				glPushMatrix();
				glTranslated( i * 43, 0, 0 ); //하트사이 x간격 
				glRotated( 90, 1, 0, 0 );
				glScaled( heart_size, heart_size, heart_size ); // 하트크기
																//왼쪽부분
				glPushMatrix();
				glTranslated( -9, 0, 0 );
				glRotated( -45, 0, 1, 0 );
				glutSolidTorus( 18, 10, 20, 20 );
				glPopMatrix();
				//오른쪽부분
				glPushMatrix();
				glTranslated( 9, 0, 0 );
				glRotated( 45, 0, 1, 0 );
				glutSolidTorus( 18, 10, 20, 20 );
				glPopMatrix();
				glPopMatrix();
			}
			glPopMatrix();
		}

		// 메뉴 - PAUSE_MODE
		/*if (*pGameMode == PAUSE_MODE)
		{
		glPushMatrix();
		glColor3f(1, 1, 1);
		int x = -70, y = -130, z = -500;
		int width = 155, height = 250;
		glEnable(GL_TEXTURE_2D);
		if (selected_menu == 0) { glBindTexture(GL_TEXTURE_2D, pTextures[MENU_0]); }
		else if (selected_menu == 1) { glBindTexture(GL_TEXTURE_2D, pTextures[MENU_1]); }
		else if (selected_menu == 2) { glBindTexture(GL_TEXTURE_2D, pTextures[MENU_2]); }
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex3f(x, y, z);
		glTexCoord2i(1, 0);
		glVertex3f(x + width, y, z);
		glTexCoord2i(1, 1);
		glVertex3f(x + width, y + height, z);
		glTexCoord2i(0, 1);
		glVertex3f(x, y + height, z);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
		}
		사망메뉴
		else if (sheep->killed)
		{
		glPushMatrix();
		glColor3f(1, 1, 1);
		int x = -70, y = -130, z = -500;
		int width = 155, height = 250;
		glEnable(GL_TEXTURE_2D);
		if (selected_menu == 1) { glBindTexture(GL_TEXTURE_2D, pTextures[DEAD_0]); }
		else if (selected_menu == 2) { glBindTexture(GL_TEXTURE_2D, pTextures[DEAD_1]); }
		glBegin(GL_QUADS);
		glTexCoord2i(0, 0);
		glVertex3f(x, y, z);
		glTexCoord2i(1, 0);
		glVertex3f(x + width, y, z);
		glTexCoord2i(1, 1);
		glVertex3f(x + width, y + height, z);
		glTexCoord2i(0, 1);
		glVertex3f(x, y + height, z);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glPopMatrix();
		}*/


	}

	glPopMatrix();
}
void Ui::update( float frameTime )
{
	if( ( *pGameMode == GAME_OVER || *pGameMode == ENDING_MODE ) && selected_menu == 0 )
	{
		selected_menu = 1;
	}

	for( auto &k : key_delay )
	{
		if( k ) { --k; }
	}

	heart_size += heart_dir*0.00025 * frameTime;

	if( heart_size > 0.6 ) { heart_dir = -1; } // 하트 최대 크기
	else if( heart_size < 0.5 ) { heart_dir = +1; } // 하트 최소 크기
}

