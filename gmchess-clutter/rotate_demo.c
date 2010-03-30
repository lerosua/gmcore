/*
 * =====================================================================================
 *
 *       Filename:  rotate_demo.c
 *
 *    Description:  载入一个图标，然后旋转它
 *
 *        Version:  1.0
 *        Created:  2010年03月18日 11时51分44秒 CST
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

ClutterActor * image = NULL;
ClutterTimeline* timeline =NULL;
gint rotation_angle=0;
gint clicked=0;

static gboolean on_image_button_press(ClutterActor* actor, ClutterEvent* event, gpointer data)
{
	gfloat x=0;
	gfloat y=0;
	clutter_event_get_coords(event,&x ,&y);
	g_print("actor click at %f:%f\n",x,y);

	if(clicked){

		  clutter_timeline_start(timeline);

	}
	else{
		  clutter_timeline_stop(timeline);


	}
	clicked=1-clicked;

}


/** 时间线里每帧的旋转角度*/
void on_timeline_new_frame(ClutterTimeline* timeline, gint frame_num,gpointer data)
{
	rotation_angle+=1;
	if(rotation_angle>360)
		rotation_angle=0;

	clutter_actor_set_rotation(image, CLUTTER_Z_AXIS,rotation_angle,0,0,0);

}


int main(int argc,char *argv[])
{

  ClutterColor stage_color = { 0x00, 0x00, 0x00, 0xff };
  ClutterColor actor_color = { 0xff,0xff,0xff,0x98};

  /** 初始化clutter */
  clutter_init(&argc, &argv);

  /** 获取默认的场景stage */
  ClutterActor *stage = clutter_stage_get_default();
  /** 设置场景大小,注意场景也actor的一种，所以可以使用actor的api设置*/
  clutter_actor_set_size(stage,400,400);
  /** 设置场景背景*/
  clutter_stage_set_color(CLUTTER_STAGE(stage), &stage_color);



  /** 载入一个图像的actor */
  //ClutterActor * image = clutter_texture_new_from_file("demo.png",NULL);
  image = clutter_texture_new_from_file("demo.png",NULL);
  if(!image){
	  printf("load image error\n");
	  exit(-1);
  }

  /** 设置actor在场景中的位置*/
  clutter_actor_set_position(image, 100,100);
  /** 缩放图像，这里设置长宽各放大了两倍*/
  clutter_actor_set_scale(image,2.0,2.0);

  /** 设置图像旋转，以y轴旋转，角度20'c */
  clutter_actor_set_rotation(image, CLUTTER_Y_AXIS, 120,0,0,0);
  /** 把actor加入场景中*/
  clutter_container_add_actor(CLUTTER_CONTAINER(stage),image);
  clutter_actor_show(image);


  /** 打开actor的事件响应*/
  clutter_actor_set_reactive(image,TRUE);

  /** 连接actor的某事件*/
  g_signal_connect(image, "button-press-event", G_CALLBACK(on_image_button_press),NULL);

  /** 加入时间线*/
  //ClutterTimeline* timeline = clutter_timeline_new(5000);
  timeline = clutter_timeline_new(5000);
  g_signal_connect(timeline, "new-frame",G_CALLBACK(on_timeline_new_frame),NULL);
  clutter_timeline_set_loop(timeline,TRUE);
  //clutter_timeline_start(timeline);

  clutter_actor_show(stage);

  clutter_main();
  
  g_object_unref(timeline);
	printf("\n");
	return 0;
}

