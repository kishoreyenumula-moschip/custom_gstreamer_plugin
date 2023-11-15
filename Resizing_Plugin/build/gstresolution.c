/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2023 Kishore Yenumula <<user@hostname.org>>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-resolution
 *
 * FIXME:Describe resolution here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 videotestsrc ! video/x-raw,format=YV12 ! resolution ! autovideosink
 * ]|
 * </refsect2>
 */

/* The flow of the code is after inheriting the parent class and forming the structure it will  register to the Gstreamer framework
 * 	then base_init() function was invoked
 * 	after that _class_init() function was initialised. here the pointers for object class and element class are created
 * 	and pad templates are registered .
 * 	then _init() function will get invoked where pads are created based on the pad templated registered with framework
 * 	the above is basic steps done until the buffer reached the sink pad of our custom element*/




#include "myheader.h"

GST_DEBUG_CATEGORY_STATIC (gst_resolution_debug); 

#define GST_CAT_DEFAULT gst_resolution_debug

/*properties of the  plugin*/
enum
{
	PROP_0,
	PROP_HEIGHT,
	PROP_WIDTH,
	PROP_X,
	PROP_Y,
	PROP_TLX,
	PROP_TLY,
	PROP_TRX,
	PROP_RTY,
	PROP_BLX,
	PROP_BLY,
	PROP_BRX,
	PROP_BRY,
	PROP_SWIDTH,
	PROP_SHEIGHT,

};

