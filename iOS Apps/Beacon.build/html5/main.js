
//Change this to true for a stretchy canvas!
//
var RESIZEABLE_CANVAS=false;

//Start us up!
//
window.onload=function( e ){

	if( RESIZEABLE_CANVAS ){
		window.onresize=function( e ){
			var canvas=document.getElementById( "GameCanvas" );

			//This vs window.innerWidth, which apparently doesn't account for scrollbar?
			var width=document.body.clientWidth;
			
			//This vs document.body.clientHeight, which does weird things - document seems to 'grow'...perhaps canvas resize pushing page down?
			var height=window.innerHeight;			

			canvas.width=width;
			canvas.height=height;
		}
		window.onresize( null );
	}
	
	game_canvas=document.getElementById( "GameCanvas" );
	
	game_console=document.getElementById( "GameConsole" );

	try{
	
		bbInit();
		bbMain();
		
		if( game_runner!=null ) game_runner();
		
	}catch( err ){
	
		alertError( err );
	}
}

var game_canvas;
var game_console;
var game_runner;

//${CONFIG_BEGIN}
CFG_BINARY_FILES="*.bin|*.dat";
CFG_CD="";
CFG_CONFIG="debug";
CFG_HOST="winnt";
CFG_IMAGE_FILES="*.png|*.jpg";
CFG_LANG="js";
CFG_MODPATH=".;J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps;C:/Program Files (x86)/Monkey/modules";
CFG_MOJO_AUTO_SUSPEND_ENABLED="0";
CFG_MUSIC_FILES="*.wav|*.ogg|*.mp3|*.m4a";
CFG_OPENGL_GLES20_ENABLED="0";
CFG_SAFEMODE="0";
CFG_SOUND_FILES="*.wav|*.ogg|*.mp3|*.m4a";
CFG_TARGET="html5";
CFG_TEXT_FILES="*.txt|*.xml|*.json";
CFG_TRANSDIR="";
//${CONFIG_END}

//${METADATA_BEGIN}
var META_DATA="[Arial10B_P_1.png];type=image/png;width=512;height=512;\n[Arial10_P_1.png];type=image/png;width=512;height=512;\n[Arial12_P_1.png];type=image/png;width=512;height=512;\n[Arial20B_P_1.png];type=image/png;width=512;height=512;\n[Arial22_P_1.png];type=image/png;width=512;height=512;\n[GUI_mac.png];type=image/png;width=550;height=250;\n[Shadow.png];type=image/png;width=50;height=50;\n[mojo_font.png];type=image/png;width=864;height=13;\n";
//${METADATA_END}

function getMetaData( path,key ){

	if( path.toLowerCase().indexOf("monkey://data/")!=0 ) return "";
	path=path.slice(14);

	var i=META_DATA.indexOf( "["+path+"]" );
	if( i==-1 ) return "";
	i+=path.length+2;

	var e=META_DATA.indexOf( "\n",i );
	if( e==-1 ) e=META_DATA.length;

	i=META_DATA.indexOf( ";"+key+"=",i )
	if( i==-1 || i>=e ) return "";
	i+=key.length+2;

	e=META_DATA.indexOf( ";",i );
	if( e==-1 ) return "";

	return META_DATA.slice( i,e );
}

function fixDataPath( path ){
	if( path.toLowerCase().indexOf("monkey://data/")==0 ) return "data/"+path.slice(14);
	return path;
}

function openXMLHttpRequest( req,path,async ){

	path=fixDataPath( path );
	
	var xhr=new XMLHttpRequest;
	xhr.open( req,path,async );
	return xhr;
}

function loadArrayBuffer( path ){

	var xhr=openXMLHttpRequest( "GET",path,false );

	if( xhr.overrideMimeType ) xhr.overrideMimeType( "text/plain; charset=x-user-defined" );

	xhr.send( null );
	
	if( xhr.status!=200 && xhr.status!=0 ) return null;

	var r=xhr.responseText;
	var buf=new ArrayBuffer( r.length );

	for( var i=0;i<r.length;++i ){
		this.dataView.setInt8( i,r.charCodeAt(i) );
	}
	return buf;
}

function loadString( path ){
	path=fixDataPath( path );
	var xhr=new XMLHttpRequest();
	xhr.open( "GET",path,false );
	xhr.send( null );
	if( (xhr.status==200) || (xhr.status==0) ) return xhr.responseText;
	return "";
}

function loadImage( path,onloadfun ){

	var ty=getMetaData( path,"type" );
	if( ty.indexOf( "image/" )!=0 ) return null;

	var image=new Image();
	
	image.meta_width=parseInt( getMetaData( path,"width" ) );
	image.meta_height=parseInt( getMetaData( path,"height" ) );
	image.onload=onloadfun;
	image.src="data/"+path.slice(14);
	
	return image;
}

function loadAudio( path ){

	path=fixDataPath( path );
	
	var audio=new Audio( path );
	return audio;
}

//${TRANSCODE_BEGIN}

// Javascript Monkey runtime.
//
// Placed into the public domain 24/02/2011.
// No warranty implied; use at your own risk.

//***** JavaScript Runtime *****

var D2R=0.017453292519943295;
var R2D=57.29577951308232;

var err_info="";
var err_stack=[];

var dbg_index=0;

function push_err(){
	err_stack.push( err_info );
}

function pop_err(){
	err_info=err_stack.pop();
}

function stackTrace(){
	if( !err_info.length ) return "";
	var str=err_info+"\n";
	for( var i=err_stack.length-1;i>0;--i ){
		str+=err_stack[i]+"\n";
	}
	return str;
}

function print( str ){
	if( game_console ){
		game_console.value+=str+"\n";
		game_console.scrollTop = game_console.scrollHeight - game_console.clientHeight;
	}
	if( window.console!=undefined ){
		window.console.log( str );
	}
	return 0;
}

function alertError( err ){
	if( typeof(err)=="string" && err=="" ) return;
	alert( "Monkey Runtime Error : "+err.toString()+"\n\n"+stackTrace() );
}

function error( err ){
	throw err;
}

function debugLog( str ){
	print( str );
}

function debugStop(){
	error( "STOP" );
}

function dbg_object( obj ){
	if( obj ) return obj;
	error( "Null object access" );
}

function dbg_array( arr,index ){
	if( index<0 || index>=arr.length ) error( "Array index out of range" );
	dbg_index=index;
	return arr;
}

function new_bool_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]=false;
	return arr;
}

function new_number_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]=0;
	return arr;
}

function new_string_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]='';
	return arr;
}

function new_array_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]=[];
	return arr;
}

function new_object_array( len ){
	var arr=Array( len );
	for( var i=0;i<len;++i ) arr[i]=null;
	return arr;
}

function resize_bool_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]=false;
	return arr;
}

function resize_number_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]=0;
	return arr;
}

function resize_string_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]="";
	return arr;
}

function resize_array_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]=[];
	return arr;
}

function resize_object_array( arr,len ){
	var i=arr.length;
	arr=arr.slice(0,len);
	if( len<=i ) return arr;
	arr.length=len;
	while( i<len ) arr[i++]=null;
	return arr;
}

function string_compare( lhs,rhs ){
	var n=Math.min( lhs.length,rhs.length ),i,t;
	for( i=0;i<n;++i ){
		t=lhs.charCodeAt(i)-rhs.charCodeAt(i);
		if( t ) return t;
	}
	return lhs.length-rhs.length;
}

function string_replace( str,find,rep ){	//no unregex replace all?!?
	var i=0;
	for(;;){
		i=str.indexOf( find,i );
		if( i==-1 ) return str;
		str=str.substring( 0,i )+rep+str.substring( i+find.length );
		i+=rep.length;
	}
}

function string_trim( str ){
	var i=0,i2=str.length;
	while( i<i2 && str.charCodeAt(i)<=32 ) i+=1;
	while( i2>i && str.charCodeAt(i2-1)<=32 ) i2-=1;
	return str.slice( i,i2 );
}

function string_startswith( str,substr ){
	return substr.length<=str.length && str.slice(0,substr.length)==substr;
}

function string_endswith( str,substr ){
	return substr.length<=str.length && str.slice(str.length-substr.length,str.length)==substr;
}

function string_tochars( str ){
	var arr=new Array( str.length );
	for( var i=0;i<str.length;++i ) arr[i]=str.charCodeAt(i);
	return arr;
}

function string_fromchars( chars ){
	var str="",i;
	for( i=0;i<chars.length;++i ){
		str+=String.fromCharCode( chars[i] );
	}
	return str;
}

function object_downcast( obj,clas ){
	if( obj instanceof clas ) return obj;
	return null;
}

function object_implements( obj,iface ){
	if( obj && obj.implments && obj.implments[iface] ) return obj;
	return null;
}

function extend_class( clas ){
	var tmp=function(){};
	tmp.prototype=clas.prototype;
	return new tmp;
}

function ThrowableObject(){
}

ThrowableObject.prototype.toString=function(){ 
	return "Uncaught Monkey Exception"; 
}

// Note: Firefox doesn't support DataView, so we have to kludge...
//
// This means pokes/peeks must be naturally aligned, but data has to be in WebGL anyway so that's OK for now.
//
function BBDataBuffer(){
	this.arrayBuffer=null;
	this.dataView=null;
	this.length=0;
}

BBDataBuffer.prototype._Init=function( buffer ){
	this.arrayBuffer=buffer;
	this.dataView=new DataView( buffer );
	this.length=buffer.byteLength;
}

BBDataBuffer.prototype._New=function( length ){
	if( this.arrayBuffer ) return false;
	
	var buf=new ArrayBuffer( length );
	if( !buf ) return false;
	
	this._Init( buf );
	return true;
}

BBDataBuffer.prototype._Load=function( path ){
	if( this.arrayBuffer ) return false;
	
	var buf=loadArrayBuffer( path );
	if( !buf ) return false;
	
	_Init( buf );
	return true;
}

BBDataBuffer.prototype.Length=function(){
	return this.length;
}

BBDataBuffer.prototype.Discard=function(){
	if( this.arrayBuffer ){
		this.arrayBuffer=null;
		this.dataView=null;
		this.length=0;
	}
}

BBDataBuffer.prototype.PokeByte=function( addr,value ){
	this.dataView.setInt8( addr,value );
}

BBDataBuffer.prototype.PokeShort=function( addr,value ){
	this.dataView.setInt16( addr,value );	
}

BBDataBuffer.prototype.PokeInt=function( addr,value ){
	this.dataView.setInt32( addr,value );	
}

BBDataBuffer.prototype.PokeFloat=function( addr,value ){
	this.dataView.setFloat32( addr,value );	
}

BBDataBuffer.prototype.PeekByte=function( addr ){
	return this.dataView.getInt8( addr );
}

BBDataBuffer.prototype.PeekShort=function( addr ){
	return this.dataView.getInt16( addr );
}

BBDataBuffer.prototype.PeekInt=function( addr ){
	return this.dataView.getInt32( addr );
}

BBDataBuffer.prototype.PeekFloat=function( addr ){
	return this.dataView.getFloat32( addr );
}

// HTML5 mojo runtime.
//
// Copyright 2011 Mark Sibly, all rights reserved.
// No warranty implied; use at your own risk.

var gl=null;	//global WebGL context - a bit rude!

KEY_LMB=1;
KEY_RMB=2;
KEY_MMB=3;
KEY_TOUCH0=0x180;

function eatEvent( e ){
	if( e.stopPropagation ){
		e.stopPropagation();
		e.preventDefault();
	}else{
		e.cancelBubble=true;
		e.returnValue=false;
	}
}

function keyToChar( key ){
	switch( key ){
	case 8:
	case 9:
	case 13:
	case 27:
	case 32:
		return key;
	case 33:
	case 34:
	case 35:
	case 36:
	case 37:
	case 38:
	case 39:
	case 40:
	case 45:
		return key | 0x10000;
	case 46:
		return 127;
	}
	return 0;
}

//***** gxtkApp class *****

function gxtkApp(){

	if( CFG_OPENGL_GLES20_ENABLED=="1" ){
		this.gl=game_canvas.getContext( "webgl" );
		if( !this.gl ) this.gl=game_canvas.getContext( "experimental-webgl" );
	}else{
		this.gl=null;
	}

	this.graphics=new gxtkGraphics( this,game_canvas );
	this.input=new gxtkInput( this );
	this.audio=new gxtkAudio( this );

	this.loading=0;
	this.maxloading=0;

	this.updateRate=0;
	this.startMillis=(new Date).getTime();
	
	this.dead=false;
	this.suspended=false;
	
	var app=this;
	var canvas=game_canvas;
	
	function gxtkMain(){
	
		var input=app.input;
	
		canvas.onkeydown=function( e ){
			input.OnKeyDown( e.keyCode );
			var chr=keyToChar( e.keyCode );
			if( chr ) input.PutChar( chr );
			if( e.keyCode<48 || (e.keyCode>111 && e.keyCode<122) ) eatEvent( e );
		}

		canvas.onkeyup=function( e ){
			input.OnKeyUp( e.keyCode );
		}

		canvas.onkeypress=function( e ){
			if( e.charCode ){
				input.PutChar( e.charCode );
			}else if( e.which ){
				input.PutChar( e.which );
			}
		}

		canvas.onmousedown=function( e ){
			switch( e.button ){
			case 0:input.OnKeyDown( KEY_LMB );break;
			case 1:input.OnKeyDown( KEY_MMB );break;
			case 2:input.OnKeyDown( KEY_RMB );break;
			}
			eatEvent( e );
		}
		
		canvas.onmouseup=function( e ){
			switch( e.button ){
			case 0:input.OnKeyUp( KEY_LMB );break;
			case 1:input.OnKeyUp( KEY_MMB );break;
			case 2:input.OnKeyUp( KEY_RMB );break;
			}
			eatEvent( e );
		}
		
		canvas.onmouseout=function( e ){
			input.OnKeyUp( KEY_LMB );
			input.OnKeyUp( KEY_MMB );
			input.OnKeyUp( KEY_RMB );
			eatEvent( e );
		}

		canvas.onmousemove=function( e ){
			var x=e.clientX+document.body.scrollLeft;
			var y=e.clientY+document.body.scrollTop;
			var c=canvas;
			while( c ){
				x-=c.offsetLeft;
				y-=c.offsetTop;
				c=c.offsetParent;
			}
			input.OnMouseMove( x,y );
			eatEvent( e );
		}

		canvas.onfocus=function( e ){
			if( CFG_MOJO_AUTO_SUSPEND_ENABLED=="1" ){
				app.InvokeOnResume();
			}
		}
		
		canvas.onblur=function( e ){
			if( CFG_MOJO_AUTO_SUSPEND_ENABLED=="1" ){
				app.InvokeOnSuspend();
			}
		}
		
		canvas.ontouchstart=function( e ){
			for( var i=0;i<e.changedTouches.length;++i ){
				var touch=e.changedTouches[i];
				var x=touch.pageX;
				var y=touch.pageY;
				var c=canvas;
				while( c ){
					x-=c.offsetLeft;
					y-=c.offsetTop;
					c=c.offsetParent;
				}
				input.OnTouchStart( touch.identifier,x,y );
			}
			eatEvent( e );
		}
		
		canvas.ontouchmove=function( e ){
			for( var i=0;i<e.changedTouches.length;++i ){
				var touch=e.changedTouches[i];
				var x=touch.pageX;
				var y=touch.pageY;
				var c=canvas;
				while( c ){
					x-=c.offsetLeft;
					y-=c.offsetTop;
					c=c.offsetParent;
				}
				input.OnTouchMove( touch.identifier,x,y );
			}
			eatEvent( e );
		}
		
		canvas.ontouchend=function( e ){
			for( var i=0;i<e.changedTouches.length;++i ){
				input.OnTouchEnd( e.changedTouches[i].identifier );
			}
			eatEvent( e );
		}
		
		window.ondevicemotion=function( e ){
			var tx=e.accelerationIncludingGravity.x/9.81;
			var ty=e.accelerationIncludingGravity.y/9.81;
			var tz=e.accelerationIncludingGravity.z/9.81;
			var x,y;
			switch( window.orientation ){
			case   0:x=+tx;y=-ty;break;
			case 180:x=-tx;y=+ty;break;
			case  90:x=-ty;y=-tx;break;
			case -90:x=+ty;y=+tx;break;
			}
			input.OnDeviceMotion( x,y,tz );
			eatEvent( e );
		}

		canvas.focus();

		app.InvokeOnCreate();
		app.InvokeOnRender();
	}

	game_runner=gxtkMain;
}

var timerSeq=0;

gxtkApp.prototype.SetFrameRate=function( fps ){

	var seq=++timerSeq;
	
	if( !fps ) return;
	
	var app=this;
	var updatePeriod=1000.0/fps;
	var nextUpdate=(new Date).getTime()+updatePeriod;
	
	function timeElapsed(){
		if( seq!=timerSeq ) return;

		var time;		
		var updates=0;

		for(;;){
			nextUpdate+=updatePeriod;

			app.InvokeOnUpdate();
			if( seq!=timerSeq ) return;
			
			if( nextUpdate>(new Date).getTime() ) break;
			
			if( ++updates==7 ){
				nextUpdate=(new Date).getTime();
				break;
			}
		}
		app.InvokeOnRender();
		if( seq!=timerSeq ) return;
			
		var delay=nextUpdate-(new Date).getTime();
		setTimeout( timeElapsed,delay>0 ? delay : 0 );
	}
	
	setTimeout( timeElapsed,updatePeriod );
}

gxtkApp.prototype.IncLoading=function(){
	++this.loading;
	if( this.loading>this.maxloading ) this.maxloading=this.loading;
	if( this.loading==1 ) this.SetFrameRate( 0 );
}

gxtkApp.prototype.DecLoading=function(){
	--this.loading;
	if( this.loading!=0 ) return;
	this.maxloading=0;
	this.SetFrameRate( this.updateRate );
}

gxtkApp.prototype.GetMetaData=function( path,key ){
	return getMetaData( path,key );
}

gxtkApp.prototype.Die=function( err ){
	this.dead=true;
	this.audio.OnSuspend();
	alertError( err );
}

gxtkApp.prototype.InvokeOnCreate=function(){
	if( this.dead ) return;
	
	try{
		gl=this.gl;
		this.OnCreate();
		gl=null;
	}catch( ex ){
		this.Die( ex );
	}
}

gxtkApp.prototype.InvokeOnUpdate=function(){
	if( this.dead || this.suspended || !this.updateRate || this.loading ) return;
	
	try{
		gl=this.gl;
		this.input.BeginUpdate();
		this.OnUpdate();		
		this.input.EndUpdate();
		gl=null;
	}catch( ex ){
		this.Die( ex );
	}
}

gxtkApp.prototype.InvokeOnSuspend=function(){
	if( this.dead || this.suspended ) return;
	
	try{
		gl=this.gl;
		this.suspended=true;
		this.OnSuspend();
		this.audio.OnSuspend();
		gl=null;
	}catch( ex ){
		this.Die( ex );
	}
}

gxtkApp.prototype.InvokeOnResume=function(){
	if( this.dead || !this.suspended ) return;
	
	try{
		gl=this.gl;
		this.audio.OnResume();
		this.OnResume();
		this.suspended=false;
		gl=null;
	}catch( ex ){
		this.Die( ex );
	}
}

gxtkApp.prototype.InvokeOnRender=function(){
	if( this.dead || this.suspended ) return;
	
	try{
		gl=this.gl;
		this.graphics.BeginRender();
		if( this.loading ){
			this.OnLoading();
		}else{
			this.OnRender();
		}
		this.graphics.EndRender();
		gl=null;
	}catch( ex ){
		this.Die( ex );
	}
}

//***** GXTK API *****

gxtkApp.prototype.GraphicsDevice=function(){
	return this.graphics;
}

gxtkApp.prototype.InputDevice=function(){
	return this.input;
}

gxtkApp.prototype.AudioDevice=function(){
	return this.audio;
}

gxtkApp.prototype.AppTitle=function(){
	return document.URL;
}

gxtkApp.prototype.LoadState=function(){
	var state=localStorage.getItem( ".mojostate@"+document.URL );
	if( state ) return state;
	return "";
}

gxtkApp.prototype.SaveState=function( state ){
	localStorage.setItem( ".mojostate@"+document.URL,state );
}

gxtkApp.prototype.LoadString=function( path ){
	return loadString( path );
}

gxtkApp.prototype.SetUpdateRate=function( fps ){
	this.updateRate=fps;
	
	if( !this.loading ) this.SetFrameRate( fps );
}

gxtkApp.prototype.MilliSecs=function(){
	return ((new Date).getTime()-this.startMillis)|0;
}

gxtkApp.prototype.Loading=function(){
	return this.loading;
}

gxtkApp.prototype.OnCreate=function(){
}

gxtkApp.prototype.OnUpdate=function(){
}

gxtkApp.prototype.OnSuspend=function(){
}

gxtkApp.prototype.OnResume=function(){
}

gxtkApp.prototype.OnRender=function(){
}

gxtkApp.prototype.OnLoading=function(){
}

//***** gxtkGraphics class *****

function gxtkGraphics( app,canvas ){
	this.app=app;
	this.canvas=canvas;
	this.gc=canvas.getContext( '2d' );
	this.tmpCanvas=null;
	this.r=255;
	this.b=255;
	this.g=255;
	this.white=true;
	this.color="rgb(255,255,255)"
	this.alpha=1;
	this.blend="source-over";
	this.ix=1;this.iy=0;
	this.jx=0;this.jy=1;
	this.tx=0;this.ty=0;
	this.tformed=false;
	this.scissorX=0;
	this.scissorY=0;
	this.scissorWidth=0;
	this.scissorHeight=0;
	this.clipped=false;
}

gxtkGraphics.prototype.BeginRender=function(){
	if( this.gc ) this.gc.save();
}

gxtkGraphics.prototype.EndRender=function(){
	if( this.gc ) this.gc.restore();
}

gxtkGraphics.prototype.Mode=function(){
	if( this.gc ) return 1;
	return 0;
}

gxtkGraphics.prototype.Width=function(){
	return this.canvas.width;
}

gxtkGraphics.prototype.Height=function(){
	return this.canvas.height;
}

gxtkGraphics.prototype.LoadSurface=function( path ){
	var app=this.app;
	
	function onloadfun(){
		app.DecLoading();
	}

	app.IncLoading();

	var image=loadImage( path,onloadfun );
	if( image ) return new gxtkSurface( image,this );

	app.DecLoading();
	return null;
}

gxtkGraphics.prototype.CreateSurface=function( width,height ){

	var canvas=document.createElement( 'canvas' );
	
	canvas.width=width;
	canvas.height=height;
	canvas.meta_width=width;
	canvas.meta_height=height;
	canvas.complete=true;
	
	var surface=new gxtkSurface( canvas,this );
	
	surface.gc=canvas.getContext( '2d' );
	
	return surface;
}

gxtkGraphics.prototype.SetAlpha=function( alpha ){
	this.alpha=alpha;
	this.gc.globalAlpha=alpha;
}

gxtkGraphics.prototype.SetColor=function( r,g,b ){
	this.r=r;
	this.g=g;
	this.b=b;
	this.white=(r==255 && g==255 && b==255);
	this.color="rgb("+(r|0)+","+(g|0)+","+(b|0)+")";
	this.gc.fillStyle=this.color;
	this.gc.strokeStyle=this.color;
}

gxtkGraphics.prototype.SetBlend=function( blend ){
	switch( blend ){
	case 1:
		this.blend="lighter";
		break;
	default:
		this.blend="source-over";
	}
	this.gc.globalCompositeOperation=this.blend;
}

gxtkGraphics.prototype.SetScissor=function( x,y,w,h ){
	this.scissorX=x;
	this.scissorY=y;
	this.scissorWidth=w;
	this.scissorHeight=h;
	this.clipped=(x!=0 || y!=0 || w!=this.canvas.width || h!=this.canvas.height);
	this.gc.restore();
	this.gc.save();
	if( this.clipped ){
		this.gc.beginPath();
		this.gc.rect( x,y,w,h );
		this.gc.clip();
		this.gc.closePath();
	}
	this.gc.fillStyle=this.color;
	this.gc.strokeStyle=this.color;
	if( this.tformed ) this.gc.setTransform( this.ix,this.iy,this.jx,this.jy,this.tx,this.ty );
}

gxtkGraphics.prototype.SetMatrix=function( ix,iy,jx,jy,tx,ty ){
	this.ix=ix;this.iy=iy;
	this.jx=jx;this.jy=jy;
	this.tx=tx;this.ty=ty;
	this.gc.setTransform( ix,iy,jx,jy,tx,ty );
	this.tformed=(ix!=1 || iy!=0 || jx!=0 || jy!=1 || tx!=0 || ty!=0);
}

gxtkGraphics.prototype.Cls=function( r,g,b ){
	if( this.tformed ) this.gc.setTransform( 1,0,0,1,0,0 );
	this.gc.fillStyle="rgb("+(r|0)+","+(g|0)+","+(b|0)+")";
	this.gc.globalAlpha=1;
	this.gc.globalCompositeOperation="source-over";
	this.gc.fillRect( 0,0,this.canvas.width,this.canvas.height );
	this.gc.fillStyle=this.color;
	this.gc.globalAlpha=this.alpha;
	this.gc.globalCompositeOperation=this.blend;
	if( this.tformed ) this.gc.setTransform( this.ix,this.iy,this.jx,this.jy,this.tx,this.ty );
}

gxtkGraphics.prototype.DrawPoint=function( x,y ){
	if( this.tformed ){
		var px=x;
		x=px * this.ix + y * this.jx + this.tx;
		y=px * this.iy + y * this.jy + this.ty;
		this.gc.setTransform( 1,0,0,1,0,0 );
		this.gc.fillRect( x,y,1,1 );
		this.gc.setTransform( this.ix,this.iy,this.jx,this.jy,this.tx,this.ty );
	}else{
		this.gc.fillRect( x,y,1,1 );
	}
}

gxtkGraphics.prototype.DrawRect=function( x,y,w,h ){
	if( w<0 ){ x+=w;w=-w; }
	if( h<0 ){ y+=h;h=-h; }
	if( w<=0 || h<=0 ) return;
	//
	this.gc.fillRect( x,y,w,h );
}

gxtkGraphics.prototype.DrawLine=function( x1,y1,x2,y2 ){
	if( this.tformed ){
		var x1_t=x1 * this.ix + y1 * this.jx + this.tx;
		var y1_t=x1 * this.iy + y1 * this.jy + this.ty;
		var x2_t=x2 * this.ix + y2 * this.jx + this.tx;
		var y2_t=x2 * this.iy + y2 * this.jy + this.ty;
		this.gc.setTransform( 1,0,0,1,0,0 );
	  	this.gc.beginPath();
	  	this.gc.moveTo( x1_t,y1_t );
	  	this.gc.lineTo( x2_t,y2_t );
	  	this.gc.stroke();
	  	this.gc.closePath();
		this.gc.setTransform( this.ix,this.iy,this.jx,this.jy,this.tx,this.ty );
	}else{
	  	this.gc.beginPath();
	  	this.gc.moveTo( x1,y1 );
	  	this.gc.lineTo( x2,y2 );
	  	this.gc.stroke();
	  	this.gc.closePath();
	}
}

gxtkGraphics.prototype.DrawOval=function( x,y,w,h ){
	if( w<0 ){ x+=w;w=-w; }
	if( h<0 ){ y+=h;h=-h; }
	if( w<=0 || h<=0 ) return;
	//
  	var w2=w/2,h2=h/2;
	this.gc.save();
	this.gc.translate( x+w2,y+h2 );
	this.gc.scale( w2,h2 );
  	this.gc.beginPath();
	this.gc.arc( 0,0,1,0,Math.PI*2,false );
	this.gc.fill();
  	this.gc.closePath();
	this.gc.restore();
}

gxtkGraphics.prototype.DrawPoly=function( verts ){
	if( verts.length<6 ) return;
	this.gc.beginPath();
	this.gc.moveTo( verts[0],verts[1] );
	for( var i=2;i<verts.length;i+=2 ){
		this.gc.lineTo( verts[i],verts[i+1] );
	}
	this.gc.fill();
	this.gc.closePath();
}

gxtkGraphics.prototype.DrawSurface=function( surface,x,y ){
	if( !surface.image.complete ) return;
	
	if( this.white ){
		this.gc.drawImage( surface.image,x,y );
		return;
	}
	
	this.DrawImageTinted( surface.image,x,y,0,0,surface.swidth,surface.sheight );
}

gxtkGraphics.prototype.DrawSurface2=function( surface,x,y,srcx,srcy,srcw,srch ){
	if( !surface.image.complete ) return;

	if( srcw<0 ){ srcx+=srcw;srcw=-srcw; }
	if( srch<0 ){ srcy+=srch;srch=-srch; }
	if( srcw<=0 || srch<=0 ) return;

	if( this.white ){
		this.gc.drawImage( surface.image,srcx,srcy,srcw,srch,x,y,srcw,srch );
		return;
	}
	
	this.DrawImageTinted( surface.image,x,y,srcx,srcy,srcw,srch  );
}

gxtkGraphics.prototype.DrawImageTinted=function( image,dx,dy,sx,sy,sw,sh ){

	if( !this.tmpCanvas ){
		this.tmpCanvas=document.createElement( "canvas" );
	}

	if( sw>this.tmpCanvas.width || sh>this.tmpCanvas.height ){
		this.tmpCanvas.width=Math.max( sw,this.tmpCanvas.width );
		this.tmpCanvas.height=Math.max( sh,this.tmpCanvas.height );
	}
	
	var tmpGC=this.tmpCanvas.getContext( "2d" );
	tmpGC.globalCompositeOperation="copy";
	
	tmpGC.drawImage( image,sx,sy,sw,sh,0,0,sw,sh );
	
	var imgData=tmpGC.getImageData( 0,0,sw,sh );
	
	var p=imgData.data,sz=sw*sh*4,i;
	
	for( i=0;i<sz;i+=4 ){
		p[i]=p[i]*this.r/255;
		p[i+1]=p[i+1]*this.g/255;
		p[i+2]=p[i+2]*this.b/255;
	}
	
	tmpGC.putImageData( imgData,0,0 );
	
	this.gc.drawImage( this.tmpCanvas,0,0,sw,sh,dx,dy,sw,sh );
}

gxtkGraphics.prototype.ReadPixels=function( pixels,x,y,width,height,offset,pitch ){

	var imgData=this.gc.getImageData( x,y,width,height );
	
	var p=imgData.data,i=0,j=offset,px,py;
	
	for( py=0;py<height;++py ){
		for( px=0;px<width;++px ){
			pixels[j++]=(p[i+3]<<24)|(p[i]<<16)|(p[i+1]<<8)|p[i+2];
			i+=4;
		}
		j+=pitch-width;
	}
}

gxtkGraphics.prototype.WritePixels2=function( surface,pixels,x,y,width,height,offset,pitch ){

	if( !surface.gc ){
		if( !surface.image.complete ) return;
		var canvas=document.createElement( "canvas" );
		canvas.width=surface.swidth;
		canvas.height=surface.sheight;
		surface.gc=canvas.getContext( "2d" );
		surface.gc.globalCompositeOperation="copy";
		surface.gc.drawImage( surface.image,0,0 );
		surface.image=canvas;
	}

	var imgData=surface.gc.createImageData( width,height );

	var p=imgData.data,i=0,j=offset,px,py,argb;
	
	for( py=0;py<height;++py ){
		for( px=0;px<width;++px ){
			argb=pixels[j++];
			p[i]=(argb>>16) & 0xff;
			p[i+1]=(argb>>8) & 0xff;
			p[i+2]=argb & 0xff;
			p[i+3]=(argb>>24) & 0xff;
			i+=4;
		}
		j+=pitch-width;
	}
	
	surface.gc.putImageData( imgData,x,y );
}

//***** gxtkSurface class *****

function gxtkSurface( image,graphics ){
	this.image=image;
	this.graphics=graphics;
	this.swidth=image.meta_width;
	this.sheight=image.meta_height;
}

//***** GXTK API *****

gxtkSurface.prototype.Discard=function(){
	if( this.image ){
		this.image=null;
	}
}

gxtkSurface.prototype.Width=function(){
	return this.swidth;
}

gxtkSurface.prototype.Height=function(){
	return this.sheight;
}

gxtkSurface.prototype.Loaded=function(){
	return this.image.complete;
}

gxtkSurface.prototype.OnUnsafeLoadComplete=function(){
	return true;
}

//***** Class gxtkInput *****

function gxtkInput( app ){
	this.app=app;
	this.keyStates=new Array( 512 );
	this.charQueue=new Array( 32 );
	this.charPut=0;
	this.charGet=0;
	this.mouseX=0;
	this.mouseY=0;
	this.joyX=0;
	this.joyY=0;
	this.joyZ=0;
	this.touchIds=new Array( 32 );
	this.touchXs=new Array( 32 );
	this.touchYs=new Array( 32 );
	this.accelX=0;
	this.accelY=0;
	this.accelZ=0;
	
	var i;
	
	for( i=0;i<512;++i ){
		this.keyStates[i]=0;
	}
	
	for( i=0;i<32;++i ){
		this.touchIds[i]=-1;
		this.touchXs[i]=0;
		this.touchYs[i]=0;
	}
}

gxtkInput.prototype.BeginUpdate=function(){
}

gxtkInput.prototype.EndUpdate=function(){
	for( var i=0;i<512;++i ){
		this.keyStates[i]&=0x100;
	}
	this.charGet=0;
	this.charPut=0;
}

gxtkInput.prototype.OnKeyDown=function( key ){
	if( (this.keyStates[key]&0x100)==0 ){
		this.keyStates[key]|=0x100;
		++this.keyStates[key];
		//
		if( key==KEY_LMB ){
			this.keyStates[KEY_TOUCH0]|=0x100;
			++this.keyStates[KEY_TOUCH0];
		}else if( key==KEY_TOUCH0 ){
			this.keyStates[KEY_LMB]|=0x100;
			++this.keyStates[KEY_LMB];
		}
		//
	}
}

gxtkInput.prototype.OnKeyUp=function( key ){
	this.keyStates[key]&=0xff;
	//
	if( key==KEY_LMB ){
		this.keyStates[KEY_TOUCH0]&=0xff;
	}else if( key==KEY_TOUCH0 ){
		this.keyStates[KEY_LMB]&=0xff;
	}
	//
}

gxtkInput.prototype.PutChar=function( chr ){
	if( this.charPut-this.charGet<32 ){
		this.charQueue[this.charPut & 31]=chr;
		this.charPut+=1;
	}
}

gxtkInput.prototype.OnMouseMove=function( x,y ){
	this.mouseX=x;
	this.mouseY=y;
	this.touchXs[0]=x;
	this.touchYs[0]=y;
}

gxtkInput.prototype.OnTouchStart=function( id,x,y ){
	for( var i=0;i<32;++i ){
		if( this.touchIds[i]==-1 ){
			this.touchIds[i]=id;
			this.touchXs[i]=x;
			this.touchYs[i]=y;
			this.OnKeyDown( KEY_TOUCH0+i );
			return;
		} 
	}
}

gxtkInput.prototype.OnTouchMove=function( id,x,y ){
	for( var i=0;i<32;++i ){
		if( this.touchIds[i]==id ){
			this.touchXs[i]=x;
			this.touchYs[i]=y;
			if( i==0 ){
				this.mouseX=x;
				this.mouseY=y;
			}
			return;
		}
	}
}

gxtkInput.prototype.OnTouchEnd=function( id ){
	for( var i=0;i<32;++i ){
		if( this.touchIds[i]==id ){
			this.touchIds[i]=-1;
			this.OnKeyUp( KEY_TOUCH0+i );
			return;
		}
	}
}

gxtkInput.prototype.OnDeviceMotion=function( x,y,z ){
	this.accelX=x;
	this.accelY=y;
	this.accelZ=z;
}

//***** GXTK API *****

gxtkInput.prototype.SetKeyboardEnabled=function( enabled ){
	return 0;
}

gxtkInput.prototype.KeyDown=function( key ){
	if( key>0 && key<512 ){
		return this.keyStates[key] >> 8;
	}
	return 0;
}

gxtkInput.prototype.KeyHit=function( key ){
	if( key>0 && key<512 ){
		return this.keyStates[key] & 0xff;
	}
	return 0;
}

gxtkInput.prototype.GetChar=function(){
	if( this.charPut!=this.charGet ){
		var chr=this.charQueue[this.charGet & 31];
		this.charGet+=1;
		return chr;
	}
	return 0;
}

gxtkInput.prototype.MouseX=function(){
	return this.mouseX;
}

gxtkInput.prototype.MouseY=function(){
	return this.mouseY;
}

gxtkInput.prototype.JoyX=function( index ){
	return this.joyX;
}

gxtkInput.prototype.JoyY=function( index ){
	return this.joyY;
}

gxtkInput.prototype.JoyZ=function( index ){
	return this.joyZ;
}

