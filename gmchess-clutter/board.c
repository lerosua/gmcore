/*
 * =====================================================================================
 *
 *       Filename:  board.c
 *
 *    Description:  clutter界面的棋盘
 *
 *        Version:  1.0
 *        Created:  2010年03月22日 16时48分37秒 CST
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lerosua (), lerosua@gmail.com
 *        Company:  Cyclone
 *
 * =====================================================================================
 */

#include <clutter/clutter.h>
#include <stdlib.h>
#include <stdio.h>
#define IMGAGE_DIR "./wood/"

enum {  
	RED_KING=0,RED_ADVISOR,RED_BISHOP,RED_KNIGHT,RED_ROOK,RED_CANNON,RED_PAWN,
	BLACK_KING,BLACK_ADVISOR,BLACK_BISHOP,BLACK_KNIGHT,BLACK_ROOK,BLACK_CANNON,BLACK_PAWN,
	RED_KING_DIE,BLACK_KING_DIE,
	NULL_CHESSMAN,SELECTED_CHESSMAN,
};

//ClutterActor* chessman_images[18];
ClutterActor* chessman_images[2];
ClutterActor * _group=NULL ;
ClutterTimeline* timeline =NULL;
int _select=0;
gint rotation_angle=0;

void load_chess()
{
#if 1
   chessman_images[0]= clutter_texture_new_from_file(IMGAGE_DIR"red_king.png",NULL);
  clutter_container_add_actor(CLUTTER_CONTAINER(_group),chessman_images[0]);
  clutter_actor_show(chessman_images[0]);
  clutter_actor_set_position(chessman_images[0],235,8);

  chessman_images[1]= clutter_texture_new_from_file(IMGAGE_DIR"black_king.png",NULL);
  clutter_container_add_actor(CLUTTER_CONTAINER(_group),chessman_images[1]);
  clutter_actor_show(chessman_images[1]);
  clutter_actor_set_position(chessman_images[1],235,68);

#else

		chessman_images[BLACK_ADVISOR] = clutter_texture_new_from_file(IMGAGE_DIR"black_advisor.png",NULL);
		chessman_images[BLACK_BISHOP] = clutter_texture_new_from_file(IMGAGE_DIR"black_bishop.png",NULL);
		chessman_images[BLACK_CANNON] = clutter_texture_new_from_file(IMGAGE_DIR"black_cannon.png",NULL);
		chessman_images[BLACK_KING] = clutter_texture_new_from_file(IMGAGE_DIR"black_king.png",NULL);
		chessman_images[BLACK_KING_DIE] = clutter_texture_new_from_file(IMGAGE_DIR"black_king_die.png",NULL);
		chessman_images[BLACK_KNIGHT] = clutter_texture_new_from_file(IMGAGE_DIR"black_knight.png",NULL);
		chessman_images[BLACK_PAWN] = clutter_texture_new_from_file(IMGAGE_DIR"black_pawn.png",NULL);
		chessman_images[BLACK_ROOK] = clutter_texture_new_from_file(IMGAGE_DIR"black_rook.png",NULL);
		chessman_images[RED_ADVISOR] = clutter_texture_new_from_file(IMGAGE_DIR"red_advisor.png",NULL);
		chessman_images[RED_BISHOP] = clutter_texture_new_from_file(IMGAGE_DIR"red_bishop.png",NULL);
		chessman_images[RED_CANNON] = clutter_texture_new_from_file(IMGAGE_DIR"red_cannon.png",NULL);
		chessman_images[RED_KING] = clutter_texture_new_from_file(IMGAGE_DIR"red_king.png",NULL);
		chessman_images[RED_KING_DIE] = clutter_texture_new_from_file(IMGAGE_DIR"red_king_die.png",NULL);
		chessman_images[RED_KNIGHT] = clutter_texture_new_from_file(IMGAGE_DIR"red_knight.png",NULL);
		chessman_images[RED_PAWN] = clutter_texture_new_from_file(IMGAGE_DIR"red_pawn.png",NULL);
		chessman_images[RED_ROOK] = clutter_texture_new_from_file(IMGAGE_DIR"red_rook.png",NULL);
		chessman_images[SELECTED_CHESSMAN] = clutter_texture_new_from_file(IMGAGE_DIR"select.png",NULL);
		chessman_images[NULL_CHESSMAN] = clutter_texture_new_from_file(IMGAGE_DIR"null.png",NULL);


		for(int i=0;i<8;i++)
			for(int j=0;j<9;j++)
			{

				clutter_actor_set_position(chessman_images[i+j],i*10+5,j*10+5);

			}
		for(int i=0;i<18;i++)
		{
			//clutter_actor_set_reactive(chessman_images[i],TRUE);
			clutter_container_add_actor(CLUTTER_CONTAINER(_group), chessman_images[i]);
			clutter_actor_show(chessman_images[i]);


		}
#endif

}