/*
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
		GST_PAD_SINK,
		GST_PAD_ALWAYS,
		GST_STATIC_CAPS("ANY")
		);

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
		GST_PAD_SRC,
		GST_PAD_ALWAYS,
		GST_STATIC_CAPS ("ANY")
		);

#define gst_resolution_parent_class parent_class    

G_DEFINE_TYPE (GstResolution, gst_resolution, GST_TYPE_ELEMENT); 

GST_ELEMENT_REGISTER_DEFINE (resolution, "resolution", GST_RANK_NONE,
		GST_TYPE_RESOLUTION);	

static void gst_resolution_set_property (GObject * object,
		guint prop_id, const GValue * value, GParamSpec * pspec);

static void gst_resolution_get_property (GObject * object,
		guint prop_id, GValue * value, GParamSpec * pspec);

static gboolean gst_resolution_sink_event (GstPad * pad,
		GstObject * parent, GstEvent * event);

static GstFlowReturn gst_resolution_chain (GstPad * pad,
		GstObject * parent, GstBuffer * buf);


/* initialize the resolution's class */
static void gst_resolution_class_init (GstResolutionClass * klass)
{
//	g_print("class init function was called\n");

	GObjectClass *gobject_class;
	GstElementClass *gstelement_class;
	GstBaseTransformClass *trans_class;

	gobject_class = (GObjectClass *) klass;
	gstelement_class = (GstElementClass *) klass;

	gobject_class->set_property = gst_resolution_set_property;    
	gobject_class->get_property = gst_resolution_get_property;

	/*installing our own property width 
	 * this is the first method 
	 * here we take x,y coordinates and width and heigth to crop the video
	 * MAXRANGE is int range value 2^32.
	 */

	g_object_class_install_property(gobject_class,PROP_WIDTH,
			g_param_spec_int("width","Width","setting video frame crop width ",
				MINRANGE,MAXRANGE,DEF_WIDTH,G_PARAM_READWRITE|G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class,PROP_HEIGHT,
			g_param_spec_int("height","Height","setting video frame crop  height",
				MINRANGE,MAXRANGE,DEF_HEIGHT,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class,PROP_X,
			g_param_spec_int("x","Xcordinate","x cordinate value to crop the video ",
				MINRANGE,MAXRANGE,DEF_X,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));	

	g_object_class_install_property(gobject_class,PROP_Y,
			g_param_spec_int("y","Ycordinate","y cordinate value to crop the video ",
				MINRANGE,MAXRANGE,DEF_Y,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));	

	/* adding properties of four corner coordinates
	 * this is second method to crop the video 
	 * in this we take all four corner input means 8 coordinates.
	 * from these 8 coordinates width and height are calculated
	 * tlx and tly are the coordinates of corner from where video frame is cropped
	 */	

	g_object_class_install_property(gobject_class,PROP_TLX,
			g_param_spec_int("tlx","TopleftX","top left x coordinate",
				MINRANGE,MAXRANGE,TLX,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class,PROP_TLY,
			g_param_spec_int("tly","TopleftY","top left y coordinate",
				MINRANGE,MAXRANGE,TLY,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class,PROP_TRX,
			g_param_spec_int("trx","ToprightX","top right x cordinate",
				MINRANGE,MAXRANGE,TRX,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class,PROP_RTY,
			g_param_spec_int("rty","ToprightY","top right y coordinate",
				MINRANGE,MAXRANGE,RTY,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class,PROP_BLX,
			g_param_spec_int("blx","BottomleftX","bottom left x coordinate",
				MINRANGE,MAXRANGE,BLX,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class,PROP_BLY,
			g_param_spec_int("bly","BottomleftY","bottom left y coordinate",
				MINRANGE,MAXRANGE,BLY,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class,PROP_BRX,
			g_param_spec_int("brx","BottomrightX","bottom right x coordinate",
				MINRANGE,MAXRANGE,BRX,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	g_object_class_install_property(gobject_class,PROP_BRY,
			g_param_spec_int("bry","BottomrightY","bottom right y coordinate",
				MINRANGE,MAXRANGE,BRY,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

	/*these two properties swidth and sheight defines the desired width and height given by the user
	 * to which the cropped video need to  resize.
	 */

	g_object_class_install_property(gobject_class,PROP_SWIDTH,
			g_param_spec_int("swidth","Scalingwidth","new width to scale the cropped video minimum is 16 ",
				MINRANGE,MAXRANGE,SWIDTH,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
	g_object_class_install_property(gobject_class,PROP_SHEIGHT,
			g_param_spec_int("sheight","Scalingheight","new height to scale the cropped video minimum is 16",
				MINRANGE,MAXRANGE,SHEIGHT,G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));


	gst_element_class_set_details_simple (gstelement_class,
			"Resolution",
			"FIXME:Generic",
			"FIXME:Generic Template Element", "Kishorey <<user@hostname.org>>");

	gst_element_class_add_pad_template (gstelement_class,
			gst_static_pad_template_get (&src_factory));
	gst_element_class_add_pad_template (gstelement_class,
			gst_static_pad_template_get (&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad callback functions
 * initialize instance structure
 */
static void gst_resolution_init (GstResolution * filter)
{
	//GST_DEBUB_OBJECT(filter,"init funcn was called\n");
	filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
	gst_pad_set_event_function (filter->sinkpad,
			GST_DEBUG_FUNCPTR (gst_resolution_sink_event));
	gst_pad_set_chain_function (filter->sinkpad,
			GST_DEBUG_FUNCPTR (gst_resolution_chain));
	GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
	gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

	filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
	GST_PAD_SET_PROXY_CAPS (filter->srcpad);

	gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

//	g_print("pads are created\n");

	/*
	 * setting properties to their default vlaues
	 */


	filter->width = DEF_WIDTH;
	filter->height = DEF_HEIGHT;
	filter->x = DEF_X;
	filter->y = DEF_Y;

	filter->tlx= TLX;
	filter->tly= TLY;
	filter->tlx= TRX;
	filter->rty= RTY;
	filter->blx= BLX;
	filter->bly= BLY;
	filter->brx= BRX;
	filter->bry= BRY;
	filter->swidth = SWIDTH;
	filter->sheight = SHEIGHT;

	//g_print("all the properties are set to default values\n");
}

static void gst_resolution_set_property (GObject * object, guint prop_id,const GValue * value, GParamSpec * pspec)
{
	//g_print("set property function was called\n");

	GstResolution *filter = GST_RESOLUTION (object);
	switch (prop_id) {

		case PROP_HEIGHT:
			filter->height = g_value_get_int (value);
			if(filter->height<0)
			{
				g_warning("given height is invalid\n");
				exit(1);
			}
			g_print("height value  is set\n");
			sflag=1;
			break;
		case PROP_WIDTH:
			filter->width = g_value_get_int(value);
			if(filter->width<0)
			{
				g_warning("given width is invalid\n");
				exit(1);
			}
			sflag=1;
			g_print("the value is set to :%d\n",g_value_get_int(value));
			break;
		case PROP_X:
			filter->x = g_value_get_int(value);
			if(filter->x<0)
			{
				g_warning("given x coordinate is invalid\n");
				exit(1);
			}
			sflag=1;
			g_print("the value of coordinates are set\n");
			break;
		case PROP_Y:
			filter->y = g_value_get_int(value);
			if(filter->height<0)
			{
				g_warning("given y is invalid\n");
				exit(1);
			}
			sflag=1;
			g_print("the value of y coordinates are set\n");
			break;	
			/* coordinate properties */

		case PROP_TLX:
			filter->tlx = g_value_get_int(value);
			if(filter->tlx<0)
			{
				g_warning("given tlx is invalid\n");
				exit(1);
			}
			flag=1;
			g_print("the value of tlx is set to :%d\n",filter->tlx);
			break;	
		case PROP_TLY:
			filter->tly = g_value_get_int(value);
			if(filter->tly<0)
			{
				g_warning("given tly is invalid\n");
				exit(1);
			}
			flag=1;
			g_print("the value of tly is set to :%d\n",filter->tly);
			break;	
		case PROP_TRX:
			filter->trx = g_value_get_int(value);
			if(filter->trx<1)
			{
				g_warning("given trx is invalid\n");
				exit(1);
			}
			flag=1;
			g_print("the value of trx is set to :%d\n",filter->trx);
			break;	
		case PROP_RTY:
			filter->rty = g_value_get_int(value);
			if(filter->rty<0)
			{
				g_warning("given rty is invalid\n");
				exit(1);
			}
			flag=1;
			g_print("the value of rty is set to :%d\n",filter->rty);
			break;	
		case PROP_BLX:
			filter->blx = g_value_get_int(value);
			if(filter->blx<0)
			{
				g_warning("given blx is invalid\n");
				exit(1);
			}
			flag=1;
			g_print("the value of blx is set to :%d\n",filter->blx);
			break;	
		case PROP_BLY:
			filter->bly = g_value_get_int(value);
			if(filter->bly<1)
			{
				g_warning("given bly is invalid\n");
				exit(1);
			}
			flag=1;
			g_print("the value of bly is set to :%d\n",filter->bly);
			break;	
		case PROP_BRX:
			filter->brx = g_value_get_int(value);
			if(filter->brx<1)
			{
				g_warning("given brx is invalid\n");
				exit(1);
			}
			flag=1;
			g_print("the value of brx is set to :%d\n",filter->brx);
			break;	
		case PROP_BRY:
			filter->bry = g_value_get_int(value);
			if(filter->bry<1)
			{
				g_warning("given bry is invalid\n");
				exit(1);
			}
			flag=1;
			g_print("the value of bry is set to :%d\n",filter->bry);
			break;
		case PROP_SWIDTH:
			filter->swidth = g_value_get_int(value);
			if(filter->swidth<16)
			{
				g_warning("given swidth is invalid less than 16\n");
				exit(1);
			}
			g_print("new width value to scale :%d\n",filter->swidth);
		
			break;	
		case PROP_SHEIGHT:
			filter->sheight = g_value_get_int(value);
			if(filter->sheight<16)
			{
				g_warning("given sheight is invalid less than 16\n");
				exit(1);
			}
			g_print("new height value to scale:%d\n",filter->sheight);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

static void gst_resolution_get_property (GObject * object, guint prop_id,GValue * value, GParamSpec * pspec)
{

	GstResolution *filter = GST_RESOLUTION (object);

	switch (prop_id) {

		case PROP_HEIGHT:
			g_value_set_int (value, filter->height);
			break;
		case PROP_WIDTH:
			g_value_set_int(value,filter->width);
			break;
		case PROP_X:
			g_value_set_int(value,filter->x);
			break;
		case PROP_Y:
			g_value_set_int(value , filter->y);
			break;
		case PROP_TLX:
			g_value_set_int(value,filter->tlx);
			break;
		case PROP_TLY:
			g_value_set_int(value,filter->tly);
			break;
		case PROP_TRX:
			g_value_set_int(value,filter->tlx);
			break;
		case PROP_RTY:
			g_value_set_int(value,filter->rty);
			break;
		case PROP_BLX:
			g_value_set_int(value,filter->blx);
			break;
		case PROP_BLY:
			g_value_set_int(value,filter->bly);
			break;
		case PROP_BRX:
			g_value_set_int(value,filter->brx);
			break;
		case PROP_BRY:
			g_value_set_int(value,filter->bry);
			break;
		case PROP_SWIDTH:
			g_value_set_int(value,filter->swidth);
			break;
		case PROP_SHEIGHT:
			g_value_set_int(value,filter->sheight);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean gst_resolution_sink_event (GstPad * pad, GstObject * parent,GstEvent * event)
{
    //	g_print("caps event called\n");
	GstResolution *filter;
	gboolean ret;
	filter = GST_RESOLUTION (parent);


	switch (GST_EVENT_TYPE (event)) {
		case GST_EVENT_CAPS:
			{
				GstCaps *caps;
				gst_event_parse_caps (event, &caps);
				//	ret = gst_pad_event_default (pad, parent, event);
				break;
			}
		default:
			ret = gst_pad_event_default (pad, parent, event);
			break;
	}
	return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn gst_resolution_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{       
	
	
	/*here checking will happen, as this plugin contains two methods to crop the incoming video
         * we have initialised two flags which are set to ZERO initially, when properties of particular  method are set 
         * that particular flag is set to ONE.
         * if both flags are set to ONE indicates both methods are selected which is invalid
         * if both the properties are not by default it uses the first method with default values
         */

        if(sflag==1 && flag==1)
        {
                g_warning(" BOTH THE METHODS ARE SELECTED\n");
                g_print(" This  plugin follows two methods to crop the incoming video\n");
                g_print("\t1.By setting x,y,width and height properties\n");
                g_print("\t2.And providing all the four corner coordinates\n");
                g_print("both methods properties of resolution element are set, choose only one\n");
                return GST_FLOW_ERROR;
        }

	GstResolution *filter;
	filter = GST_RESOLUTION (parent);

	gint in_width,in_height;
	GstCaps *caps;
	gint size;
	gint new_buf_width;
	gint new_buf_height;

	/*
	 * these buffer_new , video_info_new,vframe_new
	 * are uesd for new buffer which is cropped from main buffer
	 */

	GstBuffer *buffer_new = NULL;
	GstVideoInfo video_info_new;
	GstVideoFrame vframe_new;

	g_print("------------------------------------------------------------------------\n");
	
	if(filter->swidth==0)
		filter->swidth=1920;
	if(filter->sheight==0)
		filter->sheight=1080;

	/*here incaps indicates incoming capabilites of th video
	 * which were retrived from pad
	 */

	GstCaps *incaps = gst_caps_copy(gst_pad_get_current_caps(pad));
	GstStructure *structure = gst_caps_get_structure(incaps, 0);

	/*
	 *in_width and in_height are incoming video frame width and height
	 */


	gst_structure_get(structure,"width",G_TYPE_INT,&in_width,NULL);

	gst_structure_get(structure,"height",G_TYPE_INT,&in_height,NULL);

	/* by default first method is selected 
	 * if width and heigth are given by the user to crop they will assign 
	 * else incoming video width and height are used which copies whole data
	 */
	
	if(filter->width==0)
		filter->width=in_width;
	if(filter->height==0)
		filter->height=in_height;

	new_buf_width = filter->width;
	new_buf_height = filter->height;

	g_print("incoming width and height are :%d %d\n",in_width,in_height);
	g_print("croping width and height are :%d %d\n",filter->width,filter->height);

	/* here a boundary is checked 
	 * suppose the user desired to crop the incoming video with certain width and height
	 * which are greater than incoming ones, it is an invalid operation
	 */


	if(filter->width >in_width || filter->height > in_height)
	{
		g_print("the incoming video width(or)height is smaller than default cropping width and height \n");
		g_print("please inspect the element for properties 'width' and 'height'\n");
		return GST_FLOW_ERROR;	
	}	

	gst_structure_set(structure , "width",G_TYPE_INT,filter->swidth,"height",G_TYPE_INT,filter->sheight,"format",G_TYPE_STRING,"YV12",NULL);


	/* based on the method selected by the user, the height and width are calculated and with that width and height 
	 * we are setting the caps for new buffer(cropped buffer), with these caps new buffer is created which
	 * is store the cropped data.
	 * flag=0 indicates normal method else second method
	 */
//	g_print("flag value %d\n",flag);
	if(flag !=1)
	{
		caps = gst_caps_new_simple("video/x-raw",
				"format", G_TYPE_STRING, "YV12",
				"width", G_TYPE_INT, new_buf_width,
				"height", G_TYPE_INT,new_buf_height,
				"bpp", G_TYPE_INT, 12,
				NULL);

		/* size of the buffer is calculated using width*height*(bits per pixle)*/

		size = new_buf_width * new_buf_height * (1.5);
	}
	else{
		/* all coordinates given by the user need to verify whether they form perfect shape or not*/

		if((filter->tly !=filter->rty) || (filter->tlx !=filter->blx) || (filter->trx!=filter->brx) || (filter->bly !=filter->bry))
		{
			g_print("Choosen properties values are not proper dimension to  crop a rectangle or square\n");
			g_print("try to give the approriate coordinated\n");
			return GST_FLOW_ERROR;
		}

		/* calculating width and height using coordinates*/

		gint width = (filter->trx - filter->tlx);
		gint height = (filter->bly - filter->tly);
		g_print("width and height from coordinates %d  %d\n",width,height);

		caps = gst_caps_new_simple("video/x-raw",
				"format", G_TYPE_STRING, "YV12",
				"width", G_TYPE_INT,width,
				"height", G_TYPE_INT,height,
				"bpp", G_TYPE_INT, 12,
				NULL);
		size = width * height * (1.5);
	}


	/************scaling caps********************/
	/*GstCaps *scaps = gst_caps_new_simple("video/x-raw",
	  "format", G_TYPE_STRING, "YV12",
	  "width", G_TYPE_INT, filter->swidth,
	  "height", G_TYPE_INT,filter->sheight,
	  "bpp", G_TYPE_INT, 12,
	  NULL);*/

	/*******************************************************************************
	 *  creating and mapping the newly created buffer *****************************/

	buffer_new= gst_buffer_new_allocate(NULL, size, NULL );

	if (!gst_buffer_make_writable(buffer_new)) {
		g_print("Failed to make the buffer writable\n");
		gst_buffer_unref(buffer_new);
		return GST_FLOW_ERROR;
	}

	/* getting the video info to map the video frame*/

	if(!gst_video_info_from_caps(&video_info_new, caps))
	{
		g_print("fail to get videoinfo for cropping buffer \n");
		return GST_FLOW_ERROR;
	}
	/* mapping video frame */
	if(!gst_video_frame_map(&vframe_new, &video_info_new, buffer_new, GST_MAP_WRITE))
	{
		g_warning("give standard resolution to crop\n");
		g_print("fail to map videoframe for croping buffer\n");
		return GST_FLOW_ERROR;
	}

	/* as we know the incoming video is of YV12 format there will be three planes 
	 * getting video data, stride and pixel stride data using videoframe
	 */

	guint8 *y_pixels_new = GST_VIDEO_FRAME_PLANE_DATA(&vframe_new, 0); // Y plane
	guint8 *u_pixels_new = GST_VIDEO_FRAME_PLANE_DATA(&vframe_new, 1); // U plane 
	guint8 *v_pixels_new = GST_VIDEO_FRAME_PLANE_DATA(&vframe_new, 2);  //V plane

	guint y_stride_new = GST_VIDEO_FRAME_PLANE_STRIDE(&vframe_new, 0); // Y plane stride
	guint u_stride_new = GST_VIDEO_FRAME_PLANE_STRIDE(&vframe_new, 1); //U plane
	guint v_stride_new = GST_VIDEO_FRAME_PLANE_STRIDE(&vframe_new, 2);// V plane stride

	guint pixel_stride_new = GST_VIDEO_FRAME_COMP_PSTRIDE(&vframe_new, 0); 
	guint pixel_stride_new_u = GST_VIDEO_FRAME_COMP_PSTRIDE(&vframe_new, 1);
	guint pixel_stride_new_v = GST_VIDEO_FRAME_COMP_PSTRIDE(&vframe_new, 2);

	/**********************************************************************************
	 *************** mapping the orginal buffer to read the data from it **************/

	GstVideoInfo video_info_org;
	GstVideoFrame vframe_org;

	if(!gst_video_info_from_caps(&video_info_org, gst_pad_get_current_caps(pad)))
	{
		g_print("fail to get videoinfo of original buf\n");
		return GST_FLOW_ERROR;
	}

	if(!gst_video_frame_map(&vframe_org, &video_info_org, buf, GST_MAP_READ))
	{
		g_print("fail to map videoframe of original buf\n");
		return GST_FLOW_ERROR;
	}

	guint8 *y_pixels_org = GST_VIDEO_FRAME_PLANE_DATA(&vframe_org, 0); // Y plane
	guint8 *u_pixels_org = GST_VIDEO_FRAME_PLANE_DATA(&vframe_org, 1); // U plane 
	guint8 *v_pixels_org = GST_VIDEO_FRAME_PLANE_DATA(&vframe_org, 2);  //V plane

	guint y_stride_org = GST_VIDEO_FRAME_PLANE_STRIDE(&vframe_org, 0); // Y plane stride
	guint u_stride_org = GST_VIDEO_FRAME_PLANE_STRIDE(&vframe_org, 1); //U plane
	guint v_stride_org = GST_VIDEO_FRAME_PLANE_STRIDE(&vframe_org, 2);// V plane stride

	guint pixel_stride_org = GST_VIDEO_FRAME_COMP_PSTRIDE(&vframe_org, 0); 
	guint pixel_stride_org_u = GST_VIDEO_FRAME_COMP_PSTRIDE(&vframe_org, 1);
	guint pixel_stride_org_v = GST_VIDEO_FRAME_COMP_PSTRIDE(&vframe_org, 2);

	gint width_org = GST_VIDEO_FRAME_WIDTH(&vframe_org);
	gint height_org = GST_VIDEO_FRAME_HEIGHT(&vframe_org);

	g_print("flag vlaue is :%d\n",flag);

	/*
	 * based on the method selected the widht height and coordinates(top left) are  calculate to copy the data
	 * from original buffer to newly created buffer
	 */

	gint widtH , heighT,x,y,bottom,right;

	if(flag !=1)
	{
		g_print("selected first method\n");
		widtH = filter->width;
		heighT = filter->height;

		x = filter->x;
		y = filter->y;
		g_print("x and y coordinates are :%d %d\n",x,y);
		g_print("width and height from coordinates %d  %d\n",widtH,heighT);

		// Calculate the coordinates of the top-left and bottom-right corners
		bottom = y + heighT;
		right = x + widtH;
		g_print("rect bot:%d    rect right:%d\n",bottom,right);

		if((x+widtH)>width_org || (y+heighT)>height_org)
		{
			g_print("WARNING OUT OF BOUNDARY\n");
			return GST_FLOW_ERROR;
		}
	}	
	else
	{
		g_print("second method is selected\n");

		widtH = (filter->trx - filter->tlx);
		heighT = (filter->bly - filter->tly);
		x = filter->tlx ;
		y = filter->tly;

		g_print("x and y coordinates are :%d %d\n",x,y);
		g_print("width and height from coordinates %d  %d\n",widtH,heighT);
		bottom = y+heighT;
		right  = x+widtH;
		g_print("rect bot:%d    rect right:%d\n",bottom,right);

		if((x+widtH)>width_org || (y+heighT)>height_org)
		{
			g_print("WARNING OUT OF BOUNDARY\n");
			return GST_FLOW_ERROR;
		}
	}

	/* this part of code will copy the data from the input buffer to new buffer
	 * here h1,w1 will start from zero indicates new buffer which accepts data
	 * and h2,w2 are coordinates of the original buffer from that starts sharing 
	 * data from that point
	 */ 


	guint8 *y_pixel,*u_pixel,*v_pixel,*p_y_pixel,*p_u_pixel,*p_v_pixel;

	gint h1=0,w1=0,h2=0,w2=0;

	for(h1=0,h2=y;h1<heighT && h2< bottom ;h1++,h2++)
	{
		for(w1=0,w2=x;w1<widtH && w2 < right;w1++,w2++)
		{
			/* calculating offeset of each pixel in three planes for new buffer*/
			y_pixel = y_pixels_new + h1 * y_stride_new + w1 * pixel_stride_new;
			u_pixel = u_pixels_new + h1 / 2 * u_stride_new + (w1 / 2) * pixel_stride_new_u;
			v_pixel = v_pixels_new + h1 / 2 * v_stride_new + (w1 / 2) * pixel_stride_new_v;

			/* calculating offset of each pixel in three planes of original buffer*/
			p_y_pixel = y_pixels_org + h2 * y_stride_org + w2 * pixel_stride_org;
			p_u_pixel = u_pixels_org + h2 / 2 * u_stride_org + (w2 / 2) * pixel_stride_org_u;
			p_v_pixel = v_pixels_org + h2 / 2 * v_stride_org + (w2 / 2) * pixel_stride_org_v;

			/* data transfering */
			if (w2 >= x && w2 < right && h2 >= y && h2 < bottom) 
			{
				y_pixel[0]=p_y_pixel[0];
				u_pixel[0]=p_u_pixel[0];
				v_pixel[0]=p_v_pixel[0];
			}

		}

	}

	/* upto now croping is done 
	 * the below part of code will resizes the cropped data into particular resolution
	 * depending on the property values of swidth and sheight set by the user.
	 * filter->swidth , filter->sheight are the width and height to resize.
	 * with that width and height another new buffer is created.
	 * this buffer is pushed on to the source pad
	 */

	GstVideoInfo video_info_scale;
	GstVideoFrame vframe_scale;

	gsize s_size = (filter->swidth)*(filter->sheight)*(1.5);

	/* creating scaling buffer*/
	GstBuffer *mybuf= gst_buffer_new_allocate(NULL, s_size, NULL );

	/*
	 * in starting of chain function incoming video cpas are copied into incaps, while we change the width and height
	 * of that caps usign structure to resize width and height. 
	 * By this meta data is copied from the incoming buffer where there will be no issue of timestamps, no need of videoconvert
	 * After resize push the resize buffer and this caps
	 */

	//gst_structure_set(structure , "width",G_TYPE_INT,filter->swidth,"height",G_TYPE_INT,filter->sheight,"format",G_TYPE_STRING,"YV12",NULL);
	g_print("scaling buffer width and height are :%d %d\n",filter->swidth,filter->sheight);

	if (!gst_buffer_make_writable(mybuf)) {
		g_print("Failed to make the buffer writable\n");
		gst_buffer_unref(mybuf);
		return GST_FLOW_ERROR;
	}

	if(!gst_video_info_from_caps(&video_info_scale, incaps))
	{
		g_print("fail to get videoinfo for scaling buf\n");
		return GST_FLOW_ERROR;
	}

	if(!gst_video_frame_map(&vframe_scale, &video_info_scale, mybuf, GST_MAP_WRITE))
	{
		g_print("failed to map videoframe for scaling buf\n");
		return GST_FLOW_ERROR;
	}

	guint8 *y_pixels_scale = GST_VIDEO_FRAME_PLANE_DATA(&vframe_scale, 0); // Y plane
	guint8 *u_pixels_scale = GST_VIDEO_FRAME_PLANE_DATA(&vframe_scale, 1); // U plane (interleaved)
	guint8 *v_pixels_scale = GST_VIDEO_FRAME_PLANE_DATA(&vframe_scale, 2);  //V plane

	guint y_stride_scale = GST_VIDEO_FRAME_PLANE_STRIDE(&vframe_scale, 0); // Y plane stride
	guint u_stride_scale = GST_VIDEO_FRAME_PLANE_STRIDE(&vframe_scale, 1); //U plane
	guint v_stride_scale = GST_VIDEO_FRAME_PLANE_STRIDE(&vframe_scale, 2);// V plane stride

	guint pixel_stride_scale = GST_VIDEO_FRAME_COMP_PSTRIDE(&vframe_scale, 0); 
	guint pixel_stride_scale_u = GST_VIDEO_FRAME_COMP_PSTRIDE(&vframe_scale, 1);
	guint pixel_stride_scale_v = GST_VIDEO_FRAME_COMP_PSTRIDE(&vframe_scale, 2);

	/**
	 * using  nearest neighbour algorithm to resize
	 */

	for(gint h=0 ; h<filter->sheight ; h++)
	{
		for(gint w=0 ; w<filter->swidth ; w++)
		{
			//this is nearest neighbour algorithm
			gint new_x = (w*widtH)/filter->swidth;
			gint new_y = (h*heighT)/filter->sheight;



			guint8 *y_pixel = y_pixels_new + new_y * y_stride_new + new_x * pixel_stride_new;
			guint8 *u_pixel = u_pixels_new + (new_y/2)  * u_stride_new + (new_x/2) * pixel_stride_new_u;
			guint8 *v_pixel = v_pixels_new + (new_y/2)  * v_stride_new + (new_x/2)  * pixel_stride_new_v;


			guint8 *y_pixel_o = y_pixels_scale + h * y_stride_scale + w * pixel_stride_scale;
			guint8 *u_pixel_o = u_pixels_scale + (h/2)  * u_stride_scale + (w/2) * pixel_stride_scale_u;
			guint8 *v_pixel_o = v_pixels_scale + (h/2)  * v_stride_scale + (w/2) * pixel_stride_scale_v;

			y_pixel_o[0] = y_pixel[0];
			u_pixel_o[0] = u_pixel[0];
			v_pixel_o[0] = v_pixel[0];

		}
	}

	/** if we want use predefined algorithm uncomment the below and comment 
	 * nearest neighbour algorithm
	 */

	/*	GstSample *samp = gst_sample_new (buffer_new,caps,NULL,NULL);
		if(!samp)
		{
		g_print("failed to create the sample\n");
		}
		GstSample *sample = gst_video_convert_sample(samp,scaps,GST_CLOCK_TIME_NONE ,NULL);
		if(!sample)
		{
		g_print("failed to convert the sample\n");
		}

		GstBuffer *mybuf = gst_sample_get_buffer(sample);*/

	gst_pad_set_caps(filter->srcpad , incaps);
	
	/* this is used when using sample function*/
	//gst_pad_set_caps(filter->srcpad , caps);

	gst_video_frame_unmap(&vframe_new);	
	gst_video_frame_unmap(&vframe_scale);	
	

	/*this is used when using sample function*/
	//return gst_pad_push (filter->srcpad, buffer_new);
	return gst_pad_push (filter->srcpad, mybuf);
}


/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */

static gboolean resolution_init (GstPlugin * resolution)
{
	/* debug category for filtering log messages				
	 *
	 * exchange the string 'Template resolution' with your description
	 */
	GST_DEBUG_CATEGORY_INIT (gst_resolution_debug, "resolution",
			0, "Template resolution");
	//it is the entry point for the plugin .
	return GST_ELEMENT_REGISTER (resolution, resolution);
}

/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstresolution"
#endif

/* gstreamer looks for this structure to register resolutions
 *
 * exchange the string 'Template resolution' with your resolution description
 */
GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
		GST_VERSION_MINOR,
		resolution,
		"resolution",
		resolution_init,
		PACKAGE_VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)