gxtkInput.prototype.TouchX=function( index ){
	return this.touchXs[index];
}

gxtkInput.prototype.TouchY=function( index ){
	return this.touchYs[index];
}

gxtkInput.prototype.AccelX=function(){
	return this.accelX;
}

gxtkInput.prototype.AccelY=function(){
	return this.accelY;
}

gxtkInput.prototype.AccelZ=function(){
	return this.accelZ;
}


//***** gxtkChannel class *****
function gxtkChannel(){
	this.sample=null;
	this.audio=null;
	this.volume=1;
	this.pan=0;
	this.rate=1;
	this.flags=0;
	this.state=0;
}

//***** gxtkAudio class *****
function gxtkAudio( app ){
	this.app=app;
	this.okay=typeof(Audio)!="undefined";
	this.nextchan=0;
	this.music=null;
	this.channels=new Array(33);
	for( var i=0;i<33;++i ){
		this.channels[i]=new gxtkChannel();
	}
}

gxtkAudio.prototype.OnSuspend=function(){
	var i;
	for( i=0;i<33;++i ){
		var chan=this.channels[i];
		if( chan.state==1 ) chan.audio.pause();
	}
}

gxtkAudio.prototype.OnResume=function(){
	var i;
	for( i=0;i<33;++i ){
		var chan=this.channels[i];
		if( chan.state==1 ) chan.audio.play();
	}
}

gxtkAudio.prototype.LoadSample=function( path ){
	var audio=loadAudio( path );
	if( !audio ) return null;
	return new gxtkSample( audio );
}

gxtkAudio.prototype.PlaySample=function( sample,channel,flags ){
	if( !this.okay ) return;

	var chan=this.channels[channel];

	if( chan.state!=0 ){
		chan.audio.pause();
		chan.state=0;
	}
	
	for( var i=0;i<33;++i ){
		var chan2=this.channels[i];
		if( chan2.state==1 && chan2.audio.ended && !chan2.audio.loop ) chan.state=0;
		if( chan2.state==0 && chan2.sample ){
			chan2.sample.FreeAudio( chan2.audio );
			chan2.sample=null;
			chan2.audio=null;
		}
	}

	var audio=sample.AllocAudio();
	if( !audio ) return;
	
	audio.loop=(flags&1)!=0;
	audio.volume=chan.volume;
	audio.play();

	chan.sample=sample;
	chan.audio=audio;
	chan.flags=flags;
	chan.state=1;
}

gxtkAudio.prototype.StopChannel=function( channel ){
	var chan=this.channels[channel];
	
	if( chan.state!=0 ){
		chan.audio.pause();
		chan.state=0;
	}
}

gxtkAudio.prototype.PauseChannel=function( channel ){
	var chan=this.channels[channel];
	
	if( chan.state==1 ){
		if( chan.audio.ended && !chan.audio.loop ){
			chan.state=0;
		}else{
			chan.audio.pause();
			chan.state=2;
		}
	}
}

gxtkAudio.prototype.ResumeChannel=function( channel ){
	var chan=this.channels[channel];
	
	if( chan.state==2 ){
		chan.audio.play();
		chan.state=1;
	}
}

gxtkAudio.prototype.ChannelState=function( channel ){
	var chan=this.channels[channel];
	if( chan.state==1 && chan.audio.ended && !chan.audio.loop ) chan.state=0;
	return chan.state;
}

gxtkAudio.prototype.SetVolume=function( channel,volume ){
	var chan=this.channels[channel];
	if( chan.state!=0 ) chan.audio.volume=volume;
	chan.volume=volume;
}

gxtkAudio.prototype.SetPan=function( channel,pan ){
	var chan=this.channels[channel];
	chan.pan=pan;
}

gxtkAudio.prototype.SetRate=function( channel,rate ){
	var chan=this.channels[channel];
	chan.rate=rate;
}

gxtkAudio.prototype.PlayMusic=function( path,flags ){
	this.StopMusic();
	
	this.music=this.LoadSample( path );
	if( !this.music ) return;
	
	this.PlaySample( this.music,32,flags );
}

gxtkAudio.prototype.StopMusic=function(){
	this.StopChannel( 32 );

	if( this.music ){
		this.music.Discard();
		this.music=null;
	}
}

gxtkAudio.prototype.PauseMusic=function(){
	this.PauseChannel( 32 );
}

gxtkAudio.prototype.ResumeMusic=function(){
	this.ResumeChannel( 32 );
}

gxtkAudio.prototype.MusicState=function(){
	return this.ChannelState( 32 );
}

gxtkAudio.prototype.SetMusicVolume=function( volume ){
	this.SetVolume( 32,volume );
}

//***** gxtkSample class *****

function gxtkSample( audio ){
	this.audio=audio;
	this.free=new Array();
	this.insts=new Array();
}

gxtkSample.prototype.FreeAudio=function( audio ){
	this.free.push( audio );
}

gxtkSample.prototype.AllocAudio=function(){
	var audio;
	while( this.free.length ){
		audio=this.free.pop();
		try{
			audio.currentTime=0;
			return audio;
		}catch( ex ){
			print( "AUDIO ERROR1!" );
		}
	}
	
	//Max out?
	if( this.insts.length==8 ) return null;
	
	audio=new Audio( this.audio.src );
	
	//yucky loop handler for firefox!
	//
	audio.addEventListener( 'ended',function(){
		if( this.loop ){
			try{
				this.currentTime=0;
				this.play();
			}catch( ex ){
				print( "AUDIO ERROR2!" );
			}
		}
	},false );

	this.insts.push( audio );
	return audio;
}

gxtkSample.prototype.Discard=function(){
}


function BBThread(){
	this.running=false;
}

BBThread.prototype.Start=function(){
	this.Run__UNSAFE__();
}

BBThread.prototype.IsRunning=function(){
	return this.running;
}

BBThread.prototype.Run__UNSAFE__=function(){
}

function BBAsyncImageLoaderThread(){
	BBThread.call(this);
}

BBAsyncImageLoaderThread.prototype=extend_class( BBThread );

BBAsyncImageLoaderThread.prototype.Start=function(){

	var thread=this;

	var image=new Image();
	
	image.onload=function( e ){
		image.meta_width=image.width;
		image.meta_height=image.height;
		thread._surface=new gxtkSurface( image,thread._device )
		thread.running=false;
	}
	
	image.onerror=function( e ){
		thread._surface=null;
		thread.running=false;
	}
	
	thread.running=true;
	
	image.src=fixDataPath( thread._path );
}


function BBAsyncSoundLoaderThread(){
	BBThread.call(this);
}

BBAsyncSoundLoaderThread.prototype=extend_class( BBThread );

