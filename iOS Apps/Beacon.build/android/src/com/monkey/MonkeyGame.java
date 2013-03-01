
//${PACKAGE_BEGIN}
package com.monkey;
//${PACKAGE_END}


//${IMPORTS_BEGIN}
import java.net.*;
import java.lang.Math;
import java.lang.reflect.Array;
import java.util.Vector;
import java.text.NumberFormat;
import java.text.ParseException;
import java.io.*;
import java.nio.*;
import java.util.*;
import java.lang.reflect.*;
import android.os.*;
import android.app.*;
import android.media.*;
import android.view.*;
import android.graphics.*;
import android.content.*;
import android.util.*;
import android.hardware.*;
import android.view.inputmethod.*;
import android.opengl.*;
import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.egl.EGLConfig;
import android.content.res.AssetManager;
import android.content.res.AssetFileDescriptor;
//${IMPORTS_END}

class MonkeyConfig{
//${CONFIG_BEGIN}
static final String ANDROID_APP_LABEL="Monkey Game";
static final String ANDROID_APP_PACKAGE="com.monkey";
static final String ANDROID_NATIVE_GL_ENABLED="0";
static final String ANDROID_SCREEN_ORIENTATION="portrait";
static final String ANDROID_SDK_DIR="E:\\\\APPLICATIONS\\\\Android\\\\android-sdk";
static final String BINARY_FILES="*.bin|*.dat";
static final String CD="";
static final String CONFIG="debug";
static final String HOST="winnt";
static final String IMAGE_FILES="*.png|*.jpg|*.gif|*.bmp";
static final String LANG="java";
static final String MODPATH=".;J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps;C:/Program Files (x86)/Monkey/modules";
static final String MOJO_HICOLOR_TEXTURES="1";
static final String MOJO_IMAGE_FILTERING_ENABLED="1";
static final String MUSIC_FILES="*.wav|*.ogg|*.mp3|*.m4a";
static final String OPENGL_GLES20_ENABLED="0";
static final String SAFEMODE="0";
static final String SOUND_FILES="*.wav|*.ogg|*.mp3|*.m4a";
static final String TARGET="android";
static final String TEXT_FILES="*.txt|*.xml|*.json";
static final String TRANSDIR="";
//${CONFIG_END}
}

class MonkeyData{

	static AssetManager getAssets(){
		return MonkeyGame.activity.getAssets();
	}

	static String toString( byte[] buf ){
		int n=buf.length;
		char tmp[]=new char[n];
		for( int i=0;i<n;++i ){
			tmp[i]=(char)(buf[i] & 0xff);
		}
		return new String( tmp );
	}
	
	static String loadString( byte[] buf ){
	
		int n=buf.length;
		StringBuilder out=new StringBuilder();
		
		int t0=n>0 ? buf[0] & 0xff : -1;
		int t1=n>1 ? buf[1] & 0xff : -1;
		
		if( t0==0xfe && t1==0xff ){
			int i=2;
			while( i<n-1 ){
				int x=buf[i++] & 0xff;
				int y=buf[i++] & 0xff;
				out.append( (char)((x<<8)|y) ); 
			}
		}else if( t0==0xff && t1==0xfe ){
			int i=2;
			while( i<n-1 ){
				int x=buf[i++] & 0xff;
				int y=buf[i++] & 0xff;
				out.append( (char)((y<<8)|x) ); 
			}
		}else{
			int t2=n>2 ? buf[2] & 0xff : -1;
			int i=(t0==0xef && t1==0xbb && t2==0xbf) ? 3 : 0;
			boolean fail=false;
			while( i<n ){
				int c=buf[i++] & 0xff;
				if( (c & 0x80)!=0 ){
					if( (c & 0xe0)==0xc0 ){
						if( i>=n || (buf[i] & 0xc0)!=0x80 ){
							fail=true;
							break;
						}
						c=((c & 0x1f)<<6) | (buf[i] & 0x3f);
						i+=1;
					}else if( (c & 0xf0)==0xe0 ){
						if( i+1>=n || (buf[i] & 0xc0)!=0x80 || (buf[i+1] & 0xc0)!=0x80 ){
							fail=true;
							break;
						}
						c=((c & 0x0f)<<12) | ((buf[i] & 0x3f)<<6) | (buf[i+1] & 0x3f);
						i+=2;
					}else{
						fail=true;
						break;
					}
				}
				out.append( (char)c );
			}
			if( fail ){
				return toString( buf );
			}
		}
		return out.toString();
	}
	
	static String filePath( String path ){
		if( !path.startsWith("monkey://") ){
			return path;
		}else if( path.startsWith("monkey://internal/") ){
			File f=MonkeyGame.activity.getFilesDir();
			if( f!=null ) return f+"/"+path.substring(18);
		}else if( path.startsWith("monkey://external/") ){
			File f=MonkeyGame.activity.getExternalFilesDir(null);
			if( f!=null ) return f+"/"+path.substring(18);
		}
		return "";
	}
	
	static String assetPath( String path ){
		if( path.toLowerCase().startsWith("monkey://data/") ) return "monkey/"+path.substring(14);
		return "";
	}
	
	static byte[] loadBytes( String path ){
	
		path=assetPath( path );
		if( path=="" ) return null;
		
		try{
			//Man, they sure don't make this easy for ya do they?!?
			InputStream in=getAssets().open( path );
			
			ByteArrayOutputStream out=new ByteArrayOutputStream(1024);
			byte[] buf=new byte[4096];
			
			for(;;){
				int n=in.read( buf );
				if( n<0 ) break;
				out.write( buf,0,n );
			}
			in.close();
			return out.toByteArray();
			
		}catch( IOException e ){
		}
		return null;
	}

	static String loadString( String path ){
		byte[] bytes=loadBytes( path );
		if( bytes!=null ) return loadString( bytes );
		return "";
	}

	static Bitmap loadBitmap( String path ){

		Bitmap bitmap=null;
		
		BitmapFactory.Options opts=new BitmapFactory.Options();
		opts.inPurgeable=true;
		
		try{
			if( path.toLowerCase().startsWith("monkey://data/") ){
				path=assetPath( path );
				InputStream in=getAssets().open( path );
				bitmap=BitmapFactory.decodeStream( in,null,opts );
				in.close();
			}else{
				URL url=new URL( path );
				URLConnection con=url.openConnection();
				BufferedInputStream in=new BufferedInputStream( con.getInputStream() );
				bitmap=BitmapFactory.decodeStream( in,null,opts );
				in.close();
			}
		}catch( IOException e ){
		}
		return bitmap;
	}

	static int loadSound( String path,SoundPool pool ){
		path=assetPath( path );
		if( path=="" ) return 0;

		try{
			return pool.load( getAssets().openFd( path ),1 );
		}catch( IOException e ){
		}
		return 0;
	}
	
	static MediaPlayer openMedia( String path ){
		path=assetPath( path );
		if( path=="" ) return null;

		try{
			android.content.res.AssetFileDescriptor fd=getAssets().openFd( path );

			MediaPlayer mp=new MediaPlayer();
			mp.setDataSource( fd.getFileDescriptor(),fd.getStartOffset(),fd.getLength() );
			mp.prepare();
			
			fd.close();
			
			return mp;
		}catch( IOException e ){
		}
		return null;
	}
}

//${TRANSCODE_BEGIN}

// Java Monkey runtime.
//
// Placed into the public domain 24/02/2011.
// No warranty implied; use at your own risk.



class bb_std_lang{

	//***** Error handling *****

	static String errInfo="";
	static Vector errStack=new Vector();
	
	static float D2R=0.017453292519943295f;
	static float R2D=57.29577951308232f;
	
	static NumberFormat numberFormat=NumberFormat.getInstance();
	
	static void pushErr(){
		errStack.addElement( errInfo );
	}
	
	static void popErr(){
		if( errStack.size()==0 ) throw new Error( "STACK ERROR!" );
		errInfo=(String)errStack.remove( errStack.size()-1 );
	}
	
	static String stackTrace(){
		if( errInfo.length()==0 ) return "";
		String str=errInfo+"\n";
		for( int i=errStack.size()-1;i>0;--i ){
			str+=(String)errStack.elementAt(i)+"\n";
		}
		return str;
	}
	
	static int print( String str ){
		System.out.println( str );
		return 0;
	}
	
	static int error( String str ){
		throw new Error( str );
	}
	
	static String makeError( String err ){
		if( err.length()==0 ) return "";
		return "Monkey Runtime Error : "+err+"\n\n"+stackTrace();
	}
	
	static int debugLog( String str ){
		print( str );
		return 0;
	}
	
	static int debugStop(){
		error( "STOP" );
		return 0;
	}
	
	//***** String stuff *****

	static public String[] stringArray( int n ){
		String[] t=new String[n];
		for( int i=0;i<n;++i ) t[i]="";
		return t;
	}
	
	static String slice( String str,int from ){
		return slice( str,from,str.length() );
	}
	
	static String slice( String str,int from,int term ){
		int len=str.length();
		if( from<0 ){
			from+=len;
			if( from<0 ) from=0;
		}else if( from>len ){
			from=len;
		}
		if( term<0 ){
			term+=len;
		}else if( term>len ){
			term=len;
		}
		if( term>from ) return str.substring( from,term );
		return "";
	}
	
	static public String[] split( String str,String sep ){
		if( sep.length()==0 ){
			String[] bits=new String[str.length()];
			for( int i=0;i<str.length();++i){
				bits[i]=String.valueOf( str.charAt(i) );
			}
			return bits;
		}else{
			int i=0,i2,n=1;
			while( (i2=str.indexOf( sep,i ))!=-1 ){
				++n;
				i=i2+sep.length();
			}
			String[] bits=new String[n];
			i=0;
			for( int j=0;j<n;++j ){
				i2=str.indexOf( sep,i );
				if( i2==-1 ) i2=str.length();
				bits[j]=slice( str,i,i2 );
				i=i2+sep.length();
			}
			return bits;
		}
	}
	
	static public String join( String sep,String[] bits ){
		if( bits.length<2 ) return bits.length==1 ? bits[0] : "";
		StringBuilder buf=new StringBuilder( bits[0] );
		boolean hasSep=sep.length()>0;
		for( int i=1;i<bits.length;++i ){
			if( hasSep ) buf.append( sep );
			buf.append( bits[i] );
		}
		return buf.toString();
	}
	
	static public String replace( String str,String find,String rep ){
		int i=0;
		for(;;){
			i=str.indexOf( find,i );
			if( i==-1 ) return str;
			str=str.substring( 0,i )+rep+str.substring( i+find.length() );
			i+=rep.length();
		}
	}
	
	static public String fromChars( int[] chars ){
		int n=chars.length;
		char[] chrs=new char[n];
		for( int i=0;i<n;++i ){
			chrs[i]=(char)chars[i];
		}
		return new String( chrs,0,n );
	}
	
	static int[] toChars( String str ){
		int[] arr=new int[str.length()];
		for( int i=0;i<str.length();++i ) arr[i]=(int)str.charAt( i );
		return arr;
	}
	
	//***** Array Stuff *****
	
	static Object sliceArray( Object arr,int from ){
		return sliceArray( arr,from,Array.getLength( arr ) );
	}
	
	static Object sliceArray( Object arr,int from,int term ){
		int len=Array.getLength( arr );
		if( from<0 ){
			from+=len;
			if( from<0 ) from=0;
		}else if( from>len ){
			from=len;
		}
		if( term<0 ){
			term+=len;
		}else if( term>len ){
			term=len;
		}
		if( term<from ) term=from;
		int newlen=term-from;
		Object res=Array.newInstance( arr.getClass().getComponentType(),newlen );
		if( newlen>0 ) System.arraycopy( arr,from,res,0,newlen );
		return res;
	}
	
	static Object resizeArray( Object arr,int newlen ){
		int len=Array.getLength( arr );
		Object res=Array.newInstance( arr.getClass().getComponentType(),newlen );
		int n=Math.min( len,newlen );
		if( n>0 ) System.arraycopy( arr,0,res,0,n );
		return res;
	}
	
	static Object[] resizeArrayArray( Object[] arr,int newlen ){
		int i=arr.length;
		arr=(Object[])resizeArray( arr,newlen );
		if( i<newlen ){
			Object empty=Array.newInstance( arr.getClass().getComponentType().getComponentType(),0 );
			while( i<newlen ) arr[i++]=empty;
		}
		return arr;
	}
	
	static String[] resizeStringArray( String[] arr,int newlen ){
		int i=arr.length;
		arr=(String[])resizeArray( arr,newlen );
		while( i<newlen ) arr[i++]="";
		return arr;
	}
	
	static Object concatArrays( Object lhs,Object rhs ){
		int lhslen=Array.getLength( lhs );
		int rhslen=Array.getLength( rhs );
		int len=lhslen+rhslen;
		Object res=Array.newInstance( lhs.getClass().getComponentType(),len );
		if( lhslen>0 ) System.arraycopy( lhs,0,res,0,lhslen );
		if( rhslen>0 ) System.arraycopy( rhs,0,res,lhslen,rhslen );
		return res;
	}
	
	static int arrayLength( Object arr ){
		return arr!=null ? Array.getLength( arr ) : 0;
	}
}

class ThrowableObject extends RuntimeException{
	ThrowableObject(){
		super( "Uncaught Throwable Object" );
	}
}

class BBDataBuffer{

	boolean _New( int length ){
		if( _data!=null ) return false;
		_data=ByteBuffer.allocate( length );
		_data.order( ByteOrder.nativeOrder() );
		_length=length;
		return true;
	}
	
	boolean _Load( String path ){
		if( _data!=null ) return false;
	
		byte[] bytes=MonkeyData.loadBytes( path );
		if( bytes==null ) return false;
		
		int length=bytes.length;
		
		if( !_New( length ) ) return false;
		
		System.arraycopy( bytes,0,_data.array(),0,length );
		
		return true;
	}
	
	int Length(){
		return _length;
	}
	
	void Discard(){
		if( _data==null ) return;
		_data=null;
		_length=0;
	}
		
	void PokeByte( int addr,int value ){
		_data.put( addr,(byte)value );
	}
	
	void PokeShort( int addr,int value ){
		_data.putShort( addr,(short)value );
	}
	
	void PokeInt( int addr,int value ){
		_data.putInt( addr,value );
	}
	
	void PokeFloat( int addr,float value ){
		_data.putFloat( addr,value );
	}
	
	int PeekByte( int addr ){
		return _data.get( addr );
	}
	
	int PeekShort( int addr ){
		return _data.getShort( addr );
	}
	
	int PeekInt( int addr ){
		return _data.getInt( addr );
	}
	
	float PeekFloat( int addr ){
		return _data.getFloat( addr );
	}

	ByteBuffer GetByteBuffer(){
		return _data;
	}
	
	ByteBuffer _data;
	int _length;
}

// Android mojo runtime.
//
// Copyright 2011 Mark Sibly, all rights reserved.
// No warranty implied; use at your own risk.






public class MonkeyGame extends Activity {

	static MonkeyGame activity;
	static MonkeyView view;
	static gxtkApp app;
	
	public static class LogTool extends OutputStream{
	
		private ByteArrayOutputStream bos=new ByteArrayOutputStream();
	  
		@Override
		public void write( int b ) throws IOException{
			if( b==(int)'\n' ){
				Log.i( "[Monkey]",new String( this.bos.toByteArray() ) );
				this.bos=new ByteArrayOutputStream();
			}else{
				this.bos.write(b);
			}
		}
	}
	
	public static class MonkeyView extends GLSurfaceView implements GLSurfaceView.Renderer{

		public MonkeyView( Context context ){
			super( context );
		}
		
		public MonkeyView( Context context,AttributeSet attrs ){
			super( context,attrs );
		}
		
		public boolean dispatchKeyEventPreIme( KeyEvent event ){
			if( app==null ) return false;
			
			if( app.input.keyboardEnabled ) {
				if( event.getKeyCode()==KeyEvent.KEYCODE_BACK ){
					if( event.getAction()==KeyEvent.ACTION_DOWN ){
						app.input.PutChar( 27 );
					}
					return true;
				}
			}else{
				if( event.getKeyCode()==KeyEvent.KEYCODE_BACK ){
					if( event.getAction()==KeyEvent.ACTION_DOWN ){
						app.input.OnKeyDown( 27 );
					}else if( event.getAction()==KeyEvent.ACTION_UP ){
						app.input.OnKeyUp( 27 );
					}
					return true;
				}
			}
			return false;
		}
		
		public boolean onKeyDown( int key,KeyEvent event ){
			if( app==null || !app.input.keyboardEnabled ) return false;
			
			if( event.getKeyCode()==KeyEvent.KEYCODE_DEL ){
				app.input.PutChar( 8 );
			}else{
				int chr=event.getUnicodeChar();
				if( chr!=0 ){
					if( chr==10 ) chr=13;
					app.input.PutChar( chr );
				}
			}
			return true;
		}
		
		public boolean onKeyMultiple( int keyCode,int repeatCount,KeyEvent event ){
			if( app==null || !app.input.keyboardEnabled ) return false;
		
			gxtkInput input=app.input;
			
			String str=event.getCharacters();
			for( int i=0;i<str.length();++i ){
				int chr=str.charAt( i );
				if( chr!=0 ){
					if( chr==10 ) chr=13;
					input.PutChar( chr );
				}
			}
			return true;
		}
		
		//fields for touch event handling
		boolean useMulti,checkedMulti;
		Method getPointerCount,getPointerId,getX,getY;
		Object args1[]=new Object[1];
		
		public boolean onTouchEvent( MotionEvent event ){
			if( app==null ) return false;
		
			if( !checkedMulti ){
				//Check for multi-touch support
				//
				try{
					Class cls=event.getClass();
					Class intClass[]=new Class[]{ Integer.TYPE };
					getPointerCount=cls.getMethod( "getPointerCount" );
					getPointerId=cls.getMethod( "getPointerId",intClass );
					getX=cls.getMethod( "getX",intClass );
					getY=cls.getMethod( "getY",intClass );
					useMulti=true;
				}catch( NoSuchMethodException ex ){
					useMulti=false;
				}
				checkedMulti=true;
			}
			
			if( !useMulti ){
				//mono-touch version...
				//
				gxtkInput input=app.input;
				int action=event.getAction();
				
				switch( action ){
				case MotionEvent.ACTION_DOWN:
					input.OnKeyDown( gxtkInput.KEY_TOUCH0 );
					break;
				case MotionEvent.ACTION_UP:
					input.OnKeyUp( gxtkInput.KEY_TOUCH0 );
					break;
				}
				
				input.touchX[0]=event.getX();
				input.touchY[0]=event.getY();
		
				return true;
			}

			try{

				//multi-touch version...
				//
				final int ACTION_DOWN=0;
				final int ACTION_UP=1;
				final int ACTION_POINTER_DOWN=5;
				final int ACTION_POINTER_UP=6;
				final int ACTION_POINTER_ID_SHIFT=8;
				final int ACTION_MASK=255;
				
				gxtkInput input=app.input;
				
				int action=event.getAction();
				int maskedAction=action & ACTION_MASK;
				int pid=0;
				
				if( maskedAction==ACTION_POINTER_DOWN || maskedAction==ACTION_POINTER_UP ){
					args1[0]=Integer.valueOf( action>>ACTION_POINTER_ID_SHIFT );
					pid=((Integer)getPointerId.invoke( event,args1 )).intValue();
				}else{
					args1[0]=Integer.valueOf( 0 );
					pid=((Integer)getPointerId.invoke( event,args1 )).intValue();
				}
				
				switch( maskedAction ){
				case ACTION_DOWN:
				case ACTION_POINTER_DOWN:
					input.OnKeyDown( pid+gxtkInput.KEY_TOUCH0 );
					break;
				case ACTION_UP:
				case ACTION_POINTER_UP:
					input.OnKeyUp( pid+gxtkInput.KEY_TOUCH0 );
					break;
				}
				
				int pointerCount=((Integer)getPointerCount.invoke( event )).intValue();
				
				for( int i=0;i<pointerCount;++i ){
					args1[0]=Integer.valueOf( i );
					int pid2=((Integer)getPointerId.invoke( event,args1 )).intValue();
					input.touchX[pid2]=((Float)getX.invoke( event,args1 )).floatValue();
					input.touchY[pid2]=((Float)getY.invoke( event,args1 )).floatValue();
				}

			}catch( Exception e ){
			}
	
			return true;
		}
		
		public void onDrawFrame( GL10 gl ){
		}
	
		public void onSurfaceChanged( GL10 gl,int width,int height ){
		}
	
		public void onSurfaceCreated( GL10 gl,EGLConfig config ){
		}
	}
	
	/** Called when the activity is first created. */
	@Override
	public void onCreate( Bundle savedInstanceState ){	//onStart
		super.onCreate( savedInstanceState );
		
		System.setOut( new PrintStream( new LogTool() ) );

		activity=this;

		setContentView( R.layout.main );

		view=(MonkeyView)findViewById( R.id.monkeyview );
		view.setFocusableInTouchMode( true );
		view.requestFocus();
		
		setVolumeControlStream( AudioManager.STREAM_MUSIC );
			
		try{
		
			bb_.bbInit();
			bb_.bbMain();
			
			if( app==null ) System.exit( 0 );
			
			if( MonkeyConfig.OPENGL_GLES20_ENABLED.equals( "1" ) ){
				
				//view.setEGLContextClientVersion( 2 );	//API 8 only!
				//
				try{
					Class clas=view.getClass();
					Class parms[]=new Class[]{ Integer.TYPE };
					Method setVersion=clas.getMethod( "setEGLContextClientVersion",parms );
					Object args[]=new Object[1];
					args[0]=Integer.valueOf( 2 );
					setVersion.invoke( view,args );
				}catch( NoSuchMethodException ex ){
				}
			}
			view.setRenderer( app );
			view.setRenderMode( GLSurfaceView.RENDERMODE_WHEN_DIRTY );
			view.requestRender();

		}catch( Throwable t ){
		
			app=null;
		
			view.setRenderer( view );
			view.setRenderMode( GLSurfaceView.RENDERMODE_WHEN_DIRTY );
			view.requestRender();
			
			new gxtkAlert( t );
		}
	}
	
	@Override
	public void onRestart(){
		super.onRestart();
	}
	
	@Override
	public void onStart(){
		super.onStart();
	}
	
	@Override
	public void onResume(){
		super.onResume();
		view.onResume();
		if( app!=null ){
			app.InvokeOnResume();
		}
	}
	
	@Override 
	public void onPause(){
		super.onPause();
		if( app!=null ){
			app.InvokeOnSuspend();
		}
		view.onPause();
	}

	@Override
	public void onStop(){
		super.onStop();
	}
	
	@Override
	public void onDestroy(){
		super.onDestroy();
	}
}

class gxtkTimer implements Runnable{

	private double nextUpdate;
	private double updatePeriod;
	private boolean cancelled=false;

	public gxtkTimer( int fps ){
		updatePeriod=1000.0/fps;
		nextUpdate=SystemClock.uptimeMillis()+updatePeriod;
		MonkeyGame.view.postDelayed( this,(long)updatePeriod );
	}

	public void cancel(){
		cancelled=true;
	}

	public void run(){
		if( cancelled ) return;

		int updates=0;
		for(;;){
			nextUpdate+=updatePeriod;

			MonkeyGame.app.InvokeOnUpdate();

			if( cancelled ) return;
			
			if( (long)nextUpdate>SystemClock.uptimeMillis() ) break;
			
			if( ++updates==7 ){
				nextUpdate=SystemClock.uptimeMillis()+updatePeriod;
				break;
			}
		}

		MonkeyGame.view.requestRender();

		if( cancelled ) return;
		
		long delay=(long)nextUpdate-SystemClock.uptimeMillis();
		MonkeyGame.view.postDelayed( this,delay>0 ? delay : 0 );
	}
}

class gxtkAlert implements Runnable{

	String msg;

	gxtkAlert( Throwable t ){
	
		bb_std_lang.print( "Java exception:"+t.toString() );
		
		if( t instanceof NullPointerException ){
			msg="Attempt to access null object";
		}else if( t instanceof ArithmeticException ){
			msg="Arithmetic exception";
		}else if( t instanceof ArrayIndexOutOfBoundsException ){
			msg="Array index out of bounds";
		}else{
			msg=t.getMessage();
		}
		
		if( msg.length()==0 ) System.exit( 0 );
		
		msg=bb_std_lang.makeError( msg );

		MonkeyGame.view.postDelayed( this,0 );
	}

	public void run(){

		AlertDialog.Builder db=new AlertDialog.Builder( MonkeyGame.activity );

		db.setMessage( msg );
		
		AlertDialog dlg=db.show();
	}
	
};

class gxtkApp implements GLSurfaceView.Renderer{

	gxtkGraphics graphics;
	gxtkInput input;
	gxtkAudio audio;

	int updateRate;
	gxtkTimer timer;
	
	long startMillis;
	
	boolean dead,suspended,canrender,created;
	
	int seq;

	gxtkApp(){
		MonkeyGame.app=this;

		graphics=new gxtkGraphics();
		input=new gxtkInput();
		audio=new gxtkAudio();
		
		startMillis=System.currentTimeMillis();
	}
	
	void Die( Throwable t ){
	
		dead=true;
		audio.OnDestroy();
		
		new gxtkAlert( t );
	}
	
	//interface GLSurfaceView.Renderer
	synchronized public void onDrawFrame( GL10 gl ){
		InvokeOnRender( gl );
	}
	
	//interface GLSurfaceView.Renderer
	synchronized public void onSurfaceChanged( GL10 gl,int width,int height ){
		graphics.SetSize( width,height );
	}
	
	//interface GLSurfaceView.Renderer
	synchronized public void onSurfaceCreated( GL10 gl,EGLConfig config ){
		gxtkSurface.discarded.clear();
		canrender=true;
		seq+=1;
	}
	
	synchronized int InvokeOnUpdate(){
		if( dead || suspended || updateRate==0 ) return 0;

		try{
			input.BeginUpdate();
			OnUpdate();
			input.EndUpdate();
		}catch( Throwable t ){
			Die( t );
		}
		return 0;
	}

	synchronized int InvokeOnSuspend(){
		if( dead || suspended ) return 0;

		try{
			suspended=true;
			canrender=false;
			OnSuspend();
			audio.OnSuspend();
			if( updateRate!=0 ){
				int upr=updateRate;
				SetUpdateRate( 0 );
				updateRate=upr;
			}
		}catch( Throwable t ){
			Die( t );
		}
		return 0;
	}

	synchronized int InvokeOnResume(){
		if( dead || !suspended ) return 0;
		
		try{
			suspended=false;
			audio.OnResume();
			if( updateRate!=0 ){
				int upr=updateRate;
				updateRate=0;
				SetUpdateRate( upr );
			}
			OnResume();
		}catch( Throwable t ){
			Die( t );
		}
		return 0;
	}

	synchronized int InvokeOnRender( GL10 gl ){
		if( dead || suspended || !canrender ) return 0;
		
		try{
			graphics.BeginRender( gl );
			if( !created ){
				created=true;
				OnCreate();
			}
			OnRender();
			graphics.EndRender();
		}catch( Throwable t ){
			Die( t );
		}
		return 0;
	}
	
	//***** GXTK API *****

	gxtkGraphics GraphicsDevice(){
		return graphics;

	}
	gxtkInput InputDevice(){
		return input;
	}
	
	gxtkAudio AudioDevice(){
		return audio;
	}

	String LoadState(){
		SharedPreferences prefs=MonkeyGame.activity.getPreferences( 0 );
		return prefs.getString( "gxtkAppState","" );
	}
	
	int SaveState( String state ){
		SharedPreferences prefs=MonkeyGame.activity.getPreferences( 0 );
		SharedPreferences.Editor editor=prefs.edit();
		editor.putString( "gxtkAppState",state );
		editor.commit();
		return 0;
	}
	
	String LoadString( String path ){
		return MonkeyData.loadString( path );
	}

	int SetUpdateRate( int hertz ){
		if( timer!=null ){
			timer.cancel();
			timer=null;
		}
		updateRate=hertz;
		if( updateRate!=0 ){
			timer=new gxtkTimer( updateRate );
		}
		return 0;
	}

	int MilliSecs(){
		return (int)( System.currentTimeMillis()-startMillis );
	}
	
	int Loading(){
		return 0;
	}

	int OnCreate(){
		return 0;
	}

	int OnUpdate(){
		return 0;
	}
	
	int OnSuspend(){
		return 0;
	}
	
	int OnResume(){
		return 0;
	}

	int OnRender(){
		return 0;
	}
	
	int OnLoading(){
		return 0;
	}
	
}

class RenderOp{
	int type,count,alpha;
	gxtkSurface surf;
};

class gxtkGraphics{

	static final int MAX_VERTICES=65536/20;
	static final int MAX_RENDEROPS=MAX_VERTICES/2;	//ie: max lines
	
	int mode,width,height;
	
	float alpha;
	float r,g,b;
	int colorARGB;
	int blend;
	float ix,iy,jx,jy,tx,ty;
	boolean tformed;
	
	RenderOp renderOps[]=new RenderOp[MAX_RENDEROPS];
	RenderOp rop,nullRop;
	int nextOp,vcount;

	float[] vertices=new float[MAX_VERTICES*4];	//x,y,u,v
	int[] colors=new int[MAX_VERTICES];	//rgba
	int vp,cp;
	
	FloatBuffer vbuffer;
	IntBuffer cbuffer;
	int vbo,vbo_seq;
	
	gxtkGraphics(){
	
		if( MonkeyConfig.OPENGL_GLES20_ENABLED.equals( "1" ) ){
			mode=0;
			return;
		}
		
		mode=1;
	
		for( int i=0;i<MAX_RENDEROPS;++i ){
			renderOps[i]=new RenderOp();
		}
		nullRop=new RenderOp();
		nullRop.type=-1;

		vbuffer=FloatBuffer.wrap( vertices,0,MAX_VERTICES*4 );
		cbuffer=IntBuffer.wrap( colors,0,MAX_VERTICES );
	}
	
	void SetSize( int width,int height ){
		this.width=width;
		this.height=height;
	}
	
	void BeginRender( GL10 _gl ){
		if( mode==0 ) return;
		
		if( vbo_seq!=MonkeyGame.app.seq ){

			vbo_seq=MonkeyGame.app.seq;
			
			int[] bufs=new int[1];
			GLES11.glGenBuffers( 1,bufs,0 );
			vbo=bufs[0];
			
			GLES11.glBindBuffer( GLES11.GL_ARRAY_BUFFER,vbo );
			GLES11.glBufferData( GLES11.GL_ARRAY_BUFFER,MAX_VERTICES*20,null,GLES11.GL_DYNAMIC_DRAW );
		}
		
		GLES11.glViewport( 0,0,width,height );
		
		GLES11.glMatrixMode( GLES11.GL_PROJECTION );
		GLES11.glLoadIdentity();
		GLES11.glOrthof( 0,width,height,0,-1,1 );
		
		GLES11.glMatrixMode( GLES11.GL_MODELVIEW );
		GLES11.glLoadIdentity();
		
		GLES11.glEnable( GLES11.GL_BLEND );
		GLES11.glBlendFunc( GLES11.GL_ONE,GLES11.GL_ONE_MINUS_SRC_ALPHA );

		GLES11.glBindBuffer( GLES11.GL_ARRAY_BUFFER,vbo );
		GLES11.glEnableClientState( GLES11.GL_VERTEX_ARRAY );
		GLES11.glEnableClientState( GLES11.GL_TEXTURE_COORD_ARRAY );
		GLES11.glEnableClientState( GLES11.GL_COLOR_ARRAY );
		GLES11.glVertexPointer( 2,GLES11.GL_FLOAT,16,0 );
		GLES11.glTexCoordPointer( 2,GLES11.GL_FLOAT,16,8 );
		GLES11.glColorPointer( 4,GLES11.GL_UNSIGNED_BYTE,0,MAX_VERTICES*16 );

		Reset();
	}
	
	void EndRender(){
		if( mode==0 ) return;
		Flush();
	}

	void Reset(){
		rop=nullRop;
		nextOp=0;
		vcount=0;
	}

	void Flush(){
		if( vcount==0 ) return;
	
		GLES11.glBufferData( GLES11.GL_ARRAY_BUFFER,vcount*20,null,GLES11.GL_DYNAMIC_DRAW );
		GLES11.glBufferSubData( GLES11.GL_ARRAY_BUFFER,0,vcount*16,vbuffer );
		GLES11.glBufferSubData( GLES11.GL_ARRAY_BUFFER,vcount*16,vcount*4,cbuffer );
		GLES11.glColorPointer( 4,GLES11.GL_UNSIGNED_BYTE,0,vcount*16 );

		GLES11.glDisable( GLES11.GL_TEXTURE_2D );
		GLES11.glDisable( GLES11.GL_BLEND );

		int index=0;
		boolean blendon=false;
		gxtkSurface surf=null;

		for( int i=0;i<nextOp;++i ){

			RenderOp op=renderOps[i];
			
			if( op.surf!=null ){
				if( op.surf!=surf ){
					if( surf==null ) GLES11.glEnable( GLES11.GL_TEXTURE_2D );
					surf=op.surf;
					GLES11.glDisable( GLES11.GL_TEXTURE_2D );
					surf.Bind();
					GLES11.glEnable( GLES11.GL_TEXTURE_2D );
				}
			}else{
				if( surf!=null ){
					GLES11.glDisable( GLES11.GL_TEXTURE_2D );
					surf=null;
				}
			}
			
			//should just have another blend mode...
			if( blend==1 || (op.alpha>>>24)!=0xff || (op.surf!=null && op.surf.hasAlpha) ){
				if( !blendon ){
					GLES11.glEnable( GLES11.GL_BLEND );
					blendon=true;
				}
			}else{
				if( blendon ){
					GLES11.glDisable( GLES11.GL_BLEND );
					blendon=false;
				}
			}
	
			GLES11.glDrawArrays( op.type,index,op.count );
			
			index+=op.count;
		}
		
		Reset();
	}
	
	void Begin( int type,int count,gxtkSurface surf ){
		if( vcount+count>MAX_VERTICES ){
			Flush();
		}
		if( type!=rop.type || surf!=rop.surf ){
			if( nextOp==MAX_RENDEROPS ){
				Flush();
			}
			rop=renderOps[nextOp];
			nextOp+=1;
			rop.type=type;
			rop.surf=surf;
			rop.count=0;
			rop.alpha=~0;
		}
		rop.alpha&=colorARGB;
		rop.count+=count;
		vp=vcount*4;
		cp=vcount;
		vcount+=count;
	}

	//***** GXTK API *****

	int Mode(){
		return mode;
	}
	
	int Width(){
		return width;
	}
	
	int Height(){
		return height;
	}
	
	gxtkSurface LoadSurface__UNSAFE__( gxtkSurface surface,String path ){
		Bitmap bitmap=null;
		try{
			bitmap=MonkeyData.loadBitmap( path );
		}catch( OutOfMemoryError e ){
			throw new Error( "Out of memory error loading bitmap" );
		}
		if( bitmap==null ) return null;
		surface.SetBitmap( bitmap );
		return surface;
	}
	
	gxtkSurface LoadSurface( String path ){
		return LoadSurface__UNSAFE__( new gxtkSurface(),path );
	}
	
	gxtkSurface CreateSurface( int width,int height ){
		Bitmap bitmap=Bitmap.createBitmap( width,height,Bitmap.Config.ARGB_8888 );
		if( bitmap!=null ) return new gxtkSurface( bitmap );
		return null;
	}
	
	int SetAlpha( float alpha ){
		this.alpha=alpha;
		int a=(int)(alpha*255);
		colorARGB=(a<<24) | ((int)(b*alpha)<<16) | ((int)(g*alpha)<<8) | (int)(r*alpha);
		return 0;
	}

	int SetColor( float r,float g,float b ){
		this.r=r;
		this.g=g;
		this.b=b;
		int a=(int)(alpha*255);
		colorARGB=(a<<24) | ((int)(b*alpha)<<16) | ((int)(g*alpha)<<8) | (int)(r*alpha);
		return 0;
	}
	
	int SetBlend( int blend ){
		if( blend==this.blend ) return 0;
		
		Flush();
		
		this.blend=blend;
		
		switch( blend ){
		case 1:
			GLES11.glBlendFunc( GLES11.GL_ONE,GLES11.GL_ONE );
			break;
		default:
			GLES11.glBlendFunc( GLES11.GL_ONE,GLES11.GL_ONE_MINUS_SRC_ALPHA );
		}
		return 0;
	}
	
	int SetScissor( int x,int y,int w,int h ){
		Flush();
		
		if( x!=0 || y!=0 || w!=Width() || h!=Height() ){
			GLES11.glEnable( GLES11.GL_SCISSOR_TEST );
			y=Height()-y-h;
			GLES11.glScissor( x,y,w,h );
		}else{
			GLES11.glDisable( GLES11.GL_SCISSOR_TEST );
		}
		return 0;
	}
	
	int SetMatrix( float ix,float iy,float jx,float jy,float tx,float ty ){
	
		tformed=(ix!=1 || iy!=0 || jx!=0 || jy!=1 || tx!=0 || ty!=0);
		this.ix=ix;
		this.iy=iy;
		this.jx=jx;
		this.jy=jy;
		this.tx=tx;
		this.ty=ty;
		
		return 0;
	}
	
	int Cls( float r,float g,float b ){
		Reset();
		
		GLES11.glClearColor( r/255.0f,g/255.0f,b/255.0f,1 );
		GLES11.glClear( GLES11.GL_COLOR_BUFFER_BIT|GLES11.GL_DEPTH_BUFFER_BIT );
		
		return 0;
	}
	
	int DrawPoint( float x,float y ){
	
		if( tformed ){
			float px=x;
			x=px * ix + y * jx + tx;
			y=px * iy + y * jy + ty;
		}
		
		Begin( GLES11.GL_POINTS,1,null );
		
		vertices[vp]=x;vertices[vp+1]=y;
		
		colors[cp]=colorARGB;
		
		return 0;
	}
	
	int DrawRect( float x,float y,float w,float h ){
	
		float x0=x,x1=x+w,x2=x+w,x3=x;
		float y0=y,y1=y,y2=y+h,y3=y+h;
		
		if( tformed ){
			float tx0=x0,tx1=x1,tx2=x2,tx3=x3;
			x0=tx0 * ix + y0 * jx + tx;
			y0=tx0 * iy + y0 * jy + ty;
			x1=tx1 * ix + y1 * jx + tx;
			y1=tx1 * iy + y1 * jy + ty;
			x2=tx2 * ix + y2 * jx + tx;
			y2=tx2 * iy + y2 * jy + ty;
			x3=tx3 * ix + y3 * jx + tx;
			y3=tx3 * iy + y3 * jy + ty;
		}

		Begin( GLES11.GL_TRIANGLES,6,null );
		
		vertices[vp]=x0;vertices[vp+1]=y0;
		vertices[vp+4]=x1;vertices[vp+5]=y1;
		vertices[vp+8]=x2;vertices[vp+9]=y2;
		
		vertices[vp+12]=x0;vertices[vp+13]=y0;
		vertices[vp+16]=x2;vertices[vp+17]=y2;
		vertices[vp+20]=x3;vertices[vp+21]=y3;

		colors[cp]=colors[cp+1]=colors[cp+2]=colors[cp+3]=colors[cp+4]=colors[cp+5]=colorARGB;

		return 0;
	}
	
	int DrawLine( float x0,float y0,float x1,float y1 ){
		
		if( tformed ){
			float tx0=x0,tx1=x1;
			x0=tx0 * ix + y0 * jx + tx;
			y0=tx0 * iy + y0 * jy + ty;
			x1=tx1 * ix + y1 * jx + tx;
			y1=tx1 * iy + y1 * jy + ty;
		}

		Begin( GLES11.GL_LINES,2,null );

		vertices[vp]=x0;vertices[vp+1]=y0;
		vertices[vp+4]=x1;vertices[vp+5]=y1;

		colors[cp]=colors[cp+1]=colorARGB;

		return 0;
 	}

	int DrawOval( float x,float y,float w,float h ){

		float xr=w/2.0f;
		float yr=h/2.0f;

		int segs;	
		if( tformed ){
			float xx=xr*ix,xy=xr*iy,xd=(float)Math.sqrt(xx*xx+xy*xy);
			float yx=yr*jx,yy=yr*jy,yd=(float)Math.sqrt(yx*yx+yy*yy);
			segs=(int)( xd+yd );
		}else{
			segs=(int)( Math.abs(xr)+Math.abs(yr) );
		}

		if( segs>MAX_VERTICES ){
			segs=MAX_VERTICES;
		}else if( segs<12 ){
			segs=12;
		}else{
			segs&=~3;
		}
		
		x+=xr;
		y+=yr;
		
		Begin( GLES11.GL_TRIANGLE_FAN,segs,null );
		
		for( int i=0;i<segs;++i ){
			float th=i * 6.28318531f / segs;
			float x0=(float)(x+Math.cos(th)*xr);
			float y0=(float)(y+Math.sin(th)*yr);
			if( tformed ){
				float tx0=x0;
				x0=tx0 * ix + y0 * jx + tx;
				y0=tx0 * iy + y0 * jy + ty;
			}
			vertices[vp]=x0;
			vertices[vp+1]=y0;
			colors[cp+i]=colorARGB;
			vp+=4;
		}
		
		Flush();	//Note: could really queue these too now...
		
		return 0;
	}
	
	int DrawPoly( float[] verts ){
		if( verts.length<6 || verts.length>MAX_VERTICES*2 ) return 0;
	
		Begin( GLES11.GL_TRIANGLE_FAN,verts.length/2,null );
		
		if( tformed ){
			for( int i=0;i<verts.length;i+=2 ){
				vertices[vp  ]=verts[i] * ix + verts[i+1] * jx + tx;
				vertices[vp+1]=verts[i] * iy + verts[i+1] * jy + ty;
				colors[cp]=colorARGB;
				vp+=4;
				cp+=1;
			}
		}else{
			for( int i=0;i<verts.length;i+=2 ){
				vertices[vp  ]=verts[i];
				vertices[vp+1]=verts[i+1];
				colors[cp]=colorARGB;
				vp+=4;
				cp+=1;
			}
		}

		Flush();	//Note: could really queue these too now...

		return 0;
	}

	int DrawSurface( gxtkSurface surface,float x,float y ){
	
		float w=surface.width;
		float h=surface.height;
		float u0=0,u1=w*surface.uscale;
		float v0=0,v1=h*surface.vscale;
		
		float x0=x,x1=x+w,x2=x+w,x3=x;
		float y0=y,y1=y,y2=y+h,y3=y+h;
		
		if( tformed ){
			float tx0=x0,tx1=x1,tx2=x2,tx3=x3;
			x0=tx0 * ix + y0 * jx + tx;
			y0=tx0 * iy + y0 * jy + ty;
			x1=tx1 * ix + y1 * jx + tx;
			y1=tx1 * iy + y1 * jy + ty;
			x2=tx2 * ix + y2 * jx + tx;
			y2=tx2 * iy + y2 * jy + ty;
			x3=tx3 * ix + y3 * jx + tx;
			y3=tx3 * iy + y3 * jy + ty;
		}

		Begin( GLES11.GL_TRIANGLES,6,surface );
		
		vertices[vp]=x0;vertices[vp+1]=y0;vertices[vp+2]=u0;vertices[vp+3]=v0;
		vertices[vp+4]=x1;vertices[vp+5]=y1;vertices[vp+6]=u1;vertices[vp+7]=v0;
		vertices[vp+8]=x2;vertices[vp+9]=y2;vertices[vp+10]=u1;vertices[vp+11]=v1;
		
		vertices[vp+12]=x0;vertices[vp+13]=y0;vertices[vp+14]=u0;vertices[vp+15]=v0;
		vertices[vp+16]=x2;vertices[vp+17]=y2;vertices[vp+18]=u1;vertices[vp+19]=v1;
		vertices[vp+20]=x3;vertices[vp+21]=y3;vertices[vp+22]=u0;vertices[vp+23]=v1;

		colors[cp]=colors[cp+1]=colors[cp+2]=colors[cp+3]=colors[cp+4]=colors[cp+5]=colorARGB;

		return 0;
	}
	
	int DrawSurface2( gxtkSurface surface,float x,float y,int srcx,int srcy,int srcw,int srch ){
	
		float w=srcw;
		float h=srch;
		float u0=srcx*surface.uscale,u1=(srcx+srcw)*surface.uscale;
		float v0=srcy*surface.vscale,v1=(srcy+srch)*surface.vscale;
		
		float x0=x,x1=x+w,x2=x+w,x3=x;
		float y0=y,y1=y,y2=y+h,y3=y+h;
		
		if( tformed ){
			float tx0=x0,tx1=x1,tx2=x2,tx3=x3;
			x0=tx0 * ix + y0 * jx + tx;
			y0=tx0 * iy + y0 * jy + ty;
			x1=tx1 * ix + y1 * jx + tx;
			y1=tx1 * iy + y1 * jy + ty;
			x2=tx2 * ix + y2 * jx + tx;
			y2=tx2 * iy + y2 * jy + ty;
			x3=tx3 * ix + y3 * jx + tx;
			y3=tx3 * iy + y3 * jy + ty;
		}

		Begin( GLES11.GL_TRIANGLES,6,surface );
		
		vertices[vp]=x0;vertices[vp+1]=y0;vertices[vp+2]=u0;vertices[vp+3]=v0;
		vertices[vp+4]=x1;vertices[vp+5]=y1;vertices[vp+6]=u1;vertices[vp+7]=v0;
		vertices[vp+8]=x2;vertices[vp+9]=y2;vertices[vp+10]=u1;vertices[vp+11]=v1;
		
		vertices[vp+12]=x0;vertices[vp+13]=y0;vertices[vp+14]=u0;vertices[vp+15]=v0;
		vertices[vp+16]=x2;vertices[vp+17]=y2;vertices[vp+18]=u1;vertices[vp+19]=v1;
		vertices[vp+20]=x3;vertices[vp+21]=y3;vertices[vp+22]=u0;vertices[vp+23]=v1;

		colors[cp]=colors[cp+1]=colors[cp+2]=colors[cp+3]=colors[cp+4]=colors[cp+5]=colorARGB;

		return 0;
	}
	
	int ReadPixels( int[] pixels,int x,int y,int width,int height,int offset,int pitch ){
	
		Flush();
		
		int[] texels=new int[width*height];
		IntBuffer buf=IntBuffer.wrap( texels );

		GLES11.glReadPixels( x,this.height-y-height,width,height,GLES11.GL_RGBA,GLES11.GL_UNSIGNED_BYTE,buf );

		int i=0;
		for( int py=height-1;py>=0;--py ){
			int j=offset+py*pitch;
			for( int px=0;px<width;++px ){
				int p=texels[i++];
				//RGBA -> BGRA, Big Endian!
				pixels[j++]=(p&0xff000000)|((p<<16)&0xff0000)|(p&0xff00)|((p>>16)&0xff);
			}
		}
	
		return 0;
	}

	int WritePixels2( gxtkSurface surface,int[] pixels,int x,int y,int width,int height,int offset,int pitch ){
	
		Flush();
	
		surface.bitmap.setPixels( pixels,offset,pitch,x,y,width,height );
		
		surface.Invalidate();
	
		return 0;
	}
}

class gxtkSurface{

	Bitmap bitmap;
	
	int width,height;
	int twidth,theight;
	float uscale,vscale;
	boolean hasAlpha;
	int texId,seq;

	static Vector discarded=new Vector();
	
	gxtkSurface(){
	}
	
	gxtkSurface( Bitmap bitmap ){
		SetBitmap( bitmap );
	}
	
	void SetBitmap( Bitmap bitmap ){
		this.bitmap=bitmap;
		width=bitmap.getWidth();
		height=bitmap.getHeight();
		hasAlpha=bitmap.hasAlpha();
		twidth=Pow2Size( width );
		theight=Pow2Size( height );
		uscale=1.0f/(float)twidth;
		vscale=1.0f/(float)theight;
	}

	protected void finalize(){
		Discard();
	}
	
	int Pow2Size( int n ){
		int i=1;
		while( i<n ) i*=2;
		return i;
	}
	
	static void FlushDiscarded(){
		int n=discarded.size();
		if( n==0 ) return;
		int[] texs=new int[n];
		for( int i=0;i<n;++i ){
			texs[i]=((Integer)discarded.elementAt(i)).intValue();
		}
		GLES11.glDeleteTextures( n,texs,0 );
		discarded.clear();
	}
	
	void Invalidate(){
		if( texId!=0 ){
			if( seq==MonkeyGame.app.seq ) discarded.add( Integer.valueOf( texId ) );
			texId=0;
		}
	}
	
	void Bind(){
	
		if( texId!=0 && seq==MonkeyGame.app.seq ){
			GLES11.glBindTexture( GLES11.GL_TEXTURE_2D,texId );
			return;
		}
        
        if( bitmap==null ) throw new Error( "Attempt to use discarded image" );
		
		FlushDiscarded();

		int[] texs=new int[1];
		GLES11.glGenTextures( 1,texs,0 );
		texId=texs[0];
		if( texId==0 ) throw new Error( "glGenTextures failed" );
		seq=MonkeyGame.app.seq;
		
		GLES11.glBindTexture( GLES11.GL_TEXTURE_2D,texId );
		
		if( MonkeyConfig.MOJO_IMAGE_FILTERING_ENABLED.equals( "1" ) ){
			GLES11.glTexParameteri( GLES11.GL_TEXTURE_2D,GLES11.GL_TEXTURE_MAG_FILTER,GLES11.GL_LINEAR );
			GLES11.glTexParameteri( GLES11.GL_TEXTURE_2D,GLES11.GL_TEXTURE_MIN_FILTER,GLES11.GL_LINEAR );
		}else{
			GLES11.glTexParameteri( GLES11.GL_TEXTURE_2D,GLES11.GL_TEXTURE_MAG_FILTER,GLES11.GL_NEAREST );
			GLES11.glTexParameteri( GLES11.GL_TEXTURE_2D,GLES11.GL_TEXTURE_MIN_FILTER,GLES11.GL_NEAREST );
		}

		GLES11.glTexParameteri( GLES11.GL_TEXTURE_2D,GLES11.GL_TEXTURE_WRAP_S,GLES11.GL_CLAMP_TO_EDGE );
		GLES11.glTexParameteri( GLES11.GL_TEXTURE_2D,GLES11.GL_TEXTURE_WRAP_T,GLES11.GL_CLAMP_TO_EDGE );
		
		int pwidth=(width==twidth) ? width : width+1;
		int pheight=(height==theight) ? height : height+1;

		int sz=pwidth*pheight;
		int[] pixels=new int[sz];
		bitmap.getPixels( pixels,0,pwidth,0,0,width,height );
		
		//pad edges for non pow-2 images - not sexy!
		if( width!=pwidth ){
			for( int y=0;y<height;++y ){
				pixels[y*pwidth+width]=pixels[y*pwidth+width-1];
			}
		}
		if( height!=pheight ){
			for( int x=0;x<width;++x ){
				pixels[height*pwidth+x]=pixels[height*pwidth+x-pwidth];
			}
		}
		if( width!=pwidth && height!=pheight ){
			pixels[height*pwidth+width]=pixels[height*pwidth+width-pwidth-1];
		}
		
		GLES11.glPixelStorei( GLES11.GL_UNPACK_ALIGNMENT,1 );
		
		boolean hicolor_textures=MonkeyConfig.MOJO_HICOLOR_TEXTURES.equals( "1" );
		
		if( hicolor_textures && hasAlpha ){

			//RGBA8888...
			ByteBuffer buf=ByteBuffer.allocate( sz*4 );
			buf.order( ByteOrder.BIG_ENDIAN );

			for( int i=0;i<sz;++i ){
				int p=pixels[i];
				int a=(p>>24) & 255;
				int r=((p>>16) & 255)*a/255;
				int g=((p>>8) & 255)*a/255;
				int b=(p & 255)*a/255;
				buf.putInt( (r<<24)|(g<<16)|(b<<8)|a );
			}
			buf.position( 0 );
			GLES11.glTexImage2D( GLES11.GL_TEXTURE_2D,0,GLES11.GL_RGBA,twidth,theight,0,GLES11.GL_RGBA,GLES11.GL_UNSIGNED_BYTE,null );
			GLES11.glTexSubImage2D( GLES11.GL_TEXTURE_2D,0,0,0,pwidth,pheight,GLES11.GL_RGBA,GLES11.GL_UNSIGNED_BYTE,buf );

		}else if( hicolor_textures && !hasAlpha ){
		
			//RGB888...
			ByteBuffer buf=ByteBuffer.allocate( sz*3 );
			buf.order( ByteOrder.BIG_ENDIAN );
			
			for( int i=0;i<sz;++i ){
				int p=pixels[i];
				int r=(p>>16) & 255;
				int g=(p>>8) & 255;
				int b=p & 255;
				buf.put( (byte)r );
				buf.put( (byte)g );
				buf.put( (byte)b );
			}
			buf.position( 0 );
			GLES11.glTexImage2D( GLES11.GL_TEXTURE_2D,0,GLES11.GL_RGB,twidth,theight,0,GLES11.GL_RGB,GLES11.GL_UNSIGNED_BYTE,null );
			GLES11.glTexSubImage2D( GLES11.GL_TEXTURE_2D,0,0,0,pwidth,pheight,GLES11.GL_RGB,GLES11.GL_UNSIGNED_BYTE,buf );
			
		}else if( !hicolor_textures && hasAlpha ){

			//16 bit RGBA...
			ByteBuffer buf=ByteBuffer.allocate( sz*2 );
			buf.order( ByteOrder.LITTLE_ENDIAN );
			
			//do we need 4 bit alpha?
			boolean a4=false;
			for( int i=0;i<sz;++i ){
				int a=(pixels[i]>>28) & 15;
				if( a!=0 && a!=15 ){
					a4=true;
					break;
				}
			}
			if( a4 ){
				//RGBA4444...
				for( int i=0;i<sz;++i ){
					int p=pixels[i];
					int a=(p>>28) & 15;
					int r=((p>>20) & 15)*a/15;
					int g=((p>>12) & 15)*a/15;
					int b=((p>> 4) & 15)*a/15;
					buf.putShort( (short)( (r<<12)|(g<<8)|(b<<4)|a ) );
				}
				buf.position( 0 );
				GLES11.glTexImage2D( GLES11.GL_TEXTURE_2D,0,GLES11.GL_RGBA,twidth,theight,0,GLES11.GL_RGBA,GLES11.GL_UNSIGNED_SHORT_4_4_4_4,null );
				GLES11.glTexSubImage2D( GLES11.GL_TEXTURE_2D,0,0,0,pwidth,pheight,GLES11.GL_RGBA,GLES11.GL_UNSIGNED_SHORT_4_4_4_4,buf );
			}else{
				//RGBA5551...
				for( int i=0;i<sz;++i ){
					int p=pixels[i];
					int a=(p>>31) & 1;
					int r=((p>>19) & 31)*a;
					int g=((p>>11) & 31)*a;
					int b=((p>> 3) & 31)*a;
					buf.putShort( (short)( (r<<11)|(g<<6)|(b<<1)|a ) );
				}
				buf.position( 0 );
				GLES11.glTexImage2D( GLES11.GL_TEXTURE_2D,0,GLES11.GL_RGBA,twidth,theight,0,GLES11.GL_RGBA,GLES11.GL_UNSIGNED_SHORT_5_5_5_1,null );
				GLES11.glTexSubImage2D( GLES11.GL_TEXTURE_2D,0,0,0,pwidth,pheight,GLES11.GL_RGBA,GLES11.GL_UNSIGNED_SHORT_5_5_5_1,buf );
			}
		}else if( !hicolor_textures && !hasAlpha ){
		
			ByteBuffer buf=ByteBuffer.allocate( sz*2 );
			buf.order( ByteOrder.LITTLE_ENDIAN );
			
			//RGB565...
			for( int i=0;i<sz;++i ){
				int p=pixels[i];
				int r=(p>>19) & 31;
				int g=(p>>10) & 63;
				int b=(p>> 3) & 31;
				buf.putShort( (short)( (r<<11)|(g<<5)|b ) );
			}
			GLES11.glTexImage2D( GLES11.GL_TEXTURE_2D,0,GLES11.GL_RGB,twidth,theight,0,GLES11.GL_RGB,GLES11.GL_UNSIGNED_SHORT_5_6_5,null );
			GLES11.glTexSubImage2D( GLES11.GL_TEXTURE_2D,0,0,0,pwidth,pheight,GLES11.GL_RGB,GLES11.GL_UNSIGNED_SHORT_5_6_5,buf );
		}
	}
	
	//***** GXTK API *****
	
	int Discard(){
		Invalidate();
		bitmap=null;
		return 0;
	}

	int Width(){
		return width;
	}
	
	int Height(){
		return height;
	}

	int Loaded(){
		return 1;
	}
	
	boolean OnUnsafeLoadComplete(){
		return true;
	}
	
}

//***** gxtkInput *****

class gxtkInput implements SensorEventListener{

	int keyStates[]=new int[512];
	int charQueue[]=new int[32];
	int charPut,charGet;
	float touchX[]=new float[32];
	float touchY[]=new float[32];
	float accelX,accelY,accelZ;
	boolean keyboardEnabled,keyboardLost;
	
	static int KEY_LMB=1;
	static int KEY_TOUCH0=0x180;
	
	gxtkInput(){
		SensorManager sensorManager=(SensorManager)MonkeyGame.activity.getSystemService( Context.SENSOR_SERVICE );

//		List<Sensor> sensorList=sensorManager.getSensorList( Sensor.TYPE_ORIENTATION );
		List<Sensor> sensorList=sensorManager.getSensorList( Sensor.TYPE_ACCELEROMETER );
		Iterator<Sensor> it=sensorList.iterator();
		while( it.hasNext() ){
			Sensor sensor=it.next();
			sensorManager.registerListener( this,sensor,SensorManager.SENSOR_DELAY_GAME );
			break;	//which one?
		}
	}

	//SensorEventListener
	public void onAccuracyChanged( Sensor sensor,int accuracy ){
	}
	
	//SensorEventListener
	public void onSensorChanged( SensorEvent event ){
		Sensor sensor=event.sensor;
		switch( sensor.getType() ){
		case Sensor.TYPE_ORIENTATION:
			break;
		case Sensor.TYPE_ACCELEROMETER:
			accelX=-event.values[0]/9.81f;
			accelY=event.values[1]/9.81f;
			accelZ=-event.values[2]/9.81f;
			break;
		}
	}
	
	void BeginUpdate(){
		//
		//Ok, this isn't very polite - if keyboard enabled, we just thrash showSoftInput.
		//
		//But showSoftInput doesn't seem to be too reliable - esp. after onResume - and I haven't found a way to
		//determine if keyboard is showing, so what can yer do...
		//
		if( keyboardEnabled ){
			InputMethodManager mgr=(InputMethodManager)MonkeyGame.activity.getSystemService( Context.INPUT_METHOD_SERVICE );
			mgr.showSoftInput( MonkeyGame.view,0 );//InputMethodManager.SHOW_IMPLICIT );
		}
	}
	
	void EndUpdate(){
		for( int i=0;i<512;++i ){
			keyStates[i]&=0x100;
		}
		charGet=charPut=0;
	}
	
	void PutChar( int chr ){
		if( charPut<32 ) charQueue[charPut++]=chr;
	}

	void OnKeyDown( int key ){
		if( (keyStates[key]&0x100)==0 ){
			keyStates[key]|=0x100;
			++keyStates[key];
		}
	}
	
	void OnKeyUp( int key ){
		keyStates[key]&=0xff;
	}
	
	//***** GXTK API *****

	int SetKeyboardEnabled( int enabled ){
	
		InputMethodManager mgr=(InputMethodManager)MonkeyGame.activity.getSystemService( Context.INPUT_METHOD_SERVICE );
		
		if( enabled!=0 ){
		
			// Hack for someone's phone...My LG or Samsung don't need it...
			mgr.hideSoftInputFromWindow( MonkeyGame.view.getWindowToken(),0 );
			
			keyboardEnabled=true;
			mgr.showSoftInput( MonkeyGame.view,0 );//InputMethodManager.SHOW_IMPLICIT );
		}else{
			keyboardEnabled=false;
			mgr.hideSoftInputFromWindow( MonkeyGame.view.getWindowToken(),0 );
		}
		
		return 0;
	}
	
	int KeyDown( int key ){
		if( key>0 && key<512 ){
			if( key==KEY_LMB ) key=KEY_TOUCH0;
			return keyStates[key] >> 8;
		}
		return 0;
	}

	int KeyHit( int key ){
		if( key>0 && key<512 ){
			if( key==KEY_LMB ) key=KEY_TOUCH0;
			return keyStates[key] & 0xff;
		}
		return 0;
	}

	int GetChar(){
		if( charGet!=charPut ){
			return charQueue[ charGet++ ];
		}
		return 0;
	}
	
	float MouseX(){
		return touchX[0];
	}

	float MouseY(){
		return touchY[0];
	}

	float JoyX( int index ){
		return 0;
	}
	
	float JoyY( int index ){
		return 0;
	}
	
	float JoyZ( int index ){
		return 0;
	}
	
	float TouchX( int index ){
		if( index>=0 && index<32 ){
			return touchX[index];
		}
		return 0;
	}
	
	float TouchY( int index ){
		if( index>=0 && index<32 ){
			return touchY[index];
		}
		return 0;
	}
	
	float AccelX(){
		return accelX;
	}

	float AccelY(){
		return accelY;
	}

	float AccelZ(){
		return accelZ;
	}
}

class gxtkChannel{
	int stream;		//SoundPool stream ID, 0=none
	float volume=1;
	float rate=1;
	float pan;
	int state;
};

class gxtkAudio{

	SoundPool pool;
	MediaPlayer music;
	float musicVolume=1;
	int musicState=0;
	
	gxtkChannel[] channels=new gxtkChannel[32];
	
	gxtkAudio(){
		pool=new SoundPool( 32,AudioManager.STREAM_MUSIC,0 );
		for( int i=0;i<32;++i ){
			channels[i]=new gxtkChannel();
		}
	}
	
	void OnSuspend(){
		if( musicState==1 ) music.pause();
		for( int i=0;i<32;++i ){
			if( channels[i].state==1 ) pool.pause( channels[i].stream );
		}
	}
	
	void OnResume(){
		if( musicState==1 ) music.start();
		for( int i=0;i<32;++i ){
			if( channels[i].state==1 ) pool.resume( channels[i].stream );
		}
	}
	
	void OnDestroy(){
		for( int i=0;i<32;++i ){
			if( channels[i].state!=0 ) pool.stop( channels[i].stream );
		}
		pool.release();
		pool=null;
	}
	
	//***** GXTK API *****

	gxtkSample LoadSample__UNSAFE__( gxtkSample sample,String path ){
		gxtkSample.FlushDiscarded( pool );
		int sound=MonkeyData.loadSound( path,pool );
		if( sound==0 ) return null;
		sample.SetSound( sound );
		return sample;
	}
	
	gxtkSample LoadSample( String path ){
		return LoadSample__UNSAFE__( new gxtkSample(),path );
	}
	
	int PlaySample( gxtkSample sample,int channel,int flags ){
		gxtkChannel chan=channels[channel];
		if( chan.stream!=0 ) pool.stop( chan.stream );
		float rv=(chan.pan * .5f + .5f) * chan.volume;
		float lv=chan.volume-rv;
		int loops=(flags&1)!=0 ? -1 : 0;

		//chan.stream=pool.play( sample.sound,lv,rv,0,loops,chan.rate );
		//chan.state=1;
		//return 0;
		
		//Ugly as hell, but seems to work for now...pauses 10 secs max...
		for( int i=0;i<100;++i ){
			chan.stream=pool.play( sample.sound,lv,rv,0,loops,chan.rate );
			if( chan.stream!=0 ){
				chan.state=1;
				return 0;
			}
//			throw new Error( "PlaySample failed to play sound" );
			try{
				Thread.sleep( 100 );
			}catch( java.lang.InterruptedException ex ){
			}
		}
		throw new Error( "PlaySample failed to play sound" );
	}
	
	int StopChannel( int channel ){
		gxtkChannel chan=channels[channel];
		if( chan.state!=0 ){
			pool.stop( chan.stream );
			chan.state=0;
		}
		return 0;
	}
	
	int PauseChannel( int channel ){
		gxtkChannel chan=channels[channel];
		if( chan.state==1 ){
			pool.pause( chan.stream );
			chan.state=2;
		}
		return 0;
	}
	
	int ResumeChannel( int channel ){
		gxtkChannel chan=channels[channel];
		if( chan.state==2 ){
			pool.resume( chan.stream );
			chan.state=1;
		}
		return 0;
	}
	
	int ChannelState( int channel ){
		return -1;
	}
	
	int SetVolume( int channel,float volume ){
		gxtkChannel chan=channels[channel];
		chan.volume=volume;
		if( chan.stream!=0 ){
			float rv=(chan.pan * .5f + .5f) * chan.volume;
			float lv=chan.volume-rv;
			pool.setVolume( chan.stream,lv,rv );
		}
		return 0;
	}
	
	int SetPan( int channel,float pan ){
		gxtkChannel chan=channels[channel];
		chan.pan=pan;
		if( chan.stream!=0 ){
			float rv=(chan.pan * .5f + .5f) * chan.volume;
			float lv=chan.volume-rv;
			pool.setVolume( chan.stream,lv,rv );
		}
		return 0;
	}

	int SetRate( int channel,float rate ){
		gxtkChannel chan=channels[channel];
		chan.rate=rate;
		if( chan.stream!=0 ){
			pool.setRate( chan.stream,chan.rate );
		}
		return 0;
	}
	
	int PlayMusic( String path,int flags ){
		StopMusic();
		music=MonkeyData.openMedia( path );
		if( music==null ) return -1;
		music.setLooping( (flags&1)!=0 );
		music.setVolume( musicVolume,musicVolume );
		music.start();
		musicState=1;
		return 0;
	}
	
	int StopMusic(){
		if( musicState!=0 ){
			music.stop();
			music.release();
			musicState=0;
			music=null;
		}
		return 0;
	}
	
	int PauseMusic(){
		if( musicState==1 && music.isPlaying() ){
			music.pause();
			musicState=2;
		}
		return 0;
	}
	
	int ResumeMusic(){
		if( musicState==2 ){
			music.start();
			musicState=1;
		}
		return 0;
	}
	
	int MusicState(){
		if( musicState==1 && !music.isPlaying() ) musicState=0;
		return musicState;
	}
	
	int SetMusicVolume( float volume ){
		if( musicState!=0 ) music.setVolume( volume,volume );
		musicVolume=volume;
		return 0;
	}	
}

class gxtkSample{

	int sound;
	
	static Vector discarded=new Vector();
	
	gxtkSample(){
	}
	
	gxtkSample( int sound ){
		this.sound=sound;
	}
	
	void SetSound( int sound ){
		this.sound=sound;
	}
	
	protected void finalize(){
		Discard();
	}
	
	static void FlushDiscarded( SoundPool pool ){
		int n=discarded.size();
		if( n==0 ) return;
		Vector out=new Vector();
		for( int i=0;i<n;++i ){
			Integer val=(Integer)discarded.elementAt(i);
			if( pool.unload( val.intValue() ) ){
//				bb_std_lang.print( "unload OK!" );
			}else{
//				bb_std_lang.print( "unload failed!" );
				out.add( val );
			}
		}
		discarded=out;
//		bb_std_lang.print( "undiscarded="+out.size() );
	}

	//***** GXTK API *****
	
	int Discard(){
		if( sound!=0 ){
			discarded.add( Integer.valueOf( sound ) );
			sound=0;
		}
		return 0;
	}
}

class BBThread implements Runnable{

	boolean _running;
	Thread _thread;
	
	boolean IsRunning(){
		return _running;
	}
	
	void Start(){
		if( _running ) return;
		_running=true;
		_thread=new Thread( this );
		_thread.start();
	}
	
	void Wait(){
		while( _running ){
			try{
				_thread.join();
			}catch( InterruptedException ex ){
			}
		}
	}
	
	void Run__UNSAFE__(){
	}

	public void run(){
		Run__UNSAFE__();
		_running=false;
	}
}

class BBStream{

	int Eof(){
		return 0;
	}
	
	void Close(){
	}
	
	int Length(){
		return 0;
	}
	
	int Position(){
		return 0;
	}
	
	int Seek( int position ){
		return 0;
	}
	
	int Read( BBDataBuffer buffer,int offset,int count ){
		return 0;
	}
	
	int Write( BBDataBuffer buffer,int offset,int count ){
		return 0;
	}
}

class BBTcpStream extends BBStream{
	
	java.net.Socket _sock;
	InputStream _input;
	OutputStream _output;
	int _state;				//0=INIT, 1=CONNECTED, 2=CLOSED, -1=ERROR

	boolean Connect( String addr,int port ){
	
		if( _state!=0 ) return false;
		
		try{
			_sock=new java.net.Socket( addr,port );
			if( _sock.isConnected() ){
				_input=_sock.getInputStream();
				_output=_sock.getOutputStream();
				_state=1;
				return true;
			}
		}catch( IOException ex ){
		}
		
		_state=1;
		_sock=null;
		return false;
	}
	
	int ReadAvail(){
		try{
			return _input.available();
		}catch( IOException ex ){
		}
		_state=-1;
		return 0;
	}
	
	int WriteAvail(){
		return 0;
	}
	
	int Eof(){
		if( _state>=0 ) return (_state==2) ? 1 : 0;
		return -1;
	}
	
	void Close(){

		if( _sock==null ) return;
		
		try{
			_sock.close();
			if( _state==1 ) _state=2;
		}catch( IOException ex ){
			_state=-1;
		}
		_sock=null;
	}
	
	int Read( BBDataBuffer buffer,int offset,int count ){

		if( _state!=1 ) return 0;
		
		try{
			int n=_input.read( buffer._data.array(),offset,count );
			if( n>=0 ) return n;
			_state=2;
		}catch( IOException ex ){
			_state=-1;
		}
		return 0;
	}
	
	int Write( BBDataBuffer buffer,int offset,int count ){

		if( _state!=1 ) return 0;
		
		try{
			_output.write( buffer._data.array(),offset,count );
			return count;
		}catch( IOException ex ){
			_state=-1;
		}
		return 0;
	}
}

class BBFileStream extends BBStream{

	boolean Open( String path,String mode ){
		if( _stream!=null ) return false;
		
		try{
			
			path=MonkeyData.filePath( path );
			if( path=="" ) return false;
	
			if( mode.equals("r") ){
				mode="r";
			}else if( mode.equals("w") ){
				File f=new File(path);
				f.delete();
				if( f.exists() ) return false;
				mode="rw";
			}else if( mode.equals("u") ){
				mode="rw";
			}else{
				return false;
			}
		
			_stream=new RandomAccessFile( path,mode );
			_position=_stream.getFilePointer();
			_length=_stream.length();
			return true;
			
		}catch( IOException ex ){
		}
		_stream=null;
		_position=0;
		_length=0;
		return false;
	}

	void Close(){
		if( _stream==null ) return;

		try{
			_stream.close();
		}catch( IOException ex ){
		}
		_stream=null;
		_position=0;
		_length=0;
	}
	
	int Eof(){
		if( _stream==null ) return -1;
		
		return (_position==_length) ? 1 : 0;
	}
	
	int Length(){
		return (int)_length;
	}
	
	int Offset(){
		return (int)_position;
	}
	
	int Seek( int offset ){
		try{
			_stream.seek( offset );
			_position=_stream.getFilePointer();
		}catch( IOException ex ){
		}
		return (int)_position;
	}
		
	int Read( BBDataBuffer buffer,int offset,int count ){
		if( _stream==null ) return 0;
		
		try{
			int n=_stream.read( buffer._data.array(),offset,count );
			if( n>=0 ){
				_position+=n;
				return n;
			}
		}catch( IOException ex ){
		}
		return 0;
	}
	
	int Write( BBDataBuffer buffer,int offset,int count ){
		if( _stream==null ) return 0;
		
		try{
			_stream.write( buffer._data.array(),offset,count );
			_position+=count;
			if( _position>_length ) _length=_position;
			return count;
		}catch( IOException ex ){
		}
		return 0;
	}

	RandomAccessFile _stream;
	long _position,_length;
}
class bb_app_App extends Object{
	public bb_app_App g_new(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<109>";
		bb_app.bb_app_device=(new bb_app_AppDevice()).g_new(this);
		bb_std_lang.popErr();
		return this;
	}
	public int m_OnCreate(){
		bb_std_lang.pushErr();
		bb_std_lang.popErr();
		return 0;
	}
	public int m_OnUpdate(){
		bb_std_lang.pushErr();
		bb_std_lang.popErr();
		return 0;
	}
	public int m_OnSuspend(){
		bb_std_lang.pushErr();
		bb_std_lang.popErr();
		return 0;
	}
	public int m_OnResume(){
		bb_std_lang.pushErr();
		bb_std_lang.popErr();
		return 0;
	}
	public int m_OnRender(){
		bb_std_lang.pushErr();
		bb_std_lang.popErr();
		return 0;
	}
	public int m_OnLoading(){
		bb_std_lang.pushErr();
		bb_std_lang.popErr();
		return 0;
	}
}
class bb__Beacon extends bb_app_App{
	public bb__Beacon g_new(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<3>";
		super.g_new();
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<3>";
		bb_std_lang.popErr();
		return this;
	}
	public int m_OnCreate(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<20>";
		bb_challengergui.bb_challengergui_CHGUI_MobileMode=1;
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<21>";
		bb_app.bb_app_SetUpdateRate(30);
		bb_std_lang.popErr();
		return 0;
	}
	bb_tcpstream_TcpStream f_Server=null;
	bb_challengergui_CHGUI f_Games=null;
	bb_challengergui_CHGUI f_Title=null;
	bb_challengergui_CHGUI f_ServerLabel=null;
	bb_challengergui_CHGUI f_PwLabel=null;
	bb_challengergui_CHGUI f_Pw=null;
	bb_challengergui_CHGUI f_BeaconList=null;
	bb_challengergui_CHGUI f_On_Off=null;
	boolean f_isOn=false;
	public int m_OnRender(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<25>";
		String t_=bb_data2.bb_data2_STATUS;
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<26>";
		if(t_.compareTo("connecting")==0){
			bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<27>";
			if(f_Server.m_Connect("www.fuzzit.us",80)){
				bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<28>";
				bb_protocol.bb_protocol_RequestGameList(f_Server);
				bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<29>";
				bb_data2.bb_data2_STATUS="normal";
			}
		}else{
			bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<31>";
			if(t_.compareTo("normal")==0){
				bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<32>";
				bb_protocol.bb_protocol_ReadProtocol(f_Server);
				bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<33>";
				if(bb_protocol.bb_protocol_LastP==4){
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<34>";
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<34>";
					String[] t_2=bb_protocol.bb_protocol_SList;
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<34>";
					int t_3=0;
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<34>";
					while(t_3<bb_std_lang.arrayLength(t_2)){
						bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<34>";
						String t_eS=t_2[t_3];
						bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<34>";
						t_3=t_3+1;
						bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<35>";
						bb_challengergui.bb_challengergui_CreateDropdownItem(t_eS,f_Games,0);
					}
				}
				bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<38>";
				bb_protocol.bb_protocol_ResetP();
				bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<40>";
				bb_graphics.bb_graphics_Cls(247.0f,247.0f,247.0f);
				bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<42>";
				bb_challengergui.bb_challengergui_CHGUI_Draw();
			}else{
				bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<43>";
				if(t_.compareTo("start")==0){
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<44>";
					bb_challengergui.bb_challengergui_CHGUI_Start();
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<46>";
					f_Title=bb_data2.bb_data2_CScale(bb_challengergui.bb_challengergui_CreateLabel(50,10,"Beacon Config",null));
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<47>";
					f_ServerLabel=bb_data2.bb_data2_CScale(bb_challengergui.bb_challengergui_CreateLabel(5,60,"Server Type: Static",null));
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<48>";
					f_Games=bb_data2.bb_data2_CScale(bb_challengergui.bb_challengergui_CreateDropdown(10,110,(int)(bb_data2.bb_data2_SCALE_W-20.0f),40,"Choose Game",null));
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<49>";
					f_PwLabel=bb_data2.bb_data2_CScale(bb_challengergui.bb_challengergui_CreateLabel(5,160,"Password:",null));
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<50>";
					f_Pw=bb_data2.bb_data2_CScale(bb_challengergui.bb_challengergui_CreateTextfield(120,155,170,45,"",null));
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<51>";
					f_BeaconList=bb_data2.bb_data2_CScale(bb_challengergui.bb_challengergui_CreateDropdown(10,210,(int)(bb_data2.bb_data2_SCALE_W-20.0f),40,"Choose Beacon",null));
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<52>";
					f_On_Off=bb_data2.bb_data2_CScale(bb_challengergui.bb_challengergui_CreateButton(10,(int)(bb_data2.bb_data2_SCALE_H-50.0f),(int)(bb_data2.bb_data2_SCALE_W-20.0f),40,"On/Off",null));
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<54>";
					f_isOn=false;
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<55>";
					f_Server=(new bb_tcpstream_TcpStream()).g_new();
					bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<56>";
					bb_data2.bb_data2_STATUS="connecting";
				}
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	public int m_OnUpdate(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<61>";
		String t_=bb_data2.bb_data2_STATUS;
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<62>";
		if(t_.compareTo("connecting")==0){
			bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<63>";
			bb_challengergui.bb_challengergui_CHGUI_Update();
		}else{
			bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<64>";
			if(t_.compareTo("normal")==0){
				bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<65>";
				bb_challengergui.bb_challengergui_CHGUI_Update();
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
}
class bb_app_AppDevice extends gxtkApp{
	bb_app_App f_app=null;
	public bb_app_AppDevice g_new(bb_app_App t_app){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<49>";
		this.f_app=t_app;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<50>";
		bb_graphics.bb_graphics_SetGraphicsDevice(GraphicsDevice());
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<51>";
		bb_input.bb_input_SetInputDevice(InputDevice());
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<52>";
		bb_audio.bb_audio_SetAudioDevice(AudioDevice());
		bb_std_lang.popErr();
		return this;
	}
	public bb_app_AppDevice g_new2(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<46>";
		bb_std_lang.popErr();
		return this;
	}
	public int OnCreate(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<56>";
		bb_graphics.bb_graphics_SetFont(null,32);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<57>";
		int t_=f_app.m_OnCreate();
		bb_std_lang.popErr();
		return t_;
	}
	public int OnUpdate(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<61>";
		int t_=f_app.m_OnUpdate();
		bb_std_lang.popErr();
		return t_;
	}
	public int OnSuspend(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<65>";
		int t_=f_app.m_OnSuspend();
		bb_std_lang.popErr();
		return t_;
	}
	public int OnResume(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<69>";
		int t_=f_app.m_OnResume();
		bb_std_lang.popErr();
		return t_;
	}
	public int OnRender(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<73>";
		bb_graphics.bb_graphics_BeginRender();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<74>";
		int t_r=f_app.m_OnRender();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<75>";
		bb_graphics.bb_graphics_EndRender();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<76>";
		bb_std_lang.popErr();
		return t_r;
	}
	public int OnLoading(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<80>";
		bb_graphics.bb_graphics_BeginRender();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<81>";
		int t_r=f_app.m_OnLoading();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<82>";
		bb_graphics.bb_graphics_EndRender();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<83>";
		bb_std_lang.popErr();
		return t_r;
	}
	int f_updateRate=0;
	public int SetUpdateRate(int t_hertz){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<87>";
		super.SetUpdateRate(t_hertz);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<88>";
		f_updateRate=t_hertz;
		bb_std_lang.popErr();
		return 0;
	}
}
class bb_graphics_Image extends Object{
	static int g_DefaultFlags;
	public bb_graphics_Image g_new(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<65>";
		bb_std_lang.popErr();
		return this;
	}
	gxtkSurface f_surface=null;
	int f_width=0;
	int f_height=0;
	bb_graphics_Frame[] f_frames=new bb_graphics_Frame[0];
	int f_flags=0;
	float f_tx=.0f;
	float f_ty=.0f;
	public int m_SetHandle(float t_tx,float t_ty){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<109>";
		this.f_tx=t_tx;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<110>";
		this.f_ty=t_ty;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<111>";
		this.f_flags=this.f_flags&-2;
		bb_std_lang.popErr();
		return 0;
	}
	public int m_ApplyFlags(int t_iflags){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<178>";
		f_flags=t_iflags;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<180>";
		if((f_flags&2)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<181>";
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<181>";
			bb_graphics_Frame[] t_=f_frames;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<181>";
			int t_2=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<181>";
			while(t_2<bb_std_lang.arrayLength(t_)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<181>";
				bb_graphics_Frame t_f=t_[t_2];
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<181>";
				t_2=t_2+1;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<182>";
				t_f.f_x+=1;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<184>";
			f_width-=2;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<187>";
		if((f_flags&4)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<188>";
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<188>";
			bb_graphics_Frame[] t_3=f_frames;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<188>";
			int t_4=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<188>";
			while(t_4<bb_std_lang.arrayLength(t_3)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<188>";
				bb_graphics_Frame t_f2=t_3[t_4];
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<188>";
				t_4=t_4+1;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<189>";
				t_f2.f_y+=1;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<191>";
			f_height-=2;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<194>";
		if((f_flags&1)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<195>";
			m_SetHandle((float)(f_width)/2.0f,(float)(f_height)/2.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<198>";
		if(bb_std_lang.arrayLength(f_frames)==1 && f_frames[0].f_x==0 && f_frames[0].f_y==0 && f_width==f_surface.Width() && f_height==f_surface.Height()){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<199>";
			f_flags|=65536;
		}
		bb_std_lang.popErr();
		return 0;
	}
	public bb_graphics_Image m_Init(gxtkSurface t_surf,int t_nframes,int t_iflags){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<136>";
		f_surface=t_surf;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<138>";
		f_width=f_surface.Width()/t_nframes;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<139>";
		f_height=f_surface.Height();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<141>";
		f_frames=new bb_graphics_Frame[t_nframes];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<142>";
		for(int t_i=0;t_i<t_nframes;t_i=t_i+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<143>";
			f_frames[t_i]=(new bb_graphics_Frame()).g_new(t_i*f_width,0);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<146>";
		m_ApplyFlags(t_iflags);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<147>";
		bb_std_lang.popErr();
		return this;
	}
	bb_graphics_Image f_source=null;
	public bb_graphics_Image m_Grab(int t_x,int t_y,int t_iwidth,int t_iheight,int t_nframes,int t_iflags,bb_graphics_Image t_source){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<151>";
		this.f_source=t_source;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<152>";
		f_surface=t_source.f_surface;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<154>";
		f_width=t_iwidth;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<155>";
		f_height=t_iheight;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<157>";
		f_frames=new bb_graphics_Frame[t_nframes];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<159>";
		int t_ix=t_x;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<159>";
		int t_iy=t_y;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<161>";
		for(int t_i=0;t_i<t_nframes;t_i=t_i+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<162>";
			if(t_ix+f_width>t_source.f_width){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<163>";
				t_ix=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<164>";
				t_iy+=f_height;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<166>";
			if(t_ix+f_width>t_source.f_width || t_iy+f_height>t_source.f_height){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<167>";
				bb_std_lang.error("Image frame outside surface");
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<169>";
			f_frames[t_i]=(new bb_graphics_Frame()).g_new(t_ix+t_source.f_frames[0].f_x,t_iy+t_source.f_frames[0].f_y);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<170>";
			t_ix+=f_width;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<173>";
		m_ApplyFlags(t_iflags);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<174>";
		bb_std_lang.popErr();
		return this;
	}
	public bb_graphics_Image m_GrabImage(int t_x,int t_y,int t_width,int t_height,int t_frames,int t_flags){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<104>";
		if(bb_std_lang.arrayLength(this.f_frames)!=1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<104>";
			bb_std_lang.popErr();
			return null;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<105>";
		bb_graphics_Image t_=((new bb_graphics_Image()).g_new()).m_Grab(t_x,t_y,t_width,t_height,t_frames,t_flags,this);
		bb_std_lang.popErr();
		return t_;
	}
	public int m_Width(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<76>";
		bb_std_lang.popErr();
		return f_width;
	}
	public int m_Height(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<80>";
		bb_std_lang.popErr();
		return f_height;
	}
}
class bb_graphics_GraphicsContext extends Object{
	public bb_graphics_GraphicsContext g_new(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<24>";
		bb_std_lang.popErr();
		return this;
	}
	bb_graphics_Image f_defaultFont=null;
	bb_graphics_Image f_font=null;
	int f_firstChar=0;
	int f_matrixSp=0;
	float f_ix=1.0f;
	float f_iy=.0f;
	float f_jx=.0f;
	float f_jy=1.0f;
	float f_tx=.0f;
	float f_ty=.0f;
	int f_tformed=0;
	int f_matDirty=0;
	float f_color_r=.0f;
	float f_color_g=.0f;
	float f_color_b=.0f;
	float f_alpha=.0f;
	int f_blend=0;
	float f_scissor_x=.0f;
	float f_scissor_y=.0f;
	float f_scissor_width=.0f;
	float f_scissor_height=.0f;
	float[] f_matrixStack=new float[192];
	public int m_Validate(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<35>";
		if((f_matDirty)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<36>";
			bb_graphics.bb_graphics_renderDevice.SetMatrix(bb_graphics.bb_graphics_context.f_ix,bb_graphics.bb_graphics_context.f_iy,bb_graphics.bb_graphics_context.f_jx,bb_graphics.bb_graphics_context.f_jy,bb_graphics.bb_graphics_context.f_tx,bb_graphics.bb_graphics_context.f_ty);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<37>";
			f_matDirty=0;
		}
		bb_std_lang.popErr();
		return 0;
	}
}
class bb_graphics_Frame extends Object{
	int f_x=0;
	int f_y=0;
	public bb_graphics_Frame g_new(int t_x,int t_y){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<18>";
		this.f_x=t_x;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<19>";
		this.f_y=t_y;
		bb_std_lang.popErr();
		return this;
	}
	public bb_graphics_Frame g_new2(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<13>";
		bb_std_lang.popErr();
		return this;
	}
}
abstract class bb_stream_Stream extends Object{
	static bb_databuffer_DataBuffer g__tmpbuf;
	abstract public int m_Write(bb_databuffer_DataBuffer t_buffer,int t_offset,int t_count);
	public void m__Write(int t_n){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<125>";
		if(m_Write(g__tmpbuf,0,t_n)!=t_n){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<125>";
			throw (new bb_stream_StreamWriteError()).g_new(this);
		}
		bb_std_lang.popErr();
	}
	public void m_WriteByte(int t_value){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<82>";
		g__tmpbuf.PokeByte(0,t_value);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<83>";
		m__Write(1);
		bb_std_lang.popErr();
	}
	public void m_WriteLine(String t_str){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<102>";
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<102>";
		String t_=t_str;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<102>";
		int t_2=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<102>";
		while(t_2<t_.length()){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<102>";
			int t_ch=(int)t_.charAt(t_2);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<102>";
			t_2=t_2+1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<103>";
			m_WriteByte(t_ch);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<105>";
		m_WriteByte(13);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<106>";
		m_WriteByte(10);
		bb_std_lang.popErr();
	}
	abstract public int m_Eof();
	abstract public int m_Read(bb_databuffer_DataBuffer t_buffer,int t_offset,int t_count);
	public String m_ReadLine(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<70>";
		bb_stack_Stack t_buf=(new bb_stack_Stack()).g_new();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<71>";
		while(!((m_Eof())!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<72>";
			int t_n=m_Read(g__tmpbuf,0,1);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<73>";
			if(!((t_n)!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<73>";
				break;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<74>";
			int t_ch=g__tmpbuf.PeekByte(0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<75>";
			if(!((t_ch)!=0) || t_ch==10){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<75>";
				break;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<76>";
			if(t_ch!=13){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<76>";
				t_buf.m_Push(t_ch);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<78>";
		String t_=bb_std_lang.fromChars(t_buf.m_ToArray());
		bb_std_lang.popErr();
		return t_;
	}
	public bb_stream_Stream g_new(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<23>";
		bb_std_lang.popErr();
		return this;
	}
}
class bb_tcpstream_TcpStream extends bb_stream_Stream{
	BBTcpStream f__stream=null;
	public boolean m_Connect(String t_host,int t_port){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<29>";
		boolean t_=f__stream.Connect(t_host,t_port);
		bb_std_lang.popErr();
		return t_;
	}
	public int m_ReadAvail(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<33>";
		int t_=f__stream.ReadAvail();
		bb_std_lang.popErr();
		return t_;
	}
	public bb_tcpstream_TcpStream g_new(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<24>";
		super.g_new();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<25>";
		f__stream=(new BBTcpStream());
		bb_std_lang.popErr();
		return this;
	}
	public int m_Eof(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<49>";
		int t_=f__stream.Eof();
		bb_std_lang.popErr();
		return t_;
	}
	public int m_Read(bb_databuffer_DataBuffer t_buffer,int t_offset,int t_count){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<65>";
		int t_=f__stream.Read(t_buffer,t_offset,t_count);
		bb_std_lang.popErr();
		return t_;
	}
	public int m_Write(bb_databuffer_DataBuffer t_buffer,int t_offset,int t_count){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<69>";
		int t_=f__stream.Write(t_buffer,t_offset,t_count);
		bb_std_lang.popErr();
		return t_;
	}
}
class bb_databuffer_DataBuffer extends BBDataBuffer{
	public bb_databuffer_DataBuffer g_new(int t_length){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/databuffer.monkey<41>";
		if(!_New(t_length)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/databuffer.monkey<41>";
			bb_std_lang.error("Allocate DataBuffer failed");
		}
		bb_std_lang.popErr();
		return this;
	}
	public bb_databuffer_DataBuffer g_new2(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/databuffer.monkey<38>";
		bb_std_lang.popErr();
		return this;
	}
}
abstract class bb_stream_StreamError extends ThrowableObject{
	bb_stream_Stream f__stream=null;
	public bb_stream_StreamError g_new(bb_stream_Stream t_stream){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<133>";
		f__stream=t_stream;
		bb_std_lang.popErr();
		return this;
	}
	public bb_stream_StreamError g_new2(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<130>";
		bb_std_lang.popErr();
		return this;
	}
}
class bb_stream_StreamWriteError extends bb_stream_StreamError{
	public bb_stream_StreamWriteError g_new(bb_stream_Stream t_stream){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<162>";
		super.g_new(t_stream);
		bb_std_lang.popErr();
		return this;
	}
	public bb_stream_StreamWriteError g_new2(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<159>";
		super.g_new2();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<159>";
		bb_std_lang.popErr();
		return this;
	}
}
class bb_stack_Stack extends Object{
	public bb_stack_Stack g_new(){
		bb_std_lang.pushErr();
		bb_std_lang.popErr();
		return this;
	}
	int[] f_data=new int[0];
	int f_length=0;
	public bb_stack_Stack g_new2(int[] t_data){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<13>";
		this.f_data=((int[])bb_std_lang.sliceArray(t_data,0));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<14>";
		this.f_length=bb_std_lang.arrayLength(t_data);
		bb_std_lang.popErr();
		return this;
	}
	public int m_Push(int t_value){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<52>";
		if(f_length==bb_std_lang.arrayLength(f_data)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<53>";
			f_data=(int[])bb_std_lang.resizeArray(f_data,f_length*2+10);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<55>";
		f_data[f_length]=t_value;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<56>";
		f_length+=1;
		bb_std_lang.popErr();
		return 0;
	}
	public int m_Push2(int[] t_values,int t_offset,int t_count){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<66>";
		for(int t_i=0;t_i<t_count;t_i=t_i+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<67>";
			m_Push(t_values[t_offset+t_i]);
		}
		bb_std_lang.popErr();
		return 0;
	}
	public int m_Push3(int[] t_values,int t_offset){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<60>";
		for(int t_i=t_offset;t_i<bb_std_lang.arrayLength(t_values);t_i=t_i+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<61>";
			m_Push(t_values[t_i]);
		}
		bb_std_lang.popErr();
		return 0;
	}
	public int[] m_ToArray(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<18>";
		int[] t_t=new int[f_length];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<19>";
		for(int t_i=0;t_i<f_length;t_i=t_i+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<20>";
			t_t[t_i]=f_data[t_i];
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<22>";
		bb_std_lang.popErr();
		return t_t;
	}
}
class bb_challengergui_CHGUI extends Object{
	public bb_challengergui_CHGUI g_new(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<139>";
		bb_std_lang.popErr();
		return this;
	}
	bb_challengergui_CHGUI f_Parent=null;
	float f_Value=.0f;
	String f_Text="";
	String f_Element="";
	bb_challengergui_CHGUI[] f_DropdownItems=new bb_challengergui_CHGUI[0];
	int f_Visible=1;
	int f_Minimised=0;
	float f_X=.0f;
	float f_Y=.0f;
	float f_W=.0f;
	float f_H=.0f;
	int f_Active=1;
	int f_Shadow=0;
	int f_Close=0;
	int f_CloseOver=0;
	int f_CloseDown=0;
	int f_Minimise=0;
	int f_MinimiseOver=0;
	int f_MinimiseDown=0;
	int f_HasMenu=0;
	int f_MenuHeight=0;
	int f_Tabbed=0;
	int f_TabHeight=0;
	bb_challengergui_CHGUI[] f_Buttons=new bb_challengergui_CHGUI[0];
	int f_Over=0;
	int f_Down=0;
	bb_challengergui_CHGUI[] f_ImageButtons=new bb_challengergui_CHGUI[0];
	bb_graphics_Image f_Img=null;
	bb_challengergui_CHGUI[] f_Tickboxes=new bb_challengergui_CHGUI[0];
	bb_challengergui_CHGUI[] f_Radioboxes=new bb_challengergui_CHGUI[0];
	bb_challengergui_CHGUI[] f_Listboxes=new bb_challengergui_CHGUI[0];
	bb_challengergui_CHGUI f_ListboxSlider=null;
	int f_ListboxNumber=0;
	bb_challengergui_CHGUI[] f_ListboxItems=new bb_challengergui_CHGUI[0];
	int f_ListHeight=0;
	bb_challengergui_CHGUI f_SelectedListboxItem=null;
	bb_challengergui_CHGUI[] f_HSliders=new bb_challengergui_CHGUI[0];
	int f_MinusOver=0;
	int f_MinusDown=0;
	int f_PlusOver=0;
	int f_PlusDown=0;
	int f_SliderOver=0;
	int f_SliderDown=0;
	float f_Minimum=.0f;
	float f_Stp=.0f;
	float f_SWidth=.0f;
	bb_challengergui_CHGUI[] f_VSliders=new bb_challengergui_CHGUI[0];
	bb_challengergui_CHGUI[] f_Textfields=new bb_challengergui_CHGUI[0];
	int f_OnFocus=0;
	int f_Cursor=0;
	bb_challengergui_CHGUI[] f_Labels=new bb_challengergui_CHGUI[0];
	bb_challengergui_CHGUI[] f_Dropdowns=new bb_challengergui_CHGUI[0];
	int f_DropNumber=0;
	bb_challengergui_CHGUI[] f_Menus=new bb_challengergui_CHGUI[0];
	bb_challengergui_CHGUI[] f_Tabs=new bb_challengergui_CHGUI[0];
	bb_challengergui_CHGUI f_CurrentTab=null;
	bb_challengergui_CHGUI[] f_BottomList=new bb_challengergui_CHGUI[0];
	bb_challengergui_CHGUI[] f_VariList=new bb_challengergui_CHGUI[0];
	bb_challengergui_CHGUI[] f_TopList=new bb_challengergui_CHGUI[0];
	bb_challengergui_CHGUI[] f_MenuItems=new bb_challengergui_CHGUI[0];
	int f_IsMenuParent=0;
	int f_Tick=0;
	int f_MenuWidth=0;
	int f_MenuNumber=0;
	String f_Tooltip="";
	int f_Moveable=0;
	int f_Mode=0;
	int f_IsParent=0;
	int f_SubWindow=0;
	int f_ReOrdered=0;
	int f_Clicked=0;
	int f_DoubleClickMillisecs=0;
	int f_DoubleClicked=0;
	public int m_CheckClicked(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<349>";
		if(bb_challengergui.bb_challengergui_CHGUI_RealActive(this)==0 || bb_challengergui.bb_challengergui_CHGUI_RealVisible(this)==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<350>";
			f_Over=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<351>";
			f_Down=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<352>";
			f_Clicked=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<354>";
		if(((f_Over)!=0) && ((f_Down)!=0) && bb_input.bb_input_TouchDown(0)==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<355>";
			f_Clicked=1;
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<357>";
			f_Clicked=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<361>";
		if((f_Clicked)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<362>";
			if(bb_app.bb_app_Millisecs()<f_DoubleClickMillisecs){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<362>";
				f_DoubleClicked=1;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<363>";
			f_DoubleClickMillisecs=bb_app.bb_app_Millisecs()+275;
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<365>";
			f_DoubleClicked=0;
		}
		bb_std_lang.popErr();
		return 0;
	}
	int f_StartOvertime=0;
	int f_OverTime=0;
	public int m_CheckOver(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<276>";
		if(f_Minimised==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<277>";
			if(this!=bb_challengergui.bb_challengergui_CHGUI_Canvas){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<278>";
				int t_XX=bb_challengergui.bb_challengergui_CHGUI_RealX(this);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<279>";
				int t_YY=bb_challengergui.bb_challengergui_CHGUI_RealY(this);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<281>";
				if(bb_challengergui.bb_challengergui_CHGUI_OverFlag==0 && bb_challengergui.bb_challengergui_CHGUI_IgnoreMouse==0 && bb_input.bb_input_TouchX(0)>(float)(t_XX) && bb_input.bb_input_TouchX(0)<(float)(t_XX)+f_W && bb_input.bb_input_TouchY(0)>(float)(t_YY) && bb_input.bb_input_TouchY(0)<(float)(t_YY)+f_H){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<282>";
					f_Over=1;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<283>";
					bb_challengergui.bb_challengergui_CHGUI_Over=1;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<284>";
					bb_challengergui.bb_challengergui_CHGUI_OverFlag=1;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<285>";
					if(f_Element.compareTo("Window")!=0){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<285>";
						bb_challengergui.bb_challengergui_CHGUI_DragOver=1;
					}
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<286>";
					f_OverTime=bb_app.bb_app_Millisecs()-f_StartOvertime;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<288>";
					f_Over=0;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<289>";
					f_OverTime=0;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<290>";
					f_StartOvertime=bb_app.bb_app_Millisecs();
				}
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<294>";
			if(this!=bb_challengergui.bb_challengergui_CHGUI_Canvas){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<295>";
				int t_XX2=bb_challengergui.bb_challengergui_CHGUI_RealX(this);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<296>";
				int t_YY2=bb_challengergui.bb_challengergui_CHGUI_RealY(this);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<297>";
				if(bb_challengergui.bb_challengergui_CHGUI_OverFlag==0 && bb_challengergui.bb_challengergui_CHGUI_IgnoreMouse==0 && bb_input.bb_input_TouchX(0)>(float)(t_XX2) && bb_input.bb_input_TouchX(0)<(float)(t_XX2)+f_W && bb_input.bb_input_TouchY(0)>(float)(t_YY2) && bb_input.bb_input_TouchY(0)<(float)(t_YY2)+bb_challengergui.bb_challengergui_CHGUI_TitleHeight){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<298>";
					f_Over=1;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<299>";
					bb_challengergui.bb_challengergui_CHGUI_Over=1;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<300>";
					bb_challengergui.bb_challengergui_CHGUI_OverFlag=1;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<301>";
					f_OverTime=bb_app.bb_app_Millisecs()-f_StartOvertime;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<303>";
					f_Over=0;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<304>";
					f_OverTime=0;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<305>";
					f_StartOvertime=bb_app.bb_app_Millisecs();
				}
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	int f_StartDowntime=0;
	int f_DownTime=0;
	bb_challengergui_CHGUI f_TopVari=null;
	bb_challengergui_CHGUI f_TopTop=null;
	bb_challengergui_CHGUI f_TopBottom=null;
	public int m_CheckDown(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<314>";
		if(((f_Over)!=0) && ((bb_input.bb_input_TouchDown(0))!=0) && bb_challengergui.bb_challengergui_CHGUI_MouseBusy==0 || f_Down==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<315>";
			bb_challengergui.bb_challengergui_CHGUI_DownFlag=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<316>";
			bb_challengergui.bb_challengergui_CHGUI_MouseBusy=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<317>";
			f_Down=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<318>";
			if(f_Element.compareTo("Window")!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<318>";
				bb_challengergui.bb_challengergui_CHGUI_DragOver=1;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<319>";
			f_DownTime=bb_app.bb_app_Millisecs()-f_StartDowntime;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<320>";
			f_StartOvertime=bb_app.bb_app_Millisecs();
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<322>";
		if(bb_input.bb_input_TouchDown(0)==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<323>";
			f_Down=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<324>";
			f_DownTime=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<325>";
			f_StartDowntime=bb_app.bb_app_Millisecs();
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<327>";
		if(f_Over==1 && ((bb_input.bb_input_TouchDown(0))!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<328>";
			if(f_Down==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<328>";
				f_Over=0;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<331>";
		if((f_Down)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<332>";
			if(f_Mode==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<332>";
				bb_challengergui.bb_challengergui_CHGUI_Reorder(this);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<333>";
			bb_challengergui_CHGUI t_E=this;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<334>";
			do{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<335>";
				if(t_E.f_Parent!=null){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<336>";
					if((t_E.f_Parent.f_Element.compareTo("Window")==0) && t_E.f_Parent.f_Mode==1){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<337>";
						bb_challengergui.bb_challengergui_CHGUI_Reorder(t_E.f_Parent);
					}
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<339>";
					t_E=t_E.f_Parent;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<341>";
					break;
				}
			}while(!(false));
		}
		bb_std_lang.popErr();
		return 0;
	}
	bb_challengergui_CHGUI f_MenuActive=null;
	bb_challengergui_CHGUI f_MenuOver=null;
	int f_FormatText=1;
	int f_FormatNumber=1;
	int f_FormatSymbol=1;
	int f_FormatSpace=1;
	int f_DKeyMillisecs=0;
	float f_Maximum=.0f;
	int f_Start=0;
	int f_Group=0;
	int f_Moving=0;
	float f_MX=.0f;
	float f_MY=.0f;
	int f_DClickMillisecs=0;
}
interface bb_fontinterface_Font{
}
class bb_bitmapfont_BitmapFont extends Object implements bb_fontinterface_Font{
	bb_bitmapchar_BitMapChar[] f_faceChars=new bb_bitmapchar_BitMapChar[0];
	public int m_GetFontHeight(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<263>";
		if(f_faceChars[32]==null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<263>";
			bb_std_lang.popErr();
			return 0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<264>";
		int t_=(int)(f_faceChars[32].f_drawingMetrics.f_drawingSize.f_y);
		bb_std_lang.popErr();
		return t_;
	}
	boolean f__drawShadow=true;
	public boolean m_DrawShadow(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<66>";
		bb_std_lang.popErr();
		return f__drawShadow;
	}
	public int m_DrawShadow2(boolean t_value){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<70>";
		f__drawShadow=t_value;
		bb_std_lang.popErr();
		return 0;
	}
	bb_bitmapchar_BitMapChar[] f_borderChars=new bb_bitmapchar_BitMapChar[0];
	bb_drawingpoint_DrawingPoint f__kerning=null;
	public bb_drawingpoint_DrawingPoint m_Kerning(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<667>";
		if(f__kerning==null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<667>";
			f__kerning=(new bb_drawingpoint_DrawingPoint()).g_new();
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<668>";
		bb_std_lang.popErr();
		return f__kerning;
	}
	public void m_Kerning2(bb_drawingpoint_DrawingPoint t_value){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<672>";
		f__kerning=t_value;
		bb_std_lang.popErr();
	}
	public float m_GetTxtWidth(String t_text,int t_fromChar,int t_toChar){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<214>";
		float t_twidth=.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<215>";
		float t_MaxWidth=0.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<216>";
		int t_char=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<217>";
		int t_lastchar=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<219>";
		for(int t_i=t_fromChar;t_i<=t_toChar;t_i=t_i+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<220>";
			t_char=(int)t_text.charAt(t_i-1);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<221>";
			if(t_char>=0 && t_char<bb_std_lang.arrayLength(f_faceChars) && t_char!=10 && t_char!=13){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<222>";
				if(f_faceChars[t_char]!=null){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<223>";
					t_lastchar=t_char;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<224>";
					t_twidth=t_twidth+f_faceChars[t_char].f_drawingMetrics.f_drawingWidth+m_Kerning().f_x;
				}
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<226>";
				if(t_char==10){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<227>";
					if(bb_math.bb_math_Abs2(t_MaxWidth)<bb_math.bb_math_Abs2(t_twidth)){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<227>";
						t_MaxWidth=t_twidth-m_Kerning().f_x-f_faceChars[t_lastchar].f_drawingMetrics.f_drawingWidth+f_faceChars[t_lastchar].f_drawingMetrics.f_drawingSize.f_x;
					}
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<228>";
					t_twidth=0.0f;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<229>";
					t_lastchar=t_char;
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<232>";
		if(t_lastchar>=0 && t_lastchar<bb_std_lang.arrayLength(f_faceChars)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<233>";
			if(f_faceChars[t_lastchar]!=null){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<234>";
				t_twidth=t_twidth-f_faceChars[t_lastchar].f_drawingMetrics.f_drawingWidth;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<235>";
				t_twidth=t_twidth+f_faceChars[t_lastchar].f_drawingMetrics.f_drawingSize.f_x;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<238>";
		if(bb_math.bb_math_Abs2(t_MaxWidth)<bb_math.bb_math_Abs2(t_twidth)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<238>";
			t_MaxWidth=t_twidth-m_Kerning().f_x;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<239>";
		bb_std_lang.popErr();
		return t_MaxWidth;
	}
	public float m_GetTxtWidth2(String t_text){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<205>";
		float t_=m_GetTxtWidth(t_text,1,t_text.length());
		bb_std_lang.popErr();
		return t_;
	}
	bb_graphics_Image[] f_packedImages=new bb_graphics_Image[0];
	public int m_DrawCharsText(String t_text,float t_x,float t_y,bb_bitmapchar_BitMapChar[] t_target,int t_align,int t_startPos){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<619>";
		float t_drx=t_x;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<619>";
		float t_dry=t_y;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<620>";
		float t_oldX=t_x;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<621>";
		int t_xOffset=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<624>";
		if(t_align!=1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<625>";
			int t_lineSepPos=t_text.indexOf("\n",t_startPos);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<626>";
			if(t_lineSepPos<0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<626>";
				t_lineSepPos=t_text.length();
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<627>";
			int t_=t_align;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<628>";
			if(t_==2){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<628>";
				t_xOffset=(int)(this.m_GetTxtWidth(t_text,t_startPos,t_lineSepPos)/2.0f);
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<629>";
				if(t_==3){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<629>";
					t_xOffset=(int)(this.m_GetTxtWidth(t_text,t_startPos,t_lineSepPos));
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<633>";
		for(int t_i=t_startPos;t_i<=t_text.length();t_i=t_i+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<634>";
			int t_char=(int)t_text.charAt(t_i-1);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<635>";
			if(t_char>=0 && t_char<=bb_std_lang.arrayLength(t_target)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<636>";
				if(t_char==10){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<637>";
					t_dry+=f_faceChars[32].f_drawingMetrics.f_drawingSize.f_y+m_Kerning().f_y;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<638>";
					this.m_DrawCharsText(t_text,t_oldX,t_dry,t_target,t_align,t_i+1);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<639>";
					bb_std_lang.popErr();
					return 0;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<640>";
					if(t_target[t_char]!=null){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<641>";
						if(t_target[t_char].m_CharImageLoaded()==false){
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<642>";
							t_target[t_char].m_LoadCharImage();
						}
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<644>";
						if(t_target[t_char].f_image!=null){
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<645>";
							bb_graphics.bb_graphics_DrawImage(t_target[t_char].f_image,t_drx-(float)(t_xOffset),t_dry,0);
						}else{
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<646>";
							if(t_target[t_char].f_packedFontIndex>0){
								bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<647>";
								bb_graphics.bb_graphics_DrawImageRect(f_packedImages[t_target[t_char].f_packedFontIndex],(float)(-t_xOffset)+t_drx+t_target[t_char].f_drawingMetrics.f_drawingOffset.f_x,t_dry+t_target[t_char].f_drawingMetrics.f_drawingOffset.f_y,(int)(t_target[t_char].f_packedPosition.f_x),(int)(t_target[t_char].f_packedPosition.f_y),(int)(t_target[t_char].f_packedSize.f_x),(int)(t_target[t_char].f_packedSize.f_y),0);
							}
						}
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<649>";
						t_drx+=f_faceChars[t_char].f_drawingMetrics.f_drawingWidth+m_Kerning().f_x;
					}
				}
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	bb_bitmapchar_BitMapChar[] f_shadowChars=new bb_bitmapchar_BitMapChar[0];
	public int m_DrawCharsText2(String t_text,float t_x,float t_y,int t_mode,int t_align){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<606>";
		if(t_mode==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<607>";
			m_DrawCharsText(t_text,t_x,t_y,f_borderChars,t_align,1);
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<608>";
			if(t_mode==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<609>";
				m_DrawCharsText(t_text,t_x,t_y,f_faceChars,t_align,1);
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<611>";
				m_DrawCharsText(t_text,t_x,t_y,f_shadowChars,t_align,1);
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	boolean f__drawBorder=true;
	public boolean m_DrawBorder(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<79>";
		bb_std_lang.popErr();
		return f__drawBorder;
	}
	public int m_DrawBorder2(boolean t_value){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<83>";
		f__drawBorder=t_value;
		bb_std_lang.popErr();
		return 0;
	}
	public int m_DrawText(String t_text,float t_x,float t_y,int t_align){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<187>";
		if(m_DrawShadow()){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<187>";
			m_DrawCharsText2(t_text,t_x,t_y,2,t_align);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<188>";
		if(m_DrawBorder()){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<188>";
			m_DrawCharsText2(t_text,t_x,t_y,1,t_align);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<189>";
		m_DrawCharsText2(t_text,t_x,t_y,0,t_align);
		bb_std_lang.popErr();
		return 0;
	}
	public int m_DrawText2(String t_text,float t_x,float t_y){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<197>";
		this.m_DrawText(t_text,t_x,t_y,1);
		bb_std_lang.popErr();
		return 0;
	}
	public float m_GetTxtHeight(String t_Text){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<250>";
		int t_count=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<251>";
		for(int t_i=0;t_i<t_Text.length();t_i=t_i+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<252>";
			if((int)t_Text.charAt(t_i)==10){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<253>";
				t_count+=1;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<256>";
		float t_=(float)(t_count)*(f_faceChars[32].f_drawingMetrics.f_drawingSize.f_y+m_Kerning().f_y)+(float)(m_GetFontHeight());
		bb_std_lang.popErr();
		return t_;
	}
	public int m_LoadPacked(String t_info,String t_fontName,boolean t_dynamicLoad){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<542>";
		String t_header=bb_std_lang.slice(t_info,0,t_info.indexOf(",",0));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<544>";
		String t_separator="";
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<545>";
		String t_=t_header;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<546>";
		if(t_.compareTo("P1")==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<547>";
			t_separator=".";
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<548>";
			if(t_.compareTo("P1.01")==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<549>";
				t_separator="_P_";
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<551>";
		t_info=bb_std_lang.slice(t_info,t_info.indexOf(",",0)+1);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<552>";
		f_borderChars=new bb_bitmapchar_BitMapChar[65536];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<553>";
		f_faceChars=new bb_bitmapchar_BitMapChar[65536];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<554>";
		f_shadowChars=new bb_bitmapchar_BitMapChar[65536];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<555>";
		f_packedImages=new bb_graphics_Image[256];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<556>";
		int t_maxPacked=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<557>";
		int t_maxChar=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<559>";
		String t_prefixName=t_fontName;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<560>";
		if(t_prefixName.toLowerCase().endsWith(".txt")){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<560>";
			t_prefixName=bb_std_lang.slice(t_prefixName,0,-4);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<562>";
		String[] t_charList=bb_std_lang.split(t_info,";");
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<563>";
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<563>";
		String[] t_2=t_charList;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<563>";
		int t_3=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<563>";
		while(t_3<bb_std_lang.arrayLength(t_2)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<563>";
			String t_chr=t_2[t_3];
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<563>";
			t_3=t_3+1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<565>";
			String[] t_chrdata=bb_std_lang.split(t_chr,",");
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<566>";
			if(bb_std_lang.arrayLength(t_chrdata)<2){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<566>";
				break;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<567>";
			bb_bitmapchar_BitMapChar t_char=null;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<568>";
			int t_charIndex=Integer.parseInt((t_chrdata[0]).trim());
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<569>";
			if(t_maxChar<t_charIndex){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<569>";
				t_maxChar=t_charIndex;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<571>";
			String t_4=t_chrdata[1];
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<572>";
			if(t_4.compareTo("B")==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<573>";
				f_borderChars[t_charIndex]=(new bb_bitmapchar_BitMapChar()).g_new();
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<574>";
				t_char=f_borderChars[t_charIndex];
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<575>";
				if(t_4.compareTo("F")==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<576>";
					f_faceChars[t_charIndex]=(new bb_bitmapchar_BitMapChar()).g_new();
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<577>";
					t_char=f_faceChars[t_charIndex];
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<578>";
					if(t_4.compareTo("S")==0){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<579>";
						f_shadowChars[t_charIndex]=(new bb_bitmapchar_BitMapChar()).g_new();
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<580>";
						t_char=f_shadowChars[t_charIndex];
					}
				}
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<582>";
			t_char.f_packedFontIndex=Integer.parseInt((t_chrdata[2]).trim());
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<583>";
			if(f_packedImages[t_char.f_packedFontIndex]==null){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<584>";
				f_packedImages[t_char.f_packedFontIndex]=bb_graphics.bb_graphics_LoadImage(t_prefixName+t_separator+String.valueOf(t_char.f_packedFontIndex)+".png",1,bb_graphics_Image.g_DefaultFlags);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<585>";
				if(t_maxPacked<t_char.f_packedFontIndex){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<585>";
					t_maxPacked=t_char.f_packedFontIndex;
				}
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<587>";
			t_char.f_packedPosition.f_x=(float)(Integer.parseInt((t_chrdata[3]).trim()));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<588>";
			t_char.f_packedPosition.f_y=(float)(Integer.parseInt((t_chrdata[4]).trim()));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<589>";
			t_char.f_packedSize.f_x=(float)(Integer.parseInt((t_chrdata[5]).trim()));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<590>";
			t_char.f_packedSize.f_y=(float)(Integer.parseInt((t_chrdata[6]).trim()));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<591>";
			t_char.f_drawingMetrics.f_drawingOffset.f_x=(float)(Integer.parseInt((t_chrdata[8]).trim()));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<592>";
			t_char.f_drawingMetrics.f_drawingOffset.f_y=(float)(Integer.parseInt((t_chrdata[9]).trim()));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<593>";
			t_char.f_drawingMetrics.f_drawingSize.f_x=(float)(Integer.parseInt((t_chrdata[10]).trim()));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<594>";
			t_char.f_drawingMetrics.f_drawingSize.f_y=(float)(Integer.parseInt((t_chrdata[11]).trim()));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<595>";
			t_char.f_drawingMetrics.f_drawingWidth=(float)(Integer.parseInt((t_chrdata[12]).trim()));
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<598>";
		f_borderChars=((bb_bitmapchar_BitMapChar[])bb_std_lang.sliceArray(f_borderChars,0,t_maxChar+1));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<599>";
		f_faceChars=((bb_bitmapchar_BitMapChar[])bb_std_lang.sliceArray(f_faceChars,0,t_maxChar+1));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<600>";
		f_shadowChars=((bb_bitmapchar_BitMapChar[])bb_std_lang.sliceArray(f_shadowChars,0,t_maxChar+1));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<601>";
		f_packedImages=((bb_graphics_Image[])bb_std_lang.sliceArray(f_packedImages,0,t_maxPacked+1));
		bb_std_lang.popErr();
		return 0;
	}
	public int m_LoadFontData(String t_Info,String t_fontName,boolean t_dynamicLoad){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<440>";
		if(t_Info.startsWith("P1")){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<441>";
			m_LoadPacked(t_Info,t_fontName,t_dynamicLoad);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<442>";
			bb_std_lang.popErr();
			return 0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<444>";
		String[] t_tokenStream=bb_std_lang.split(t_Info,",");
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<445>";
		int t_index=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<446>";
		f_borderChars=new bb_bitmapchar_BitMapChar[65536];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<447>";
		f_faceChars=new bb_bitmapchar_BitMapChar[65536];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<448>";
		f_shadowChars=new bb_bitmapchar_BitMapChar[65536];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<450>";
		String t_prefixName=t_fontName;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<451>";
		if(t_prefixName.toLowerCase().endsWith(".txt")){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<451>";
			t_prefixName=bb_std_lang.slice(t_prefixName,0,-4);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<453>";
		int t_char=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<454>";
		while(t_index<bb_std_lang.arrayLength(t_tokenStream)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<456>";
			String t_strChar=t_tokenStream[t_index];
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<457>";
			if(t_strChar.trim().compareTo("")==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<459>";
				t_index+=1;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<460>";
				break;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<462>";
			t_char=Integer.parseInt((t_strChar).trim());
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<464>";
			t_index+=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<466>";
			String t_kind=t_tokenStream[t_index];
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<468>";
			t_index+=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<470>";
			String t_=t_kind;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<471>";
			if(t_.compareTo("{BR")==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<472>";
				t_index+=3;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<473>";
				f_borderChars[t_char]=(new bb_bitmapchar_BitMapChar()).g_new();
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<474>";
				f_borderChars[t_char].f_drawingMetrics.f_drawingOffset.f_x=(float)(Integer.parseInt((t_tokenStream[t_index]).trim()));
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<475>";
				f_borderChars[t_char].f_drawingMetrics.f_drawingOffset.f_y=(float)(Integer.parseInt((t_tokenStream[t_index+1]).trim()));
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<476>";
				f_borderChars[t_char].f_drawingMetrics.f_drawingSize.f_x=(float)(Integer.parseInt((t_tokenStream[t_index+2]).trim()));
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<477>";
				f_borderChars[t_char].f_drawingMetrics.f_drawingSize.f_y=(float)(Integer.parseInt((t_tokenStream[t_index+3]).trim()));
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<478>";
				f_borderChars[t_char].f_drawingMetrics.f_drawingWidth=(float)(Integer.parseInt((t_tokenStream[t_index+4]).trim()));
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<479>";
				if(t_dynamicLoad==false){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<480>";
					f_borderChars[t_char].f_image=bb_graphics.bb_graphics_LoadImage(t_prefixName+"_BORDER_"+String.valueOf(t_char)+".png",1,bb_graphics_Image.g_DefaultFlags);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<481>";
					f_borderChars[t_char].f_image.m_SetHandle(-f_borderChars[t_char].f_drawingMetrics.f_drawingOffset.f_x,-f_borderChars[t_char].f_drawingMetrics.f_drawingOffset.f_y);
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<483>";
					f_borderChars[t_char].m_SetImageResourceName(t_prefixName+"_BORDER_"+String.valueOf(t_char)+".png");
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<485>";
				t_index+=5;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<486>";
				t_index+=1;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<488>";
				if(t_.compareTo("{SH")==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<489>";
					t_index+=3;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<490>";
					f_shadowChars[t_char]=(new bb_bitmapchar_BitMapChar()).g_new();
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<491>";
					f_shadowChars[t_char].f_drawingMetrics.f_drawingOffset.f_x=(float)(Integer.parseInt((t_tokenStream[t_index]).trim()));
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<492>";
					f_shadowChars[t_char].f_drawingMetrics.f_drawingOffset.f_y=(float)(Integer.parseInt((t_tokenStream[t_index+1]).trim()));
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<493>";
					f_shadowChars[t_char].f_drawingMetrics.f_drawingSize.f_x=(float)(Integer.parseInt((t_tokenStream[t_index+2]).trim()));
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<494>";
					f_shadowChars[t_char].f_drawingMetrics.f_drawingSize.f_y=(float)(Integer.parseInt((t_tokenStream[t_index+3]).trim()));
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<495>";
					f_shadowChars[t_char].f_drawingMetrics.f_drawingWidth=(float)(Integer.parseInt((t_tokenStream[t_index+4]).trim()));
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<496>";
					String t_filename=t_prefixName+"_SHADOW_"+String.valueOf(t_char)+".png";
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<497>";
					if(t_dynamicLoad==false){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<498>";
						f_shadowChars[t_char].f_image=bb_graphics.bb_graphics_LoadImage(t_filename,1,bb_graphics_Image.g_DefaultFlags);
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<499>";
						f_shadowChars[t_char].f_image.m_SetHandle(-f_shadowChars[t_char].f_drawingMetrics.f_drawingOffset.f_x,-f_shadowChars[t_char].f_drawingMetrics.f_drawingOffset.f_y);
					}else{
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<501>";
						f_shadowChars[t_char].m_SetImageResourceName(t_filename);
					}
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<508>";
					t_index+=5;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<509>";
					t_index+=1;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<511>";
					if(t_.compareTo("{FC")==0){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<512>";
						t_index+=3;
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<513>";
						f_faceChars[t_char]=(new bb_bitmapchar_BitMapChar()).g_new();
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<514>";
						f_faceChars[t_char].f_drawingMetrics.f_drawingOffset.f_x=(float)(Integer.parseInt((t_tokenStream[t_index]).trim()));
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<515>";
						f_faceChars[t_char].f_drawingMetrics.f_drawingOffset.f_y=(float)(Integer.parseInt((t_tokenStream[t_index+1]).trim()));
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<516>";
						f_faceChars[t_char].f_drawingMetrics.f_drawingSize.f_x=(float)(Integer.parseInt((t_tokenStream[t_index+2]).trim()));
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<517>";
						f_faceChars[t_char].f_drawingMetrics.f_drawingSize.f_y=(float)(Integer.parseInt((t_tokenStream[t_index+3]).trim()));
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<518>";
						f_faceChars[t_char].f_drawingMetrics.f_drawingWidth=(float)(Integer.parseInt((t_tokenStream[t_index+4]).trim()));
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<519>";
						if(t_dynamicLoad==false){
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<520>";
							f_faceChars[t_char].f_image=bb_graphics.bb_graphics_LoadImage(t_prefixName+"_"+String.valueOf(t_char)+".png",1,bb_graphics_Image.g_DefaultFlags);
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<521>";
							f_faceChars[t_char].f_image.m_SetHandle(-f_faceChars[t_char].f_drawingMetrics.f_drawingOffset.f_x,-f_faceChars[t_char].f_drawingMetrics.f_drawingOffset.f_y);
						}else{
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<523>";
							f_faceChars[t_char].m_SetImageResourceName(t_prefixName+"_"+String.valueOf(t_char)+".png");
						}
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<525>";
						t_index+=5;
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<526>";
						t_index+=1;
					}else{
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<529>";
						bb_std_lang.print("Error loading font! Char = "+String.valueOf(t_char));
					}
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<533>";
		f_borderChars=((bb_bitmapchar_BitMapChar[])bb_std_lang.sliceArray(f_borderChars,0,t_char+1));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<534>";
		f_faceChars=((bb_bitmapchar_BitMapChar[])bb_std_lang.sliceArray(f_faceChars,0,t_char+1));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<535>";
		f_shadowChars=((bb_bitmapchar_BitMapChar[])bb_std_lang.sliceArray(f_shadowChars,0,t_char+1));
		bb_std_lang.popErr();
		return 0;
	}
	public bb_bitmapfont_BitmapFont g_new(String t_fontDescriptionFilePath,boolean t_dynamicLoad){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<45>";
		String t_text=bb_app.bb_app_LoadString(t_fontDescriptionFilePath);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<46>";
		if(t_text.compareTo("")==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<46>";
			bb_std_lang.print("FONT "+t_fontDescriptionFilePath+" WAS NOT FOUND!!!");
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<47>";
		m_LoadFontData(t_text,t_fontDescriptionFilePath,t_dynamicLoad);
		bb_std_lang.popErr();
		return this;
	}
	public bb_bitmapfont_BitmapFont g_new2(String t_fontDescriptionFilePath){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<55>";
		String t_text=bb_app.bb_app_LoadString(t_fontDescriptionFilePath);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<56>";
		if(t_text.compareTo("")==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<56>";
			bb_std_lang.print("FONT "+t_fontDescriptionFilePath+" WAS NOT FOUND!!!");
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<57>";
		m_LoadFontData(t_text,t_fontDescriptionFilePath,true);
		bb_std_lang.popErr();
		return this;
	}
	public bb_bitmapfont_BitmapFont g_new3(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<28>";
		bb_std_lang.popErr();
		return this;
	}
	static public bb_bitmapfont_BitmapFont g_Load(String t_fontName,boolean t_dynamicLoad){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<35>";
		bb_bitmapfont_BitmapFont t_font=(new bb_bitmapfont_BitmapFont()).g_new(t_fontName,t_dynamicLoad);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<36>";
		bb_std_lang.popErr();
		return t_font;
	}
}
class bb_bitmapchar_BitMapChar extends Object{
	bb_bitmapcharmetrics_BitMapCharMetrics f_drawingMetrics=(new bb_bitmapcharmetrics_BitMapCharMetrics()).g_new();
	bb_graphics_Image f_image=null;
	String f_imageResourceName="";
	public boolean m_CharImageLoaded(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<42>";
		if(f_image==null && (f_imageResourceName.compareTo("")!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<42>";
			bb_std_lang.popErr();
			return false;
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<42>";
			bb_std_lang.popErr();
			return true;
		}
	}
	String f_imageResourceNameBackup="";
	public int m_LoadCharImage(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<30>";
		if(m_CharImageLoaded()==false){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<31>";
			f_image=bb_graphics.bb_graphics_LoadImage(f_imageResourceName,1,bb_graphics_Image.g_DefaultFlags);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<32>";
			f_image.m_SetHandle(-this.f_drawingMetrics.f_drawingOffset.f_x,-this.f_drawingMetrics.f_drawingOffset.f_y);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<33>";
			f_imageResourceNameBackup=f_imageResourceName;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<34>";
			f_imageResourceName="";
		}
		bb_std_lang.popErr();
		return 0;
	}
	int f_packedFontIndex=0;
	bb_drawingpoint_DrawingPoint f_packedPosition=(new bb_drawingpoint_DrawingPoint()).g_new();
	bb_drawingpoint_DrawingPoint f_packedSize=(new bb_drawingpoint_DrawingPoint()).g_new();
	public bb_bitmapchar_BitMapChar g_new(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<15>";
		bb_std_lang.popErr();
		return this;
	}
	public int m_SetImageResourceName(String t_value){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<46>";
		f_imageResourceName=t_value;
		bb_std_lang.popErr();
		return 0;
	}
}
class bb_bitmapcharmetrics_BitMapCharMetrics extends Object{
	public bb_bitmapcharmetrics_BitMapCharMetrics g_new(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapcharmetrics.monkey<12>";
		bb_std_lang.popErr();
		return this;
	}
	bb_drawingpoint_DrawingPoint f_drawingSize=(new bb_drawingpoint_DrawingPoint()).g_new();
	float f_drawingWidth=.0f;
	bb_drawingpoint_DrawingPoint f_drawingOffset=(new bb_drawingpoint_DrawingPoint()).g_new();
}
class bb_drawingpoint_DrawingPoint extends Object{
	public bb_drawingpoint_DrawingPoint g_new(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/fontmachine/drawingpoint.monkey<8>";
		bb_std_lang.popErr();
		return this;
	}
	float f_y=.0f;
	float f_x=.0f;
}
abstract class bb_edrawmode_eDrawMode extends Object{
}
abstract class bb_edrawalign_eDrawAlign extends Object{
}
class bb_asyncevent{
}
class bb_asyncstream{
}
class bb_asynctcpconnector{
}
class bb_asynctcpstream{
}
class bb_brl{
}
class bb_databuffer{
}
class bb_datastream{
}
class bb_filestream{
}
class bb_pool{
}
class bb_ringbuffer{
}
class bb_stream{
}
class bb_tcpstream{
}
class bb_thread{
}
class bb_challengergui{
	static int bb_challengergui_CHGUI_MobileMode;
	static public bb_challengergui_CHGUI bb_challengergui_CreateDropdownItem(String t_Text,bb_challengergui_CHGUI t_Dropdown,int t_Value){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<515>";
		bb_challengergui_CHGUI t_N=(new bb_challengergui_CHGUI()).g_new();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<516>";
		t_N.f_Parent=t_Dropdown;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<518>";
		t_N.f_Value=(float)(t_Value);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<519>";
		t_N.f_Text=t_Text;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<520>";
		t_N.f_Element="DropdownItem";
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<522>";
		t_N.f_Parent.f_DropdownItems=(bb_challengergui_CHGUI[])bb_std_lang.resizeArray(t_N.f_Parent.f_DropdownItems,bb_std_lang.arrayLength(t_N.f_Parent.f_DropdownItems)+1);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<523>";
		t_N.f_Parent.f_DropdownItems[bb_std_lang.arrayLength(t_N.f_Parent.f_DropdownItems)-1]=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<524>";
		bb_std_lang.popErr();
		return t_N;
	}
	static bb_challengergui_CHGUI[] bb_challengergui_CHGUI_BottomList;
	static bb_challengergui_CHGUI bb_challengergui_CHGUI_Canvas;
	static public int bb_challengergui_CHGUI_RealVisible(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3921>";
		bb_challengergui_CHGUI t_E=null;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3922>";
		t_E=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3923>";
		int t_V=t_N.f_Visible;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3924>";
		if(t_V==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3924>";
			bb_std_lang.popErr();
			return t_V;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3925>";
		do{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3926>";
			if(t_E.f_Parent!=null){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3927>";
				t_V=t_E.f_Parent.f_Visible;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3928>";
				if(t_V==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3928>";
					break;
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3929>";
				t_E=t_E.f_Parent;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3931>";
				break;
			}
		}while(!(false));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3934>";
		if(bb_challengergui.bb_challengergui_CHGUI_Canvas.f_Visible==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3934>";
			t_V=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3935>";
		bb_std_lang.popErr();
		return t_V;
	}
	static public int bb_challengergui_CHGUI_RealMinimised(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3962>";
		bb_challengergui_CHGUI t_E=null;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3963>";
		t_E=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3964>";
		int t_M=t_E.f_Minimised;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3965>";
		if(t_M==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3965>";
			bb_std_lang.popErr();
			return t_M;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3966>";
		do{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3967>";
			if(t_E.f_Parent!=null){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3968>";
				t_M=t_E.f_Parent.f_Minimised;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3969>";
				if(t_M==1){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3969>";
					break;
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3970>";
				t_E=t_E.f_Parent;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3972>";
				break;
			}
		}while(!(false));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3975>";
		bb_std_lang.popErr();
		return t_M;
	}
	static float bb_challengergui_CHGUI_OffsetX;
	static public int bb_challengergui_CHGUI_RealX(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3881>";
		bb_challengergui_CHGUI t_E=null;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3882>";
		t_E=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3883>";
		int t_X=(int)(t_N.f_X);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3884>";
		do{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3885>";
			if(t_E.f_Parent!=null){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3886>";
				if(t_E.f_Parent.f_Element.compareTo("Tab")!=0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3887>";
					t_X=(int)((float)(t_X)+t_E.f_Parent.f_X);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3888>";
					t_E=t_E.f_Parent;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3890>";
					t_E=t_E.f_Parent;
				}
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3893>";
				break;
			}
		}while(!(false));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3896>";
		t_X=(int)((float)(t_X)+bb_challengergui.bb_challengergui_CHGUI_OffsetX);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3897>";
		bb_std_lang.popErr();
		return t_X;
	}
	static float bb_challengergui_CHGUI_OffsetY;
	static public int bb_challengergui_CHGUI_RealY(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3901>";
		bb_challengergui_CHGUI t_E=null;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3902>";
		t_E=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3903>";
		int t_Y=(int)(t_N.f_Y);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3904>";
		do{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3905>";
			if(t_E.f_Parent!=null){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3906>";
				if(t_E.f_Parent.f_Element.compareTo("Tab")!=0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3907>";
					t_Y=(int)((float)(t_Y)+t_E.f_Parent.f_Y);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3908>";
					t_E=t_E.f_Parent;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3910>";
					t_E=t_E.f_Parent;
				}
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3913>";
				break;
			}
		}while(!(false));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3916>";
		t_Y=(int)((float)(t_Y)+bb_challengergui.bb_challengergui_CHGUI_OffsetY);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3917>";
		bb_std_lang.popErr();
		return t_Y;
	}
	static float bb_challengergui_CHGUI_TitleHeight;
	static bb_challengergui_CHGUI bb_challengergui_CHGUI_LockedWIndow;
	static public int bb_challengergui_CHGUI_RealActive(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3939>";
		bb_challengergui_CHGUI t_E=null;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3940>";
		t_E=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3941>";
		int t_A=t_N.f_Active;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3942>";
		if(t_A==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3942>";
			bb_std_lang.popErr();
			return t_A;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3943>";
		do{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3944>";
			if(t_E==bb_challengergui.bb_challengergui_CHGUI_LockedWIndow){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3944>";
				bb_std_lang.popErr();
				return 1;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3945>";
			if(t_E.f_Parent!=null){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3946>";
				if(t_E.f_Parent==bb_challengergui.bb_challengergui_CHGUI_LockedWIndow){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3947>";
					bb_std_lang.popErr();
					return 1;
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3949>";
				t_A=t_E.f_Parent.f_Active;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3950>";
				if(t_A==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3950>";
					break;
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3952>";
				t_E=t_E.f_Parent;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3954>";
				break;
			}
		}while(!(false));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3958>";
		bb_std_lang.popErr();
		return t_A;
	}
	static int bb_challengergui_CHGUI_Shadow;
	static bb_graphics_Image bb_challengergui_CHGUI_ShadowImg;
	static bb_graphics_Image bb_challengergui_CHGUI_Style;
	static bb_bitmapfont_BitmapFont bb_challengergui_CHGUI_TitleFont;
	static public float bb_challengergui_CHGUI_TextHeight(bb_bitmapfont_BitmapFont t_Fnt,String t_Text){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3871>";
		String[] t_Split=bb_std_lang.split(t_Text,"\n");
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3872>";
		float t_H=(float)(t_Fnt.m_GetFontHeight());
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3873>";
		float t_Height=0.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3874>";
		for(int t_N=0;t_N<=bb_std_lang.arrayLength(t_Split)-1;t_N=t_N+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3875>";
			t_Height=t_Height+t_H;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3877>";
		bb_std_lang.popErr();
		return t_Height;
	}
	static bb_bitmapfont_BitmapFont bb_challengergui_CHGUI_Font;
	static public int bb_challengergui_CHGUI_DrawWindow(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2228>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2229>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2230>";
		float t_W=t_N.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2231>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2232>";
		float t_TH=bb_challengergui.bb_challengergui_CHGUI_TitleHeight;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2233>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2234>";
		if(bb_challengergui.bb_challengergui_CHGUI_LockedWIndow==t_N){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2234>";
			t_Active=1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2236>";
		if(t_N!=bb_challengergui.bb_challengergui_CHGUI_Canvas){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2238>";
			if(((t_N.f_Shadow)!=0) && ((bb_challengergui.bb_challengergui_CHGUI_Shadow)!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2239>";
				if((t_N.f_Minimised)!=0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2241>";
					bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y-10.0f,0,0,20,20,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2243>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+10.0f,t_Y-10.0f,20,0,10,10,0.0f,(t_W-20.0f)/10.0f,1.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2245>";
					bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W-10.0f,t_Y-10.0f,30,0,20,20,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2247>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y+10.0f,0,20,10,10,0.0f,1.0f,(t_TH-20.0f)/10.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2249>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y+10.0f,40,20,10,10,0.0f,1.0f,(t_TH-20.0f)/10.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2251>";
					bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y+t_TH-10.0f,0,30,20,20,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2253>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+10.0f,t_Y+t_TH,20,40,10,10,0.0f,(t_W-20.0f)/10.0f,1.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2255>";
					bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W-10.0f,t_Y+t_TH-10.0f,30,30,20,20,0);
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2258>";
					bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y-10.0f,0,0,20,20,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2260>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+10.0f,t_Y-10.0f,20,0,10,10,0.0f,(t_W-20.0f)/10.0f,1.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2262>";
					bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W-10.0f,t_Y-10.0f,30,0,20,20,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2264>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y+10.0f,0,20,10,10,0.0f,1.0f,(t_H-20.0f)/10.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2266>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y+10.0f,40,20,10,10,0.0f,1.0f,(t_H-20.0f)/10.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2268>";
					bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y+t_H-10.0f,0,30,20,20,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2270>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+10.0f,t_Y+t_H,20,40,10,10,0.0f,(t_W-20.0f)/10.0f,1.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2272>";
					bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W-10.0f,t_Y+t_H-10.0f,30,30,20,20,0);
				}
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2276>";
			float t_XOf=10.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2277>";
			float t_YOf=10.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2278>";
			if(bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N)==0 && bb_challengergui.bb_challengergui_CHGUI_LockedWIndow!=t_N){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2278>";
				t_YOf=t_YOf+30.0f;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2281>";
			if(t_N.f_Text.compareTo("")!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2283>";
				bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,(int)(t_XOf),(int)(t_YOf),10,10,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2285>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y,(int)(t_XOf+10.0f),(int)(t_YOf),50,10,0.0f,(t_W-20.0f)/50.0f,1.0f,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2287>";
				bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y,(int)(t_XOf+60.0f),(int)(t_YOf),10,10,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2289>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+10.0f,(int)(t_XOf),(int)(t_YOf+10.0f),10,10,0.0f,1.0f,(t_TH-20.0f)/10.0f,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2291>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+10.0f,(int)(t_XOf+10.0f),(int)(t_YOf+10.0f),50,10,0.0f,(t_W-20.0f)/50.0f,(t_TH-20.0f)/10.0f,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2293>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+10.0f,(int)(t_XOf+60.0f),(int)(t_YOf+10.0f),10,10,0.0f,1.0f,(t_TH-20.0f)/10.0f,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2295>";
				bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_TH-10.0f,(int)(t_XOf),(int)(t_YOf+20.0f),10,10,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2297>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+t_TH-10.0f,(int)(t_XOf+10.0f),(int)(t_YOf+20.0f),50,10,0.0f,(t_W-20.0f)/50.0f,1.0f,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2299>";
				bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+t_TH-10.0f,(int)(t_XOf+60.0f),(int)(t_YOf+20.0f),10,10,0);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2302>";
			if(t_N.f_Minimised==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2303>";
				if(t_N.f_Text.compareTo("")==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2305>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,10,70,10,10,0.0f,1.0f,1.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2307>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y,20,70,50,10,0.0f,(t_W-20.0f)/50.0f,1.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2309>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y,70,70,10,10,0.0f,1.0f,1.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2311>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+10.0f,10,80,10,40,0.0f,1.0f,(t_H-20.0f)/40.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2313>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+10.0f,70,80,10,40,0.0f,1.0f,(t_H-20.0f)/40.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2315>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+10.0f,20,80,50,40,0.0f,(t_W-20.0f)/50.0f,(t_H-20.0f)/40.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2317>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0f,10,120,10,10,0.0f,1.0f,1.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2319>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+t_H-10.0f,20,120,50,10,0.0f,(t_W-20.0f)/50.0f,1.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2321>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+t_H-10.0f,70,120,10,10,0.0f,1.0f,1.0f,0);
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2324>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_TH,10,80,10,40,0.0f,1.0f,(t_H-t_TH-10.0f)/40.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2326>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+t_TH,70,80,10,40,0.0f,1.0f,(t_H-t_TH-10.0f)/40.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2328>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+t_TH,20,80,50,40,0.0f,(t_W-20.0f)/50.0f,(t_H-t_TH-10.0f)/40.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2330>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0f,10,120,10,10,0.0f,1.0f,1.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2332>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+t_H-10.0f,20,120,50,10,0.0f,(t_W-20.0f)/50.0f,1.0f,0);
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2334>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+t_H-10.0f,70,120,10,10,0.0f,1.0f,1.0f,0);
				}
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2339>";
			if(t_N.f_Close==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2340>";
				if(((t_N.f_CloseOver)!=0) && t_N.f_CloseDown==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2341>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-t_TH/2.5f-10.0f,t_Y+(t_TH-t_TH/2.5f)/2.0f,105,10,15,15,0.0f,t_TH/2.5f/15.0f,t_TH/2.5f/15.0f,0);
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2342>";
					if((t_N.f_CloseDown)!=0){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2343>";
						bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-t_TH/2.5f-10.0f,t_Y+(t_TH-t_TH/2.5f)/2.0f,120,10,15,15,0.0f,t_TH/2.5f/15.0f,t_TH/2.5f/15.0f,0);
					}else{
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2344>";
						if((t_Active)!=0){
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2345>";
							bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-t_TH/2.5f-10.0f,t_Y+(t_TH-t_TH/2.5f)/2.0f,90,10,15,15,0.0f,t_TH/2.5f/15.0f,t_TH/2.5f/15.0f,0);
						}else{
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2347>";
							bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-t_TH/2.5f-10.0f,t_Y+(t_TH-t_TH/2.5f)/2.0f,135,10,15,15,0.0f,t_TH/2.5f/15.0f,t_TH/2.5f/15.0f,0);
						}
					}
				}
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2352>";
			if(t_N.f_Minimise==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2353>";
				if(((t_N.f_MinimiseOver)!=0) && t_N.f_MinimiseDown==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2354>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-(t_TH/2.5f+t_TH/2.5f)-t_TH/1.5f,t_Y+(t_TH-t_TH/2.5f)/2.0f,105,25,15,15,0.0f,t_TH/2.5f/15.0f,t_TH/2.5f/15.0f,0);
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2355>";
					if((t_N.f_MinimiseDown)!=0){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2356>";
						bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-(t_TH/2.5f+t_TH/2.5f)-t_TH/1.5f,t_Y+(t_TH-t_TH/2.5f)/2.0f,120,25,15,15,0.0f,t_TH/2.5f/15.0f,t_TH/2.5f/15.0f,0);
					}else{
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2357>";
						if((t_Active)!=0){
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2358>";
							bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-(t_TH/2.5f+t_TH/2.5f)-t_TH/1.5f,t_Y+(t_TH-t_TH/2.5f)/2.0f,90,25,15,15,0.0f,t_TH/2.5f/15.0f,t_TH/2.5f/15.0f,0);
						}else{
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2360>";
							bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-(t_TH/2.5f+t_TH/2.5f)-t_TH/1.5f,t_Y+(t_TH-t_TH/2.5f)/2.0f,135,25,15,15,0.0f,t_TH/2.5f/15.0f,t_TH/2.5f/15.0f,0);
						}
					}
				}
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2369>";
			float t_XOff=(t_TH-t_TH/2.0f)/2.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2370>";
			float t_YOff=t_TH-bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_TitleFont,t_N.f_Text);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2371>";
			bb_graphics.bb_graphics_SetAlpha(0.25f);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2372>";
			if((t_Active)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2372>";
				bb_graphics.bb_graphics_SetAlpha(1.0f);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2373>";
			bb_challengergui.bb_challengergui_CHGUI_TitleFont.m_DrawText2(t_N.f_Text,t_X+t_XOff,t_Y+t_YOff/2.0f);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2374>";
			bb_graphics.bb_graphics_SetAlpha(1.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2377>";
		if((t_N.f_HasMenu)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2378>";
			if(t_N!=bb_challengergui.bb_challengergui_CHGUI_Canvas){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2379>";
				if((bb_challengergui.bb_challengergui_CHGUI_Shadow)!=0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2379>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X,t_Y+t_TH+(float)(t_N.f_MenuHeight),20,40,10,10,0.0f,t_W/10.0f,1.0f,0);
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2380>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+1.0f,t_Y+bb_challengergui.bb_challengergui_CHGUI_TitleHeight,100,90,40,10,0.0f,(t_W-2.0f)/40.0f,(10.0f+bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text)-10.0f)/10.0f,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2381>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+1.0f,t_Y+bb_challengergui.bb_challengergui_CHGUI_TitleHeight+(float)(t_N.f_MenuHeight-10),100,100,40,10,0.0f,(t_W-2.0f)/40.0f,1.0f,0);
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2383>";
				if((bb_challengergui.bb_challengergui_CHGUI_Shadow)!=0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2383>";
					bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X,t_Y+(float)(t_N.f_MenuHeight),20,40,10,10,0.0f,t_W/10.0f,1.0f,0);
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2384>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+1.0f,t_Y,100,90,40,10,0.0f,(t_W-2.0f)/40.0f,(10.0f+bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text)-10.0f)/10.0f,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2385>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+1.0f,t_Y+(float)(t_N.f_MenuHeight-10),100,100,40,10,0.0f,(t_W-2.0f)/40.0f,1.0f,0);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2390>";
		if(((t_N.f_Tabbed)!=0) && t_N.f_Minimised==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2391>";
			int t_YY=(int)(t_Y+(float)(t_N.f_MenuHeight));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2392>";
			if(t_N.f_Text.compareTo("")!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2392>";
				t_YY=(int)((float)(t_YY)+bb_challengergui.bb_challengergui_CHGUI_TitleHeight);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2393>";
			float t_Height=(float)(t_N.f_TabHeight+5);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2396>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+1.0f,(float)(t_YY),10,140,10,10,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2398>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-11.0f,(float)(t_YY),60,140,10,10,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2400>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+11.0f,(float)(t_YY),20,140,40,10,0.0f,(t_W-22.0f)/40.0f,1.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2402>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+1.0f,(float)(t_YY+10),10,150,10,10,0.0f,1.0f,(t_Height-10.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2404>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-11.0f,(float)(t_YY+10),60,150,10,10,0.0f,1.0f,(t_Height-10.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2406>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+11.0f,(float)(t_YY+10),20,150,40,10,0.0f,(t_N.f_W-22.0f)/40.0f,(t_Height-10.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2408>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+1.0f,(float)(t_YY)+t_Height-10.0f,10,160,10,10,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2410>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-11.0f,(float)(t_YY)+t_Height-10.0f,60,160,10,10,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2412>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+11.0f,(float)(t_YY)+t_Height-10.0f,20,160,40,10,0.0f,(t_W-22.0f)/40.0f,1.0f,0);
		}
		bb_std_lang.popErr();
		return 0;
	}
	static bb_challengergui_CHGUI[] bb_challengergui_CHGUI_KeyboardButtons;
	static int bb_challengergui_CHGUI_ShiftHold;
	static public int bb_challengergui_CHGUI_DrawButton(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2418>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2419>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2420>";
		float t_W=t_N.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2421>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2422>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2423>";
		int t_State=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2424>";
		if((t_N.f_Over)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2424>";
			t_State=40;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2425>";
		if((t_N.f_Down)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2425>";
			t_State=80;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2426>";
		if(t_N==bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104] && ((bb_challengergui.bb_challengergui_CHGUI_ShiftHold)!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2426>";
			t_State=80;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2427>";
		if(t_Active==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2427>";
			t_State=120;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2432>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,160,10+t_State,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2434>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y,170,10+t_State,40,10,0.0f,(t_W-20.0f)/40.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2436>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y,210,10+t_State,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2438>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+10.0f,160,20+t_State,10,20,0.0f,1.0f,(t_H-20.0f)/20.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2440>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+10.0f,210,20+t_State,10,20,0.0f,1.0f,(t_H-20.0f)/20.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2442>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+10.0f,170,20+t_State,40,20,0.0f,(t_W-20.0f)/40.0f,(t_H-20.0f)/20.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2444>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0f,160,40+t_State,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2446>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+t_H-10.0f,170,40+t_State,40,10,0.0f,(t_W-20.0f)/40.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2448>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+t_H-10.0f,210,40+t_State,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2451>";
		float t_XOff=(t_W-bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text))/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2452>";
		float t_YOff=(t_H-bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text))/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2453>";
		bb_graphics.bb_graphics_SetAlpha(0.25f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2454>";
		if((t_Active)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2454>";
			bb_graphics.bb_graphics_SetAlpha(1.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2455>";
		bb_challengergui.bb_challengergui_CHGUI_Font.m_DrawText2(t_N.f_Text,t_X+t_XOff,t_Y+t_YOff);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2456>";
		bb_graphics.bb_graphics_SetAlpha(1.0f);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawImageButton(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2479>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2480>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2481>";
		float t_W=t_N.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2482>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2483>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2484>";
		int t_State=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2485>";
		if((t_N.f_Over)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2485>";
			t_State=(int)(t_W);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2486>";
		if((t_N.f_Down)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2486>";
			t_State=(int)(t_W*2.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2487>";
		if(t_Active==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2487>";
			t_State=(int)(t_W*3.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2489>";
		bb_graphics.bb_graphics_DrawImageRect(t_N.f_Img,t_X,t_Y,t_State,0,(int)(t_W),(int)(t_H),0);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawTickbox(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2495>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2496>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2497>";
		float t_W=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2498>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2499>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2500>";
		int t_OffX=230;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2501>";
		int t_OffY=10;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2502>";
		int t_OffW=20;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2503>";
		int t_OffH=20;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2504>";
		if((t_N.f_Over)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2504>";
			t_OffY=30;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2505>";
		if((t_N.f_Down)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2505>";
			t_OffY=50;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2506>";
		if(t_Active==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2506>";
			t_OffY=70;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2507>";
		if(t_N.f_Value>0.0f){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2507>";
			t_OffX=250;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2510>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,t_OffW,t_OffH,0.0f,t_W/(float)(t_OffW),t_H/(float)(t_OffH),0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2512>";
		float t_XOff=t_W/4.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2513>";
		float t_YOff=(t_H-bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text))/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2514>";
		bb_graphics.bb_graphics_SetAlpha(0.25f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2515>";
		if((t_Active)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2515>";
			bb_graphics.bb_graphics_SetAlpha(1.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2516>";
		bb_challengergui.bb_challengergui_CHGUI_Font.m_DrawText2(t_N.f_Text,t_X+t_W+t_XOff,t_Y+t_YOff);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2517>";
		bb_graphics.bb_graphics_SetAlpha(1.0f);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawRadiobox(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2523>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2524>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2525>";
		float t_W=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2526>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2527>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2528>";
		int t_OffX=230;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2529>";
		int t_OffY=100;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2530>";
		int t_OffW=20;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2531>";
		int t_OffH=20;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2532>";
		if((t_N.f_Over)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2532>";
			t_OffY=120;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2533>";
		if((t_N.f_Down)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2533>";
			t_OffY=140;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2534>";
		if(t_Active==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2534>";
			t_OffY=160;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2535>";
		if(t_N.f_Value>0.0f){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2535>";
			t_OffX=250;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2538>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,t_OffW,t_OffH,0.0f,t_W/(float)(t_OffW),t_H/(float)(t_OffH),0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2540>";
		float t_XOff=t_W/4.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2541>";
		float t_YOff=(t_H-bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text))/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2542>";
		bb_graphics.bb_graphics_SetAlpha(0.25f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2543>";
		if((t_Active)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2543>";
			bb_graphics.bb_graphics_SetAlpha(1.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2544>";
		bb_challengergui.bb_challengergui_CHGUI_Font.m_DrawText2(t_N.f_Text,t_X+t_W+t_XOff,t_Y+t_YOff);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2545>";
		bb_graphics.bb_graphics_SetAlpha(1.0f);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawListbox(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3074>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3075>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3076>";
		float t_W=t_N.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3077>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3078>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3079>";
		t_N.f_ListboxSlider.f_X=t_N.f_X+t_W-17.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3080>";
		t_N.f_ListboxSlider.f_Y=t_N.f_ListboxSlider.f_Parent.f_Y+t_N.f_Y-t_N.f_Parent.f_Y;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3081>";
		t_N.f_ListboxSlider.f_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3082>";
		int t_OffX=90;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3083>";
		int t_OffY=80;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3087>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3089>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y,t_OffX+10,t_OffY,40,10,0.0f,(t_W-20.0f)/40.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3091>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y,t_OffX+50,t_OffY,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3093>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+10.0f,t_OffX,t_OffY+10,10,10,0.0f,1.0f,(t_H-20.0f)/10.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3095>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+10.0f,t_OffX+50,t_OffY+10,10,10,0.0f,1.0f,(t_H-20.0f)/10.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3097>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+10.0f,t_OffX+10,t_OffY+10,40,10,0.0f,(t_W-20.0f)/40.0f,(t_H-20.0f)/10.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3099>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0f,t_OffX,t_OffY+20,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3101>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+t_H-10.0f,t_OffX+10,t_OffY+20,40,10,0.0f,(t_W-20.0f)/40.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3103>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+t_H-10.0f,t_OffX+50,t_OffY+20,10,10,0);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawListboxItem(bb_challengergui_CHGUI t_N,int t_C){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3109>";
		t_N.f_X=0.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3110>";
		t_N.f_Y=(float)(t_C*t_N.f_Parent.f_ListHeight);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3111>";
		t_N.f_W=t_N.f_Parent.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3112>";
		t_N.f_H=(float)(t_N.f_Parent.f_ListHeight);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3114>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3115>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3116>";
		float t_W=t_N.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3117>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3118>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3120>";
		if(t_N.f_Over==1 || ((t_N.f_Down)!=0) || t_N.f_Parent.f_SelectedListboxItem==t_N){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3122>";
			int t_OffX=90;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3123>";
			int t_OffY=110;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3124>";
			if(((t_N.f_Down)!=0) || t_N.f_Parent.f_SelectedListboxItem==t_N){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3125>";
				t_OffX=90;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3126>";
				t_OffY=140;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3130>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3132>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y,t_OffX+10,t_OffY,40,10,0.0f,(t_W-20.0f)/40.0f,1.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3134>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y,t_OffX+50,t_OffY,10,10,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3136>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+10.0f,t_OffX,t_OffY+10,10,10,0.0f,1.0f,(t_H-20.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3138>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+10.0f,t_OffX+50,t_OffY+10,10,10,0.0f,1.0f,(t_H-20.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3140>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+10.0f,t_OffX+10,t_OffY+10,40,10,0.0f,(t_W-20.0f)/40.0f,(t_H-20.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3142>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0f,t_OffX,t_OffY+20,10,10,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3144>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+t_H-10.0f,t_OffX+10,t_OffY+20,40,10,0.0f,(t_W-20.0f)/40.0f,1.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3146>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+t_H-10.0f,t_OffX+50,t_OffY+20,10,10,0);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3151>";
		float t_YOff=(t_H-bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text))/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3152>";
		bb_graphics.bb_graphics_SetAlpha(0.25f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3153>";
		if((t_Active)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3153>";
			bb_graphics.bb_graphics_SetAlpha(1.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3154>";
		bb_challengergui.bb_challengergui_CHGUI_Font.m_DrawText2(t_N.f_Text,t_X+10.0f,t_Y+t_YOff);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3155>";
		bb_graphics.bb_graphics_SetAlpha(1.0f);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawHSlider(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2857>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2858>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2859>";
		float t_W=t_N.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2860>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2861>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2863>";
		int t_OffX=460;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2864>";
		int t_OffY=10;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2866>";
		if(t_Active==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2866>";
			t_OffY=70;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2869>";
		if(((t_N.f_MinusOver)!=0) && t_N.f_MinusDown==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2870>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY+20,20,20,0.0f,t_H/20.0f,t_H/20.0f,0);
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2871>";
			if((t_N.f_MinusDown)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2872>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY+40,20,20,0.0f,t_H/20.0f,t_H/20.0f,0);
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2874>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,20,20,0.0f,t_H/20.0f,t_H/20.0f,0);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2878>";
		if(((t_N.f_PlusOver)!=0) && t_N.f_PlusDown==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2879>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-t_H,t_Y,t_OffX+60,t_OffY+20,20,20,0.0f,t_H/20.0f,t_H/20.0f,0);
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2880>";
			if((t_N.f_PlusDown)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2881>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-t_H,t_Y,t_OffX+60,t_OffY+40,20,20,0.0f,t_H/20.0f,t_H/20.0f,0);
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2883>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-t_H,t_Y,t_OffX+60,t_OffY,20,20,0.0f,t_H/20.0f,t_H/20.0f,0);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2888>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_H,t_Y,t_OffX+20,t_OffY,40,20,0.0f,(t_W-t_H-t_H)/40.0f,t_H/20.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2891>";
		int t_XOF=475;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2892>";
		int t_YOF=100;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2893>";
		if(((t_N.f_SliderOver)!=0) && t_N.f_SliderDown==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2893>";
			t_YOF=t_YOF+20;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2894>";
		if((t_N.f_SliderDown)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2894>";
			t_YOF=t_YOF+40;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2895>";
		if(t_Active==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2895>";
			t_YOF=t_YOF+60;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2896>";
		float t_XPOS=t_X+t_H-5.0f+(t_N.f_Value-t_N.f_Minimum)*t_N.f_Stp;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2898>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_XPOS,t_Y,t_XOF,t_YOF,5,20,0.0f,1.0f,t_H/20.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2899>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_XPOS+5.0f,t_Y,t_XOF+5,t_YOF,40,20,0.0f,(t_N.f_SWidth-10.0f)/40.0f,t_H/20.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2900>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_XPOS+t_N.f_SWidth-5.0f,t_Y,t_XOF+45,t_YOF,5,20,0.0f,1.0f,t_H/20.0f,0);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawVSlider(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2905>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2906>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2907>";
		float t_W=t_N.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2908>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2909>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2911>";
		int t_OffX=370;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2912>";
		int t_OffY=10;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2914>";
		if(t_Active==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2914>";
			t_OffX=430;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2917>";
		if(((t_N.f_MinusOver)!=0) && t_N.f_MinusDown==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2918>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX+20,t_OffY,20,20,0.0f,t_W/20.0f,t_W/20.0f,0);
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2919>";
			if((t_N.f_MinusDown)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2920>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX+40,t_OffY,20,20,0.0f,t_W/20.0f,t_W/20.0f,0);
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2922>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,20,20,0.0f,t_W/20.0f,t_W/20.0f,0);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2926>";
		if(((t_N.f_PlusOver)!=0) && t_N.f_PlusDown==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2927>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-t_W,t_OffX+20,t_OffY+60,20,20,0.0f,t_W/20.0f,t_W/20.0f,0);
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2928>";
			if((t_N.f_PlusDown)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2929>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-t_W,t_OffX+40,t_OffY+60,20,20,0.0f,t_W/20.0f,t_W/20.0f,0);
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2931>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-t_W,t_OffX,t_OffY+60,20,20,0.0f,t_W/20.0f,t_W/20.0f,0);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2936>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_W,t_OffX,t_OffY+20,20,40,0.0f,t_W/20.0f,(t_H-t_W-t_W)/40.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2939>";
		int t_XOF=370;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2940>";
		int t_YOF=100;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2941>";
		if(((t_N.f_SliderOver)!=0) && t_N.f_SliderDown==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2941>";
			t_XOF=t_XOF+20;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2942>";
		if((t_N.f_SliderDown)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2942>";
			t_XOF=t_XOF+40;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2943>";
		if(t_Active==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2943>";
			t_XOF=t_XOF+60;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2944>";
		float t_YPOS=t_Y+t_W-5.0f+(t_N.f_Value-t_N.f_Minimum)*t_N.f_Stp;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2946>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_YPOS,t_XOF,t_YOF,20,5,0.0f,t_W/20.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2947>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_YPOS+5.0f,t_XOF,t_YOF+5,20,40,0.0f,t_W/20.0f,(t_N.f_SWidth-10.0f)/40.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2948>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_YPOS+t_N.f_SWidth-5.0f,t_XOF,t_YOF+45,20,5,0.0f,t_W/20.0f,1.0f,0);
		bb_std_lang.popErr();
		return 0;
	}
	static int bb_challengergui_CHGUI_Cursor;
	static public int bb_challengergui_CHGUI_DrawTextfield(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2953>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2954>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2955>";
		float t_W=t_N.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2956>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2957>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2959>";
		int t_OffX=280;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2960>";
		int t_OffY=10;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2961>";
		int t_OffH=40;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2963>";
		if((t_N.f_Over)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2963>";
			t_OffY=50;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2964>";
		if(((t_N.f_Down)!=0) && t_N.f_OnFocus==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2964>";
			t_OffY=90;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2965>";
		if(bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N)==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2965>";
			t_OffY=130;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2967>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,0.0f,1.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2969>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y,t_OffX+10,t_OffY,20,10,0.0f,(t_W-20.0f)/20.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2971>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y,t_OffX+30,t_OffY,10,10,0.0f,1.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2973>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+10.0f,t_OffX,t_OffY+10,10,20,0.0f,1.0f,(t_H-20.0f)/20.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2975>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+10.0f,t_OffX+30,t_OffY+10,10,20,0.0f,1.0f,(t_H-20.0f)/20.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2977>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+10.0f,t_OffX+10,t_OffY+10,20,20,0.0f,(t_W-20.0f)/20.0f,(t_H-20.0f)/20.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2979>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0f,t_OffX,t_OffY+30,10,10,0.0f,1.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2981>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+t_H-10.0f,t_OffX+10,t_OffY+30,20,10,0.0f,(t_W-20.0f)/20.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2983>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+t_H-10.0f,t_OffX+30,t_OffY+30,10,10,0.0f,1.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2986>";
		float t_YOff=(t_H-bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text))/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2987>";
		bb_graphics.bb_graphics_SetAlpha(0.25f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2988>";
		if((t_Active)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2988>";
			bb_graphics.bb_graphics_SetAlpha(1.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2989>";
		bb_challengergui.bb_challengergui_CHGUI_Font.m_DrawText2(t_N.f_Text,t_X+5.0f,t_Y+t_YOff);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2990>";
		bb_graphics.bb_graphics_SetAlpha(1.0f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2993>";
		if((t_N.f_OnFocus)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2994>";
			String t_Before=bb_std_lang.slice(t_N.f_Text,0,t_N.f_Cursor);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2995>";
			int t_Length=(int)(bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_Before+"NOT")-bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2("NOT"));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2996>";
			if((bb_challengergui.bb_challengergui_CHGUI_Cursor)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2997>";
				bb_graphics.bb_graphics_SetColor(0.0f,0.0f,0.0f);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2998>";
				bb_graphics.bb_graphics_DrawLine(t_X+(float)(t_Length)+8.0f,t_Y+t_YOff,t_X+(float)(t_Length)+8.0f,t_Y+t_YOff+bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text));
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2999>";
				bb_graphics.bb_graphics_SetColor(255.0f,255.0f,255.0f);
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawLabel(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2462>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2463>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2464>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2467>";
		float t_XOff=.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2468>";
		float t_YOff=.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2470>";
		bb_graphics.bb_graphics_SetAlpha(0.25f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2471>";
		if((t_Active)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2471>";
			bb_graphics.bb_graphics_SetAlpha(1.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2472>";
		bb_challengergui.bb_challengergui_CHGUI_Font.m_DrawText2(t_N.f_Text,t_X,t_Y);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2473>";
		bb_graphics.bb_graphics_SetAlpha(1.0f);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawDropdown(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2551>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2552>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2553>";
		float t_W=t_N.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2554>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2555>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2557>";
		int t_OffX=280;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2558>";
		int t_OffY=10;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2559>";
		if((t_N.f_Over)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2559>";
			t_OffY=50;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2560>";
		if(((t_N.f_Down)!=0) || ((t_N.f_OnFocus)!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2560>";
			t_OffY=90;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2561>";
		if(t_Active==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2561>";
			t_OffY=130;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2564>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2566>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y,t_OffX+10,t_OffY,10,10,0.0f,(t_W-20.0f)/10.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2568>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y,t_OffX+30,t_OffY,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2570>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+10.0f,t_OffX,t_OffY+10,10,10,0.0f,1.0f,(t_H-20.0f)/10.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2572>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+10.0f,t_OffX+30,t_OffY+10,10,10,0.0f,1.0f,(t_H-20.0f)/10.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2574>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+10.0f,t_OffX+10,t_OffY+10,20,20,0.0f,(t_W-20.0f)/20.0f,(t_H-20.0f)/20.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2576>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0f,t_OffX,t_OffY+30,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2578>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+t_H-10.0f,t_OffX+10,t_OffY+30,10,10,0.0f,(t_W-20.0f)/10.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2580>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+t_H-10.0f,t_OffX+30,t_OffY+30,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2583>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-t_H,t_Y,t_OffX+40,t_OffY,40,40,0.0f,t_H/40.0f,t_H/40.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2586>";
		float t_YOff=(t_H-bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text))/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2587>";
		bb_graphics.bb_graphics_SetAlpha(0.25f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2588>";
		if((t_Active)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2588>";
			bb_graphics.bb_graphics_SetAlpha(1.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2589>";
		bb_challengergui.bb_challengergui_CHGUI_Font.m_DrawText2(t_N.f_Text,t_X+5.0f,t_Y+t_YOff);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2590>";
		bb_graphics.bb_graphics_SetAlpha(1.0f);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawDropdownItem(bb_challengergui_CHGUI t_N,int t_C){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2597>";
		t_N.f_X=0.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2598>";
		t_N.f_Y=(float)(t_C+1)*t_N.f_Parent.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2599>";
		t_N.f_W=t_N.f_Parent.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2600>";
		t_N.f_H=t_N.f_Parent.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2602>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2603>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2604>";
		float t_W=t_N.f_Parent.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2605>";
		float t_H=t_N.f_Parent.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2606>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2608>";
		int t_OffX=90;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2609>";
		int t_OffY=80;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2610>";
		int t_OffH=30;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2612>";
		if((t_N.f_Over)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2612>";
			t_OffY=110;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2613>";
		if(((t_N.f_Down)!=0) && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N))!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2613>";
			t_OffY=140;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2615>";
		if(t_C!=0 && t_C!=t_N.f_Parent.f_DropNumber){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2617>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY+10,10,10,0.0f,1.0f,t_H/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2619>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y,t_OffX+50,t_OffY+10,10,10,0.0f,1.0f,t_H/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2621>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y,t_OffX+10,t_OffY+10,40,10,0.0f,(t_W-20.0f)/40.0f,t_H/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2623>";
			if((bb_challengergui.bb_challengergui_CHGUI_Shadow)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2624>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y,40,20,10,10,0.0f,1.0f,t_H/10.0f,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2625>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y,0,20,10,10,0.0f,1.0f,t_H/10.0f,0);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2629>";
		if(t_C==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2631>";
			if((bb_challengergui.bb_challengergui_CHGUI_Shadow)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2632>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y+10.0f,40,20,10,10,0.0f,1.0f,(t_H-10.0f)/10.0f,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2633>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y+10.0f,0,20,10,10,0.0f,1.0f,(t_H-10.0f)/10.0f,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2635>";
				bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y-10.0f,0,0,10,20,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2637>";
				bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y-10.0f,40,0,10,20,0);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2641>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,0.0f,1.0f,1.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2643>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y,t_OffX+10,t_OffY,40,10,0.0f,(t_W-20.0f)/40.0f,1.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2645>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y,t_OffX+50,t_OffY,10,10,0.0f,1.0f,1.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2647>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+10.0f,t_OffX,t_OffY+10,10,10,0.0f,1.0f,(t_H-10.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2649>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+10.0f,t_OffX+50,t_OffY+10,10,10,0.0f,1.0f,(t_H-10.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2651>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+10.0f,t_OffX+10,t_OffY+10,40,10,0.0f,(t_W-20.0f)/40.0f,(t_H-10.0f)/10.0f,0);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2654>";
		if(t_C==t_N.f_Parent.f_DropNumber){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2656>";
			if((bb_challengergui.bb_challengergui_CHGUI_Shadow)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2657>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y,40,20,10,10,0.0f,1.0f,(t_H-10.0f)/10.0f,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2658>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y,0,20,10,10,0.0f,1.0f,(t_H-10.0f)/10.0f,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2660>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+10.0f,t_Y+t_H,20,40,10,10,0.0f,(t_W-20.0f)/10.0f,1.0f,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2662>";
				bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W-10.0f,t_Y+t_H-10.0f,30,30,20,20,0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2664>";
				bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y+t_H-10.0f,0,30,20,20,0);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2667>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY+10,10,10,0.0f,1.0f,(t_H-10.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2669>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y,t_OffX+50,t_OffY+10,10,10,0.0f,1.0f,(t_H-10.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2671>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y,t_OffX+10,t_OffY+10,40,10,0.0f,(t_W-20.0f)/40.0f,(t_H-10.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2673>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-10.0f,t_OffX,t_OffY+20,10,10,0.0f,1.0f,1.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2675>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+t_H-10.0f,t_OffX+10,t_OffY+20,40,10,0.0f,(t_W-20.0f)/40.0f,1.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2677>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+t_H-10.0f,t_OffX+50,t_OffY+20,10,10,0.0f,1.0f,1.0f,0);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2681>";
		float t_YOff=(t_H-bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text))/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2682>";
		bb_graphics.bb_graphics_SetAlpha(0.25f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2683>";
		if((t_Active)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2683>";
			bb_graphics.bb_graphics_SetAlpha(1.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2684>";
		bb_challengergui.bb_challengergui_CHGUI_Font.m_DrawText2(t_N.f_Text,t_X+5.0f,t_Y+t_YOff);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2685>";
		bb_graphics.bb_graphics_SetAlpha(1.0f);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawMenu(bb_challengergui_CHGUI t_N,int t_XOffset,int t_C){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2689>";
		if(t_C==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2689>";
			t_XOffset=t_XOffset+1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2690>";
		t_N.f_X=(float)(t_XOffset-t_C);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2691>";
		if(t_N.f_Parent!=bb_challengergui.bb_challengergui_CHGUI_Canvas && (t_N.f_Parent.f_Text.compareTo("")!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2692>";
			t_N.f_Y=bb_challengergui.bb_challengergui_CHGUI_TitleHeight;
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2694>";
			t_N.f_Y=0.0f;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2696>";
		t_N.f_W=20.0f+bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2697>";
		t_N.f_H=10.0f+bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2698>";
		t_N.f_Parent.f_HasMenu=1;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2699>";
		t_N.f_Parent.f_MenuHeight=(int)(t_N.f_H);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2700>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2701>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2702>";
		float t_W=t_N.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2703>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2704>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2705>";
		int t_OffX=100;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2706>";
		int t_OffY=90;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2707>";
		if((t_N.f_Over)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2707>";
			t_OffY=120;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2708>";
		if(((t_N.f_Down)!=0) || ((t_N.f_OnFocus)!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2709>";
			if(t_Active==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2709>";
				t_OffY=150;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2713>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,40,10,0.0f,t_W/40.0f,(t_H-10.0f)/10.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2714>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+(t_H-10.0f),t_OffX,t_OffY+10,40,10,0.0f,t_W/40.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2717>";
		float t_XOff=(t_W-bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text))/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2718>";
		float t_YOff=(t_H-bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text))/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2719>";
		bb_graphics.bb_graphics_SetAlpha(0.25f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2720>";
		if((t_Active)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2720>";
			bb_graphics.bb_graphics_SetAlpha(1.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2721>";
		bb_challengergui.bb_challengergui_CHGUI_Font.m_DrawText2(t_N.f_Text,t_X+t_XOff,t_Y+t_YOff);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2722>";
		bb_graphics.bb_graphics_SetAlpha(1.0f);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawTab(bb_challengergui_CHGUI t_N,int t_Offset){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3005>";
		t_N.f_X=(float)(t_Offset);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3006>";
		t_N.f_W=20.0f+bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3007>";
		t_N.f_H=10.0f+bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3008>";
		t_N.f_Parent.f_Tabbed=1;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3009>";
		t_N.f_Parent.f_TabHeight=(int)(t_N.f_H);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3010>";
		t_N.f_Y=5.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3011>";
		if(t_N.f_Parent.f_Text.compareTo("")!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3011>";
			t_N.f_Y=t_N.f_Y+bb_challengergui.bb_challengergui_CHGUI_TitleHeight;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3012>";
		if((t_N.f_Parent.f_HasMenu)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3012>";
			t_N.f_Y=t_N.f_Y+(float)(t_N.f_Parent.f_MenuHeight);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3014>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3015>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3016>";
		float t_W=t_N.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3017>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3018>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3020>";
		int t_OffX=10;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3021>";
		int t_OffY=180;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3022>";
		if(t_N.f_Parent.f_CurrentTab==t_N){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3023>";
			t_OffX=70;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3024>";
			t_OffY=180;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3026>";
		if((t_N.f_Down)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3027>";
			t_OffX=10;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3028>";
			t_OffY=210;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3030>";
		if(((t_N.f_Over)!=0) && t_N.f_Down==0 && t_N.f_Parent.f_CurrentTab!=t_N){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3031>";
			t_OffX=40;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3032>";
			t_OffY=180;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3035>";
		if(t_Active==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3036>";
			t_OffX=40;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3037>";
			t_OffY=210;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3042>";
		int t_YY=(int)(t_Y);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3045>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,(float)(t_YY),t_OffX,t_OffY,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3047>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,(float)(t_YY),t_OffX+20,t_OffY,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3049>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,(float)(t_YY),t_OffX+10,t_OffY,10,10,0.0f,(t_W-20.0f)/10.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3051>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,(float)(t_YY+10),t_OffX,t_OffY+10,10,10,0.0f,1.0f,(float)((t_N.f_Parent.f_TabHeight-10)/10),0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3053>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,(float)(t_YY+10),t_OffX+20,t_OffY+10,10,10,0.0f,1.0f,(float)((t_N.f_Parent.f_TabHeight-10)/10),0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3055>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,(float)(t_YY+10),t_OffX+10,t_OffY+10,10,10,0.0f,(t_W-20.0f)/10.0f,(float)((t_N.f_Parent.f_TabHeight-10)/10),0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3058>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,(float)(t_YY+t_N.f_Parent.f_TabHeight-10),t_OffX,t_OffY+20,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3060>";
		bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,(float)(t_YY+t_N.f_Parent.f_TabHeight-10),t_OffX+20,t_OffY+20,10,10,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3062>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,(float)(t_YY+t_N.f_Parent.f_TabHeight-10),t_OffX+10,t_OffY+20,10,10,0.0f,(t_W-20.0f)/10.0f,1.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3065>";
		float t_XOff=(t_W-bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text))/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3066>";
		float t_YOff=((float)(t_N.f_Parent.f_TabHeight)-bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text))/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3067>";
		bb_graphics.bb_graphics_SetAlpha(0.25f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3068>";
		if((t_Active)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3068>";
			bb_graphics.bb_graphics_SetAlpha(1.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3069>";
		bb_challengergui.bb_challengergui_CHGUI_Font.m_DrawText2(t_N.f_Text,t_X+t_XOff,(float)(t_YY)+t_YOff);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3070>";
		bb_graphics.bb_graphics_SetAlpha(1.0f);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawMenuItem(bb_challengergui_CHGUI t_N,int t_C){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2728>";
		if(t_N.f_Parent.f_Element.compareTo("MenuItem")!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2729>";
			t_N.f_X=0.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2730>";
			t_N.f_Y=t_N.f_Parent.f_H+(float)(t_C)*t_N.f_Parent.f_H-(float)(t_C);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2731>";
			t_N.f_Parent.f_HasMenu=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2732>";
			t_N.f_Parent.f_MenuHeight=(int)(t_N.f_H);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2734>";
		if(t_N.f_Parent.f_Element.compareTo("MenuItem")==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2735>";
			t_N.f_X=t_N.f_Parent.f_W-1.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2736>";
			t_N.f_Y=(float)(t_C)*t_N.f_Parent.f_H-(float)(t_C);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2739>";
		t_N.f_H=10.0f+bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2740>";
		t_N.f_W=20.0f+bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2742>";
		if((t_N.f_IsMenuParent)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2742>";
			t_N.f_W=t_N.f_W+t_N.f_H/3.0f;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2743>";
		if((t_N.f_Tick)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2743>";
			t_N.f_W=t_N.f_W+t_N.f_H/3.0f;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2745>";
		if(t_N.f_W>(float)(t_N.f_Parent.f_MenuWidth)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2745>";
			t_N.f_Parent.f_MenuWidth=(int)(t_N.f_W);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2746>";
		if((float)(t_N.f_Parent.f_MenuWidth)>t_N.f_W){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2746>";
			t_N.f_W=(float)(t_N.f_Parent.f_MenuWidth);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2748>";
		if((t_N.f_Tick)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2749>";
			if((float)(t_N.f_Parent.f_MenuWidth)+t_N.f_H<t_N.f_W){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2749>";
				t_N.f_Parent.f_MenuWidth=(int)(t_N.f_W+t_N.f_H);
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2751>";
			if((float)(t_N.f_Parent.f_MenuWidth)<t_N.f_W){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2751>";
				t_N.f_Parent.f_MenuWidth=(int)(t_N.f_W);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2754>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2755>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2756>";
		float t_W=t_N.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2757>";
		float t_H=t_N.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2758>";
		int t_Active=bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2760>";
		int t_OffX=100;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2761>";
		int t_OffY=90;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2762>";
		if(((t_N.f_Over)!=0) || ((t_N.f_OnFocus)!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2762>";
			t_OffY=120;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2763>";
		if(((t_N.f_Down)!=0) && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N))!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2763>";
			t_OffY=150;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2766>";
		if(t_C==t_N.f_Parent.f_MenuNumber && ((bb_challengergui.bb_challengergui_CHGUI_Shadow)!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2767>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y,40,20,10,10,0.0f,1.0f,(t_H-10.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2768>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y,0,20,10,10,0.0f,1.0f,(t_H-10.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2770>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+10.0f,t_Y+t_H,20,40,10,10,0.0f,(t_W-20.0f)/10.0f,1.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2772>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W-10.0f,t_Y+t_H-10.0f,30,30,20,20,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2774>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y+t_H-10.0f,0,30,20,20,0);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2778>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y,t_OffX,t_OffY,40,10,0.0f,(t_W-20.0f)/40.0f,t_H/10.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2779>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX-10,t_OffY,10,10,0.0f,1.0f,t_H/10.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2780>";
		bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y,t_OffX+40,t_OffY,10,10,0.0f,1.0f,t_H/10.0f,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2783>";
		if(((bb_challengergui.bb_challengergui_CHGUI_Shadow)!=0) && t_C!=t_N.f_Parent.f_MenuNumber){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2785>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y,40,20,10,10,0.0f,1.0f,(t_H-1.0f)/10.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2787>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_ShadowImg,t_X-10.0f,t_Y,0,20,10,10,0.0f,1.0f,(t_H-1.0f)/10.0f,0);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2791>";
		if(t_C==t_N.f_Parent.f_MenuNumber){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2792>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y+(t_H-10.0f),t_OffX-10,t_OffY+10,10,10,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2793>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y+(t_H-10.0f),t_OffX,t_OffY+10,40,10,0.0f,(t_W-20.0f)/40.0f,1.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2794>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y+(t_H-10.0f),t_OffX+40,t_OffY+10,10,10,0);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2797>";
		if(t_C==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2798>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX-10,t_OffY-10,10,10,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2799>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+10.0f,t_Y,t_OffX,t_OffY-10,40,10,0.0f,(t_W-20.0f)/40.0f,1.0f,0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2800>";
			bb_graphics.bb_graphics_DrawImageRect(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-10.0f,t_Y,t_OffX+40,t_OffY-10,10,10,0);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2804>";
		if((t_N.f_Tick)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2805>";
			int t_XOF=230;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2806>";
			int t_YOF=10;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2807>";
			if((t_N.f_Over)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2807>";
				t_YOF=30;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2808>";
			if((t_N.f_Down)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2808>";
				t_YOF=50;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2809>";
			if(bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2809>";
				t_XOF=70;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2810>";
			if(t_N.f_Value>0.0f){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2810>";
				t_XOF=250;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2812>";
			bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+5.0f,t_Y+(t_H-t_H/2.6f)/2.0f,t_XOF,t_YOF,20,20,0.0f,t_H/2.6f/20.0f,t_H/2.6f/20.0f,0);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2819>";
		float t_YOff=(t_H-bb_challengergui.bb_challengergui_CHGUI_TextHeight(bb_challengergui.bb_challengergui_CHGUI_Font,t_N.f_Text))/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2820>";
		bb_graphics.bb_graphics_SetAlpha(0.25f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2821>";
		if((t_Active)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2821>";
			bb_graphics.bb_graphics_SetAlpha(1.0f);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2822>";
		if(t_N.f_Tick==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2823>";
			bb_challengergui.bb_challengergui_CHGUI_Font.m_DrawText2(t_N.f_Text,t_X+10.0f,t_Y+t_YOff);
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2825>";
			bb_challengergui.bb_challengergui_CHGUI_Font.m_DrawText2(t_N.f_Text,t_X+10.0f+t_H/2.0f,t_Y+t_YOff);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2827>";
		bb_graphics.bb_graphics_SetAlpha(1.0f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2830>";
		if((t_N.f_IsMenuParent)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2831>";
			if(bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N)==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2832>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-t_H/4.0f-8.0f,t_Y+(t_H-t_H/4.0f)/2.0f,130,180,10,10,0.0f,t_H/4.0f/10.0f,t_H/4.0f/10.0f,0);
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2834>";
				bb_graphics.bb_graphics_DrawImageRect2(bb_challengergui.bb_challengergui_CHGUI_Style,t_X+t_W-t_H/4.0f-8.0f,t_Y+(t_H-t_H/4.0f)/2.0f,140,180,10,10,0.0f,t_H/4.0f/10.0f,t_H/4.0f/10.0f,0);
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_SubMenu(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2840>";
		int t_C=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2841>";
		int t_XX=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2842>";
		if((t_N.f_OnFocus)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2843>";
			t_C=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2844>";
			t_N.f_HasMenu=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2845>";
			for(t_XX=0;t_XX<=bb_std_lang.arrayLength(t_N.f_MenuItems)-1;t_XX=t_XX+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2846>";
				if(bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_MenuItems[t_XX])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_MenuItems[t_XX]))!=0)){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2846>";
					bb_challengergui.bb_challengergui_CHGUI_DrawMenuItem(t_N.f_MenuItems[t_XX],t_C);
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2847>";
				if(bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_MenuItems[t_XX])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_MenuItems[t_XX]))!=0)){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2847>";
					t_C=t_C+1;
				}
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2849>";
			if(bb_std_lang.arrayLength(t_N.f_MenuItems)>0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2849>";
				t_N.f_MenuNumber=t_C-1;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2850>";
			for(t_XX=0;t_XX<=bb_std_lang.arrayLength(t_N.f_MenuItems)-1;t_XX=t_XX+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2851>";
				if((t_N.f_MenuItems[t_XX].f_IsMenuParent)!=0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2851>";
					bb_challengergui.bb_challengergui_CHGUI_SubMenu(t_N.f_MenuItems[t_XX]);
				}
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_DrawContents(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2092>";
		int t_X=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2093>";
		int t_XX=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2094>";
		int t_XOffset=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2095>";
		int t_C=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2098>";
		if(t_N.f_Element.compareTo("Tab")!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2099>";
			if(t_N.f_Parent!=null){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2100>";
				if(bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N)==1 && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Parent)==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2100>";
					bb_challengergui.bb_challengergui_CHGUI_DrawWindow(t_N);
				}
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2102>";
				bb_challengergui.bb_challengergui_CHGUI_DrawWindow(t_N);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2107>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_Buttons)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2108>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Buttons[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Buttons[t_X])==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2108>";
				bb_challengergui.bb_challengergui_CHGUI_DrawButton(t_N.f_Buttons[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2112>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_ImageButtons)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2113>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_ImageButtons[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_ImageButtons[t_X])==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2113>";
				bb_challengergui.bb_challengergui_CHGUI_DrawImageButton(t_N.f_ImageButtons[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2117>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_Tickboxes)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2118>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Tickboxes[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Tickboxes[t_X])==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2118>";
				bb_challengergui.bb_challengergui_CHGUI_DrawTickbox(t_N.f_Tickboxes[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2122>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_Radioboxes)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2123>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Radioboxes[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Radioboxes[t_X])==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2123>";
				bb_challengergui.bb_challengergui_CHGUI_DrawRadiobox(t_N.f_Radioboxes[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2127>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_Listboxes)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2128>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Listboxes[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Listboxes[t_X])==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2129>";
				bb_challengergui.bb_challengergui_CHGUI_DrawListbox(t_N.f_Listboxes[t_X]);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2131>";
				int t_C2=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2133>";
				for(t_XX=(int)(t_N.f_Listboxes[t_X].f_ListboxSlider.f_Value);(float)(t_XX)<=t_N.f_Listboxes[t_X].f_ListboxSlider.f_Value+(float)(t_N.f_Listboxes[t_X].f_ListboxNumber);t_XX=t_XX+1){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2134>";
					if(t_XX<bb_std_lang.arrayLength(t_N.f_Listboxes[t_X].f_ListboxItems) && t_XX>-1){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2135>";
						if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Listboxes[t_X].f_ListboxItems[t_XX]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Listboxes[t_X].f_ListboxItems[t_XX])==0){
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2136>";
							bb_challengergui.bb_challengergui_CHGUI_DrawListboxItem(t_N.f_Listboxes[t_X].f_ListboxItems[t_XX],t_C2);
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2137>";
							t_C2=t_C2+1;
						}
					}
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2145>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_HSliders)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2146>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_HSliders[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_HSliders[t_X])==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2146>";
				bb_challengergui.bb_challengergui_CHGUI_DrawHSlider(t_N.f_HSliders[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2150>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_VSliders)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2151>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_VSliders[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_VSliders[t_X])==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2151>";
				bb_challengergui.bb_challengergui_CHGUI_DrawVSlider(t_N.f_VSliders[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2155>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_Textfields)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2156>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Textfields[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Textfields[t_X])==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2156>";
				bb_challengergui.bb_challengergui_CHGUI_DrawTextfield(t_N.f_Textfields[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2161>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_Labels)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2162>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Labels[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Labels[t_X])==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2162>";
				bb_challengergui.bb_challengergui_CHGUI_DrawLabel(t_N.f_Labels[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2166>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_Dropdowns)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2167>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Dropdowns[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Dropdowns[t_X])==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2168>";
				bb_challengergui.bb_challengergui_CHGUI_DrawDropdown(t_N.f_Dropdowns[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2173>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_Dropdowns)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2174>";
			if(t_N.f_Dropdowns[t_X].f_OnFocus==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2175>";
				t_C=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2176>";
				for(t_XX=0;t_XX<=bb_std_lang.arrayLength(t_N.f_Dropdowns[t_X].f_DropdownItems)-1;t_XX=t_XX+1){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2177>";
					if(bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Dropdowns[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Dropdowns[t_X].f_DropdownItems[t_XX]))!=0)){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2177>";
						bb_challengergui.bb_challengergui_CHGUI_DrawDropdownItem(t_N.f_Dropdowns[t_X].f_DropdownItems[t_XX],t_C);
					}
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2178>";
					if(bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Dropdowns[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Dropdowns[t_X].f_DropdownItems[t_XX]))!=0)){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2178>";
						t_C=t_C+1;
					}
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2180>";
				if(bb_std_lang.arrayLength(t_N.f_Dropdowns)>0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2180>";
					t_N.f_Dropdowns[t_X].f_DropNumber=t_C-1;
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2185>";
		t_XOffset=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2186>";
		t_N.f_HasMenu=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2187>";
		t_C=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2188>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_Menus)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2189>";
			if(bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Menus[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Menus[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2189>";
				bb_challengergui.bb_challengergui_CHGUI_DrawMenu(t_N.f_Menus[t_X],t_XOffset,t_C);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2190>";
			if(bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Menus[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Menus[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2190>";
				t_XOffset=(int)((float)(t_XOffset)+t_N.f_Menus[t_X].f_W);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2191>";
			if(bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Menus[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Menus[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2191>";
				t_C=t_C+1;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2195>";
		t_C=5;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2196>";
		t_N.f_Tabbed=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2197>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_Tabs)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2198>";
			if(bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Tabs[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Tabs[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2198>";
				bb_challengergui.bb_challengergui_CHGUI_DrawTab(t_N.f_Tabs[t_X],t_C);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2199>";
			if(bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Tabs[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Tabs[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2199>";
				t_C=(int)((float)(t_C)+t_N.f_Tabs[t_X].f_W);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2203>";
		for(int t_NN=0;t_NN<=bb_std_lang.arrayLength(t_N.f_BottomList)-1;t_NN=t_NN+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2204>";
			if((t_N.f_BottomList[t_NN].f_Visible)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2204>";
				bb_challengergui.bb_challengergui_CHGUI_DrawContents(t_N.f_BottomList[t_NN]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2206>";
		for(int t_NN2=0;t_NN2<=bb_std_lang.arrayLength(t_N.f_VariList)-1;t_NN2=t_NN2+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2207>";
			if((t_N.f_VariList[t_NN2].f_Visible)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2207>";
				bb_challengergui.bb_challengergui_CHGUI_DrawContents(t_N.f_VariList[t_NN2]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2209>";
		for(int t_NN3=0;t_NN3<=bb_std_lang.arrayLength(t_N.f_TopList)-1;t_NN3=t_NN3+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2210>";
			if((t_N.f_TopList[t_NN3].f_Visible)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2210>";
				bb_challengergui.bb_challengergui_CHGUI_DrawContents(t_N.f_TopList[t_NN3]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2213>";
		if((t_N.f_Tabbed)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2214>";
			bb_challengergui.bb_challengergui_CHGUI_DrawContents(t_N.f_CurrentTab);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2218>";
		for(t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_Menus)-1;t_X=t_X+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2219>";
			bb_challengergui.bb_challengergui_CHGUI_SubMenu(t_N.f_Menus[t_X]);
		}
		bb_std_lang.popErr();
		return 0;
	}
	static bb_challengergui_CHGUI[] bb_challengergui_CHGUI_VariList;
	static bb_challengergui_CHGUI[] bb_challengergui_CHGUI_TopList;
	static bb_challengergui_CHGUI bb_challengergui_CHGUI_TooltipFlag;
	static bb_bitmapfont_BitmapFont bb_challengergui_CHGUI_TooltipFont;
	static public int bb_challengergui_CHGUI_DrawTooltip(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3161>";
		int t_X=(int)((float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N))+t_N.f_W);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3162>";
		int t_Y=(int)((float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N))+t_N.f_H);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3163>";
		int t_W=(int)(bb_challengergui.bb_challengergui_CHGUI_TooltipFont.m_GetTxtWidth2(t_N.f_Tooltip)+10.0f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3164>";
		int t_H=(int)(bb_challengergui.bb_challengergui_CHGUI_TooltipFont.m_GetTxtHeight(t_N.f_Tooltip)+10.0f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3165>";
		if(t_X+t_W>bb_graphics.bb_graphics_DeviceWidth()){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3165>";
			t_X=bb_graphics.bb_graphics_DeviceWidth()-t_W;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3166>";
		if(t_Y+t_H>bb_graphics.bb_graphics_DeviceHeight()){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3166>";
			t_Y=bb_challengergui.bb_challengergui_CHGUI_RealY(t_N)-t_H;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3167>";
		bb_graphics.bb_graphics_SetColor(100.0f,100.0f,100.0f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3168>";
		bb_graphics.bb_graphics_DrawRect((float)(t_X),(float)(t_Y),(float)(t_W),(float)(t_H));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3169>";
		bb_graphics.bb_graphics_SetColor(250.0f,250.0f,210.0f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3170>";
		bb_graphics.bb_graphics_DrawRect((float)(t_X+1),(float)(t_Y+1),(float)(t_W-2),(float)(t_H-2));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3171>";
		bb_graphics.bb_graphics_SetColor(255.0f,255.0f,255.0f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3172>";
		bb_challengergui.bb_challengergui_CHGUI_TooltipFont.m_DrawText2(t_N.f_Tooltip,(float)(t_X+5),(float)(t_Y+5));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3173>";
		bb_graphics.bb_graphics_SetColor(255.0f,255.0f,255.0f);
		bb_std_lang.popErr();
		return 0;
	}
	static int bb_challengergui_CHGUI_Millisecs;
	static int bb_challengergui_CHGUI_FPSCounter;
	static int bb_challengergui_CHGUI_FPS;
	static public int bb_challengergui_CHGUI_FPSUpdate(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4023>";
		if(bb_app.bb_app_Millisecs()>bb_challengergui.bb_challengergui_CHGUI_Millisecs){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4024>";
			bb_challengergui.bb_challengergui_CHGUI_Millisecs=bb_app.bb_app_Millisecs()+1000;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4025>";
			bb_challengergui.bb_challengergui_CHGUI_FPS=bb_challengergui.bb_challengergui_CHGUI_FPSCounter;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4026>";
			bb_challengergui.bb_challengergui_CHGUI_FPSCounter=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4028>";
		bb_challengergui.bb_challengergui_CHGUI_FPSCounter=bb_challengergui.bb_challengergui_CHGUI_FPSCounter+1;
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_Draw(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<850>";
		int t_N=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<851>";
		int t_NN=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<853>";
		bb_graphics.bb_graphics_SetBlend(0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<854>";
		bb_graphics.bb_graphics_SetColor(255.0f,255.0f,255.0f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<856>";
		for(t_N=0;t_N<=bb_std_lang.arrayLength(bb_challengergui.bb_challengergui_CHGUI_BottomList)-1;t_N=t_N+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<857>";
			if((bb_challengergui.bb_challengergui_CHGUI_BottomList[t_N].f_Visible)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<857>";
				bb_challengergui.bb_challengergui_CHGUI_DrawContents(bb_challengergui.bb_challengergui_CHGUI_BottomList[t_N]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<859>";
		for(t_N=0;t_N<=bb_std_lang.arrayLength(bb_challengergui.bb_challengergui_CHGUI_VariList)-1;t_N=t_N+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<860>";
			if((bb_challengergui.bb_challengergui_CHGUI_VariList[t_N].f_Visible)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<860>";
				bb_challengergui.bb_challengergui_CHGUI_DrawContents(bb_challengergui.bb_challengergui_CHGUI_VariList[t_N]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<862>";
		for(t_N=0;t_N<=bb_std_lang.arrayLength(bb_challengergui.bb_challengergui_CHGUI_TopList)-1;t_N=t_N+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<863>";
			if((bb_challengergui.bb_challengergui_CHGUI_TopList[t_N].f_Visible)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<863>";
				bb_challengergui.bb_challengergui_CHGUI_DrawContents(bb_challengergui.bb_challengergui_CHGUI_TopList[t_N]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<866>";
		if(bb_challengergui.bb_challengergui_CHGUI_TooltipFlag!=null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<866>";
			bb_challengergui.bb_challengergui_CHGUI_DrawTooltip(bb_challengergui.bb_challengergui_CHGUI_TooltipFlag);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<868>";
		bb_challengergui.bb_challengergui_CHGUI_FPSUpdate();
		bb_std_lang.popErr();
		return 0;
	}
	static int bb_challengergui_CHGUI_Width;
	static int bb_challengergui_CHGUI_Height;
	static int bb_challengergui_CHGUI_CanvasFlag;
	static int bb_challengergui_CHGUI_Started;
	static bb_challengergui_CHGUI bb_challengergui_CHGUI_TopTop;
	static public bb_challengergui_CHGUI bb_challengergui_CreateWindow(int t_X,int t_Y,int t_W,int t_H,String t_Title,int t_Moveable,int t_CloseButton,int t_MinimiseButton,int t_Mode,bb_challengergui_CHGUI t_Parent){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<377>";
		if(bb_challengergui.bb_challengergui_CHGUI_Started==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<378>";
			bb_challengergui.bb_challengergui_CHGUI_Started=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<379>";
			bb_challengergui.bb_challengergui_CHGUI_Start();
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<381>";
		bb_challengergui_CHGUI t_N=(new bb_challengergui_CHGUI()).g_new();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<382>";
		t_N.f_X=(float)(t_X);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<383>";
		t_N.f_Y=(float)(t_Y);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<384>";
		t_N.f_W=(float)(t_W);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<385>";
		t_N.f_H=(float)(t_H);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<386>";
		t_N.f_Text=t_Title;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<387>";
		t_N.f_Shadow=bb_challengergui.bb_challengergui_CHGUI_Shadow;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<388>";
		t_N.f_Close=t_CloseButton;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<389>";
		t_N.f_Minimise=t_MinimiseButton;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<390>";
		t_N.f_Moveable=t_Moveable;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<391>";
		t_N.f_Mode=t_Mode;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<392>";
		t_N.f_Parent=t_Parent;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<393>";
		if(t_N.f_Parent==null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<393>";
			t_N.f_Parent=bb_challengergui.bb_challengergui_CHGUI_Canvas;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<394>";
		if(t_N.f_Parent!=null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<394>";
			t_N.f_Parent.f_IsParent=1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<395>";
		t_N.f_Element="Window";
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<396>";
		if(bb_challengergui.bb_challengergui_CHGUI_CanvasFlag==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<397>";
			if(t_Mode==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<398>";
				bb_challengergui.bb_challengergui_CHGUI_BottomList=(bb_challengergui_CHGUI[])bb_std_lang.resizeArray(bb_challengergui.bb_challengergui_CHGUI_BottomList,bb_std_lang.arrayLength(bb_challengergui.bb_challengergui_CHGUI_BottomList)+1);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<399>";
				bb_challengergui.bb_challengergui_CHGUI_BottomList[bb_std_lang.arrayLength(bb_challengergui.bb_challengergui_CHGUI_BottomList)-1]=t_N;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<401>";
			if(t_Mode==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<402>";
				bb_challengergui.bb_challengergui_CHGUI_VariList=(bb_challengergui_CHGUI[])bb_std_lang.resizeArray(bb_challengergui.bb_challengergui_CHGUI_VariList,bb_std_lang.arrayLength(bb_challengergui.bb_challengergui_CHGUI_VariList)+1);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<403>";
				bb_challengergui.bb_challengergui_CHGUI_VariList[bb_std_lang.arrayLength(bb_challengergui.bb_challengergui_CHGUI_VariList)-1]=t_N;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<405>";
			if(t_Mode==2){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<406>";
				bb_challengergui.bb_challengergui_CHGUI_TopList=(bb_challengergui_CHGUI[])bb_std_lang.resizeArray(bb_challengergui.bb_challengergui_CHGUI_TopList,bb_std_lang.arrayLength(bb_challengergui.bb_challengergui_CHGUI_TopList)+1);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<407>";
				bb_challengergui.bb_challengergui_CHGUI_TopList[bb_std_lang.arrayLength(bb_challengergui.bb_challengergui_CHGUI_TopList)-1]=t_N;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<408>";
				bb_challengergui.bb_challengergui_CHGUI_TopTop=t_N;
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<411>";
			t_N.f_SubWindow=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<412>";
			if(t_Mode==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<413>";
				t_N.f_Parent.f_BottomList=(bb_challengergui_CHGUI[])bb_std_lang.resizeArray(t_N.f_Parent.f_BottomList,bb_std_lang.arrayLength(t_N.f_Parent.f_BottomList)+1);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<414>";
				t_N.f_Parent.f_BottomList[bb_std_lang.arrayLength(t_N.f_Parent.f_BottomList)-1]=t_N;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<416>";
			if(t_Mode==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<417>";
				t_N.f_Parent.f_VariList=(bb_challengergui_CHGUI[])bb_std_lang.resizeArray(t_N.f_Parent.f_VariList,bb_std_lang.arrayLength(t_N.f_Parent.f_VariList)+1);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<418>";
				t_N.f_Parent.f_VariList[bb_std_lang.arrayLength(t_N.f_Parent.f_VariList)-1]=t_N;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<420>";
			if(t_Mode==2){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<421>";
				t_N.f_Parent.f_TopList=(bb_challengergui_CHGUI[])bb_std_lang.resizeArray(t_N.f_Parent.f_TopList,bb_std_lang.arrayLength(t_N.f_Parent.f_TopList)+1);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<422>";
				t_N.f_Parent.f_TopList[bb_std_lang.arrayLength(t_N.f_Parent.f_TopList)-1]=t_N;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<426>";
		bb_std_lang.popErr();
		return t_N;
	}
	static bb_challengergui_CHGUI bb_challengergui_CHGUI_KeyboardWindow;
	static public bb_challengergui_CHGUI bb_challengergui_CHGUI_CreateKeyButton(int t_X,int t_Y,int t_W,int t_H,String t_Text,bb_challengergui_CHGUI t_Parent){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3209>";
		bb_challengergui_CHGUI t_N=(new bb_challengergui_CHGUI()).g_new();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3210>";
		t_N.f_Parent=t_Parent;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3211>";
		if(t_Parent==null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3211>";
			t_N.f_Parent=bb_challengergui.bb_challengergui_CHGUI_Canvas;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3212>";
		t_N.f_X=(float)(t_X);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3213>";
		t_N.f_Y=(float)(t_Y);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3214>";
		t_N.f_W=(float)(t_W);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3215>";
		t_N.f_H=(float)(t_H);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3216>";
		t_N.f_Text=t_Text;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3218>";
		t_N.f_Parent.f_Buttons=(bb_challengergui_CHGUI[])bb_std_lang.resizeArray(t_N.f_Parent.f_Buttons,bb_std_lang.arrayLength(t_N.f_Parent.f_Buttons)+1);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3219>";
		t_N.f_Parent.f_Buttons[bb_std_lang.arrayLength(t_N.f_Parent.f_Buttons)-1]=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3220>";
		bb_std_lang.popErr();
		return t_N;
	}
	static public int bb_challengergui_CHGUI_CreateKeyboard(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3224>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow=bb_challengergui.bb_challengergui_CreateWindow(0,0,bb_graphics.bb_graphics_DeviceWidth(),100,"",0,0,0,2,null);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3225>";
		float t_KeyWidth=(float)(bb_graphics.bb_graphics_DeviceWidth())/12.5f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3226>";
		float t_KeyHeight=(float)(bb_graphics.bb_graphics_DeviceWidth())/12.5f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3228>";
		float t_GapX=t_KeyWidth*2.0f/9.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3229>";
		float t_GapY=t_KeyWidth*2.0f/9.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3231>";
		if(bb_graphics.bb_graphics_DeviceWidth()>bb_graphics.bb_graphics_DeviceHeight()){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3232>";
			t_KeyHeight=t_KeyHeight/1.7f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3233>";
			t_GapY=t_GapY/1.2f;
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3235>";
			t_KeyHeight=t_KeyHeight*1.5f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3236>";
			t_GapY=t_GapY*1.2f;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3240>";
		float t_EndGap=((float)(bb_graphics.bb_graphics_DeviceWidth())-t_KeyWidth*10.0f-t_GapX*9.0f)/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3241>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_H=t_EndGap*2.0f+t_GapY*3.0f+t_KeyHeight*4.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3242>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_Y=(float)(bb_graphics.bb_graphics_DeviceHeight())-bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3245>";
		float t_SX=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3246>";
		float t_SY=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3249>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[0]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"q",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3250>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3251>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[1]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"w",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3252>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3253>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[2]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"e",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3254>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3255>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[3]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"r",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3256>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3257>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[4]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"t",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3258>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3259>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[5]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"y",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3260>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3261>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[6]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"u",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3262>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3263>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[7]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"i",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3264>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3265>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[8]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"o",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3266>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3267>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[9]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"p",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3268>";
		t_SX=t_EndGap+t_KeyWidth/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3269>";
		t_SY=t_SY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3270>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[10]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"a",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3271>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3272>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[11]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"s",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3273>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3274>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[12]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"d",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3275>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3276>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[13]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"f",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3277>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3278>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[14]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"g",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3279>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3280>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[15]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"h",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3281>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3282>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[16]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"j",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3283>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3284>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[17]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"k",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3285>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3286>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[18]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"l",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3287>";
		t_SX=t_EndGap+t_KeyWidth/2.0f+t_GapX+t_KeyWidth;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3288>";
		t_SY=t_SY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3289>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[19]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"z",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3290>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3291>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[20]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"x",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3292>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3293>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[21]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"c",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3294>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3295>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[22]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"v",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3296>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3297>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[23]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"b",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3298>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3299>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[24]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"n",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3300>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3301>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[25]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"m",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3303>";
		t_SX=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3304>";
		t_SY=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3307>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[26]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"Q",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3308>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3309>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[27]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"W",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3310>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3311>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[28]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"E",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3312>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3313>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[29]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"R",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3314>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3315>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[30]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"T",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3316>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3317>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[31]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"Y",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3318>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3319>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[32]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"U",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3320>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3321>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[33]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"I",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3322>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3323>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[34]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"O",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3324>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3325>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[35]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"P",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3326>";
		t_SX=t_EndGap+t_KeyWidth/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3327>";
		t_SY=t_SY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3328>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[36]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"A",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3329>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3330>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[37]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"S",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3331>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3332>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[38]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"D",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3333>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3334>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[39]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"F",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3335>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3336>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[40]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"G",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3337>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3338>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[41]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"H",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3339>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3340>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[42]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"J",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3341>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3342>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[43]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"K",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3343>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3344>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[44]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"L",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3345>";
		t_SX=t_EndGap+t_KeyWidth/2.0f+t_GapX+t_KeyWidth;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3346>";
		t_SY=t_SY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3347>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[45]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"Z",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3348>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3349>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[46]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"X",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3350>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3351>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[47]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"C",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3352>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3353>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[48]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"V",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3354>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3355>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[49]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"B",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3356>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3357>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[50]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"N",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3358>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3359>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[51]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"M",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3362>";
		t_SX=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3363>";
		t_SY=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3366>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[52]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"1",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3367>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3368>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[53]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"2",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3369>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3370>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[54]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"3",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3371>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3372>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[55]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"4",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3373>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3374>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[56]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"5",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3375>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3376>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[57]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"6",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3377>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3378>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[58]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"7",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3379>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3380>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[59]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"8",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3381>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3382>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[60]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"9",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3383>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3384>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[61]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"0",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3385>";
		t_SX=t_EndGap+t_KeyWidth/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3386>";
		t_SY=t_SY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3387>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[62]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"-",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3388>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3389>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[63]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"/",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3390>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3391>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[64]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"\\",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3392>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3393>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[65]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),":",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3394>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3395>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[66]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),";",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3396>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3397>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[67]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"(",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3398>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3399>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[68]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),")",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3400>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3401>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[69]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"\u00a3",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3402>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3403>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[70]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"&",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3404>";
		t_SX=t_EndGap+t_KeyWidth/2.0f+t_GapX+t_KeyWidth;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3405>";
		t_SY=t_SY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3406>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[71]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"@",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3407>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3408>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[72]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),".",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3409>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3410>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[73]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),",",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3411>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3412>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[74]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"?",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3413>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3414>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[75]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"!",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3415>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3416>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[76]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"'",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3417>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3418>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[77]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),String.valueOf((char)(34)),bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3421>";
		t_SX=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3422>";
		t_SY=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3425>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[78]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"[",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3426>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3427>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[79]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"]",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3428>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3429>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[80]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"{",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3430>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3431>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[81]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"}",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3432>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3433>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[82]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"#",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3434>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3435>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[83]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"%",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3436>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3437>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[84]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"^",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3438>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3439>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[85]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"*",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3440>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3441>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[86]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"+",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3442>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3443>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[87]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"=",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3444>";
		t_SX=t_EndGap+t_KeyWidth/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3445>";
		t_SY=t_SY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3446>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[88]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"_",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3447>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3448>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[89]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"|",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3449>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3450>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[90]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"~",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3451>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3452>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[91]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"<",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3453>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3454>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[92]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),">",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3455>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3456>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[93]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"$",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3457>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3458>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[94]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"\u20ac",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3459>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3460>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[95]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"\u00e9",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3461>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3462>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[96]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"\u00ac",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3463>";
		t_SX=t_EndGap+t_KeyWidth/2.0f+t_GapX+t_KeyWidth;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3464>";
		t_SY=t_SY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3465>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[97]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"@",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3466>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3467>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[98]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),".",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3468>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3469>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[99]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),",",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3470>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3471>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[100]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"?",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3472>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3473>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[101]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"!",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3474>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3475>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[102]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),"'",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3476>";
		t_SX=t_SX+t_KeyWidth+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3477>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[103]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth),(int)(t_KeyHeight),String.valueOf((char)(34)),bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3480>";
		t_SX=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3481>";
		t_SY=t_EndGap+t_KeyHeight+t_GapY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3482>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth+t_KeyWidth/2.0f),(int)(t_KeyHeight),"Shft",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3483>";
		t_SX=t_EndGap+t_KeyWidth*9.0f+t_GapX*9.0f-t_KeyWidth/2.0f-t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3484>";
		t_SY=t_EndGap+t_KeyHeight+t_GapY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3485>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[105]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth+t_KeyWidth/2.0f),(int)(t_KeyHeight),"<--",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3486>";
		t_SX=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3487>";
		t_SY=t_SY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3488>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth*2.0f+t_KeyWidth/2.0f+t_GapX),(int)(t_KeyHeight),"123",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3489>";
		t_SX=t_SX+t_KeyWidth*2.0f+t_KeyWidth/2.0f+t_GapX+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3490>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[107]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth*5.0f+t_GapX*4.0f),(int)(t_KeyHeight)," ",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3491>";
		t_SX=t_SX+t_KeyWidth*5.0f+t_GapX*5.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3492>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[108]=bb_challengergui.bb_challengergui_CHGUI_CreateKeyButton((int)(t_SX),(int)(t_SY),(int)(t_KeyWidth*2.0f+t_KeyWidth/2.0f+t_GapX),(int)(t_KeyHeight),"Enter",bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3494>";
		for(int t_C=0;t_C<=108;t_C=t_C+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3495>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C].f_Visible=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3497>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_Visible=0;
		bb_std_lang.popErr();
		return 0;
	}
	static bb_challengergui_CHGUI bb_challengergui_CHGUI_MsgBoxWindow;
	static public bb_challengergui_CHGUI bb_challengergui_CreateLabel(int t_X,int t_Y,String t_Text,bb_challengergui_CHGUI t_Parent){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<678>";
		if(bb_challengergui.bb_challengergui_CHGUI_Started==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<679>";
			bb_challengergui.bb_challengergui_CHGUI_Started=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<680>";
			bb_challengergui.bb_challengergui_CHGUI_Start();
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<682>";
		bb_challengergui_CHGUI t_N=(new bb_challengergui_CHGUI()).g_new();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<683>";
		t_N.f_Parent=t_Parent;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<684>";
		if(t_Parent==null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<684>";
			t_N.f_Parent=bb_challengergui.bb_challengergui_CHGUI_Canvas;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<685>";
		t_N.f_X=(float)(t_X);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<686>";
		t_N.f_Y=(float)(t_Y);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<687>";
		t_N.f_Text=t_Text;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<689>";
		t_N.f_Element="Label";
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<691>";
		t_N.f_Parent.f_Labels=(bb_challengergui_CHGUI[])bb_std_lang.resizeArray(t_N.f_Parent.f_Labels,bb_std_lang.arrayLength(t_N.f_Parent.f_Labels)+1);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<692>";
		t_N.f_Parent.f_Labels[bb_std_lang.arrayLength(t_N.f_Parent.f_Labels)-1]=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<693>";
		bb_std_lang.popErr();
		return t_N;
	}
	static bb_challengergui_CHGUI bb_challengergui_CHGUI_MsgBoxLabel;
	static public bb_challengergui_CHGUI bb_challengergui_CreateButton(int t_X,int t_Y,int t_W,int t_H,String t_Text,bb_challengergui_CHGUI t_Parent){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<431>";
		if(bb_challengergui.bb_challengergui_CHGUI_Started==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<432>";
			bb_challengergui.bb_challengergui_CHGUI_Started=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<433>";
			bb_challengergui.bb_challengergui_CHGUI_Start();
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<435>";
		bb_challengergui_CHGUI t_N=(new bb_challengergui_CHGUI()).g_new();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<436>";
		t_N.f_Parent=t_Parent;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<437>";
		if(t_Parent==null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<437>";
			t_N.f_Parent=bb_challengergui.bb_challengergui_CHGUI_Canvas;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<438>";
		t_N.f_X=(float)(t_X);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<439>";
		t_N.f_Y=(float)(t_Y);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<440>";
		t_N.f_W=(float)(t_W);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<441>";
		t_N.f_H=(float)(t_H);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<442>";
		t_N.f_Text=t_Text;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<443>";
		t_N.f_Element="Button";
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<445>";
		t_N.f_Parent.f_Buttons=(bb_challengergui_CHGUI[])bb_std_lang.resizeArray(t_N.f_Parent.f_Buttons,bb_std_lang.arrayLength(t_N.f_Parent.f_Buttons)+1);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<446>";
		t_N.f_Parent.f_Buttons[bb_std_lang.arrayLength(t_N.f_Parent.f_Buttons)-1]=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<447>";
		bb_std_lang.popErr();
		return t_N;
	}
	static bb_challengergui_CHGUI bb_challengergui_CHGUI_MsgBoxButton;
	static public int bb_challengergui_CHGUI_Start(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3180>";
		if(bb_challengergui.bb_challengergui_CHGUI_Width==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3180>";
			bb_challengergui.bb_challengergui_CHGUI_Width=bb_graphics.bb_graphics_DeviceWidth();
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3181>";
		if(bb_challengergui.bb_challengergui_CHGUI_Height==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3181>";
			bb_challengergui.bb_challengergui_CHGUI_Height=bb_graphics.bb_graphics_DeviceHeight();
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3182>";
		bb_challengergui.bb_challengergui_CHGUI_CanvasFlag=1;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3183>";
		bb_challengergui.bb_challengergui_CHGUI_Canvas=bb_challengergui.bb_challengergui_CreateWindow(0,0,bb_challengergui.bb_challengergui_CHGUI_Width,bb_challengergui.bb_challengergui_CHGUI_Height,"",0,0,0,0,null);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3184>";
		bb_challengergui.bb_challengergui_CHGUI_CanvasFlag=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3185>";
		if(bb_challengergui.bb_challengergui_CHGUI_Style==null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3185>";
			bb_challengergui.bb_challengergui_CHGUI_Style=bb_graphics.bb_graphics_LoadImage("GUI_mac.png",1,bb_graphics_Image.g_DefaultFlags);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3186>";
		bb_challengergui.bb_challengergui_CHGUI_ShadowImg=bb_graphics.bb_graphics_LoadImage("Shadow.png",1,bb_graphics_Image.g_DefaultFlags);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3187>";
		if(bb_challengergui.bb_challengergui_CHGUI_MobileMode==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3188>";
			if(bb_challengergui.bb_challengergui_CHGUI_TitleFont==null){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3188>";
				bb_challengergui.bb_challengergui_CHGUI_TitleFont=bb_bitmapfont_BitmapFont.g_Load("Arial10B.txt",true);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3189>";
			if(bb_challengergui.bb_challengergui_CHGUI_Font==null){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3189>";
				bb_challengergui.bb_challengergui_CHGUI_Font=bb_bitmapfont_BitmapFont.g_Load("Arial12.txt",true);
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3191>";
			if(bb_challengergui.bb_challengergui_CHGUI_TitleFont==null){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3191>";
				bb_challengergui.bb_challengergui_CHGUI_TitleFont=bb_bitmapfont_BitmapFont.g_Load("Arial20B.txt",true);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3192>";
			if(bb_challengergui.bb_challengergui_CHGUI_Font==null){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3192>";
				bb_challengergui.bb_challengergui_CHGUI_Font=bb_bitmapfont_BitmapFont.g_Load("Arial22.txt",true);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3193>";
			bb_challengergui.bb_challengergui_CHGUI_TitleHeight=50.0f;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3195>";
		bb_challengergui.bb_challengergui_CHGUI_CreateKeyboard();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3196>";
		bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow=bb_challengergui.bb_challengergui_CreateWindow(100,100,200,100,"Message box",0,0,0,2,null);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3197>";
		bb_challengergui.bb_challengergui_CHGUI_MsgBoxLabel=bb_challengergui.bb_challengergui_CreateLabel(100,50,"Message text",bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3198>";
		bb_challengergui.bb_challengergui_CHGUI_MsgBoxButton=bb_challengergui.bb_challengergui_CreateButton(150,70,100,25,"Ok",bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3199>";
		bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow.f_Visible=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3200>";
		bb_challengergui.bb_challengergui_CHGUI_TooltipFont=bb_bitmapfont_BitmapFont.g_Load("Arial10.txt",true);
		bb_std_lang.popErr();
		return 0;
	}
	static public bb_challengergui_CHGUI bb_challengergui_CreateDropdown(int t_X,int t_Y,int t_W,int t_H,String t_Text,bb_challengergui_CHGUI t_Parent){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<494>";
		if(bb_challengergui.bb_challengergui_CHGUI_Started==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<495>";
			bb_challengergui.bb_challengergui_CHGUI_Started=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<496>";
			bb_challengergui.bb_challengergui_CHGUI_Start();
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<498>";
		bb_challengergui_CHGUI t_N=(new bb_challengergui_CHGUI()).g_new();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<499>";
		t_N.f_Parent=t_Parent;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<500>";
		if(t_N.f_Parent==null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<500>";
			t_N.f_Parent=bb_challengergui.bb_challengergui_CHGUI_Canvas;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<501>";
		t_N.f_X=(float)(t_X);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<502>";
		t_N.f_Y=(float)(t_Y);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<503>";
		t_N.f_H=(float)(t_H);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<504>";
		t_N.f_W=(float)(t_W);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<505>";
		t_N.f_Text=t_Text;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<506>";
		t_N.f_Element="Dropdown";
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<508>";
		t_N.f_Parent.f_Dropdowns=(bb_challengergui_CHGUI[])bb_std_lang.resizeArray(t_N.f_Parent.f_Dropdowns,bb_std_lang.arrayLength(t_N.f_Parent.f_Dropdowns)+1);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<509>";
		t_N.f_Parent.f_Dropdowns[bb_std_lang.arrayLength(t_N.f_Parent.f_Dropdowns)-1]=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<510>";
		bb_std_lang.popErr();
		return t_N;
	}
	static public bb_challengergui_CHGUI bb_challengergui_CreateTextfield(int t_X,int t_Y,int t_W,int t_H,String t_Text,bb_challengergui_CHGUI t_Parent){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<634>";
		if(bb_challengergui.bb_challengergui_CHGUI_Started==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<635>";
			bb_challengergui.bb_challengergui_CHGUI_Started=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<636>";
			bb_challengergui.bb_challengergui_CHGUI_Start();
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<638>";
		bb_challengergui_CHGUI t_N=(new bb_challengergui_CHGUI()).g_new();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<639>";
		t_N.f_Parent=t_Parent;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<640>";
		if(t_Parent==null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<640>";
			t_N.f_Parent=bb_challengergui.bb_challengergui_CHGUI_Canvas;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<641>";
		t_N.f_X=(float)(t_X);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<642>";
		t_N.f_Y=(float)(t_Y);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<643>";
		t_N.f_W=(float)(t_W);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<644>";
		t_N.f_H=(float)(t_H);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<645>";
		t_N.f_Text=t_Text;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<646>";
		t_N.f_Element="Textfield";
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<648>";
		t_N.f_Parent.f_Textfields=(bb_challengergui_CHGUI[])bb_std_lang.resizeArray(t_N.f_Parent.f_Textfields,bb_std_lang.arrayLength(t_N.f_Parent.f_Textfields)+1);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<649>";
		t_N.f_Parent.f_Textfields[bb_std_lang.arrayLength(t_N.f_Parent.f_Textfields)-1]=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<650>";
		bb_std_lang.popErr();
		return t_N;
	}
	static int bb_challengergui_CHGUI_MouseBusy;
	static int bb_challengergui_CHGUI_Over;
	static int bb_challengergui_CHGUI_OverFlag;
	static int bb_challengergui_CHGUI_DownFlag;
	static int bb_challengergui_CHGUI_MenuOver;
	static int bb_challengergui_CHGUI_TextBoxOver;
	static int bb_challengergui_CHGUI_TextboxOnFocus;
	static int bb_challengergui_CHGUI_TextBoxDown;
	static int bb_challengergui_CHGUI_DragOver;
	static int bb_challengergui_CHGUI_Moving;
	static float bb_challengergui_CHGUI_TargetY;
	static float bb_challengergui_CHGUI_TargetX;
	static int bb_challengergui_CHGUI_IgnoreMouse;
	static public int bb_challengergui_CHGUI_ReorderSubWindows(bb_challengergui_CHGUI t_Top){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3984>";
		if(t_Top.f_Parent.f_TopVari!=t_Top && t_Top.f_Mode==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3985>";
			int t_N=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3986>";
			for(t_N=0;t_N<=bb_std_lang.arrayLength(t_Top.f_Parent.f_VariList)-1;t_N=t_N+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3987>";
				if(t_Top.f_Parent.f_VariList[t_N]==t_Top){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3987>";
					break;
				}
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3989>";
			for(int t_NN=t_N;t_NN<=bb_std_lang.arrayLength(t_Top.f_Parent.f_VariList)-2;t_NN=t_NN+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3990>";
				t_Top.f_Parent.f_VariList[t_NN]=t_Top.f_Parent.f_VariList[t_NN+1];
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3992>";
			t_Top.f_Parent.f_VariList[bb_std_lang.arrayLength(t_Top.f_Parent.f_VariList)-1]=t_Top;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3993>";
			t_Top.f_Parent.f_TopVari=t_Top;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3996>";
		if(t_Top.f_Parent.f_TopTop!=t_Top && t_Top.f_Mode==2){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3997>";
			int t_N2=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3998>";
			for(t_N2=0;t_N2<=bb_std_lang.arrayLength(t_Top.f_Parent.f_TopList)-1;t_N2=t_N2+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3999>";
				if(t_Top.f_Parent.f_TopList[t_N2]==t_Top){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3999>";
					break;
				}
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4001>";
			for(int t_NN2=t_N2;t_NN2<=bb_std_lang.arrayLength(t_Top.f_Parent.f_TopList)-2;t_NN2=t_NN2+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4002>";
				t_Top.f_Parent.f_TopList[t_NN2]=t_Top.f_Parent.f_TopList[t_NN2+1];
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4004>";
			t_Top.f_Parent.f_TopList[bb_std_lang.arrayLength(t_Top.f_Parent.f_TopList)-1]=t_Top;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4005>";
			t_Top.f_Parent.f_TopTop=t_Top;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4008>";
		if(t_Top.f_Parent.f_TopBottom!=t_Top && t_Top.f_Mode==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4009>";
			int t_N3=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4010>";
			for(t_N3=0;t_N3<=bb_std_lang.arrayLength(t_Top.f_Parent.f_BottomList)-1;t_N3=t_N3+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4011>";
				if(t_Top.f_Parent.f_BottomList[t_N3]==t_Top){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4011>";
					break;
				}
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4013>";
			for(int t_NN3=t_N3;t_NN3<=bb_std_lang.arrayLength(t_Top.f_Parent.f_BottomList)-2;t_NN3=t_NN3+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4014>";
				t_Top.f_Parent.f_BottomList[t_NN3]=t_Top.f_Parent.f_BottomList[t_NN3+1];
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4016>";
			t_Top.f_Parent.f_BottomList[bb_std_lang.arrayLength(t_Top.f_Parent.f_BottomList)-1]=t_Top;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4017>";
			t_Top.f_Parent.f_TopBottom=t_Top;
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_Reorder(bb_challengergui_CHGUI t_Top){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3979>";
		if(t_Top.f_Mode==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3979>";
			bb_challengergui.bb_challengergui_CHGUI_ReorderSubWindows(t_Top);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3980>";
		if(t_Top.f_Mode==2){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3980>";
			bb_challengergui.bb_challengergui_CHGUI_ReorderSubWindows(t_Top);
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_CloseMenu(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1614>";
		t_N.f_OnFocus=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1615>";
		bb_challengergui_CHGUI t_E=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1616>";
		do{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1617>";
			if(t_E.f_Parent.f_Element.compareTo("Window")==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1617>";
				t_E.f_Parent.f_MenuActive=null;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1617>";
				break;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1618>";
			if(t_E.f_Parent.f_Element.compareTo("MenuItem")==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1618>";
				t_E.f_Parent.f_OnFocus=0;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1619>";
			if(t_E.f_Parent.f_Element.compareTo("Menu")==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1619>";
				t_E.f_Parent.f_OnFocus=0;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1621>";
			t_E=t_E.f_Parent;
		}while(!(false));
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_CloseMenuReverse(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1626>";
		t_N.f_OnFocus=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1627>";
		bb_challengergui_CHGUI t_E=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1628>";
		int t_C=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1629>";
		for(t_C=0;t_C<=bb_std_lang.arrayLength(t_E.f_MenuItems)-1;t_C=t_C+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1630>";
			if(bb_std_lang.arrayLength(t_E.f_MenuItems)>0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1631>";
				bb_challengergui.bb_challengergui_CHGUI_CloseMenuReverse(t_E.f_MenuItems[t_C]);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1632>";
				t_E.f_MenuItems[t_C].f_OnFocus=0;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1634>";
				break;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static int bb_challengergui_CHGUI_Tooltips;
	static int bb_challengergui_CHGUI_TooltipTime;
	static int bb_challengergui_CHGUI_MenuClose;
	static public int bb_challengergui_CHGUI_UpdateMenuItem(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1566>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1567>";
		if((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N))!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1567>";
			t_N.m_CheckDown();
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1568>";
		if(bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N)==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1568>";
			bb_challengergui.bb_challengergui_CHGUI_CloseMenu(t_N);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1570>";
		if((t_N.f_Over)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1570>";
			bb_challengergui.bb_challengergui_CHGUI_MenuOver=1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1572>";
		if(((t_N.f_IsMenuParent)!=0) && ((t_N.f_Over)!=0) && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N))!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1573>";
			t_N.f_OnFocus=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1574>";
			if(t_N.f_Parent.f_MenuOver!=null && t_N!=t_N.f_Parent.f_MenuOver){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1575>";
				t_N.f_Parent.f_MenuOver.f_OnFocus=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1576>";
				bb_challengergui.bb_challengergui_CHGUI_CloseMenuReverse(t_N.f_Parent.f_MenuOver);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1578>";
			t_N.f_Parent.f_MenuOver=t_N;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1581>";
		if(t_N.f_Parent.f_MenuOver!=null && ((t_N.f_Over)!=0) && t_N!=t_N.f_Parent.f_MenuOver){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1581>";
			t_N.f_Parent.f_MenuOver.f_OnFocus=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1583>";
		if((t_N.f_Clicked)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1584>";
			if(t_N.f_IsMenuParent==0 && t_N.f_Tick==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1585>";
				bb_challengergui.bb_challengergui_CHGUI_CloseMenu(t_N);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1587>";
			if((t_N.f_Tick)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1588>";
				if(t_N.f_Value==0.0f){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1589>";
					t_N.f_Value=1.0f;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1591>";
					t_N.f_Value=0.0f;
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1596>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1597>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1597>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1599>";
		if((bb_challengergui.bb_challengergui_CHGUI_MenuClose)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1599>";
			bb_challengergui.bb_challengergui_CHGUI_CloseMenu(t_N);
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateSubMenu(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1603>";
		int t_XX=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1604>";
		for(t_XX=bb_std_lang.arrayLength(t_N.f_MenuItems)-1;t_XX>=0;t_XX=t_XX+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1605>";
			t_N.f_MenuItems[t_XX].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1606>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_MenuItems[t_XX]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_MenuItems[t_XX])==0 && ((t_N.f_OnFocus)!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1606>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateMenuItem(t_N.f_MenuItems[t_XX]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1608>";
		for(t_XX=bb_std_lang.arrayLength(t_N.f_MenuItems)-1;t_XX>=0;t_XX=t_XX+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1609>";
			if((t_N.f_MenuItems[t_XX].f_IsMenuParent)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1609>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateSubMenu(t_N.f_MenuItems[t_XX]);
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateTab(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2028>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2029>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2031>";
		if((t_N.f_Clicked)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2031>";
			t_N.f_Parent.f_CurrentTab=t_N;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2033>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2034>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2034>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateMenu(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1534>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1535>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1536>";
		if((t_N.f_Over)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1536>";
			bb_challengergui.bb_challengergui_CHGUI_MenuOver=1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1538>";
		if((t_N.f_Clicked)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1539>";
			if(t_N.f_Parent.f_MenuActive!=t_N){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1540>";
				if(bb_std_lang.arrayLength(t_N.f_MenuItems)>0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1541>";
					t_N.f_OnFocus=1;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1542>";
					t_N.f_Parent.f_MenuActive=t_N;
				}
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1545>";
				t_N.f_OnFocus=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1546>";
				bb_challengergui.bb_challengergui_CHGUI_CloseMenuReverse(t_N.f_Parent.f_MenuActive);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1547>";
				t_N.f_Parent.f_MenuActive=null;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1552>";
		if(((t_N.f_OnFocus)!=0) && bb_std_lang.arrayLength(t_N.f_MenuItems)<1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1552>";
			t_N.f_OnFocus=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1553>";
		if(((t_N.f_Over)!=0) && t_N.f_Parent.f_MenuActive!=null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1554>";
			t_N.f_Parent.f_MenuActive.f_OnFocus=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1555>";
			bb_challengergui.bb_challengergui_CHGUI_CloseMenuReverse(t_N.f_Parent.f_MenuActive);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1556>";
			t_N.f_Parent.f_MenuActive=t_N;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1557>";
			t_N.f_OnFocus=1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1560>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1561>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1561>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateDropdownItem(bb_challengergui_CHGUI t_N,int t_C){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1506>";
		t_N.f_H=t_N.f_Parent.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1507>";
		t_N.f_W=t_N.f_Parent.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1508>";
		t_N.f_X=0.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1509>";
		t_N.f_Y=t_N.f_Parent.f_H+(float)(t_C)*t_N.f_H-(float)(t_C)-1.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1511>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1512>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1514>";
		if((t_N.f_Clicked)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1515>";
			t_N.f_Parent.f_Text=t_N.f_Text;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1516>";
			t_N.f_Parent.f_Value=t_N.f_Value;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1517>";
			t_N.f_Parent.f_OnFocus=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1520>";
		if((bb_input.bb_input_TouchDown(0))!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1521>";
			if(bb_input.bb_input_TouchX(0)<(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N.f_Parent)) || bb_input.bb_input_TouchX(0)>(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N.f_Parent))+t_N.f_W || bb_input.bb_input_TouchY(0)<(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N.f_Parent)) || bb_input.bb_input_TouchY(0)>(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N.f_Parent))+t_N.f_H*(float)(bb_std_lang.arrayLength(t_N.f_Parent.f_DropdownItems)+1)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1522>";
				t_N.f_Parent.f_OnFocus=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1523>";
				t_N.f_Parent.f_Over=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1524>";
				t_N.f_Parent.f_Down=0;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1528>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1529>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1529>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateDropdown(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1489>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1490>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1492>";
		if((t_N.f_Clicked)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1493>";
			if(t_N.f_OnFocus==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1494>";
				t_N.f_OnFocus=0;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1496>";
				t_N.f_OnFocus=1;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1500>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1501>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1501>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateLabel(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2081>";
		t_N.f_W=bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2082>";
		t_N.f_H=bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtHeight(t_N.f_Text);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2084>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2085>";
		t_N.m_CheckDown();
		bb_std_lang.popErr();
		return 0;
	}
	static bb_challengergui_CHGUI bb_challengergui_CHGUI_TextboxFocus;
	static int bb_challengergui_CHGUI_Keyboard;
	static int bb_challengergui_CHGUI_ShowKeyboard;
	static int bb_challengergui_CHGUI_AutoTextScroll;
	static public int bb_challengergui_CHGUI_GetText(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1957>";
		String t_Before=bb_std_lang.slice(t_N.f_Text,0,t_N.f_Cursor);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1958>";
		String t_After=bb_std_lang.slice(t_N.f_Text,t_N.f_Cursor);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1959>";
		int t_In=bb_input.bb_input_GetChar();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1962>";
		if(t_In>96 && t_In<123 && t_N.f_FormatText==1 && bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text)<t_N.f_W-12.0f){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1963>";
			t_Before=t_Before+String.valueOf((char)(t_In));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1964>";
			t_N.f_Cursor=t_N.f_Cursor+1;
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1967>";
			if(t_In>64 && t_In<91 && t_N.f_FormatText==1 && bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text)<t_N.f_W-12.0f){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1968>";
				t_Before=t_Before+String.valueOf((char)(t_In));
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1969>";
				t_N.f_Cursor=t_N.f_Cursor+1;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1972>";
				if(t_In>45 && t_In<58 && t_N.f_FormatNumber==1 && bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text)<t_N.f_W-12.0f){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1973>";
					if(t_In!=47 || t_N.f_FormatSymbol==1){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1974>";
						t_Before=t_Before+String.valueOf((char)(t_In));
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1975>";
						t_N.f_Cursor=t_N.f_Cursor+1;
					}
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1978>";
					if(t_In>32 && t_In<48 && t_N.f_FormatSymbol==1 && bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text)<t_N.f_W-12.0f){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1979>";
						t_Before=t_Before+String.valueOf((char)(t_In));
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1980>";
						t_N.f_Cursor=t_N.f_Cursor+1;
					}else{
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1983>";
						if(t_In>57 && t_In<65 && t_N.f_FormatSymbol==1 && bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text)<t_N.f_W-12.0f){
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1984>";
							t_Before=t_Before+String.valueOf((char)(t_In));
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1985>";
							t_N.f_Cursor=t_N.f_Cursor+1;
						}else{
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1988>";
							if(t_In>90 && t_In<97 && t_N.f_FormatSymbol==1 && bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text)<t_N.f_W-12.0f){
								bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1989>";
								t_Before=t_Before+String.valueOf((char)(t_In));
								bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1990>";
								t_N.f_Cursor=t_N.f_Cursor+1;
							}else{
								bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1993>";
								if(t_In>122 && t_In<127 && t_N.f_FormatSymbol==1 && bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text)<t_N.f_W-12.0f){
									bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1994>";
									t_Before=t_Before+String.valueOf((char)(t_In));
									bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1995>";
									t_N.f_Cursor=t_N.f_Cursor+1;
								}else{
									bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1998>";
									if(t_In==32 && t_N.f_FormatSpace==1 && bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text)<t_N.f_W-12.0f){
										bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1999>";
										t_Before=t_Before+String.valueOf((char)(t_In));
										bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2000>";
										t_N.f_Cursor=t_N.f_Cursor+1;
									}else{
										bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2002>";
										if(t_In==8){
											bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2004>";
											t_Before=bb_std_lang.slice(t_Before,0,t_Before.length()-1);
											bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2005>";
											if(t_N.f_Cursor>0){
												bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2005>";
												t_N.f_Cursor=t_N.f_Cursor-1;
											}
										}else{
											bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2007>";
											if(t_In==127){
												bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2009>";
												t_After=bb_std_lang.slice(t_After,1);
											}else{
												bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2011>";
												if(t_In==65575){
													bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2013>";
													if(t_N.f_Cursor<t_N.f_Text.length()){
														bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2013>";
														t_N.f_Cursor=t_N.f_Cursor+1;
													}
												}else{
													bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2015>";
													if(t_In==65573){
														bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2017>";
														if(t_N.f_Cursor>0){
															bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2017>";
															t_N.f_Cursor=t_N.f_Cursor-1;
														}
													}else{
														bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2019>";
														if(t_In==13){
															bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2021>";
															t_N.f_OnFocus=0;
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
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2024>";
		t_N.f_Text=t_Before+t_After;
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateKeyboardSizes(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3725>";
		float t_KeyWidth=(float)(bb_graphics.bb_graphics_DeviceWidth())/12.5f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3726>";
		float t_KeyHeight=(float)(bb_graphics.bb_graphics_DeviceWidth())/12.5f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3728>";
		float t_GapX=t_KeyWidth*2.0f/9.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3729>";
		float t_GapY=t_KeyWidth*2.0f/9.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3731>";
		if(bb_graphics.bb_graphics_DeviceWidth()>bb_graphics.bb_graphics_DeviceHeight()){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3732>";
			t_KeyHeight=t_KeyHeight/1.7f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3733>";
			t_GapY=t_GapY/1.2f;
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3735>";
			t_KeyHeight=t_KeyHeight*1.5f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3736>";
			t_GapY=t_GapY*1.2f;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3740>";
		float t_EndGap=((float)(bb_graphics.bb_graphics_DeviceWidth())-t_KeyWidth*10.0f-t_GapX*9.0f)/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3742>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_H=t_EndGap*2.0f+t_GapY*3.0f+t_KeyHeight*4.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3743>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_Y=(float)(bb_graphics.bb_graphics_DeviceHeight())-bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_H;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3744>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_W=(float)(bb_graphics.bb_graphics_DeviceWidth());
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3746>";
		float t_SX=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3747>";
		float t_SY=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3749>";
		for(int t_C=0;t_C<=9;t_C=t_C+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3750>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C].f_X=t_SX;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3751>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C].f_Y=t_SY;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3752>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C].f_W=t_KeyWidth;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3753>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C].f_H=t_KeyHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3754>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C+26].f_X=t_SX;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3755>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C+26].f_Y=t_SY;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3756>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C+26].f_W=t_KeyWidth;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3757>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C+26].f_H=t_KeyHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3758>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C+52].f_X=t_SX;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3759>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C+52].f_Y=t_SY;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3760>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C+52].f_W=t_KeyWidth;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3761>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C+52].f_H=t_KeyHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3762>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C+78].f_X=t_SX;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3763>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C+78].f_Y=t_SY;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3764>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C+78].f_W=t_KeyWidth;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3765>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C+78].f_H=t_KeyHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3766>";
			t_SX=t_SX+t_GapX+t_KeyWidth;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3769>";
		t_SX=t_EndGap+t_KeyWidth/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3770>";
		t_SY=t_SY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3772>";
		for(int t_C2=10;t_C2<=18;t_C2=t_C2+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3773>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2].f_X=t_SX;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3774>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2].f_Y=t_SY;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3775>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2].f_W=t_KeyWidth;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3776>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2].f_H=t_KeyHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3777>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2+26].f_X=t_SX;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3778>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2+26].f_Y=t_SY;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3779>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2+26].f_W=t_KeyWidth;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3780>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2+26].f_H=t_KeyHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3781>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2+52].f_X=t_SX;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3782>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2+52].f_Y=t_SY;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3783>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2+52].f_W=t_KeyWidth;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3784>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2+52].f_H=t_KeyHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3785>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2+78].f_X=t_SX;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3786>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2+78].f_Y=t_SY;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3787>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2+78].f_W=t_KeyWidth;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3788>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2+78].f_H=t_KeyHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3789>";
			t_SX=t_SX+t_GapX+t_KeyWidth;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3792>";
		t_SX=t_EndGap+t_KeyWidth/2.0f+t_GapX+t_KeyWidth;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3793>";
		t_SY=t_SY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3795>";
		for(int t_C3=19;t_C3<=25;t_C3=t_C3+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3796>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3].f_X=t_SX;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3797>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3].f_Y=t_SY;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3798>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3].f_W=t_KeyWidth;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3799>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3].f_H=t_KeyHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3800>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3+26].f_X=t_SX;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3801>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3+26].f_Y=t_SY;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3802>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3+26].f_W=t_KeyWidth;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3803>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3+26].f_H=t_KeyHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3804>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3+52].f_X=t_SX;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3805>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3+52].f_Y=t_SY;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3806>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3+52].f_W=t_KeyWidth;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3807>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3+52].f_H=t_KeyHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3808>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3+78].f_X=t_SX;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3809>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3+78].f_Y=t_SY;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3810>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3+78].f_W=t_KeyWidth;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3811>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3+78].f_H=t_KeyHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3812>";
			t_SX=t_SX+t_GapX+t_KeyWidth;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3815>";
		t_SX=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3816>";
		t_SY=t_EndGap+t_KeyHeight+t_GapY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3817>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_X=t_SX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3818>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_Y=t_SY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3819>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_W=t_KeyWidth+t_KeyWidth/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3820>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_H=t_KeyHeight;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3823>";
		t_SX=t_EndGap+t_KeyWidth*9.0f+t_GapX*9.0f-t_KeyWidth/2.0f-t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3824>";
		t_SY=t_EndGap+t_KeyHeight+t_GapY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3825>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[105].f_X=t_SX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3826>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[105].f_Y=t_SY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3827>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[105].f_W=t_KeyWidth+t_KeyWidth/2.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3828>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[105].f_H=t_KeyHeight;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3830>";
		t_SX=t_EndGap;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3831>";
		t_SY=t_SY+t_KeyHeight+t_GapY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3832>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_X=t_SX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3833>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_Y=t_SY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3834>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_W=t_KeyWidth*2.0f+t_KeyWidth/2.0f+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3835>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_H=t_KeyHeight;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3837>";
		t_SX=t_SX+t_KeyWidth*2.0f+t_KeyWidth/2.0f+t_GapX+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3838>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[107].f_X=t_SX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3839>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[107].f_Y=t_SY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3840>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[107].f_W=t_KeyWidth*5.0f+t_GapX*4.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3841>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[107].f_H=t_KeyHeight;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3843>";
		t_SX=t_SX+t_KeyWidth*5.0f+t_GapX*5.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3844>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[108].f_X=t_SX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3845>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[108].f_Y=t_SY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3846>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[108].f_W=t_KeyWidth*2.0f+t_KeyWidth/2.0f+t_GapX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3847>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[108].f_H=t_KeyHeight;
		bb_std_lang.popErr();
		return 0;
	}
	static int bb_challengergui_CHGUI_KeyboardPage;
	static int bb_challengergui_CHGUI_KeyboardShift;
	static float bb_challengergui_CHGUI_OldX;
	static float bb_challengergui_CHGUI_OldY;
	static public int bb_challengergui_CHGUI_UpdateKeyboard(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3502>";
		bb_challengergui.bb_challengergui_CHGUI_Reorder(bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3504>";
		bb_challengergui.bb_challengergui_CHGUI_UpdateKeyboardSizes();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3507>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_X=0.0f-bb_challengergui.bb_challengergui_CHGUI_OffsetX;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3508>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_Y=(float)(bb_graphics.bb_graphics_DeviceHeight())-bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_H-bb_challengergui.bb_challengergui_CHGUI_OffsetY;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3511>";
		if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage>1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3512>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_Active=0;
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3514>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_Active=1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3519>";
		for(int t_C=0;t_C<=108;t_C=t_C+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3520>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C].f_Active=1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3523>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_Active=1;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3525>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_Active=1;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3527>";
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[107].f_Active=1;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3530>";
		if(t_N.f_FormatSpace==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3530>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[107].f_Active=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3534>";
		if(t_N.f_FormatText==1 && t_N.f_FormatNumber==0 && t_N.f_FormatSymbol==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3535>";
			if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage>1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3535>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=0;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3536>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_Active=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3539>";
		if(t_N.f_FormatNumber==1 && t_N.f_FormatText==0 && t_N.f_FormatSymbol==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3540>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=2;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3541>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_Active=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3542>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_Active=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3543>";
			for(int t_C2=62;t_C2<=77;t_C2=t_C2+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3544>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C2].f_Active=0;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3548>";
		if(t_N.f_FormatNumber==0 && t_N.f_FormatText==0 && t_N.f_FormatSymbol==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3549>";
			if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage<2){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3549>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=2;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3550>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_Active=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3551>";
			for(int t_C3=52;t_C3<=61;t_C3=t_C3+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3552>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C3].f_Active=0;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3556>";
		if(t_N.f_FormatText==1 && t_N.f_FormatNumber==1 && t_N.f_FormatSymbol==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3557>";
			if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage>2){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3557>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=0;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3558>";
			for(int t_C4=62;t_C4<=77;t_C4=t_C4+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3559>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C4].f_Active=0;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3563>";
		if(t_N.f_FormatText==1 && t_N.f_FormatNumber==0 && t_N.f_FormatSymbol==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3564>";
			for(int t_C5=52;t_C5<=61;t_C5=t_C5+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3565>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C5].f_Active=0;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3569>";
		if(t_N.f_FormatText==0 && t_N.f_FormatNumber==1 && t_N.f_FormatSymbol==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3570>";
			if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage<2){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3570>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=2;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3571>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_Active=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3576>";
		if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage>1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3576>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_Active=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3583>";
		if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage<2){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3584>";
			if((bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_Clicked)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3585>";
				if(bb_challengergui.bb_challengergui_CHGUI_KeyboardShift==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3586>";
					bb_challengergui.bb_challengergui_CHGUI_KeyboardShift=1;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3587>";
					bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=1;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3588>";
					bb_challengergui.bb_challengergui_CHGUI_ShiftHold=0;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3590>";
					bb_challengergui.bb_challengergui_CHGUI_KeyboardShift=0;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3591>";
					bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=0;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3592>";
					bb_challengergui.bb_challengergui_CHGUI_ShiftHold=0;
				}
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3596>";
			if((bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_DoubleClicked)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3597>";
				bb_challengergui.bb_challengergui_CHGUI_ShiftHold=1;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3598>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardShift=1;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3599>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=1;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3604>";
		if((bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_Clicked)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3605>";
			if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3606>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=2;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3607>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_Active=0;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3608>";
				if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==1){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3609>";
					bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=2;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3610>";
					bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_Active=0;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3611>";
					if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==2){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3612>";
						bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=3;
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3613>";
						bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[104].f_Active=0;
					}else{
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3614>";
						if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==3){
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3615>";
							bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=0;
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3616>";
							bb_challengergui.bb_challengergui_CHGUI_KeyboardShift=0;
						}
					}
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3622>";
		if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==0 || bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3623>";
			if(t_N.f_FormatNumber==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3624>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_Text="123";
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3626>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_Text="#+=";
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3630>";
		if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==2){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3631>";
			if(t_N.f_FormatSymbol==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3632>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_Text="#+=";
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3634>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_Text="Abc";
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3638>";
		if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==3){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3639>";
			if(t_N.f_FormatText==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3640>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_Text="#+=";
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3641>";
				if(t_N.f_FormatNumber==1){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3642>";
					bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_Text="123";
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3644>";
					bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[106].f_Text="#+=";
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3651>";
		for(int t_C6=0;t_C6<=108;t_C6=t_C6+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3652>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C6].f_Visible=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3655>";
		if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3656>";
			for(int t_C7=0;t_C7<=25;t_C7=t_C7+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3657>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C7].f_Visible=1;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3660>";
		if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3661>";
			for(int t_C8=26;t_C8<=51;t_C8=t_C8+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3662>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C8].f_Visible=1;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3665>";
		if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==2){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3666>";
			for(int t_C9=52;t_C9<=77;t_C9=t_C9+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3667>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C9].f_Visible=1;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3670>";
		if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==3){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3671>";
			for(int t_C10=78;t_C10<=103;t_C10=t_C10+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3672>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C10].f_Visible=1;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3675>";
		for(int t_C11=104;t_C11<=108;t_C11=t_C11+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3676>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C11].f_Visible=1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3681>";
		String t_Before=bb_std_lang.slice(t_N.f_Text,0,t_N.f_Cursor);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3682>";
		String t_After=bb_std_lang.slice(t_N.f_Text,t_N.f_Cursor);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3685>";
		for(int t_C1=0;t_C1<=103;t_C1=t_C1+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3686>";
			if(((bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C1].f_Clicked)!=0) && bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text)<t_N.f_W-12.0f){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3687>";
				t_Before=t_Before+bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[t_C1].f_Text;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3688>";
				t_N.f_Cursor=t_N.f_Cursor+1;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3689>";
				if(bb_challengergui.bb_challengergui_CHGUI_ShiftHold==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3690>";
					if(bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==0 || bb_challengergui.bb_challengergui_CHGUI_KeyboardPage==1){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3691>";
						bb_challengergui.bb_challengergui_CHGUI_KeyboardShift=0;
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3691>";
						bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=0;
					}
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3697>";
		if((bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[105].f_Down)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3698>";
			if(bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[105].f_DKeyMillisecs<bb_app.bb_app_Millisecs()){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3699>";
				bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[105].f_DKeyMillisecs=bb_app.bb_app_Millisecs()+150;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3700>";
				t_Before=bb_std_lang.slice(t_Before,0,t_Before.length()-1);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3701>";
				if(t_N.f_Cursor>0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3701>";
					t_N.f_Cursor=t_N.f_Cursor-1;
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3705>";
		if(((bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[107].f_Clicked)!=0) && bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text)<t_N.f_W-12.0f){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3706>";
			t_Before=t_Before+" ";
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3707>";
			t_N.f_Cursor=t_N.f_Cursor+1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3710>";
		if((bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons[108].f_Clicked)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3711>";
			t_N.f_OnFocus=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3712>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_Visible=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3713>";
			bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3714>";
			if(bb_challengergui.bb_challengergui_CHGUI_AutoTextScroll==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3715>";
				bb_challengergui.bb_challengergui_CHGUI_TargetX=bb_challengergui.bb_challengergui_CHGUI_OldX;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3716>";
				bb_challengergui.bb_challengergui_CHGUI_TargetY=bb_challengergui.bb_challengergui_CHGUI_OldY;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3717>";
				bb_challengergui.bb_challengergui_CHGUI_Moving=1;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3721>";
		t_N.f_Text=t_Before+t_After;
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateTextfield(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1873>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1874>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1876>";
		if((t_N.f_Over)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1876>";
			bb_challengergui.bb_challengergui_CHGUI_TextBoxOver=1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1877>";
		if((t_N.f_Down)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1877>";
			bb_challengergui.bb_challengergui_CHGUI_TextBoxDown=1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1878>";
		if((t_N.f_OnFocus)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1878>";
			bb_challengergui.bb_challengergui_CHGUI_TextboxOnFocus=1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1881>";
		if((t_N.f_Clicked)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1882>";
			bb_challengergui.bb_challengergui_CHGUI_TextboxFocus=t_N;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1883>";
			if(t_N.f_OnFocus==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1884>";
				t_N.f_Cursor=t_N.f_Text.length();
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1885>";
				t_N.f_OnFocus=1;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1886>";
				if(bb_challengergui.bb_challengergui_CHGUI_Keyboard==1){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1887>";
					bb_input.bb_input_EnableKeyboard();
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1888>";
					if(bb_challengergui.bb_challengergui_CHGUI_Keyboard==2){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1889>";
						bb_challengergui.bb_challengergui_CHGUI_ShowKeyboard=1;
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1890>";
						bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_Visible=1;
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1891>";
						bb_challengergui.bb_challengergui_CHGUI_ShiftHold=0;
					}
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1894>";
				if((bb_challengergui.bb_challengergui_CHGUI_AutoTextScroll)!=0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1895>";
					bb_challengergui.bb_challengergui_CHGUI_TargetX=(float)(-bb_challengergui.bb_challengergui_CHGUI_RealX(t_N))+bb_challengergui.bb_challengergui_CHGUI_OffsetX+100.0f;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1896>";
					bb_challengergui.bb_challengergui_CHGUI_TargetY=(float)(-bb_challengergui.bb_challengergui_CHGUI_RealY(t_N))+bb_challengergui.bb_challengergui_CHGUI_OffsetY+100.0f;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1897>";
					bb_challengergui.bb_challengergui_CHGUI_Moving=1;
				}
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1903>";
				int t_C=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1904>";
				for(t_C=0;t_C<=t_N.f_Text.length()-1;t_C=t_C+1){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1905>";
					if(bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(bb_std_lang.slice(t_N.f_Text,0,t_C)+"NON")-bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2("NON")>bb_input.bb_input_TouchX(0)-(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N))-10.0f){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1905>";
						break;
					}
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1907>";
				t_N.f_Cursor=t_C;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1912>";
		if((t_N.f_OnFocus)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1913>";
			if(bb_challengergui.bb_challengergui_CHGUI_Keyboard==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1914>";
				bb_challengergui.bb_challengergui_CHGUI_GetText(t_N);
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1915>";
				if(bb_challengergui.bb_challengergui_CHGUI_Keyboard==2){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1916>";
					bb_challengergui.bb_challengergui_CHGUI_UpdateKeyboard(t_N);
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1918>";
					bb_challengergui.bb_challengergui_CHGUI_GetText(t_N);
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1927>";
		if(((bb_input.bb_input_TouchDown(0))!=0) && t_N.f_Over==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1928>";
			if(bb_challengergui.bb_challengergui_CHGUI_Keyboard==2){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1929>";
				if(bb_input.bb_input_TouchY(0)<(float)(bb_graphics.bb_graphics_DeviceHeight())-bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_H){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1930>";
					t_N.f_OnFocus=0;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1931>";
					if(bb_challengergui.bb_challengergui_CHGUI_Keyboard==1){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1932>";
						bb_input.bb_input_DisableKeyboard();
					}else{
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1933>";
						if(bb_challengergui.bb_challengergui_CHGUI_Keyboard==2){
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1934>";
							bb_challengergui.bb_challengergui_CHGUI_ShowKeyboard=0;
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1935>";
							bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_Visible=0;
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1936>";
							bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=0;
						}
					}
				}
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1940>";
				t_N.f_OnFocus=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1941>";
				if(bb_challengergui.bb_challengergui_CHGUI_Keyboard==1){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1942>";
					bb_input.bb_input_DisableKeyboard();
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1943>";
					if(bb_challengergui.bb_challengergui_CHGUI_Keyboard==2){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1944>";
						bb_challengergui.bb_challengergui_CHGUI_ShowKeyboard=0;
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1945>";
						bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_Visible=0;
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1946>";
						bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=0;
					}
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1951>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1952>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1952>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateHSlider(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1640>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1641>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1642>";
		if(t_N.f_Value<t_N.f_Minimum){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1642>";
			t_N.f_Value=t_N.f_Minimum;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1643>";
		if(t_N.f_Value>t_N.f_Maximum){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1643>";
			t_N.f_Value=t_N.f_Maximum;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1644>";
		int t_X=bb_challengergui.bb_challengergui_CHGUI_RealX(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1645>";
		int t_Y=bb_challengergui.bb_challengergui_CHGUI_RealY(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1646>";
		int t_W=(int)(t_N.f_W);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1647>";
		int t_H=(int)(t_N.f_H);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1650>";
		t_N.f_Stp=(t_N.f_W-2.0f*t_N.f_H)/(t_N.f_Maximum-t_N.f_Minimum);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1651>";
		t_N.f_SWidth=t_N.f_Stp;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1652>";
		if(t_N.f_SWidth<t_N.f_H){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1652>";
			t_N.f_SWidth=t_N.f_H;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1653>";
		if(t_N.f_SWidth>t_N.f_W-t_N.f_H-t_N.f_H-10.0f){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1653>";
			t_N.f_SWidth=t_N.f_W-t_N.f_H-t_N.f_H-10.0f;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1654>";
		t_N.f_Stp=(t_N.f_W-t_N.f_SWidth-t_N.f_H-t_N.f_H)/(t_N.f_Maximum-t_N.f_Minimum);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1655>";
		t_N.f_SWidth=t_N.f_SWidth+10.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1658>";
		if(((t_N.f_MinusOver)!=0) && ((t_N.f_MinusDown)!=0) && bb_input.bb_input_TouchDown(0)==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1659>";
			t_N.f_MinusOver=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1660>";
			t_N.f_MinusDown=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1661>";
			t_N.f_Value=t_N.f_Value-1.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1662>";
			if(t_N.f_Value<t_N.f_Minimum){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1662>";
				t_N.f_Value=t_N.f_Minimum;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1666>";
		if(((t_N.f_Over)!=0) && bb_input.bb_input_TouchX(0)>(float)(t_X) && bb_input.bb_input_TouchX(0)<(float)(t_X+t_H) && bb_input.bb_input_TouchY(0)>(float)(t_Y) && bb_input.bb_input_TouchY(0)<(float)(t_Y+t_H)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1667>";
			if(bb_challengergui.bb_challengergui_CHGUI_MouseBusy==0 || ((t_N.f_MinusDown)!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1668>";
				t_N.f_MinusOver=1;
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1671>";
			t_N.f_MinusOver=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1675>";
		if(((t_N.f_MinusOver)!=0) || ((t_N.f_MinusDown)!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1676>";
			if((bb_input.bb_input_TouchDown(0))!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1677>";
				t_N.f_MinusDown=1;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1679>";
				t_N.f_MinusDown=0;
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1682>";
			t_N.f_MinusDown=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1686>";
		if(((t_N.f_PlusOver)!=0) && ((t_N.f_PlusDown)!=0) && bb_input.bb_input_TouchDown(0)==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1687>";
			t_N.f_PlusOver=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1688>";
			t_N.f_PlusDown=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1689>";
			t_N.f_Value=t_N.f_Value+1.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1690>";
			if(t_N.f_Value>t_N.f_Maximum){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1690>";
				t_N.f_Value=t_N.f_Maximum;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1694>";
		if(bb_input.bb_input_TouchX(0)>(float)(t_X+t_W-t_H) && bb_input.bb_input_TouchX(0)<(float)(t_X+t_W) && bb_input.bb_input_TouchY(0)>(float)(t_Y) && bb_input.bb_input_TouchY(0)<(float)(t_Y+t_H)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1695>";
			if(bb_challengergui.bb_challengergui_CHGUI_MouseBusy==0 || ((t_N.f_PlusDown)!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1696>";
				t_N.f_PlusOver=1;
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1699>";
			t_N.f_PlusOver=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1703>";
		if(((t_N.f_PlusOver)!=0) || ((t_N.f_PlusDown)!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1704>";
			if((bb_input.bb_input_TouchDown(0))!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1705>";
				t_N.f_PlusDown=1;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1707>";
				t_N.f_PlusDown=0;
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1710>";
			t_N.f_PlusDown=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1714>";
		float t_XPOS=(float)(t_X+t_H-5)+(t_N.f_Value-t_N.f_Minimum)*t_N.f_Stp;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1716>";
		if(((t_N.f_Over)!=0) && bb_input.bb_input_TouchX(0)>t_XPOS && bb_input.bb_input_TouchX(0)<t_XPOS+t_N.f_SWidth && t_N.f_PlusOver==0 && t_N.f_MinusOver==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1717>";
			if(bb_challengergui.bb_challengergui_CHGUI_MouseBusy==0 || ((t_N.f_SliderDown)!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1718>";
				t_N.f_SliderOver=1;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1719>";
				if(t_N.f_SliderDown==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1719>";
					t_N.f_Start=(int)(bb_input.bb_input_TouchX(0));
				}
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1722>";
			t_N.f_SliderOver=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1725>";
		if(((t_N.f_SliderOver)!=0) || ((t_N.f_SliderDown)!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1726>";
			if((bb_input.bb_input_TouchDown(0))!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1727>";
				t_N.f_SliderDown=1;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1729>";
				t_N.f_SliderDown=0;
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1732>";
			t_N.f_SliderDown=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1735>";
		if((t_N.f_SliderDown)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1736>";
			float t_Change=bb_input.bb_input_TouchX(0)-(float)(t_N.f_Start);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1737>";
			t_N.f_Value=t_N.f_Value+t_Change/t_N.f_Stp;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1738>";
			t_N.f_Start=(int)(bb_input.bb_input_TouchX(0));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1739>";
			if(t_N.f_Value<t_N.f_Minimum){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1739>";
				t_N.f_Value=t_N.f_Minimum;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1740>";
			if(t_N.f_Value>t_N.f_Maximum){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1740>";
				t_N.f_Value=t_N.f_Maximum;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1743>";
		if(t_N.f_SliderDown==0 && t_N.f_MinusDown==0 && t_N.f_PlusDown==0 && ((t_N.f_Down)!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1744>";
			t_N.f_Value=(bb_input.bb_input_TouchX(0)-(float)(t_X)-(float)(t_H)-(float)(t_H)+10.0f)/t_N.f_Stp+t_N.f_Minimum;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1745>";
			if(t_N.f_Value<t_N.f_Minimum){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1745>";
				t_N.f_Value=t_N.f_Minimum;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1746>";
			if(t_N.f_Value>t_N.f_Maximum){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1746>";
				t_N.f_Value=t_N.f_Maximum;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1749>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1750>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1750>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateVSlider(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1756>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1757>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1758>";
		if(t_N.f_Value<t_N.f_Minimum){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1758>";
			t_N.f_Value=t_N.f_Minimum;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1759>";
		if(t_N.f_Value>t_N.f_Maximum){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1759>";
			t_N.f_Value=t_N.f_Maximum;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1760>";
		int t_X=bb_challengergui.bb_challengergui_CHGUI_RealX(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1761>";
		int t_Y=bb_challengergui.bb_challengergui_CHGUI_RealY(t_N);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1762>";
		int t_W=(int)(t_N.f_W);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1763>";
		int t_H=(int)(t_N.f_H);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1766>";
		t_N.f_Stp=(t_N.f_H-2.0f*t_N.f_W)/(t_N.f_Maximum-t_N.f_Minimum);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1767>";
		t_N.f_SWidth=t_N.f_Stp;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1768>";
		if(t_N.f_SWidth<t_N.f_W){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1768>";
			t_N.f_SWidth=t_N.f_W;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1769>";
		if(t_N.f_SWidth>t_N.f_H-t_N.f_W-t_N.f_W-10.0f){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1769>";
			t_N.f_SWidth=t_N.f_H-t_N.f_W-t_N.f_W-10.0f;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1770>";
		t_N.f_Stp=(t_N.f_H-t_N.f_SWidth-t_N.f_W-t_N.f_W)/(t_N.f_Maximum-t_N.f_Minimum);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1771>";
		t_N.f_SWidth=t_N.f_SWidth+10.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1775>";
		if(((t_N.f_MinusOver)!=0) && ((t_N.f_MinusDown)!=0) && bb_input.bb_input_TouchDown(0)==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1776>";
			t_N.f_MinusOver=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1777>";
			t_N.f_MinusDown=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1778>";
			t_N.f_Value=t_N.f_Value-1.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1779>";
			if(t_N.f_Value<t_N.f_Minimum){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1779>";
				t_N.f_Value=t_N.f_Minimum;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1783>";
		if(bb_input.bb_input_TouchX(0)>(float)(t_X) && bb_input.bb_input_TouchX(0)<(float)(t_X+t_W) && bb_input.bb_input_TouchY(0)>(float)(t_Y) && bb_input.bb_input_TouchY(0)<(float)(t_Y+t_W)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1784>";
			if(bb_challengergui.bb_challengergui_CHGUI_MouseBusy==0 || ((t_N.f_MinusDown)!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1785>";
				t_N.f_MinusOver=1;
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1788>";
			t_N.f_MinusOver=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1792>";
		if(((t_N.f_MinusOver)!=0) || ((t_N.f_MinusDown)!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1793>";
			if((bb_input.bb_input_TouchDown(0))!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1794>";
				t_N.f_MinusDown=1;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1796>";
				t_N.f_MinusDown=0;
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1799>";
			t_N.f_MinusDown=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1803>";
		if(((t_N.f_PlusOver)!=0) && ((t_N.f_PlusDown)!=0) && bb_input.bb_input_TouchDown(0)==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1804>";
			t_N.f_PlusOver=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1805>";
			t_N.f_PlusDown=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1806>";
			t_N.f_Value=t_N.f_Value+1.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1807>";
			if(t_N.f_Value>t_N.f_Maximum){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1807>";
				t_N.f_Value=t_N.f_Maximum;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1811>";
		if(((t_N.f_Over)!=0) && bb_input.bb_input_TouchX(0)>(float)(t_X) && bb_input.bb_input_TouchX(0)<(float)(t_X+t_W) && bb_input.bb_input_TouchY(0)>(float)(t_Y+t_H-t_W) && bb_input.bb_input_TouchY(0)<(float)(t_Y+t_H)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1812>";
			if(bb_challengergui.bb_challengergui_CHGUI_MouseBusy==0 || ((t_N.f_PlusDown)!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1813>";
				t_N.f_PlusOver=1;
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1816>";
			t_N.f_PlusOver=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1820>";
		if(((t_N.f_PlusOver)!=0) || ((t_N.f_PlusDown)!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1821>";
			if((bb_input.bb_input_TouchDown(0))!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1822>";
				t_N.f_PlusDown=1;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1824>";
				t_N.f_PlusDown=0;
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1827>";
			t_N.f_PlusDown=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1831>";
		float t_YPOS=(float)(t_Y+t_W-5)+(t_N.f_Value-t_N.f_Minimum)*t_N.f_Stp;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1833>";
		if(((t_N.f_Over)!=0) && bb_input.bb_input_TouchY(0)>t_YPOS && bb_input.bb_input_TouchY(0)<t_YPOS+t_N.f_SWidth && t_N.f_PlusOver==0 && t_N.f_MinusOver==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1834>";
			if(bb_challengergui.bb_challengergui_CHGUI_MouseBusy==0 || ((t_N.f_SliderDown)!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1835>";
				t_N.f_SliderOver=1;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1836>";
				if(t_N.f_SliderDown==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1836>";
					t_N.f_Start=(int)(bb_input.bb_input_TouchY(0));
				}
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1839>";
			t_N.f_SliderOver=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1842>";
		if(((t_N.f_SliderOver)!=0) || ((t_N.f_SliderDown)!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1843>";
			if((bb_input.bb_input_TouchDown(0))!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1844>";
				t_N.f_SliderDown=1;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1846>";
				t_N.f_SliderDown=0;
			}
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1849>";
			t_N.f_SliderDown=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1852>";
		if((t_N.f_SliderDown)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1853>";
			float t_Change=bb_input.bb_input_TouchY(0)-(float)(t_N.f_Start);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1854>";
			t_N.f_Value=t_N.f_Value+t_Change/t_N.f_Stp;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1855>";
			t_N.f_Start=(int)(bb_input.bb_input_TouchY(0));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1856>";
			if(t_N.f_Value<t_N.f_Minimum){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1856>";
				t_N.f_Value=t_N.f_Minimum;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1857>";
			if(t_N.f_Value>t_N.f_Maximum){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1857>";
				t_N.f_Value=t_N.f_Maximum;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1860>";
		if(t_N.f_SliderDown==0 && t_N.f_MinusDown==0 && t_N.f_PlusDown==0 && ((t_N.f_Down)!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1861>";
			t_N.f_Value=(bb_input.bb_input_TouchY(0)-(float)(t_Y)-(float)(t_W)-(float)(t_W)+10.0f)/t_N.f_Stp+t_N.f_Minimum;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1862>";
			if(t_N.f_Value<t_N.f_Minimum){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1862>";
				t_N.f_Value=t_N.f_Minimum;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1863>";
			if(t_N.f_Value>t_N.f_Maximum){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1863>";
				t_N.f_Value=t_N.f_Maximum;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1866>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1867>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1867>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateListboxItem(bb_challengergui_CHGUI t_N,int t_C){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2060>";
		t_N.f_X=0.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2061>";
		t_N.f_Y=(float)(t_C*t_N.f_Parent.f_ListHeight);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2062>";
		t_N.f_W=t_N.f_Parent.f_W;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2063>";
		t_N.f_H=(float)(t_N.f_Parent.f_ListHeight);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2065>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2066>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2068>";
		if((t_N.f_Clicked)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2069>";
			t_N.f_Parent.f_Text=t_N.f_Text;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2070>";
			t_N.f_Parent.f_SelectedListboxItem=t_N;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2071>";
			t_N.f_Parent.f_Value=t_N.f_Value;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2074>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2075>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2075>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateListbox(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2039>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2040>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2042>";
		t_N.f_ListHeight=bb_challengergui.bb_challengergui_CHGUI_Font.m_GetFontHeight()+10;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2043>";
		t_N.f_ListboxNumber=(int)(t_N.f_H/(float)(t_N.f_ListHeight)-1.0f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2045>";
		t_N.f_ListboxSlider.f_Minimum=0.0f;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2046>";
		t_N.f_ListboxSlider.f_Maximum=(float)(bb_std_lang.arrayLength(t_N.f_ListboxItems)-t_N.f_ListboxNumber-1);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2048>";
		if(t_N.f_ListboxSlider.f_Maximum<1.0f){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2049>";
			t_N.f_ListboxSlider.f_Visible=0;
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2051>";
			t_N.f_ListboxSlider.f_Visible=1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2054>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2055>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2055>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateRadiobox(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1471>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1472>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1473>";
		t_N.f_W=t_N.f_H+t_N.f_H/4.0f+bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1474>";
		if((t_N.f_Clicked)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1475>";
			t_N.f_Value=1.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1476>";
			for(int t_X=0;t_X<=bb_std_lang.arrayLength(t_N.f_Parent.f_Radioboxes)-1;t_X=t_X+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1477>";
				if(t_N.f_Parent.f_Radioboxes[t_X].f_Group==t_N.f_Group && t_N.f_Parent.f_Radioboxes[t_X]!=t_N){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1478>";
					t_N.f_Parent.f_Radioboxes[t_X].f_Value=0.0f;
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1483>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1484>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1484>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateTickbox(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1452>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1453>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1454>";
		t_N.f_W=t_N.f_H+t_N.f_H/4.0f+bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(t_N.f_Text);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1455>";
		if((t_N.f_Clicked)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1456>";
			if(t_N.f_Value==0.0f){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1457>";
				t_N.f_Value=1.0f;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1459>";
				t_N.f_Value=0.0f;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1464>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1465>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1465>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateImageButton(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1441>";
		t_N.f_W=(float)(t_N.f_Img.m_Width()/4);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1442>";
		t_N.f_H=(float)(t_N.f_Img.m_Height());
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1443>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1444>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1446>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1447>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1447>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateButton(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1432>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1433>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1435>";
		if(bb_challengergui.bb_challengergui_CHGUI_Tooltips==1 && (t_N.f_Tooltip.compareTo("")!=0) && t_N.f_OverTime>bb_challengergui.bb_challengergui_CHGUI_TooltipTime){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1436>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1436>";
				bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=t_N;
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_Locked(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4032>";
		bb_challengergui_CHGUI t_E=null;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4033>";
		t_E=t_N;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4034>";
		if(t_E==bb_challengergui.bb_challengergui_CHGUI_LockedWIndow){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4034>";
			bb_std_lang.popErr();
			return 1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4035>";
		do{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4036>";
			if(t_E.f_Parent!=null){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4037>";
				if(t_E.f_Parent==bb_challengergui.bb_challengergui_CHGUI_LockedWIndow){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4037>";
					bb_std_lang.popErr();
					return 1;
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4038>";
				t_E=t_E.f_Parent;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4040>";
				bb_std_lang.popErr();
				return 0;
			}
		}while(!(false));
	}
	static public int bb_challengergui_CHGUI_UpdateWindow(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1278>";
		float t_X=(float)(bb_challengergui.bb_challengergui_CHGUI_RealX(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1279>";
		float t_Y=(float)(bb_challengergui.bb_challengergui_CHGUI_RealY(t_N));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1280>";
		int t_W=(int)(t_N.f_W);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1281>";
		int t_H=(int)(t_N.f_H);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1283>";
		if((t_N.f_Minimised)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1283>";
			t_H=(int)(bb_challengergui.bb_challengergui_CHGUI_TitleHeight);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1284>";
		int t_TH=(int)(bb_challengergui.bb_challengergui_CHGUI_TitleHeight);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1286>";
		if(t_N.f_Text.compareTo("")==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1287>";
			t_N.f_Close=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1288>";
			t_N.f_Minimise=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1291>";
		t_N.m_CheckOver();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1292>";
		t_N.m_CheckDown();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1295>";
		if((t_N.f_Over)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1296>";
			if(bb_input.bb_input_TouchY(0)>t_Y && bb_input.bb_input_TouchY(0)<t_Y+bb_challengergui.bb_challengergui_CHGUI_TitleHeight){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1297>";
				bb_challengergui.bb_challengergui_CHGUI_DragOver=1;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1304>";
		if((t_N.f_Close)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1305>";
			float t_TH2=bb_challengergui.bb_challengergui_CHGUI_TitleHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1306>";
			if(((t_N.f_CloseOver)!=0) && ((t_N.f_CloseDown)!=0) && bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1307>";
				t_N.f_CloseOver=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1308>";
				t_N.f_CloseDown=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1309>";
				t_N.f_Visible=0;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1311>";
			if(((t_N.f_Over)!=0) && bb_input.bb_input_TouchX(0)>t_X+(float)(t_W)-t_TH2/2.5f-10.0f && bb_input.bb_input_TouchX(0)<t_X+(float)(t_W)-t_TH2/2.5f-10.0f+t_TH2/2.5f && bb_input.bb_input_TouchY(0)>t_Y+(t_TH2-t_TH2/2.5f)/2.0f && bb_input.bb_input_TouchY(0)<t_Y+(t_TH2-t_TH2/2.5f)/2.0f+t_TH2/2.5f){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1312>";
				if(bb_challengergui.bb_challengergui_CHGUI_MouseBusy==0 || ((t_N.f_CloseDown)!=0)){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1313>";
					t_N.f_CloseOver=1;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1314>";
					bb_challengergui.bb_challengergui_CHGUI_DragOver=1;
				}
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1317>";
				t_N.f_CloseOver=0;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1319>";
			if(((t_N.f_CloseOver)!=0) || ((t_N.f_CloseDown)!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1320>";
				if((bb_input.bb_input_TouchDown(0))!=0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1321>";
					t_N.f_CloseDown=1;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1322>";
					bb_challengergui.bb_challengergui_CHGUI_DragOver=1;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1324>";
					t_N.f_CloseDown=0;
				}
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1327>";
				t_N.f_CloseDown=0;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1332>";
		if((t_N.f_Minimise)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1333>";
			float t_TH1=bb_challengergui.bb_challengergui_CHGUI_TitleHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1334>";
			int t_Off2=(int)((t_TH1-t_TH1/2.0f)/2.0f);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1336>";
			if(((t_N.f_MinimiseOver)!=0) && ((t_N.f_MinimiseDown)!=0) && bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1337>";
				t_N.f_CloseOver=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1338>";
				t_N.f_CloseDown=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1339>";
				if(t_N.f_Minimised==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1340>";
					t_N.f_Minimised=1;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1342>";
					t_N.f_Minimised=0;
				}
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1346>";
			if(((t_N.f_Over)!=0) && bb_input.bb_input_TouchX(0)>t_X+(float)(t_W)-((float)(t_TH)/2.5f+(float)(t_TH)/2.5f)-(float)(t_TH)/1.5f && bb_input.bb_input_TouchX(0)<t_X+(float)(t_W)-((float)(t_TH)/2.5f+(float)(t_TH)/2.5f)-(float)(t_TH)/1.5f+(float)(t_TH)/2.5f && bb_input.bb_input_TouchY(0)>t_Y+((float)(t_TH)-(float)(t_TH)/2.5f)/2.0f && bb_input.bb_input_TouchY(0)<t_Y+((float)(t_TH)-(float)(t_TH)/2.5f)/2.0f+(float)(t_TH)/2.5f){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1347>";
				if(bb_challengergui.bb_challengergui_CHGUI_MouseBusy==0 || ((t_N.f_MinimiseDown)!=0)){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1348>";
					t_N.f_MinimiseOver=1;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1349>";
					bb_challengergui.bb_challengergui_CHGUI_DragOver=1;
				}
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1352>";
				t_N.f_MinimiseOver=0;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1354>";
			if(((t_N.f_MinimiseOver)!=0) || ((t_N.f_MinimiseDown)!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1355>";
				if((bb_input.bb_input_TouchDown(0))!=0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1356>";
					t_N.f_MinimiseDown=1;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1357>";
					bb_challengergui.bb_challengergui_CHGUI_DragOver=1;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1359>";
					t_N.f_MinimiseDown=0;
				}
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1362>";
				t_N.f_MinimiseDown=0;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1367>";
		if(t_N.f_Moveable==1 && ((t_N.f_Over)!=0) && t_N.f_Moving==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1368>";
			if(bb_input.bb_input_TouchY(0)>t_Y && bb_input.bb_input_TouchY(0)<t_Y+bb_challengergui.bb_challengergui_CHGUI_TitleHeight && ((bb_input.bb_input_TouchDown(0))!=0) && (t_N.f_Text.compareTo("")!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1369>";
				if(t_N.f_CloseOver==0 && t_N.f_MinimiseOver==0 && t_N.f_CloseDown==0 && t_N.f_MinimiseDown==0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1370>";
					if(bb_challengergui.bb_challengergui_CHGUI_Moving==0){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1371>";
						t_N.f_Moving=1;
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1372>";
						t_N.f_MX=bb_input.bb_input_TouchX(0);
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1373>";
						t_N.f_MY=bb_input.bb_input_TouchY(0);
					}
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1378>";
		if(bb_input.bb_input_TouchDown(0)==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1378>";
			t_N.f_Moving=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1380>";
		if(t_N.f_Moving==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1381>";
			t_N.f_X=t_N.f_X+(bb_input.bb_input_TouchX(0)-t_N.f_MX);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1382>";
			t_N.f_Y=t_N.f_Y+(bb_input.bb_input_TouchY(0)-t_N.f_MY);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1383>";
			t_N.f_MX=bb_input.bb_input_TouchX(0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1384>";
			t_N.f_MY=bb_input.bb_input_TouchY(0);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1385>";
			bb_challengergui.bb_challengergui_CHGUI_DragOver=1;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1387>";
			bb_challengergui_CHGUI t_RP=t_N.f_Parent;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1389>";
			do{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1390>";
				if(t_RP.f_Element.compareTo("Tab")!=0){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1390>";
					break;
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1391>";
				if(t_RP.f_Parent!=null){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1392>";
					t_RP=t_RP.f_Parent;
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1394>";
					break;
				}
			}while(!(false));
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1399>";
			if(t_N.f_X<0.0f){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1399>";
				t_N.f_X=0.0f;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1401>";
			if(t_N.f_X>t_RP.f_W-t_N.f_W){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1401>";
				t_N.f_X=t_RP.f_W-t_N.f_W;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1403>";
			if(t_N.f_Y>t_RP.f_H-(float)(t_H)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1403>";
				t_N.f_Y=t_RP.f_H-(float)(t_H);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1405>";
			int t_YVal=t_RP.f_MenuHeight;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1406>";
			if(t_RP.f_Text.compareTo("")!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1406>";
				t_YVal=(int)((float)(t_YVal)+bb_challengergui.bb_challengergui_CHGUI_TitleHeight);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1407>";
			if((t_RP.f_Tabbed)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1407>";
				t_YVal=t_YVal+t_RP.f_TabHeight+5;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1409>";
			if(t_N.f_Y<(float)(t_YVal)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1409>";
				t_N.f_Y=(float)(t_YVal);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1414>";
		if(((t_N.f_Clicked)!=0) && bb_input.bb_input_TouchY(0)>t_Y && bb_input.bb_input_TouchY(0)<t_Y+bb_challengergui.bb_challengergui_CHGUI_TitleHeight && (t_N.f_Text.compareTo("")!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1415>";
			if(((t_N.f_Minimise)!=0) && t_N.f_CloseOver==0 && t_N.f_MinimiseOver==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1416>";
				if(t_N.f_DClickMillisecs>bb_app.bb_app_Millisecs()){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1417>";
					if(t_N.f_Minimised==1){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1418>";
						t_N.f_Minimised=0;
					}else{
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1420>";
						t_N.f_Minimised=1;
					}
				}else{
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1423>";
					t_N.f_DClickMillisecs=bb_app.bb_app_Millisecs()+275;
				}
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateContents(bb_challengergui_CHGUI t_N){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1145>";
		int t_X=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1146>";
		int t_XX=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1147>";
		int t_C=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1148>";
		t_N.f_ReOrdered=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1150>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_Menus)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1151>";
			t_N.f_Menus[t_X].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1152>";
			bb_challengergui.bb_challengergui_CHGUI_UpdateSubMenu(t_N.f_Menus[t_X]);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1155>";
		if((t_N.f_Tabbed)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1156>";
			bb_challengergui.bb_challengergui_CHGUI_UpdateContents(t_N.f_CurrentTab);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1160>";
		for(int t_NN=bb_std_lang.arrayLength(t_N.f_TopList)-1;t_NN>=0;t_NN=t_NN+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1161>";
			bb_challengergui.bb_challengergui_CHGUI_UpdateContents(t_N.f_TopList[t_NN]);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1164>";
		for(int t_NN2=bb_std_lang.arrayLength(t_N.f_VariList)-1;t_NN2>=0;t_NN2=t_NN2+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1165>";
			bb_challengergui.bb_challengergui_CHGUI_UpdateContents(t_N.f_VariList[t_NN2]);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1168>";
		for(int t_NN3=bb_std_lang.arrayLength(t_N.f_BottomList)-1;t_NN3>=0;t_NN3=t_NN3+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1169>";
			bb_challengergui.bb_challengergui_CHGUI_UpdateContents(t_N.f_BottomList[t_NN3]);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1173>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_Tabs)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1174>";
			t_N.f_Tabs[t_X].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1175>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N.f_Tabs[t_X]))!=0) && ((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Tabs[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Tabs[t_X])==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1175>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateTab(t_N.f_Tabs[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1180>";
		if((bb_challengergui.bb_challengergui_CHGUI_MenuClose)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1180>";
			t_N.f_MenuActive=null;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1181>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_Menus)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1182>";
			t_N.f_Menus[t_X].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1183>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Menus[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Menus[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N.f_Menus[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1183>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateMenu(t_N.f_Menus[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1187>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_Dropdowns)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1188>";
			t_C=0;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1189>";
			for(t_XX=0;t_XX<=bb_std_lang.arrayLength(t_N.f_Dropdowns[t_X].f_DropdownItems)-1;t_XX=t_XX+1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1190>";
				t_N.f_Dropdowns[t_X].f_DropdownItems[t_XX].m_CheckClicked();
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1191>";
				if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Dropdowns[t_X].f_DropdownItems[t_XX]))!=0) && ((t_N.f_Dropdowns[t_X].f_OnFocus)!=0) && ((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Dropdowns[t_X]))!=0)){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1191>";
					bb_challengergui.bb_challengergui_CHGUI_UpdateDropdownItem(t_N.f_Dropdowns[t_X].f_DropdownItems[t_XX],t_C);
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1192>";
				if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Dropdowns[t_X].f_DropdownItems[t_XX]))!=0) && ((t_N.f_Dropdowns[t_X].f_OnFocus)!=0) && ((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Dropdowns[t_X]))!=0)){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1192>";
					t_C=t_C+1;
				}
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1197>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_Dropdowns)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1198>";
			t_N.f_Dropdowns[t_X].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1199>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N.f_Dropdowns[t_X]))!=0) && ((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Dropdowns[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Dropdowns[t_X])==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1199>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateDropdown(t_N.f_Dropdowns[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1203>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_Labels)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1204>";
			t_N.f_Labels[t_X].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1205>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Labels[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Labels[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N.f_Labels[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1205>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateLabel(t_N.f_Labels[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1209>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_Textfields)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1210>";
			t_N.f_Textfields[t_X].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1211>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Textfields[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Textfields[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N.f_Textfields[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1211>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateTextfield(t_N.f_Textfields[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1215>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_HSliders)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1216>";
			t_N.f_HSliders[t_X].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1217>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_HSliders[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_HSliders[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N.f_HSliders[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1217>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateHSlider(t_N.f_HSliders[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1221>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_VSliders)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1222>";
			t_N.f_VSliders[t_X].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1223>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_VSliders[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_VSliders[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N.f_VSliders[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1223>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateVSlider(t_N.f_VSliders[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1227>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_Listboxes)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1228>";
			t_N.f_Listboxes[t_X].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1229>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Listboxes[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Listboxes[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N.f_Listboxes[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1230>";
				int t_C2=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1232>";
				for(t_XX=(int)(t_N.f_Listboxes[t_X].f_ListboxSlider.f_Value);(float)(t_XX)<=t_N.f_Listboxes[t_X].f_ListboxSlider.f_Value+(float)(t_N.f_Listboxes[t_X].f_ListboxNumber);t_XX=t_XX+1){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1233>";
					if(t_XX<bb_std_lang.arrayLength(t_N.f_Listboxes[t_X].f_ListboxItems) && t_XX>-1){
						bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1234>";
						if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Listboxes[t_X].f_ListboxItems[t_XX]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Listboxes[t_X].f_ListboxItems[t_XX])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N.f_Listboxes[t_X].f_ListboxItems[t_XX]))!=0)){
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1235>";
							t_N.f_Listboxes[t_X].f_ListboxItems[t_XX].m_CheckClicked();
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1236>";
							bb_challengergui.bb_challengergui_CHGUI_UpdateListboxItem(t_N.f_Listboxes[t_X].f_ListboxItems[t_XX],t_C2);
							bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1237>";
							t_C2=t_C2+1;
						}
					}
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1241>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateListbox(t_N.f_Listboxes[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1246>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_Radioboxes)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1247>";
			t_N.f_Radioboxes[t_X].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1248>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Radioboxes[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Radioboxes[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N.f_Radioboxes[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1248>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateRadiobox(t_N.f_Radioboxes[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1252>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_Tickboxes)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1253>";
			t_N.f_Tickboxes[t_X].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1254>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Tickboxes[t_X]))!=0) && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N.f_Tickboxes[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Tickboxes[t_X])==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1254>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateTickbox(t_N.f_Tickboxes[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1258>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_ImageButtons)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1259>";
			t_N.f_ImageButtons[t_X].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1260>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_ImageButtons[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_ImageButtons[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N.f_ImageButtons[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1260>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateImageButton(t_N.f_ImageButtons[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1264>";
		for(t_X=bb_std_lang.arrayLength(t_N.f_Buttons)-1;t_X>=0;t_X=t_X+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1265>";
			t_N.f_Buttons[t_X].m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1266>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N.f_Buttons[t_X]))!=0) && bb_challengergui.bb_challengergui_CHGUI_RealMinimised(t_N.f_Buttons[t_X])==0 && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N.f_Buttons[t_X]))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1266>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateButton(t_N.f_Buttons[t_X]);
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1269>";
		if(t_N.f_Element.compareTo("Tab")!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1271>";
			t_N.m_CheckClicked();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1272>";
			if(((bb_challengergui.bb_challengergui_CHGUI_RealVisible(t_N))!=0) && ((bb_challengergui.bb_challengergui_CHGUI_RealActive(t_N))!=0) || ((bb_challengergui.bb_challengergui_CHGUI_Locked(t_N))!=0)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1272>";
				bb_challengergui.bb_challengergui_CHGUI_UpdateWindow(t_N);
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static int bb_challengergui_CHGUI_CursorMillisecs;
	static int bb_challengergui_CHGUI_DragScroll;
	static int bb_challengergui_CHGUI_DragMoving;
	static float bb_challengergui_CHGUI_OffsetMX;
	static float bb_challengergui_CHGUI_OffsetMY;
	static public int bb_challengergui_LockFocus(bb_challengergui_CHGUI t_Window){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<873>";
		if(bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow.f_Visible==0 || t_Window==bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<874>";
			bb_challengergui.bb_challengergui_CHGUI_LockedWIndow=t_Window;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<875>";
			bb_challengergui.bb_challengergui_CHGUI_Reorder(t_Window);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<876>";
			bb_challengergui.bb_challengergui_CHGUI_Canvas.f_Active=0;
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_UnlockFocus(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<882>";
		if(bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow.f_Visible==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<883>";
			bb_challengergui.bb_challengergui_CHGUI_LockedWIndow=null;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<884>";
			bb_challengergui.bb_challengergui_CHGUI_Canvas.f_Active=1;
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_UpdateMsgBox(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3855>";
		if((bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow.f_Visible)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3856>";
			bb_challengergui.bb_challengergui_LockFocus(bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3858>";
			bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow.f_X=(float)(bb_graphics.bb_graphics_DeviceWidth()/2)-bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow.f_W/2.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3859>";
			bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow.f_Y=(float)(bb_graphics.bb_graphics_DeviceHeight()/2)-bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow.f_H/2.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3860>";
			bb_challengergui.bb_challengergui_CHGUI_MsgBoxLabel.f_X=(bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow.f_W-bb_challengergui.bb_challengergui_CHGUI_Font.m_GetTxtWidth2(bb_challengergui.bb_challengergui_CHGUI_MsgBoxLabel.f_Text))/2.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3862>";
			if((bb_challengergui.bb_challengergui_CHGUI_MsgBoxButton.f_Clicked)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3863>";
				bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow.f_Visible=0;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3864>";
				bb_challengergui.bb_challengergui_UnlockFocus();
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_challengergui_CHGUI_Update(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<743>";
		if(bb_input.bb_input_TouchDown(0)==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<743>";
			bb_challengergui.bb_challengergui_CHGUI_MouseBusy=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<744>";
		bb_challengergui.bb_challengergui_CHGUI_Over=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<745>";
		bb_challengergui.bb_challengergui_CHGUI_OverFlag=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<746>";
		bb_challengergui.bb_challengergui_CHGUI_DownFlag=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<747>";
		bb_challengergui.bb_challengergui_CHGUI_MenuOver=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<748>";
		bb_challengergui.bb_challengergui_CHGUI_TextBoxOver=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<749>";
		bb_challengergui.bb_challengergui_CHGUI_TextboxOnFocus=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<750>";
		bb_challengergui.bb_challengergui_CHGUI_TextBoxDown=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<751>";
		bb_challengergui.bb_challengergui_CHGUI_DragOver=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<752>";
		bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=null;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<753>";
		if(bb_challengergui.bb_challengergui_CHGUI_Canvas!=null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<754>";
			bb_challengergui.bb_challengergui_CHGUI_Canvas.f_W=(float)(bb_challengergui.bb_challengergui_CHGUI_Width);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<755>";
			bb_challengergui.bb_challengergui_CHGUI_Canvas.f_H=(float)(bb_challengergui.bb_challengergui_CHGUI_Height);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<759>";
		if(bb_challengergui.bb_challengergui_CHGUI_Moving==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<760>";
			bb_challengergui.bb_challengergui_CHGUI_OffsetY=bb_challengergui.bb_challengergui_CHGUI_OffsetY-(bb_challengergui.bb_challengergui_CHGUI_OffsetY-bb_challengergui.bb_challengergui_CHGUI_TargetY)/8.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<761>";
			bb_challengergui.bb_challengergui_CHGUI_OffsetX=bb_challengergui.bb_challengergui_CHGUI_OffsetX-(bb_challengergui.bb_challengergui_CHGUI_OffsetX-bb_challengergui.bb_challengergui_CHGUI_TargetX)/8.0f;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<762>";
			if(bb_challengergui.bb_challengergui_CHGUI_OffsetY-bb_challengergui.bb_challengergui_CHGUI_TargetY>-1.0f && bb_challengergui.bb_challengergui_CHGUI_OffsetY-bb_challengergui.bb_challengergui_CHGUI_TargetY<1.0f && bb_challengergui.bb_challengergui_CHGUI_OffsetX-bb_challengergui.bb_challengergui_CHGUI_TargetX>-1.0f && bb_challengergui.bb_challengergui_CHGUI_OffsetX-bb_challengergui.bb_challengergui_CHGUI_TargetX<1.0f){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<763>";
				bb_challengergui.bb_challengergui_CHGUI_OffsetY=bb_challengergui.bb_challengergui_CHGUI_TargetY;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<764>";
				bb_challengergui.bb_challengergui_CHGUI_OffsetX=bb_challengergui.bb_challengergui_CHGUI_TargetX;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<765>";
				bb_challengergui.bb_challengergui_CHGUI_Moving=0;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<769>";
		for(int t_N=bb_std_lang.arrayLength(bb_challengergui.bb_challengergui_CHGUI_TopList)-1;t_N>=0;t_N=t_N+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<770>";
			bb_challengergui.bb_challengergui_CHGUI_UpdateContents(bb_challengergui.bb_challengergui_CHGUI_TopList[t_N]);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<772>";
		for(int t_N2=bb_std_lang.arrayLength(bb_challengergui.bb_challengergui_CHGUI_VariList)-1;t_N2>=0;t_N2=t_N2+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<773>";
			bb_challengergui.bb_challengergui_CHGUI_UpdateContents(bb_challengergui.bb_challengergui_CHGUI_VariList[t_N2]);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<775>";
		for(int t_N3=bb_std_lang.arrayLength(bb_challengergui.bb_challengergui_CHGUI_BottomList)-1;t_N3>=0;t_N3=t_N3+-1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<776>";
			bb_challengergui.bb_challengergui_CHGUI_UpdateContents(bb_challengergui.bb_challengergui_CHGUI_BottomList[t_N3]);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<781>";
		if(((bb_input.bb_input_TouchDown(0))!=0) && bb_challengergui.bb_challengergui_CHGUI_DownFlag==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<782>";
			bb_challengergui.bb_challengergui_CHGUI_IgnoreMouse=1;
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<784>";
			bb_challengergui.bb_challengergui_CHGUI_IgnoreMouse=0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<786>";
		bb_challengergui.bb_challengergui_CHGUI_MenuClose=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<787>";
		if(bb_challengergui.bb_challengergui_CHGUI_MenuOver==0 && ((bb_input.bb_input_TouchDown(0))!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<787>";
			bb_challengergui.bb_challengergui_CHGUI_MenuClose=1;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<788>";
		if(bb_challengergui.bb_challengergui_CHGUI_CursorMillisecs<bb_app.bb_app_Millisecs()){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<789>";
			bb_challengergui.bb_challengergui_CHGUI_CursorMillisecs=bb_app.bb_app_Millisecs()+300;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<790>";
			if(bb_challengergui.bb_challengergui_CHGUI_Cursor==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<791>";
				bb_challengergui.bb_challengergui_CHGUI_Cursor=1;
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<793>";
				bb_challengergui.bb_challengergui_CHGUI_Cursor=0;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<797>";
		if(bb_challengergui.bb_challengergui_CHGUI_DragScroll==1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<798>";
			if(bb_challengergui.bb_challengergui_CHGUI_DragOver==0 && ((bb_input.bb_input_TouchDown(0))!=0) && bb_challengergui.bb_challengergui_CHGUI_DragMoving==0 && bb_challengergui.bb_challengergui_CHGUI_TextboxOnFocus==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<799>";
				bb_challengergui.bb_challengergui_CHGUI_OffsetMX=bb_input.bb_input_TouchX(0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<800>";
				bb_challengergui.bb_challengergui_CHGUI_OffsetMY=bb_input.bb_input_TouchY(0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<801>";
				bb_challengergui.bb_challengergui_CHGUI_DragMoving=1;
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<804>";
			if(bb_challengergui.bb_challengergui_CHGUI_DragMoving==1){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<805>";
				bb_challengergui.bb_challengergui_CHGUI_OffsetX=bb_challengergui.bb_challengergui_CHGUI_OffsetX+(bb_input.bb_input_TouchX(0)-bb_challengergui.bb_challengergui_CHGUI_OffsetMX);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<806>";
				bb_challengergui.bb_challengergui_CHGUI_OffsetY=bb_challengergui.bb_challengergui_CHGUI_OffsetY+(bb_input.bb_input_TouchY(0)-bb_challengergui.bb_challengergui_CHGUI_OffsetMY);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<807>";
				bb_challengergui.bb_challengergui_CHGUI_OffsetMX=bb_input.bb_input_TouchX(0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<808>";
				bb_challengergui.bb_challengergui_CHGUI_OffsetMY=bb_input.bb_input_TouchY(0);
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<811>";
				if(bb_challengergui.bb_challengergui_CHGUI_OffsetX<(float)(-1*(bb_challengergui.bb_challengergui_CHGUI_Width-bb_graphics.bb_graphics_DeviceWidth()))){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<811>";
					bb_challengergui.bb_challengergui_CHGUI_OffsetX=(float)(-1*(bb_challengergui.bb_challengergui_CHGUI_Width-bb_graphics.bb_graphics_DeviceWidth()));
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<812>";
				if(bb_challengergui.bb_challengergui_CHGUI_OffsetX>0.0f){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<812>";
					bb_challengergui.bb_challengergui_CHGUI_OffsetX=0.0f;
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<814>";
				if(bb_challengergui.bb_challengergui_CHGUI_OffsetY<(float)(bb_graphics.bb_graphics_DeviceHeight()-bb_challengergui.bb_challengergui_CHGUI_Height)){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<814>";
					bb_challengergui.bb_challengergui_CHGUI_OffsetY=(float)(bb_graphics.bb_graphics_DeviceHeight()-bb_challengergui.bb_challengergui_CHGUI_Height);
				}
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<815>";
				if(bb_challengergui.bb_challengergui_CHGUI_OffsetY>0.0f){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<815>";
					bb_challengergui.bb_challengergui_CHGUI_OffsetY=0.0f;
				}
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<818>";
			if(bb_challengergui.bb_challengergui_CHGUI_Width<bb_graphics.bb_graphics_DeviceWidth()){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<818>";
				bb_challengergui.bb_challengergui_CHGUI_OffsetX=(float)(bb_graphics.bb_graphics_DeviceWidth()/2-bb_challengergui.bb_challengergui_CHGUI_Width/2);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<819>";
			if(bb_challengergui.bb_challengergui_CHGUI_Height<bb_graphics.bb_graphics_DeviceHeight()){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<819>";
				bb_challengergui.bb_challengergui_CHGUI_OffsetY=(float)(bb_graphics.bb_graphics_DeviceHeight()/2-bb_challengergui.bb_challengergui_CHGUI_Height/2);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<822>";
			if(bb_input.bb_input_TouchDown(0)==0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<822>";
				bb_challengergui.bb_challengergui_CHGUI_DragMoving=0;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<825>";
		if(((bb_input.bb_input_TouchDown(0))!=0) && bb_challengergui.bb_challengergui_CHGUI_TextBoxDown==0 && bb_challengergui.bb_challengergui_CHGUI_DragMoving==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<826>";
			if(bb_challengergui.bb_challengergui_CHGUI_Keyboard==2){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<827>";
				if(bb_input.bb_input_TouchY(0)<(float)(bb_graphics.bb_graphics_DeviceHeight())-bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow.f_H){
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<828>";
					bb_challengergui.bb_challengergui_CHGUI_TargetX=bb_challengergui.bb_challengergui_CHGUI_OldX;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<829>";
					bb_challengergui.bb_challengergui_CHGUI_TargetY=bb_challengergui.bb_challengergui_CHGUI_OldY;
					bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<830>";
					bb_challengergui.bb_challengergui_CHGUI_Moving=1;
				}
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<833>";
				bb_challengergui.bb_challengergui_CHGUI_TargetX=bb_challengergui.bb_challengergui_CHGUI_OldX;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<834>";
				bb_challengergui.bb_challengergui_CHGUI_TargetY=bb_challengergui.bb_challengergui_CHGUI_OldY;
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<835>";
				bb_challengergui.bb_challengergui_CHGUI_Moving=1;
			}
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<839>";
		if(bb_challengergui.bb_challengergui_CHGUI_TextBoxOver==0 && bb_challengergui.bb_challengergui_CHGUI_TextboxOnFocus==0 && bb_challengergui.bb_challengergui_CHGUI_Moving==0 && bb_challengergui.bb_challengergui_CHGUI_DragMoving==0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<840>";
			bb_challengergui.bb_challengergui_CHGUI_OldX=bb_challengergui.bb_challengergui_CHGUI_OffsetX;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<841>";
			bb_challengergui.bb_challengergui_CHGUI_OldY=bb_challengergui.bb_challengergui_CHGUI_OffsetY;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<844>";
		bb_challengergui.bb_challengergui_CHGUI_UpdateMsgBox();
		bb_std_lang.popErr();
		return 0;
	}
}
class bb_bitmapchar{
}
class bb_bitmapcharmetrics{
}
class bb_bitmapfont{
}
class bb_drawingpoint{
}
class bb_drawingrectangle{
}
class bb_edrawalign{
}
class bb_edrawmode{
}
class bb_fontinterface{
}
class bb_fontmachine{
}
class bb_app{
	static bb_app_AppDevice bb_app_device;
	static public int bb_app_SetUpdateRate(int t_hertz){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<145>";
		int t_=bb_app.bb_app_device.SetUpdateRate(t_hertz);
		bb_std_lang.popErr();
		return t_;
	}
	static public int bb_app_Millisecs(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<153>";
		int t_=bb_app.bb_app_device.MilliSecs();
		bb_std_lang.popErr();
		return t_;
	}
	static public String bb_app_LoadString(String t_path){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<141>";
		String t_=bb_app.bb_app_device.LoadString(bb_data.bb_data_FixDataPath(t_path));
		bb_std_lang.popErr();
		return t_;
	}
}
class bb_asyncimageloader{
}
class bb_asyncloaders{
}
class bb_asyncsoundloader{
}
class bb_audio{
	static gxtkAudio bb_audio_device;
	static public int bb_audio_SetAudioDevice(gxtkAudio t_dev){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/audio.monkey<17>";
		bb_audio.bb_audio_device=t_dev;
		bb_std_lang.popErr();
		return 0;
	}
}
class bb_audiodevice{
}
class bb_data{
	static public String bb_data_FixDataPath(String t_path){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<3>";
		int t_i=t_path.indexOf(":/",0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<4>";
		if(t_i!=-1 && t_path.indexOf("/",0)==t_i+1){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<4>";
			bb_std_lang.popErr();
			return t_path;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<5>";
		if(t_path.startsWith("./") || t_path.startsWith("/")){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<5>";
			bb_std_lang.popErr();
			return t_path;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<6>";
		String t_="monkey://data/"+t_path;
		bb_std_lang.popErr();
		return t_;
	}
}
class bb_graphics{
	static gxtkGraphics bb_graphics_device;
	static public int bb_graphics_SetGraphicsDevice(gxtkGraphics t_dev){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<58>";
		bb_graphics.bb_graphics_device=t_dev;
		bb_std_lang.popErr();
		return 0;
	}
	static bb_graphics_GraphicsContext bb_graphics_context;
	static public bb_graphics_Image bb_graphics_LoadImage(String t_path,int t_frameCount,int t_flags){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<229>";
		gxtkSurface t_surf=bb_graphics.bb_graphics_device.LoadSurface(bb_data.bb_data_FixDataPath(t_path));
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<230>";
		if((t_surf)!=null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<230>";
			bb_graphics_Image t_=((new bb_graphics_Image()).g_new()).m_Init(t_surf,t_frameCount,t_flags);
			bb_std_lang.popErr();
			return t_;
		}
		bb_std_lang.popErr();
		return null;
	}
	static public bb_graphics_Image bb_graphics_LoadImage2(String t_path,int t_frameWidth,int t_frameHeight,int t_frameCount,int t_flags){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<234>";
		bb_graphics_Image t_atlas=bb_graphics.bb_graphics_LoadImage(t_path,1,0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<235>";
		if((t_atlas)!=null){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<235>";
			bb_graphics_Image t_=t_atlas.m_GrabImage(0,0,t_frameWidth,t_frameHeight,t_frameCount,t_flags);
			bb_std_lang.popErr();
			return t_;
		}
		bb_std_lang.popErr();
		return null;
	}
	static public int bb_graphics_SetFont(bb_graphics_Image t_font,int t_firstChar){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<524>";
		if(!((t_font)!=null)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<525>";
			if(!((bb_graphics.bb_graphics_context.f_defaultFont)!=null)){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<526>";
				bb_graphics.bb_graphics_context.f_defaultFont=bb_graphics.bb_graphics_LoadImage("mojo_font.png",96,2);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<528>";
			t_font=bb_graphics.bb_graphics_context.f_defaultFont;
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<529>";
			t_firstChar=32;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<531>";
		bb_graphics.bb_graphics_context.f_font=t_font;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<532>";
		bb_graphics.bb_graphics_context.f_firstChar=t_firstChar;
		bb_std_lang.popErr();
		return 0;
	}
	static gxtkGraphics bb_graphics_renderDevice;
	static public int bb_graphics_SetMatrix(float t_ix,float t_iy,float t_jx,float t_jy,float t_tx,float t_ty){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<289>";
		bb_graphics.bb_graphics_context.f_ix=t_ix;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<290>";
		bb_graphics.bb_graphics_context.f_iy=t_iy;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<291>";
		bb_graphics.bb_graphics_context.f_jx=t_jx;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<292>";
		bb_graphics.bb_graphics_context.f_jy=t_jy;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<293>";
		bb_graphics.bb_graphics_context.f_tx=t_tx;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<294>";
		bb_graphics.bb_graphics_context.f_ty=t_ty;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<295>";
		bb_graphics.bb_graphics_context.f_tformed=((t_ix!=1.0f || t_iy!=0.0f || t_jx!=0.0f || t_jy!=1.0f || t_tx!=0.0f || t_ty!=0.0f)?1:0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<296>";
		bb_graphics.bb_graphics_context.f_matDirty=1;
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_SetMatrix2(float[] t_m){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<285>";
		bb_graphics.bb_graphics_SetMatrix(t_m[0],t_m[1],t_m[2],t_m[3],t_m[4],t_m[5]);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_SetColor(float t_r,float t_g,float t_b){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<244>";
		bb_graphics.bb_graphics_context.f_color_r=t_r;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<245>";
		bb_graphics.bb_graphics_context.f_color_g=t_g;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<246>";
		bb_graphics.bb_graphics_context.f_color_b=t_b;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<247>";
		bb_graphics.bb_graphics_renderDevice.SetColor(t_r,t_g,t_b);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_SetAlpha(float t_alpha){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<255>";
		bb_graphics.bb_graphics_context.f_alpha=t_alpha;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<256>";
		bb_graphics.bb_graphics_renderDevice.SetAlpha(t_alpha);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_SetBlend(int t_blend){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<264>";
		bb_graphics.bb_graphics_context.f_blend=t_blend;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<265>";
		bb_graphics.bb_graphics_renderDevice.SetBlend(t_blend);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_DeviceWidth(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<221>";
		int t_=bb_graphics.bb_graphics_device.Width();
		bb_std_lang.popErr();
		return t_;
	}
	static public int bb_graphics_DeviceHeight(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<225>";
		int t_=bb_graphics.bb_graphics_device.Height();
		bb_std_lang.popErr();
		return t_;
	}
	static public int bb_graphics_SetScissor(float t_x,float t_y,float t_width,float t_height){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<273>";
		bb_graphics.bb_graphics_context.f_scissor_x=t_x;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<274>";
		bb_graphics.bb_graphics_context.f_scissor_y=t_y;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<275>";
		bb_graphics.bb_graphics_context.f_scissor_width=t_width;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<276>";
		bb_graphics.bb_graphics_context.f_scissor_height=t_height;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<277>";
		bb_graphics.bb_graphics_renderDevice.SetScissor((int)(t_x),(int)(t_y),(int)(t_width),(int)(t_height));
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_BeginRender(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<206>";
		if(!((bb_graphics.bb_graphics_device.Mode())!=0)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<206>";
			bb_std_lang.popErr();
			return 0;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<207>";
		bb_graphics.bb_graphics_renderDevice=bb_graphics.bb_graphics_device;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<208>";
		bb_graphics.bb_graphics_context.f_matrixSp=0;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<209>";
		bb_graphics.bb_graphics_SetMatrix(1.0f,0.0f,0.0f,1.0f,0.0f,0.0f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<210>";
		bb_graphics.bb_graphics_SetColor(255.0f,255.0f,255.0f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<211>";
		bb_graphics.bb_graphics_SetAlpha(1.0f);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<212>";
		bb_graphics.bb_graphics_SetBlend(0);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<213>";
		bb_graphics.bb_graphics_SetScissor(0.0f,0.0f,(float)(bb_graphics.bb_graphics_DeviceWidth()),(float)(bb_graphics.bb_graphics_DeviceHeight()));
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_EndRender(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<217>";
		bb_graphics.bb_graphics_renderDevice=null;
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_DebugRenderDevice(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<48>";
		if(!((bb_graphics.bb_graphics_renderDevice)!=null)){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<48>";
			bb_std_lang.error("Rendering operations can only be performed inside OnRender");
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_Cls(float t_r,float t_g,float t_b){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<354>";
		bb_graphics.bb_graphics_DebugRenderDevice();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<356>";
		bb_graphics.bb_graphics_renderDevice.Cls(t_r,t_g,t_b);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_PushMatrix(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<310>";
		int t_sp=bb_graphics.bb_graphics_context.f_matrixSp;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<311>";
		bb_graphics.bb_graphics_context.f_matrixStack[t_sp+0]=bb_graphics.bb_graphics_context.f_ix;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<312>";
		bb_graphics.bb_graphics_context.f_matrixStack[t_sp+1]=bb_graphics.bb_graphics_context.f_iy;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<313>";
		bb_graphics.bb_graphics_context.f_matrixStack[t_sp+2]=bb_graphics.bb_graphics_context.f_jx;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<314>";
		bb_graphics.bb_graphics_context.f_matrixStack[t_sp+3]=bb_graphics.bb_graphics_context.f_jy;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<315>";
		bb_graphics.bb_graphics_context.f_matrixStack[t_sp+4]=bb_graphics.bb_graphics_context.f_tx;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<316>";
		bb_graphics.bb_graphics_context.f_matrixStack[t_sp+5]=bb_graphics.bb_graphics_context.f_ty;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<317>";
		bb_graphics.bb_graphics_context.f_matrixSp=t_sp+6;
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_Transform(float t_ix,float t_iy,float t_jx,float t_jy,float t_tx,float t_ty){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<331>";
		float t_ix2=t_ix*bb_graphics.bb_graphics_context.f_ix+t_iy*bb_graphics.bb_graphics_context.f_jx;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<332>";
		float t_iy2=t_ix*bb_graphics.bb_graphics_context.f_iy+t_iy*bb_graphics.bb_graphics_context.f_jy;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<333>";
		float t_jx2=t_jx*bb_graphics.bb_graphics_context.f_ix+t_jy*bb_graphics.bb_graphics_context.f_jx;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<334>";
		float t_jy2=t_jx*bb_graphics.bb_graphics_context.f_iy+t_jy*bb_graphics.bb_graphics_context.f_jy;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<335>";
		float t_tx2=t_tx*bb_graphics.bb_graphics_context.f_ix+t_ty*bb_graphics.bb_graphics_context.f_jx+bb_graphics.bb_graphics_context.f_tx;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<336>";
		float t_ty2=t_tx*bb_graphics.bb_graphics_context.f_iy+t_ty*bb_graphics.bb_graphics_context.f_jy+bb_graphics.bb_graphics_context.f_ty;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<337>";
		bb_graphics.bb_graphics_SetMatrix(t_ix2,t_iy2,t_jx2,t_jy2,t_tx2,t_ty2);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_Transform2(float[] t_m){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<327>";
		bb_graphics.bb_graphics_Transform(t_m[0],t_m[1],t_m[2],t_m[3],t_m[4],t_m[5]);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_Translate(float t_x,float t_y){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<341>";
		bb_graphics.bb_graphics_Transform(1.0f,0.0f,0.0f,1.0f,t_x,t_y);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_PopMatrix(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<321>";
		int t_sp=bb_graphics.bb_graphics_context.f_matrixSp-6;
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<322>";
		bb_graphics.bb_graphics_SetMatrix(bb_graphics.bb_graphics_context.f_matrixStack[t_sp+0],bb_graphics.bb_graphics_context.f_matrixStack[t_sp+1],bb_graphics.bb_graphics_context.f_matrixStack[t_sp+2],bb_graphics.bb_graphics_context.f_matrixStack[t_sp+3],bb_graphics.bb_graphics_context.f_matrixStack[t_sp+4],bb_graphics.bb_graphics_context.f_matrixStack[t_sp+5]);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<323>";
		bb_graphics.bb_graphics_context.f_matrixSp=t_sp;
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_DrawImageRect(bb_graphics_Image t_image,float t_x,float t_y,int t_srcX,int t_srcY,int t_srcWidth,int t_srcHeight,int t_frame){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<473>";
		bb_graphics.bb_graphics_DebugRenderDevice();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<475>";
		bb_graphics_Frame t_f=t_image.f_frames[t_frame];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<477>";
		if((bb_graphics.bb_graphics_context.f_tformed)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<478>";
			bb_graphics.bb_graphics_PushMatrix();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<481>";
			bb_graphics.bb_graphics_Translate(-t_image.f_tx+t_x,-t_image.f_ty+t_y);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<483>";
			bb_graphics.bb_graphics_context.m_Validate();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<485>";
			bb_graphics.bb_graphics_renderDevice.DrawSurface2(t_image.f_surface,0.0f,0.0f,t_srcX+t_f.f_x,t_srcY+t_f.f_y,t_srcWidth,t_srcHeight);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<487>";
			bb_graphics.bb_graphics_PopMatrix();
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<489>";
			bb_graphics.bb_graphics_context.m_Validate();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<491>";
			bb_graphics.bb_graphics_renderDevice.DrawSurface2(t_image.f_surface,-t_image.f_tx+t_x,-t_image.f_ty+t_y,t_srcX+t_f.f_x,t_srcY+t_f.f_y,t_srcWidth,t_srcHeight);
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_Rotate(float t_angle){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<349>";
		bb_graphics.bb_graphics_Transform((float)Math.cos((t_angle)*bb_std_lang.D2R),-(float)Math.sin((t_angle)*bb_std_lang.D2R),(float)Math.sin((t_angle)*bb_std_lang.D2R),(float)Math.cos((t_angle)*bb_std_lang.D2R),0.0f,0.0f);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_Scale(float t_x,float t_y){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<345>";
		bb_graphics.bb_graphics_Transform(t_x,0.0f,0.0f,t_y,0.0f,0.0f);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_DrawImageRect2(bb_graphics_Image t_image,float t_x,float t_y,int t_srcX,int t_srcY,int t_srcWidth,int t_srcHeight,float t_rotation,float t_scaleX,float t_scaleY,int t_frame){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<497>";
		bb_graphics.bb_graphics_DebugRenderDevice();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<499>";
		bb_graphics_Frame t_f=t_image.f_frames[t_frame];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<501>";
		bb_graphics.bb_graphics_PushMatrix();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<503>";
		bb_graphics.bb_graphics_Translate(t_x,t_y);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<504>";
		bb_graphics.bb_graphics_Rotate(t_rotation);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<505>";
		bb_graphics.bb_graphics_Scale(t_scaleX,t_scaleY);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<506>";
		bb_graphics.bb_graphics_Translate(-t_image.f_tx,-t_image.f_ty);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<508>";
		bb_graphics.bb_graphics_context.m_Validate();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<510>";
		bb_graphics.bb_graphics_renderDevice.DrawSurface2(t_image.f_surface,0.0f,0.0f,t_srcX+t_f.f_x,t_srcY+t_f.f_y,t_srcWidth,t_srcHeight);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<512>";
		bb_graphics.bb_graphics_PopMatrix();
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_DrawImage(bb_graphics_Image t_image,float t_x,float t_y,int t_frame){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<417>";
		bb_graphics.bb_graphics_DebugRenderDevice();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<419>";
		bb_graphics_Frame t_f=t_image.f_frames[t_frame];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<421>";
		if((bb_graphics.bb_graphics_context.f_tformed)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<422>";
			bb_graphics.bb_graphics_PushMatrix();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<424>";
			bb_graphics.bb_graphics_Translate(t_x-t_image.f_tx,t_y-t_image.f_ty);
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<426>";
			bb_graphics.bb_graphics_context.m_Validate();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<428>";
			if((t_image.f_flags&65536)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<429>";
				bb_graphics.bb_graphics_renderDevice.DrawSurface(t_image.f_surface,0.0f,0.0f);
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<431>";
				bb_graphics.bb_graphics_renderDevice.DrawSurface2(t_image.f_surface,0.0f,0.0f,t_f.f_x,t_f.f_y,t_image.f_width,t_image.f_height);
			}
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<434>";
			bb_graphics.bb_graphics_PopMatrix();
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<436>";
			bb_graphics.bb_graphics_context.m_Validate();
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<438>";
			if((t_image.f_flags&65536)!=0){
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<439>";
				bb_graphics.bb_graphics_renderDevice.DrawSurface(t_image.f_surface,t_x-t_image.f_tx,t_y-t_image.f_ty);
			}else{
				bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<441>";
				bb_graphics.bb_graphics_renderDevice.DrawSurface2(t_image.f_surface,t_x-t_image.f_tx,t_y-t_image.f_ty,t_f.f_x,t_f.f_y,t_image.f_width,t_image.f_height);
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_DrawImage2(bb_graphics_Image t_image,float t_x,float t_y,float t_rotation,float t_scaleX,float t_scaleY,int t_frame){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<448>";
		bb_graphics.bb_graphics_DebugRenderDevice();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<450>";
		bb_graphics_Frame t_f=t_image.f_frames[t_frame];
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<452>";
		bb_graphics.bb_graphics_PushMatrix();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<454>";
		bb_graphics.bb_graphics_Translate(t_x,t_y);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<455>";
		bb_graphics.bb_graphics_Rotate(t_rotation);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<456>";
		bb_graphics.bb_graphics_Scale(t_scaleX,t_scaleY);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<458>";
		bb_graphics.bb_graphics_Translate(-t_image.f_tx,-t_image.f_ty);
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<460>";
		bb_graphics.bb_graphics_context.m_Validate();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<462>";
		if((t_image.f_flags&65536)!=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<463>";
			bb_graphics.bb_graphics_renderDevice.DrawSurface(t_image.f_surface,0.0f,0.0f);
		}else{
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<465>";
			bb_graphics.bb_graphics_renderDevice.DrawSurface2(t_image.f_surface,0.0f,0.0f,t_f.f_x,t_f.f_y,t_image.f_width,t_image.f_height);
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<468>";
		bb_graphics.bb_graphics_PopMatrix();
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_DrawLine(float t_x1,float t_y1,float t_x2,float t_y2){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<377>";
		bb_graphics.bb_graphics_DebugRenderDevice();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<379>";
		bb_graphics.bb_graphics_context.m_Validate();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<380>";
		bb_graphics.bb_graphics_renderDevice.DrawLine(t_x1,t_y1,t_x2,t_y2);
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_graphics_DrawRect(float t_x,float t_y,float t_w,float t_h){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<369>";
		bb_graphics.bb_graphics_DebugRenderDevice();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<371>";
		bb_graphics.bb_graphics_context.m_Validate();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<372>";
		bb_graphics.bb_graphics_renderDevice.DrawRect(t_x,t_y,t_w,t_h);
		bb_std_lang.popErr();
		return 0;
	}
}
class bb_graphicsdevice{
}
class bb_input{
	static gxtkInput bb_input_device;
	static public int bb_input_SetInputDevice(gxtkInput t_dev){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<16>";
		bb_input.bb_input_device=t_dev;
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_input_TouchDown(int t_index){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<93>";
		int t_=bb_input.bb_input_device.KeyDown(384+t_index);
		bb_std_lang.popErr();
		return t_;
	}
	static public float bb_input_TouchX(int t_index){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<85>";
		float t_=bb_input.bb_input_device.TouchX(t_index);
		bb_std_lang.popErr();
		return t_;
	}
	static public float bb_input_TouchY(int t_index){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<89>";
		float t_=bb_input.bb_input_device.TouchY(t_index);
		bb_std_lang.popErr();
		return t_;
	}
	static public int bb_input_EnableKeyboard(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<41>";
		int t_=bb_input.bb_input_device.SetKeyboardEnabled(1);
		bb_std_lang.popErr();
		return t_;
	}
	static public int bb_input_GetChar(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<57>";
		int t_=bb_input.bb_input_device.GetChar();
		bb_std_lang.popErr();
		return t_;
	}
	static public int bb_input_DisableKeyboard(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<45>";
		int t_=bb_input.bb_input_device.SetKeyboardEnabled(0);
		bb_std_lang.popErr();
		return t_;
	}
}
class bb_inputdevice{
}
class bb_mojo{
}
class bb_boxes{
}
class bb_lang{
}
class bb_list{
}
class bb_map{
}
class bb_math{
	static public int bb_math_Abs(int t_x){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<46>";
		if(t_x>=0){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<46>";
			bb_std_lang.popErr();
			return t_x;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<47>";
		int t_=-t_x;
		bb_std_lang.popErr();
		return t_;
	}
	static public float bb_math_Abs2(float t_x){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<73>";
		if(t_x>=0.0f){
			bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<73>";
			bb_std_lang.popErr();
			return t_x;
		}
		bb_std_lang.errInfo="C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<74>";
		float t_=-t_x;
		bb_std_lang.popErr();
		return t_;
	}
}
class bb_monkey{
}
class bb_random{
}
class bb_set{
}
class bb_stack{
}
class bb_{
	static public int bbMain(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<72>";
		(new bb__Beacon()).g_new();
		bb_std_lang.popErr();
		return 0;
	}
	public static int bbInit(){
		bb_graphics.bb_graphics_device=null;
		bb_input.bb_input_device=null;
		bb_audio.bb_audio_device=null;
		bb_app.bb_app_device=null;
		bb_graphics.bb_graphics_context=(new bb_graphics_GraphicsContext()).g_new();
		bb_graphics_Image.g_DefaultFlags=0;
		bb_graphics.bb_graphics_renderDevice=null;
		bb_challengergui.bb_challengergui_CHGUI_MobileMode=0;
		bb_data2.bb_data2_STATUS="start";
		bb_stream_Stream.g__tmpbuf=(new bb_databuffer_DataBuffer()).g_new(4096);
		bb_protocol.bb_protocol_LastP=0;
		bb_protocol.bb_protocol_SList=new String[]{""};
		bb_challengergui.bb_challengergui_CHGUI_BottomList=new bb_challengergui_CHGUI[0];
		bb_challengergui.bb_challengergui_CHGUI_Canvas=null;
		bb_challengergui.bb_challengergui_CHGUI_OffsetX=.0f;
		bb_challengergui.bb_challengergui_CHGUI_OffsetY=.0f;
		bb_challengergui.bb_challengergui_CHGUI_TitleHeight=25.0f;
		bb_challengergui.bb_challengergui_CHGUI_LockedWIndow=null;
		bb_challengergui.bb_challengergui_CHGUI_Shadow=1;
		bb_challengergui.bb_challengergui_CHGUI_ShadowImg=null;
		bb_challengergui.bb_challengergui_CHGUI_Style=null;
		bb_challengergui.bb_challengergui_CHGUI_TitleFont=null;
		bb_challengergui.bb_challengergui_CHGUI_Font=null;
		bb_challengergui.bb_challengergui_CHGUI_KeyboardButtons=new bb_challengergui_CHGUI[109];
		bb_challengergui.bb_challengergui_CHGUI_ShiftHold=0;
		bb_challengergui.bb_challengergui_CHGUI_Cursor=0;
		bb_challengergui.bb_challengergui_CHGUI_VariList=new bb_challengergui_CHGUI[0];
		bb_challengergui.bb_challengergui_CHGUI_TopList=new bb_challengergui_CHGUI[0];
		bb_challengergui.bb_challengergui_CHGUI_TooltipFlag=null;
		bb_challengergui.bb_challengergui_CHGUI_TooltipFont=null;
		bb_challengergui.bb_challengergui_CHGUI_Millisecs=0;
		bb_challengergui.bb_challengergui_CHGUI_FPSCounter=0;
		bb_challengergui.bb_challengergui_CHGUI_FPS=0;
		bb_challengergui.bb_challengergui_CHGUI_Width=0;
		bb_challengergui.bb_challengergui_CHGUI_Height=0;
		bb_challengergui.bb_challengergui_CHGUI_CanvasFlag=0;
		bb_challengergui.bb_challengergui_CHGUI_Started=0;
		bb_challengergui.bb_challengergui_CHGUI_TopTop=null;
		bb_challengergui.bb_challengergui_CHGUI_KeyboardWindow=null;
		bb_challengergui.bb_challengergui_CHGUI_MsgBoxWindow=null;
		bb_challengergui.bb_challengergui_CHGUI_MsgBoxLabel=null;
		bb_challengergui.bb_challengergui_CHGUI_MsgBoxButton=null;
		bb_data2.bb_data2_SCALE_W=300.0f;
		bb_data2.bb_data2_SCALE_H=480.0f;
		bb_challengergui.bb_challengergui_CHGUI_MouseBusy=0;
		bb_challengergui.bb_challengergui_CHGUI_Over=0;
		bb_challengergui.bb_challengergui_CHGUI_OverFlag=0;
		bb_challengergui.bb_challengergui_CHGUI_DownFlag=0;
		bb_challengergui.bb_challengergui_CHGUI_MenuOver=0;
		bb_challengergui.bb_challengergui_CHGUI_TextBoxOver=0;
		bb_challengergui.bb_challengergui_CHGUI_TextboxOnFocus=0;
		bb_challengergui.bb_challengergui_CHGUI_TextBoxDown=0;
		bb_challengergui.bb_challengergui_CHGUI_DragOver=0;
		bb_challengergui.bb_challengergui_CHGUI_Moving=0;
		bb_challengergui.bb_challengergui_CHGUI_TargetY=.0f;
		bb_challengergui.bb_challengergui_CHGUI_TargetX=.0f;
		bb_challengergui.bb_challengergui_CHGUI_IgnoreMouse=0;
		bb_challengergui.bb_challengergui_CHGUI_Tooltips=1;
		bb_challengergui.bb_challengergui_CHGUI_TooltipTime=1500;
		bb_challengergui.bb_challengergui_CHGUI_MenuClose=0;
		bb_challengergui.bb_challengergui_CHGUI_TextboxFocus=null;
		bb_challengergui.bb_challengergui_CHGUI_Keyboard=1;
		bb_challengergui.bb_challengergui_CHGUI_ShowKeyboard=0;
		bb_challengergui.bb_challengergui_CHGUI_AutoTextScroll=0;
		bb_challengergui.bb_challengergui_CHGUI_KeyboardPage=0;
		bb_challengergui.bb_challengergui_CHGUI_KeyboardShift=0;
		bb_challengergui.bb_challengergui_CHGUI_OldX=.0f;
		bb_challengergui.bb_challengergui_CHGUI_OldY=.0f;
		bb_challengergui.bb_challengergui_CHGUI_CursorMillisecs=0;
		bb_challengergui.bb_challengergui_CHGUI_DragScroll=0;
		bb_challengergui.bb_challengergui_CHGUI_DragMoving=0;
		bb_challengergui.bb_challengergui_CHGUI_OffsetMX=.0f;
		bb_challengergui.bb_challengergui_CHGUI_OffsetMY=.0f;
		return 0;
	}
}
class bb_data2{
	static String bb_data2_STATUS;
	static float bb_data2_SCALE_W;
	static float bb_data2_SCALE_H;
	static public bb_challengergui_CHGUI bb_data2_CScale(bb_challengergui_CHGUI t_c){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<16>";
		t_c.f_X*=(float)(bb_graphics.bb_graphics_DeviceWidth())/bb_data2.bb_data2_SCALE_W;
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<17>";
		t_c.f_W*=(float)(bb_graphics.bb_graphics_DeviceWidth())/bb_data2.bb_data2_SCALE_W;
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<18>";
		t_c.f_Y*=(float)(bb_graphics.bb_graphics_DeviceHeight())/bb_data2.bb_data2_SCALE_H;
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<19>";
		t_c.f_H*=(float)(bb_graphics.bb_graphics_DeviceHeight())/bb_data2.bb_data2_SCALE_H;
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<20>";
		bb_std_lang.popErr();
		return t_c;
	}
}
class bb_protocol{
	static public int bb_protocol_Post(bb_tcpstream_TcpStream t_stream,String t_url){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<12>";
		t_stream.m_WriteLine("GET "+t_url+" HTTP/1.0");
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<13>";
		t_stream.m_WriteLine(String.valueOf((char)(10)));
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_protocol_RequestGameList(bb_tcpstream_TcpStream t_stream){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<17>";
		bb_protocol.bb_protocol_Post(t_stream,"http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=mobilegetgamelist");
		bb_std_lang.popErr();
		return 0;
	}
	static int bb_protocol_LastP;
	static String[] bb_protocol_SList;
	static public int bb_protocol__readp(bb_tcpstream_TcpStream t_stream){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<36>";
		do{
			bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<37>";
			String t_Dat=t_stream.m_ReadLine();
			bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<38>";
			if(t_Dat.compareTo("__server__protocol__")==0){
				bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<39>";
				break;
			}
		}while(!(false));
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<42>";
		int t_Kind=Integer.parseInt((t_stream.m_ReadLine()).trim());
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<43>";
		int t_=t_Kind;
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<44>";
		if(t_==4){
			bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<45>";
			bb_protocol.bb_protocol_LastP=4;
			bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<46>";
			int t_am=Integer.parseInt((t_stream.m_ReadLine()).trim());
			bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<47>";
			bb_protocol.bb_protocol_SList=(String[])bb_std_lang.resizeStringArray(bb_protocol.bb_protocol_SList,t_am);
			bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<48>";
			for(int t_es=0;t_es<=t_am-1;t_es=t_es+1){
				bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<49>";
				bb_protocol.bb_protocol_SList[t_es]=t_stream.m_ReadLine();
			}
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_protocol_ReadProtocol(bb_tcpstream_TcpStream t_stream){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<29>";
		while((t_stream.m_ReadAvail())!=0){
			bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<30>";
			bb_protocol.bb_protocol__readp(t_stream);
		}
		bb_std_lang.popErr();
		return 0;
	}
	static public int bb_protocol_ResetP(){
		bb_std_lang.pushErr();
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<7>";
		bb_protocol.bb_protocol_LastP=0;
		bb_std_lang.errInfo="J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<8>";
		bb_protocol.bb_protocol_SList=new String[]{""};
		bb_std_lang.popErr();
		return 0;
	}
}
//${TRANSCODE_END}