gboolean on_rking_button_press(ClutterActor* actor,ClutterEvent* event, gpointer data)
{

	gfloat x=0;
	gfloat y=0;
	clutter_event_get_coords(event,&x ,&y);

	if(_select){
		clutter_timeline_start(timeline);
		_select=1-_select;
	}
	else
	{
		clutter_timeline_stop(timeline);
		_select=1-_select;
	}
}

void on_timeline_new_frame(ClutterTimeline* timeline, gint frame_num,gpointer data)
{
	rotation_angle+=1;
	if(rotation_angle>360)
		rotation_angle=0;

	clutter_actor_set_rotation(CLUTTER_ACTOR(data), CLUTTER_Y_AXIS,rotation_angle,0,25,0);

}

int main(int argc,char *argv[])
{

  ClutterColor stage_color = { 0x00, 0x00, 0x00, 0xff };

  clutter_init(&argc, &argv);

  ClutterActor *stage = clutter_stage_get_default ();
  clutter_actor_set_size (stage, 521, 577);
  clutter_stage_set_color (CLUTTER_STAGE (stage), &stage_color);

  //ClutterActor * _group = clutter_group_new();
   _group = clutter_group_new();
  clutter_actor_set_position(_group, 0,0);
  clutter_container_add_actor(CLUTTER_CONTAINER(stage),_group);
  clutter_actor_show(_group);

  /** 加载棋盘图片*/
  ClutterActor* _board = clutter_texture_new_from_file("./wood/wood.png",NULL);

  clutter_actor_set_position(_board,0,0);
  //clutter_actor_set_rotation(_board, CLUTTER_Y_AXIS, 20,0,0,0);
  //clutter_actor_set_rotation(_board, CLUTTER_X_AXIS, 20,0,0,0);
  //clutter_actor_set_rotation(_board, CLUTTER_Z_AXIS, 20,0,0,0);


  //clutter_container_add_actor(CLUTTER_CONTAINER(stage),_board);
  clutter_container_add_actor(CLUTTER_CONTAINER(_group),_board);
  clutter_actor_show(_board);



#if 0
  load_chess();
#else 
  /** 加载棋子*/
  ClutterActor* _rking = clutter_texture_new_from_file("./wood/red_king.png",NULL);
  clutter_actor_set_position(_rking,235,8);
  clutter_container_add_actor(CLUTTER_CONTAINER(_group),_rking);
  clutter_actor_show(_rking);

  clutter_actor_set_reactive(_rking,TRUE);
  g_signal_connect(_rking, "button-press-event",G_CALLBACK(on_rking_button_press),NULL);

  timeline=clutter_timeline_new(3000);
  //g_signal_connect(timeline,"new-frame",G_CALLBACK(on_timeline_new_frame),_rking);
  clutter_timeline_set_loop(timeline,TRUE);

  ClutterAlpha* alpha_ = clutter_alpha_new_full(timeline,
         CLUTTER_EASE_IN_OUT_QUAD);//CLUTTER_EASE_IN_SINE);

  ClutterBehaviour* behaviour_ = clutter_behaviour_rotate_new(alpha_, 
		  CLUTTER_Y_AXIS,
		CLUTTER_ROTATE_CW,
		0,
		360);
  clutter_behaviour_rotate_set_center(CLUTTER_BEHAVIOUR_ROTATE(behaviour_), clutter_actor_get_width(_rking)/2,0,0);
  clutter_behaviour_apply(behaviour_, _rking);

  clutter_actor_set_rotation(_group, CLUTTER_X_AXIS, 40,2,600,0);
  //clutter_actor_set_rotation(_group, CLUTTER_Y_AXIS, 40,221,200,0);


#endif

  clutter_actor_show(stage);
  clutter_main();

  g_object_unref(timeline);

	printf("\n");
	return 0;
}