BBAsyncSoundLoaderThread.prototype.Start=function(){
	this._sample=this._device.LoadSample( this._path );
}
function bb_app_App(){
	Object.call(this);
}
function bb_app_App_new(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<109>";
	bb_app_device=bb_app_AppDevice_new.call(new bb_app_AppDevice,this);
	pop_err();
	return this;
}
bb_app_App.prototype.m_OnCreate=function(){
	push_err();
	pop_err();
	return 0;
}
bb_app_App.prototype.m_OnUpdate=function(){
	push_err();
	pop_err();
	return 0;
}
bb_app_App.prototype.m_OnSuspend=function(){
	push_err();
	pop_err();
	return 0;
}
bb_app_App.prototype.m_OnResume=function(){
	push_err();
	pop_err();
	return 0;
}
bb_app_App.prototype.m_OnRender=function(){
	push_err();
	pop_err();
	return 0;
}
bb_app_App.prototype.m_OnLoading=function(){
	push_err();
	pop_err();
	return 0;
}
function bb_Beacon_Beacon(){
	bb_app_App.call(this);
	this.f_Title=null;
	this.f_ServerLabel=null;
	this.f_Games=null;
	this.f_PwLabel=null;
	this.f_Pw=null;
	this.f_isOn=false;
}
bb_Beacon_Beacon.prototype=extend_class(bb_app_App);
function bb_Beacon_Beacon_new(){
	push_err();
	err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<3>";
	bb_app_App_new.call(this);
	err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<3>";
	pop_err();
	return this;
}
bb_Beacon_Beacon.prototype.m_OnCreate=function(){
	push_err();
	err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<17>";
	bb_challengergui_CHGUI_MobileMode=1;
	err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<18>";
	bb_app_SetUpdateRate(30);
	pop_err();
	return 0;
}
bb_Beacon_Beacon.prototype.m_OnRender=function(){
	push_err();
	err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<22>";
	var t_=bb_data2_STATUS;
	err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<23>";
	if(t_=="normal"){
		err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<24>";
		bb_graphics_Cls(247.0,247.0,247.0);
		err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<25>";
		bb_challengergui_CHGUI_Draw();
	}else{
		err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<26>";
		if(t_=="start"){
			err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<27>";
			this.f_Title=bb_data2_CScale(bb_challengergui_CreateLabel(5,10,"Beacon Config",null));
			err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<28>";
			this.f_ServerLabel=bb_data2_CScale(bb_challengergui_CreateLabel(5,60,"Server Type: Static",null));
			err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<29>";
			this.f_Games=bb_data2_CScale(bb_challengergui_CreateDropdown(10,110,((bb_data2_SCALE_W-20.0)|0),40,"Games",null));
			err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<30>";
			this.f_PwLabel=bb_data2_CScale(bb_challengergui_CreateLabel(5,170,"Password:",null));
			err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<31>";
			this.f_Pw=bb_data2_CScale(bb_challengergui_CreateTextfield(150,170,100,36,"",null));
			err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<33>";
			this.f_isOn=false;
			err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<34>";
			bb_data2_STATUS="normal";
		}
	}
	pop_err();
	return 0;
}
bb_Beacon_Beacon.prototype.m_OnUpdate=function(){
	push_err();
	err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<39>";
	var t_=bb_data2_STATUS;
	err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<40>";
	if(t_=="normal"){
		err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<41>";
		bb_challengergui_CHGUI_Update();
	}
	pop_err();
	return 0;
}
function bb_app_AppDevice(){
	gxtkApp.call(this);
	this.f_app=null;
	this.f_updateRate=0;
}
bb_app_AppDevice.prototype=extend_class(gxtkApp);
function bb_app_AppDevice_new(t_app){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<49>";
	dbg_object(this).f_app=t_app;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<50>";
	bb_graphics_SetGraphicsDevice(this.GraphicsDevice());
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<51>";
	bb_input_SetInputDevice(this.InputDevice());
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<52>";
	bb_audio_SetAudioDevice(this.AudioDevice());
	pop_err();
	return this;
}
function bb_app_AppDevice_new2(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<46>";
	pop_err();
	return this;
}
bb_app_AppDevice.prototype.OnCreate=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<56>";
	bb_graphics_SetFont(null,32);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<57>";
	var t_=this.f_app.m_OnCreate();
	pop_err();
	return t_;
}
bb_app_AppDevice.prototype.OnUpdate=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<61>";
	var t_=this.f_app.m_OnUpdate();
	pop_err();
	return t_;
}
bb_app_AppDevice.prototype.OnSuspend=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<65>";
	var t_=this.f_app.m_OnSuspend();
	pop_err();
	return t_;
}
bb_app_AppDevice.prototype.OnResume=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<69>";
	var t_=this.f_app.m_OnResume();
	pop_err();
	return t_;
}
bb_app_AppDevice.prototype.OnRender=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<73>";
	bb_graphics_BeginRender();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<74>";
	var t_r=this.f_app.m_OnRender();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<75>";
	bb_graphics_EndRender();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<76>";
	pop_err();
	return t_r;
}
bb_app_AppDevice.prototype.OnLoading=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<80>";
	bb_graphics_BeginRender();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<81>";
	var t_r=this.f_app.m_OnLoading();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<82>";
	bb_graphics_EndRender();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<83>";
	pop_err();
	return t_r;
}
bb_app_AppDevice.prototype.SetUpdateRate=function(t_hertz){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<87>";
	gxtkApp.prototype.SetUpdateRate.call(this,t_hertz);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<88>";
	this.f_updateRate=t_hertz;
	pop_err();
	return 0;
}
var bb_graphics_device;
function bb_graphics_SetGraphicsDevice(t_dev){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<58>";
	bb_graphics_device=t_dev;
	pop_err();
	return 0;
}
var bb_input_device;
function bb_input_SetInputDevice(t_dev){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<16>";
	bb_input_device=t_dev;
	pop_err();
	return 0;
}
var bb_audio_device;
function bb_audio_SetAudioDevice(t_dev){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/audio.monkey<17>";
	bb_audio_device=t_dev;
	pop_err();
	return 0;
}
var bb_app_device;
function bbMain(){
	push_err();
	err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<48>";
	bb_Beacon_Beacon_new.call(new bb_Beacon_Beacon);
	pop_err();
	return 0;
}
function bb_graphics_Image(){
	Object.call(this);
	this.f_surface=null;
	this.f_width=0;
	this.f_height=0;
	this.f_frames=[];
	this.f_flags=0;
	this.f_tx=.0;
	this.f_ty=.0;
	this.f_source=null;
}
var bb_graphics_Image_DefaultFlags;
function bb_graphics_Image_new(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<65>";
	pop_err();
	return this;
}
bb_graphics_Image.prototype.m_SetHandle=function(t_tx,t_ty){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<109>";
	dbg_object(this).f_tx=t_tx;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<110>";
	dbg_object(this).f_ty=t_ty;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<111>";
	dbg_object(this).f_flags=dbg_object(this).f_flags&-2;
	pop_err();
	return 0;
}
bb_graphics_Image.prototype.m_ApplyFlags=function(t_iflags){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<178>";
	this.f_flags=t_iflags;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<180>";
	if((this.f_flags&2)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<181>";
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<181>";
		var t_=this.f_frames;
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<181>";
		var t_2=0;
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<181>";
		while(t_2<t_.length){
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<181>";
			var t_f=dbg_array(t_,t_2)[dbg_index];
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<181>";
			t_2=t_2+1;
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<182>";
			dbg_object(t_f).f_x+=1;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<184>";
		this.f_width-=2;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<187>";
	if((this.f_flags&4)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<188>";
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<188>";
		var t_3=this.f_frames;
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<188>";
		var t_4=0;
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<188>";
		while(t_4<t_3.length){
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<188>";
			var t_f2=dbg_array(t_3,t_4)[dbg_index];
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<188>";
			t_4=t_4+1;
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<189>";
			dbg_object(t_f2).f_y+=1;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<191>";
		this.f_height-=2;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<194>";
	if((this.f_flags&1)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<195>";
		this.m_SetHandle((this.f_width)/2.0,(this.f_height)/2.0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<198>";
	if(this.f_frames.length==1 && dbg_object(dbg_array(this.f_frames,0)[dbg_index]).f_x==0 && dbg_object(dbg_array(this.f_frames,0)[dbg_index]).f_y==0 && this.f_width==this.f_surface.Width() && this.f_height==this.f_surface.Height()){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<199>";
		this.f_flags|=65536;
	}
	pop_err();
	return 0;
}
bb_graphics_Image.prototype.m_Init=function(t_surf,t_nframes,t_iflags){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<136>";
	this.f_surface=t_surf;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<138>";
	this.f_width=((this.f_surface.Width()/t_nframes)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<139>";
	this.f_height=this.f_surface.Height();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<141>";
	this.f_frames=new_object_array(t_nframes);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<142>";
	for(var t_i=0;t_i<t_nframes;t_i=t_i+1){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<143>";
		dbg_array(this.f_frames,t_i)[dbg_index]=bb_graphics_Frame_new.call(new bb_graphics_Frame,t_i*this.f_width,0)
	}
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<146>";
	this.m_ApplyFlags(t_iflags);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<147>";
	pop_err();
	return this;
}
bb_graphics_Image.prototype.m_Grab=function(t_x,t_y,t_iwidth,t_iheight,t_nframes,t_iflags,t_source){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<151>";
	dbg_object(this).f_source=t_source;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<152>";
	this.f_surface=dbg_object(t_source).f_surface;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<154>";
	this.f_width=t_iwidth;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<155>";
	this.f_height=t_iheight;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<157>";
	this.f_frames=new_object_array(t_nframes);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<159>";
	var t_ix=t_x;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<159>";
	var t_iy=t_y;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<161>";
	for(var t_i=0;t_i<t_nframes;t_i=t_i+1){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<162>";
		if(t_ix+this.f_width>dbg_object(t_source).f_width){
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<163>";
			t_ix=0;
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<164>";
			t_iy+=this.f_height;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<166>";
		if(t_ix+this.f_width>dbg_object(t_source).f_width || t_iy+this.f_height>dbg_object(t_source).f_height){
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<167>";
			error("Image frame outside surface");
		}
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<169>";
		dbg_array(this.f_frames,t_i)[dbg_index]=bb_graphics_Frame_new.call(new bb_graphics_Frame,t_ix+dbg_object(dbg_array(dbg_object(t_source).f_frames,0)[dbg_index]).f_x,t_iy+dbg_object(dbg_array(dbg_object(t_source).f_frames,0)[dbg_index]).f_y)
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<170>";
		t_ix+=this.f_width;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<173>";
	this.m_ApplyFlags(t_iflags);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<174>";
	pop_err();
	return this;
}
bb_graphics_Image.prototype.m_GrabImage=function(t_x,t_y,t_width,t_height,t_frames,t_flags){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<104>";
	if(dbg_object(this).f_frames.length!=1){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<104>";
		pop_err();
		return null;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<105>";
	var t_=(bb_graphics_Image_new.call(new bb_graphics_Image)).m_Grab(t_x,t_y,t_width,t_height,t_frames,t_flags,this);
	pop_err();
	return t_;
}
bb_graphics_Image.prototype.m_Width=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<76>";
	pop_err();
	return this.f_width;
}
bb_graphics_Image.prototype.m_Height=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<80>";
	pop_err();
	return this.f_height;
}
function bb_graphics_GraphicsContext(){
	Object.call(this);
	this.f_defaultFont=null;
	this.f_font=null;
	this.f_firstChar=0;
	this.f_matrixSp=0;
	this.f_ix=1.0;
	this.f_iy=.0;
	this.f_jx=.0;
	this.f_jy=1.0;
	this.f_tx=.0;
	this.f_ty=.0;
	this.f_tformed=0;
	this.f_matDirty=0;
	this.f_color_r=.0;
	this.f_color_g=.0;
	this.f_color_b=.0;
	this.f_alpha=.0;
	this.f_blend=0;
	this.f_scissor_x=.0;
	this.f_scissor_y=.0;
	this.f_scissor_width=.0;
	this.f_scissor_height=.0;
	this.f_matrixStack=new_number_array(192);
}
function bb_graphics_GraphicsContext_new(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<24>";
	pop_err();
	return this;
}
bb_graphics_GraphicsContext.prototype.m_Validate=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<35>";
	if((this.f_matDirty)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<36>";
		bb_graphics_renderDevice.SetMatrix(dbg_object(bb_graphics_context).f_ix,dbg_object(bb_graphics_context).f_iy,dbg_object(bb_graphics_context).f_jx,dbg_object(bb_graphics_context).f_jy,dbg_object(bb_graphics_context).f_tx,dbg_object(bb_graphics_context).f_ty);
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<37>";
		this.f_matDirty=0;
	}
	pop_err();
	return 0;
}
var bb_graphics_context;
function bb_data_FixDataPath(t_path){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<3>";
	var t_i=t_path.indexOf(":/",0);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<4>";
	if(t_i!=-1 && t_path.indexOf("/",0)==t_i+1){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<4>";
		pop_err();
		return t_path;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<5>";
	if(string_startswith(t_path,"./") || string_startswith(t_path,"/")){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<5>";
		pop_err();
		return t_path;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<6>";
	var t_="monkey://data/"+t_path;
	pop_err();
	return t_;
}
function bb_graphics_Frame(){
	Object.call(this);
	this.f_x=0;
	this.f_y=0;
}
function bb_graphics_Frame_new(t_x,t_y){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<18>";
	dbg_object(this).f_x=t_x;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<19>";
	dbg_object(this).f_y=t_y;
	pop_err();
	return this;
}
function bb_graphics_Frame_new2(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<13>";
	pop_err();
	return this;
}
function bb_graphics_LoadImage(t_path,t_frameCount,t_flags){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<229>";
	var t_surf=bb_graphics_device.LoadSurface(bb_data_FixDataPath(t_path));
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<230>";
	if((t_surf)!=null){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<230>";
		var t_=(bb_graphics_Image_new.call(new bb_graphics_Image)).m_Init(t_surf,t_frameCount,t_flags);
		pop_err();
		return t_;
	}
	pop_err();
	return null;
}
function bb_graphics_LoadImage2(t_path,t_frameWidth,t_frameHeight,t_frameCount,t_flags){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<234>";
	var t_atlas=bb_graphics_LoadImage(t_path,1,0);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<235>";
	if((t_atlas)!=null){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<235>";
		var t_=t_atlas.m_GrabImage(0,0,t_frameWidth,t_frameHeight,t_frameCount,t_flags);
		pop_err();
		return t_;
	}
	pop_err();
	return null;
}
function bb_graphics_SetFont(t_font,t_firstChar){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<524>";
	if(!((t_font)!=null)){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<525>";
		if(!((dbg_object(bb_graphics_context).f_defaultFont)!=null)){
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<526>";
			dbg_object(bb_graphics_context).f_defaultFont=bb_graphics_LoadImage("mojo_font.png",96,2);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<528>";
		t_font=dbg_object(bb_graphics_context).f_defaultFont;
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<529>";
		t_firstChar=32;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<531>";
	dbg_object(bb_graphics_context).f_font=t_font;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<532>";
	dbg_object(bb_graphics_context).f_firstChar=t_firstChar;
	pop_err();
	return 0;
}
var bb_graphics_renderDevice;
function bb_graphics_SetMatrix(t_ix,t_iy,t_jx,t_jy,t_tx,t_ty){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<289>";
	dbg_object(bb_graphics_context).f_ix=t_ix;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<290>";
	dbg_object(bb_graphics_context).f_iy=t_iy;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<291>";
	dbg_object(bb_graphics_context).f_jx=t_jx;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<292>";
	dbg_object(bb_graphics_context).f_jy=t_jy;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<293>";
	dbg_object(bb_graphics_context).f_tx=t_tx;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<294>";
	dbg_object(bb_graphics_context).f_ty=t_ty;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<295>";
	dbg_object(bb_graphics_context).f_tformed=((t_ix!=1.0 || t_iy!=0.0 || t_jx!=0.0 || t_jy!=1.0 || t_tx!=0.0 || t_ty!=0.0)?1:0);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<296>";
	dbg_object(bb_graphics_context).f_matDirty=1;
	pop_err();
	return 0;
}
function bb_graphics_SetMatrix2(t_m){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<285>";
	bb_graphics_SetMatrix(dbg_array(t_m,0)[dbg_index],dbg_array(t_m,1)[dbg_index],dbg_array(t_m,2)[dbg_index],dbg_array(t_m,3)[dbg_index],dbg_array(t_m,4)[dbg_index],dbg_array(t_m,5)[dbg_index]);
	pop_err();
	return 0;
}
function bb_graphics_SetColor(t_r,t_g,t_b){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<244>";
	dbg_object(bb_graphics_context).f_color_r=t_r;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<245>";
	dbg_object(bb_graphics_context).f_color_g=t_g;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<246>";
	dbg_object(bb_graphics_context).f_color_b=t_b;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<247>";
	bb_graphics_renderDevice.SetColor(t_r,t_g,t_b);
	pop_err();
	return 0;
}
function bb_graphics_SetAlpha(t_alpha){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<255>";
	dbg_object(bb_graphics_context).f_alpha=t_alpha;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<256>";
	bb_graphics_renderDevice.SetAlpha(t_alpha);
	pop_err();
	return 0;
}
function bb_graphics_SetBlend(t_blend){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<264>";
	dbg_object(bb_graphics_context).f_blend=t_blend;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<265>";
	bb_graphics_renderDevice.SetBlend(t_blend);
	pop_err();
	return 0;
}
function bb_graphics_DeviceWidth(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<221>";
	var t_=bb_graphics_device.Width();
	pop_err();
	return t_;
}
function bb_graphics_DeviceHeight(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<225>";
	var t_=bb_graphics_device.Height();
	pop_err();
	return t_;
}
function bb_graphics_SetScissor(t_x,t_y,t_width,t_height){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<273>";
	dbg_object(bb_graphics_context).f_scissor_x=t_x;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<274>";
	dbg_object(bb_graphics_context).f_scissor_y=t_y;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<275>";
	dbg_object(bb_graphics_context).f_scissor_width=t_width;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<276>";
	dbg_object(bb_graphics_context).f_scissor_height=t_height;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<277>";
	bb_graphics_renderDevice.SetScissor(((t_x)|0),((t_y)|0),((t_width)|0),((t_height)|0));
	pop_err();
	return 0;
}
function bb_graphics_BeginRender(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<206>";
	if(!((bb_graphics_device.Mode())!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<206>";
		pop_err();
		return 0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<207>";
	bb_graphics_renderDevice=bb_graphics_device;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<208>";
	dbg_object(bb_graphics_context).f_matrixSp=0;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<209>";
	bb_graphics_SetMatrix(1.0,0.0,0.0,1.0,0.0,0.0);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<210>";
	bb_graphics_SetColor(255.0,255.0,255.0);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<211>";
	bb_graphics_SetAlpha(1.0);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<212>";
	bb_graphics_SetBlend(0);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<213>";
	bb_graphics_SetScissor(0.0,0.0,(bb_graphics_DeviceWidth()),(bb_graphics_DeviceHeight()));
	pop_err();
	return 0;
}
function bb_graphics_EndRender(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<217>";
	bb_graphics_renderDevice=null;
	pop_err();
	return 0;
}
var bb_challengergui_CHGUI_MobileMode;
function bb_app_SetUpdateRate(t_hertz){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<145>";
	var t_=bb_app_device.SetUpdateRate(t_hertz);
	pop_err();
	return t_;
}
var bb_data2_STATUS;
function bb_graphics_DebugRenderDevice(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<48>";
	if(!((bb_graphics_renderDevice)!=null)){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<48>";
		error("Rendering operations can only be performed inside OnRender");
	}
	pop_err();
	return 0;
}
function bb_graphics_Cls(t_r,t_g,t_b){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<354>";
	bb_graphics_DebugRenderDevice();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<356>";
	bb_graphics_renderDevice.Cls(t_r,t_g,t_b);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI(){
	Object.call(this);
	this.f_Visible=1;
	this.f_Element="";
	this.f_Parent=null;
	this.f_Minimised=0;
	this.f_X=.0;
	this.f_Y=.0;
	this.f_W=.0;
	this.f_H=.0;
	this.f_Active=1;
	this.f_Shadow=0;
	this.f_Text="";
	this.f_Close=0;
	this.f_CloseOver=0;
	this.f_CloseDown=0;
	this.f_Minimise=0;
	this.f_MinimiseOver=0;
	this.f_MinimiseDown=0;
	this.f_HasMenu=0;
	this.f_MenuHeight=0;
	this.f_Tabbed=0;
	this.f_TabHeight=0;
	this.f_Buttons=[];
	this.f_Over=0;
	this.f_Down=0;
	this.f_ImageButtons=[];
	this.f_Img=null;
	this.f_Tickboxes=[];
	this.f_Value=.0;
	this.f_Radioboxes=[];
	this.f_Listboxes=[];
	this.f_ListboxSlider=null;
	this.f_ListboxNumber=0;
	this.f_ListboxItems=[];
	this.f_ListHeight=0;
	this.f_SelectedListboxItem=null;
	this.f_HSliders=[];
	this.f_MinusOver=0;
	this.f_MinusDown=0;
	this.f_PlusOver=0;
	this.f_PlusDown=0;
	this.f_SliderOver=0;
	this.f_SliderDown=0;
	this.f_Minimum=.0;
	this.f_Stp=.0;
	this.f_SWidth=.0;
	this.f_VSliders=[];
	this.f_Textfields=[];
	this.f_OnFocus=0;
	this.f_Cursor=0;
	this.f_Labels=[];
	this.f_Dropdowns=[];
	this.f_DropdownItems=[];
	this.f_DropNumber=0;
	this.f_Menus=[];
	this.f_Tabs=[];
	this.f_CurrentTab=null;
	this.f_BottomList=[];
	this.f_VariList=[];
	this.f_TopList=[];
	this.f_MenuItems=[];
	this.f_IsMenuParent=0;
	this.f_Tick=0;
	this.f_MenuWidth=0;
	this.f_MenuNumber=0;
	this.f_Tooltip="";
	this.f_Moveable=0;
	this.f_Mode=0;
	this.f_IsParent=0;
	this.f_SubWindow=0;
	this.f_ReOrdered=0;
	this.f_Clicked=0;
	this.f_DoubleClickMillisecs=0;
	this.f_DoubleClicked=0;
	this.f_StartOvertime=0;
	this.f_OverTime=0;
	this.f_StartDowntime=0;
	this.f_DownTime=0;
	this.f_TopVari=null;
	this.f_TopTop=null;
	this.f_TopBottom=null;
	this.f_MenuActive=null;
	this.f_MenuOver=null;
	this.f_FormatText=1;
	this.f_FormatNumber=1;
	this.f_FormatSymbol=1;
	this.f_FormatSpace=1;
	this.f_DKeyMillisecs=0;
	this.f_Maximum=.0;
	this.f_Start=0;
	this.f_Group=0;
	this.f_Moving=0;
	this.f_MX=.0;
	this.f_MY=.0;
	this.f_DClickMillisecs=0;
}
function bb_challengergui_CHGUI_new(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<139>";
	pop_err();
	return this;
}
bb_challengergui_CHGUI.prototype.m_CheckClicked=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<349>";
	if(bb_challengergui_CHGUI_RealActive(this)==0 || bb_challengergui_CHGUI_RealVisible(this)==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<350>";
		this.f_Over=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<351>";
		this.f_Down=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<352>";
		this.f_Clicked=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<354>";
	if(((this.f_Over)!=0) && ((this.f_Down)!=0) && bb_input_TouchDown(0)==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<355>";
		this.f_Clicked=1;
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<357>";
		this.f_Clicked=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<361>";
	if((this.f_Clicked)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<362>";
		if(bb_app_Millisecs()<this.f_DoubleClickMillisecs){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<362>";
			this.f_DoubleClicked=1;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<363>";
		this.f_DoubleClickMillisecs=bb_app_Millisecs()+275;
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<365>";
		this.f_DoubleClicked=0;
	}
	pop_err();
	return 0;
}
bb_challengergui_CHGUI.prototype.m_CheckOver=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<276>";
	if(this.f_Minimised==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<277>";
		if(this!=bb_challengergui_CHGUI_Canvas){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<278>";
			var t_XX=bb_challengergui_CHGUI_RealX(this);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<279>";
			var t_YY=bb_challengergui_CHGUI_RealY(this);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<281>";
			if(bb_challengergui_CHGUI_OverFlag==0 && bb_challengergui_CHGUI_IgnoreMouse==0 && bb_input_TouchX(0)>(t_XX) && bb_input_TouchX(0)<(t_XX)+this.f_W && bb_input_TouchY(0)>(t_YY) && bb_input_TouchY(0)<(t_YY)+this.f_H){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<282>";
				this.f_Over=1;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<283>";
				bb_challengergui_CHGUI_Over=1;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<284>";
				bb_challengergui_CHGUI_OverFlag=1;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<285>";
				if(this.f_Element!="Window"){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<285>";
					bb_challengergui_CHGUI_DragOver=1;
				}
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<286>";
				this.f_OverTime=bb_app_Millisecs()-this.f_StartOvertime;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<288>";
				this.f_Over=0;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<289>";
				this.f_OverTime=0;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<290>";
				this.f_StartOvertime=bb_app_Millisecs();
			}
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<294>";
		if(this!=bb_challengergui_CHGUI_Canvas){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<295>";
			var t_XX2=bb_challengergui_CHGUI_RealX(this);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<296>";
			var t_YY2=bb_challengergui_CHGUI_RealY(this);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<297>";
			if(bb_challengergui_CHGUI_OverFlag==0 && bb_challengergui_CHGUI_IgnoreMouse==0 && bb_input_TouchX(0)>(t_XX2) && bb_input_TouchX(0)<(t_XX2)+this.f_W && bb_input_TouchY(0)>(t_YY2) && bb_input_TouchY(0)<(t_YY2)+bb_challengergui_CHGUI_TitleHeight){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<298>";
				this.f_Over=1;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<299>";
				bb_challengergui_CHGUI_Over=1;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<300>";
				bb_challengergui_CHGUI_OverFlag=1;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<301>";
				this.f_OverTime=bb_app_Millisecs()-this.f_StartOvertime;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<303>";
				this.f_Over=0;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<304>";
				this.f_OverTime=0;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<305>";
				this.f_StartOvertime=bb_app_Millisecs();
			}
		}
	}
	pop_err();
	return 0;
}
bb_challengergui_CHGUI.prototype.m_CheckDown=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<314>";
	if(((this.f_Over)!=0) && ((bb_input_TouchDown(0))!=0) && bb_challengergui_CHGUI_MouseBusy==0 || this.f_Down==1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<315>";
		bb_challengergui_CHGUI_DownFlag=1;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<316>";
		bb_challengergui_CHGUI_MouseBusy=1;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<317>";
		this.f_Down=1;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<318>";
		if(this.f_Element!="Window"){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<318>";
			bb_challengergui_CHGUI_DragOver=1;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<319>";
		this.f_DownTime=bb_app_Millisecs()-this.f_StartDowntime;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<320>";
		this.f_StartOvertime=bb_app_Millisecs();
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<322>";
	if(bb_input_TouchDown(0)==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<323>";
		this.f_Down=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<324>";
		this.f_DownTime=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<325>";
		this.f_StartDowntime=bb_app_Millisecs();
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<327>";
	if(this.f_Over==1 && ((bb_input_TouchDown(0))!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<328>";
		if(this.f_Down==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<328>";
			this.f_Over=0;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<331>";
	if((this.f_Down)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<332>";
		if(this.f_Mode==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<332>";
			bb_challengergui_CHGUI_Reorder(this);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<333>";
		var t_E=this;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<334>";
		do{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<335>";
			if(dbg_object(t_E).f_Parent!=null){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<336>";
				if(dbg_object(dbg_object(t_E).f_Parent).f_Element=="Window" && dbg_object(dbg_object(t_E).f_Parent).f_Mode==1){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<337>";
					bb_challengergui_CHGUI_Reorder(dbg_object(t_E).f_Parent);
				}
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<339>";
				t_E=dbg_object(t_E).f_Parent;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<341>";
				break;
			}
		}while(!(false));
	}
	pop_err();
	return 0;
}
var bb_challengergui_CHGUI_BottomList;
var bb_challengergui_CHGUI_Canvas;
function bb_challengergui_CHGUI_RealVisible(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3921>";
	var t_E=null;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3922>";
	t_E=t_N;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3923>";
	var t_V=dbg_object(t_N).f_Visible;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3924>";
	if(t_V==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3924>";
		pop_err();
		return t_V;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3925>";
	do{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3926>";
		if(dbg_object(t_E).f_Parent!=null){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3927>";
			t_V=dbg_object(dbg_object(t_E).f_Parent).f_Visible;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3928>";
			if(t_V==0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3928>";
				break;
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3929>";
			t_E=dbg_object(t_E).f_Parent;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3931>";
			break;
		}
	}while(!(false));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3934>";
	if(dbg_object(bb_challengergui_CHGUI_Canvas).f_Visible==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3934>";
		t_V=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3935>";
	pop_err();
	return t_V;
}
function bb_challengergui_CHGUI_RealMinimised(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3962>";
	var t_E=null;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3963>";
	t_E=t_N;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3964>";
	var t_M=dbg_object(t_E).f_Minimised;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3965>";
	if(t_M==1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3965>";
		pop_err();
		return t_M;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3966>";
	do{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3967>";
		if(dbg_object(t_E).f_Parent!=null){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3968>";
			t_M=dbg_object(dbg_object(t_E).f_Parent).f_Minimised;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3969>";
			if(t_M==1){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3969>";
				break;
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3970>";
			t_E=dbg_object(t_E).f_Parent;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3972>";
			break;
		}
	}while(!(false));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3975>";
	pop_err();
	return t_M;
}
var bb_challengergui_CHGUI_OffsetX;
function bb_challengergui_CHGUI_RealX(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3881>";
	var t_E=null;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3882>";
	t_E=t_N;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3883>";
	var t_X=((dbg_object(t_N).f_X)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3884>";
	do{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3885>";
		if(dbg_object(t_E).f_Parent!=null){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3886>";
			if(dbg_object(dbg_object(t_E).f_Parent).f_Element!="Tab"){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3887>";
				t_X=(((t_X)+dbg_object(dbg_object(t_E).f_Parent).f_X)|0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3888>";
				t_E=dbg_object(t_E).f_Parent;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3890>";
				t_E=dbg_object(t_E).f_Parent;
			}
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3893>";
			break;
		}
	}while(!(false));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3896>";
	t_X=(((t_X)+bb_challengergui_CHGUI_OffsetX)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3897>";
	pop_err();
	return t_X;
}
var bb_challengergui_CHGUI_OffsetY;
function bb_challengergui_CHGUI_RealY(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3901>";
	var t_E=null;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3902>";
	t_E=t_N;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3903>";
	var t_Y=((dbg_object(t_N).f_Y)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3904>";
	do{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3905>";
		if(dbg_object(t_E).f_Parent!=null){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3906>";
			if(dbg_object(dbg_object(t_E).f_Parent).f_Element!="Tab"){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3907>";
				t_Y=(((t_Y)+dbg_object(dbg_object(t_E).f_Parent).f_Y)|0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3908>";
				t_E=dbg_object(t_E).f_Parent;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3910>";
				t_E=dbg_object(t_E).f_Parent;
			}
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3913>";
			break;
		}
	}while(!(false));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3916>";
	t_Y=(((t_Y)+bb_challengergui_CHGUI_OffsetY)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3917>";
	pop_err();
	return t_Y;
}
var bb_challengergui_CHGUI_TitleHeight;
var bb_challengergui_CHGUI_LockedWIndow;
function bb_challengergui_CHGUI_RealActive(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3939>";
	var t_E=null;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3940>";
	t_E=t_N;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3941>";
	var t_A=dbg_object(t_N).f_Active;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3942>";
	if(t_A==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3942>";
		pop_err();
		return t_A;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3943>";
	do{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3944>";
		if(t_E==bb_challengergui_CHGUI_LockedWIndow){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3944>";
			pop_err();
			return 1;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3945>";
		if(dbg_object(t_E).f_Parent!=null){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3946>";
			if(dbg_object(t_E).f_Parent==bb_challengergui_CHGUI_LockedWIndow){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3947>";
				pop_err();
				return 1;
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3949>";
			t_A=dbg_object(dbg_object(t_E).f_Parent).f_Active;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3950>";
			if(t_A==0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3950>";
				break;
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3952>";
			t_E=dbg_object(t_E).f_Parent;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3954>";
			break;
		}
	}while(!(false));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3958>";
	pop_err();
	return t_A;
}
var bb_challengergui_CHGUI_Shadow;
var bb_challengergui_CHGUI_ShadowImg;
function bb_graphics_PushMatrix(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<310>";
	var t_sp=dbg_object(bb_graphics_context).f_matrixSp;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<311>";
	dbg_array(dbg_object(bb_graphics_context).f_matrixStack,t_sp+0)[dbg_index]=dbg_object(bb_graphics_context).f_ix
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<312>";
	dbg_array(dbg_object(bb_graphics_context).f_matrixStack,t_sp+1)[dbg_index]=dbg_object(bb_graphics_context).f_iy
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<313>";
	dbg_array(dbg_object(bb_graphics_context).f_matrixStack,t_sp+2)[dbg_index]=dbg_object(bb_graphics_context).f_jx
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<314>";
	dbg_array(dbg_object(bb_graphics_context).f_matrixStack,t_sp+3)[dbg_index]=dbg_object(bb_graphics_context).f_jy
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<315>";
	dbg_array(dbg_object(bb_graphics_context).f_matrixStack,t_sp+4)[dbg_index]=dbg_object(bb_graphics_context).f_tx
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<316>";
	dbg_array(dbg_object(bb_graphics_context).f_matrixStack,t_sp+5)[dbg_index]=dbg_object(bb_graphics_context).f_ty
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<317>";
	dbg_object(bb_graphics_context).f_matrixSp=t_sp+6;
	pop_err();
	return 0;
}
function bb_graphics_Transform(t_ix,t_iy,t_jx,t_jy,t_tx,t_ty){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<331>";
	var t_ix2=t_ix*dbg_object(bb_graphics_context).f_ix+t_iy*dbg_object(bb_graphics_context).f_jx;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<332>";
	var t_iy2=t_ix*dbg_object(bb_graphics_context).f_iy+t_iy*dbg_object(bb_graphics_context).f_jy;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<333>";
	var t_jx2=t_jx*dbg_object(bb_graphics_context).f_ix+t_jy*dbg_object(bb_graphics_context).f_jx;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<334>";
	var t_jy2=t_jx*dbg_object(bb_graphics_context).f_iy+t_jy*dbg_object(bb_graphics_context).f_jy;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<335>";
	var t_tx2=t_tx*dbg_object(bb_graphics_context).f_ix+t_ty*dbg_object(bb_graphics_context).f_jx+dbg_object(bb_graphics_context).f_tx;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<336>";
	var t_ty2=t_tx*dbg_object(bb_graphics_context).f_iy+t_ty*dbg_object(bb_graphics_context).f_jy+dbg_object(bb_graphics_context).f_ty;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<337>";
	bb_graphics_SetMatrix(t_ix2,t_iy2,t_jx2,t_jy2,t_tx2,t_ty2);
	pop_err();
	return 0;
}
function bb_graphics_Transform2(t_m){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<327>";
	bb_graphics_Transform(dbg_array(t_m,0)[dbg_index],dbg_array(t_m,1)[dbg_index],dbg_array(t_m,2)[dbg_index],dbg_array(t_m,3)[dbg_index],dbg_array(t_m,4)[dbg_index],dbg_array(t_m,5)[dbg_index]);
	pop_err();
	return 0;
}
function bb_graphics_Translate(t_x,t_y){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<341>";
	bb_graphics_Transform(1.0,0.0,0.0,1.0,t_x,t_y);
	pop_err();
	return 0;
}
function bb_graphics_PopMatrix(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<321>";
	var t_sp=dbg_object(bb_graphics_context).f_matrixSp-6;
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<322>";
	bb_graphics_SetMatrix(dbg_array(dbg_object(bb_graphics_context).f_matrixStack,t_sp+0)[dbg_index],dbg_array(dbg_object(bb_graphics_context).f_matrixStack,t_sp+1)[dbg_index],dbg_array(dbg_object(bb_graphics_context).f_matrixStack,t_sp+2)[dbg_index],dbg_array(dbg_object(bb_graphics_context).f_matrixStack,t_sp+3)[dbg_index],dbg_array(dbg_object(bb_graphics_context).f_matrixStack,t_sp+4)[dbg_index],dbg_array(dbg_object(bb_graphics_context).f_matrixStack,t_sp+5)[dbg_index]);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<323>";
	dbg_object(bb_graphics_context).f_matrixSp=t_sp;
	pop_err();
	return 0;
}
function bb_graphics_DrawImageRect(t_image,t_x,t_y,t_srcX,t_srcY,t_srcWidth,t_srcHeight,t_frame){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<473>";
	bb_graphics_DebugRenderDevice();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<475>";
	var t_f=dbg_array(dbg_object(t_image).f_frames,t_frame)[dbg_index];
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<477>";
	if((dbg_object(bb_graphics_context).f_tformed)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<478>";
		bb_graphics_PushMatrix();
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<481>";
		bb_graphics_Translate(-dbg_object(t_image).f_tx+t_x,-dbg_object(t_image).f_ty+t_y);
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<483>";
		bb_graphics_context.m_Validate();
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<485>";
		bb_graphics_renderDevice.DrawSurface2(dbg_object(t_image).f_surface,0.0,0.0,t_srcX+dbg_object(t_f).f_x,t_srcY+dbg_object(t_f).f_y,t_srcWidth,t_srcHeight);
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<487>";
		bb_graphics_PopMatrix();
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<489>";
		bb_graphics_context.m_Validate();
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<491>";
		bb_graphics_renderDevice.DrawSurface2(dbg_object(t_image).f_surface,-dbg_object(t_image).f_tx+t_x,-dbg_object(t_image).f_ty+t_y,t_srcX+dbg_object(t_f).f_x,t_srcY+dbg_object(t_f).f_y,t_srcWidth,t_srcHeight);
	}
	pop_err();
	return 0;
}
function bb_graphics_Rotate(t_angle){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<349>";
	bb_graphics_Transform(Math.cos((t_angle)*D2R),-Math.sin((t_angle)*D2R),Math.sin((t_angle)*D2R),Math.cos((t_angle)*D2R),0.0,0.0);
	pop_err();
	return 0;
}
function bb_graphics_Scale(t_x,t_y){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<345>";
	bb_graphics_Transform(t_x,0.0,0.0,t_y,0.0,0.0);
	pop_err();
	return 0;
}
function bb_graphics_DrawImageRect2(t_image,t_x,t_y,t_srcX,t_srcY,t_srcWidth,t_srcHeight,t_rotation,t_scaleX,t_scaleY,t_frame){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<497>";
	bb_graphics_DebugRenderDevice();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<499>";
	var t_f=dbg_array(dbg_object(t_image).f_frames,t_frame)[dbg_index];
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<501>";
	bb_graphics_PushMatrix();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<503>";
	bb_graphics_Translate(t_x,t_y);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<504>";
	bb_graphics_Rotate(t_rotation);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<505>";
	bb_graphics_Scale(t_scaleX,t_scaleY);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<506>";
	bb_graphics_Translate(-dbg_object(t_image).f_tx,-dbg_object(t_image).f_ty);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<508>";
	bb_graphics_context.m_Validate();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<510>";
	bb_graphics_renderDevice.DrawSurface2(dbg_object(t_image).f_surface,0.0,0.0,t_srcX+dbg_object(t_f).f_x,t_srcY+dbg_object(t_f).f_y,t_srcWidth,t_srcHeight);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<512>";
	bb_graphics_PopMatrix();
	pop_err();
	return 0;
}
var bb_challengergui_CHGUI_Style;
function bb_bitmapfont_BitmapFont(){
	Object.call(this);
	this.f_faceChars=[];
	this.f__drawShadow=true;
	this.f_borderChars=[];
	this.f__kerning=null;
	this.f_packedImages=[];
	this.f_shadowChars=[];
	this.f__drawBorder=true;
	this.implments={bb_fontinterface_Font:1};
}
bb_bitmapfont_BitmapFont.prototype.m_GetFontHeight=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<263>";
	if(dbg_array(this.f_faceChars,32)[dbg_index]==null){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<263>";
		pop_err();
		return 0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<264>";
	var t_=((dbg_object(dbg_object(dbg_object(dbg_array(this.f_faceChars,32)[dbg_index]).f_drawingMetrics).f_drawingSize).f_y)|0);
	pop_err();
	return t_;
}
bb_bitmapfont_BitmapFont.prototype.m_DrawShadow=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<66>";
	pop_err();
	return this.f__drawShadow;
}
bb_bitmapfont_BitmapFont.prototype.m_DrawShadow2=function(t_value){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<70>";
	this.f__drawShadow=t_value;
	pop_err();
	return 0;
}
bb_bitmapfont_BitmapFont.prototype.m_Kerning=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<667>";
	if(this.f__kerning==null){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<667>";
		this.f__kerning=bb_drawingpoint_DrawingPoint_new.call(new bb_drawingpoint_DrawingPoint);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<668>";
	pop_err();
	return this.f__kerning;
}
bb_bitmapfont_BitmapFont.prototype.m_Kerning2=function(t_value){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<672>";
	this.f__kerning=t_value;
	pop_err();
}
bb_bitmapfont_BitmapFont.prototype.m_GetTxtWidth=function(t_text,t_fromChar,t_toChar){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<214>";
	var t_twidth=.0;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<215>";
	var t_MaxWidth=0.0;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<216>";
	var t_char=0;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<217>";
	var t_lastchar=0;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<219>";
	for(var t_i=t_fromChar;t_i<=t_toChar;t_i=t_i+1){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<220>";
		t_char=t_text.charCodeAt(t_i-1);
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<221>";
		if(t_char>=0 && t_char<this.f_faceChars.length && t_char!=10 && t_char!=13){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<222>";
			if(dbg_array(this.f_faceChars,t_char)[dbg_index]!=null){
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<223>";
				t_lastchar=t_char;
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<224>";
				t_twidth=t_twidth+dbg_object(dbg_object(dbg_array(this.f_faceChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingWidth+dbg_object(this.m_Kerning()).f_x;
			}
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<226>";
			if(t_char==10){
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<227>";
				if(bb_math_Abs2(t_MaxWidth)<bb_math_Abs2(t_twidth)){
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<227>";
					t_MaxWidth=t_twidth-dbg_object(this.m_Kerning()).f_x-dbg_object(dbg_object(dbg_array(this.f_faceChars,t_lastchar)[dbg_index]).f_drawingMetrics).f_drawingWidth+dbg_object(dbg_object(dbg_object(dbg_array(this.f_faceChars,t_lastchar)[dbg_index]).f_drawingMetrics).f_drawingSize).f_x;
				}
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<228>";
				t_twidth=0.0;
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<229>";
				t_lastchar=t_char;
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<232>";
	if(t_lastchar>=0 && t_lastchar<this.f_faceChars.length){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<233>";
		if(dbg_array(this.f_faceChars,t_lastchar)[dbg_index]!=null){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<234>";
			t_twidth=t_twidth-dbg_object(dbg_object(dbg_array(this.f_faceChars,t_lastchar)[dbg_index]).f_drawingMetrics).f_drawingWidth;
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<235>";
			t_twidth=t_twidth+dbg_object(dbg_object(dbg_object(dbg_array(this.f_faceChars,t_lastchar)[dbg_index]).f_drawingMetrics).f_drawingSize).f_x;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<238>";
	if(bb_math_Abs2(t_MaxWidth)<bb_math_Abs2(t_twidth)){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<238>";
		t_MaxWidth=t_twidth-dbg_object(this.m_Kerning()).f_x;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<239>";
	pop_err();
	return t_MaxWidth;
}
bb_bitmapfont_BitmapFont.prototype.m_GetTxtWidth2=function(t_text){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<205>";
	var t_=this.m_GetTxtWidth(t_text,1,t_text.length);
	pop_err();
	return t_;
}
bb_bitmapfont_BitmapFont.prototype.m_DrawCharsText=function(t_text,t_x,t_y,t_target,t_align,t_startPos){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<619>";
	var t_drx=t_x;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<619>";
	var t_dry=t_y;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<620>";
	var t_oldX=t_x;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<621>";
	var t_xOffset=0;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<624>";
	if(t_align!=1){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<625>";
		var t_lineSepPos=t_text.indexOf("\n",t_startPos);
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<626>";
		if(t_lineSepPos<0){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<626>";
			t_lineSepPos=t_text.length;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<627>";
		var t_=t_align;
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<628>";
		if(t_==2){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<628>";
			t_xOffset=((this.m_GetTxtWidth(t_text,t_startPos,t_lineSepPos)/2.0)|0);
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<629>";
			if(t_==3){
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<629>";
				t_xOffset=((this.m_GetTxtWidth(t_text,t_startPos,t_lineSepPos))|0);
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<633>";
	for(var t_i=t_startPos;t_i<=t_text.length;t_i=t_i+1){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<634>";
		var t_char=t_text.charCodeAt(t_i-1);
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<635>";
		if(t_char>=0 && t_char<=t_target.length){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<636>";
			if(t_char==10){
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<637>";
				t_dry+=dbg_object(dbg_object(dbg_object(dbg_array(this.f_faceChars,32)[dbg_index]).f_drawingMetrics).f_drawingSize).f_y+dbg_object(this.m_Kerning()).f_y;
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<638>";
				this.m_DrawCharsText(t_text,t_oldX,t_dry,t_target,t_align,t_i+1);
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<639>";
				pop_err();
				return 0;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<640>";
				if(dbg_array(t_target,t_char)[dbg_index]!=null){
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<641>";
					if(dbg_array(t_target,t_char)[dbg_index].m_CharImageLoaded()==false){
						err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<642>";
						dbg_array(t_target,t_char)[dbg_index].m_LoadCharImage();
					}
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<644>";
					if(dbg_object(dbg_array(t_target,t_char)[dbg_index]).f_image!=null){
						err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<645>";
						bb_graphics_DrawImage(dbg_object(dbg_array(t_target,t_char)[dbg_index]).f_image,t_drx-(t_xOffset),t_dry,0);
					}else{
						err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<646>";
						if(dbg_object(dbg_array(t_target,t_char)[dbg_index]).f_packedFontIndex>0){
							err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<647>";
							bb_graphics_DrawImageRect(dbg_array(this.f_packedImages,dbg_object(dbg_array(t_target,t_char)[dbg_index]).f_packedFontIndex)[dbg_index],(-t_xOffset)+t_drx+dbg_object(dbg_object(dbg_object(dbg_array(t_target,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_x,t_dry+dbg_object(dbg_object(dbg_object(dbg_array(t_target,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_y,((dbg_object(dbg_object(dbg_array(t_target,t_char)[dbg_index]).f_packedPosition).f_x)|0),((dbg_object(dbg_object(dbg_array(t_target,t_char)[dbg_index]).f_packedPosition).f_y)|0),((dbg_object(dbg_object(dbg_array(t_target,t_char)[dbg_index]).f_packedSize).f_x)|0),((dbg_object(dbg_object(dbg_array(t_target,t_char)[dbg_index]).f_packedSize).f_y)|0),0);
						}
					}
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<649>";
					t_drx+=dbg_object(dbg_object(dbg_array(this.f_faceChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingWidth+dbg_object(this.m_Kerning()).f_x;
				}
			}
		}
	}
	pop_err();
	return 0;
}
bb_bitmapfont_BitmapFont.prototype.m_DrawCharsText2=function(t_text,t_x,t_y,t_mode,t_align){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<606>";
	if(t_mode==1){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<607>";
		this.m_DrawCharsText(t_text,t_x,t_y,this.f_borderChars,t_align,1);
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<608>";
		if(t_mode==0){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<609>";
			this.m_DrawCharsText(t_text,t_x,t_y,this.f_faceChars,t_align,1);
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<611>";
			this.m_DrawCharsText(t_text,t_x,t_y,this.f_shadowChars,t_align,1);
		}
	}
	pop_err();
	return 0;
}
bb_bitmapfont_BitmapFont.prototype.m_DrawBorder=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<79>";
	pop_err();
	return this.f__drawBorder;
}
bb_bitmapfont_BitmapFont.prototype.m_DrawBorder2=function(t_value){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<83>";
	this.f__drawBorder=t_value;
	pop_err();
	return 0;
}
bb_bitmapfont_BitmapFont.prototype.m_DrawText=function(t_text,t_x,t_y,t_align){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<187>";
	if(this.m_DrawShadow()){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<187>";
		this.m_DrawCharsText2(t_text,t_x,t_y,2,t_align);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<188>";
	if(this.m_DrawBorder()){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<188>";
		this.m_DrawCharsText2(t_text,t_x,t_y,1,t_align);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<189>";
	this.m_DrawCharsText2(t_text,t_x,t_y,0,t_align);
	pop_err();
	return 0;
}
bb_bitmapfont_BitmapFont.prototype.m_DrawText2=function(t_text,t_x,t_y){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<197>";
	this.m_DrawText(t_text,t_x,t_y,1);
	pop_err();
	return 0;
}
bb_bitmapfont_BitmapFont.prototype.m_GetTxtHeight=function(t_Text){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<250>";
	var t_count=0;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<251>";
	for(var t_i=0;t_i<t_Text.length;t_i=t_i+1){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<252>";
		if(t_Text.charCodeAt(t_i)==10){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<253>";
			t_count+=1;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<256>";
	var t_=(t_count)*(dbg_object(dbg_object(dbg_object(dbg_array(this.f_faceChars,32)[dbg_index]).f_drawingMetrics).f_drawingSize).f_y+dbg_object(this.m_Kerning()).f_y)+(this.m_GetFontHeight());
	pop_err();
	return t_;
}
bb_bitmapfont_BitmapFont.prototype.m_LoadPacked=function(t_info,t_fontName,t_dynamicLoad){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<542>";
	var t_header=t_info.slice(0,t_info.indexOf(",",0));
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<544>";
	var t_separator="";
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<545>";
	var t_=t_header;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<546>";
	if(t_=="P1"){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<547>";
		t_separator=".";
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<548>";
		if(t_=="P1.01"){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<549>";
			t_separator="_P_";
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<551>";
	t_info=t_info.slice(t_info.indexOf(",",0)+1);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<552>";
	this.f_borderChars=new_object_array(65536);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<553>";
	this.f_faceChars=new_object_array(65536);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<554>";
	this.f_shadowChars=new_object_array(65536);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<555>";
	this.f_packedImages=new_object_array(256);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<556>";
	var t_maxPacked=0;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<557>";
	var t_maxChar=0;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<559>";
	var t_prefixName=t_fontName;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<560>";
	if(string_endswith(t_prefixName.toLowerCase(),".txt")){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<560>";
		t_prefixName=t_prefixName.slice(0,-4);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<562>";
	var t_charList=t_info.split(";");
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<563>";
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<563>";
	var t_2=t_charList;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<563>";
	var t_3=0;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<563>";
	while(t_3<t_2.length){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<563>";
		var t_chr=dbg_array(t_2,t_3)[dbg_index];
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<563>";
		t_3=t_3+1;
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<565>";
		var t_chrdata=t_chr.split(",");
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<566>";
		if(t_chrdata.length<2){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<566>";
			break;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<567>";
		var t_char=null;
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<568>";
		var t_charIndex=parseInt((dbg_array(t_chrdata,0)[dbg_index]),10);
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<569>";
		if(t_maxChar<t_charIndex){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<569>";
			t_maxChar=t_charIndex;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<571>";
		var t_4=dbg_array(t_chrdata,1)[dbg_index];
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<572>";
		if(t_4=="B"){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<573>";
			dbg_array(this.f_borderChars,t_charIndex)[dbg_index]=bb_bitmapchar_BitMapChar_new.call(new bb_bitmapchar_BitMapChar)
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<574>";
			t_char=dbg_array(this.f_borderChars,t_charIndex)[dbg_index];
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<575>";
			if(t_4=="F"){
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<576>";
				dbg_array(this.f_faceChars,t_charIndex)[dbg_index]=bb_bitmapchar_BitMapChar_new.call(new bb_bitmapchar_BitMapChar)
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<577>";
				t_char=dbg_array(this.f_faceChars,t_charIndex)[dbg_index];
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<578>";
				if(t_4=="S"){
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<579>";
					dbg_array(this.f_shadowChars,t_charIndex)[dbg_index]=bb_bitmapchar_BitMapChar_new.call(new bb_bitmapchar_BitMapChar)
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<580>";
					t_char=dbg_array(this.f_shadowChars,t_charIndex)[dbg_index];
				}
			}
		}
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<582>";
		dbg_object(t_char).f_packedFontIndex=parseInt((dbg_array(t_chrdata,2)[dbg_index]),10);
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<583>";
		if(dbg_array(this.f_packedImages,dbg_object(t_char).f_packedFontIndex)[dbg_index]==null){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<584>";
			dbg_array(this.f_packedImages,dbg_object(t_char).f_packedFontIndex)[dbg_index]=bb_graphics_LoadImage(t_prefixName+t_separator+String(dbg_object(t_char).f_packedFontIndex)+".png",1,bb_graphics_Image_DefaultFlags)
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<585>";
			if(t_maxPacked<dbg_object(t_char).f_packedFontIndex){
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<585>";
				t_maxPacked=dbg_object(t_char).f_packedFontIndex;
			}
		}
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<587>";
		dbg_object(dbg_object(t_char).f_packedPosition).f_x=(parseInt((dbg_array(t_chrdata,3)[dbg_index]),10));
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<588>";
		dbg_object(dbg_object(t_char).f_packedPosition).f_y=(parseInt((dbg_array(t_chrdata,4)[dbg_index]),10));
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<589>";
		dbg_object(dbg_object(t_char).f_packedSize).f_x=(parseInt((dbg_array(t_chrdata,5)[dbg_index]),10));
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<590>";
		dbg_object(dbg_object(t_char).f_packedSize).f_y=(parseInt((dbg_array(t_chrdata,6)[dbg_index]),10));
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<591>";
		dbg_object(dbg_object(dbg_object(t_char).f_drawingMetrics).f_drawingOffset).f_x=(parseInt((dbg_array(t_chrdata,8)[dbg_index]),10));
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<592>";
		dbg_object(dbg_object(dbg_object(t_char).f_drawingMetrics).f_drawingOffset).f_y=(parseInt((dbg_array(t_chrdata,9)[dbg_index]),10));
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<593>";
		dbg_object(dbg_object(dbg_object(t_char).f_drawingMetrics).f_drawingSize).f_x=(parseInt((dbg_array(t_chrdata,10)[dbg_index]),10));
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<594>";
		dbg_object(dbg_object(dbg_object(t_char).f_drawingMetrics).f_drawingSize).f_y=(parseInt((dbg_array(t_chrdata,11)[dbg_index]),10));
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<595>";
		dbg_object(dbg_object(t_char).f_drawingMetrics).f_drawingWidth=(parseInt((dbg_array(t_chrdata,12)[dbg_index]),10));
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<598>";
	this.f_borderChars=this.f_borderChars.slice(0,t_maxChar+1);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<599>";
	this.f_faceChars=this.f_faceChars.slice(0,t_maxChar+1);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<600>";
	this.f_shadowChars=this.f_shadowChars.slice(0,t_maxChar+1);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<601>";
	this.f_packedImages=this.f_packedImages.slice(0,t_maxPacked+1);
	pop_err();
	return 0;
}
bb_bitmapfont_BitmapFont.prototype.m_LoadFontData=function(t_Info,t_fontName,t_dynamicLoad){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<440>";
	if(string_startswith(t_Info,"P1")){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<441>";
		this.m_LoadPacked(t_Info,t_fontName,t_dynamicLoad);
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<442>";
		pop_err();
		return 0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<444>";
	var t_tokenStream=t_Info.split(",");
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<445>";
	var t_index=0;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<446>";
	this.f_borderChars=new_object_array(65536);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<447>";
	this.f_faceChars=new_object_array(65536);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<448>";
	this.f_shadowChars=new_object_array(65536);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<450>";
	var t_prefixName=t_fontName;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<451>";
	if(string_endswith(t_prefixName.toLowerCase(),".txt")){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<451>";
		t_prefixName=t_prefixName.slice(0,-4);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<453>";
	var t_char=0;
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<454>";
	while(t_index<t_tokenStream.length){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<456>";
		var t_strChar=dbg_array(t_tokenStream,t_index)[dbg_index];
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<457>";
		if(string_trim(t_strChar)==""){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<459>";
			t_index+=1;
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<460>";
			break;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<462>";
		t_char=parseInt((t_strChar),10);
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<464>";
		t_index+=1;
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<466>";
		var t_kind=dbg_array(t_tokenStream,t_index)[dbg_index];
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<468>";
		t_index+=1;
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<470>";
		var t_=t_kind;
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<471>";
		if(t_=="{BR"){
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<472>";
			t_index+=3;
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<473>";
			dbg_array(this.f_borderChars,t_char)[dbg_index]=bb_bitmapchar_BitMapChar_new.call(new bb_bitmapchar_BitMapChar)
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<474>";
			dbg_object(dbg_object(dbg_object(dbg_array(this.f_borderChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_x=(parseInt((dbg_array(t_tokenStream,t_index)[dbg_index]),10));
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<475>";
			dbg_object(dbg_object(dbg_object(dbg_array(this.f_borderChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_y=(parseInt((dbg_array(t_tokenStream,t_index+1)[dbg_index]),10));
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<476>";
			dbg_object(dbg_object(dbg_object(dbg_array(this.f_borderChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingSize).f_x=(parseInt((dbg_array(t_tokenStream,t_index+2)[dbg_index]),10));
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<477>";
			dbg_object(dbg_object(dbg_object(dbg_array(this.f_borderChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingSize).f_y=(parseInt((dbg_array(t_tokenStream,t_index+3)[dbg_index]),10));
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<478>";
			dbg_object(dbg_object(dbg_array(this.f_borderChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingWidth=(parseInt((dbg_array(t_tokenStream,t_index+4)[dbg_index]),10));
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<479>";
			if(t_dynamicLoad==false){
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<480>";
				dbg_object(dbg_array(this.f_borderChars,t_char)[dbg_index]).f_image=bb_graphics_LoadImage(t_prefixName+"_BORDER_"+String(t_char)+".png",1,bb_graphics_Image_DefaultFlags);
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<481>";
				dbg_object(dbg_array(this.f_borderChars,t_char)[dbg_index]).f_image.m_SetHandle(-dbg_object(dbg_object(dbg_object(dbg_array(this.f_borderChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_x,-dbg_object(dbg_object(dbg_object(dbg_array(this.f_borderChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_y);
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<483>";
				dbg_array(this.f_borderChars,t_char)[dbg_index].m_SetImageResourceName(t_prefixName+"_BORDER_"+String(t_char)+".png");
			}
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<485>";
			t_index+=5;
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<486>";
			t_index+=1;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<488>";
			if(t_=="{SH"){
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<489>";
				t_index+=3;
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<490>";
				dbg_array(this.f_shadowChars,t_char)[dbg_index]=bb_bitmapchar_BitMapChar_new.call(new bb_bitmapchar_BitMapChar)
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<491>";
				dbg_object(dbg_object(dbg_object(dbg_array(this.f_shadowChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_x=(parseInt((dbg_array(t_tokenStream,t_index)[dbg_index]),10));
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<492>";
				dbg_object(dbg_object(dbg_object(dbg_array(this.f_shadowChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_y=(parseInt((dbg_array(t_tokenStream,t_index+1)[dbg_index]),10));
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<493>";
				dbg_object(dbg_object(dbg_object(dbg_array(this.f_shadowChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingSize).f_x=(parseInt((dbg_array(t_tokenStream,t_index+2)[dbg_index]),10));
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<494>";
				dbg_object(dbg_object(dbg_object(dbg_array(this.f_shadowChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingSize).f_y=(parseInt((dbg_array(t_tokenStream,t_index+3)[dbg_index]),10));
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<495>";
				dbg_object(dbg_object(dbg_array(this.f_shadowChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingWidth=(parseInt((dbg_array(t_tokenStream,t_index+4)[dbg_index]),10));
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<496>";
				var t_filename=t_prefixName+"_SHADOW_"+String(t_char)+".png";
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<497>";
				if(t_dynamicLoad==false){
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<498>";
					dbg_object(dbg_array(this.f_shadowChars,t_char)[dbg_index]).f_image=bb_graphics_LoadImage(t_filename,1,bb_graphics_Image_DefaultFlags);
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<499>";
					dbg_object(dbg_array(this.f_shadowChars,t_char)[dbg_index]).f_image.m_SetHandle(-dbg_object(dbg_object(dbg_object(dbg_array(this.f_shadowChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_x,-dbg_object(dbg_object(dbg_object(dbg_array(this.f_shadowChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_y);
				}else{
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<501>";
					dbg_array(this.f_shadowChars,t_char)[dbg_index].m_SetImageResourceName(t_filename);
				}
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<508>";
				t_index+=5;
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<509>";
				t_index+=1;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<511>";
				if(t_=="{FC"){
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<512>";
					t_index+=3;
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<513>";
					dbg_array(this.f_faceChars,t_char)[dbg_index]=bb_bitmapchar_BitMapChar_new.call(new bb_bitmapchar_BitMapChar)
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<514>";
					dbg_object(dbg_object(dbg_object(dbg_array(this.f_faceChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_x=(parseInt((dbg_array(t_tokenStream,t_index)[dbg_index]),10));
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<515>";
					dbg_object(dbg_object(dbg_object(dbg_array(this.f_faceChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_y=(parseInt((dbg_array(t_tokenStream,t_index+1)[dbg_index]),10));
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<516>";
					dbg_object(dbg_object(dbg_object(dbg_array(this.f_faceChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingSize).f_x=(parseInt((dbg_array(t_tokenStream,t_index+2)[dbg_index]),10));
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<517>";
					dbg_object(dbg_object(dbg_object(dbg_array(this.f_faceChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingSize).f_y=(parseInt((dbg_array(t_tokenStream,t_index+3)[dbg_index]),10));
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<518>";
					dbg_object(dbg_object(dbg_array(this.f_faceChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingWidth=(parseInt((dbg_array(t_tokenStream,t_index+4)[dbg_index]),10));
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<519>";
					if(t_dynamicLoad==false){
						err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<520>";
						dbg_object(dbg_array(this.f_faceChars,t_char)[dbg_index]).f_image=bb_graphics_LoadImage(t_prefixName+"_"+String(t_char)+".png",1,bb_graphics_Image_DefaultFlags);
						err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<521>";
						dbg_object(dbg_array(this.f_faceChars,t_char)[dbg_index]).f_image.m_SetHandle(-dbg_object(dbg_object(dbg_object(dbg_array(this.f_faceChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_x,-dbg_object(dbg_object(dbg_object(dbg_array(this.f_faceChars,t_char)[dbg_index]).f_drawingMetrics).f_drawingOffset).f_y);
					}else{
						err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<523>";
						dbg_array(this.f_faceChars,t_char)[dbg_index].m_SetImageResourceName(t_prefixName+"_"+String(t_char)+".png");
					}
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<525>";
					t_index+=5;
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<526>";
					t_index+=1;
				}else{
					err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<529>";
					print("Error loading font! Char = "+String(t_char));
				}
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<533>";
	this.f_borderChars=this.f_borderChars.slice(0,t_char+1);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<534>";
	this.f_faceChars=this.f_faceChars.slice(0,t_char+1);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<535>";
	this.f_shadowChars=this.f_shadowChars.slice(0,t_char+1);
	pop_err();
	return 0;
}
function bb_bitmapfont_BitmapFont_new(t_fontDescriptionFilePath,t_dynamicLoad){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<45>";
	var t_text=bb_app_LoadString(t_fontDescriptionFilePath);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<46>";
	if(t_text==""){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<46>";
		print("FONT "+t_fontDescriptionFilePath+" WAS NOT FOUND!!!");
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<47>";
	this.m_LoadFontData(t_text,t_fontDescriptionFilePath,t_dynamicLoad);
	pop_err();
	return this;
}
function bb_bitmapfont_BitmapFont_new2(t_fontDescriptionFilePath){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<55>";
	var t_text=bb_app_LoadString(t_fontDescriptionFilePath);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<56>";
	if(t_text==""){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<56>";
		print("FONT "+t_fontDescriptionFilePath+" WAS NOT FOUND!!!");
	}
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<57>";
	this.m_LoadFontData(t_text,t_fontDescriptionFilePath,true);
	pop_err();
	return this;
}
function bb_bitmapfont_BitmapFont_new3(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<28>";
	pop_err();
	return this;
}
function bb_bitmapfont_BitmapFont_Load(t_fontName,t_dynamicLoad){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<35>";
	var t_font=bb_bitmapfont_BitmapFont_new.call(new bb_bitmapfont_BitmapFont,t_fontName,t_dynamicLoad);
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<36>";
	pop_err();
	return t_font;
}
var bb_challengergui_CHGUI_TitleFont;
function bb_bitmapchar_BitMapChar(){
	Object.call(this);
	this.f_drawingMetrics=bb_bitmapcharmetrics_BitMapCharMetrics_new.call(new bb_bitmapcharmetrics_BitMapCharMetrics);
	this.f_image=null;
	this.f_imageResourceName="";
	this.f_imageResourceNameBackup="";
	this.f_packedFontIndex=0;
	this.f_packedPosition=bb_drawingpoint_DrawingPoint_new.call(new bb_drawingpoint_DrawingPoint);
	this.f_packedSize=bb_drawingpoint_DrawingPoint_new.call(new bb_drawingpoint_DrawingPoint);
}
bb_bitmapchar_BitMapChar.prototype.m_CharImageLoaded=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<42>";
	if(this.f_image==null && this.f_imageResourceName!=""){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<42>";
		pop_err();
		return false;
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<42>";
		pop_err();
		return true;
	}
}
bb_bitmapchar_BitMapChar.prototype.m_LoadCharImage=function(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<30>";
	if(this.m_CharImageLoaded()==false){
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<31>";
		this.f_image=bb_graphics_LoadImage(this.f_imageResourceName,1,bb_graphics_Image_DefaultFlags);
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<32>";
		this.f_image.m_SetHandle(-dbg_object(dbg_object(dbg_object(this).f_drawingMetrics).f_drawingOffset).f_x,-dbg_object(dbg_object(dbg_object(this).f_drawingMetrics).f_drawingOffset).f_y);
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<33>";
		this.f_imageResourceNameBackup=this.f_imageResourceName;
		err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<34>";
		this.f_imageResourceName="";
	}
	pop_err();
	return 0;
}
function bb_bitmapchar_BitMapChar_new(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<15>";
	pop_err();
	return this;
}
bb_bitmapchar_BitMapChar.prototype.m_SetImageResourceName=function(t_value){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<46>";
	this.f_imageResourceName=t_value;
	pop_err();
	return 0;
}
function bb_bitmapcharmetrics_BitMapCharMetrics(){
	Object.call(this);
	this.f_drawingSize=bb_drawingpoint_DrawingPoint_new.call(new bb_drawingpoint_DrawingPoint);
	this.f_drawingWidth=.0;
	this.f_drawingOffset=bb_drawingpoint_DrawingPoint_new.call(new bb_drawingpoint_DrawingPoint);
}
function bb_bitmapcharmetrics_BitMapCharMetrics_new(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapcharmetrics.monkey<12>";
	pop_err();
	return this;
}
function bb_drawingpoint_DrawingPoint(){
	Object.call(this);
	this.f_y=.0;
	this.f_x=.0;
}
function bb_drawingpoint_DrawingPoint_new(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/fontmachine/drawingpoint.monkey<8>";
	pop_err();
	return this;
}
function bb_challengergui_CHGUI_TextHeight(t_Fnt,t_Text){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3871>";
	var t_Split=t_Text.split("\n");
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3872>";
	var t_H=(t_Fnt.m_GetFontHeight());
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3873>";
	var t_Height=0.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3874>";
	for(var t_N=0;t_N<=t_Split.length-1;t_N=t_N+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3875>";
		t_Height=t_Height+t_H;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3877>";
	pop_err();
	return t_Height;
}
function bb_edrawmode_eDrawMode(){
	Object.call(this);
}
function bb_edrawalign_eDrawAlign(){
	Object.call(this);
}
function bb_math_Abs(t_x){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<46>";
	if(t_x>=0){
		err_info="C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<46>";
		pop_err();
		return t_x;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<47>";
	var t_=-t_x;
	pop_err();
	return t_;
}
function bb_math_Abs2(t_x){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<73>";
	if(t_x>=0.0){
		err_info="C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<73>";
		pop_err();
		return t_x;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<74>";
	var t_=-t_x;
	pop_err();
	return t_;
}
function bb_graphics_DrawImage(t_image,t_x,t_y,t_frame){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<417>";
	bb_graphics_DebugRenderDevice();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<419>";
	var t_f=dbg_array(dbg_object(t_image).f_frames,t_frame)[dbg_index];
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<421>";
	if((dbg_object(bb_graphics_context).f_tformed)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<422>";
		bb_graphics_PushMatrix();
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<424>";
		bb_graphics_Translate(t_x-dbg_object(t_image).f_tx,t_y-dbg_object(t_image).f_ty);
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<426>";
		bb_graphics_context.m_Validate();
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<428>";
		if((dbg_object(t_image).f_flags&65536)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<429>";
			bb_graphics_renderDevice.DrawSurface(dbg_object(t_image).f_surface,0.0,0.0);
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<431>";
			bb_graphics_renderDevice.DrawSurface2(dbg_object(t_image).f_surface,0.0,0.0,dbg_object(t_f).f_x,dbg_object(t_f).f_y,dbg_object(t_image).f_width,dbg_object(t_image).f_height);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<434>";
		bb_graphics_PopMatrix();
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<436>";
		bb_graphics_context.m_Validate();
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<438>";
		if((dbg_object(t_image).f_flags&65536)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<439>";
			bb_graphics_renderDevice.DrawSurface(dbg_object(t_image).f_surface,t_x-dbg_object(t_image).f_tx,t_y-dbg_object(t_image).f_ty);
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<441>";
			bb_graphics_renderDevice.DrawSurface2(dbg_object(t_image).f_surface,t_x-dbg_object(t_image).f_tx,t_y-dbg_object(t_image).f_ty,dbg_object(t_f).f_x,dbg_object(t_f).f_y,dbg_object(t_image).f_width,dbg_object(t_image).f_height);
		}
	}
	pop_err();
	return 0;
}
function bb_graphics_DrawImage2(t_image,t_x,t_y,t_rotation,t_scaleX,t_scaleY,t_frame){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<448>";
	bb_graphics_DebugRenderDevice();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<450>";
	var t_f=dbg_array(dbg_object(t_image).f_frames,t_frame)[dbg_index];
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<452>";
	bb_graphics_PushMatrix();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<454>";
	bb_graphics_Translate(t_x,t_y);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<455>";
	bb_graphics_Rotate(t_rotation);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<456>";
	bb_graphics_Scale(t_scaleX,t_scaleY);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<458>";
	bb_graphics_Translate(-dbg_object(t_image).f_tx,-dbg_object(t_image).f_ty);
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<460>";
	bb_graphics_context.m_Validate();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<462>";
	if((dbg_object(t_image).f_flags&65536)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<463>";
		bb_graphics_renderDevice.DrawSurface(dbg_object(t_image).f_surface,0.0,0.0);
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<465>";
		bb_graphics_renderDevice.DrawSurface2(dbg_object(t_image).f_surface,0.0,0.0,dbg_object(t_f).f_x,dbg_object(t_f).f_y,dbg_object(t_image).f_width,dbg_object(t_image).f_height);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<468>";
	bb_graphics_PopMatrix();
	pop_err();
	return 0;
}
var bb_challengergui_CHGUI_Font;
function bb_challengergui_CHGUI_DrawWindow(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2228>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2229>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2230>";
	var t_W=dbg_object(t_N).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2231>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2232>";
	var t_TH=bb_challengergui_CHGUI_TitleHeight;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2233>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2234>";
	if(bb_challengergui_CHGUI_LockedWIndow==t_N){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2234>";
		t_Active=1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2236>";
	if(t_N!=bb_challengergui_CHGUI_Canvas){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2238>";
		if(((dbg_object(t_N).f_Shadow)!=0) && ((bb_challengergui_CHGUI_Shadow)!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2239>";
			if((dbg_object(t_N).f_Minimised)!=0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2241>";
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y-10.0,0,0,20,20,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2243>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+10.0,t_Y-10.0,20,0,10,10,0.0,(t_W-20.0)/10.0,1.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2245>";
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W-10.0,t_Y-10.0,30,0,20,20,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2247>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y+10.0,0,20,10,10,0.0,1.0,(t_TH-20.0)/10.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2249>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y+10.0,40,20,10,10,0.0,1.0,(t_TH-20.0)/10.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2251>";
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y+t_TH-10.0,0,30,20,20,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2253>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+10.0,t_Y+t_TH,20,40,10,10,0.0,(t_W-20.0)/10.0,1.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2255>";
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W-10.0,t_Y+t_TH-10.0,30,30,20,20,0);
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2258>";
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y-10.0,0,0,20,20,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2260>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+10.0,t_Y-10.0,20,0,10,10,0.0,(t_W-20.0)/10.0,1.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2262>";
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W-10.0,t_Y-10.0,30,0,20,20,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2264>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y+10.0,0,20,10,10,0.0,1.0,(t_H-20.0)/10.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2266>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y+10.0,40,20,10,10,0.0,1.0,(t_H-20.0)/10.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2268>";
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y+t_H-10.0,0,30,20,20,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2270>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+10.0,t_Y+t_H,20,40,10,10,0.0,(t_W-20.0)/10.0,1.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2272>";
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W-10.0,t_Y+t_H-10.0,30,30,20,20,0);
			}
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2276>";
		var t_XOf=10.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2277>";
		var t_YOf=10.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2278>";
		if(bb_challengergui_CHGUI_RealActive(t_N)==0 && bb_challengergui_CHGUI_LockedWIndow!=t_N){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2278>";
			t_YOf=t_YOf+30.0;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2281>";
		if(dbg_object(t_N).f_Text!=""){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2283>";
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y,((t_XOf)|0),((t_YOf)|0),10,10,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2285>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y,((t_XOf+10.0)|0),((t_YOf)|0),50,10,0.0,(t_W-20.0)/50.0,1.0,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2287>";
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y,((t_XOf+60.0)|0),((t_YOf)|0),10,10,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2289>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+10.0,((t_XOf)|0),((t_YOf+10.0)|0),10,10,0.0,1.0,(t_TH-20.0)/10.0,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2291>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+10.0,((t_XOf+10.0)|0),((t_YOf+10.0)|0),50,10,0.0,(t_W-20.0)/50.0,(t_TH-20.0)/10.0,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2293>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+10.0,((t_XOf+60.0)|0),((t_YOf+10.0)|0),10,10,0.0,1.0,(t_TH-20.0)/10.0,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2295>";
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y+t_TH-10.0,((t_XOf)|0),((t_YOf+20.0)|0),10,10,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2297>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+t_TH-10.0,((t_XOf+10.0)|0),((t_YOf+20.0)|0),50,10,0.0,(t_W-20.0)/50.0,1.0,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2299>";
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+t_TH-10.0,((t_XOf+60.0)|0),((t_YOf+20.0)|0),10,10,0);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2302>";
		if(dbg_object(t_N).f_Minimised==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2303>";
			if(dbg_object(t_N).f_Text==""){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2305>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,10,70,10,10,0.0,1.0,1.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2307>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y,20,70,50,10,0.0,(t_W-20.0)/50.0,1.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2309>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y,70,70,10,10,0.0,1.0,1.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2311>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+10.0,10,80,10,40,0.0,1.0,(t_H-20.0)/40.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2313>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+10.0,70,80,10,40,0.0,1.0,(t_H-20.0)/40.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2315>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+10.0,20,80,50,40,0.0,(t_W-20.0)/50.0,(t_H-20.0)/40.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2317>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0,10,120,10,10,0.0,1.0,1.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2319>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+t_H-10.0,20,120,50,10,0.0,(t_W-20.0)/50.0,1.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2321>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+t_H-10.0,70,120,10,10,0.0,1.0,1.0,0);
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2324>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_TH,10,80,10,40,0.0,1.0,(t_H-t_TH-10.0)/40.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2326>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+t_TH,70,80,10,40,0.0,1.0,(t_H-t_TH-10.0)/40.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2328>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+t_TH,20,80,50,40,0.0,(t_W-20.0)/50.0,(t_H-t_TH-10.0)/40.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2330>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0,10,120,10,10,0.0,1.0,1.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2332>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+t_H-10.0,20,120,50,10,0.0,(t_W-20.0)/50.0,1.0,0);
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2334>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+t_H-10.0,70,120,10,10,0.0,1.0,1.0,0);
			}
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2339>";
		if(dbg_object(t_N).f_Close==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2340>";
			if(((dbg_object(t_N).f_CloseOver)!=0) && dbg_object(t_N).f_CloseDown==0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2341>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_TH/2.5-10.0,t_Y+(t_TH-t_TH/2.5)/2.0,105,10,15,15,0.0,t_TH/2.5/15.0,t_TH/2.5/15.0,0);
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2342>";
				if((dbg_object(t_N).f_CloseDown)!=0){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2343>";
					bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_TH/2.5-10.0,t_Y+(t_TH-t_TH/2.5)/2.0,120,10,15,15,0.0,t_TH/2.5/15.0,t_TH/2.5/15.0,0);
				}else{
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2344>";
					if((t_Active)!=0){
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2345>";
						bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_TH/2.5-10.0,t_Y+(t_TH-t_TH/2.5)/2.0,90,10,15,15,0.0,t_TH/2.5/15.0,t_TH/2.5/15.0,0);
					}else{
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2347>";
						bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_TH/2.5-10.0,t_Y+(t_TH-t_TH/2.5)/2.0,135,10,15,15,0.0,t_TH/2.5/15.0,t_TH/2.5/15.0,0);
					}
				}
			}
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2352>";
		if(dbg_object(t_N).f_Minimise==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2353>";
			if(((dbg_object(t_N).f_MinimiseOver)!=0) && dbg_object(t_N).f_MinimiseDown==0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2354>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-(t_TH/2.5+t_TH/2.5)-t_TH/1.5,t_Y+(t_TH-t_TH/2.5)/2.0,105,25,15,15,0.0,t_TH/2.5/15.0,t_TH/2.5/15.0,0);
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2355>";
				if((dbg_object(t_N).f_MinimiseDown)!=0){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2356>";
					bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-(t_TH/2.5+t_TH/2.5)-t_TH/1.5,t_Y+(t_TH-t_TH/2.5)/2.0,120,25,15,15,0.0,t_TH/2.5/15.0,t_TH/2.5/15.0,0);
				}else{
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2357>";
					if((t_Active)!=0){
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2358>";
						bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-(t_TH/2.5+t_TH/2.5)-t_TH/1.5,t_Y+(t_TH-t_TH/2.5)/2.0,90,25,15,15,0.0,t_TH/2.5/15.0,t_TH/2.5/15.0,0);
					}else{
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2360>";
						bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-(t_TH/2.5+t_TH/2.5)-t_TH/1.5,t_Y+(t_TH-t_TH/2.5)/2.0,135,25,15,15,0.0,t_TH/2.5/15.0,t_TH/2.5/15.0,0);
					}
				}
			}
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2369>";
		var t_XOff=(t_TH-t_TH/2.0)/2.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2370>";
		var t_YOff=t_TH-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_TitleFont,dbg_object(t_N).f_Text);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2371>";
		bb_graphics_SetAlpha(0.25);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2372>";
		if((t_Active)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2372>";
			bb_graphics_SetAlpha(1.0);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2373>";
		bb_challengergui_CHGUI_TitleFont.m_DrawText2(dbg_object(t_N).f_Text,t_X+t_XOff,t_Y+t_YOff/2.0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2374>";
		bb_graphics_SetAlpha(1.0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2377>";
	if((dbg_object(t_N).f_HasMenu)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2378>";
		if(t_N!=bb_challengergui_CHGUI_Canvas){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2379>";
			if((bb_challengergui_CHGUI_Shadow)!=0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2379>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X,t_Y+t_TH+(dbg_object(t_N).f_MenuHeight),20,40,10,10,0.0,t_W/10.0,1.0,0);
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2380>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+1.0,t_Y+bb_challengergui_CHGUI_TitleHeight,100,90,40,10,0.0,(t_W-2.0)/40.0,(10.0+bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text)-10.0)/10.0,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2381>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+1.0,t_Y+bb_challengergui_CHGUI_TitleHeight+(dbg_object(t_N).f_MenuHeight-10),100,100,40,10,0.0,(t_W-2.0)/40.0,1.0,0);
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2383>";
			if((bb_challengergui_CHGUI_Shadow)!=0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2383>";
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X,t_Y+(dbg_object(t_N).f_MenuHeight),20,40,10,10,0.0,t_W/10.0,1.0,0);
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2384>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+1.0,t_Y,100,90,40,10,0.0,(t_W-2.0)/40.0,(10.0+bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text)-10.0)/10.0,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2385>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+1.0,t_Y+(dbg_object(t_N).f_MenuHeight-10),100,100,40,10,0.0,(t_W-2.0)/40.0,1.0,0);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2390>";
	if(((dbg_object(t_N).f_Tabbed)!=0) && dbg_object(t_N).f_Minimised==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2391>";
		var t_YY=((t_Y+(dbg_object(t_N).f_MenuHeight))|0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2392>";
		if(dbg_object(t_N).f_Text!=""){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2392>";
			t_YY=(((t_YY)+bb_challengergui_CHGUI_TitleHeight)|0);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2393>";
		var t_Height=(dbg_object(t_N).f_TabHeight+5);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2396>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+1.0,(t_YY),10,140,10,10,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2398>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-11.0,(t_YY),60,140,10,10,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2400>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+11.0,(t_YY),20,140,40,10,0.0,(t_W-22.0)/40.0,1.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2402>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+1.0,(t_YY+10),10,150,10,10,0.0,1.0,(t_Height-10.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2404>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-11.0,(t_YY+10),60,150,10,10,0.0,1.0,(t_Height-10.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2406>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+11.0,(t_YY+10),20,150,40,10,0.0,(dbg_object(t_N).f_W-22.0)/40.0,(t_Height-10.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2408>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+1.0,(t_YY)+t_Height-10.0,10,160,10,10,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2410>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-11.0,(t_YY)+t_Height-10.0,60,160,10,10,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2412>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+11.0,(t_YY)+t_Height-10.0,20,160,40,10,0.0,(t_W-22.0)/40.0,1.0,0);
	}
	pop_err();
	return 0;
}
var bb_challengergui_CHGUI_KeyboardButtons;
var bb_challengergui_CHGUI_ShiftHold;
function bb_challengergui_CHGUI_DrawButton(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2418>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2419>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2420>";
	var t_W=dbg_object(t_N).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2421>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2422>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2423>";
	var t_State=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2424>";
	if((dbg_object(t_N).f_Over)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2424>";
		t_State=40;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2425>";
	if((dbg_object(t_N).f_Down)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2425>";
		t_State=80;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2426>";
	if(t_N==dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index] && ((bb_challengergui_CHGUI_ShiftHold)!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2426>";
		t_State=80;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2427>";
	if(t_Active==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2427>";
		t_State=120;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2432>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y,160,10+t_State,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2434>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y,170,10+t_State,40,10,0.0,(t_W-20.0)/40.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2436>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y,210,10+t_State,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2438>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+10.0,160,20+t_State,10,20,0.0,1.0,(t_H-20.0)/20.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2440>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+10.0,210,20+t_State,10,20,0.0,1.0,(t_H-20.0)/20.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2442>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+10.0,170,20+t_State,40,20,0.0,(t_W-20.0)/40.0,(t_H-20.0)/20.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2444>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0,160,40+t_State,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2446>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+t_H-10.0,170,40+t_State,40,10,0.0,(t_W-20.0)/40.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2448>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+t_H-10.0,210,40+t_State,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2451>";
	var t_XOff=(t_W-bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text))/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2452>";
	var t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text))/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2453>";
	bb_graphics_SetAlpha(0.25);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2454>";
	if((t_Active)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2454>";
		bb_graphics_SetAlpha(1.0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2455>";
	bb_challengergui_CHGUI_Font.m_DrawText2(dbg_object(t_N).f_Text,t_X+t_XOff,t_Y+t_YOff);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2456>";
	bb_graphics_SetAlpha(1.0);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawImageButton(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2479>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2480>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2481>";
	var t_W=dbg_object(t_N).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2482>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2483>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2484>";
	var t_State=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2485>";
	if((dbg_object(t_N).f_Over)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2485>";
		t_State=((t_W)|0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2486>";
	if((dbg_object(t_N).f_Down)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2486>";
		t_State=((t_W*2.0)|0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2487>";
	if(t_Active==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2487>";
		t_State=((t_W*3.0)|0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2489>";
	bb_graphics_DrawImageRect(dbg_object(t_N).f_Img,t_X,t_Y,t_State,0,((t_W)|0),((t_H)|0),0);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawTickbox(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2495>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2496>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2497>";
	var t_W=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2498>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2499>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2500>";
	var t_OffX=230;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2501>";
	var t_OffY=10;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2502>";
	var t_OffW=20;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2503>";
	var t_OffH=20;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2504>";
	if((dbg_object(t_N).f_Over)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2504>";
		t_OffY=30;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2505>";
	if((dbg_object(t_N).f_Down)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2505>";
		t_OffY=50;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2506>";
	if(t_Active==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2506>";
		t_OffY=70;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2507>";
	if(dbg_object(t_N).f_Value>0.0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2507>";
		t_OffX=250;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2510>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,t_OffW,t_OffH,0.0,t_W/(t_OffW),t_H/(t_OffH),0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2512>";
	var t_XOff=t_W/4.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2513>";
	var t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text))/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2514>";
	bb_graphics_SetAlpha(0.25);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2515>";
	if((t_Active)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2515>";
		bb_graphics_SetAlpha(1.0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2516>";
	bb_challengergui_CHGUI_Font.m_DrawText2(dbg_object(t_N).f_Text,t_X+t_W+t_XOff,t_Y+t_YOff);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2517>";
	bb_graphics_SetAlpha(1.0);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawRadiobox(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2523>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2524>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2525>";
	var t_W=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2526>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2527>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2528>";
	var t_OffX=230;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2529>";
	var t_OffY=100;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2530>";
	var t_OffW=20;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2531>";
	var t_OffH=20;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2532>";
	if((dbg_object(t_N).f_Over)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2532>";
		t_OffY=120;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2533>";
	if((dbg_object(t_N).f_Down)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2533>";
		t_OffY=140;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2534>";
	if(t_Active==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2534>";
		t_OffY=160;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2535>";
	if(dbg_object(t_N).f_Value>0.0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2535>";
		t_OffX=250;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2538>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,t_OffW,t_OffH,0.0,t_W/(t_OffW),t_H/(t_OffH),0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2540>";
	var t_XOff=t_W/4.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2541>";
	var t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text))/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2542>";
	bb_graphics_SetAlpha(0.25);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2543>";
	if((t_Active)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2543>";
		bb_graphics_SetAlpha(1.0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2544>";
	bb_challengergui_CHGUI_Font.m_DrawText2(dbg_object(t_N).f_Text,t_X+t_W+t_XOff,t_Y+t_YOff);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2545>";
	bb_graphics_SetAlpha(1.0);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawListbox(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3074>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3075>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3076>";
	var t_W=dbg_object(t_N).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3077>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3078>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3079>";
	dbg_object(dbg_object(t_N).f_ListboxSlider).f_X=dbg_object(t_N).f_X+t_W-17.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3080>";
	dbg_object(dbg_object(t_N).f_ListboxSlider).f_Y=dbg_object(dbg_object(dbg_object(t_N).f_ListboxSlider).f_Parent).f_Y+dbg_object(t_N).f_Y-dbg_object(dbg_object(t_N).f_Parent).f_Y;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3081>";
	dbg_object(dbg_object(t_N).f_ListboxSlider).f_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3082>";
	var t_OffX=90;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3083>";
	var t_OffY=80;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3087>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3089>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y,t_OffX+10,t_OffY,40,10,0.0,(t_W-20.0)/40.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3091>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y,t_OffX+50,t_OffY,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3093>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+10.0,t_OffX,t_OffY+10,10,10,0.0,1.0,(t_H-20.0)/10.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3095>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+10.0,t_OffX+50,t_OffY+10,10,10,0.0,1.0,(t_H-20.0)/10.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3097>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+10.0,t_OffX+10,t_OffY+10,40,10,0.0,(t_W-20.0)/40.0,(t_H-20.0)/10.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3099>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0,t_OffX,t_OffY+20,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3101>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+t_H-10.0,t_OffX+10,t_OffY+20,40,10,0.0,(t_W-20.0)/40.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3103>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+t_H-10.0,t_OffX+50,t_OffY+20,10,10,0);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawListboxItem(t_N,t_C){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3109>";
	dbg_object(t_N).f_X=0.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3110>";
	dbg_object(t_N).f_Y=(t_C*dbg_object(dbg_object(t_N).f_Parent).f_ListHeight);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3111>";
	dbg_object(t_N).f_W=dbg_object(dbg_object(t_N).f_Parent).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3112>";
	dbg_object(t_N).f_H=(dbg_object(dbg_object(t_N).f_Parent).f_ListHeight);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3114>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3115>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3116>";
	var t_W=dbg_object(t_N).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3117>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3118>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3120>";
	if(dbg_object(t_N).f_Over==1 || ((dbg_object(t_N).f_Down)!=0) || dbg_object(dbg_object(t_N).f_Parent).f_SelectedListboxItem==t_N){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3122>";
		var t_OffX=90;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3123>";
		var t_OffY=110;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3124>";
		if(((dbg_object(t_N).f_Down)!=0) || dbg_object(dbg_object(t_N).f_Parent).f_SelectedListboxItem==t_N){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3125>";
			t_OffX=90;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3126>";
			t_OffY=140;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3130>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3132>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y,t_OffX+10,t_OffY,40,10,0.0,(t_W-20.0)/40.0,1.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3134>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y,t_OffX+50,t_OffY,10,10,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3136>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+10.0,t_OffX,t_OffY+10,10,10,0.0,1.0,(t_H-20.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3138>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+10.0,t_OffX+50,t_OffY+10,10,10,0.0,1.0,(t_H-20.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3140>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+10.0,t_OffX+10,t_OffY+10,40,10,0.0,(t_W-20.0)/40.0,(t_H-20.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3142>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0,t_OffX,t_OffY+20,10,10,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3144>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+t_H-10.0,t_OffX+10,t_OffY+20,40,10,0.0,(t_W-20.0)/40.0,1.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3146>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+t_H-10.0,t_OffX+50,t_OffY+20,10,10,0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3151>";
	var t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text))/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3152>";
	bb_graphics_SetAlpha(0.25);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3153>";
	if((t_Active)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3153>";
		bb_graphics_SetAlpha(1.0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3154>";
	bb_challengergui_CHGUI_Font.m_DrawText2(dbg_object(t_N).f_Text,t_X+10.0,t_Y+t_YOff);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3155>";
	bb_graphics_SetAlpha(1.0);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawHSlider(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2857>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2858>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2859>";
	var t_W=dbg_object(t_N).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2860>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2861>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2863>";
	var t_OffX=460;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2864>";
	var t_OffY=10;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2866>";
	if(t_Active==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2866>";
		t_OffY=70;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2869>";
	if(((dbg_object(t_N).f_MinusOver)!=0) && dbg_object(t_N).f_MinusDown==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2870>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY+20,20,20,0.0,t_H/20.0,t_H/20.0,0);
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2871>";
		if((dbg_object(t_N).f_MinusDown)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2872>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY+40,20,20,0.0,t_H/20.0,t_H/20.0,0);
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2874>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,20,20,0.0,t_H/20.0,t_H/20.0,0);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2878>";
	if(((dbg_object(t_N).f_PlusOver)!=0) && dbg_object(t_N).f_PlusDown==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2879>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_H,t_Y,t_OffX+60,t_OffY+20,20,20,0.0,t_H/20.0,t_H/20.0,0);
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2880>";
		if((dbg_object(t_N).f_PlusDown)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2881>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_H,t_Y,t_OffX+60,t_OffY+40,20,20,0.0,t_H/20.0,t_H/20.0,0);
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2883>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_H,t_Y,t_OffX+60,t_OffY,20,20,0.0,t_H/20.0,t_H/20.0,0);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2888>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_H,t_Y,t_OffX+20,t_OffY,40,20,0.0,(t_W-t_H-t_H)/40.0,t_H/20.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2891>";
	var t_XOF=475;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2892>";
	var t_YOF=100;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2893>";
	if(((dbg_object(t_N).f_SliderOver)!=0) && dbg_object(t_N).f_SliderDown==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2893>";
		t_YOF=t_YOF+20;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2894>";
	if((dbg_object(t_N).f_SliderDown)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2894>";
		t_YOF=t_YOF+40;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2895>";
	if(t_Active==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2895>";
		t_YOF=t_YOF+60;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2896>";
	var t_XPOS=t_X+t_H-5.0+(dbg_object(t_N).f_Value-dbg_object(t_N).f_Minimum)*dbg_object(t_N).f_Stp;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2898>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_XPOS,t_Y,t_XOF,t_YOF,5,20,0.0,1.0,t_H/20.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2899>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_XPOS+5.0,t_Y,t_XOF+5,t_YOF,40,20,0.0,(dbg_object(t_N).f_SWidth-10.0)/40.0,t_H/20.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2900>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_XPOS+dbg_object(t_N).f_SWidth-5.0,t_Y,t_XOF+45,t_YOF,5,20,0.0,1.0,t_H/20.0,0);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawVSlider(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2905>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2906>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2907>";
	var t_W=dbg_object(t_N).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2908>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2909>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2911>";
	var t_OffX=370;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2912>";
	var t_OffY=10;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2914>";
	if(t_Active==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2914>";
		t_OffX=430;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2917>";
	if(((dbg_object(t_N).f_MinusOver)!=0) && dbg_object(t_N).f_MinusDown==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2918>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX+20,t_OffY,20,20,0.0,t_W/20.0,t_W/20.0,0);
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2919>";
		if((dbg_object(t_N).f_MinusDown)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2920>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX+40,t_OffY,20,20,0.0,t_W/20.0,t_W/20.0,0);
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2922>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,20,20,0.0,t_W/20.0,t_W/20.0,0);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2926>";
	if(((dbg_object(t_N).f_PlusOver)!=0) && dbg_object(t_N).f_PlusDown==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2927>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-t_W,t_OffX+20,t_OffY+60,20,20,0.0,t_W/20.0,t_W/20.0,0);
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2928>";
		if((dbg_object(t_N).f_PlusDown)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2929>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-t_W,t_OffX+40,t_OffY+60,20,20,0.0,t_W/20.0,t_W/20.0,0);
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2931>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-t_W,t_OffX,t_OffY+60,20,20,0.0,t_W/20.0,t_W/20.0,0);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2936>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_W,t_OffX,t_OffY+20,20,40,0.0,t_W/20.0,(t_H-t_W-t_W)/40.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2939>";
	var t_XOF=370;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2940>";
	var t_YOF=100;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2941>";
	if(((dbg_object(t_N).f_SliderOver)!=0) && dbg_object(t_N).f_SliderDown==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2941>";
		t_XOF=t_XOF+20;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2942>";
	if((dbg_object(t_N).f_SliderDown)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2942>";
		t_XOF=t_XOF+40;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2943>";
	if(t_Active==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2943>";
		t_XOF=t_XOF+60;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2944>";
	var t_YPOS=t_Y+t_W-5.0+(dbg_object(t_N).f_Value-dbg_object(t_N).f_Minimum)*dbg_object(t_N).f_Stp;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2946>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_YPOS,t_XOF,t_YOF,20,5,0.0,t_W/20.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2947>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_YPOS+5.0,t_XOF,t_YOF+5,20,40,0.0,t_W/20.0,(dbg_object(t_N).f_SWidth-10.0)/40.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2948>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_YPOS+dbg_object(t_N).f_SWidth-5.0,t_XOF,t_YOF+45,20,5,0.0,t_W/20.0,1.0,0);
	pop_err();
	return 0;
}
var bb_challengergui_CHGUI_Cursor;
function bb_graphics_DrawLine(t_x1,t_y1,t_x2,t_y2){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<377>";
	bb_graphics_DebugRenderDevice();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<379>";
	bb_graphics_context.m_Validate();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<380>";
	bb_graphics_renderDevice.DrawLine(t_x1,t_y1,t_x2,t_y2);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawTextfield(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2953>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2954>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2955>";
	var t_W=dbg_object(t_N).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2956>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2957>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2959>";
	var t_OffX=280;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2960>";
	var t_OffY=10;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2961>";
	var t_OffH=40;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2963>";
	if((dbg_object(t_N).f_Over)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2963>";
		t_OffY=50;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2964>";
	if(((dbg_object(t_N).f_Down)!=0) && dbg_object(t_N).f_OnFocus==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2964>";
		t_OffY=90;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2965>";
	if(bb_challengergui_CHGUI_RealActive(t_N)==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2965>";
		t_OffY=130;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2967>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,0.0,1.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2969>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y,t_OffX+10,t_OffY,20,10,0.0,(t_W-20.0)/20.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2971>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y,t_OffX+30,t_OffY,10,10,0.0,1.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2973>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+10.0,t_OffX,t_OffY+10,10,20,0.0,1.0,(t_H-20.0)/20.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2975>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+10.0,t_OffX+30,t_OffY+10,10,20,0.0,1.0,(t_H-20.0)/20.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2977>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+10.0,t_OffX+10,t_OffY+10,20,20,0.0,(t_W-20.0)/20.0,(t_H-20.0)/20.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2979>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0,t_OffX,t_OffY+30,10,10,0.0,1.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2981>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+t_H-10.0,t_OffX+10,t_OffY+30,20,10,0.0,(t_W-20.0)/20.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2983>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+t_H-10.0,t_OffX+30,t_OffY+30,10,10,0.0,1.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2986>";
	var t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text))/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2987>";
	bb_graphics_SetAlpha(0.25);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2988>";
	if((t_Active)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2988>";
		bb_graphics_SetAlpha(1.0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2989>";
	bb_challengergui_CHGUI_Font.m_DrawText2(dbg_object(t_N).f_Text,t_X+5.0,t_Y+t_YOff);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2990>";
	bb_graphics_SetAlpha(1.0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2993>";
	if((dbg_object(t_N).f_OnFocus)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2994>";
		var t_Before=dbg_object(t_N).f_Text.slice(0,dbg_object(t_N).f_Cursor);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2995>";
		var t_Length=((bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_Before+"NOT")-bb_challengergui_CHGUI_Font.m_GetTxtWidth2("NOT"))|0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2996>";
		if((bb_challengergui_CHGUI_Cursor)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2997>";
			bb_graphics_SetColor(0.0,0.0,0.0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2998>";
			bb_graphics_DrawLine(t_X+(t_Length)+8.0,t_Y+t_YOff,t_X+(t_Length)+8.0,t_Y+t_YOff+bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text));
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2999>";
			bb_graphics_SetColor(255.0,255.0,255.0);
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawLabel(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2462>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2463>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2464>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2467>";
	var t_XOff=.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2468>";
	var t_YOff=.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2470>";
	bb_graphics_SetAlpha(0.25);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2471>";
	if((t_Active)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2471>";
		bb_graphics_SetAlpha(1.0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2472>";
	bb_challengergui_CHGUI_Font.m_DrawText2(dbg_object(t_N).f_Text,t_X,t_Y);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2473>";
	bb_graphics_SetAlpha(1.0);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawDropdown(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2551>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2552>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2553>";
	var t_W=dbg_object(t_N).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2554>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2555>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2557>";
	var t_OffX=280;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2558>";
	var t_OffY=10;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2559>";
	if((dbg_object(t_N).f_Over)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2559>";
		t_OffY=50;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2560>";
	if(((dbg_object(t_N).f_Down)!=0) || ((dbg_object(t_N).f_OnFocus)!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2560>";
		t_OffY=90;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2561>";
	if(t_Active==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2561>";
		t_OffY=130;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2564>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2566>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y,t_OffX+10,t_OffY,10,10,0.0,(t_W-20.0)/10.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2568>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y,t_OffX+30,t_OffY,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2570>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+10.0,t_OffX,t_OffY+10,10,10,0.0,1.0,(t_H-20.0)/10.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2572>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+10.0,t_OffX+30,t_OffY+10,10,10,0.0,1.0,(t_H-20.0)/10.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2574>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+10.0,t_OffX+10,t_OffY+10,20,20,0.0,(t_W-20.0)/20.0,(t_H-20.0)/20.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2576>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0,t_OffX,t_OffY+30,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2578>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+t_H-10.0,t_OffX+10,t_OffY+30,10,10,0.0,(t_W-20.0)/10.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2580>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+t_H-10.0,t_OffX+30,t_OffY+30,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2583>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_H,t_Y,t_OffX+40,t_OffY,40,40,0.0,t_H/40.0,t_H/40.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2586>";
	var t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text))/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2587>";
	bb_graphics_SetAlpha(0.25);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2588>";
	if((t_Active)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2588>";
		bb_graphics_SetAlpha(1.0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2589>";
	bb_challengergui_CHGUI_Font.m_DrawText2(dbg_object(t_N).f_Text,t_X+5.0,t_Y+t_YOff);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2590>";
	bb_graphics_SetAlpha(1.0);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawDropdownItem(t_N,t_C){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2597>";
	dbg_object(t_N).f_X=0.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2598>";
	dbg_object(t_N).f_Y=(t_C+1)*dbg_object(dbg_object(t_N).f_Parent).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2599>";
	dbg_object(t_N).f_W=dbg_object(dbg_object(t_N).f_Parent).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2600>";
	dbg_object(t_N).f_H=dbg_object(dbg_object(t_N).f_Parent).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2602>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2603>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2604>";
	var t_W=dbg_object(dbg_object(t_N).f_Parent).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2605>";
	var t_H=dbg_object(dbg_object(t_N).f_Parent).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2606>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2608>";
	var t_OffX=90;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2609>";
	var t_OffY=80;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2610>";
	var t_OffH=30;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2612>";
	if((dbg_object(t_N).f_Over)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2612>";
		t_OffY=110;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2613>";
	if(((dbg_object(t_N).f_Down)!=0) && ((bb_challengergui_CHGUI_RealActive(t_N))!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2613>";
		t_OffY=140;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2615>";
	if(t_C!=0 && t_C!=dbg_object(dbg_object(t_N).f_Parent).f_DropNumber){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2617>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY+10,10,10,0.0,1.0,t_H/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2619>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y,t_OffX+50,t_OffY+10,10,10,0.0,1.0,t_H/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2621>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y,t_OffX+10,t_OffY+10,40,10,0.0,(t_W-20.0)/40.0,t_H/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2623>";
		if((bb_challengergui_CHGUI_Shadow)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2624>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y,40,20,10,10,0.0,1.0,t_H/10.0,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2625>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y,0,20,10,10,0.0,1.0,t_H/10.0,0);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2629>";
	if(t_C==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2631>";
		if((bb_challengergui_CHGUI_Shadow)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2632>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y+10.0,40,20,10,10,0.0,1.0,(t_H-10.0)/10.0,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2633>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y+10.0,0,20,10,10,0.0,1.0,(t_H-10.0)/10.0,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2635>";
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y-10.0,0,0,10,20,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2637>";
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y-10.0,40,0,10,20,0);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2641>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,0.0,1.0,1.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2643>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y,t_OffX+10,t_OffY,40,10,0.0,(t_W-20.0)/40.0,1.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2645>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y,t_OffX+50,t_OffY,10,10,0.0,1.0,1.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2647>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+10.0,t_OffX,t_OffY+10,10,10,0.0,1.0,(t_H-10.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2649>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+10.0,t_OffX+50,t_OffY+10,10,10,0.0,1.0,(t_H-10.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2651>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+10.0,t_OffX+10,t_OffY+10,40,10,0.0,(t_W-20.0)/40.0,(t_H-10.0)/10.0,0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2654>";
	if(t_C==dbg_object(dbg_object(t_N).f_Parent).f_DropNumber){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2656>";
		if((bb_challengergui_CHGUI_Shadow)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2657>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y,40,20,10,10,0.0,1.0,(t_H-10.0)/10.0,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2658>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y,0,20,10,10,0.0,1.0,(t_H-10.0)/10.0,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2660>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+10.0,t_Y+t_H,20,40,10,10,0.0,(t_W-20.0)/10.0,1.0,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2662>";
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W-10.0,t_Y+t_H-10.0,30,30,20,20,0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2664>";
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y+t_H-10.0,0,30,20,20,0);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2667>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY+10,10,10,0.0,1.0,(t_H-10.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2669>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y,t_OffX+50,t_OffY+10,10,10,0.0,1.0,(t_H-10.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2671>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y,t_OffX+10,t_OffY+10,40,10,0.0,(t_W-20.0)/40.0,(t_H-10.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2673>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0,t_OffX,t_OffY+20,10,10,0.0,1.0,1.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2675>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+t_H-10.0,t_OffX+10,t_OffY+20,40,10,0.0,(t_W-20.0)/40.0,1.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2677>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+t_H-10.0,t_OffX+50,t_OffY+20,10,10,0.0,1.0,1.0,0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2681>";
	var t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text))/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2682>";
	bb_graphics_SetAlpha(0.25);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2683>";
	if((t_Active)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2683>";
		bb_graphics_SetAlpha(1.0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2684>";
	bb_challengergui_CHGUI_Font.m_DrawText2(dbg_object(t_N).f_Text,t_X+5.0,t_Y+t_YOff);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2685>";
	bb_graphics_SetAlpha(1.0);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawMenu(t_N,t_XOffset,t_C){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2689>";
	if(t_C==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2689>";
		t_XOffset=t_XOffset+1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2690>";
	dbg_object(t_N).f_X=(t_XOffset-t_C);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2691>";
	if(dbg_object(t_N).f_Parent!=bb_challengergui_CHGUI_Canvas && dbg_object(dbg_object(t_N).f_Parent).f_Text!=""){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2692>";
		dbg_object(t_N).f_Y=bb_challengergui_CHGUI_TitleHeight;
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2694>";
		dbg_object(t_N).f_Y=0.0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2696>";
	dbg_object(t_N).f_W=20.0+bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2697>";
	dbg_object(t_N).f_H=10.0+bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2698>";
	dbg_object(dbg_object(t_N).f_Parent).f_HasMenu=1;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2699>";
	dbg_object(dbg_object(t_N).f_Parent).f_MenuHeight=((dbg_object(t_N).f_H)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2700>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2701>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2702>";
	var t_W=dbg_object(t_N).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2703>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2704>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2705>";
	var t_OffX=100;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2706>";
	var t_OffY=90;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2707>";
	if((dbg_object(t_N).f_Over)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2707>";
		t_OffY=120;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2708>";
	if(((dbg_object(t_N).f_Down)!=0) || ((dbg_object(t_N).f_OnFocus)!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2709>";
		if(t_Active==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2709>";
			t_OffY=150;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2713>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,40,10,0.0,t_W/40.0,(t_H-10.0)/10.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2714>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+(t_H-10.0),t_OffX,t_OffY+10,40,10,0.0,t_W/40.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2717>";
	var t_XOff=(t_W-bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text))/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2718>";
	var t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text))/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2719>";
	bb_graphics_SetAlpha(0.25);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2720>";
	if((t_Active)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2720>";
		bb_graphics_SetAlpha(1.0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2721>";
	bb_challengergui_CHGUI_Font.m_DrawText2(dbg_object(t_N).f_Text,t_X+t_XOff,t_Y+t_YOff);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2722>";
	bb_graphics_SetAlpha(1.0);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawTab(t_N,t_Offset){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3005>";
	dbg_object(t_N).f_X=(t_Offset);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3006>";
	dbg_object(t_N).f_W=20.0+bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3007>";
	dbg_object(t_N).f_H=10.0+bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3008>";
	dbg_object(dbg_object(t_N).f_Parent).f_Tabbed=1;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3009>";
	dbg_object(dbg_object(t_N).f_Parent).f_TabHeight=((dbg_object(t_N).f_H)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3010>";
	dbg_object(t_N).f_Y=5.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3011>";
	if(dbg_object(dbg_object(t_N).f_Parent).f_Text!=""){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3011>";
		dbg_object(t_N).f_Y=dbg_object(t_N).f_Y+bb_challengergui_CHGUI_TitleHeight;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3012>";
	if((dbg_object(dbg_object(t_N).f_Parent).f_HasMenu)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3012>";
		dbg_object(t_N).f_Y=dbg_object(t_N).f_Y+(dbg_object(dbg_object(t_N).f_Parent).f_MenuHeight);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3014>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3015>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3016>";
	var t_W=dbg_object(t_N).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3017>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3018>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3020>";
	var t_OffX=10;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3021>";
	var t_OffY=180;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3022>";
	if(dbg_object(dbg_object(t_N).f_Parent).f_CurrentTab==t_N){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3023>";
		t_OffX=70;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3024>";
		t_OffY=180;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3026>";
	if((dbg_object(t_N).f_Down)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3027>";
		t_OffX=10;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3028>";
		t_OffY=210;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3030>";
	if(((dbg_object(t_N).f_Over)!=0) && dbg_object(t_N).f_Down==0 && dbg_object(dbg_object(t_N).f_Parent).f_CurrentTab!=t_N){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3031>";
		t_OffX=40;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3032>";
		t_OffY=180;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3035>";
	if(t_Active==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3036>";
		t_OffX=40;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3037>";
		t_OffY=210;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3042>";
	var t_YY=((t_Y)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3045>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,(t_YY),t_OffX,t_OffY,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3047>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,(t_YY),t_OffX+20,t_OffY,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3049>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,(t_YY),t_OffX+10,t_OffY,10,10,0.0,(t_W-20.0)/10.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3051>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,(t_YY+10),t_OffX,t_OffY+10,10,10,0.0,1.0,(((dbg_object(dbg_object(t_N).f_Parent).f_TabHeight-10)/10)|0),0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3053>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,(t_YY+10),t_OffX+20,t_OffY+10,10,10,0.0,1.0,(((dbg_object(dbg_object(t_N).f_Parent).f_TabHeight-10)/10)|0),0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3055>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,(t_YY+10),t_OffX+10,t_OffY+10,10,10,0.0,(t_W-20.0)/10.0,(((dbg_object(dbg_object(t_N).f_Parent).f_TabHeight-10)/10)|0),0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3058>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,(t_YY+dbg_object(dbg_object(t_N).f_Parent).f_TabHeight-10),t_OffX,t_OffY+20,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3060>";
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,(t_YY+dbg_object(dbg_object(t_N).f_Parent).f_TabHeight-10),t_OffX+20,t_OffY+20,10,10,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3062>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,(t_YY+dbg_object(dbg_object(t_N).f_Parent).f_TabHeight-10),t_OffX+10,t_OffY+20,10,10,0.0,(t_W-20.0)/10.0,1.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3065>";
	var t_XOff=(t_W-bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text))/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3066>";
	var t_YOff=((dbg_object(dbg_object(t_N).f_Parent).f_TabHeight)-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text))/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3067>";
	bb_graphics_SetAlpha(0.25);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3068>";
	if((t_Active)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3068>";
		bb_graphics_SetAlpha(1.0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3069>";
	bb_challengergui_CHGUI_Font.m_DrawText2(dbg_object(t_N).f_Text,t_X+t_XOff,(t_YY)+t_YOff);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3070>";
	bb_graphics_SetAlpha(1.0);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawMenuItem(t_N,t_C){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2728>";
	if(dbg_object(dbg_object(t_N).f_Parent).f_Element!="MenuItem"){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2729>";
		dbg_object(t_N).f_X=0.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2730>";
		dbg_object(t_N).f_Y=dbg_object(dbg_object(t_N).f_Parent).f_H+(t_C)*dbg_object(dbg_object(t_N).f_Parent).f_H-(t_C);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2731>";
		dbg_object(dbg_object(t_N).f_Parent).f_HasMenu=1;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2732>";
		dbg_object(dbg_object(t_N).f_Parent).f_MenuHeight=((dbg_object(t_N).f_H)|0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2734>";
	if(dbg_object(dbg_object(t_N).f_Parent).f_Element=="MenuItem"){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2735>";
		dbg_object(t_N).f_X=dbg_object(dbg_object(t_N).f_Parent).f_W-1.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2736>";
		dbg_object(t_N).f_Y=(t_C)*dbg_object(dbg_object(t_N).f_Parent).f_H-(t_C);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2739>";
	dbg_object(t_N).f_H=10.0+bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2740>";
	dbg_object(t_N).f_W=20.0+bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2742>";
	if((dbg_object(t_N).f_IsMenuParent)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2742>";
		dbg_object(t_N).f_W=dbg_object(t_N).f_W+dbg_object(t_N).f_H/3.0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2743>";
	if((dbg_object(t_N).f_Tick)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2743>";
		dbg_object(t_N).f_W=dbg_object(t_N).f_W+dbg_object(t_N).f_H/3.0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2745>";
	if(dbg_object(t_N).f_W>(dbg_object(dbg_object(t_N).f_Parent).f_MenuWidth)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2745>";
		dbg_object(dbg_object(t_N).f_Parent).f_MenuWidth=((dbg_object(t_N).f_W)|0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2746>";
	if((dbg_object(dbg_object(t_N).f_Parent).f_MenuWidth)>dbg_object(t_N).f_W){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2746>";
		dbg_object(t_N).f_W=(dbg_object(dbg_object(t_N).f_Parent).f_MenuWidth);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2748>";
	if((dbg_object(t_N).f_Tick)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2749>";
		if((dbg_object(dbg_object(t_N).f_Parent).f_MenuWidth)+dbg_object(t_N).f_H<dbg_object(t_N).f_W){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2749>";
			dbg_object(dbg_object(t_N).f_Parent).f_MenuWidth=((dbg_object(t_N).f_W+dbg_object(t_N).f_H)|0);
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2751>";
		if((dbg_object(dbg_object(t_N).f_Parent).f_MenuWidth)<dbg_object(t_N).f_W){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2751>";
			dbg_object(dbg_object(t_N).f_Parent).f_MenuWidth=((dbg_object(t_N).f_W)|0);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2754>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2755>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2756>";
	var t_W=dbg_object(t_N).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2757>";
	var t_H=dbg_object(t_N).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2758>";
	var t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2760>";
	var t_OffX=100;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2761>";
	var t_OffY=90;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2762>";
	if(((dbg_object(t_N).f_Over)!=0) || ((dbg_object(t_N).f_OnFocus)!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2762>";
		t_OffY=120;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2763>";
	if(((dbg_object(t_N).f_Down)!=0) && ((bb_challengergui_CHGUI_RealActive(t_N))!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2763>";
		t_OffY=150;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2766>";
	if(t_C==dbg_object(dbg_object(t_N).f_Parent).f_MenuNumber && ((bb_challengergui_CHGUI_Shadow)!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2767>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y,40,20,10,10,0.0,1.0,(t_H-10.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2768>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y,0,20,10,10,0.0,1.0,(t_H-10.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2770>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+10.0,t_Y+t_H,20,40,10,10,0.0,(t_W-20.0)/10.0,1.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2772>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W-10.0,t_Y+t_H-10.0,30,30,20,20,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2774>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y+t_H-10.0,0,30,20,20,0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2778>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y,t_OffX,t_OffY,40,10,0.0,(t_W-20.0)/40.0,t_H/10.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2779>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX-10,t_OffY,10,10,0.0,1.0,t_H/10.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2780>";
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y,t_OffX+40,t_OffY,10,10,0.0,1.0,t_H/10.0,0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2783>";
	if(((bb_challengergui_CHGUI_Shadow)!=0) && t_C!=dbg_object(dbg_object(t_N).f_Parent).f_MenuNumber){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2785>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y,40,20,10,10,0.0,1.0,(t_H-1.0)/10.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2787>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-10.0,t_Y,0,20,10,10,0.0,1.0,(t_H-1.0)/10.0,0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2791>";
	if(t_C==dbg_object(dbg_object(t_N).f_Parent).f_MenuNumber){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2792>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y+(t_H-10.0),t_OffX-10,t_OffY+10,10,10,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2793>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y+(t_H-10.0),t_OffX,t_OffY+10,40,10,0.0,(t_W-20.0)/40.0,1.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2794>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y+(t_H-10.0),t_OffX+40,t_OffY+10,10,10,0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2797>";
	if(t_C==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2798>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX-10,t_OffY-10,10,10,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2799>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+10.0,t_Y,t_OffX,t_OffY-10,40,10,0.0,(t_W-20.0)/40.0,1.0,0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2800>";
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-10.0,t_Y,t_OffX+40,t_OffY-10,10,10,0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2804>";
	if((dbg_object(t_N).f_Tick)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2805>";
		var t_XOF=230;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2806>";
		var t_YOF=10;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2807>";
		if((dbg_object(t_N).f_Over)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2807>";
			t_YOF=30;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2808>";
		if((dbg_object(t_N).f_Down)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2808>";
			t_YOF=50;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2809>";
		if(bb_challengergui_CHGUI_RealActive(t_N)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2809>";
			t_XOF=70;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2810>";
		if(dbg_object(t_N).f_Value>0.0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2810>";
			t_XOF=250;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2812>";
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+5.0,t_Y+(t_H-t_H/2.6)/2.0,t_XOF,t_YOF,20,20,0.0,t_H/2.6/20.0,t_H/2.6/20.0,0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2819>";
	var t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,dbg_object(t_N).f_Text))/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2820>";
	bb_graphics_SetAlpha(0.25);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2821>";
	if((t_Active)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2821>";
		bb_graphics_SetAlpha(1.0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2822>";
	if(dbg_object(t_N).f_Tick==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2823>";
		bb_challengergui_CHGUI_Font.m_DrawText2(dbg_object(t_N).f_Text,t_X+10.0,t_Y+t_YOff);
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2825>";
		bb_challengergui_CHGUI_Font.m_DrawText2(dbg_object(t_N).f_Text,t_X+10.0+t_H/2.0,t_Y+t_YOff);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2827>";
	bb_graphics_SetAlpha(1.0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2830>";
	if((dbg_object(t_N).f_IsMenuParent)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2831>";
		if(bb_challengergui_CHGUI_RealActive(t_N)==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2832>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_H/4.0-8.0,t_Y+(t_H-t_H/4.0)/2.0,130,180,10,10,0.0,t_H/4.0/10.0,t_H/4.0/10.0,0);
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2834>";
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_H/4.0-8.0,t_Y+(t_H-t_H/4.0)/2.0,140,180,10,10,0.0,t_H/4.0/10.0,t_H/4.0/10.0,0);
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_SubMenu(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2840>";
	var t_C=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2841>";
	var t_XX=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2842>";
	if((dbg_object(t_N).f_OnFocus)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2843>";
		t_C=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2844>";
		dbg_object(t_N).f_HasMenu=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2845>";
		for(t_XX=0;t_XX<=dbg_object(t_N).f_MenuItems.length-1;t_XX=t_XX+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2846>";
			if(bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_MenuItems,t_XX)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_MenuItems,t_XX)[dbg_index]))!=0)){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2846>";
				bb_challengergui_CHGUI_DrawMenuItem(dbg_array(dbg_object(t_N).f_MenuItems,t_XX)[dbg_index],t_C);
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2847>";
			if(bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_MenuItems,t_XX)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_MenuItems,t_XX)[dbg_index]))!=0)){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2847>";
				t_C=t_C+1;
			}
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2849>";
		if(dbg_object(t_N).f_MenuItems.length>0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2849>";
			dbg_object(t_N).f_MenuNumber=t_C-1;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2850>";
		for(t_XX=0;t_XX<=dbg_object(t_N).f_MenuItems.length-1;t_XX=t_XX+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2851>";
			if((dbg_object(dbg_array(dbg_object(t_N).f_MenuItems,t_XX)[dbg_index]).f_IsMenuParent)!=0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2851>";
				bb_challengergui_CHGUI_SubMenu(dbg_array(dbg_object(t_N).f_MenuItems,t_XX)[dbg_index]);
			}
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawContents(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2092>";
	var t_X=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2093>";
	var t_XX=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2094>";
	var t_XOffset=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2095>";
	var t_C=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2098>";
	if(dbg_object(t_N).f_Element!="Tab"){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2099>";
		if(dbg_object(t_N).f_Parent!=null){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2100>";
			if(bb_challengergui_CHGUI_RealVisible(t_N)==1 && bb_challengergui_CHGUI_RealMinimised(dbg_object(t_N).f_Parent)==0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2100>";
				bb_challengergui_CHGUI_DrawWindow(t_N);
			}
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2102>";
			bb_challengergui_CHGUI_DrawWindow(t_N);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2107>";
	for(t_X=0;t_X<=dbg_object(t_N).f_Buttons.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2108>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Buttons,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Buttons,t_X)[dbg_index])==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2108>";
			bb_challengergui_CHGUI_DrawButton(dbg_array(dbg_object(t_N).f_Buttons,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2112>";
	for(t_X=0;t_X<=dbg_object(t_N).f_ImageButtons.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2113>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_ImageButtons,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_ImageButtons,t_X)[dbg_index])==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2113>";
			bb_challengergui_CHGUI_DrawImageButton(dbg_array(dbg_object(t_N).f_ImageButtons,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2117>";
	for(t_X=0;t_X<=dbg_object(t_N).f_Tickboxes.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2118>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Tickboxes,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Tickboxes,t_X)[dbg_index])==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2118>";
			bb_challengergui_CHGUI_DrawTickbox(dbg_array(dbg_object(t_N).f_Tickboxes,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2122>";
	for(t_X=0;t_X<=dbg_object(t_N).f_Radioboxes.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2123>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Radioboxes,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Radioboxes,t_X)[dbg_index])==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2123>";
			bb_challengergui_CHGUI_DrawRadiobox(dbg_array(dbg_object(t_N).f_Radioboxes,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2127>";
	for(t_X=0;t_X<=dbg_object(t_N).f_Listboxes.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2128>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index])==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2129>";
			bb_challengergui_CHGUI_DrawListbox(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2131>";
			var t_C2=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2133>";
			for(t_XX=((dbg_object(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxSlider).f_Value)|0);(t_XX)<=dbg_object(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxSlider).f_Value+(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxNumber);t_XX=t_XX+1){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2134>";
				if(t_XX<dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxItems.length && t_XX>-1){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2135>";
					if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxItems,t_XX)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxItems,t_XX)[dbg_index])==0){
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2136>";
						bb_challengergui_CHGUI_DrawListboxItem(dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxItems,t_XX)[dbg_index],t_C2);
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2137>";
						t_C2=t_C2+1;
					}
				}
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2145>";
	for(t_X=0;t_X<=dbg_object(t_N).f_HSliders.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2146>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_HSliders,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_HSliders,t_X)[dbg_index])==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2146>";
			bb_challengergui_CHGUI_DrawHSlider(dbg_array(dbg_object(t_N).f_HSliders,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2150>";
	for(t_X=0;t_X<=dbg_object(t_N).f_VSliders.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2151>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_VSliders,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_VSliders,t_X)[dbg_index])==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2151>";
			bb_challengergui_CHGUI_DrawVSlider(dbg_array(dbg_object(t_N).f_VSliders,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2155>";
	for(t_X=0;t_X<=dbg_object(t_N).f_Textfields.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2156>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Textfields,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Textfields,t_X)[dbg_index])==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2156>";
			bb_challengergui_CHGUI_DrawTextfield(dbg_array(dbg_object(t_N).f_Textfields,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2161>";
	for(t_X=0;t_X<=dbg_object(t_N).f_Labels.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2162>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Labels,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Labels,t_X)[dbg_index])==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2162>";
			bb_challengergui_CHGUI_DrawLabel(dbg_array(dbg_object(t_N).f_Labels,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2166>";
	for(t_X=0;t_X<=dbg_object(t_N).f_Dropdowns.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2167>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index])==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2168>";
			bb_challengergui_CHGUI_DrawDropdown(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2173>";
	for(t_X=0;t_X<=dbg_object(t_N).f_Dropdowns.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2174>";
		if(dbg_object(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]).f_OnFocus==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2175>";
			t_C=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2176>";
			for(t_XX=0;t_XX<=dbg_object(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]).f_DropdownItems.length-1;t_XX=t_XX+1){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2177>";
				if(bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]).f_DropdownItems,t_XX)[dbg_index]))!=0)){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2177>";
					bb_challengergui_CHGUI_DrawDropdownItem(dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]).f_DropdownItems,t_XX)[dbg_index],t_C);
				}
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2178>";
				if(bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]).f_DropdownItems,t_XX)[dbg_index]))!=0)){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2178>";
					t_C=t_C+1;
				}
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2180>";
			if(dbg_object(t_N).f_Dropdowns.length>0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2180>";
				dbg_object(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]).f_DropNumber=t_C-1;
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2185>";
	t_XOffset=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2186>";
	dbg_object(t_N).f_HasMenu=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2187>";
	t_C=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2188>";
	for(t_X=0;t_X<=dbg_object(t_N).f_Menus.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2189>";
		if(bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2189>";
			bb_challengergui_CHGUI_DrawMenu(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index],t_XOffset,t_C);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2190>";
		if(bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2190>";
			t_XOffset=(((t_XOffset)+dbg_object(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index]).f_W)|0);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2191>";
		if(bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2191>";
			t_C=t_C+1;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2195>";
	t_C=5;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2196>";
	dbg_object(t_N).f_Tabbed=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2197>";
	for(t_X=0;t_X<=dbg_object(t_N).f_Tabs.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2198>";
		if(bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Tabs,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Tabs,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2198>";
			bb_challengergui_CHGUI_DrawTab(dbg_array(dbg_object(t_N).f_Tabs,t_X)[dbg_index],t_C);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2199>";
		if(bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Tabs,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Tabs,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2199>";
			t_C=(((t_C)+dbg_object(dbg_array(dbg_object(t_N).f_Tabs,t_X)[dbg_index]).f_W)|0);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2203>";
	for(var t_NN=0;t_NN<=dbg_object(t_N).f_BottomList.length-1;t_NN=t_NN+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2204>";
		if((dbg_object(dbg_array(dbg_object(t_N).f_BottomList,t_NN)[dbg_index]).f_Visible)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2204>";
			bb_challengergui_CHGUI_DrawContents(dbg_array(dbg_object(t_N).f_BottomList,t_NN)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2206>";
	for(var t_NN2=0;t_NN2<=dbg_object(t_N).f_VariList.length-1;t_NN2=t_NN2+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2207>";
		if((dbg_object(dbg_array(dbg_object(t_N).f_VariList,t_NN2)[dbg_index]).f_Visible)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2207>";
			bb_challengergui_CHGUI_DrawContents(dbg_array(dbg_object(t_N).f_VariList,t_NN2)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2209>";
	for(var t_NN3=0;t_NN3<=dbg_object(t_N).f_TopList.length-1;t_NN3=t_NN3+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2210>";
		if((dbg_object(dbg_array(dbg_object(t_N).f_TopList,t_NN3)[dbg_index]).f_Visible)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2210>";
			bb_challengergui_CHGUI_DrawContents(dbg_array(dbg_object(t_N).f_TopList,t_NN3)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2213>";
	if((dbg_object(t_N).f_Tabbed)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2214>";
		bb_challengergui_CHGUI_DrawContents(dbg_object(t_N).f_CurrentTab);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2218>";
	for(t_X=0;t_X<=dbg_object(t_N).f_Menus.length-1;t_X=t_X+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2219>";
		bb_challengergui_CHGUI_SubMenu(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index]);
	}
	pop_err();
	return 0;
}
var bb_challengergui_CHGUI_VariList;
var bb_challengergui_CHGUI_TopList;
var bb_challengergui_CHGUI_TooltipFlag;
var bb_challengergui_CHGUI_TooltipFont;
function bb_graphics_DrawRect(t_x,t_y,t_w,t_h){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<369>";
	bb_graphics_DebugRenderDevice();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<371>";
	bb_graphics_context.m_Validate();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<372>";
	bb_graphics_renderDevice.DrawRect(t_x,t_y,t_w,t_h);
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_DrawTooltip(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3161>";
	var t_X=(((bb_challengergui_CHGUI_RealX(t_N))+dbg_object(t_N).f_W)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3162>";
	var t_Y=(((bb_challengergui_CHGUI_RealY(t_N))+dbg_object(t_N).f_H)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3163>";
	var t_W=((bb_challengergui_CHGUI_TooltipFont.m_GetTxtWidth2(dbg_object(t_N).f_Tooltip)+10.0)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3164>";
	var t_H=((bb_challengergui_CHGUI_TooltipFont.m_GetTxtHeight(dbg_object(t_N).f_Tooltip)+10.0)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3165>";
	if(t_X+t_W>bb_graphics_DeviceWidth()){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3165>";
		t_X=bb_graphics_DeviceWidth()-t_W;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3166>";
	if(t_Y+t_H>bb_graphics_DeviceHeight()){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3166>";
		t_Y=bb_challengergui_CHGUI_RealY(t_N)-t_H;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3167>";
	bb_graphics_SetColor(100.0,100.0,100.0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3168>";
	bb_graphics_DrawRect((t_X),(t_Y),(t_W),(t_H));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3169>";
	bb_graphics_SetColor(250.0,250.0,210.0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3170>";
	bb_graphics_DrawRect((t_X+1),(t_Y+1),(t_W-2),(t_H-2));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3171>";
	bb_graphics_SetColor(255.0,255.0,255.0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3172>";
	bb_challengergui_CHGUI_TooltipFont.m_DrawText2(dbg_object(t_N).f_Tooltip,(t_X+5),(t_Y+5));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3173>";
	bb_graphics_SetColor(255.0,255.0,255.0);
	pop_err();
	return 0;
}
function bb_app_Millisecs(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<153>";
	var t_=bb_app_device.MilliSecs();
	pop_err();
	return t_;
}
var bb_challengergui_CHGUI_Millisecs;
var bb_challengergui_CHGUI_FPSCounter;
var bb_challengergui_CHGUI_FPS;
function bb_challengergui_CHGUI_FPSUpdate(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4023>";
	if(bb_app_Millisecs()>bb_challengergui_CHGUI_Millisecs){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4024>";
		bb_challengergui_CHGUI_Millisecs=bb_app_Millisecs()+1000;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4025>";
		bb_challengergui_CHGUI_FPS=bb_challengergui_CHGUI_FPSCounter;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4026>";
		bb_challengergui_CHGUI_FPSCounter=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4028>";
	bb_challengergui_CHGUI_FPSCounter=bb_challengergui_CHGUI_FPSCounter+1;
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_Draw(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<850>";
	var t_N=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<851>";
	var t_NN=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<853>";
	bb_graphics_SetBlend(0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<854>";
	bb_graphics_SetColor(255.0,255.0,255.0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<856>";
	for(t_N=0;t_N<=bb_challengergui_CHGUI_BottomList.length-1;t_N=t_N+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<857>";
		if((dbg_object(dbg_array(bb_challengergui_CHGUI_BottomList,t_N)[dbg_index]).f_Visible)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<857>";
			bb_challengergui_CHGUI_DrawContents(dbg_array(bb_challengergui_CHGUI_BottomList,t_N)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<859>";
	for(t_N=0;t_N<=bb_challengergui_CHGUI_VariList.length-1;t_N=t_N+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<860>";
		if((dbg_object(dbg_array(bb_challengergui_CHGUI_VariList,t_N)[dbg_index]).f_Visible)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<860>";
			bb_challengergui_CHGUI_DrawContents(dbg_array(bb_challengergui_CHGUI_VariList,t_N)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<862>";
	for(t_N=0;t_N<=bb_challengergui_CHGUI_TopList.length-1;t_N=t_N+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<863>";
		if((dbg_object(dbg_array(bb_challengergui_CHGUI_TopList,t_N)[dbg_index]).f_Visible)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<863>";
			bb_challengergui_CHGUI_DrawContents(dbg_array(bb_challengergui_CHGUI_TopList,t_N)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<866>";
	if(bb_challengergui_CHGUI_TooltipFlag!=null){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<866>";
		bb_challengergui_CHGUI_DrawTooltip(bb_challengergui_CHGUI_TooltipFlag);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<868>";
	bb_challengergui_CHGUI_FPSUpdate();
	pop_err();
	return 0;
}
var bb_challengergui_CHGUI_Started;
var bb_challengergui_CHGUI_Width;
var bb_challengergui_CHGUI_Height;
var bb_challengergui_CHGUI_CanvasFlag;
var bb_challengergui_CHGUI_TopTop;
function bb_challengergui_CreateWindow(t_X,t_Y,t_W,t_H,t_Title,t_Moveable,t_CloseButton,t_MinimiseButton,t_Mode,t_Parent){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<377>";
	if(bb_challengergui_CHGUI_Started==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<378>";
		bb_challengergui_CHGUI_Started=1;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<379>";
		bb_challengergui_CHGUI_Start();
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<381>";
	var t_N=bb_challengergui_CHGUI_new.call(new bb_challengergui_CHGUI);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<382>";
	dbg_object(t_N).f_X=(t_X);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<383>";
	dbg_object(t_N).f_Y=(t_Y);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<384>";
	dbg_object(t_N).f_W=(t_W);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<385>";
	dbg_object(t_N).f_H=(t_H);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<386>";
	dbg_object(t_N).f_Text=t_Title;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<387>";
	dbg_object(t_N).f_Shadow=bb_challengergui_CHGUI_Shadow;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<388>";
	dbg_object(t_N).f_Close=t_CloseButton;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<389>";
	dbg_object(t_N).f_Minimise=t_MinimiseButton;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<390>";
	dbg_object(t_N).f_Moveable=t_Moveable;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<391>";
	dbg_object(t_N).f_Mode=t_Mode;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<392>";
	dbg_object(t_N).f_Parent=t_Parent;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<393>";
	if(dbg_object(t_N).f_Parent==null){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<393>";
		dbg_object(t_N).f_Parent=bb_challengergui_CHGUI_Canvas;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<394>";
	if(dbg_object(t_N).f_Parent!=null){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<394>";
		dbg_object(dbg_object(t_N).f_Parent).f_IsParent=1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<395>";
	dbg_object(t_N).f_Element="Window";
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<396>";
	if(bb_challengergui_CHGUI_CanvasFlag==1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<397>";
		if(t_Mode==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<398>";
			bb_challengergui_CHGUI_BottomList=resize_object_array(bb_challengergui_CHGUI_BottomList,bb_challengergui_CHGUI_BottomList.length+1);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<399>";
			dbg_array(bb_challengergui_CHGUI_BottomList,bb_challengergui_CHGUI_BottomList.length-1)[dbg_index]=t_N
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<401>";
		if(t_Mode==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<402>";
			bb_challengergui_CHGUI_VariList=resize_object_array(bb_challengergui_CHGUI_VariList,bb_challengergui_CHGUI_VariList.length+1);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<403>";
			dbg_array(bb_challengergui_CHGUI_VariList,bb_challengergui_CHGUI_VariList.length-1)[dbg_index]=t_N
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<405>";
		if(t_Mode==2){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<406>";
			bb_challengergui_CHGUI_TopList=resize_object_array(bb_challengergui_CHGUI_TopList,bb_challengergui_CHGUI_TopList.length+1);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<407>";
			dbg_array(bb_challengergui_CHGUI_TopList,bb_challengergui_CHGUI_TopList.length-1)[dbg_index]=t_N
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<408>";
			bb_challengergui_CHGUI_TopTop=t_N;
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<411>";
		dbg_object(t_N).f_SubWindow=1;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<412>";
		if(t_Mode==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<413>";
			dbg_object(dbg_object(t_N).f_Parent).f_BottomList=resize_object_array(dbg_object(dbg_object(t_N).f_Parent).f_BottomList,dbg_object(dbg_object(t_N).f_Parent).f_BottomList.length+1);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<414>";
			dbg_array(dbg_object(dbg_object(t_N).f_Parent).f_BottomList,dbg_object(dbg_object(t_N).f_Parent).f_BottomList.length-1)[dbg_index]=t_N
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<416>";
		if(t_Mode==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<417>";
			dbg_object(dbg_object(t_N).f_Parent).f_VariList=resize_object_array(dbg_object(dbg_object(t_N).f_Parent).f_VariList,dbg_object(dbg_object(t_N).f_Parent).f_VariList.length+1);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<418>";
			dbg_array(dbg_object(dbg_object(t_N).f_Parent).f_VariList,dbg_object(dbg_object(t_N).f_Parent).f_VariList.length-1)[dbg_index]=t_N
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<420>";
		if(t_Mode==2){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<421>";
			dbg_object(dbg_object(t_N).f_Parent).f_TopList=resize_object_array(dbg_object(dbg_object(t_N).f_Parent).f_TopList,dbg_object(dbg_object(t_N).f_Parent).f_TopList.length+1);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<422>";
			dbg_array(dbg_object(dbg_object(t_N).f_Parent).f_TopList,dbg_object(dbg_object(t_N).f_Parent).f_TopList.length-1)[dbg_index]=t_N
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<426>";
	pop_err();
	return t_N;
}
function bb_app_LoadString(t_path){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<141>";
	var t_=bb_app_device.LoadString(bb_data_FixDataPath(t_path));
	pop_err();
	return t_;
}
var bb_challengergui_CHGUI_KeyboardWindow;
function bb_challengergui_CHGUI_CreateKeyButton(t_X,t_Y,t_W,t_H,t_Text,t_Parent){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3209>";
	var t_N=bb_challengergui_CHGUI_new.call(new bb_challengergui_CHGUI);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3210>";
	dbg_object(t_N).f_Parent=t_Parent;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3211>";
	if(t_Parent==null){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3211>";
		dbg_object(t_N).f_Parent=bb_challengergui_CHGUI_Canvas;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3212>";
	dbg_object(t_N).f_X=(t_X);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3213>";
	dbg_object(t_N).f_Y=(t_Y);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3214>";
	dbg_object(t_N).f_W=(t_W);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3215>";
	dbg_object(t_N).f_H=(t_H);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3216>";
	dbg_object(t_N).f_Text=t_Text;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3218>";
	dbg_object(dbg_object(t_N).f_Parent).f_Buttons=resize_object_array(dbg_object(dbg_object(t_N).f_Parent).f_Buttons,dbg_object(dbg_object(t_N).f_Parent).f_Buttons.length+1);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3219>";
	dbg_array(dbg_object(dbg_object(t_N).f_Parent).f_Buttons,dbg_object(dbg_object(t_N).f_Parent).f_Buttons.length-1)[dbg_index]=t_N
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3220>";
	pop_err();
	return t_N;
}
function bb_challengergui_CHGUI_CreateKeyboard(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3224>";
	bb_challengergui_CHGUI_KeyboardWindow=bb_challengergui_CreateWindow(0,0,bb_graphics_DeviceWidth(),100,"",0,0,0,2,null);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3225>";
	var t_KeyWidth=(bb_graphics_DeviceWidth())/12.5;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3226>";
	var t_KeyHeight=(bb_graphics_DeviceWidth())/12.5;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3228>";
	var t_GapX=t_KeyWidth*2.0/9.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3229>";
	var t_GapY=t_KeyWidth*2.0/9.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3231>";
	if(bb_graphics_DeviceWidth()>bb_graphics_DeviceHeight()){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3232>";
		t_KeyHeight=t_KeyHeight/1.7;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3233>";
		t_GapY=t_GapY/1.2;
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3235>";
		t_KeyHeight=t_KeyHeight*1.5;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3236>";
		t_GapY=t_GapY*1.2;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3240>";
	var t_EndGap=((bb_graphics_DeviceWidth())-t_KeyWidth*10.0-t_GapX*9.0)/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3241>";
	dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_H=t_EndGap*2.0+t_GapY*3.0+t_KeyHeight*4.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3242>";
	dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_Y=(bb_graphics_DeviceHeight())-dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3245>";
	var t_SX=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3246>";
	var t_SY=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3249>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,0)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"q",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3250>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3251>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,1)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"w",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3252>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3253>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,2)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"e",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3254>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3255>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,3)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"r",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3256>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3257>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,4)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"t",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3258>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3259>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,5)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"y",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3260>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3261>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,6)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"u",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3262>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3263>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,7)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"i",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3264>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3265>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,8)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"o",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3266>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3267>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,9)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"p",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3268>";
	t_SX=t_EndGap+t_KeyWidth/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3269>";
	t_SY=t_SY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3270>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,10)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"a",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3271>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3272>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,11)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"s",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3273>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3274>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,12)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"d",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3275>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3276>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,13)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"f",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3277>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3278>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,14)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"g",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3279>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3280>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,15)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"h",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3281>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3282>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,16)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"j",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3283>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3284>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,17)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"k",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3285>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3286>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,18)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"l",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3287>";
	t_SX=t_EndGap+t_KeyWidth/2.0+t_GapX+t_KeyWidth;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3288>";
	t_SY=t_SY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3289>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,19)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"z",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3290>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3291>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,20)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"x",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3292>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3293>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,21)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"c",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3294>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3295>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,22)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"v",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3296>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3297>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,23)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"b",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3298>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3299>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,24)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"n",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3300>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3301>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,25)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"m",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3303>";
	t_SX=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3304>";
	t_SY=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3307>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,26)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"Q",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3308>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3309>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,27)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"W",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3310>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3311>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,28)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"E",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3312>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3313>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,29)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"R",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3314>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3315>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,30)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"T",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3316>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3317>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,31)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"Y",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3318>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3319>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,32)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"U",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3320>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3321>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,33)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"I",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3322>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3323>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,34)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"O",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3324>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3325>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,35)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"P",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3326>";
	t_SX=t_EndGap+t_KeyWidth/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3327>";
	t_SY=t_SY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3328>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,36)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"A",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3329>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3330>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,37)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"S",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3331>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3332>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,38)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"D",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3333>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3334>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,39)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"F",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3335>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3336>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,40)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"G",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3337>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3338>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,41)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"H",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3339>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3340>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,42)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"J",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3341>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3342>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,43)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"K",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3343>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3344>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,44)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"L",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3345>";
	t_SX=t_EndGap+t_KeyWidth/2.0+t_GapX+t_KeyWidth;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3346>";
	t_SY=t_SY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3347>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,45)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"Z",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3348>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3349>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,46)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"X",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3350>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3351>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,47)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"C",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3352>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3353>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,48)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"V",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3354>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3355>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,49)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"B",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3356>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3357>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,50)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"N",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3358>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3359>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,51)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"M",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3362>";
	t_SX=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3363>";
	t_SY=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3366>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,52)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"1",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3367>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3368>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,53)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"2",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3369>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3370>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,54)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"3",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3371>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3372>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,55)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"4",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3373>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3374>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,56)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"5",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3375>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3376>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,57)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"6",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3377>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3378>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,58)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"7",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3379>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3380>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,59)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"8",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3381>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3382>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,60)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"9",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3383>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3384>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,61)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"0",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3385>";
	t_SX=t_EndGap+t_KeyWidth/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3386>";
	t_SY=t_SY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3387>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,62)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"-",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3388>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3389>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,63)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"/",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3390>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3391>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,64)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"\\",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3392>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3393>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,65)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),":",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3394>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3395>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,66)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),";",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3396>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3397>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,67)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"(",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3398>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3399>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,68)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),")",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3400>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3401>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,69)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"\u00a3",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3402>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3403>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,70)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"&",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3404>";
	t_SX=t_EndGap+t_KeyWidth/2.0+t_GapX+t_KeyWidth;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3405>";
	t_SY=t_SY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3406>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,71)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"@",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3407>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3408>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,72)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),".",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3409>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3410>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,73)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),",",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3411>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3412>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,74)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"?",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3413>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3414>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,75)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"!",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3415>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3416>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,76)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"'",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3417>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3418>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,77)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),String.fromCharCode(34),bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3421>";
	t_SX=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3422>";
	t_SY=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3425>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,78)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"[",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3426>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3427>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,79)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"]",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3428>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3429>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,80)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"{",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3430>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3431>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,81)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"}",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3432>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3433>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,82)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"#",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3434>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3435>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,83)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"%",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3436>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3437>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,84)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"^",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3438>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3439>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,85)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"*",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3440>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3441>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,86)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"+",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3442>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3443>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,87)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"=",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3444>";
	t_SX=t_EndGap+t_KeyWidth/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3445>";
	t_SY=t_SY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3446>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,88)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"_",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3447>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3448>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,89)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"|",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3449>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3450>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,90)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"~",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3451>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3452>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,91)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"<",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3453>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3454>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,92)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),">",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3455>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3456>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,93)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"$",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3457>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3458>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,94)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"\u20ac",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3459>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3460>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,95)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"\u00e9",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3461>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3462>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,96)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"\u00ac",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3463>";
	t_SX=t_EndGap+t_KeyWidth/2.0+t_GapX+t_KeyWidth;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3464>";
	t_SY=t_SY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3465>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,97)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"@",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3466>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3467>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,98)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),".",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3468>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3469>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,99)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),",",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3470>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3471>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,100)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"?",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3472>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3473>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,101)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"!",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3474>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3475>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,102)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),"'",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3476>";
	t_SX=t_SX+t_KeyWidth+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3477>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,103)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth)|0),((t_KeyHeight)|0),String.fromCharCode(34),bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3480>";
	t_SX=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3481>";
	t_SY=t_EndGap+t_KeyHeight+t_GapY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3482>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth+t_KeyWidth/2.0)|0),((t_KeyHeight)|0),"Shft",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3483>";
	t_SX=t_EndGap+t_KeyWidth*9.0+t_GapX*9.0-t_KeyWidth/2.0-t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3484>";
	t_SY=t_EndGap+t_KeyHeight+t_GapY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3485>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,105)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth+t_KeyWidth/2.0)|0),((t_KeyHeight)|0),"<--",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3486>";
	t_SX=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3487>";
	t_SY=t_SY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3488>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth*2.0+t_KeyWidth/2.0+t_GapX)|0),((t_KeyHeight)|0),"123",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3489>";
	t_SX=t_SX+t_KeyWidth*2.0+t_KeyWidth/2.0+t_GapX+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3490>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,107)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth*5.0+t_GapX*4.0)|0),((t_KeyHeight)|0)," ",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3491>";
	t_SX=t_SX+t_KeyWidth*5.0+t_GapX*5.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3492>";
	dbg_array(bb_challengergui_CHGUI_KeyboardButtons,108)[dbg_index]=bb_challengergui_CHGUI_CreateKeyButton(((t_SX)|0),((t_SY)|0),((t_KeyWidth*2.0+t_KeyWidth/2.0+t_GapX)|0),((t_KeyHeight)|0),"Enter",bb_challengergui_CHGUI_KeyboardWindow)
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3494>";
	for(var t_C=0;t_C<=108;t_C=t_C+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3495>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C)[dbg_index]).f_Visible=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3497>";
	dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_Visible=0;
	pop_err();
	return 0;
}
var bb_challengergui_CHGUI_MsgBoxWindow;
var bb_challengergui_CHGUI_MsgBoxLabel;
function bb_challengergui_CreateButton(t_X,t_Y,t_W,t_H,t_Text,t_Parent){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<431>";
	if(bb_challengergui_CHGUI_Started==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<432>";
		bb_challengergui_CHGUI_Started=1;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<433>";
		bb_challengergui_CHGUI_Start();
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<435>";
	var t_N=bb_challengergui_CHGUI_new.call(new bb_challengergui_CHGUI);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<436>";
	dbg_object(t_N).f_Parent=t_Parent;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<437>";
	if(t_Parent==null){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<437>";
		dbg_object(t_N).f_Parent=bb_challengergui_CHGUI_Canvas;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<438>";
	dbg_object(t_N).f_X=(t_X);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<439>";
	dbg_object(t_N).f_Y=(t_Y);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<440>";
	dbg_object(t_N).f_W=(t_W);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<441>";
	dbg_object(t_N).f_H=(t_H);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<442>";
	dbg_object(t_N).f_Text=t_Text;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<443>";
	dbg_object(t_N).f_Element="Button";
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<445>";
	dbg_object(dbg_object(t_N).f_Parent).f_Buttons=resize_object_array(dbg_object(dbg_object(t_N).f_Parent).f_Buttons,dbg_object(dbg_object(t_N).f_Parent).f_Buttons.length+1);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<446>";
	dbg_array(dbg_object(dbg_object(t_N).f_Parent).f_Buttons,dbg_object(dbg_object(t_N).f_Parent).f_Buttons.length-1)[dbg_index]=t_N
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<447>";
	pop_err();
	return t_N;
}
var bb_challengergui_CHGUI_MsgBoxButton;
function bb_challengergui_CHGUI_Start(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3180>";
	if(bb_challengergui_CHGUI_Width==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3180>";
		bb_challengergui_CHGUI_Width=bb_graphics_DeviceWidth();
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3181>";
	if(bb_challengergui_CHGUI_Height==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3181>";
		bb_challengergui_CHGUI_Height=bb_graphics_DeviceHeight();
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3182>";
	bb_challengergui_CHGUI_CanvasFlag=1;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3183>";
	bb_challengergui_CHGUI_Canvas=bb_challengergui_CreateWindow(0,0,bb_challengergui_CHGUI_Width,bb_challengergui_CHGUI_Height,"",0,0,0,0,null);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3184>";
	bb_challengergui_CHGUI_CanvasFlag=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3185>";
	if(bb_challengergui_CHGUI_Style==null){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3185>";
		bb_challengergui_CHGUI_Style=bb_graphics_LoadImage("GUI_mac.png",1,bb_graphics_Image_DefaultFlags);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3186>";
	bb_challengergui_CHGUI_ShadowImg=bb_graphics_LoadImage("Shadow.png",1,bb_graphics_Image_DefaultFlags);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3187>";
	if(bb_challengergui_CHGUI_MobileMode==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3188>";
		if(bb_challengergui_CHGUI_TitleFont==null){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3188>";
			bb_challengergui_CHGUI_TitleFont=bb_bitmapfont_BitmapFont_Load("Arial10B.txt",true);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3189>";
		if(bb_challengergui_CHGUI_Font==null){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3189>";
			bb_challengergui_CHGUI_Font=bb_bitmapfont_BitmapFont_Load("Arial12.txt",true);
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3191>";
		if(bb_challengergui_CHGUI_TitleFont==null){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3191>";
			bb_challengergui_CHGUI_TitleFont=bb_bitmapfont_BitmapFont_Load("Arial20B.txt",true);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3192>";
		if(bb_challengergui_CHGUI_Font==null){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3192>";
			bb_challengergui_CHGUI_Font=bb_bitmapfont_BitmapFont_Load("Arial22.txt",true);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3193>";
		bb_challengergui_CHGUI_TitleHeight=50.0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3195>";
	bb_challengergui_CHGUI_CreateKeyboard();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3196>";
	bb_challengergui_CHGUI_MsgBoxWindow=bb_challengergui_CreateWindow(100,100,200,100,"Message box",0,0,0,2,null);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3197>";
	bb_challengergui_CHGUI_MsgBoxLabel=bb_challengergui_CreateLabel(100,50,"Message text",bb_challengergui_CHGUI_MsgBoxWindow);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3198>";
	bb_challengergui_CHGUI_MsgBoxButton=bb_challengergui_CreateButton(150,70,100,25,"Ok",bb_challengergui_CHGUI_MsgBoxWindow);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3199>";
	dbg_object(bb_challengergui_CHGUI_MsgBoxWindow).f_Visible=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3200>";
	bb_challengergui_CHGUI_TooltipFont=bb_bitmapfont_BitmapFont_Load("Arial10.txt",true);
	pop_err();
	return 0;
}
function bb_challengergui_CreateLabel(t_X,t_Y,t_Text,t_Parent){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<678>";
	if(bb_challengergui_CHGUI_Started==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<679>";
		bb_challengergui_CHGUI_Started=1;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<680>";
		bb_challengergui_CHGUI_Start();
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<682>";
	var t_N=bb_challengergui_CHGUI_new.call(new bb_challengergui_CHGUI);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<683>";
	dbg_object(t_N).f_Parent=t_Parent;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<684>";
	if(t_Parent==null){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<684>";
		dbg_object(t_N).f_Parent=bb_challengergui_CHGUI_Canvas;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<685>";
	dbg_object(t_N).f_X=(t_X);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<686>";
	dbg_object(t_N).f_Y=(t_Y);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<687>";
	dbg_object(t_N).f_Text=t_Text;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<689>";
	dbg_object(t_N).f_Element="Label";
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<691>";
	dbg_object(dbg_object(t_N).f_Parent).f_Labels=resize_object_array(dbg_object(dbg_object(t_N).f_Parent).f_Labels,dbg_object(dbg_object(t_N).f_Parent).f_Labels.length+1);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<692>";
	dbg_array(dbg_object(dbg_object(t_N).f_Parent).f_Labels,dbg_object(dbg_object(t_N).f_Parent).f_Labels.length-1)[dbg_index]=t_N
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<693>";
	pop_err();
	return t_N;
}
var bb_data2_SCALE_W;
var bb_data2_SCALE_H;
function bb_data2_CScale(t_c){
	push_err();
	err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<13>";
	dbg_object(t_c).f_X*=(bb_graphics_DeviceWidth())/bb_data2_SCALE_W;
	err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<14>";
	dbg_object(t_c).f_W*=(bb_graphics_DeviceWidth())/bb_data2_SCALE_W;
	err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<15>";
	dbg_object(t_c).f_Y*=(bb_graphics_DeviceWidth())/bb_data2_SCALE_H;
	err_info="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<16>";
	dbg_object(t_c).f_H*=(bb_graphics_DeviceWidth())/bb_data2_SCALE_H;
	pop_err();
	return null;
}
function bb_challengergui_CreateDropdown(t_X,t_Y,t_W,t_H,t_Text,t_Parent){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<494>";
	if(bb_challengergui_CHGUI_Started==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<495>";
		bb_challengergui_CHGUI_Started=1;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<496>";
		bb_challengergui_CHGUI_Start();
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<498>";
	var t_N=bb_challengergui_CHGUI_new.call(new bb_challengergui_CHGUI);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<499>";
	dbg_object(t_N).f_Parent=t_Parent;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<500>";
	if(dbg_object(t_N).f_Parent==null){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<500>";
		dbg_object(t_N).f_Parent=bb_challengergui_CHGUI_Canvas;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<501>";
	dbg_object(t_N).f_X=(t_X);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<502>";
	dbg_object(t_N).f_Y=(t_Y);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<503>";
	dbg_object(t_N).f_H=(t_H);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<504>";
	dbg_object(t_N).f_W=(t_W);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<505>";
	dbg_object(t_N).f_Text=t_Text;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<506>";
	dbg_object(t_N).f_Element="Dropdown";
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<508>";
	dbg_object(dbg_object(t_N).f_Parent).f_Dropdowns=resize_object_array(dbg_object(dbg_object(t_N).f_Parent).f_Dropdowns,dbg_object(dbg_object(t_N).f_Parent).f_Dropdowns.length+1);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<509>";
	dbg_array(dbg_object(dbg_object(t_N).f_Parent).f_Dropdowns,dbg_object(dbg_object(t_N).f_Parent).f_Dropdowns.length-1)[dbg_index]=t_N
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<510>";
	pop_err();
	return t_N;
}
function bb_challengergui_CreateTextfield(t_X,t_Y,t_W,t_H,t_Text,t_Parent){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<634>";
	if(bb_challengergui_CHGUI_Started==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<635>";
		bb_challengergui_CHGUI_Started=1;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<636>";
		bb_challengergui_CHGUI_Start();
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<638>";
	var t_N=bb_challengergui_CHGUI_new.call(new bb_challengergui_CHGUI);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<639>";
	dbg_object(t_N).f_Parent=t_Parent;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<640>";
	if(t_Parent==null){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<640>";
		dbg_object(t_N).f_Parent=bb_challengergui_CHGUI_Canvas;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<641>";
	dbg_object(t_N).f_X=(t_X);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<642>";
	dbg_object(t_N).f_Y=(t_Y);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<643>";
	dbg_object(t_N).f_W=(t_W);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<644>";
	dbg_object(t_N).f_H=(t_H);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<645>";
	dbg_object(t_N).f_Text=t_Text;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<646>";
	dbg_object(t_N).f_Element="Textfield";
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<648>";
	dbg_object(dbg_object(t_N).f_Parent).f_Textfields=resize_object_array(dbg_object(dbg_object(t_N).f_Parent).f_Textfields,dbg_object(dbg_object(t_N).f_Parent).f_Textfields.length+1);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<649>";
	dbg_array(dbg_object(dbg_object(t_N).f_Parent).f_Textfields,dbg_object(dbg_object(t_N).f_Parent).f_Textfields.length-1)[dbg_index]=t_N
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<650>";
	pop_err();
	return t_N;
}
function bb_input_TouchDown(t_index){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<93>";
	var t_=bb_input_device.KeyDown(384+t_index);
	pop_err();
	return t_;
}
var bb_challengergui_CHGUI_MouseBusy;
var bb_challengergui_CHGUI_Over;
var bb_challengergui_CHGUI_OverFlag;
var bb_challengergui_CHGUI_DownFlag;
var bb_challengergui_CHGUI_MenuOver;
var bb_challengergui_CHGUI_TextBoxOver;
var bb_challengergui_CHGUI_TextboxOnFocus;
var bb_challengergui_CHGUI_TextBoxDown;
var bb_challengergui_CHGUI_DragOver;
var bb_challengergui_CHGUI_Moving;
var bb_challengergui_CHGUI_TargetY;
var bb_challengergui_CHGUI_TargetX;
var bb_challengergui_CHGUI_IgnoreMouse;
function bb_input_TouchX(t_index){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<85>";
	var t_=bb_input_device.TouchX(t_index);
	pop_err();
	return t_;
}
function bb_input_TouchY(t_index){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<89>";
	var t_=bb_input_device.TouchY(t_index);
	pop_err();
	return t_;
}
function bb_challengergui_CHGUI_ReorderSubWindows(t_Top){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3984>";
	if(dbg_object(dbg_object(t_Top).f_Parent).f_TopVari!=t_Top && dbg_object(t_Top).f_Mode==1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3985>";
		var t_N=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3986>";
		for(t_N=0;t_N<=dbg_object(dbg_object(t_Top).f_Parent).f_VariList.length-1;t_N=t_N+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3987>";
			if(dbg_array(dbg_object(dbg_object(t_Top).f_Parent).f_VariList,t_N)[dbg_index]==t_Top){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3987>";
				break;
			}
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3989>";
		for(var t_NN=t_N;t_NN<=dbg_object(dbg_object(t_Top).f_Parent).f_VariList.length-2;t_NN=t_NN+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3990>";
			dbg_array(dbg_object(dbg_object(t_Top).f_Parent).f_VariList,t_NN)[dbg_index]=dbg_array(dbg_object(dbg_object(t_Top).f_Parent).f_VariList,t_NN+1)[dbg_index]
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3992>";
		dbg_array(dbg_object(dbg_object(t_Top).f_Parent).f_VariList,dbg_object(dbg_object(t_Top).f_Parent).f_VariList.length-1)[dbg_index]=t_Top
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3993>";
		dbg_object(dbg_object(t_Top).f_Parent).f_TopVari=t_Top;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3996>";
	if(dbg_object(dbg_object(t_Top).f_Parent).f_TopTop!=t_Top && dbg_object(t_Top).f_Mode==2){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3997>";
		var t_N2=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3998>";
		for(t_N2=0;t_N2<=dbg_object(dbg_object(t_Top).f_Parent).f_TopList.length-1;t_N2=t_N2+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3999>";
			if(dbg_array(dbg_object(dbg_object(t_Top).f_Parent).f_TopList,t_N2)[dbg_index]==t_Top){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3999>";
				break;
			}
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4001>";
		for(var t_NN2=t_N2;t_NN2<=dbg_object(dbg_object(t_Top).f_Parent).f_TopList.length-2;t_NN2=t_NN2+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4002>";
			dbg_array(dbg_object(dbg_object(t_Top).f_Parent).f_TopList,t_NN2)[dbg_index]=dbg_array(dbg_object(dbg_object(t_Top).f_Parent).f_TopList,t_NN2+1)[dbg_index]
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4004>";
		dbg_array(dbg_object(dbg_object(t_Top).f_Parent).f_TopList,dbg_object(dbg_object(t_Top).f_Parent).f_TopList.length-1)[dbg_index]=t_Top
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4005>";
		dbg_object(dbg_object(t_Top).f_Parent).f_TopTop=t_Top;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4008>";
	if(dbg_object(dbg_object(t_Top).f_Parent).f_TopBottom!=t_Top && dbg_object(t_Top).f_Mode==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4009>";
		var t_N3=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4010>";
		for(t_N3=0;t_N3<=dbg_object(dbg_object(t_Top).f_Parent).f_BottomList.length-1;t_N3=t_N3+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4011>";
			if(dbg_array(dbg_object(dbg_object(t_Top).f_Parent).f_BottomList,t_N3)[dbg_index]==t_Top){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4011>";
				break;
			}
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4013>";
		for(var t_NN3=t_N3;t_NN3<=dbg_object(dbg_object(t_Top).f_Parent).f_BottomList.length-2;t_NN3=t_NN3+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4014>";
			dbg_array(dbg_object(dbg_object(t_Top).f_Parent).f_BottomList,t_NN3)[dbg_index]=dbg_array(dbg_object(dbg_object(t_Top).f_Parent).f_BottomList,t_NN3+1)[dbg_index]
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4016>";
		dbg_array(dbg_object(dbg_object(t_Top).f_Parent).f_BottomList,dbg_object(dbg_object(t_Top).f_Parent).f_BottomList.length-1)[dbg_index]=t_Top
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4017>";
		dbg_object(dbg_object(t_Top).f_Parent).f_TopBottom=t_Top;
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_Reorder(t_Top){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3979>";
	if(dbg_object(t_Top).f_Mode==1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3979>";
		bb_challengergui_CHGUI_ReorderSubWindows(t_Top);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3980>";
	if(dbg_object(t_Top).f_Mode==2){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3980>";
		bb_challengergui_CHGUI_ReorderSubWindows(t_Top);
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_CloseMenu(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1614>";
	dbg_object(t_N).f_OnFocus=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1615>";
	var t_E=t_N;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1616>";
	do{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1617>";
		if(dbg_object(dbg_object(t_E).f_Parent).f_Element=="Window"){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1617>";
			dbg_object(dbg_object(t_E).f_Parent).f_MenuActive=null;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1617>";
			break;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1618>";
		if(dbg_object(dbg_object(t_E).f_Parent).f_Element=="MenuItem"){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1618>";
			dbg_object(dbg_object(t_E).f_Parent).f_OnFocus=0;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1619>";
		if(dbg_object(dbg_object(t_E).f_Parent).f_Element=="Menu"){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1619>";
			dbg_object(dbg_object(t_E).f_Parent).f_OnFocus=0;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1621>";
		t_E=dbg_object(t_E).f_Parent;
	}while(!(false));
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_CloseMenuReverse(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1626>";
	dbg_object(t_N).f_OnFocus=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1627>";
	var t_E=t_N;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1628>";
	var t_C=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1629>";
	for(t_C=0;t_C<=dbg_object(t_E).f_MenuItems.length-1;t_C=t_C+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1630>";
		if(dbg_object(t_E).f_MenuItems.length>0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1631>";
			bb_challengergui_CHGUI_CloseMenuReverse(dbg_array(dbg_object(t_E).f_MenuItems,t_C)[dbg_index]);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1632>";
			dbg_object(dbg_array(dbg_object(t_E).f_MenuItems,t_C)[dbg_index]).f_OnFocus=0;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1634>";
			break;
		}
	}
	pop_err();
	return 0;
}
var bb_challengergui_CHGUI_Tooltips;
var bb_challengergui_CHGUI_TooltipTime;
var bb_challengergui_CHGUI_MenuClose;
function bb_challengergui_CHGUI_UpdateMenuItem(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1566>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1567>";
	if((bb_challengergui_CHGUI_RealActive(t_N))!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1567>";
		t_N.m_CheckDown();
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1568>";
	if(bb_challengergui_CHGUI_RealVisible(t_N)==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1568>";
		bb_challengergui_CHGUI_CloseMenu(t_N);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1570>";
	if((dbg_object(t_N).f_Over)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1570>";
		bb_challengergui_CHGUI_MenuOver=1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1572>";
	if(((dbg_object(t_N).f_IsMenuParent)!=0) && ((dbg_object(t_N).f_Over)!=0) && ((bb_challengergui_CHGUI_RealActive(t_N))!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1573>";
		dbg_object(t_N).f_OnFocus=1;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1574>";
		if(dbg_object(dbg_object(t_N).f_Parent).f_MenuOver!=null && t_N!=dbg_object(dbg_object(t_N).f_Parent).f_MenuOver){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1575>";
			dbg_object(dbg_object(dbg_object(t_N).f_Parent).f_MenuOver).f_OnFocus=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1576>";
			bb_challengergui_CHGUI_CloseMenuReverse(dbg_object(dbg_object(t_N).f_Parent).f_MenuOver);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1578>";
		dbg_object(dbg_object(t_N).f_Parent).f_MenuOver=t_N;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1581>";
	if(dbg_object(dbg_object(t_N).f_Parent).f_MenuOver!=null && ((dbg_object(t_N).f_Over)!=0) && t_N!=dbg_object(dbg_object(t_N).f_Parent).f_MenuOver){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1581>";
		dbg_object(dbg_object(dbg_object(t_N).f_Parent).f_MenuOver).f_OnFocus=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1583>";
	if((dbg_object(t_N).f_Clicked)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1584>";
		if(dbg_object(t_N).f_IsMenuParent==0 && dbg_object(t_N).f_Tick==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1585>";
			bb_challengergui_CHGUI_CloseMenu(t_N);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1587>";
		if((dbg_object(t_N).f_Tick)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1588>";
			if(dbg_object(t_N).f_Value==0.0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1589>";
				dbg_object(t_N).f_Value=1.0;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1591>";
				dbg_object(t_N).f_Value=0.0;
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1596>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1597>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1597>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1599>";
	if((bb_challengergui_CHGUI_MenuClose)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1599>";
		bb_challengergui_CHGUI_CloseMenu(t_N);
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateSubMenu(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1603>";
	var t_XX=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1604>";
	for(t_XX=dbg_object(t_N).f_MenuItems.length-1;t_XX>=0;t_XX=t_XX+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1605>";
		dbg_array(dbg_object(t_N).f_MenuItems,t_XX)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1606>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_MenuItems,t_XX)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_MenuItems,t_XX)[dbg_index])==0 && ((dbg_object(t_N).f_OnFocus)!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1606>";
			bb_challengergui_CHGUI_UpdateMenuItem(dbg_array(dbg_object(t_N).f_MenuItems,t_XX)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1608>";
	for(t_XX=dbg_object(t_N).f_MenuItems.length-1;t_XX>=0;t_XX=t_XX+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1609>";
		if((dbg_object(dbg_array(dbg_object(t_N).f_MenuItems,t_XX)[dbg_index]).f_IsMenuParent)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1609>";
			bb_challengergui_CHGUI_UpdateSubMenu(dbg_array(dbg_object(t_N).f_MenuItems,t_XX)[dbg_index]);
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateTab(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2028>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2029>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2031>";
	if((dbg_object(t_N).f_Clicked)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2031>";
		dbg_object(dbg_object(t_N).f_Parent).f_CurrentTab=t_N;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2033>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2034>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2034>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateMenu(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1534>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1535>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1536>";
	if((dbg_object(t_N).f_Over)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1536>";
		bb_challengergui_CHGUI_MenuOver=1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1538>";
	if((dbg_object(t_N).f_Clicked)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1539>";
		if(dbg_object(dbg_object(t_N).f_Parent).f_MenuActive!=t_N){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1540>";
			if(dbg_object(t_N).f_MenuItems.length>0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1541>";
				dbg_object(t_N).f_OnFocus=1;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1542>";
				dbg_object(dbg_object(t_N).f_Parent).f_MenuActive=t_N;
			}
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1545>";
			dbg_object(t_N).f_OnFocus=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1546>";
			bb_challengergui_CHGUI_CloseMenuReverse(dbg_object(dbg_object(t_N).f_Parent).f_MenuActive);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1547>";
			dbg_object(dbg_object(t_N).f_Parent).f_MenuActive=null;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1552>";
	if(((dbg_object(t_N).f_OnFocus)!=0) && dbg_object(t_N).f_MenuItems.length<1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1552>";
		dbg_object(t_N).f_OnFocus=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1553>";
	if(((dbg_object(t_N).f_Over)!=0) && dbg_object(dbg_object(t_N).f_Parent).f_MenuActive!=null){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1554>";
		dbg_object(dbg_object(dbg_object(t_N).f_Parent).f_MenuActive).f_OnFocus=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1555>";
		bb_challengergui_CHGUI_CloseMenuReverse(dbg_object(dbg_object(t_N).f_Parent).f_MenuActive);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1556>";
		dbg_object(dbg_object(t_N).f_Parent).f_MenuActive=t_N;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1557>";
		dbg_object(t_N).f_OnFocus=1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1560>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1561>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1561>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateDropdownItem(t_N,t_C){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1506>";
	dbg_object(t_N).f_H=dbg_object(dbg_object(t_N).f_Parent).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1507>";
	dbg_object(t_N).f_W=dbg_object(dbg_object(t_N).f_Parent).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1508>";
	dbg_object(t_N).f_X=0.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1509>";
	dbg_object(t_N).f_Y=dbg_object(dbg_object(t_N).f_Parent).f_H+(t_C)*dbg_object(t_N).f_H-(t_C)-1.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1511>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1512>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1514>";
	if((dbg_object(t_N).f_Clicked)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1515>";
		dbg_object(dbg_object(t_N).f_Parent).f_Text=dbg_object(t_N).f_Text;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1516>";
		dbg_object(dbg_object(t_N).f_Parent).f_Value=dbg_object(t_N).f_Value;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1517>";
		dbg_object(dbg_object(t_N).f_Parent).f_OnFocus=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1520>";
	if((bb_input_TouchDown(0))!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1521>";
		if(bb_input_TouchX(0)<(bb_challengergui_CHGUI_RealX(dbg_object(t_N).f_Parent)) || bb_input_TouchX(0)>(bb_challengergui_CHGUI_RealX(dbg_object(t_N).f_Parent))+dbg_object(t_N).f_W || bb_input_TouchY(0)<(bb_challengergui_CHGUI_RealY(dbg_object(t_N).f_Parent)) || bb_input_TouchY(0)>(bb_challengergui_CHGUI_RealY(dbg_object(t_N).f_Parent))+dbg_object(t_N).f_H*(dbg_object(dbg_object(t_N).f_Parent).f_DropdownItems.length+1)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1522>";
			dbg_object(dbg_object(t_N).f_Parent).f_OnFocus=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1523>";
			dbg_object(dbg_object(t_N).f_Parent).f_Over=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1524>";
			dbg_object(dbg_object(t_N).f_Parent).f_Down=0;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1528>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1529>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1529>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateDropdown(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1489>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1490>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1492>";
	if((dbg_object(t_N).f_Clicked)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1493>";
		if(dbg_object(t_N).f_OnFocus==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1494>";
			dbg_object(t_N).f_OnFocus=0;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1496>";
			dbg_object(t_N).f_OnFocus=1;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1500>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1501>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1501>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateLabel(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2081>";
	dbg_object(t_N).f_W=bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2082>";
	dbg_object(t_N).f_H=bb_challengergui_CHGUI_Font.m_GetTxtHeight(dbg_object(t_N).f_Text);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2084>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2085>";
	t_N.m_CheckDown();
	pop_err();
	return 0;
}
var bb_challengergui_CHGUI_TextboxFocus;
var bb_challengergui_CHGUI_Keyboard;
function bb_input_EnableKeyboard(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<41>";
	var t_=bb_input_device.SetKeyboardEnabled(1);
	pop_err();
	return t_;
}
var bb_challengergui_CHGUI_ShowKeyboard;
var bb_challengergui_CHGUI_AutoTextScroll;
function bb_input_GetChar(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<57>";
	var t_=bb_input_device.GetChar();
	pop_err();
	return t_;
}
function bb_challengergui_CHGUI_GetText(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1957>";
	var t_Before=dbg_object(t_N).f_Text.slice(0,dbg_object(t_N).f_Cursor);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1958>";
	var t_After=dbg_object(t_N).f_Text.slice(dbg_object(t_N).f_Cursor);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1959>";
	var t_In=bb_input_GetChar();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1962>";
	if(t_In>96 && t_In<123 && dbg_object(t_N).f_FormatText==1 && bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text)<dbg_object(t_N).f_W-12.0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1963>";
		t_Before=t_Before+String.fromCharCode(t_In);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1964>";
		dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor+1;
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1967>";
		if(t_In>64 && t_In<91 && dbg_object(t_N).f_FormatText==1 && bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text)<dbg_object(t_N).f_W-12.0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1968>";
			t_Before=t_Before+String.fromCharCode(t_In);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1969>";
			dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor+1;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1972>";
			if(t_In>45 && t_In<58 && dbg_object(t_N).f_FormatNumber==1 && bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text)<dbg_object(t_N).f_W-12.0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1973>";
				if(t_In!=47 || dbg_object(t_N).f_FormatSymbol==1){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1974>";
					t_Before=t_Before+String.fromCharCode(t_In);
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1975>";
					dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor+1;
				}
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1978>";
				if(t_In>32 && t_In<48 && dbg_object(t_N).f_FormatSymbol==1 && bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text)<dbg_object(t_N).f_W-12.0){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1979>";
					t_Before=t_Before+String.fromCharCode(t_In);
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1980>";
					dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor+1;
				}else{
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1983>";
					if(t_In>57 && t_In<65 && dbg_object(t_N).f_FormatSymbol==1 && bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text)<dbg_object(t_N).f_W-12.0){
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1984>";
						t_Before=t_Before+String.fromCharCode(t_In);
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1985>";
						dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor+1;
					}else{
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1988>";
						if(t_In>90 && t_In<97 && dbg_object(t_N).f_FormatSymbol==1 && bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text)<dbg_object(t_N).f_W-12.0){
							err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1989>";
							t_Before=t_Before+String.fromCharCode(t_In);
							err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1990>";
							dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor+1;
						}else{
							err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1993>";
							if(t_In>122 && t_In<127 && dbg_object(t_N).f_FormatSymbol==1 && bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text)<dbg_object(t_N).f_W-12.0){
								err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1994>";
								t_Before=t_Before+String.fromCharCode(t_In);
								err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1995>";
								dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor+1;
							}else{
								err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1998>";
								if(t_In==32 && dbg_object(t_N).f_FormatSpace==1 && bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text)<dbg_object(t_N).f_W-12.0){
									err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1999>";
									t_Before=t_Before+String.fromCharCode(t_In);
									err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2000>";
									dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor+1;
								}else{
									err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2002>";
									if(t_In==8){
										err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2004>";
										t_Before=t_Before.slice(0,t_Before.length-1);
										err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2005>";
										if(dbg_object(t_N).f_Cursor>0){
											err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2005>";
											dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor-1;
										}
									}else{
										err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2007>";
										if(t_In==127){
											err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2009>";
											t_After=t_After.slice(1);
										}else{
											err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2011>";
											if(t_In==65575){
												err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2013>";
												if(dbg_object(t_N).f_Cursor<dbg_object(t_N).f_Text.length){
													err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2013>";
													dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor+1;
												}
											}else{
												err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2015>";
												if(t_In==65573){
													err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2017>";
													if(dbg_object(t_N).f_Cursor>0){
														err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2017>";
														dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor-1;
													}
												}else{
													err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2019>";
													if(t_In==13){
														err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2021>";
														dbg_object(t_N).f_OnFocus=0;
													}
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2024>";
	dbg_object(t_N).f_Text=t_Before+t_After;
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateKeyboardSizes(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3725>";
	var t_KeyWidth=(bb_graphics_DeviceWidth())/12.5;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3726>";
	var t_KeyHeight=(bb_graphics_DeviceWidth())/12.5;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3728>";
	var t_GapX=t_KeyWidth*2.0/9.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3729>";
	var t_GapY=t_KeyWidth*2.0/9.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3731>";
	if(bb_graphics_DeviceWidth()>bb_graphics_DeviceHeight()){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3732>";
		t_KeyHeight=t_KeyHeight/1.7;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3733>";
		t_GapY=t_GapY/1.2;
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3735>";
		t_KeyHeight=t_KeyHeight*1.5;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3736>";
		t_GapY=t_GapY*1.2;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3740>";
	var t_EndGap=((bb_graphics_DeviceWidth())-t_KeyWidth*10.0-t_GapX*9.0)/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3742>";
	dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_H=t_EndGap*2.0+t_GapY*3.0+t_KeyHeight*4.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3743>";
	dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_Y=(bb_graphics_DeviceHeight())-dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_H;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3744>";
	dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_W=(bb_graphics_DeviceWidth());
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3746>";
	var t_SX=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3747>";
	var t_SY=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3749>";
	for(var t_C=0;t_C<=9;t_C=t_C+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3750>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C)[dbg_index]).f_X=t_SX;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3751>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C)[dbg_index]).f_Y=t_SY;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3752>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C)[dbg_index]).f_W=t_KeyWidth;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3753>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C)[dbg_index]).f_H=t_KeyHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3754>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C+26)[dbg_index]).f_X=t_SX;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3755>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C+26)[dbg_index]).f_Y=t_SY;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3756>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C+26)[dbg_index]).f_W=t_KeyWidth;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3757>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C+26)[dbg_index]).f_H=t_KeyHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3758>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C+52)[dbg_index]).f_X=t_SX;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3759>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C+52)[dbg_index]).f_Y=t_SY;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3760>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C+52)[dbg_index]).f_W=t_KeyWidth;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3761>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C+52)[dbg_index]).f_H=t_KeyHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3762>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C+78)[dbg_index]).f_X=t_SX;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3763>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C+78)[dbg_index]).f_Y=t_SY;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3764>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C+78)[dbg_index]).f_W=t_KeyWidth;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3765>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C+78)[dbg_index]).f_H=t_KeyHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3766>";
		t_SX=t_SX+t_GapX+t_KeyWidth;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3769>";
	t_SX=t_EndGap+t_KeyWidth/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3770>";
	t_SY=t_SY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3772>";
	for(var t_C2=10;t_C2<=18;t_C2=t_C2+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3773>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2)[dbg_index]).f_X=t_SX;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3774>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2)[dbg_index]).f_Y=t_SY;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3775>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2)[dbg_index]).f_W=t_KeyWidth;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3776>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2)[dbg_index]).f_H=t_KeyHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3777>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2+26)[dbg_index]).f_X=t_SX;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3778>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2+26)[dbg_index]).f_Y=t_SY;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3779>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2+26)[dbg_index]).f_W=t_KeyWidth;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3780>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2+26)[dbg_index]).f_H=t_KeyHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3781>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2+52)[dbg_index]).f_X=t_SX;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3782>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2+52)[dbg_index]).f_Y=t_SY;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3783>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2+52)[dbg_index]).f_W=t_KeyWidth;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3784>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2+52)[dbg_index]).f_H=t_KeyHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3785>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2+78)[dbg_index]).f_X=t_SX;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3786>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2+78)[dbg_index]).f_Y=t_SY;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3787>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2+78)[dbg_index]).f_W=t_KeyWidth;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3788>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2+78)[dbg_index]).f_H=t_KeyHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3789>";
		t_SX=t_SX+t_GapX+t_KeyWidth;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3792>";
	t_SX=t_EndGap+t_KeyWidth/2.0+t_GapX+t_KeyWidth;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3793>";
	t_SY=t_SY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3795>";
	for(var t_C3=19;t_C3<=25;t_C3=t_C3+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3796>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3)[dbg_index]).f_X=t_SX;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3797>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3)[dbg_index]).f_Y=t_SY;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3798>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3)[dbg_index]).f_W=t_KeyWidth;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3799>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3)[dbg_index]).f_H=t_KeyHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3800>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3+26)[dbg_index]).f_X=t_SX;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3801>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3+26)[dbg_index]).f_Y=t_SY;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3802>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3+26)[dbg_index]).f_W=t_KeyWidth;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3803>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3+26)[dbg_index]).f_H=t_KeyHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3804>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3+52)[dbg_index]).f_X=t_SX;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3805>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3+52)[dbg_index]).f_Y=t_SY;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3806>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3+52)[dbg_index]).f_W=t_KeyWidth;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3807>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3+52)[dbg_index]).f_H=t_KeyHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3808>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3+78)[dbg_index]).f_X=t_SX;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3809>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3+78)[dbg_index]).f_Y=t_SY;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3810>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3+78)[dbg_index]).f_W=t_KeyWidth;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3811>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3+78)[dbg_index]).f_H=t_KeyHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3812>";
		t_SX=t_SX+t_GapX+t_KeyWidth;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3815>";
	t_SX=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3816>";
	t_SY=t_EndGap+t_KeyHeight+t_GapY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3817>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_X=t_SX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3818>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_Y=t_SY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3819>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_W=t_KeyWidth+t_KeyWidth/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3820>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_H=t_KeyHeight;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3823>";
	t_SX=t_EndGap+t_KeyWidth*9.0+t_GapX*9.0-t_KeyWidth/2.0-t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3824>";
	t_SY=t_EndGap+t_KeyHeight+t_GapY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3825>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,105)[dbg_index]).f_X=t_SX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3826>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,105)[dbg_index]).f_Y=t_SY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3827>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,105)[dbg_index]).f_W=t_KeyWidth+t_KeyWidth/2.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3828>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,105)[dbg_index]).f_H=t_KeyHeight;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3830>";
	t_SX=t_EndGap;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3831>";
	t_SY=t_SY+t_KeyHeight+t_GapY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3832>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_X=t_SX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3833>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_Y=t_SY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3834>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_W=t_KeyWidth*2.0+t_KeyWidth/2.0+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3835>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_H=t_KeyHeight;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3837>";
	t_SX=t_SX+t_KeyWidth*2.0+t_KeyWidth/2.0+t_GapX+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3838>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,107)[dbg_index]).f_X=t_SX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3839>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,107)[dbg_index]).f_Y=t_SY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3840>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,107)[dbg_index]).f_W=t_KeyWidth*5.0+t_GapX*4.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3841>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,107)[dbg_index]).f_H=t_KeyHeight;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3843>";
	t_SX=t_SX+t_KeyWidth*5.0+t_GapX*5.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3844>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,108)[dbg_index]).f_X=t_SX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3845>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,108)[dbg_index]).f_Y=t_SY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3846>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,108)[dbg_index]).f_W=t_KeyWidth*2.0+t_KeyWidth/2.0+t_GapX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3847>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,108)[dbg_index]).f_H=t_KeyHeight;
	pop_err();
	return 0;
}
var bb_challengergui_CHGUI_KeyboardPage;
var bb_challengergui_CHGUI_KeyboardShift;
var bb_challengergui_CHGUI_OldX;
var bb_challengergui_CHGUI_OldY;
function bb_challengergui_CHGUI_UpdateKeyboard(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3502>";
	bb_challengergui_CHGUI_Reorder(bb_challengergui_CHGUI_KeyboardWindow);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3504>";
	bb_challengergui_CHGUI_UpdateKeyboardSizes();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3507>";
	dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_X=0.0-bb_challengergui_CHGUI_OffsetX;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3508>";
	dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_Y=(bb_graphics_DeviceHeight())-dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_H-bb_challengergui_CHGUI_OffsetY;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3511>";
	if(bb_challengergui_CHGUI_KeyboardPage>1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3512>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_Active=0;
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3514>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_Active=1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3519>";
	for(var t_C=0;t_C<=108;t_C=t_C+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3520>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C)[dbg_index]).f_Active=1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3523>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_Active=1;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3525>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_Active=1;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3527>";
	dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,107)[dbg_index]).f_Active=1;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3530>";
	if(dbg_object(t_N).f_FormatSpace==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3530>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,107)[dbg_index]).f_Active=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3534>";
	if(dbg_object(t_N).f_FormatText==1 && dbg_object(t_N).f_FormatNumber==0 && dbg_object(t_N).f_FormatSymbol==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3535>";
		if(bb_challengergui_CHGUI_KeyboardPage>1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3535>";
			bb_challengergui_CHGUI_KeyboardPage=0;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3536>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_Active=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3539>";
	if(dbg_object(t_N).f_FormatNumber==1 && dbg_object(t_N).f_FormatText==0 && dbg_object(t_N).f_FormatSymbol==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3540>";
		bb_challengergui_CHGUI_KeyboardPage=2;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3541>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_Active=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3542>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_Active=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3543>";
		for(var t_C2=62;t_C2<=77;t_C2=t_C2+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3544>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C2)[dbg_index]).f_Active=0;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3548>";
	if(dbg_object(t_N).f_FormatNumber==0 && dbg_object(t_N).f_FormatText==0 && dbg_object(t_N).f_FormatSymbol==1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3549>";
		if(bb_challengergui_CHGUI_KeyboardPage<2){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3549>";
			bb_challengergui_CHGUI_KeyboardPage=2;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3550>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_Active=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3551>";
		for(var t_C3=52;t_C3<=61;t_C3=t_C3+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3552>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C3)[dbg_index]).f_Active=0;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3556>";
	if(dbg_object(t_N).f_FormatText==1 && dbg_object(t_N).f_FormatNumber==1 && dbg_object(t_N).f_FormatSymbol==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3557>";
		if(bb_challengergui_CHGUI_KeyboardPage>2){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3557>";
			bb_challengergui_CHGUI_KeyboardPage=0;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3558>";
		for(var t_C4=62;t_C4<=77;t_C4=t_C4+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3559>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C4)[dbg_index]).f_Active=0;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3563>";
	if(dbg_object(t_N).f_FormatText==1 && dbg_object(t_N).f_FormatNumber==0 && dbg_object(t_N).f_FormatSymbol==1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3564>";
		for(var t_C5=52;t_C5<=61;t_C5=t_C5+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3565>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C5)[dbg_index]).f_Active=0;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3569>";
	if(dbg_object(t_N).f_FormatText==0 && dbg_object(t_N).f_FormatNumber==1 && dbg_object(t_N).f_FormatSymbol==1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3570>";
		if(bb_challengergui_CHGUI_KeyboardPage<2){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3570>";
			bb_challengergui_CHGUI_KeyboardPage=2;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3571>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_Active=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3576>";
	if(bb_challengergui_CHGUI_KeyboardPage>1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3576>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_Active=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3583>";
	if(bb_challengergui_CHGUI_KeyboardPage<2){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3584>";
		if((dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_Clicked)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3585>";
			if(bb_challengergui_CHGUI_KeyboardShift==0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3586>";
				bb_challengergui_CHGUI_KeyboardShift=1;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3587>";
				bb_challengergui_CHGUI_KeyboardPage=1;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3588>";
				bb_challengergui_CHGUI_ShiftHold=0;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3590>";
				bb_challengergui_CHGUI_KeyboardShift=0;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3591>";
				bb_challengergui_CHGUI_KeyboardPage=0;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3592>";
				bb_challengergui_CHGUI_ShiftHold=0;
			}
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3596>";
		if((dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_DoubleClicked)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3597>";
			bb_challengergui_CHGUI_ShiftHold=1;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3598>";
			bb_challengergui_CHGUI_KeyboardShift=1;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3599>";
			bb_challengergui_CHGUI_KeyboardPage=1;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3604>";
	if((dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_Clicked)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3605>";
		if(bb_challengergui_CHGUI_KeyboardPage==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3606>";
			bb_challengergui_CHGUI_KeyboardPage=2;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3607>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_Active=0;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3608>";
			if(bb_challengergui_CHGUI_KeyboardPage==1){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3609>";
				bb_challengergui_CHGUI_KeyboardPage=2;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3610>";
				dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_Active=0;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3611>";
				if(bb_challengergui_CHGUI_KeyboardPage==2){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3612>";
					bb_challengergui_CHGUI_KeyboardPage=3;
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3613>";
					dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,104)[dbg_index]).f_Active=0;
				}else{
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3614>";
					if(bb_challengergui_CHGUI_KeyboardPage==3){
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3615>";
						bb_challengergui_CHGUI_KeyboardPage=0;
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3616>";
						bb_challengergui_CHGUI_KeyboardShift=0;
					}
				}
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3622>";
	if(bb_challengergui_CHGUI_KeyboardPage==0 || bb_challengergui_CHGUI_KeyboardPage==1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3623>";
		if(dbg_object(t_N).f_FormatNumber==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3624>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_Text="123";
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3626>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_Text="#+=";
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3630>";
	if(bb_challengergui_CHGUI_KeyboardPage==2){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3631>";
		if(dbg_object(t_N).f_FormatSymbol==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3632>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_Text="#+=";
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3634>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_Text="Abc";
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3638>";
	if(bb_challengergui_CHGUI_KeyboardPage==3){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3639>";
		if(dbg_object(t_N).f_FormatText==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3640>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_Text="#+=";
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3641>";
			if(dbg_object(t_N).f_FormatNumber==1){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3642>";
				dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_Text="123";
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3644>";
				dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,106)[dbg_index]).f_Text="#+=";
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3651>";
	for(var t_C6=0;t_C6<=108;t_C6=t_C6+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3652>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C6)[dbg_index]).f_Visible=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3655>";
	if(bb_challengergui_CHGUI_KeyboardPage==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3656>";
		for(var t_C7=0;t_C7<=25;t_C7=t_C7+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3657>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C7)[dbg_index]).f_Visible=1;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3660>";
	if(bb_challengergui_CHGUI_KeyboardPage==1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3661>";
		for(var t_C8=26;t_C8<=51;t_C8=t_C8+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3662>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C8)[dbg_index]).f_Visible=1;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3665>";
	if(bb_challengergui_CHGUI_KeyboardPage==2){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3666>";
		for(var t_C9=52;t_C9<=77;t_C9=t_C9+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3667>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C9)[dbg_index]).f_Visible=1;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3670>";
	if(bb_challengergui_CHGUI_KeyboardPage==3){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3671>";
		for(var t_C10=78;t_C10<=103;t_C10=t_C10+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3672>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C10)[dbg_index]).f_Visible=1;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3675>";
	for(var t_C11=104;t_C11<=108;t_C11=t_C11+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3676>";
		dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C11)[dbg_index]).f_Visible=1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3681>";
	var t_Before=dbg_object(t_N).f_Text.slice(0,dbg_object(t_N).f_Cursor);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3682>";
	var t_After=dbg_object(t_N).f_Text.slice(dbg_object(t_N).f_Cursor);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3685>";
	for(var t_C1=0;t_C1<=103;t_C1=t_C1+1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3686>";
		if(((dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C1)[dbg_index]).f_Clicked)!=0) && bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text)<dbg_object(t_N).f_W-12.0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3687>";
			t_Before=t_Before+dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,t_C1)[dbg_index]).f_Text;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3688>";
			dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor+1;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3689>";
			if(bb_challengergui_CHGUI_ShiftHold==0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3690>";
				if(bb_challengergui_CHGUI_KeyboardPage==0 || bb_challengergui_CHGUI_KeyboardPage==1){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3691>";
					bb_challengergui_CHGUI_KeyboardShift=0;
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3691>";
					bb_challengergui_CHGUI_KeyboardPage=0;
				}
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3697>";
	if((dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,105)[dbg_index]).f_Down)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3698>";
		if(dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,105)[dbg_index]).f_DKeyMillisecs<bb_app_Millisecs()){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3699>";
			dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,105)[dbg_index]).f_DKeyMillisecs=bb_app_Millisecs()+150;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3700>";
			t_Before=t_Before.slice(0,t_Before.length-1);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3701>";
			if(dbg_object(t_N).f_Cursor>0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3701>";
				dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor-1;
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3705>";
	if(((dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,107)[dbg_index]).f_Clicked)!=0) && bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text)<dbg_object(t_N).f_W-12.0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3706>";
		t_Before=t_Before+" ";
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3707>";
		dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Cursor+1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3710>";
	if((dbg_object(dbg_array(bb_challengergui_CHGUI_KeyboardButtons,108)[dbg_index]).f_Clicked)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3711>";
		dbg_object(t_N).f_OnFocus=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3712>";
		dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_Visible=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3713>";
		bb_challengergui_CHGUI_KeyboardPage=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3714>";
		if(bb_challengergui_CHGUI_AutoTextScroll==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3715>";
			bb_challengergui_CHGUI_TargetX=bb_challengergui_CHGUI_OldX;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3716>";
			bb_challengergui_CHGUI_TargetY=bb_challengergui_CHGUI_OldY;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3717>";
			bb_challengergui_CHGUI_Moving=1;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3721>";
	dbg_object(t_N).f_Text=t_Before+t_After;
	pop_err();
	return 0;
}
function bb_input_DisableKeyboard(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<45>";
	var t_=bb_input_device.SetKeyboardEnabled(0);
	pop_err();
	return t_;
}
function bb_challengergui_CHGUI_UpdateTextfield(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1873>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1874>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1876>";
	if((dbg_object(t_N).f_Over)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1876>";
		bb_challengergui_CHGUI_TextBoxOver=1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1877>";
	if((dbg_object(t_N).f_Down)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1877>";
		bb_challengergui_CHGUI_TextBoxDown=1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1878>";
	if((dbg_object(t_N).f_OnFocus)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1878>";
		bb_challengergui_CHGUI_TextboxOnFocus=1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1881>";
	if((dbg_object(t_N).f_Clicked)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1882>";
		bb_challengergui_CHGUI_TextboxFocus=t_N;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1883>";
		if(dbg_object(t_N).f_OnFocus==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1884>";
			dbg_object(t_N).f_Cursor=dbg_object(t_N).f_Text.length;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1885>";
			dbg_object(t_N).f_OnFocus=1;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1886>";
			if(bb_challengergui_CHGUI_Keyboard==1){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1887>";
				bb_input_EnableKeyboard();
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1888>";
				if(bb_challengergui_CHGUI_Keyboard==2){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1889>";
					bb_challengergui_CHGUI_ShowKeyboard=1;
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1890>";
					dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_Visible=1;
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1891>";
					bb_challengergui_CHGUI_ShiftHold=0;
				}
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1894>";
			if((bb_challengergui_CHGUI_AutoTextScroll)!=0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1895>";
				bb_challengergui_CHGUI_TargetX=(-bb_challengergui_CHGUI_RealX(t_N))+bb_challengergui_CHGUI_OffsetX+100.0;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1896>";
				bb_challengergui_CHGUI_TargetY=(-bb_challengergui_CHGUI_RealY(t_N))+bb_challengergui_CHGUI_OffsetY+100.0;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1897>";
				bb_challengergui_CHGUI_Moving=1;
			}
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1903>";
			var t_C=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1904>";
			for(t_C=0;t_C<=dbg_object(t_N).f_Text.length-1;t_C=t_C+1){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1905>";
				if(bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text.slice(0,t_C)+"NON")-bb_challengergui_CHGUI_Font.m_GetTxtWidth2("NON")>bb_input_TouchX(0)-(bb_challengergui_CHGUI_RealX(t_N))-10.0){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1905>";
					break;
				}
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1907>";
			dbg_object(t_N).f_Cursor=t_C;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1912>";
	if((dbg_object(t_N).f_OnFocus)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1913>";
		if(bb_challengergui_CHGUI_Keyboard==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1914>";
			bb_challengergui_CHGUI_GetText(t_N);
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1915>";
			if(bb_challengergui_CHGUI_Keyboard==2){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1916>";
				bb_challengergui_CHGUI_UpdateKeyboard(t_N);
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1918>";
				bb_challengergui_CHGUI_GetText(t_N);
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1927>";
	if(((bb_input_TouchDown(0))!=0) && dbg_object(t_N).f_Over==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1928>";
		if(bb_challengergui_CHGUI_Keyboard==2){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1929>";
			if(bb_input_TouchY(0)<(bb_graphics_DeviceHeight())-dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_H){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1930>";
				dbg_object(t_N).f_OnFocus=0;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1931>";
				if(bb_challengergui_CHGUI_Keyboard==1){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1932>";
					bb_input_DisableKeyboard();
				}else{
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1933>";
					if(bb_challengergui_CHGUI_Keyboard==2){
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1934>";
						bb_challengergui_CHGUI_ShowKeyboard=0;
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1935>";
						dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_Visible=0;
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1936>";
						bb_challengergui_CHGUI_KeyboardPage=0;
					}
				}
			}
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1940>";
			dbg_object(t_N).f_OnFocus=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1941>";
			if(bb_challengergui_CHGUI_Keyboard==1){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1942>";
				bb_input_DisableKeyboard();
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1943>";
				if(bb_challengergui_CHGUI_Keyboard==2){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1944>";
					bb_challengergui_CHGUI_ShowKeyboard=0;
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1945>";
					dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_Visible=0;
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1946>";
					bb_challengergui_CHGUI_KeyboardPage=0;
				}
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1951>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1952>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1952>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateHSlider(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1640>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1641>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1642>";
	if(dbg_object(t_N).f_Value<dbg_object(t_N).f_Minimum){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1642>";
		dbg_object(t_N).f_Value=dbg_object(t_N).f_Minimum;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1643>";
	if(dbg_object(t_N).f_Value>dbg_object(t_N).f_Maximum){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1643>";
		dbg_object(t_N).f_Value=dbg_object(t_N).f_Maximum;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1644>";
	var t_X=bb_challengergui_CHGUI_RealX(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1645>";
	var t_Y=bb_challengergui_CHGUI_RealY(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1646>";
	var t_W=((dbg_object(t_N).f_W)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1647>";
	var t_H=((dbg_object(t_N).f_H)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1650>";
	dbg_object(t_N).f_Stp=(dbg_object(t_N).f_W-2.0*dbg_object(t_N).f_H)/(dbg_object(t_N).f_Maximum-dbg_object(t_N).f_Minimum);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1651>";
	dbg_object(t_N).f_SWidth=dbg_object(t_N).f_Stp;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1652>";
	if(dbg_object(t_N).f_SWidth<dbg_object(t_N).f_H){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1652>";
		dbg_object(t_N).f_SWidth=dbg_object(t_N).f_H;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1653>";
	if(dbg_object(t_N).f_SWidth>dbg_object(t_N).f_W-dbg_object(t_N).f_H-dbg_object(t_N).f_H-10.0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1653>";
		dbg_object(t_N).f_SWidth=dbg_object(t_N).f_W-dbg_object(t_N).f_H-dbg_object(t_N).f_H-10.0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1654>";
	dbg_object(t_N).f_Stp=(dbg_object(t_N).f_W-dbg_object(t_N).f_SWidth-dbg_object(t_N).f_H-dbg_object(t_N).f_H)/(dbg_object(t_N).f_Maximum-dbg_object(t_N).f_Minimum);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1655>";
	dbg_object(t_N).f_SWidth=dbg_object(t_N).f_SWidth+10.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1658>";
	if(((dbg_object(t_N).f_MinusOver)!=0) && ((dbg_object(t_N).f_MinusDown)!=0) && bb_input_TouchDown(0)==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1659>";
		dbg_object(t_N).f_MinusOver=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1660>";
		dbg_object(t_N).f_MinusDown=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1661>";
		dbg_object(t_N).f_Value=dbg_object(t_N).f_Value-1.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1662>";
		if(dbg_object(t_N).f_Value<dbg_object(t_N).f_Minimum){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1662>";
			dbg_object(t_N).f_Value=dbg_object(t_N).f_Minimum;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1666>";
	if(((dbg_object(t_N).f_Over)!=0) && bb_input_TouchX(0)>(t_X) && bb_input_TouchX(0)<(t_X+t_H) && bb_input_TouchY(0)>(t_Y) && bb_input_TouchY(0)<(t_Y+t_H)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1667>";
		if(bb_challengergui_CHGUI_MouseBusy==0 || ((dbg_object(t_N).f_MinusDown)!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1668>";
			dbg_object(t_N).f_MinusOver=1;
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1671>";
		dbg_object(t_N).f_MinusOver=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1675>";
	if(((dbg_object(t_N).f_MinusOver)!=0) || ((dbg_object(t_N).f_MinusDown)!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1676>";
		if((bb_input_TouchDown(0))!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1677>";
			dbg_object(t_N).f_MinusDown=1;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1679>";
			dbg_object(t_N).f_MinusDown=0;
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1682>";
		dbg_object(t_N).f_MinusDown=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1686>";
	if(((dbg_object(t_N).f_PlusOver)!=0) && ((dbg_object(t_N).f_PlusDown)!=0) && bb_input_TouchDown(0)==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1687>";
		dbg_object(t_N).f_PlusOver=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1688>";
		dbg_object(t_N).f_PlusDown=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1689>";
		dbg_object(t_N).f_Value=dbg_object(t_N).f_Value+1.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1690>";
		if(dbg_object(t_N).f_Value>dbg_object(t_N).f_Maximum){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1690>";
			dbg_object(t_N).f_Value=dbg_object(t_N).f_Maximum;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1694>";
	if(bb_input_TouchX(0)>(t_X+t_W-t_H) && bb_input_TouchX(0)<(t_X+t_W) && bb_input_TouchY(0)>(t_Y) && bb_input_TouchY(0)<(t_Y+t_H)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1695>";
		if(bb_challengergui_CHGUI_MouseBusy==0 || ((dbg_object(t_N).f_PlusDown)!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1696>";
			dbg_object(t_N).f_PlusOver=1;
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1699>";
		dbg_object(t_N).f_PlusOver=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1703>";
	if(((dbg_object(t_N).f_PlusOver)!=0) || ((dbg_object(t_N).f_PlusDown)!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1704>";
		if((bb_input_TouchDown(0))!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1705>";
			dbg_object(t_N).f_PlusDown=1;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1707>";
			dbg_object(t_N).f_PlusDown=0;
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1710>";
		dbg_object(t_N).f_PlusDown=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1714>";
	var t_XPOS=(t_X+t_H-5)+(dbg_object(t_N).f_Value-dbg_object(t_N).f_Minimum)*dbg_object(t_N).f_Stp;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1716>";
	if(((dbg_object(t_N).f_Over)!=0) && bb_input_TouchX(0)>t_XPOS && bb_input_TouchX(0)<t_XPOS+dbg_object(t_N).f_SWidth && dbg_object(t_N).f_PlusOver==0 && dbg_object(t_N).f_MinusOver==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1717>";
		if(bb_challengergui_CHGUI_MouseBusy==0 || ((dbg_object(t_N).f_SliderDown)!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1718>";
			dbg_object(t_N).f_SliderOver=1;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1719>";
			if(dbg_object(t_N).f_SliderDown==0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1719>";
				dbg_object(t_N).f_Start=((bb_input_TouchX(0))|0);
			}
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1722>";
		dbg_object(t_N).f_SliderOver=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1725>";
	if(((dbg_object(t_N).f_SliderOver)!=0) || ((dbg_object(t_N).f_SliderDown)!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1726>";
		if((bb_input_TouchDown(0))!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1727>";
			dbg_object(t_N).f_SliderDown=1;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1729>";
			dbg_object(t_N).f_SliderDown=0;
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1732>";
		dbg_object(t_N).f_SliderDown=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1735>";
	if((dbg_object(t_N).f_SliderDown)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1736>";
		var t_Change=bb_input_TouchX(0)-(dbg_object(t_N).f_Start);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1737>";
		dbg_object(t_N).f_Value=dbg_object(t_N).f_Value+t_Change/dbg_object(t_N).f_Stp;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1738>";
		dbg_object(t_N).f_Start=((bb_input_TouchX(0))|0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1739>";
		if(dbg_object(t_N).f_Value<dbg_object(t_N).f_Minimum){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1739>";
			dbg_object(t_N).f_Value=dbg_object(t_N).f_Minimum;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1740>";
		if(dbg_object(t_N).f_Value>dbg_object(t_N).f_Maximum){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1740>";
			dbg_object(t_N).f_Value=dbg_object(t_N).f_Maximum;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1743>";
	if(dbg_object(t_N).f_SliderDown==0 && dbg_object(t_N).f_MinusDown==0 && dbg_object(t_N).f_PlusDown==0 && ((dbg_object(t_N).f_Down)!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1744>";
		dbg_object(t_N).f_Value=(bb_input_TouchX(0)-(t_X)-(t_H)-(t_H)+10.0)/dbg_object(t_N).f_Stp+dbg_object(t_N).f_Minimum;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1745>";
		if(dbg_object(t_N).f_Value<dbg_object(t_N).f_Minimum){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1745>";
			dbg_object(t_N).f_Value=dbg_object(t_N).f_Minimum;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1746>";
		if(dbg_object(t_N).f_Value>dbg_object(t_N).f_Maximum){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1746>";
			dbg_object(t_N).f_Value=dbg_object(t_N).f_Maximum;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1749>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1750>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1750>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateVSlider(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1756>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1757>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1758>";
	if(dbg_object(t_N).f_Value<dbg_object(t_N).f_Minimum){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1758>";
		dbg_object(t_N).f_Value=dbg_object(t_N).f_Minimum;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1759>";
	if(dbg_object(t_N).f_Value>dbg_object(t_N).f_Maximum){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1759>";
		dbg_object(t_N).f_Value=dbg_object(t_N).f_Maximum;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1760>";
	var t_X=bb_challengergui_CHGUI_RealX(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1761>";
	var t_Y=bb_challengergui_CHGUI_RealY(t_N);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1762>";
	var t_W=((dbg_object(t_N).f_W)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1763>";
	var t_H=((dbg_object(t_N).f_H)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1766>";
	dbg_object(t_N).f_Stp=(dbg_object(t_N).f_H-2.0*dbg_object(t_N).f_W)/(dbg_object(t_N).f_Maximum-dbg_object(t_N).f_Minimum);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1767>";
	dbg_object(t_N).f_SWidth=dbg_object(t_N).f_Stp;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1768>";
	if(dbg_object(t_N).f_SWidth<dbg_object(t_N).f_W){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1768>";
		dbg_object(t_N).f_SWidth=dbg_object(t_N).f_W;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1769>";
	if(dbg_object(t_N).f_SWidth>dbg_object(t_N).f_H-dbg_object(t_N).f_W-dbg_object(t_N).f_W-10.0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1769>";
		dbg_object(t_N).f_SWidth=dbg_object(t_N).f_H-dbg_object(t_N).f_W-dbg_object(t_N).f_W-10.0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1770>";
	dbg_object(t_N).f_Stp=(dbg_object(t_N).f_H-dbg_object(t_N).f_SWidth-dbg_object(t_N).f_W-dbg_object(t_N).f_W)/(dbg_object(t_N).f_Maximum-dbg_object(t_N).f_Minimum);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1771>";
	dbg_object(t_N).f_SWidth=dbg_object(t_N).f_SWidth+10.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1775>";
	if(((dbg_object(t_N).f_MinusOver)!=0) && ((dbg_object(t_N).f_MinusDown)!=0) && bb_input_TouchDown(0)==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1776>";
		dbg_object(t_N).f_MinusOver=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1777>";
		dbg_object(t_N).f_MinusDown=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1778>";
		dbg_object(t_N).f_Value=dbg_object(t_N).f_Value-1.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1779>";
		if(dbg_object(t_N).f_Value<dbg_object(t_N).f_Minimum){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1779>";
			dbg_object(t_N).f_Value=dbg_object(t_N).f_Minimum;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1783>";
	if(bb_input_TouchX(0)>(t_X) && bb_input_TouchX(0)<(t_X+t_W) && bb_input_TouchY(0)>(t_Y) && bb_input_TouchY(0)<(t_Y+t_W)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1784>";
		if(bb_challengergui_CHGUI_MouseBusy==0 || ((dbg_object(t_N).f_MinusDown)!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1785>";
			dbg_object(t_N).f_MinusOver=1;
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1788>";
		dbg_object(t_N).f_MinusOver=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1792>";
	if(((dbg_object(t_N).f_MinusOver)!=0) || ((dbg_object(t_N).f_MinusDown)!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1793>";
		if((bb_input_TouchDown(0))!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1794>";
			dbg_object(t_N).f_MinusDown=1;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1796>";
			dbg_object(t_N).f_MinusDown=0;
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1799>";
		dbg_object(t_N).f_MinusDown=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1803>";
	if(((dbg_object(t_N).f_PlusOver)!=0) && ((dbg_object(t_N).f_PlusDown)!=0) && bb_input_TouchDown(0)==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1804>";
		dbg_object(t_N).f_PlusOver=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1805>";
		dbg_object(t_N).f_PlusDown=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1806>";
		dbg_object(t_N).f_Value=dbg_object(t_N).f_Value+1.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1807>";
		if(dbg_object(t_N).f_Value>dbg_object(t_N).f_Maximum){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1807>";
			dbg_object(t_N).f_Value=dbg_object(t_N).f_Maximum;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1811>";
	if(((dbg_object(t_N).f_Over)!=0) && bb_input_TouchX(0)>(t_X) && bb_input_TouchX(0)<(t_X+t_W) && bb_input_TouchY(0)>(t_Y+t_H-t_W) && bb_input_TouchY(0)<(t_Y+t_H)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1812>";
		if(bb_challengergui_CHGUI_MouseBusy==0 || ((dbg_object(t_N).f_PlusDown)!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1813>";
			dbg_object(t_N).f_PlusOver=1;
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1816>";
		dbg_object(t_N).f_PlusOver=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1820>";
	if(((dbg_object(t_N).f_PlusOver)!=0) || ((dbg_object(t_N).f_PlusDown)!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1821>";
		if((bb_input_TouchDown(0))!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1822>";
			dbg_object(t_N).f_PlusDown=1;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1824>";
			dbg_object(t_N).f_PlusDown=0;
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1827>";
		dbg_object(t_N).f_PlusDown=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1831>";
	var t_YPOS=(t_Y+t_W-5)+(dbg_object(t_N).f_Value-dbg_object(t_N).f_Minimum)*dbg_object(t_N).f_Stp;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1833>";
	if(((dbg_object(t_N).f_Over)!=0) && bb_input_TouchY(0)>t_YPOS && bb_input_TouchY(0)<t_YPOS+dbg_object(t_N).f_SWidth && dbg_object(t_N).f_PlusOver==0 && dbg_object(t_N).f_MinusOver==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1834>";
		if(bb_challengergui_CHGUI_MouseBusy==0 || ((dbg_object(t_N).f_SliderDown)!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1835>";
			dbg_object(t_N).f_SliderOver=1;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1836>";
			if(dbg_object(t_N).f_SliderDown==0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1836>";
				dbg_object(t_N).f_Start=((bb_input_TouchY(0))|0);
			}
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1839>";
		dbg_object(t_N).f_SliderOver=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1842>";
	if(((dbg_object(t_N).f_SliderOver)!=0) || ((dbg_object(t_N).f_SliderDown)!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1843>";
		if((bb_input_TouchDown(0))!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1844>";
			dbg_object(t_N).f_SliderDown=1;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1846>";
			dbg_object(t_N).f_SliderDown=0;
		}
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1849>";
		dbg_object(t_N).f_SliderDown=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1852>";
	if((dbg_object(t_N).f_SliderDown)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1853>";
		var t_Change=bb_input_TouchY(0)-(dbg_object(t_N).f_Start);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1854>";
		dbg_object(t_N).f_Value=dbg_object(t_N).f_Value+t_Change/dbg_object(t_N).f_Stp;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1855>";
		dbg_object(t_N).f_Start=((bb_input_TouchY(0))|0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1856>";
		if(dbg_object(t_N).f_Value<dbg_object(t_N).f_Minimum){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1856>";
			dbg_object(t_N).f_Value=dbg_object(t_N).f_Minimum;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1857>";
		if(dbg_object(t_N).f_Value>dbg_object(t_N).f_Maximum){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1857>";
			dbg_object(t_N).f_Value=dbg_object(t_N).f_Maximum;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1860>";
	if(dbg_object(t_N).f_SliderDown==0 && dbg_object(t_N).f_MinusDown==0 && dbg_object(t_N).f_PlusDown==0 && ((dbg_object(t_N).f_Down)!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1861>";
		dbg_object(t_N).f_Value=(bb_input_TouchY(0)-(t_Y)-(t_W)-(t_W)+10.0)/dbg_object(t_N).f_Stp+dbg_object(t_N).f_Minimum;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1862>";
		if(dbg_object(t_N).f_Value<dbg_object(t_N).f_Minimum){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1862>";
			dbg_object(t_N).f_Value=dbg_object(t_N).f_Minimum;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1863>";
		if(dbg_object(t_N).f_Value>dbg_object(t_N).f_Maximum){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1863>";
			dbg_object(t_N).f_Value=dbg_object(t_N).f_Maximum;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1866>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1867>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1867>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateListboxItem(t_N,t_C){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2060>";
	dbg_object(t_N).f_X=0.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2061>";
	dbg_object(t_N).f_Y=(t_C*dbg_object(dbg_object(t_N).f_Parent).f_ListHeight);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2062>";
	dbg_object(t_N).f_W=dbg_object(dbg_object(t_N).f_Parent).f_W;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2063>";
	dbg_object(t_N).f_H=(dbg_object(dbg_object(t_N).f_Parent).f_ListHeight);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2065>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2066>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2068>";
	if((dbg_object(t_N).f_Clicked)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2069>";
		dbg_object(dbg_object(t_N).f_Parent).f_Text=dbg_object(t_N).f_Text;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2070>";
		dbg_object(dbg_object(t_N).f_Parent).f_SelectedListboxItem=t_N;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2071>";
		dbg_object(dbg_object(t_N).f_Parent).f_Value=dbg_object(t_N).f_Value;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2074>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2075>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2075>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateListbox(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2039>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2040>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2042>";
	dbg_object(t_N).f_ListHeight=bb_challengergui_CHGUI_Font.m_GetFontHeight()+10;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2043>";
	dbg_object(t_N).f_ListboxNumber=((dbg_object(t_N).f_H/(dbg_object(t_N).f_ListHeight)-1.0)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2045>";
	dbg_object(dbg_object(t_N).f_ListboxSlider).f_Minimum=0.0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2046>";
	dbg_object(dbg_object(t_N).f_ListboxSlider).f_Maximum=(dbg_object(t_N).f_ListboxItems.length-dbg_object(t_N).f_ListboxNumber-1);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2048>";
	if(dbg_object(dbg_object(t_N).f_ListboxSlider).f_Maximum<1.0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2049>";
		dbg_object(dbg_object(t_N).f_ListboxSlider).f_Visible=0;
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2051>";
		dbg_object(dbg_object(t_N).f_ListboxSlider).f_Visible=1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2054>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2055>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2055>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateRadiobox(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1471>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1472>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1473>";
	dbg_object(t_N).f_W=dbg_object(t_N).f_H+dbg_object(t_N).f_H/4.0+bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1474>";
	if((dbg_object(t_N).f_Clicked)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1475>";
		dbg_object(t_N).f_Value=1.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1476>";
		for(var t_X=0;t_X<=dbg_object(dbg_object(t_N).f_Parent).f_Radioboxes.length-1;t_X=t_X+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1477>";
			if(dbg_object(dbg_array(dbg_object(dbg_object(t_N).f_Parent).f_Radioboxes,t_X)[dbg_index]).f_Group==dbg_object(t_N).f_Group && dbg_array(dbg_object(dbg_object(t_N).f_Parent).f_Radioboxes,t_X)[dbg_index]!=t_N){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1478>";
				dbg_object(dbg_array(dbg_object(dbg_object(t_N).f_Parent).f_Radioboxes,t_X)[dbg_index]).f_Value=0.0;
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1483>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1484>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1484>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateTickbox(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1452>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1453>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1454>";
	dbg_object(t_N).f_W=dbg_object(t_N).f_H+dbg_object(t_N).f_H/4.0+bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(t_N).f_Text);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1455>";
	if((dbg_object(t_N).f_Clicked)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1456>";
		if(dbg_object(t_N).f_Value==0.0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1457>";
			dbg_object(t_N).f_Value=1.0;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1459>";
			dbg_object(t_N).f_Value=0.0;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1464>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1465>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1465>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateImageButton(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1441>";
	dbg_object(t_N).f_W=((dbg_object(t_N).f_Img.m_Width()/4)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1442>";
	dbg_object(t_N).f_H=(dbg_object(t_N).f_Img.m_Height());
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1443>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1444>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1446>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1447>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1447>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateButton(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1432>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1433>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1435>";
	if(bb_challengergui_CHGUI_Tooltips==1 && dbg_object(t_N).f_Tooltip!="" && dbg_object(t_N).f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1436>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1436>";
			bb_challengergui_CHGUI_TooltipFlag=t_N;
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_Locked(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4032>";
	var t_E=null;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4033>";
	t_E=t_N;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4034>";
	if(t_E==bb_challengergui_CHGUI_LockedWIndow){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4034>";
		pop_err();
		return 1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4035>";
	do{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4036>";
		if(dbg_object(t_E).f_Parent!=null){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4037>";
			if(dbg_object(t_E).f_Parent==bb_challengergui_CHGUI_LockedWIndow){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4037>";
				pop_err();
				return 1;
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4038>";
			t_E=dbg_object(t_E).f_Parent;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4040>";
			pop_err();
			return 0;
		}
	}while(!(false));
}
function bb_challengergui_CHGUI_UpdateWindow(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1278>";
	var t_X=(bb_challengergui_CHGUI_RealX(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1279>";
	var t_Y=(bb_challengergui_CHGUI_RealY(t_N));
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1280>";
	var t_W=((dbg_object(t_N).f_W)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1281>";
	var t_H=((dbg_object(t_N).f_H)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1283>";
	if((dbg_object(t_N).f_Minimised)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1283>";
		t_H=((bb_challengergui_CHGUI_TitleHeight)|0);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1284>";
	var t_TH=((bb_challengergui_CHGUI_TitleHeight)|0);
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1286>";
	if(dbg_object(t_N).f_Text==""){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1287>";
		dbg_object(t_N).f_Close=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1288>";
		dbg_object(t_N).f_Minimise=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1291>";
	t_N.m_CheckOver();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1292>";
	t_N.m_CheckDown();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1295>";
	if((dbg_object(t_N).f_Over)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1296>";
		if(bb_input_TouchY(0)>t_Y && bb_input_TouchY(0)<t_Y+bb_challengergui_CHGUI_TitleHeight){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1297>";
			bb_challengergui_CHGUI_DragOver=1;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1304>";
	if((dbg_object(t_N).f_Close)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1305>";
		var t_TH2=bb_challengergui_CHGUI_TitleHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1306>";
		if(((dbg_object(t_N).f_CloseOver)!=0) && ((dbg_object(t_N).f_CloseDown)!=0) && bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1307>";
			dbg_object(t_N).f_CloseOver=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1308>";
			dbg_object(t_N).f_CloseDown=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1309>";
			dbg_object(t_N).f_Visible=0;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1311>";
		if(((dbg_object(t_N).f_Over)!=0) && bb_input_TouchX(0)>t_X+(t_W)-t_TH2/2.5-10.0 && bb_input_TouchX(0)<t_X+(t_W)-t_TH2/2.5-10.0+t_TH2/2.5 && bb_input_TouchY(0)>t_Y+(t_TH2-t_TH2/2.5)/2.0 && bb_input_TouchY(0)<t_Y+(t_TH2-t_TH2/2.5)/2.0+t_TH2/2.5){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1312>";
			if(bb_challengergui_CHGUI_MouseBusy==0 || ((dbg_object(t_N).f_CloseDown)!=0)){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1313>";
				dbg_object(t_N).f_CloseOver=1;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1314>";
				bb_challengergui_CHGUI_DragOver=1;
			}
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1317>";
			dbg_object(t_N).f_CloseOver=0;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1319>";
		if(((dbg_object(t_N).f_CloseOver)!=0) || ((dbg_object(t_N).f_CloseDown)!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1320>";
			if((bb_input_TouchDown(0))!=0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1321>";
				dbg_object(t_N).f_CloseDown=1;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1322>";
				bb_challengergui_CHGUI_DragOver=1;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1324>";
				dbg_object(t_N).f_CloseDown=0;
			}
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1327>";
			dbg_object(t_N).f_CloseDown=0;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1332>";
	if((dbg_object(t_N).f_Minimise)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1333>";
		var t_TH1=bb_challengergui_CHGUI_TitleHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1334>";
		var t_Off2=(((t_TH1-t_TH1/2.0)/2.0)|0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1336>";
		if(((dbg_object(t_N).f_MinimiseOver)!=0) && ((dbg_object(t_N).f_MinimiseDown)!=0) && bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1337>";
			dbg_object(t_N).f_CloseOver=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1338>";
			dbg_object(t_N).f_CloseDown=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1339>";
			if(dbg_object(t_N).f_Minimised==0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1340>";
				dbg_object(t_N).f_Minimised=1;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1342>";
				dbg_object(t_N).f_Minimised=0;
			}
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1346>";
		if(((dbg_object(t_N).f_Over)!=0) && bb_input_TouchX(0)>t_X+(t_W)-((t_TH)/2.5+(t_TH)/2.5)-(t_TH)/1.5 && bb_input_TouchX(0)<t_X+(t_W)-((t_TH)/2.5+(t_TH)/2.5)-(t_TH)/1.5+(t_TH)/2.5 && bb_input_TouchY(0)>t_Y+((t_TH)-(t_TH)/2.5)/2.0 && bb_input_TouchY(0)<t_Y+((t_TH)-(t_TH)/2.5)/2.0+(t_TH)/2.5){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1347>";
			if(bb_challengergui_CHGUI_MouseBusy==0 || ((dbg_object(t_N).f_MinimiseDown)!=0)){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1348>";
				dbg_object(t_N).f_MinimiseOver=1;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1349>";
				bb_challengergui_CHGUI_DragOver=1;
			}
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1352>";
			dbg_object(t_N).f_MinimiseOver=0;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1354>";
		if(((dbg_object(t_N).f_MinimiseOver)!=0) || ((dbg_object(t_N).f_MinimiseDown)!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1355>";
			if((bb_input_TouchDown(0))!=0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1356>";
				dbg_object(t_N).f_MinimiseDown=1;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1357>";
				bb_challengergui_CHGUI_DragOver=1;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1359>";
				dbg_object(t_N).f_MinimiseDown=0;
			}
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1362>";
			dbg_object(t_N).f_MinimiseDown=0;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1367>";
	if(dbg_object(t_N).f_Moveable==1 && ((dbg_object(t_N).f_Over)!=0) && dbg_object(t_N).f_Moving==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1368>";
		if(bb_input_TouchY(0)>t_Y && bb_input_TouchY(0)<t_Y+bb_challengergui_CHGUI_TitleHeight && ((bb_input_TouchDown(0))!=0) && dbg_object(t_N).f_Text!=""){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1369>";
			if(dbg_object(t_N).f_CloseOver==0 && dbg_object(t_N).f_MinimiseOver==0 && dbg_object(t_N).f_CloseDown==0 && dbg_object(t_N).f_MinimiseDown==0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1370>";
				if(bb_challengergui_CHGUI_Moving==0){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1371>";
					dbg_object(t_N).f_Moving=1;
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1372>";
					dbg_object(t_N).f_MX=bb_input_TouchX(0);
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1373>";
					dbg_object(t_N).f_MY=bb_input_TouchY(0);
				}
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1378>";
	if(bb_input_TouchDown(0)==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1378>";
		dbg_object(t_N).f_Moving=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1380>";
	if(dbg_object(t_N).f_Moving==1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1381>";
		dbg_object(t_N).f_X=dbg_object(t_N).f_X+(bb_input_TouchX(0)-dbg_object(t_N).f_MX);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1382>";
		dbg_object(t_N).f_Y=dbg_object(t_N).f_Y+(bb_input_TouchY(0)-dbg_object(t_N).f_MY);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1383>";
		dbg_object(t_N).f_MX=bb_input_TouchX(0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1384>";
		dbg_object(t_N).f_MY=bb_input_TouchY(0);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1385>";
		bb_challengergui_CHGUI_DragOver=1;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1387>";
		var t_RP=dbg_object(t_N).f_Parent;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1389>";
		do{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1390>";
			if(dbg_object(t_RP).f_Element!="Tab"){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1390>";
				break;
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1391>";
			if(dbg_object(t_RP).f_Parent!=null){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1392>";
				t_RP=dbg_object(t_RP).f_Parent;
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1394>";
				break;
			}
		}while(!(false));
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1399>";
		if(dbg_object(t_N).f_X<0.0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1399>";
			dbg_object(t_N).f_X=0.0;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1401>";
		if(dbg_object(t_N).f_X>dbg_object(t_RP).f_W-dbg_object(t_N).f_W){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1401>";
			dbg_object(t_N).f_X=dbg_object(t_RP).f_W-dbg_object(t_N).f_W;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1403>";
		if(dbg_object(t_N).f_Y>dbg_object(t_RP).f_H-(t_H)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1403>";
			dbg_object(t_N).f_Y=dbg_object(t_RP).f_H-(t_H);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1405>";
		var t_YVal=dbg_object(t_RP).f_MenuHeight;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1406>";
		if(dbg_object(t_RP).f_Text!=""){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1406>";
			t_YVal=(((t_YVal)+bb_challengergui_CHGUI_TitleHeight)|0);
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1407>";
		if((dbg_object(t_RP).f_Tabbed)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1407>";
			t_YVal=t_YVal+dbg_object(t_RP).f_TabHeight+5;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1409>";
		if(dbg_object(t_N).f_Y<(t_YVal)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1409>";
			dbg_object(t_N).f_Y=(t_YVal);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1414>";
	if(((dbg_object(t_N).f_Clicked)!=0) && bb_input_TouchY(0)>t_Y && bb_input_TouchY(0)<t_Y+bb_challengergui_CHGUI_TitleHeight && dbg_object(t_N).f_Text!=""){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1415>";
		if(((dbg_object(t_N).f_Minimise)!=0) && dbg_object(t_N).f_CloseOver==0 && dbg_object(t_N).f_MinimiseOver==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1416>";
			if(dbg_object(t_N).f_DClickMillisecs>bb_app_Millisecs()){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1417>";
				if(dbg_object(t_N).f_Minimised==1){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1418>";
					dbg_object(t_N).f_Minimised=0;
				}else{
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1420>";
					dbg_object(t_N).f_Minimised=1;
				}
			}else{
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1423>";
				dbg_object(t_N).f_DClickMillisecs=bb_app_Millisecs()+275;
			}
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateContents(t_N){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1145>";
	var t_X=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1146>";
	var t_XX=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1147>";
	var t_C=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1148>";
	dbg_object(t_N).f_ReOrdered=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1150>";
	for(t_X=dbg_object(t_N).f_Menus.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1151>";
		dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1152>";
		bb_challengergui_CHGUI_UpdateSubMenu(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index]);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1155>";
	if((dbg_object(t_N).f_Tabbed)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1156>";
		bb_challengergui_CHGUI_UpdateContents(dbg_object(t_N).f_CurrentTab);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1160>";
	for(var t_NN=dbg_object(t_N).f_TopList.length-1;t_NN>=0;t_NN=t_NN+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1161>";
		bb_challengergui_CHGUI_UpdateContents(dbg_array(dbg_object(t_N).f_TopList,t_NN)[dbg_index]);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1164>";
	for(var t_NN2=dbg_object(t_N).f_VariList.length-1;t_NN2>=0;t_NN2=t_NN2+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1165>";
		bb_challengergui_CHGUI_UpdateContents(dbg_array(dbg_object(t_N).f_VariList,t_NN2)[dbg_index]);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1168>";
	for(var t_NN3=dbg_object(t_N).f_BottomList.length-1;t_NN3>=0;t_NN3=t_NN3+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1169>";
		bb_challengergui_CHGUI_UpdateContents(dbg_array(dbg_object(t_N).f_BottomList,t_NN3)[dbg_index]);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1173>";
	for(t_X=dbg_object(t_N).f_Tabs.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1174>";
		dbg_array(dbg_object(t_N).f_Tabs,t_X)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1175>";
		if(((bb_challengergui_CHGUI_RealActive(dbg_array(dbg_object(t_N).f_Tabs,t_X)[dbg_index]))!=0) && ((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Tabs,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Tabs,t_X)[dbg_index])==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1175>";
			bb_challengergui_CHGUI_UpdateTab(dbg_array(dbg_object(t_N).f_Tabs,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1180>";
	if((bb_challengergui_CHGUI_MenuClose)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1180>";
		dbg_object(t_N).f_MenuActive=null;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1181>";
	for(t_X=dbg_object(t_N).f_Menus.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1182>";
		dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1183>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealActive(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1183>";
			bb_challengergui_CHGUI_UpdateMenu(dbg_array(dbg_object(t_N).f_Menus,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1187>";
	for(t_X=dbg_object(t_N).f_Dropdowns.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1188>";
		t_C=0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1189>";
		for(t_XX=0;t_XX<=dbg_object(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]).f_DropdownItems.length-1;t_XX=t_XX+1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1190>";
			dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]).f_DropdownItems,t_XX)[dbg_index].m_CheckClicked();
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1191>";
			if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]).f_DropdownItems,t_XX)[dbg_index]))!=0) && ((dbg_object(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]).f_OnFocus)!=0) && ((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]))!=0)){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1191>";
				bb_challengergui_CHGUI_UpdateDropdownItem(dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]).f_DropdownItems,t_XX)[dbg_index],t_C);
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1192>";
			if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]).f_DropdownItems,t_XX)[dbg_index]))!=0) && ((dbg_object(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]).f_OnFocus)!=0) && ((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]))!=0)){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1192>";
				t_C=t_C+1;
			}
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1197>";
	for(t_X=dbg_object(t_N).f_Dropdowns.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1198>";
		dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1199>";
		if(((bb_challengergui_CHGUI_RealActive(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]))!=0) && ((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index])==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1199>";
			bb_challengergui_CHGUI_UpdateDropdown(dbg_array(dbg_object(t_N).f_Dropdowns,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1203>";
	for(t_X=dbg_object(t_N).f_Labels.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1204>";
		dbg_array(dbg_object(t_N).f_Labels,t_X)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1205>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Labels,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Labels,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealActive(dbg_array(dbg_object(t_N).f_Labels,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1205>";
			bb_challengergui_CHGUI_UpdateLabel(dbg_array(dbg_object(t_N).f_Labels,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1209>";
	for(t_X=dbg_object(t_N).f_Textfields.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1210>";
		dbg_array(dbg_object(t_N).f_Textfields,t_X)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1211>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Textfields,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Textfields,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealActive(dbg_array(dbg_object(t_N).f_Textfields,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1211>";
			bb_challengergui_CHGUI_UpdateTextfield(dbg_array(dbg_object(t_N).f_Textfields,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1215>";
	for(t_X=dbg_object(t_N).f_HSliders.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1216>";
		dbg_array(dbg_object(t_N).f_HSliders,t_X)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1217>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_HSliders,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_HSliders,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealActive(dbg_array(dbg_object(t_N).f_HSliders,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1217>";
			bb_challengergui_CHGUI_UpdateHSlider(dbg_array(dbg_object(t_N).f_HSliders,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1221>";
	for(t_X=dbg_object(t_N).f_VSliders.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1222>";
		dbg_array(dbg_object(t_N).f_VSliders,t_X)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1223>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_VSliders,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_VSliders,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealActive(dbg_array(dbg_object(t_N).f_VSliders,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1223>";
			bb_challengergui_CHGUI_UpdateVSlider(dbg_array(dbg_object(t_N).f_VSliders,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1227>";
	for(t_X=dbg_object(t_N).f_Listboxes.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1228>";
		dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1229>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealActive(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1230>";
			var t_C2=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1232>";
			for(t_XX=((dbg_object(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxSlider).f_Value)|0);(t_XX)<=dbg_object(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxSlider).f_Value+(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxNumber);t_XX=t_XX+1){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1233>";
				if(t_XX<dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxItems.length && t_XX>-1){
					err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1234>";
					if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxItems,t_XX)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxItems,t_XX)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealActive(dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxItems,t_XX)[dbg_index]))!=0)){
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1235>";
						dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxItems,t_XX)[dbg_index].m_CheckClicked();
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1236>";
						bb_challengergui_CHGUI_UpdateListboxItem(dbg_array(dbg_object(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]).f_ListboxItems,t_XX)[dbg_index],t_C2);
						err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1237>";
						t_C2=t_C2+1;
					}
				}
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1241>";
			bb_challengergui_CHGUI_UpdateListbox(dbg_array(dbg_object(t_N).f_Listboxes,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1246>";
	for(t_X=dbg_object(t_N).f_Radioboxes.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1247>";
		dbg_array(dbg_object(t_N).f_Radioboxes,t_X)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1248>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Radioboxes,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Radioboxes,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealActive(dbg_array(dbg_object(t_N).f_Radioboxes,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1248>";
			bb_challengergui_CHGUI_UpdateRadiobox(dbg_array(dbg_object(t_N).f_Radioboxes,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1252>";
	for(t_X=dbg_object(t_N).f_Tickboxes.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1253>";
		dbg_array(dbg_object(t_N).f_Tickboxes,t_X)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1254>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Tickboxes,t_X)[dbg_index]))!=0) && ((bb_challengergui_CHGUI_RealActive(dbg_array(dbg_object(t_N).f_Tickboxes,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Tickboxes,t_X)[dbg_index])==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1254>";
			bb_challengergui_CHGUI_UpdateTickbox(dbg_array(dbg_object(t_N).f_Tickboxes,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1258>";
	for(t_X=dbg_object(t_N).f_ImageButtons.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1259>";
		dbg_array(dbg_object(t_N).f_ImageButtons,t_X)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1260>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_ImageButtons,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_ImageButtons,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealActive(dbg_array(dbg_object(t_N).f_ImageButtons,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1260>";
			bb_challengergui_CHGUI_UpdateImageButton(dbg_array(dbg_object(t_N).f_ImageButtons,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1264>";
	for(t_X=dbg_object(t_N).f_Buttons.length-1;t_X>=0;t_X=t_X+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1265>";
		dbg_array(dbg_object(t_N).f_Buttons,t_X)[dbg_index].m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1266>";
		if(((bb_challengergui_CHGUI_RealVisible(dbg_array(dbg_object(t_N).f_Buttons,t_X)[dbg_index]))!=0) && bb_challengergui_CHGUI_RealMinimised(dbg_array(dbg_object(t_N).f_Buttons,t_X)[dbg_index])==0 && ((bb_challengergui_CHGUI_RealActive(dbg_array(dbg_object(t_N).f_Buttons,t_X)[dbg_index]))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1266>";
			bb_challengergui_CHGUI_UpdateButton(dbg_array(dbg_object(t_N).f_Buttons,t_X)[dbg_index]);
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1269>";
	if(dbg_object(t_N).f_Element!="Tab"){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1271>";
		t_N.m_CheckClicked();
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1272>";
		if(((bb_challengergui_CHGUI_RealVisible(t_N))!=0) && ((bb_challengergui_CHGUI_RealActive(t_N))!=0) || ((bb_challengergui_CHGUI_Locked(t_N))!=0)){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1272>";
			bb_challengergui_CHGUI_UpdateWindow(t_N);
		}
	}
	pop_err();
	return 0;
}
var bb_challengergui_CHGUI_CursorMillisecs;
var bb_challengergui_CHGUI_DragScroll;
var bb_challengergui_CHGUI_DragMoving;
var bb_challengergui_CHGUI_OffsetMX;
var bb_challengergui_CHGUI_OffsetMY;
function bb_challengergui_LockFocus(t_Window){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<873>";
	if(dbg_object(bb_challengergui_CHGUI_MsgBoxWindow).f_Visible==0 || t_Window==bb_challengergui_CHGUI_MsgBoxWindow){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<874>";
		bb_challengergui_CHGUI_LockedWIndow=t_Window;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<875>";
		bb_challengergui_CHGUI_Reorder(t_Window);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<876>";
		dbg_object(bb_challengergui_CHGUI_Canvas).f_Active=0;
	}
	pop_err();
	return 0;
}
function bb_challengergui_UnlockFocus(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<882>";
	if(dbg_object(bb_challengergui_CHGUI_MsgBoxWindow).f_Visible==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<883>";
		bb_challengergui_CHGUI_LockedWIndow=null;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<884>";
		dbg_object(bb_challengergui_CHGUI_Canvas).f_Active=1;
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_UpdateMsgBox(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3855>";
	if((dbg_object(bb_challengergui_CHGUI_MsgBoxWindow).f_Visible)!=0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3856>";
		bb_challengergui_LockFocus(bb_challengergui_CHGUI_MsgBoxWindow);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3858>";
		dbg_object(bb_challengergui_CHGUI_MsgBoxWindow).f_X=((bb_graphics_DeviceWidth()/2)|0)-dbg_object(bb_challengergui_CHGUI_MsgBoxWindow).f_W/2.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3859>";
		dbg_object(bb_challengergui_CHGUI_MsgBoxWindow).f_Y=((bb_graphics_DeviceHeight()/2)|0)-dbg_object(bb_challengergui_CHGUI_MsgBoxWindow).f_H/2.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3860>";
		dbg_object(bb_challengergui_CHGUI_MsgBoxLabel).f_X=(dbg_object(bb_challengergui_CHGUI_MsgBoxWindow).f_W-bb_challengergui_CHGUI_Font.m_GetTxtWidth2(dbg_object(bb_challengergui_CHGUI_MsgBoxLabel).f_Text))/2.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3862>";
		if((dbg_object(bb_challengergui_CHGUI_MsgBoxButton).f_Clicked)!=0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3863>";
			dbg_object(bb_challengergui_CHGUI_MsgBoxWindow).f_Visible=0;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3864>";
			bb_challengergui_UnlockFocus();
		}
	}
	pop_err();
	return 0;
}
function bb_challengergui_CHGUI_Update(){
	push_err();
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<743>";
	if(bb_input_TouchDown(0)==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<743>";
		bb_challengergui_CHGUI_MouseBusy=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<744>";
	bb_challengergui_CHGUI_Over=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<745>";
	bb_challengergui_CHGUI_OverFlag=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<746>";
	bb_challengergui_CHGUI_DownFlag=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<747>";
	bb_challengergui_CHGUI_MenuOver=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<748>";
	bb_challengergui_CHGUI_TextBoxOver=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<749>";
	bb_challengergui_CHGUI_TextboxOnFocus=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<750>";
	bb_challengergui_CHGUI_TextBoxDown=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<751>";
	bb_challengergui_CHGUI_DragOver=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<752>";
	bb_challengergui_CHGUI_TooltipFlag=null;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<753>";
	if(bb_challengergui_CHGUI_Canvas!=null){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<754>";
		dbg_object(bb_challengergui_CHGUI_Canvas).f_W=(bb_challengergui_CHGUI_Width);
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<755>";
		dbg_object(bb_challengergui_CHGUI_Canvas).f_H=(bb_challengergui_CHGUI_Height);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<759>";
	if(bb_challengergui_CHGUI_Moving==1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<760>";
		bb_challengergui_CHGUI_OffsetY=bb_challengergui_CHGUI_OffsetY-(bb_challengergui_CHGUI_OffsetY-bb_challengergui_CHGUI_TargetY)/8.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<761>";
		bb_challengergui_CHGUI_OffsetX=bb_challengergui_CHGUI_OffsetX-(bb_challengergui_CHGUI_OffsetX-bb_challengergui_CHGUI_TargetX)/8.0;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<762>";
		if(bb_challengergui_CHGUI_OffsetY-bb_challengergui_CHGUI_TargetY>-1.0 && bb_challengergui_CHGUI_OffsetY-bb_challengergui_CHGUI_TargetY<1.0 && bb_challengergui_CHGUI_OffsetX-bb_challengergui_CHGUI_TargetX>-1.0 && bb_challengergui_CHGUI_OffsetX-bb_challengergui_CHGUI_TargetX<1.0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<763>";
			bb_challengergui_CHGUI_OffsetY=bb_challengergui_CHGUI_TargetY;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<764>";
			bb_challengergui_CHGUI_OffsetX=bb_challengergui_CHGUI_TargetX;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<765>";
			bb_challengergui_CHGUI_Moving=0;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<769>";
	for(var t_N=bb_challengergui_CHGUI_TopList.length-1;t_N>=0;t_N=t_N+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<770>";
		bb_challengergui_CHGUI_UpdateContents(dbg_array(bb_challengergui_CHGUI_TopList,t_N)[dbg_index]);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<772>";
	for(var t_N2=bb_challengergui_CHGUI_VariList.length-1;t_N2>=0;t_N2=t_N2+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<773>";
		bb_challengergui_CHGUI_UpdateContents(dbg_array(bb_challengergui_CHGUI_VariList,t_N2)[dbg_index]);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<775>";
	for(var t_N3=bb_challengergui_CHGUI_BottomList.length-1;t_N3>=0;t_N3=t_N3+-1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<776>";
		bb_challengergui_CHGUI_UpdateContents(dbg_array(bb_challengergui_CHGUI_BottomList,t_N3)[dbg_index]);
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<781>";
	if(((bb_input_TouchDown(0))!=0) && bb_challengergui_CHGUI_DownFlag==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<782>";
		bb_challengergui_CHGUI_IgnoreMouse=1;
	}else{
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<784>";
		bb_challengergui_CHGUI_IgnoreMouse=0;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<786>";
	bb_challengergui_CHGUI_MenuClose=0;
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<787>";
	if(bb_challengergui_CHGUI_MenuOver==0 && ((bb_input_TouchDown(0))!=0)){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<787>";
		bb_challengergui_CHGUI_MenuClose=1;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<788>";
	if(bb_challengergui_CHGUI_CursorMillisecs<bb_app_Millisecs()){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<789>";
		bb_challengergui_CHGUI_CursorMillisecs=bb_app_Millisecs()+300;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<790>";
		if(bb_challengergui_CHGUI_Cursor==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<791>";
			bb_challengergui_CHGUI_Cursor=1;
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<793>";
			bb_challengergui_CHGUI_Cursor=0;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<797>";
	if(bb_challengergui_CHGUI_DragScroll==1){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<798>";
		if(bb_challengergui_CHGUI_DragOver==0 && ((bb_input_TouchDown(0))!=0) && bb_challengergui_CHGUI_DragMoving==0 && bb_challengergui_CHGUI_TextboxOnFocus==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<799>";
			bb_challengergui_CHGUI_OffsetMX=bb_input_TouchX(0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<800>";
			bb_challengergui_CHGUI_OffsetMY=bb_input_TouchY(0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<801>";
			bb_challengergui_CHGUI_DragMoving=1;
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<804>";
		if(bb_challengergui_CHGUI_DragMoving==1){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<805>";
			bb_challengergui_CHGUI_OffsetX=bb_challengergui_CHGUI_OffsetX+(bb_input_TouchX(0)-bb_challengergui_CHGUI_OffsetMX);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<806>";
			bb_challengergui_CHGUI_OffsetY=bb_challengergui_CHGUI_OffsetY+(bb_input_TouchY(0)-bb_challengergui_CHGUI_OffsetMY);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<807>";
			bb_challengergui_CHGUI_OffsetMX=bb_input_TouchX(0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<808>";
			bb_challengergui_CHGUI_OffsetMY=bb_input_TouchY(0);
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<811>";
			if(bb_challengergui_CHGUI_OffsetX<(-1*(bb_challengergui_CHGUI_Width-bb_graphics_DeviceWidth()))){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<811>";
				bb_challengergui_CHGUI_OffsetX=(-1*(bb_challengergui_CHGUI_Width-bb_graphics_DeviceWidth()));
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<812>";
			if(bb_challengergui_CHGUI_OffsetX>0.0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<812>";
				bb_challengergui_CHGUI_OffsetX=0.0;
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<814>";
			if(bb_challengergui_CHGUI_OffsetY<(bb_graphics_DeviceHeight()-bb_challengergui_CHGUI_Height)){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<814>";
				bb_challengergui_CHGUI_OffsetY=(bb_graphics_DeviceHeight()-bb_challengergui_CHGUI_Height);
			}
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<815>";
			if(bb_challengergui_CHGUI_OffsetY>0.0){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<815>";
				bb_challengergui_CHGUI_OffsetY=0.0;
			}
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<818>";
		if(bb_challengergui_CHGUI_Width<bb_graphics_DeviceWidth()){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<818>";
			bb_challengergui_CHGUI_OffsetX=(((bb_graphics_DeviceWidth()/2)|0)-((bb_challengergui_CHGUI_Width/2)|0));
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<819>";
		if(bb_challengergui_CHGUI_Height<bb_graphics_DeviceHeight()){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<819>";
			bb_challengergui_CHGUI_OffsetY=(((bb_graphics_DeviceHeight()/2)|0)-((bb_challengergui_CHGUI_Height/2)|0));
		}
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<822>";
		if(bb_input_TouchDown(0)==0){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<822>";
			bb_challengergui_CHGUI_DragMoving=0;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<825>";
	if(((bb_input_TouchDown(0))!=0) && bb_challengergui_CHGUI_TextBoxDown==0 && bb_challengergui_CHGUI_DragMoving==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<826>";
		if(bb_challengergui_CHGUI_Keyboard==2){
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<827>";
			if(bb_input_TouchY(0)<(bb_graphics_DeviceHeight())-dbg_object(bb_challengergui_CHGUI_KeyboardWindow).f_H){
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<828>";
				bb_challengergui_CHGUI_TargetX=bb_challengergui_CHGUI_OldX;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<829>";
				bb_challengergui_CHGUI_TargetY=bb_challengergui_CHGUI_OldY;
				err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<830>";
				bb_challengergui_CHGUI_Moving=1;
			}
		}else{
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<833>";
			bb_challengergui_CHGUI_TargetX=bb_challengergui_CHGUI_OldX;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<834>";
			bb_challengergui_CHGUI_TargetY=bb_challengergui_CHGUI_OldY;
			err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<835>";
			bb_challengergui_CHGUI_Moving=1;
		}
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<839>";
	if(bb_challengergui_CHGUI_TextBoxOver==0 && bb_challengergui_CHGUI_TextboxOnFocus==0 && bb_challengergui_CHGUI_Moving==0 && bb_challengergui_CHGUI_DragMoving==0){
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<840>";
		bb_challengergui_CHGUI_OldX=bb_challengergui_CHGUI_OffsetX;
		err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<841>";
		bb_challengergui_CHGUI_OldY=bb_challengergui_CHGUI_OffsetY;
	}
	err_info="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<844>";
	bb_challengergui_CHGUI_UpdateMsgBox();
	pop_err();
	return 0;
}
function bbInit(){
	bb_graphics_device=null;
	bb_input_device=null;
	bb_audio_device=null;
	bb_app_device=null;
	bb_graphics_context=bb_graphics_GraphicsContext_new.call(new bb_graphics_GraphicsContext);
	bb_graphics_Image_DefaultFlags=0;
	bb_graphics_renderDevice=null;
	bb_challengergui_CHGUI_MobileMode=0;
	bb_data2_STATUS="start";
	bb_challengergui_CHGUI_BottomList=[];
	bb_challengergui_CHGUI_Canvas=null;
	bb_challengergui_CHGUI_OffsetX=.0;
	bb_challengergui_CHGUI_OffsetY=.0;
	bb_challengergui_CHGUI_TitleHeight=25.0;
	bb_challengergui_CHGUI_LockedWIndow=null;
	bb_challengergui_CHGUI_Shadow=1;
	bb_challengergui_CHGUI_ShadowImg=null;
	bb_challengergui_CHGUI_Style=null;
	bb_challengergui_CHGUI_TitleFont=null;
	bb_challengergui_CHGUI_Font=null;
	bb_challengergui_CHGUI_KeyboardButtons=new_object_array(109);
	bb_challengergui_CHGUI_ShiftHold=0;
	bb_challengergui_CHGUI_Cursor=0;
	bb_challengergui_CHGUI_VariList=[];
	bb_challengergui_CHGUI_TopList=[];
	bb_challengergui_CHGUI_TooltipFlag=null;
	bb_challengergui_CHGUI_TooltipFont=null;
	bb_challengergui_CHGUI_Millisecs=0;
	bb_challengergui_CHGUI_FPSCounter=0;
	bb_challengergui_CHGUI_FPS=0;
	bb_challengergui_CHGUI_Started=0;
	bb_challengergui_CHGUI_Width=0;
	bb_challengergui_CHGUI_Height=0;
	bb_challengergui_CHGUI_CanvasFlag=0;
	bb_challengergui_CHGUI_TopTop=null;
	bb_challengergui_CHGUI_KeyboardWindow=null;
	bb_challengergui_CHGUI_MsgBoxWindow=null;
	bb_challengergui_CHGUI_MsgBoxLabel=null;
	bb_challengergui_CHGUI_MsgBoxButton=null;
	bb_data2_SCALE_W=300.0;
	bb_data2_SCALE_H=480.0;
	bb_challengergui_CHGUI_MouseBusy=0;
	bb_challengergui_CHGUI_Over=0;
	bb_challengergui_CHGUI_OverFlag=0;
	bb_challengergui_CHGUI_DownFlag=0;
	bb_challengergui_CHGUI_MenuOver=0;
	bb_challengergui_CHGUI_TextBoxOver=0;
	bb_challengergui_CHGUI_TextboxOnFocus=0;
	bb_challengergui_CHGUI_TextBoxDown=0;
	bb_challengergui_CHGUI_DragOver=0;
	bb_challengergui_CHGUI_Moving=0;
	bb_challengergui_CHGUI_TargetY=.0;
	bb_challengergui_CHGUI_TargetX=.0;
	bb_challengergui_CHGUI_IgnoreMouse=0;
	bb_challengergui_CHGUI_Tooltips=1;
	bb_challengergui_CHGUI_TooltipTime=1500;
	bb_challengergui_CHGUI_MenuClose=0;
	bb_challengergui_CHGUI_TextboxFocus=null;
	bb_challengergui_CHGUI_Keyboard=1;
	bb_challengergui_CHGUI_ShowKeyboard=0;
	bb_challengergui_CHGUI_AutoTextScroll=0;
	bb_challengergui_CHGUI_KeyboardPage=0;
	bb_challengergui_CHGUI_KeyboardShift=0;
	bb_challengergui_CHGUI_OldX=.0;
	bb_challengergui_CHGUI_OldY=.0;
	bb_challengergui_CHGUI_CursorMillisecs=0;
	bb_challengergui_CHGUI_DragScroll=0;
	bb_challengergui_CHGUI_DragMoving=0;
	bb_challengergui_CHGUI_OffsetMX=.0;
	bb_challengergui_CHGUI_OffsetMY=.0;
}
//${TRANSCODE_END}
