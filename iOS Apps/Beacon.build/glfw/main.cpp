
#include "main.h"

//${CONFIG_BEGIN}
#define CFG_BINARY_FILES *.bin|*.dat|*.fuz
#define CFG_CD 
#define CFG_CONFIG debug
#define CFG_GLFW_USE_MINGW 0
#define CFG_GLFW_WINDOW_FULLSCREEN 0
#define CFG_GLFW_WINDOW_HEIGHT 480
#define CFG_GLFW_WINDOW_RESIZABLE 0
#define CFG_GLFW_WINDOW_TITLE iOS Beacon Demo
#define CFG_GLFW_WINDOW_WIDTH 300
#define CFG_HOST winnt
#define CFG_IMAGE_FILES *.png|*.jpg
#define CFG_LANG cpp
#define CFG_MODPATH .;J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps;C:/Program Files (x86)/Monkey/modules
#define CFG_MOJO_AUTO_SUSPEND_ENABLED 0
#define CFG_MOJO_IMAGE_FILTERING_ENABLED 1
#define CFG_MUSIC_FILES *.wav|*.ogg
#define CFG_OPENGL_DEPTH_BUFFER_ENABLED 0
#define CFG_OPENGL_GLES20_ENABLED 0
#define CFG_REFLECTION_FILTER diddy.exception
#define CFG_SAFEMODE 0
#define CFG_SOUND_FILES *.wav|*.ogg
#define CFG_TARGET glfw
#define CFG_TEXT_FILES *.txt|*.xml|*.json
#define CFG_TRANSDIR 
//${CONFIG_END}

#define _QUOTE(X) #X
#define _STRINGIZE( X ) _QUOTE(X)

//For monkey main to set...
int (*runner)();

//${TRANSCODE_BEGIN}

// C++ Monkey runtime.
//
// Placed into the public domain 24/02/2011.
// No warranty implied; use at your own risk.

//***** Monkey Types *****

typedef wchar_t Char;
template<class T> class Array;
class String;
class Object;

#if CFG_CPP_DOUBLE_PRECISION_FLOATS
typedef double Float;
#define FLOAT(X) X
#else
typedef float Float;
#define FLOAT(X) X##f
#endif

void dbg_error( const char *p );

#if !_MSC_VER
#define sprintf_s sprintf
#define sscanf_s sscanf
#endif

//***** GC Config *****

//How many objects to mark per update/render
//
#ifndef CFG_CPP_GC_MARK_RATE
#define CFG_CPP_GC_MARK_RATE 2500
#endif

//How much to alloc before GC - set to 0 for continuous GC
//
#ifndef CFG_CPP_GC_TRIGGER
#define CFG_CPP_GC_TRIGGER 4*1024*1024
#endif

//#define DEBUG_GC 1

// ***** GC *****

#if _WIN32

int gc_micros(){
	static int f;
	static LARGE_INTEGER pcf;
	if( !f ){
		if( QueryPerformanceFrequency( &pcf ) && pcf.QuadPart>=1000000L ){
			pcf.QuadPart/=1000000L;
			f=1;
		}else{
			f=-1;
		}
	}
	if( f>0 ){
		LARGE_INTEGER pc;
		if( QueryPerformanceCounter( &pc ) ) return pc.QuadPart/pcf.QuadPart;
		f=-1;
	}
	return 0;// timeGetTime()*1000;
}

#elif __APPLE__

#include <mach/mach_time.h>

int gc_micros(){
	static int f;
	static mach_timebase_info_data_t timeInfo;
	if( !f ){
		mach_timebase_info( &timeInfo );
		timeInfo.denom*=1000L;
		f=1;
	}
	return mach_absolute_time()*timeInfo.numer/timeInfo.denom;
}

#else

int gc_micros(){
	return 0;
}

#endif

//***** New GC *****

#define gc_mark_roots gc_mark

void gc_mark_roots();

struct gc_object;

gc_object *gc_malloc( int size );
void gc_free( gc_object *p );

struct gc_object{
	gc_object *succ;
	gc_object *pred;
	int flags;
	
	virtual ~gc_object(){
	}
	
	virtual void mark(){
	}
	
	void *operator new( size_t size ){
		return gc_malloc( size );
	}
	
	void operator delete( void *p ){
		gc_free( (gc_object*)p );
	}
};

gc_object gc_free_list;
gc_object gc_marked_list;
gc_object gc_unmarked_list;
gc_object gc_queued_list;

int gc_free_bytes;
int gc_marked_objs;
int gc_marked_bytes;
int gc_alloced_bytes;
int gc_max_alloced_bytes;
int gc_markbit=1;

gc_object *gc_cache[8];

#define GC_LIST_EMPTY( LIST ) ((LIST).succ==&(LIST))

#define GC_REMOVE_NODE( NODE ){\
(NODE)->pred->succ=(NODE)->succ;\
(NODE)->succ->pred=(NODE)->pred;}

#define GC_INSERT_NODE( NODE,SUCC ){\
(NODE)->pred=(SUCC)->pred;\
(NODE)->succ=(SUCC);\
(SUCC)->pred->succ=(NODE);\
(SUCC)->pred=(NODE);}

void gc_init1(){
	gc_free_list.succ=gc_free_list.pred=&gc_free_list;
	gc_marked_list.succ=gc_marked_list.pred=&gc_marked_list;
	gc_unmarked_list.succ=gc_unmarked_list.pred=&gc_unmarked_list;
	gc_queued_list.succ=gc_queued_list.pred=&gc_queued_list;
}

void gc_init2(){
	gc_mark_roots();
}

gc_object *gc_malloc( int size ){

	size=(size+7)&~7;
	
	int t=gc_free_bytes-size;
	while( gc_free_bytes && gc_free_bytes>t ){
		gc_object *p=gc_free_list.succ;
		if( !p || p==&gc_free_list ){
			printf("ERROR:p=%p gc_free_bytes=%i\n",p,gc_free_bytes);
			fflush(stdout);
			gc_free_bytes=0;
			break;
		}
		GC_REMOVE_NODE(p);
		delete p;	//...to gc_free
	}
	
	gc_object *p;
	if( size<64 ){
		if( (p=gc_cache[size>>3]) ){
			gc_cache[size>>3]=p->succ;
		}else{
			p=(gc_object*)malloc( size );
		}
	}else{
		p=(gc_object*)malloc( size );
	}
	
	p->flags=size|gc_markbit;
	
	GC_INSERT_NODE( p,&gc_unmarked_list );
	
	gc_alloced_bytes+=size;
	
	if( gc_alloced_bytes>gc_max_alloced_bytes ) gc_max_alloced_bytes=gc_alloced_bytes;
	
	return p;
}

void gc_free( gc_object *p ){

	int size=p->flags & ~7;
	gc_free_bytes-=size;
	
	if( size<64 ){
		p->succ=gc_cache[size>>3];
		gc_cache[size>>3]=p;
	}else{
		free( p );
	}
}

template<class T> void gc_mark( T *t ){

	gc_object *p=dynamic_cast<gc_object*>(t);
	
	if( p && (p->flags & 3)==gc_markbit ){
		p->flags^=1;
		GC_REMOVE_NODE( p );
		GC_INSERT_NODE( p,&gc_marked_list );
		gc_marked_bytes+=(p->flags & ~7);
		gc_marked_objs+=1;
		p->mark();
	}
}

template<class T> void gc_mark_q( T *t ){

	gc_object *p=dynamic_cast<gc_object*>(t);
	
	if( p && (p->flags & 3)==gc_markbit ){
		p->flags^=1;
		GC_REMOVE_NODE( p );
		GC_INSERT_NODE( p,&gc_queued_list );
	}
}

template<class T,class V> void gc_assign( T *&lhs,V *rhs ){

	gc_object *p=dynamic_cast<gc_object*>(rhs);

	if( p && (p->flags & 3)==gc_markbit ){
		p->flags^=1;
		GC_REMOVE_NODE( p );
		GC_INSERT_NODE( p,&gc_queued_list );
	}

	lhs=rhs;
}

void gc_collect(){

#if DEBUG_GC
	int us=gc_micros();
#endif

	static int last_alloced;
	
	int sweep=0;

#if CFG_CPP_GC_TRIGGER!=0	
	if( gc_alloced_bytes>last_alloced+CFG_CPP_GC_TRIGGER ){
		sweep=1;
	}
#endif	
	
	int tomark=sweep ? 0x7fffffff : gc_marked_objs+CFG_CPP_GC_MARK_RATE;

	while( !GC_LIST_EMPTY( gc_queued_list ) && gc_marked_objs<tomark ){
		gc_object *p=gc_queued_list.succ;
		GC_REMOVE_NODE( p );
		GC_INSERT_NODE( p,&gc_marked_list );
		gc_marked_bytes+=(p->flags & ~7);
		gc_marked_objs+=1;
		p->mark();
	}

#if CFG_CPP_GC_TRIGGER==0
	if( GC_LIST_EMPTY( gc_queued_list ) ){
		sweep=1;
	}
#endif	
	
	int reclaimed_bytes=-1;
	
	if( sweep && !GC_LIST_EMPTY( gc_unmarked_list ) ){
	
		reclaimed_bytes=gc_alloced_bytes-gc_marked_bytes;
		
		//append unmarked list to end of free list
		gc_object *head=gc_unmarked_list.succ;
		gc_object *tail=gc_unmarked_list.pred;
		gc_object *succ=&gc_free_list;
		gc_object *pred=succ->pred;
		head->pred=pred;
		tail->succ=succ;
		pred->succ=head;
		succ->pred=tail;
		
		//move marked to unmarked.
		gc_unmarked_list=gc_marked_list;
		gc_unmarked_list.succ->pred=gc_unmarked_list.pred->succ=&gc_unmarked_list;
		
		//clear marked.
		gc_marked_list.succ=gc_marked_list.pred=&gc_marked_list;
		
		//adjust sizes
		gc_alloced_bytes=gc_marked_bytes;
		gc_free_bytes+=reclaimed_bytes;
		gc_marked_bytes=0;
		gc_marked_objs=0;
		gc_markbit^=1;
		
		gc_mark_roots();
		
		last_alloced=gc_alloced_bytes;
	}

#if DEBUG_GC
	int us2=gc_micros(),us3=us2-us;
	if( reclaimed_bytes>=0 || us3>=1000 ){
		printf("gc_collect :: us:%i reclaimed:%i alloced_bytes:%i max_alloced_bytes:%i free_bytes:%i\n",us2-us,reclaimed_bytes,gc_alloced_bytes,gc_max_alloced_bytes,gc_free_bytes );
	}		
	fflush(stdout);
#endif
}

// ***** Array *****

template<class T> T *t_memcpy( T *dst,const T *src,int n ){
	memcpy( dst,src,n*sizeof(T) );
	return dst+n;
}

template<class T> T *t_memset( T *dst,int val,int n ){
	memset( dst,val,n*sizeof(T) );
	return dst+n;
}

template<class T> int t_memcmp( const T *x,const T *y,int n ){
	return memcmp( x,y,n*sizeof(T) );
}

template<class T> int t_strlen( const T *p ){
	const T *q=p++;
	while( *q++ ){}
	return q-p;
}

template<class T> T *t_create( int n,T *p ){
	t_memset( p,0,n );
	return p+n;
}

template<class T> T *t_create( int n,T *p,const T *q ){
	t_memcpy( p,q,n );
	return p+n;
}

template<class T> void t_destroy( int n,T *p ){
}

//for int, float etc arrays...needs to go before Array<> decl to shut xcode 4.0.2 up.
template<class T> void gc_mark_array( int n,T *p ){
}

template<class T> class Array{
public:
	Array(){
		static Rep null;
		rep=&null;
	}

	//Uses default...
//	Array( const Array<T> &t )...
	
	Array( int length ):rep( Rep::alloc( length ) ){
		t_create( rep->length,rep->data );
	}
	
	Array( const T *p,int length ):rep( Rep::alloc(length) ){
		t_create( rep->length,rep->data,p );
	}
	
	~Array(){
	}

	//Uses default...
//	Array &operator=( const Array &t )...
	
	int Length()const{ 
		return rep->length; 
	}
	
	T &At( int index ){
		if( index<0 || index>=rep->length ) dbg_error( "Array index out of range" );
		return rep->data[index]; 
	}
	
	const T &At( int index )const{
		if( index<0 || index>=rep->length ) dbg_error( "Array index out of range" );
		return rep->data[index]; 
	}
	
	T &operator[]( int index ){
		return rep->data[index]; 
	}

	const T &operator[]( int index )const{
		return rep->data[index]; 
	}
	
	Array Slice( int from,int term )const{
		int len=rep->length;
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
		if( term<=from ) return Array();
		return Array( rep->data+from,term-from );
	}

	Array Slice( int from )const{
		return Slice( from,rep->length );
	}
	
	Array Resize( int newlen )const{
		if( newlen<=0 ) return Array();
		int n=rep->length;
		if( newlen<n ) n=newlen;
		Rep *p=Rep::alloc( newlen );
		T *q=p->data;
		q=t_create( n,q,rep->data );
		q=t_create( (newlen-n),q );
		return Array( p );
	}
	
private:
	struct Rep : public gc_object{
		int length;
		T data[0];
		
		Rep():length(0){
			flags=3;
		}
		
		Rep( int length ):length(length){
		}
		
		~Rep(){
			t_destroy( length,data );
		}
		
		void mark(){
			gc_mark_array( length,data );
		}
		
		static Rep *alloc( int length ){
			static Rep null;
			if( !length ) return &null;
			void *p=gc_malloc( sizeof(Rep)+length*sizeof(T) );
			return ::new(p) Rep( length );
		}
	};
	Rep *rep;
	
	template<class C> friend void gc_mark( Array<C> &t );
	template<class C> friend void gc_mark_q( Array<C> &t );
	template<class C> friend void gc_assign( Array<C> &lhs,Array<C> rhs );
	
	Array( Rep *rep ):rep(rep){
	}
};

template<class T> Array<T> *t_create( int n,Array<T> *p ){
	for( int i=0;i<n;++i ) *p++=Array<T>();
	return p;
}

template<class T> Array<T> *t_create( int n,Array<T> *p,const Array<T> *q ){
	for( int i=0;i<n;++i ) *p++=*q++;
	return p;
}

template<class T> void gc_mark( Array<T> &t ){
	gc_mark( t.rep );
}

template<class T> void gc_mark_q( Array<T> &t ){
	gc_mark_q( t.rep );
}

//for object arrays....
template<class T> void gc_mark_array( int n,T **p ){
	for( int i=0;i<n;++i ) gc_mark( p[i] );
}

//for array arrays...
template<class T> void gc_mark_array( int n,Array<T> *p ){
	for( int i=0;i<n;++i ) gc_mark( p[i] );
}

template<class T> void gc_assign( Array<T> &lhs,Array<T> rhs ){
	gc_mark( rhs.rep );
	lhs=rhs;
}
		
// ***** String *****

class String{
public:
	String():rep( Rep::alloc(0) ){
	}
	
	String( const String &t ):rep( t.rep ){
		rep->retain();
	}

	String( int n ){
		char buf[256];
		sprintf_s( buf,"%i",n );
		rep=Rep::alloc( t_strlen(buf) );
		for( int i=0;i<rep->length;++i ) rep->data[i]=buf[i];
	}
	
	String( Float n ){
		char buf[256];
		
		//would rather use snprintf, but it's doing weird things in MingW.
		//
		sprintf_s( buf,"%.17lg",n );
		//
		char *p;
		for( p=buf;*p;++p ){
			if( *p=='.' || *p=='e' ) break;
		}
		if( !*p ){
			*p++='.';
			*p++='0';
			*p=0;
		}

		rep=Rep::alloc( t_strlen(buf) );
		for( int i=0;i<rep->length;++i ) rep->data[i]=buf[i];
	}

	String( Char ch,int length ):rep( Rep::alloc(length) ){
		for( int i=0;i<length;++i ) rep->data[i]=ch;
	}

	String( const Char *p ):rep( Rep::alloc(t_strlen(p)) ){
		t_memcpy( rep->data,p,rep->length );
	}

	String( const Char *p,int length ):rep( Rep::alloc(length) ){
		t_memcpy( rep->data,p,rep->length );
	}
	
#if __OBJC__	
	String( NSString *nsstr ):rep( Rep::alloc([nsstr length]) ){
		unichar *buf=(unichar*)malloc( rep->length * sizeof(unichar) );
		[nsstr getCharacters:buf range:NSMakeRange(0,rep->length)];
		for( int i=0;i<rep->length;++i ) rep->data[i]=buf[i];
		free( buf );
	}
#endif

	~String(){
		rep->release();
	}
	
	template<class C> String( const C *p ):rep( Rep::alloc(t_strlen(p)) ){
		for( int i=0;i<rep->length;++i ) rep->data[i]=p[i];
	}
	
	template<class C> String( const C *p,int length ):rep( Rep::alloc(length) ){
		for( int i=0;i<rep->length;++i ) rep->data[i]=p[i];
	}
	
	int Length()const{
		return rep->length;
	}
	
	const Char *Data()const{
		return rep->data;
	}
	
	Char operator[]( int index )const{
		return rep->data[index];
	}
	
	String &operator=( const String &t ){
		t.rep->retain();
		rep->release();
		rep=t.rep;
		return *this;
	}
	
	String &operator+=( const String &t ){
		return operator=( *this+t );
	}
	
	int Compare( const String &t )const{
		int n=rep->length<t.rep->length ? rep->length : t.rep->length;
		for( int i=0;i<n;++i ){
			if( int q=(int)(rep->data[i])-(int)(t.rep->data[i]) ) return q;
		}
		return rep->length-t.rep->length;
	}
	
	bool operator==( const String &t )const{
		return rep->length==t.rep->length && t_memcmp( rep->data,t.rep->data,rep->length )==0;
	}
	
	bool operator!=( const String &t )const{
		return rep->length!=t.rep->length || t_memcmp( rep->data,t.rep->data,rep->length )!=0;
	}
	
	bool operator<( const String &t )const{
		return Compare( t )<0;
	}
	
	bool operator<=( const String &t )const{
		return Compare( t )<=0;
	}
	
	bool operator>( const String &t )const{
		return Compare( t )>0;
	}
	
	bool operator>=( const String &t )const{
		return Compare( t )>=0;
	}
	
	String operator+( const String &t )const{
		if( !rep->length ) return t;
		if( !t.rep->length ) return *this;
		Rep *p=Rep::alloc( rep->length+t.rep->length );
		Char *q=p->data;
		q=t_memcpy( q,rep->data,rep->length );
		q=t_memcpy( q,t.rep->data,t.rep->length );
		return String( p );
	}
	
	int Find( String find,int start=0 )const{
		if( start<0 ) start=0;
		while( start+find.rep->length<=rep->length ){
			if( !t_memcmp( rep->data+start,find.rep->data,find.rep->length ) ) return start;
			++start;
		}
		return -1;
	}
	
	int FindLast( String find )const{
		int start=rep->length-find.rep->length;
		while( start>=0 ){
			if( !t_memcmp( rep->data+start,find.rep->data,find.rep->length ) ) return start;
			--start;
		}
		return -1;
	}
	
	int FindLast( String find,int start )const{
		if( start>rep->length-find.rep->length ) start=rep->length-find.rep->length;
		while( start>=0 ){
			if( !t_memcmp( rep->data+start,find.rep->data,find.rep->length ) ) return start;
			--start;
		}
		return -1;
	}
	
	String Trim()const{
		int i=0,i2=rep->length;
		while( i<i2 && rep->data[i]<=32 ) ++i;
		while( i2>i && rep->data[i2-1]<=32 ) --i2;
		if( i==0 && i2==rep->length ) return *this;
		return String( rep->data+i,i2-i );
	}

	Array<String> Split( String sep )const{
	
		if( !sep.rep->length ){
			Array<String> bits( rep->length );
			for( int i=0;i<rep->length;++i ){
				bits[i]=String( (Char)(*this)[i],1 );
			}
			return bits;
		}
		
		int i=0,i2,n=1;
		while( (i2=Find( sep,i ))!=-1 ){
			++n;
			i=i2+sep.rep->length;
		}
		Array<String> bits( n );
		if( n==1 ){
			bits[0]=*this;
			return bits;
		}
		i=0;n=0;
		while( (i2=Find( sep,i ))!=-1 ){
			bits[n++]=Slice( i,i2 );
			i=i2+sep.rep->length;
		}
		bits[n]=Slice( i );
		return bits;
	}

	String Join( Array<String> bits )const{
		if( bits.Length()==0 ) return String();
		if( bits.Length()==1 ) return bits[0];
		int newlen=rep->length * (bits.Length()-1);
		for( int i=0;i<bits.Length();++i ){
			newlen+=bits[i].rep->length;
		}
		Rep *p=Rep::alloc( newlen );
		Char *q=p->data;
		q=t_memcpy( q,bits[0].rep->data,bits[0].rep->length );
		for( int i=1;i<bits.Length();++i ){
			q=t_memcpy( q,rep->data,rep->length );
			q=t_memcpy( q,bits[i].rep->data,bits[i].rep->length );
		}
		return String( p );
	}

	String Replace( String find,String repl )const{
		int i=0,i2,newlen=0;
		while( (i2=Find( find,i ))!=-1 ){
			newlen+=(i2-i)+repl.rep->length;
			i=i2+find.rep->length;
		}
		if( !i ) return *this;
		newlen+=rep->length-i;
		Rep *p=Rep::alloc( newlen );
		Char *q=p->data;
		i=0;
		while( (i2=Find( find,i ))!=-1 ){
			q=t_memcpy( q,rep->data+i,i2-i );
			q=t_memcpy( q,repl.rep->data,repl.rep->length );
			i=i2+find.rep->length;
		}
		q=t_memcpy( q,rep->data+i,rep->length-i );
		return String( p );
	}

	String ToLower()const{
		for( int i=0;i<rep->length;++i ){
			Char t=tolower( rep->data[i] );
			if( t==rep->data[i] ) continue;
			Rep *p=Rep::alloc( rep->length );
			Char *q=p->data;
			t_memcpy( q,rep->data,i );
			for( q[i++]=t;i<rep->length;++i ){
				q[i]=tolower( rep->data[i] );
			}
			return String( p );
		}
		return *this;
	}

	String ToUpper()const{
		for( int i=0;i<rep->length;++i ){
			Char t=toupper( rep->data[i] );
			if( t==rep->data[i] ) continue;
			Rep *p=Rep::alloc( rep->length );
			Char *q=p->data;
			t_memcpy( q,rep->data,i );
			for( q[i++]=t;i<rep->length;++i ){
				q[i]=toupper( rep->data[i] );
			}
			return String( p );
		}
		return *this;
	}
	
	bool Contains( String sub )const{
		return Find( sub )!=-1;
	}

	bool StartsWith( String sub )const{
		return sub.rep->length<=rep->length && !t_memcmp( rep->data,sub.rep->data,sub.rep->length );
	}

	bool EndsWith( String sub )const{
		return sub.rep->length<=rep->length && !t_memcmp( rep->data+rep->length-sub.rep->length,sub.rep->data,sub.rep->length );
	}
	
	String Slice( int from,int term )const{
		int len=rep->length;
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
		if( term<from ) return String();
		if( from==0 && term==len ) return *this;
		return String( rep->data+from,term-from );
	}

	String Slice( int from )const{
		return Slice( from,rep->length );
	}
	
	Array<int> ToChars()const{
		Array<int> chars( rep->length );
		for( int i=0;i<rep->length;++i ) chars[i]=rep->data[i];
		return chars;
	}
	
	int ToInt()const{
		return atoi( ToCString<char>() );
	}
	
	Float ToFloat()const{
		return atof( ToCString<char>() );
	}
	
	template<class C> C *ToCString()const{

		C *p=&Array<C>( rep->length+1 )[0];
		
		for( int i=0;i<rep->length;++i ) p[i]=rep->data[i];
		p[rep->length]=0;
		return p;
	}

#if __OBJC__	
	NSString *ToNSString()const{
		return [NSString stringWithCharacters:ToCString<unichar>() length:rep->length];
	}
#endif

	bool Save( FILE *fp ){
		std::vector<unsigned char> buf;
		Save( buf );
		return buf.size() ? fwrite( &buf[0],1,buf.size(),fp )==buf.size() : true;
	}
	
	void Save( std::vector<unsigned char> &buf ){
	
		Char *p=rep->data;
		Char *e=p+rep->length;
		
		while( p<e ){
			Char c=*p++;
			if( c<0x80 ){
				buf.push_back( c );
			}else if( c<0x800 ){
				buf.push_back( 0xc0 | (c>>6) );
				buf.push_back( 0x80 | (c & 0x3f) );
			}else{
				buf.push_back( 0xe0 | (c>>12) );
				buf.push_back( 0x80 | ((c>>6) & 0x3f) );
				buf.push_back( 0x80 | (c & 0x3f) );
			}
		}
	}
	
	static String FromChars( Array<int> chars ){
		int n=chars.Length();
		Rep *p=Rep::alloc( n );
		for( int i=0;i<n;++i ){
			p->data[i]=chars[i];
		}
		return String( p );
	}

	static String Load( FILE *fp ){
		unsigned char tmp[4096];
		std::vector<unsigned char> buf;
		for(;;){
			int n=fread( tmp,1,4096,fp );
			if( n>0 ) buf.insert( buf.end(),tmp,tmp+n );
			if( n!=4096 ) break;
		}
		return buf.size() ? String::Load( &buf[0],buf.size() ) : String();
	}
	
	static String Load( unsigned char *p,int n ){
	
		unsigned char *e=p+n;
		std::vector<Char> chars;
		
		int t0=n>0 ? p[0] : -1;
		int t1=n>1 ? p[1] : -1;

		if( t0==0xfe && t1==0xff ){
			p+=2;
			while( p<e-1 ){
				int c=*p++;
				chars.push_back( (c<<8)|*p++ );
			}
		}else if( t0==0xff && t1==0xfe ){
			p+=2;
			while( p<e-1 ){
				int c=*p++;
				chars.push_back( (*p++<<8)|c );
			}
		}else{
			int t2=n>2 ? p[2] : -1;
			if( t0==0xef && t1==0xbb && t2==0xbf ) p+=3;
			unsigned char *q=p;
			bool fail=false;
			while( p<e ){
				unsigned int c=*p++;
				if( c & 0x80 ){
					if( (c & 0xe0)==0xc0 ){
						if( p>=e || (p[0] & 0xc0)!=0x80 ){
							fail=true;
							break;
						}
						c=((c & 0x1f)<<6) | (p[0] & 0x3f);
						p+=1;
					}else if( (c & 0xf0)==0xe0 ){
						if( p+1>=e || (p[0] & 0xc0)!=0x80 || (p[1] & 0xc0)!=0x80 ){
							fail=true;
							break;
						}
						c=((c & 0x0f)<<12) | ((p[0] & 0x3f)<<6) | (p[1] & 0x3f);
						p+=2;
					}else{
						fail=true;
						break;
					}
				}
				chars.push_back( c );
			}
			if( fail ){
				puts( "Invalid UTF-8!" );fflush( stdout );
				return String( q,n );
			}
		}
		return chars.size() ? String( &chars[0],chars.size() ) : String();
	}
	
private:
	struct Rep{
		int refs;
		int length;
		Char data[0];
		
		Rep():refs(1),length(0){
		}
		
		Rep( int length ):refs(1),length(length){
		}
		
		void retain(){
			++refs;
		}
		
		void release(){
			if( --refs || !length ) return;
			free( this );
		}

		static Rep *alloc( int length ){
			if( !length ){
				static Rep null;
				return &null;
			}
			void *p=malloc( sizeof(Rep)+length*sizeof(Char) );
			return new(p) Rep( length );
		}
	};
	Rep *rep;
	
	String( Rep *rep ):rep(rep){
	}
};

String *t_create( int n,String *p ){
	for( int i=0;i<n;++i ) new( &p[i] ) String();
	return p+n;
}

String *t_create( int n,String *p,const String *q ){
	for( int i=0;i<n;++i ) new( &p[i] ) String( q[i] );
	return p+n;
}

void t_destroy( int n,String *p ){
	for( int i=0;i<n;++i ) p[i].~String();
}

String T( const char *p ){
	return String( p );
}

String T( const wchar_t *p ){
	return String( p );
}

// ***** Object *****

class Object : public gc_object{
public:
	virtual bool Equals( Object *obj ){
		return this==obj;
	}
	
	virtual int Compare( Object *obj ){
		return (char*)this-(char*)obj;
	}
	
	virtual String debug(){
		return "+Object\n";
	}
};

class ThrowableObject : public Object{
};

struct gc_interface{
	virtual ~gc_interface(){}
};


//***** Debugger *****

int Print( String t );

#define dbg_stream stderr

#if _MSC_VER
#define dbg_typeof decltype
#else
#define dbg_typeof __typeof__
#endif 

struct dbg_func;
struct dbg_var_type;

static int dbg_suspend;
static int dbg_stepmode;

const char *dbg_info;
String dbg_exstack;

static void *dbg_var_buf[65536*3];
static void **dbg_var_ptr=dbg_var_buf;

static dbg_func *dbg_func_buf[1024];
static dbg_func **dbg_func_ptr=dbg_func_buf;

String dbg_type( bool *p ){
	return "Bool";
}

String dbg_type( int *p ){
	return "Int";
}

String dbg_type( Float *p ){
	return "Float";
}

String dbg_type( String *p ){
	return "String";
}

template<class T> String dbg_type( T *p ){
	return "Object";
}

template<class T> String dbg_type( Array<T> *p ){
	return dbg_type( &(*p)[0] )+"[]";
}

String dbg_value( bool *p ){
	return *p ? "True" : "False";
}

String dbg_value( int *p ){
	return String( *p );
}

String dbg_value( Float *p ){
	return String( *p );
}

String dbg_value( String *p ){
	String t=*p;
	if( t.Length()>100 ) t=t.Slice( 0,100 )+"...";
	t=t.Replace( "\"","~q" );
	t=t.Replace( "\t","~t" );
	t=t.Replace( "\n","~n" );
	t=t.Replace( "\r","~r" );
	return String("\"")+t+"\"";
}

template<class T> String dbg_value( T *t ){
	Object *p=dynamic_cast<Object*>( *t );
	char buf[64];
	sprintf_s( buf,"%p",p );
	return String("@") + (buf[0]=='0' && buf[1]=='x' ? buf+2 : buf );
}

template<class T> String dbg_value( Array<T> *p ){
	String t="[";
	int n=(*p).Length();
	for( int i=0;i<n;++i ){
		if( i ) t+=",";
		t+=dbg_value( &(*p)[i] );
	}
	return t+"]";
}

template<class T> String dbg_decl( const char *id,T *ptr ){
	return String( id )+":"+dbg_type(ptr)+"="+dbg_value(ptr)+"\n";
}

struct dbg_var_type{
	virtual String type( void *p )=0;
	virtual String value( void *p )=0;
};

template<class T> struct dbg_var_type_t : public dbg_var_type{

	String type( void *p ){
		return dbg_type( (T*)p );
	}
	
	String value( void *p ){
		return dbg_value( (T*)p );
	}
	
	static dbg_var_type_t<T> info;
};
template<class T> dbg_var_type_t<T> dbg_var_type_t<T>::info;

struct dbg_blk{
	void **var_ptr;
	
	dbg_blk():var_ptr(dbg_var_ptr){
		if( dbg_stepmode=='l' ) --dbg_suspend;
	}
	
	~dbg_blk(){
		if( dbg_stepmode=='l' ) ++dbg_suspend;
		dbg_var_ptr=var_ptr;
	}
};

struct dbg_func : public dbg_blk{
	const char *id;
	const char *info;

	dbg_func( const char *p ):id(p),info(dbg_info){
		*dbg_func_ptr++=this;
		if( dbg_stepmode=='s' ) --dbg_suspend;
	}
	
	~dbg_func(){
		if( dbg_stepmode=='s' ) ++dbg_suspend;
		--dbg_func_ptr;
		dbg_info=info;
	}
};

int dbg_print( String t ){
	static char *buf;
	static int len;
	int n=t.Length();
	if( n+100>len ){
		len=n+100;
		free( buf );
		buf=(char*)malloc( len );
	}
	buf[n]='\n';
	for( int i=0;i<n;++i ) buf[i]=t[i];
	fwrite( buf,n+1,1,dbg_stream );
	fflush( dbg_stream );
	return 0;
}

void dbg_callstack(){

	void **var_ptr=dbg_var_buf;
	dbg_func **func_ptr=dbg_func_buf;
	
	while( var_ptr!=dbg_var_ptr ){
		while( func_ptr!=dbg_func_ptr && var_ptr==(*func_ptr)->var_ptr ){
			const char *id=(*func_ptr++)->id;
			const char *info=func_ptr!=dbg_func_ptr ? (*func_ptr)->info : dbg_info;
			fprintf( dbg_stream,"+%s;%s\n",id,info );
		}
		void *vp=*var_ptr++;
		const char *nm=(const char*)*var_ptr++;
		dbg_var_type *ty=(dbg_var_type*)*var_ptr++;
		dbg_print( String(nm)+":"+ty->type(vp)+"="+ty->value(vp) );
	}
	while( func_ptr!=dbg_func_ptr ){
		const char *id=(*func_ptr++)->id;
		const char *info=func_ptr!=dbg_func_ptr ? (*func_ptr)->info : dbg_info;
		fprintf( dbg_stream,"+%s;%s\n",id,info );
	}
}

String dbg_stacktrace(){
	if( !dbg_info || !dbg_info[0] ) return "";
	String str=String( dbg_info )+"\n";
	dbg_func **func_ptr=dbg_func_ptr;
	if( func_ptr==dbg_func_buf ) return str;
	while( --func_ptr!=dbg_func_buf ){
		str+=String( (*func_ptr)->info )+"\n";
	}
	return str;
}

void dbg_throw( const char *err ){
	dbg_exstack=dbg_stacktrace();
	throw err;
}

void dbg_stop(){

#ifdef TARGET_OS_IPHONE
	dbg_throw( "STOP" );
#endif

	fprintf( dbg_stream,"{{~~%s~~}}\n",dbg_info );
	dbg_callstack();
	dbg_print( "" );
	
	for(;;){

		char buf[256];
		char *e=fgets( buf,256,stdin );
		if( !e ) exit( -1 );
		
		e=strchr( buf,'\n' );
		if( !e ) exit( -1 );
		
		*e=0;
		
		Object *p;
		
		switch( buf[0] ){
		case '?':
			break;
		case 'r':	//run
			dbg_suspend=0;		
			dbg_stepmode=0;
			return;
		case 's':	//step
			dbg_suspend=1;
			dbg_stepmode='s';
			return;
		case 'e':	//enter func
			dbg_suspend=1;
			dbg_stepmode='e';
			return;
		case 'l':	//leave block
			dbg_suspend=0;
			dbg_stepmode='l';
			return;
		case '@':	//dump object
			p=0;
			sscanf_s( buf+1,"%p",&p );
			if( p ){
				dbg_print( p->debug() );
			}else{
				dbg_print( "" );
			}
			break;
		case 'q':	//quit!
			exit( 0 );
			break;			
		default:
			printf( "????? %s ?????",buf );fflush( stdout );
			exit( -1 );
		}
	}
}

void dbg_error( const char *err ){

#ifdef TARGET_OS_IPHONE
	dbg_throw( err );
#endif

	for(;;){
		Print( String("Monkey Runtime Error : ")+err );
		Print( dbg_stacktrace() );
		dbg_stop();
	}
}

#define DBG_INFO(X) dbg_info=(X);if( dbg_suspend>0 ) dbg_stop();

#define DBG_ENTER(P) dbg_func _dbg_func(P);

#define DBG_BLOCK(T) dbg_blk _dbg_blk;

#define DBG_GLOBAL( ID,NAME )	//TODO!

#define DBG_LOCAL( ID,NAME )\
*dbg_var_ptr++=&ID;\
*dbg_var_ptr++=(void*)NAME;\
*dbg_var_ptr++=&dbg_var_type_t<dbg_typeof(ID)>::info;

//**** main ****

int argc;
const char **argv;

Float D2R=0.017453292519943295f;
Float R2D=57.29577951308232f;

int Print( String t ){
	static char *buf;
	static int len;
	int n=t.Length();
	if( n+100>len ){
		len=n+100;
		free( buf );
		buf=(char*)malloc( len );
	}
	for( int i=0;i<n;++i ) buf[i]=t[i];
	buf[n]=0;
	puts( buf );
	fflush( stdout );
	return 0;
}

int Error( String err ){
	if( !err.Length() ) exit( 0 );
	dbg_error( err.ToCString<char>() );
	return 0;
}

int DebugLog( String t ){
	Print( t );
	return 0;
}

int DebugStop(){
	dbg_stop();
	return 0;
}

int bbInit();
int bbMain();

#if _MSC_VER

static void _cdecl seTranslator( unsigned int ex,EXCEPTION_POINTERS *p ){

	switch( ex ){
	case EXCEPTION_ACCESS_VIOLATION:dbg_error( "Memory access violation" );
	case EXCEPTION_ILLEGAL_INSTRUCTION:dbg_error( "Illegal instruction" );
	case EXCEPTION_INT_DIVIDE_BY_ZERO:dbg_error( "Integer divide by zero" );
	case EXCEPTION_STACK_OVERFLOW:dbg_error( "Stack overflow" );
	}
	dbg_error( "Unknown exception" );
}

#else

void sighandler( int sig  ){
	switch( sig ){
	case SIGSEGV:dbg_error( "Memory access violation" );
	case SIGILL:dbg_error( "Illegal instruction" );
	case SIGFPE:dbg_error( "Floating point exception" );
#if !_WIN32
	case SIGBUS:dbg_error( "Bus error" );
#endif	
	}
	dbg_error( "Unknown signal" );
}

#endif

//entry point call by target main()...
//
int bb_std_main( int argc,const char **argv ){

	::argc=argc;
	::argv=argv;
	
#if _MSC_VER

	_set_se_translator( seTranslator );

#else
	
	signal( SIGSEGV,sighandler );
	signal( SIGILL,sighandler );
	signal( SIGFPE,sighandler );
#if !_WIN32
	signal( SIGBUS,sighandler );
#endif

#endif

	gc_init1();

	bbInit();
	
	gc_init2();

	bbMain();

	return 0;
}

// ***** databuffer.h *****

class BBDataBuffer : public Object{

public:
	
	BBDataBuffer();
	~BBDataBuffer();
	
	void Discard();
	
	int Length();
	
	const void *ReadPointer( int offset );
	void *WritePointer( int offset );
	
	void PokeByte( int addr,int value );
	void PokeShort( int addr,int value );
	void PokeInt( int addr,int value );
	void PokeFloat( int addr,float value );
	
	int PeekByte( int addr );
	int PeekShort( int addr );
	int PeekInt( int addr );
	float PeekFloat( int addr );
	
	bool LoadBuffer( String path );
	bool CreateBuffer( int length );
	
	bool _New( int length );
	bool _Load( String path );

private:
	signed char *_data;
	int _length;
};

// ***** databuffer.cpp *****

//Forward refs to data functions.
FILE *fopenFile( String path,String mode );

BBDataBuffer::BBDataBuffer():_data(0),_length(0){
}

BBDataBuffer::~BBDataBuffer(){
	if( _data ) free( _data );
}

bool BBDataBuffer::_New( int length ){
	if( _data ) return false;
	_data=(signed char*)malloc( length );
	_length=length;
	return true;
}

bool BBDataBuffer::_Load( String path ){
	if( _data ) return false;
	if( FILE *f=fopenFile( path,"rb" ) ){
		const int BUF_SZ=4096;
		std::vector<void*> tmps;
		for(;;){
			void *p=malloc( BUF_SZ );
			int n=fread( p,1,BUF_SZ,f );
			tmps.push_back( p );
			_length+=n;
			if( n!=BUF_SZ ) break;
		}
		fclose( f );
		_data=(signed char*)malloc( _length );
		signed char *p=_data;
		int sz=_length;
		for( int i=0;i<tmps.size();++i ){
			int n=sz>BUF_SZ ? BUF_SZ : sz;
			memcpy( p,tmps[i],n );
			free( tmps[i] );
			sz-=n;
		}
		return true;
	}
	return false;
}

void BBDataBuffer::Discard(){
	if( !_data ) return;
	free( _data );
	_data=0;
	_length=0;
}

int BBDataBuffer::Length(){
	return _length;
}

const void *BBDataBuffer::ReadPointer( int offset ){
	return _data+offset;
}

void *BBDataBuffer::WritePointer( int offset ){
	return _data+offset;
}

void BBDataBuffer::PokeByte( int addr,int value ){
	*(_data+addr)=value;
}

void BBDataBuffer::PokeShort( int addr,int value ){
	*(short*)(_data+addr)=value;
}

void BBDataBuffer::PokeInt( int addr,int value ){
	*(int*)(_data+addr)=value;
}

void BBDataBuffer::PokeFloat( int addr,float value ){
	*(float*)(_data+addr)=value;
}

int BBDataBuffer::PeekByte( int addr ){
	return *(_data+addr);
}

int BBDataBuffer::PeekShort( int addr ){
	return *(short*)(_data+addr);
}

int BBDataBuffer::PeekInt( int addr ){
	return *(int*)(_data+addr);
}

float BBDataBuffer::PeekFloat( int addr ){
	return *(float*)(_data+addr);
}

// GLFW mojo runtime.
//
// Copyright 2011 Mark Sibly, all rights reserved.
// No warranty implied; use at your own risk.

#ifndef GL_BGRA
#define GL_BGRA  0x80e1
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812f
#endif

#ifndef GL_GENERATE_MIPMAP
#define GL_GENERATE_MIPMAP 0x8191
#endif

class gxtkApp;
class gxtkGraphics;
class gxtkSurface;
class gxtkInput;
class gxtkAudio;
class gxtkSample;

#define KEY_LMB 1
#define KEY_RMB 2
#define KEY_MMB 3
#define KEY_TOUCH0 0x180

int Pow2Size( int n ){
	int i=1;
	while( i<n ) i*=2;
	return i;
}

//Forward refs to data functions.
FILE *fopenFile( String path,String mode );

unsigned char *loadImage( String path,int *width,int *height,int *format );
unsigned char *loadImage( unsigned char *buf,int len,int *width,int *height,int *format );
void unloadImage( unsigned char *data );

unsigned char *loadSound( String path,int *length,int *channels,int *format,int *hertz );
void unloadSound( unsigned char *data );

enum{
	VKEY_BACKSPACE=8,VKEY_TAB,
	VKEY_ENTER=13,
	VKEY_SHIFT=16,
	VKEY_CONTROL=17,
	VKEY_ESC=27,
	VKEY_SPACE=32,
	VKEY_PAGEUP=33,VKEY_PAGEDOWN,VKEY_END,VKEY_HOME,
	VKEY_LEFT=37,VKEY_UP,VKEY_RIGHT,VKEY_DOWN,
	VKEY_INSERT=45,VKEY_DELETE,
	VKEY_0=48,VKEY_1,VKEY_2,VKEY_3,VKEY_4,VKEY_5,VKEY_6,VKEY_7,VKEY_8,VKEY_9,
	VKEY_A=65,VKEY_B,VKEY_C,VKEY_D,VKEY_E,VKEY_F,VKEY_G,VKEY_H,VKEY_I,VKEY_J,
	VKEY_K,VKEY_L,VKEY_M,VKEY_N,VKEY_O,VKEY_P,VKEY_Q,VKEY_R,VKEY_S,VKEY_T,
	VKEY_U,VKEY_V,VKEY_W,VKEY_X,VKEY_Y,VKEY_Z,
	
	VKEY_LSYS=91,VKEY_RSYS,
	
	VKEY_NUM0=96,VKEY_NUM1,VKEY_NUM2,VKEY_NUM3,VKEY_NUM4,
	VKEY_NUM5,VKEY_NUM6,VKEY_NUM7,VKEY_NUM8,VKEY_NUM9,
	VKEY_NUMMULTIPLY=106,VKEY_NUMADD,VKEY_NUMSLASH,
	VKEY_NUMSUBTRACT,VKEY_NUMDECIMAL,VKEY_NUMDIVIDE,

	VKEY_F1=112,VKEY_F2,VKEY_F3,VKEY_F4,VKEY_F5,VKEY_F6,
	VKEY_F7,VKEY_F8,VKEY_F9,VKEY_F10,VKEY_F11,VKEY_F12,

	VKEY_LSHIFT=160,VKEY_RSHIFT,
	VKEY_LCONTROL=162,VKEY_RCONTROL,
	VKEY_LALT=164,VKEY_RALT,

	VKEY_TILDE=192,VKEY_MINUS=189,VKEY_EQUALS=187,
	VKEY_OPENBRACKET=219,VKEY_BACKSLASH=220,VKEY_CLOSEBRACKET=221,
	VKEY_SEMICOLON=186,VKEY_QUOTES=222,
	VKEY_COMMA=188,VKEY_PERIOD=190,VKEY_SLASH=191
};

//glfw key to monkey key!
int TransKey( int key ){

	if( key>='0' && key<='9' ) return key;
	if( key>='A' && key<='Z' ) return key;

	switch( key ){

	case ' ':return VKEY_SPACE;
	case ';':return VKEY_SEMICOLON;
	case '=':return VKEY_EQUALS;
	case ',':return VKEY_COMMA;
	case '-':return VKEY_MINUS;
	case '.':return VKEY_PERIOD;
	case '/':return VKEY_SLASH;
	case '~':return VKEY_TILDE;
	case '[':return VKEY_OPENBRACKET;
	case ']':return VKEY_CLOSEBRACKET;
	case '\"':return VKEY_QUOTES;
	case '\\':return VKEY_BACKSLASH;
	
	case '`':return VKEY_TILDE;
	case '\'':return VKEY_QUOTES;
	
	case GLFW_KEY_LSHIFT:return VKEY_LSHIFT;
	case GLFW_KEY_RSHIFT:return VKEY_RSHIFT;
	case GLFW_KEY_LCTRL:return VKEY_LCONTROL;
	case GLFW_KEY_RCTRL:return VKEY_RCONTROL;
	
	case GLFW_KEY_BACKSPACE:return VKEY_BACKSPACE;
	case GLFW_KEY_TAB:return VKEY_TAB;
	case GLFW_KEY_ENTER:return VKEY_ENTER;
	case GLFW_KEY_ESC:return VKEY_ESC;
	case GLFW_KEY_INSERT:return VKEY_INSERT;
	case GLFW_KEY_DEL:return VKEY_DELETE;
	case GLFW_KEY_PAGEUP:return VKEY_PAGEUP;
	case GLFW_KEY_PAGEDOWN:return VKEY_PAGEDOWN;
	case GLFW_KEY_HOME:return VKEY_HOME;
	case GLFW_KEY_END:return VKEY_END;
	case GLFW_KEY_UP:return VKEY_UP;
	case GLFW_KEY_DOWN:return VKEY_DOWN;
	case GLFW_KEY_LEFT:return VKEY_LEFT;
	case GLFW_KEY_RIGHT:return VKEY_RIGHT;
	
	case GLFW_KEY_F1:return VKEY_F1;
	case GLFW_KEY_F2:return VKEY_F2;
	case GLFW_KEY_F3:return VKEY_F3;
	case GLFW_KEY_F4:return VKEY_F4;
	case GLFW_KEY_F5:return VKEY_F5;
	case GLFW_KEY_F6:return VKEY_F6;
	case GLFW_KEY_F7:return VKEY_F7;
	case GLFW_KEY_F8:return VKEY_F8;
	case GLFW_KEY_F9:return VKEY_F9;
	case GLFW_KEY_F10:return VKEY_F10;
	case GLFW_KEY_F11:return VKEY_F11;
	case GLFW_KEY_F12:return VKEY_F12;
	}
	return 0;
}

//monkey key to special monkey char
int KeyToChar( int key ){
	switch( key ){
	case VKEY_BACKSPACE:
	case VKEY_TAB:
	case VKEY_ENTER:
	case VKEY_ESC:
		return key;
	case VKEY_PAGEUP:
	case VKEY_PAGEDOWN:
	case VKEY_END:
	case VKEY_HOME:
	case VKEY_LEFT:
	case VKEY_UP:
	case VKEY_RIGHT:
	case VKEY_DOWN:
	case VKEY_INSERT:
		return key | 0x10000;
	case VKEY_DELETE:
		return 127;
	}
	return 0;
}

gxtkApp *app;

class gxtkObject : public Object{
public:
};

class gxtkApp : public gxtkObject{
public:
	gxtkGraphics *graphics;
	gxtkInput *input;
	gxtkAudio *audio;
	
	int updateRate;
	double nextUpdate;
	double updatePeriod;
	
	bool suspended;
	
	gxtkApp();
	
	void Run();
	
	static void GLFWCALL OnWindowRefresh();
	static void GLFWCALL OnWindowSize( int width,int height );
	static void GLFWCALL OnKey( int key,int action );
	static void GLFWCALL OnChar( int chr,int action );
	static void GLFWCALL OnMouseButton( int button,int action );
	
	void InvokeOnCreate();
	void InvokeOnSuspend();
	void InvokeOnResume();
	void InvokeOnUpdate();
	void InvokeOnRender();
	
	//***** GXTK API *****

	virtual gxtkGraphics *GraphicsDevice();
	virtual gxtkInput *InputDevice();
	virtual gxtkAudio *AudioDevice();
	virtual String AppTitle();
	virtual String LoadState();
	virtual int SaveState( String state );
	virtual String LoadString( String path );
	virtual int SetUpdateRate( int hertz );
	virtual int MilliSecs();
	virtual int Loading();
	
	virtual int OnCreate();
	virtual int OnSuspend();
	virtual int OnResume();
	
	virtual int OnUpdate();
	virtual int OnRender();
	virtual int OnLoading();
};

//***** START OF COMMON OPENGL CODE *****

#define MAX_VERTS 1024
#define MAX_POINTS MAX_VERTS
#define MAX_LINES (MAX_VERTS/2)
#define MAX_QUADS (MAX_VERTS/4)

class gxtkGraphics : public gxtkObject{
public:

	int mode;
	int width;
	int height;

	int colorARGB;
	float r,g,b,alpha;
	float ix,iy,jx,jy,tx,ty;
	bool tformed;

	float vertices[MAX_VERTS*5];
	unsigned short quadIndices[MAX_QUADS*6];

	int primType;
	int primCount;
	gxtkSurface *primSurf;
	
	gxtkGraphics();
	
	bool Validate();		
	void BeginRender();
	void EndRender();
	void Flush();
	
	//***** GXTK API *****
	virtual int Mode();
	virtual int Width();
	virtual int Height();

	virtual gxtkSurface *LoadSurface( String path );
	virtual gxtkSurface *LoadSurface__UNSAFE__( gxtkSurface *surface,String path );
	virtual gxtkSurface *CreateSurface( int width,int height );
//	virtual gxtkSurface *CreateSurface2( BBDataBuffer *data );
	
	virtual int Cls( float r,float g,float b );
	virtual int SetAlpha( float alpha );
	virtual int SetColor( float r,float g,float b );
	virtual int SetBlend( int blend );
	virtual int SetScissor( int x,int y,int w,int h );
	virtual int SetMatrix( float ix,float iy,float jx,float jy,float tx,float ty );
	
	virtual int DrawPoint( float x,float y );
	virtual int DrawRect( float x,float y,float w,float h );
	virtual int DrawLine( float x1,float y1,float x2,float y2 );
	virtual int DrawOval( float x1,float y1,float x2,float y2 );
	virtual int DrawPoly( Array<float> verts );
	virtual int DrawSurface( gxtkSurface *surface,float x,float y );
	virtual int DrawSurface2( gxtkSurface *surface,float x,float y,int srcx,int srcy,int srcw,int srch );
	
	virtual int ReadPixels( Array<int> pixels,int x,int y,int width,int height,int offset,int pitch );
	virtual int WritePixels2( gxtkSurface *surface,Array<int> pixels,int x,int y,int width,int height,int offset,int pitch );
};

//***** gxtkSurface *****

class gxtkSurface : public gxtkObject{
public:
	unsigned char *data;
	int width;
	int height;
	int depth;
	GLuint texture;
	float uscale;
	float vscale;
	
	gxtkSurface();
	gxtkSurface( unsigned char *data,int width,int height,int depth );
	
	void SetData( unsigned char *data,int width,int height,int depth );
	
	~gxtkSurface();
	
	//***** GXTK API *****
	virtual int Discard();
	virtual int Width();
	virtual int Height();
	virtual int Loaded();
	virtual bool OnUnsafeLoadComplete();
};

//***** gxtkGraphics *****

gxtkGraphics::gxtkGraphics(){

	mode=width=height=0;
	
	if( CFG_OPENGL_GLES20_ENABLED ) return;
	
	mode=1;
	
	for( int i=0;i<MAX_QUADS;++i ){
		quadIndices[i*6  ]=(short)(i*4);
		quadIndices[i*6+1]=(short)(i*4+1);
		quadIndices[i*6+2]=(short)(i*4+2);
		quadIndices[i*6+3]=(short)(i*4);
		quadIndices[i*6+4]=(short)(i*4+2);
		quadIndices[i*6+5]=(short)(i*4+3);
	}
}

void gxtkGraphics::BeginRender(){
	if( !mode ) return;
	
	glViewport( 0,0,width,height );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0,width,height,0,-1,1 );
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	
	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 2,GL_FLOAT,20,&vertices[0] );	
	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer( 2,GL_FLOAT,20,&vertices[2] );
	
	glEnableClientState( GL_COLOR_ARRAY );
	glColorPointer( 4,GL_UNSIGNED_BYTE,20,&vertices[4] );
	
	glEnable( GL_BLEND );
	glBlendFunc( GL_ONE,GL_ONE_MINUS_SRC_ALPHA );
	
	glDisable( GL_TEXTURE_2D );
	
	primCount=0;
}

void gxtkGraphics::Flush(){
	if( !primCount ) return;

	if( primSurf ){
		glEnable( GL_TEXTURE_2D );
		glBindTexture( GL_TEXTURE_2D,primSurf->texture );
	}
		
	switch( primType ){
	case 1:
		glDrawArrays( GL_POINTS,0,primCount );
		break;
	case 2:
		glDrawArrays( GL_LINES,0,primCount*2 );
		break;
	case 4:
		glDrawElements( GL_TRIANGLES,primCount*6,GL_UNSIGNED_SHORT,quadIndices );
		break;
	case 5:
		glDrawArrays( GL_TRIANGLE_FAN,0,primCount );
		break;
	}

	if( primSurf ){
		glDisable( GL_TEXTURE_2D );
	}

	primCount=0;
}

//***** GXTK API *****

int gxtkGraphics::Mode(){
	return mode;
}

int gxtkGraphics::Width(){
	return width;
}

int gxtkGraphics::Height(){
	return height;
}

int gxtkGraphics::Cls( float r,float g,float b ){
	primCount=0;

	glClearColor( r/255.0f,g/255.0f,b/255.0f,1 );
	glClear( GL_COLOR_BUFFER_BIT );

	return 0;
}

int gxtkGraphics::SetAlpha( float alpha ){
	this->alpha=alpha;
	
	int a=int(alpha*255);
	
	colorARGB=(a<<24) | (int(b*alpha)<<16) | (int(g*alpha)<<8) | int(r*alpha);
	
	return 0;
}

int gxtkGraphics::SetColor( float r,float g,float b ){
	this->r=r;
	this->g=g;
	this->b=b;

	int a=int(alpha*255);
	
	colorARGB=(a<<24) | (int(b*alpha)<<16) | (int(g*alpha)<<8) | int(r*alpha);
	
	return 0;
}

int gxtkGraphics::SetBlend( int blend ){
	Flush();
	
	switch( blend ){
	case 1:
		glBlendFunc( GL_ONE,GL_ONE );
		break;
	default:
		glBlendFunc( GL_ONE,GL_ONE_MINUS_SRC_ALPHA );
	}

	return 0;
}

int gxtkGraphics::SetScissor( int x,int y,int w,int h ){
	Flush();
	
	if( x!=0 || y!=0 || w!=Width() || h!=Height() ){
		glEnable( GL_SCISSOR_TEST );
		y=Height()-y-h;
		glScissor( x,y,w,h );
	}else{
		glDisable( GL_SCISSOR_TEST );
	}
	return 0;
}

int gxtkGraphics::SetMatrix( float ix,float iy,float jx,float jy,float tx,float ty ){

	tformed=(ix!=1 || iy!=0 || jx!=0 || jy!=1 || tx!=0 || ty!=0);

	this->ix=ix;this->iy=iy;this->jx=jx;this->jy=jy;this->tx=tx;this->ty=ty;

	return 0;
}

int gxtkGraphics::DrawLine( float x0,float y0,float x1,float y1 ){
	if( primType!=2 || primCount==MAX_LINES || primSurf ){
		Flush();
		primType=2;
		primSurf=0;
	}

	if( tformed ){
		float tx0=x0,tx1=x1;
		x0=tx0 * ix + y0 * jx + tx;y0=tx0 * iy + y0 * jy + ty;
		x1=tx1 * ix + y1 * jx + tx;y1=tx1 * iy + y1 * jy + ty;
	}
	
	float *vp=&vertices[primCount++*10];
	
	vp[0]=x0;vp[1]=y0;(int&)vp[4]=colorARGB;
	vp[5]=x1;vp[6]=y1;(int&)vp[9]=colorARGB;
	
	return 0;
}

int gxtkGraphics::DrawPoint( float x,float y ){
	if( primType!=1 || primCount==MAX_POINTS || primSurf ){
		Flush();
		primType=1;
		primSurf=0;
	}
	
	if( tformed ){
		float px=x;
		x=px * ix + y * jx + tx;
		y=px * iy + y * jy + ty;
	}
	
	float *vp=&vertices[primCount++*5];
	
	vp[0]=x;vp[1]=y;(int&)vp[4]=colorARGB;

	return 0;	
}
	
int gxtkGraphics::DrawRect( float x,float y,float w,float h ){
	if( primType!=4 || primCount==MAX_QUADS || primSurf ){
		Flush();
		primType=4;
		primSurf=0;
	}

	float x0=x,x1=x+w,x2=x+w,x3=x;
	float y0=y,y1=y,y2=y+h,y3=y+h;

	if( tformed ){
		float tx0=x0,tx1=x1,tx2=x2,tx3=x3;
		x0=tx0 * ix + y0 * jx + tx;y0=tx0 * iy + y0 * jy + ty;
		x1=tx1 * ix + y1 * jx + tx;y1=tx1 * iy + y1 * jy + ty;
		x2=tx2 * ix + y2 * jx + tx;y2=tx2 * iy + y2 * jy + ty;
		x3=tx3 * ix + y3 * jx + tx;y3=tx3 * iy + y3 * jy + ty;
	}
	
	float *vp=&vertices[primCount++*20];
	
	vp[0 ]=x0;vp[1 ]=y0;(int&)vp[4 ]=colorARGB;
	vp[5 ]=x1;vp[6 ]=y1;(int&)vp[9 ]=colorARGB;
	vp[10]=x2;vp[11]=y2;(int&)vp[14]=colorARGB;
	vp[15]=x3;vp[16]=y3;(int&)vp[19]=colorARGB;

	return 0;
}

int gxtkGraphics::DrawOval( float x,float y,float w,float h ){
	Flush();
	primType=5;
	primSurf=0;
	
	float xr=w/2.0f;
	float yr=h/2.0f;

	int segs;
	if( tformed ){
		float dx_x=xr * ix;
		float dx_y=xr * iy;
		float dx=sqrtf( dx_x*dx_x+dx_y*dx_y );
		float dy_x=yr * jx;
		float dy_y=yr * jy;
		float dy=sqrtf( dy_x*dy_x+dy_y*dy_y );
		segs=(int)( dx+dy );
	}else{
		segs=(int)( abs( xr )+abs( yr ) );
	}
	
	if( segs<12 ){
		segs=12;
	}else if( segs>MAX_VERTS ){
		segs=MAX_VERTS;
	}else{
		segs&=~3;
	}

	float x0=x+xr,y0=y+yr;
	
	float *vp=vertices;

	for( int i=0;i<segs;++i ){
	
		float th=i * 6.28318531f / segs;

		float px=x0+cosf( th ) * xr;
		float py=y0-sinf( th ) * yr;
		
		if( tformed ){
			float ppx=px;
			px=ppx * ix + py * jx + tx;
			py=ppx * iy + py * jy + ty;
		}
		
		vp[0]=px;vp[1]=py;(int&)vp[4]=colorARGB;
		vp+=5;
	}
	
	primCount=segs;

	Flush();
	
	return 0;
}

int gxtkGraphics::DrawPoly( Array<float> verts ){
	int n=verts.Length()/2;
	if( n<3 || n>MAX_VERTS ) return 0;
	
	Flush();
	primType=5;
	primSurf=0;
	
	float *vp=vertices;
	
	for( int i=0;i<n;++i ){
	
		float px=verts[i*2];
		float py=verts[i*2+1];
		
		if( tformed ){
			float ppx=px;
			px=ppx * ix + py * jx + tx;
			py=ppx * iy + py * jy + ty;
		}
		
		vp[0]=px;vp[1]=py;(int&)vp[4]=colorARGB;
		vp+=5;
	}

	primCount=n;
	
	Flush();
	
	return 0;
}


int gxtkGraphics::DrawSurface( gxtkSurface *surf,float x,float y ){
	if( primType!=4 || primCount==MAX_QUADS || surf!=primSurf ){
		Flush();
		primType=4;
		primSurf=surf;
	}
	
	float w=surf->Width();
	float h=surf->Height();
	float x0=x,x1=x+w,x2=x+w,x3=x;
	float y0=y,y1=y,y2=y+h,y3=y+h;
	float u0=0,u1=w*surf->uscale;
	float v0=0,v1=h*surf->vscale;

	if( tformed ){
		float tx0=x0,tx1=x1,tx2=x2,tx3=x3;
		x0=tx0 * ix + y0 * jx + tx;y0=tx0 * iy + y0 * jy + ty;
		x1=tx1 * ix + y1 * jx + tx;y1=tx1 * iy + y1 * jy + ty;
		x2=tx2 * ix + y2 * jx + tx;y2=tx2 * iy + y2 * jy + ty;
		x3=tx3 * ix + y3 * jx + tx;y3=tx3 * iy + y3 * jy + ty;
	}
	
	float *vp=&vertices[primCount++*20];
	
	vp[0 ]=x0;vp[1 ]=y0;vp[2 ]=u0;vp[3 ]=v0;(int&)vp[4 ]=colorARGB;
	vp[5 ]=x1;vp[6 ]=y1;vp[7 ]=u1;vp[8 ]=v0;(int&)vp[9 ]=colorARGB;
	vp[10]=x2;vp[11]=y2;vp[12]=u1;vp[13]=v1;(int&)vp[14]=colorARGB;
	vp[15]=x3;vp[16]=y3;vp[17]=u0;vp[18]=v1;(int&)vp[19]=colorARGB;
	
	return 0;
}

int gxtkGraphics::DrawSurface2( gxtkSurface *surf,float x,float y,int srcx,int srcy,int srcw,int srch ){
	if( primType!=4 || primCount==MAX_QUADS || surf!=primSurf ){
		Flush();
		primType=4;
		primSurf=surf;
	}
	
	float w=srcw;
	float h=srch;
	float x0=x,x1=x+w,x2=x+w,x3=x;
	float y0=y,y1=y,y2=y+h,y3=y+h;
	float u0=srcx*surf->uscale,u1=(srcx+srcw)*surf->uscale;
	float v0=srcy*surf->vscale,v1=(srcy+srch)*surf->vscale;

	if( tformed ){
		float tx0=x0,tx1=x1,tx2=x2,tx3=x3;
		x0=tx0 * ix + y0 * jx + tx;y0=tx0 * iy + y0 * jy + ty;
		x1=tx1 * ix + y1 * jx + tx;y1=tx1 * iy + y1 * jy + ty;
		x2=tx2 * ix + y2 * jx + tx;y2=tx2 * iy + y2 * jy + ty;
		x3=tx3 * ix + y3 * jx + tx;y3=tx3 * iy + y3 * jy + ty;
	}
	
	float *vp=&vertices[primCount++*20];
	
	vp[0 ]=x0;vp[1 ]=y0;vp[2 ]=u0;vp[3 ]=v0;(int&)vp[4 ]=colorARGB;
	vp[5 ]=x1;vp[6 ]=y1;vp[7 ]=u1;vp[8 ]=v0;(int&)vp[9 ]=colorARGB;
	vp[10]=x2;vp[11]=y2;vp[12]=u1;vp[13]=v1;(int&)vp[14]=colorARGB;
	vp[15]=x3;vp[16]=y3;vp[17]=u0;vp[18]=v1;(int&)vp[19]=colorARGB;
	
	return 0;
}
	
int gxtkGraphics::ReadPixels( Array<int> pixels,int x,int y,int width,int height,int offset,int pitch ){

	Flush();

	unsigned *p=(unsigned*)malloc(width*height*4);

	glReadPixels( x,this->height-y-height,width,height,GL_BGRA,GL_UNSIGNED_BYTE,p );
	
	for( int py=0;py<height;++py ){
		memcpy( &pixels[offset+py*pitch],&p[(height-py-1)*width],width*4 );
	}
	
	free( p );
	
	return 0;
}

int gxtkGraphics::WritePixels2( gxtkSurface *surface,Array<int> pixels,int x,int y,int width,int height,int offset,int pitch ){

	Flush();
	
	unsigned *p=(unsigned*)malloc(width*height*4);

	unsigned *d=p;
	for( int py=0;py<height;++py ){
		unsigned *s=(unsigned*)&pixels[offset+py*pitch];
		for( int px=0;px<width;++px ){
			unsigned p=*s++;
			unsigned a=p>>24;
			*d++=(a<<24) | ((p>>16&0xff)*a/255<<16) | ((p>>8&0xff)*a/255<<8) | ((p&0xff)*a/255);
		}
	}

	glBindTexture( GL_TEXTURE_2D,surface->texture );

	glTexSubImage2D( GL_TEXTURE_2D,0,x,y,width,height,GL_BGRA,GL_UNSIGNED_BYTE,p );
	
	free( p );
	
	return 0;
}

//***** gxtkSurface *****

gxtkSurface::gxtkSurface():
data(0),width(0),height(0),depth(0),texture(0),uscale(0),vscale(0){
}

gxtkSurface::gxtkSurface( unsigned char *data,int width,int height,int depth ):
data(data),width(width),height(height),depth(depth),texture(0),uscale(0),vscale(0){
}

void gxtkSurface::SetData( unsigned char *data,int width,int height,int depth ){
	this->data=data;
	this->width=width;
	this->height=height;
	this->depth=depth;
}

gxtkSurface::~gxtkSurface(){
	Discard();
}

int gxtkSurface::Discard(){
	if( texture ){
		glDeleteTextures( 1,&texture );
		texture=0;
	}
	if( data ){
		unloadImage( data );
		data=0;
	}
	return 0;
}

int gxtkSurface::Width(){
	return width;
}

int gxtkSurface::Height(){
	return height;
}

int gxtkSurface::Loaded(){
	return 1;
}

bool gxtkSurface::OnUnsafeLoadComplete(){

	unsigned char *p=data;
	int n=width*height,fmt=0;
	
	switch( depth ){
	case 1:
		fmt=GL_LUMINANCE;
		break;
	case 2:
		if( data ){
			while( n-- ){	//premultiply alpha
				p[0]=p[0]*p[1]/255;
				p+=2;
			}
		}
		fmt=GL_LUMINANCE_ALPHA;
		break;
	case 3:
		fmt=GL_RGB;
		break;
	case 4:
		if( data ){
			while( n-- ){	//premultiply alpha
				p[0]=p[0]*p[3]/255;
				p[1]=p[1]*p[3]/255;
				p[2]=p[2]*p[3]/255;
				p+=4;
			}
		}
		fmt=GL_RGBA;
		break;
	default:
		exit( -1 );
	}
	
	glGenTextures( 1,&texture );
	glBindTexture( GL_TEXTURE_2D,texture );

	if( CFG_MOJO_IMAGE_FILTERING_ENABLED ){
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR );
	}else{
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST );
	}

	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE );

	bool ok=true;
	
	int texwidth=width;
	int texheight=height;
	
	glTexImage2D( GL_TEXTURE_2D,0,fmt,texwidth,texheight,0,fmt,GL_UNSIGNED_BYTE,0 );
	if( glGetError()!=GL_NO_ERROR ){
	
		texwidth=Pow2Size( width );
		texheight=Pow2Size( height );
		
		glTexImage2D( GL_TEXTURE_2D,0,fmt,texwidth,texheight,0,fmt,GL_UNSIGNED_BYTE,0 );
		if( glGetError()!=GL_NO_ERROR ) ok=false;
	}
	
	if( ok ){
		if( data ){
			glPixelStorei( GL_UNPACK_ALIGNMENT,1 );
			glTexSubImage2D( GL_TEXTURE_2D,0,0,0,width,height,fmt,GL_UNSIGNED_BYTE,data );
		}
		uscale=1.0/texwidth;
		vscale=1.0/texheight;
	}
	
	if( data ){
		unloadImage( data );
		data=0;
	}
	
	return ok;
}

//***** END OF COMMON OPENGL CODE *****

bool gxtkGraphics::Validate(){
	width=height=0;
	glfwGetWindowSize( &width,&height );
	return width>0 && height>0;
}

void gxtkGraphics::EndRender(){
	if( mode ) Flush();
	glfwSwapBuffers();
}

gxtkSurface *gxtkGraphics::LoadSurface__UNSAFE__( gxtkSurface *surface,String path ){
	int width,height,depth;
	unsigned char *data=loadImage( path,&width,&height,&depth );
	if( !data ) return 0;
	surface->SetData( data,width,height,depth );
	return surface;
}

gxtkSurface *gxtkGraphics::LoadSurface( String path ){

	gxtkSurface *surf=LoadSurface__UNSAFE__( new gxtkSurface(),path );
	if( surf && !surf->OnUnsafeLoadComplete() ) surf=0;
	return surf;
}

gxtkSurface *gxtkGraphics::CreateSurface( int width,int height ){

	gxtkSurface *surf=new gxtkSurface( 0,width,height,4 );
	if( surf && !surf->OnUnsafeLoadComplete() ) surf=0;
	return surf;
}

//gxtkSurface *gxtkGraphics::CreateSurface2( BBDataBuffer *buf ){
//	int width,height,depth;
//	unsigned char *data=loadImage( (unsigned char*)buf->ReadPointer(0),buf->Length(),&width,&height,&depth );
//	if( data ) return new gxtkSurface( data,width,height,depth );
//	return 0;
//}

// ***** End of graphics ******

class gxtkInput : public gxtkObject{
public:
	int keyStates[512];
	int charQueue[32];
	int charPut,charGet;
	float mouseX,mouseY;
	
	float joyPos[6];
	int joyButton[32];
	
	gxtkInput();
	~gxtkInput(){
//		Print( "~gxtkInput" );
	}
	
	void BeginUpdate();
	void EndUpdate();
	
	void OnKeyDown( int key );
	void OnKeyUp( int key );
	void PutChar( int chr );
	
	//***** GXTK API *****
	virtual int SetKeyboardEnabled( int enabled );
	
	virtual int KeyDown( int key );
	virtual int KeyHit( int key );
	virtual int GetChar();
	
	virtual float MouseX();
	virtual float MouseY();

	virtual float JoyX( int index );
	virtual float JoyY( int index );
	virtual float JoyZ( int index );

	virtual float TouchX( int index );
	virtual float TouchY( int index );
	
	virtual float AccelX();
	virtual float AccelY();
	virtual float AccelZ();
};

class gxtkChannel{
public:
	ALuint source;
	gxtkSample *sample;
	int flags;
	int state;
	
	int AL_Source();
};

class gxtkAudio : public gxtkObject{
public:
	gxtkChannel channels[33];

	gxtkAudio();

	void mark();
	void OnSuspend();
	void OnResume();

	//***** GXTK API *****
	virtual gxtkSample *LoadSample__UNSAFE__( gxtkSample *sample,String path );
	virtual gxtkSample *LoadSample( String path );
	virtual int PlaySample( gxtkSample *sample,int channel,int flags );

	virtual int StopChannel( int channel );
	virtual int PauseChannel( int channel );
	virtual int ResumeChannel( int channel );
	virtual int ChannelState( int channel );
	virtual int SetVolume( int channel,float volume );
	virtual int SetPan( int channel,float pan );
	virtual int SetRate( int channel,float rate );
	
	virtual int PlayMusic( String path,int flags );
	virtual int StopMusic();
	virtual int PauseMusic();
	virtual int ResumeMusic();
	virtual int MusicState();
	virtual int SetMusicVolume( float volume );
};

class gxtkSample : public gxtkObject{
public:
	ALuint al_buffer;

	gxtkSample();
	gxtkSample( ALuint buf );
	~gxtkSample();
	
	void SetBuffer( ALuint buf );
	
	//***** GXTK API *****
	virtual int Discard();
};

//***** gxtkApp *****

int RunApp(){
	app->Run();
	return 0;
}

gxtkApp::gxtkApp(){
	app=this;

	graphics=new gxtkGraphics;
	input=new gxtkInput;
	audio=new gxtkAudio;

	updateRate=0;
	suspended=false;

	runner=RunApp;
}

void gxtkApp::Run(){

	glfwEnable( GLFW_KEY_REPEAT );
	glfwDisable( GLFW_AUTO_POLL_EVENTS );

	glfwSetKeyCallback( OnKey );
	glfwSetCharCallback( OnChar );
	glfwSetWindowSizeCallback( OnWindowSize );
	glfwSetWindowRefreshCallback( OnWindowRefresh );
	glfwSetMouseButtonCallback( OnMouseButton );

	InvokeOnCreate();
	InvokeOnRender();

	while( glfwGetWindowParam( GLFW_OPENED ) ){
	
		if( glfwGetWindowParam( GLFW_ICONIFIED ) ){
			if( !suspended ){
				InvokeOnSuspend();
				continue;
			}
		}else if( glfwGetWindowParam( GLFW_ACTIVE ) ){
			if( suspended ){
				InvokeOnResume();
				continue;
			}
		}else if( CFG_MOJO_AUTO_SUSPEND_ENABLED ){
			if( !suspended ){
				InvokeOnSuspend();
				continue;
			}
		}
	
		if( !updateRate || suspended ){
			InvokeOnRender();
			glfwWaitEvents();
			continue;
		}
		
		float time=glfwGetTime();
		if( time<nextUpdate ){
			glfwSleep( nextUpdate-time );
			continue;
		}

		glfwPollEvents();
				
		int updates=0;
		for(;;){
			nextUpdate+=updatePeriod;
			
			InvokeOnUpdate();
			if( !updateRate ) break;
			
			if( nextUpdate>glfwGetTime() ){
				break;
			}
			
			if( ++updates==7 ){
				nextUpdate=glfwGetTime();
				break;
			}
		}
		InvokeOnRender();
	}
}

void gxtkApp::OnWindowSize( int width,int height ){
//	Print( "OnWindowSize!" );
//	if( width>0 && height>0 ){
//		app->InvokeOnResume();
//	}else{
//		app->InvokeOnSuspend();
//	}
}

void gxtkApp::OnWindowRefresh(){
//	Print( "OnWindowRefresh!" );
//	app->InvokeOnRender();
}

void gxtkApp::OnMouseButton( int button,int action ){
	int key;
	switch( button ){
	case GLFW_MOUSE_BUTTON_LEFT:key=KEY_LMB;break;
	case GLFW_MOUSE_BUTTON_RIGHT:key=KEY_RMB;break;
	case GLFW_MOUSE_BUTTON_MIDDLE:key=KEY_MMB;break;
	default:return;
	}
	switch( action ){
	case GLFW_PRESS:
		app->input->OnKeyDown( key );
		break;
	case GLFW_RELEASE:
		app->input->OnKeyUp( key );
		break;
	}
}

void gxtkApp::OnKey( int key,int action ){

	key=TransKey( key );
	if( !key ) return;
	
	switch( action ){
	case GLFW_PRESS:
		app->input->OnKeyDown( key );
		
		if( int chr=KeyToChar( key ) ){
			app->input->PutChar( chr );
		}
		
		break;
	case GLFW_RELEASE:
		app->input->OnKeyUp( key );
		break;
	}
}

void gxtkApp::OnChar( int chr,int action ){

	switch( action ){
	case GLFW_PRESS:
		app->input->PutChar( chr );
		break;
	}
}

void gxtkApp::InvokeOnCreate(){
	if( !graphics->Validate() ) abort();
	
	OnCreate();
	
	gc_collect();
}

void gxtkApp::InvokeOnSuspend(){
	if( suspended ) return;
	
	suspended=true;
	OnSuspend();
	audio->OnSuspend();
	if( updateRate ){
		int upr=updateRate;
		SetUpdateRate( 0 );
		updateRate=upr;
	}
	
	gc_collect();
}

void gxtkApp::InvokeOnResume(){
	if( !suspended ) return;
	
	if( updateRate ){
		int upr=updateRate;
		updateRate=0;
		SetUpdateRate( upr );
	}
	audio->OnResume();
	OnResume();
	suspended=false;
	
	gc_collect();
}

void gxtkApp::InvokeOnUpdate(){
	if( suspended || !updateRate || !graphics->Validate() ) return;
	
	input->BeginUpdate();
	OnUpdate();
	input->EndUpdate();
	
	gc_collect();
}

void gxtkApp::InvokeOnRender(){
	if( suspended || !graphics->Validate() ) return;
	
	graphics->BeginRender();
	OnRender();
	graphics->EndRender();
	
	gc_collect();
}

//***** GXTK API *****

gxtkGraphics *gxtkApp::GraphicsDevice(){
	return graphics;
}

gxtkInput *gxtkApp::InputDevice(){
	return input;
}

gxtkAudio *gxtkApp::AudioDevice(){
	return audio;
}

String gxtkApp::AppTitle(){
	return "<TODO>";
}

String gxtkApp::LoadState(){
	if( FILE *fp=fopen( ".monkeystate","rb" ) ){
		String str=String::Load( fp );
		fclose( fp );
		return str;
	}
	return "";
}

int gxtkApp::SaveState( String state ){
	if( FILE *fp=fopen( ".monkeystate","wb" ) ){
		bool ok=state.Save( fp );
		fclose( fp );
		return ok ? 0 : -2;
	}
	return -1;
}

String gxtkApp::LoadString( String path ){
	if( FILE *fp=fopenFile( path,"rb" ) ){
		String str=String::Load( fp );
		fclose( fp );
		return str;
	}
	return "";
}

int gxtkApp::SetUpdateRate( int hertz ){
	updateRate=hertz;

	if( updateRate ){
		updatePeriod=1.0/updateRate;
		nextUpdate=glfwGetTime()+updatePeriod;
	}
	return 0;
}

int gxtkApp::MilliSecs(){
	return glfwGetTime()*1000.0;
}

int gxtkApp::Loading(){
	return 0;
}

int gxtkApp::OnCreate(){
	return 0;
}

int gxtkApp::OnSuspend(){
	return 0;
}

int gxtkApp::OnResume(){
	return 0;
}

int gxtkApp::OnUpdate(){
	return 0;
}

int gxtkApp::OnRender(){
	return 0;
}

int gxtkApp::OnLoading(){
	return 0;
}

// ***** gxtkInput *****

gxtkInput::gxtkInput(){
	memset( keyStates,0,sizeof(keyStates) );
	memset( charQueue,0,sizeof(charQueue) );
	mouseX=mouseY=0;
	charPut=charGet=0;
	
}

void gxtkInput::BeginUpdate(){

	int x=0,y=0;
	glfwGetMousePos( &x,&y );
	mouseX=x;
	mouseY=y;
	
	int n_axes=glfwGetJoystickParam( GLFW_JOYSTICK_1,GLFW_AXES );
	int n_buttons=glfwGetJoystickParam( GLFW_JOYSTICK_1,GLFW_BUTTONS );

//	printf( "n_axes=%i, n_buttons=%i\n",n_axes,n_buttons );fflush( stdout );
	
	memset( joyPos,0,sizeof(joyPos) );	
	glfwGetJoystickPos( GLFW_JOYSTICK_1,joyPos,n_axes );
	
	unsigned char buttons[32];
	memset( buttons,0,sizeof(buttons) );
	glfwGetJoystickButtons( GLFW_JOYSTICK_1,buttons,n_buttons );

	float t;
	switch( n_axes ){
	case 4:	//my saitek...axes=4, buttons=14
		joyPos[4]=joyPos[2];
		joyPos[3]=joyPos[3];
		joyPos[2]=0;
		break;
	case 5:	//xbox360...axes=5, buttons=10
		t=joyPos[3];
		joyPos[3]=joyPos[4];
		joyPos[4]=t;
		break;
	}
	
	for( int i=0;i<n_buttons;++i ){
		if( buttons[i]==GLFW_PRESS ){
			OnKeyDown( 256+i );
		}else{
			OnKeyUp( 256+i );
		}
	}
}

void gxtkInput::EndUpdate(){
	for( int i=0;i<512;++i ){
		keyStates[i]&=0x100;
	}
	charGet=0;
	charPut=0;
}

void gxtkInput::OnKeyDown( int key ){
	if( keyStates[key] & 0x100 ) return;
	
	keyStates[key]|=0x100;
	++keyStates[key];
	
	switch( key ){
	case VKEY_LSHIFT:case VKEY_RSHIFT:
		if( (keyStates[VKEY_LSHIFT]&0x100) || (keyStates[VKEY_RSHIFT]&0x100) ) OnKeyDown( VKEY_SHIFT );
		break;
	case VKEY_LCONTROL:case VKEY_RCONTROL:
		if( (keyStates[VKEY_LCONTROL]&0x100) || (keyStates[VKEY_RCONTROL]&0x100) ) OnKeyDown( VKEY_CONTROL );
		break;
	}
}

void gxtkInput::OnKeyUp( int key ){
	if( !(keyStates[key] & 0x100) ) return;

	keyStates[key]&=0xff;
	
	switch( key ){
	case VKEY_LSHIFT:case VKEY_RSHIFT:
		if( !(keyStates[VKEY_LSHIFT]&0x100) && !(keyStates[VKEY_RSHIFT]&0x100) ) OnKeyUp( VKEY_SHIFT );
		break;
	case VKEY_LCONTROL:case VKEY_RCONTROL:
		if( !(keyStates[VKEY_LCONTROL]&0x100) && !(keyStates[VKEY_RCONTROL]&0x100) ) OnKeyUp( VKEY_CONTROL );
		break;
	}
}

void gxtkInput::PutChar( int chr ){
	if( charPut<32 ) charQueue[charPut++]=chr;
}

//***** GXTK API *****

int gxtkInput::SetKeyboardEnabled( int enabled ){
	return 0;
}

int gxtkInput::KeyDown( int key ){
	if( key>0 && key<512 ){
		if( key==KEY_TOUCH0 ) key=KEY_LMB;
		return keyStates[key] >> 8;
	}
	return 0;
}

int gxtkInput::KeyHit( int key ){
	if( key>0 && key<512 ){
		if( key==KEY_TOUCH0 ) key=KEY_LMB;
		return keyStates[key] & 0xff;
	}
	return 0;
}

int gxtkInput::GetChar(){
	if( charGet<charPut ){
		return charQueue[charGet++];
	}
	return 0;
}
	
float gxtkInput::MouseX(){
	return mouseX;
}

float gxtkInput::MouseY(){
	return mouseY;
}

float gxtkInput::JoyX( int index ){
	switch( index ){
	case 0:return joyPos[0];
	case 1:return joyPos[3];
	}
	return 0;
}

float gxtkInput::JoyY( int index ){
	switch( index ){
	case 0:return joyPos[1];
	case 1:return -joyPos[4];
	}
	return 0;
}

float gxtkInput::JoyZ( int index ){
	switch( index ){
	case 0:return joyPos[2];
	case 1:return joyPos[5];
	}
	return 0;
}

float gxtkInput::TouchX( int index ){
	return mouseX;
}

float gxtkInput::TouchY( int index ){
	return mouseY;
}

float gxtkInput::AccelX(){
	return 0;
}

float gxtkInput::AccelY(){
	return 0;
}

float gxtkInput::AccelZ(){
	return 0;
}

//***** gxtkAudio *****
static std::vector<ALuint> discarded;

static void FlushDiscarded( gxtkAudio *audio ){

	if( !discarded.size() ) return;
	
	for( int i=0;i<33;++i ){
		gxtkChannel *chan=&audio->channels[i];
		if( chan->state ){
			int state=0;
			alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
			if( state==AL_STOPPED ) alSourcei( chan->source,AL_BUFFER,0 );
		}
	}
	
	std::vector<ALuint> out;
	
	for( int i=0;i<discarded.size();++i ){
		ALuint buf=discarded[i];
		alDeleteBuffers( 1,&buf );
		ALenum err=alGetError();
		if( err==AL_NO_ERROR ){
//			printf( "alDeleteBuffers OK!\n" );fflush( stdout );
		}else{
//			printf( "alDeleteBuffers failed...\n" );fflush( stdout );
			out.push_back( buf );
		}
	}
	discarded=out;
}

static void CheckAL(){
	ALenum err=alGetError();
	if( err!=AL_NO_ERROR ){
		printf( "AL Error:%i\n",err );
		fflush( stdout );
	}
}

int gxtkChannel::AL_Source(){
	if( !source ) alGenSources( 1,&source );
	return source;
}

gxtkAudio::gxtkAudio(){
	alDistanceModel( AL_NONE );
	memset( channels,0,sizeof(channels) );
}

void gxtkAudio::mark(){
	for( int i=0;i<33;++i ){
		gxtkChannel *chan=&channels[i];
		if( chan->state!=0 ){
			int state=0;
			alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
			if( state!=AL_STOPPED ) gc_mark( chan->sample );
		}
	}
}

void gxtkAudio::OnSuspend(){
	for( int i=0;i<33;++i ){
		gxtkChannel *chan=&channels[i];
		if( chan->state==1 ){
			int state=0;
			alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
			if( state==AL_PLAYING ) alSourcePause( chan->source );
		}
	}
}

void gxtkAudio::OnResume(){
	for( int i=0;i<33;++i ){
		gxtkChannel *chan=&channels[i];
		if( chan->state==1 ){
			int state=0;
			alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
			if( state==AL_PAUSED ) alSourcePlay( chan->source );
		}
	}
}

gxtkSample *gxtkAudio::LoadSample__UNSAFE__( gxtkSample *sample,String path ){

	int length=0;
	int channels=0;
	int format=0;
	int hertz=0;
	unsigned char *data=loadSound( path,&length,&channels,&format,&hertz );
	if( !data ) return 0;
	
	int al_format=0;
	if( format==1 && channels==1 ){
		al_format=AL_FORMAT_MONO8;
	}else if( format==1 && channels==2 ){
		al_format=AL_FORMAT_STEREO8;
	}else if( format==2 && channels==1 ){
		al_format=AL_FORMAT_MONO16;
	}else if( format==2 && channels==2 ){
		al_format=AL_FORMAT_STEREO16;
	}
	
	int size=length*channels*format;
	
	ALuint al_buffer;
	alGenBuffers( 1,&al_buffer );
	alBufferData( al_buffer,al_format,data,size,hertz );
	unloadSound( data );
	
	sample->SetBuffer( al_buffer );
	return sample;
}

gxtkSample *gxtkAudio::LoadSample( String path ){

	FlushDiscarded( this );

	return LoadSample__UNSAFE__( new gxtkSample(),path );
}

int gxtkAudio::PlaySample( gxtkSample *sample,int channel,int flags ){

	FlushDiscarded( this );
	
	gxtkChannel *chan=&channels[channel];
	
	chan->AL_Source();
	
	alSourceStop( chan->source );
	alSourcei( chan->source,AL_BUFFER,sample->al_buffer );
	alSourcei( chan->source,AL_LOOPING,flags ? 1 : 0 );
	alSourcePlay( chan->source );
	
	gc_assign( chan->sample,sample );

	chan->flags=flags;
	chan->state=1;

	return 0;
}

int gxtkAudio::StopChannel( int channel ){
	gxtkChannel *chan=&channels[channel];

	if( chan->state!=0 ){
		alSourceStop( chan->source );
		chan->state=0;
	}
	return 0;
}

int gxtkAudio::PauseChannel( int channel ){
	gxtkChannel *chan=&channels[channel];

	if( chan->state==1 ){
		int state=0;
		alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
		if( state==AL_STOPPED ){
			chan->state=0;
		}else{
			alSourcePause( chan->source );
			chan->state=2;
		}
	}
	return 0;
}

int gxtkAudio::ResumeChannel( int channel ){
	gxtkChannel *chan=&channels[channel];

	if( chan->state==2 ){
		alSourcePlay( chan->source );
		chan->state=1;
	}
	return 0;
}

int gxtkAudio::ChannelState( int channel ){
	gxtkChannel *chan=&channels[channel];
	
	if( chan->state==1 ){
		int state=0;
		alGetSourcei( chan->source,AL_SOURCE_STATE,&state );
		if( state==AL_STOPPED ) chan->state=0;
	}
	return chan->state;
}

int gxtkAudio::SetVolume( int channel,float volume ){
	gxtkChannel *chan=&channels[channel];

	alSourcef( chan->AL_Source(),AL_GAIN,volume );
	return 0;
}

int gxtkAudio::SetPan( int channel,float pan ){
	gxtkChannel *chan=&channels[channel];

	alSource3f( chan->AL_Source(),AL_POSITION,pan,0,0 );
	return 0;
}

int gxtkAudio::SetRate( int channel,float rate ){
	gxtkChannel *chan=&channels[channel];

	alSourcef( chan->AL_Source(),AL_PITCH,rate );
	return 0;
}

int gxtkAudio::PlayMusic( String path,int flags ){
	StopMusic();
	
	gxtkSample *music=LoadSample( path );
	if( !music ) return -1;
	
	PlaySample( music,32,flags );
	return 0;
}

int gxtkAudio::StopMusic(){
	StopChannel( 32 );
	return 0;
}

int gxtkAudio::PauseMusic(){
	PauseChannel( 32 );
	return 0;
}

int gxtkAudio::ResumeMusic(){
	ResumeChannel( 32 );
	return 0;
}

int gxtkAudio::MusicState(){
	return ChannelState( 32 );
}

int gxtkAudio::SetMusicVolume( float volume ){
	SetVolume( 32,volume );
	return 0;
}

//***** gxtkSample *****

gxtkSample::gxtkSample():
al_buffer(0){
}

gxtkSample::gxtkSample( ALuint buf ):
al_buffer(buf){
}

gxtkSample::~gxtkSample(){
	Discard();
}

void gxtkSample::SetBuffer( ALuint buf ){
	al_buffer=buf;
}

int gxtkSample::Discard(){
	if( al_buffer ){
		discarded.push_back( al_buffer );
		al_buffer=0;
	}
	return 0;
}

// ***** thread.h *****

class BBThread : public Object{
public:
	BBThread();
	~BBThread();
	
	virtual void Start();
	virtual bool IsRunning();
	virtual void Wait();
	
	virtual void Run__UNSAFE__();
	
private:

	enum{
		INIT=0,
		RUNNING=1,
		FINISHED=2
	};

	int _state;
	
#if __cplusplus_winrt

#elif _WIN32

	DWORD _id;
	HANDLE _handle;
	
	static DWORD WINAPI run( void *p );
	
#else

	pthread_t _handle;
	
	static void *run( void *p );
	
#endif

};

// ***** thread.cpp *****

BBThread::BBThread():_state( INIT ){
}

BBThread::~BBThread(){
	Wait();
}

bool BBThread::IsRunning(){
	return _state==RUNNING;
}

void BBThread::Run__UNSAFE__(){
}

#if __cplusplus_winrt

#elif _WIN32

void BBThread::Start(){
	if( _state==RUNNING ) return;
	
	if( _state==FINISHED ) CloseHandle( _handle );

	_state=RUNNING;

	_handle=CreateThread( 0,0,run,this,0,&_id );
	
//	_handle=CreateThread( 0,0,run,this,CREATE_SUSPENDED,&_id );
//	SetThreadPriority( _handle,THREAD_PRIORITY_ABOVE_NORMAL );
//	ResumeThread( _handle );
}

void BBThread::Wait(){
	if( _state==INIT ) return;

	WaitForSingleObject( _handle,INFINITE );
	CloseHandle( _handle );

	_state=INIT;
}

DWORD WINAPI BBThread::run( void *p ){
	BBThread *thread=(BBThread*)p;

	thread->Run__UNSAFE__();
	
	thread->_state=FINISHED;
	return 0;
}

#else

void BBThread::Start(){
	if( _state==RUNNING ) return;
	
	if( _state==FINISHED ) pthread_join( _handle,0 );
	
	_state=RUNNING;
	
	pthread_create( &_handle,0,run,this );
}

void BBThread::Wait(){
	if( _state==INIT ) return;
	
	pthread_join( _handle,0 );
	
	_state=INIT;
}

void *BBThread::run( void *p ){
	BBThread *thread=(BBThread*)p;

	thread->Run__UNSAFE__();

	thread->_state=FINISHED;
	return 0;
}

#endif

// ***** stream.h *****

class BBStream : public Object{
public:

	virtual int Eof(){
		return 0;
	}

	virtual void Close(){
	}

	virtual int Length(){
		return 0;
	}
	
	virtual int Position(){
		return 0;
	}
	
	virtual int Seek( int position ){
		return 0;
	}
	
	virtual int Read( BBDataBuffer *buffer,int offset,int count ){
		return 0;
	}

	virtual int Write( BBDataBuffer *buffer,int offset,int count ){
		return 0;
	}
};

// ***** stream.cpp *****

// ***** tcpsocket.h *****

#if _WIN32

#include <winsock.h>

#else

#include <netdb.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#define closesocket close
#define ioctlsocket ioctl

#endif

class BBTcpStream : public BBStream{
public:

	BBTcpStream();
	~BBTcpStream();
	
	bool Connect( String addr,int port );
	int ReadAvail();
	int WriteAvail();
	
	int Eof();
	void Close();
	int Read( BBDataBuffer *buffer,int offset,int count );
	int Write( BBDataBuffer *buffer,int offset,int count );
	
private:
	int _sock;
	int _state;	//0=INIT, 1=CONNECTED, 2=CLOSED, -1=ERROR
};

// ***** socket.cpp *****

BBTcpStream::BBTcpStream():_sock(-1),_state(0){
#if _WIN32
	static bool started;
	if( !started ){
		WSADATA ws;
		WSAStartup( 0x101,&ws );
		started=true;
	}
#endif
}

BBTcpStream::~BBTcpStream(){
	if( _sock>=0 ) closesocket( _sock );
}

bool BBTcpStream::Connect( String addr,int port ){

	if( _state ) return false;

	if( addr.Length()>1023 ) return false;
	
	char buf[1024];
	for( int i=0;i<addr.Length();++i ) buf[i]=addr[i];
	buf[addr.Length()]=0;

	_sock=socket( AF_INET,SOCK_STREAM,IPPROTO_TCP );
	if( _sock>=0 ){
		if( hostent *host=gethostbyname( buf ) ){
			if( char *hostip=inet_ntoa(*(struct in_addr *)*host->h_addr_list) ){
				struct sockaddr_in sa;
				sa.sin_family=AF_INET;
				sa.sin_addr.s_addr=inet_addr( hostip );
				sa.sin_port=htons( port );
				if( connect( _sock,(const sockaddr*)&sa,sizeof(sa) )>=0 ){
/*				
					int rcvbuf=16384;
					if( setsockopt( _sock,SOL_SOCKET,SO_RCVBUF,(const char*)&rcvbuf,sizeof(rcvbuf) )<0 ){
						puts( "setsockopt failed!" );
					}
*/
				
					int nodelay=1;
					if( setsockopt( _sock,IPPROTO_TCP,TCP_NODELAY,(const char*)&nodelay,sizeof(nodelay) )<0 ){
						puts( "setsockopt failed!" );
					}

					_state=1;
					return true;
				}
			}
		}
		closesocket( _sock );
		_sock=-1;
	}
	return false;
}

int BBTcpStream::ReadAvail(){

	if( _state!=1 ) return 0;
	
#ifdef FIONREAD	
	u_long arg;
	if( ioctlsocket( _sock,FIONREAD,&arg )>=0 ) return arg;
	_state=-1;
#endif

	return 0;
}

int BBTcpStream::WriteAvail(){

	if( _state!=1 ) return 0;

#ifdef FIONWRITE
	u_long arg;
	if( ioctlsocket( _sock,FIONREAD,&arg )>=0 ) return arg;
	_state=-1;
#endif

	return 0;
}

int BBTcpStream::Eof(){
	if( _state>=0 ) return _state==2;
	return -1;
}

void BBTcpStream::Close(){
	if( _sock<0 ) return;
	if( _state==1 ) _state=2;
	closesocket( _sock );
	_sock=-1;
}

int BBTcpStream::Read( BBDataBuffer *buffer,int offset,int count ){

	if( _state!=1 ) return 0;

	int n=recv( _sock,(char*)buffer->WritePointer(offset),count,0 );
	if( n>0 || (n==0 && count==0) ) return n;
	_state=(n==0) ? 2 : -1;
	return 0;
	
}

int BBTcpStream::Write( BBDataBuffer *buffer,int offset,int count ){
	if( _state!=1 ) return 0;
	int n=send( _sock,(const char*)buffer->ReadPointer(offset),count,0 );
	if( n>=0 ) return n;
	_state=-1;
	return 0;
}

// ***** filestream.h *****

class BBFileStream : public BBStream{
public:

	BBFileStream();
	~BBFileStream();

	void Close();
	int Eof();
	int Length();
	int Position();
	int Seek( int position );
	int Read( BBDataBuffer *buffer,int offset,int count );
	int Write( BBDataBuffer *buffer,int offset,int count );

	bool Open( String path,String mode );
	
private:
	FILE *_file;
	int _position;
	int _length;
};

// ***** filestream.cpp *****

BBFileStream::BBFileStream():_file(0),_position(0),_length(0){
}

BBFileStream::~BBFileStream(){
	if( _file ) fclose( _file );
}

bool BBFileStream::Open( String path,String mode ){
	if( _file ) return false;
	
	if( mode=="r" ){
		mode="rb";
	}else if( mode=="w" ){
		mode="wb";
	}else if( mode=="u" ){
		mode="wb+";
	}else{
		return false;
	}
	
	_file=fopenFile( path,mode );
	if( !_file ) return false;
	
	fseek( _file,0,SEEK_END );
	_position=0;
	_length=ftell( _file );
	fseek( _file,0,SEEK_SET );
	
	return true;
}

void BBFileStream::Close(){
	if( !_file ) return;
	
	fclose( _file );
	_file=0;
	_position=0;
	_length=0;
}

int BBFileStream::Eof(){
	if( !_file ) return -1;
	
	return _position==_length;
}

int BBFileStream::Length(){
	return _length;
}

int BBFileStream::Position(){
	return _position;
}

int BBFileStream::Seek( int position ){
	if( !_file ) return 0;
	
	fseek( _file,0,SEEK_SET );
	_position=ftell( _file );
	return _position;
}

int BBFileStream::Read( BBDataBuffer *buffer,int offset,int count ){
	if( !_file ) return 0;
	
	int n=fread( buffer->WritePointer(offset),1,count,_file );
	_position+=n;
	return n;
}

int BBFileStream::Write( BBDataBuffer *buffer,int offset,int count ){
	if( !_file ) return 0;
	
	int n=fwrite( buffer->ReadPointer(offset),1,count,_file );
	_position+=n;
	if( _position>_length ) _length=_position;
	return n;
}
#include <time.h>
#include <Shellapi.h>

float diddy_mouseWheel = 0.0f;

float diddy_mouseZ() {
	float ret = glfwGetMouseWheel() - diddy_mouseWheel;
	diddy_mouseWheel = glfwGetMouseWheel();
	return ret;
}

class diddy
{
	public:

	static float mouseZ()
	{
		return diddy_mouseZ();
	}
	
	static void mouseZInit()
	{
		return;
	}
	
	// only accurate to 1 second 
	static int systemMillisecs() {
		time_t seconds;
		seconds = time (NULL);
		return seconds * 1000;
	}

	static void flushKeys() {
		for( int i=0;i<512;++i ){
			app->input->keyStates[i]&=0x100;
		}
	}
	
	static int getUpdateRate() {
		return app->updateRate;
	}
	
	static void showMouse()
	{
		//ShowCursor(true);
		glfwEnable( GLFW_MOUSE_CURSOR );
	}
	
	static void hideMouse()
	{
		//ShowCursor(false);
		glfwDisable( GLFW_MOUSE_CURSOR );
	}
	
	static void setGraphics(int w, int h)
	{
		glfwSetWindowSize(w, h);
		GLFWvidmode desktopMode;
		glfwGetDesktopMode( &desktopMode );
		glfwSetWindowPos( (desktopMode.Width-w)/2,(desktopMode.Height-h)/2 );
	}
	
	static void setMouse(int x, int y)
	{
		glfwSetMousePos(x, y);
	}
	
	static void showKeyboard()
	{
	}
	static void launchBrowser(String address, String windowName)
	{
		LPCSTR addressStr = address.ToCString<char>();
		ShellExecute(HWND_DESKTOP, "open", addressStr, NULL, NULL, SW_SHOWNORMAL);
	}
	static void launchEmail(String email, String subject, String text)
	{
		String tmp = "mailto:";
		tmp+=email;
		tmp+="&subject=";
		tmp+=subject;
		tmp+="&body=";
		tmp+=text;
		LPCSTR addressStr = tmp.ToCString<char>();
		ShellExecute(HWND_DESKTOP, "open", addressStr, NULL, NULL, SW_SHOWNORMAL);
	}

	static void startVibrate(int millisecs)
	{
	}
	static void stopVibrate()
	{
	}
	
	static int getDayOfMonth()
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		return st.wDay;
	}
	
	static int getDayOfWeek()
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		return st.wDayOfWeek;
	}
	
	static int getMonth()
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		return st.wMonth;
	}
	
	static int getYear()
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		return st.wYear;
	}
	
	static int getHours()
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		return st.wHour;
	}
	
	static int getMinutes()
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		return st.wMinute;
	}
	
	static int getSeconds()
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		return st.wSecond;
	}
	
	static int getMilliSeconds()
	{
		SYSTEMTIME st;
		GetSystemTime(&st);
		return st.wMilliseconds;
	}
	
	static void startGps()
	{
	}
	static String getLatitiude()
	{
		return "";
	}
	static String getLongitude()
	{
		return "";
	}
	static void showAlertDialog(String title, String message)
	{
	}
	static String getInputString()
	{
		return "";
	}
	static int getPixel(int x, int y)
	{
		unsigned char pix[4];
		glReadPixels(x, app->graphics->height-y ,1 ,1 ,GL_RGBA ,GL_UNSIGNED_BYTE ,pix);
		return (pix[3]<<24) | (pix[0]<<16) | (pix[1]<<8) |  pix[2];
	}
	
	static int seekMusic(int timeMillis)
	{
		gxtkChannel *chan = &(app->audio->channels[32]);
		if(chan && chan->state==1)
		{
			alSourcef(chan->source, AL_SEC_OFFSET, (float)(timeMillis / 1000.0));
		}
		// TODO: check it worked
		return 1;
	}
};
class bb_exception_DiddyException;
class bb_reflection_ClassInfo;
class bb_map_Map;
class bb_map_StringMap;
class bb_map_Node;
class bb_reflection__GetClass;
class bb_exception_AssertException;
class bb_exception_ConcurrentModificationException;
class bb_exception_IndexOutOfBoundsException;
class bb_exception_IllegalArgumentException;
class bb_exception_XMLParseException;
class bb_boxes_BoolObject;
class bb_boxes_IntObject;
class bb_boxes_FloatObject;
class bb_boxes_StringObject;
class bb_reflection_R16;
class bb_reflection_R17;
class bb_reflection_R18;
class bb_reflection_R31;
class bb_reflection_R33;
class bb_reflection_R35;
class bb_reflection_R37;
class bb_reflection_R39;
class bb_reflection_R41;
class bb_reflection_R47;
class bb_reflection_R57;
class bb_reflection_R67;
class bb_reflection_FunctionInfo;
class bb_reflection_R4;
class bb_reflection_R5;
class bb_reflection_R6;
class bb_reflection_R7;
class bb_reflection_R8;
class bb_reflection_R9;
class bb_reflection_R10;
class bb_reflection_R11;
class bb_reflection_R12;
class bb_reflection_R13;
class bb_reflection_R14;
class bb_reflection_R15;
class bb_reflection___GetClass;
class bb_app_App;
class bb_bapp_BApp;
class bb_Beacon_Beacon;
class bb_app_AppDevice;
class bb_reflection_ConstInfo;
class bb_stack_Stack;
class bb_reflection_FieldInfo;
class bb_stack_Stack2;
class bb_reflection_GlobalInfo;
class bb_stack_Stack3;
class bb_reflection_MethodInfo;
class bb_stack_Stack4;
class bb_stack_Stack5;
class bb_reflection_R19;
class bb_reflection_R20;
class bb_reflection_R21;
class bb_reflection_R22;
class bb_reflection_R23;
class bb_reflection_R24;
class bb_reflection_R25;
class bb_reflection_R26;
class bb_reflection_R27;
class bb_reflection_R28;
class bb_reflection_R30;
class bb_reflection_R29;
class bb_reflection_R32;
class bb_reflection_R34;
class bb_reflection_R36;
class bb_reflection_R38;
class bb_reflection_R40;
class bb_reflection_R42;
class bb_reflection_R44;
class bb_reflection_R45;
class bb_reflection_R43;
class bb_reflection_R46;
class bb_reflection_R48;
class bb_reflection_R51;
class bb_reflection_R52;
class bb_reflection_R53;
class bb_reflection_R54;
class bb_reflection_R55;
class bb_reflection_R49;
class bb_reflection_R50;
class bb_reflection_R56;
class bb_reflection_R58;
class bb_reflection_R61;
class bb_reflection_R62;
class bb_reflection_R63;
class bb_reflection_R64;
class bb_reflection_R65;
class bb_reflection_R59;
class bb_reflection_R60;
class bb_reflection_R66;
class bb_reflection_R68;
class bb_reflection_R72;
class bb_reflection_R73;
class bb_reflection_R74;
class bb_reflection_R69;
class bb_reflection_R70;
class bb_reflection_R71;
class bb_reflection_R75;
class bb_reflection_UnknownClass;
class bb_graphics_Image;
class bb_graphics_GraphicsContext;
class bb_graphics_Frame;
class bb_stream_Stream;
class bb_tcpstream_TcpStream;
class bb_databuffer_DataBuffer;
class bb_stream_StreamError;
class bb_stream_StreamWriteError;
class bb_stack_Stack6;
class bb_challengergui_CHGUI;
class bb_fontinterface_Font;
class bb_bitmapfont_BitmapFont;
class bb_bitmapchar_BitMapChar;
class bb_bitmapcharmetrics_BitMapCharMetrics;
class bb_drawingpoint_DrawingPoint;
class bb_edrawmode_eDrawMode;
class bb_edrawalign_eDrawAlign;
class bb_exception_DiddyException : public ThrowableObject{
	public:
	String f_message;
	ThrowableObject* f_cause;
	String f_type;
	String f_fullType;
	bb_exception_DiddyException();
	virtual String m_Message();
	virtual void m_Message2(String);
	virtual ThrowableObject* m_Cause();
	virtual void m_Cause2(ThrowableObject*);
	virtual String m_Type();
	virtual String m_FullType();
	virtual String m_ToString(bool);
	bb_exception_DiddyException* g_new(String,ThrowableObject*);
	void mark();
	String debug();
};
String dbg_type(bb_exception_DiddyException**p){return "DiddyException";}
class bb_reflection_ClassInfo : public Object{
	public:
	String f__name;
	int f__attrs;
	bb_reflection_ClassInfo* f__sclass;
	Array<bb_reflection_ClassInfo* > f__ifaces;
	Array<bb_reflection_ConstInfo* > f__rconsts;
	Array<bb_reflection_ConstInfo* > f__consts;
	Array<bb_reflection_FieldInfo* > f__rfields;
	Array<bb_reflection_FieldInfo* > f__fields;
	Array<bb_reflection_GlobalInfo* > f__rglobals;
	Array<bb_reflection_GlobalInfo* > f__globals;
	Array<bb_reflection_MethodInfo* > f__rmethods;
	Array<bb_reflection_MethodInfo* > f__methods;
	Array<bb_reflection_FunctionInfo* > f__rfunctions;
	Array<bb_reflection_FunctionInfo* > f__functions;
	Array<bb_reflection_FunctionInfo* > f__ctors;
	bb_reflection_ClassInfo();
	virtual String m_Name();
	bb_reflection_ClassInfo* g_new(String,int,bb_reflection_ClassInfo*,Array<bb_reflection_ClassInfo* >);
	bb_reflection_ClassInfo* g_new2();
	virtual int m_Init();
	virtual int m_InitR();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_ClassInfo**p){return "ClassInfo";}
class bb_map_Map : public Object{
	public:
	bb_map_Node* f_root;
	bb_map_Map();
	bb_map_Map* g_new();
	virtual int m_Compare(String,String)=0;
	virtual int m_RotateLeft(bb_map_Node*);
	virtual int m_RotateRight(bb_map_Node*);
	virtual int m_InsertFixup(bb_map_Node*);
	virtual bool m_Set(String,bb_reflection_ClassInfo*);
	virtual bb_map_Node* m_FindNode(String);
	virtual bool m_Contains(String);
	virtual bb_reflection_ClassInfo* m_Get(String);
	void mark();
	String debug();
};
String dbg_type(bb_map_Map**p){return "Map";}
class bb_map_StringMap : public bb_map_Map{
	public:
	bb_map_StringMap();
	bb_map_StringMap* g_new();
	virtual int m_Compare(String,String);
	void mark();
	String debug();
};
String dbg_type(bb_map_StringMap**p){return "StringMap";}
extern bb_map_StringMap* bb_reflection__classesMap;
extern Array<bb_reflection_ClassInfo* > bb_reflection__classes;
class bb_map_Node : public Object{
	public:
	String f_key;
	bb_map_Node* f_right;
	bb_map_Node* f_left;
	bb_reflection_ClassInfo* f_value;
	int f_color;
	bb_map_Node* f_parent;
	bb_map_Node();
	bb_map_Node* g_new(String,bb_reflection_ClassInfo*,int,bb_map_Node*);
	bb_map_Node* g_new2();
	void mark();
	String debug();
};
String dbg_type(bb_map_Node**p){return "Node";}
bb_reflection_ClassInfo* bb_reflection_GetClass(String);
class bb_reflection__GetClass : public Object{
	public:
	bb_reflection__GetClass();
	virtual bb_reflection_ClassInfo* m_GetClass(Object*)=0;
	bb_reflection__GetClass* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection__GetClass**p){return "_GetClass";}
extern bb_reflection__GetClass* bb_reflection__getClass;
bb_reflection_ClassInfo* bb_reflection_GetClass2(Object*);
class bb_exception_AssertException : public bb_exception_DiddyException{
	public:
	bb_exception_AssertException();
	bb_exception_AssertException* g_new(String,ThrowableObject*);
	void mark();
	String debug();
};
String dbg_type(bb_exception_AssertException**p){return "AssertException";}
class bb_exception_ConcurrentModificationException : public bb_exception_DiddyException{
	public:
	bb_exception_ConcurrentModificationException();
	bb_exception_ConcurrentModificationException* g_new(String,ThrowableObject*);
	void mark();
	String debug();
};
String dbg_type(bb_exception_ConcurrentModificationException**p){return "ConcurrentModificationException";}
class bb_exception_IndexOutOfBoundsException : public bb_exception_DiddyException{
	public:
	bb_exception_IndexOutOfBoundsException();
	bb_exception_IndexOutOfBoundsException* g_new(String,ThrowableObject*);
	void mark();
	String debug();
};
String dbg_type(bb_exception_IndexOutOfBoundsException**p){return "IndexOutOfBoundsException";}
class bb_exception_IllegalArgumentException : public bb_exception_DiddyException{
	public:
	bb_exception_IllegalArgumentException();
	bb_exception_IllegalArgumentException* g_new(String,ThrowableObject*);
	void mark();
	String debug();
};
String dbg_type(bb_exception_IllegalArgumentException**p){return "IllegalArgumentException";}
class bb_exception_XMLParseException : public bb_exception_DiddyException{
	public:
	bb_exception_XMLParseException();
	bb_exception_XMLParseException* g_new(String,ThrowableObject*);
	void mark();
	String debug();
};
String dbg_type(bb_exception_XMLParseException**p){return "XMLParseException";}
class bb_boxes_BoolObject : public Object{
	public:
	bool f_value;
	bb_boxes_BoolObject();
	bb_boxes_BoolObject* g_new(bool);
	virtual bool m_ToBool();
	virtual bool m_Equals(bb_boxes_BoolObject*);
	bb_boxes_BoolObject* g_new2();
	void mark();
	String debug();
};
String dbg_type(bb_boxes_BoolObject**p){return "BoolObject";}
class bb_boxes_IntObject : public Object{
	public:
	int f_value;
	bb_boxes_IntObject();
	bb_boxes_IntObject* g_new(int);
	bb_boxes_IntObject* g_new2(Float);
	virtual int m_ToInt();
	virtual Float m_ToFloat();
	virtual String m_ToString2();
	virtual bool m_Equals2(bb_boxes_IntObject*);
	virtual int m_Compare2(bb_boxes_IntObject*);
	bb_boxes_IntObject* g_new3();
	void mark();
	String debug();
};
String dbg_type(bb_boxes_IntObject**p){return "IntObject";}
class bb_boxes_FloatObject : public Object{
	public:
	Float f_value;
	bb_boxes_FloatObject();
	bb_boxes_FloatObject* g_new(int);
	bb_boxes_FloatObject* g_new2(Float);
	virtual int m_ToInt();
	virtual Float m_ToFloat();
	virtual String m_ToString2();
	virtual bool m_Equals3(bb_boxes_FloatObject*);
	virtual int m_Compare3(bb_boxes_FloatObject*);
	bb_boxes_FloatObject* g_new3();
	void mark();
	String debug();
};
String dbg_type(bb_boxes_FloatObject**p){return "FloatObject";}
class bb_boxes_StringObject : public Object{
	public:
	String f_value;
	bb_boxes_StringObject();
	bb_boxes_StringObject* g_new(int);
	bb_boxes_StringObject* g_new2(Float);
	bb_boxes_StringObject* g_new3(String);
	virtual String m_ToString2();
	virtual bool m_Equals4(bb_boxes_StringObject*);
	virtual int m_Compare4(bb_boxes_StringObject*);
	bb_boxes_StringObject* g_new4();
	void mark();
	String debug();
};
String dbg_type(bb_boxes_StringObject**p){return "StringObject";}
Object* bb_boxes_BoxBool(bool);
Object* bb_boxes_BoxInt(int);
Object* bb_boxes_BoxFloat(Float);
Object* bb_boxes_BoxString(String);
bool bb_boxes_UnboxBool(Object*);
int bb_boxes_UnboxInt(Object*);
Float bb_boxes_UnboxFloat(Object*);
String bb_boxes_UnboxString(Object*);
class bb_reflection_R16 : public bb_reflection_ClassInfo{
	public:
	bb_reflection_R16();
	bb_reflection_R16* g_new();
	virtual int m_Init();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R16**p){return "R16";}
class bb_reflection_R17 : public bb_reflection_ClassInfo{
	public:
	bb_reflection_R17();
	bb_reflection_R17* g_new();
	virtual int m_Init();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R17**p){return "R17";}
class bb_reflection_R18 : public bb_reflection_ClassInfo{
	public:
	bb_reflection_R18();
	bb_reflection_R18* g_new();
	virtual int m_Init();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R18**p){return "R18";}
class bb_reflection_R31 : public bb_reflection_ClassInfo{
	public:
	bb_reflection_R31();
	bb_reflection_R31* g_new();
	virtual int m_Init();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R31**p){return "R31";}
class bb_reflection_R33 : public bb_reflection_ClassInfo{
	public:
	bb_reflection_R33();
	bb_reflection_R33* g_new();
	virtual int m_Init();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R33**p){return "R33";}
class bb_reflection_R35 : public bb_reflection_ClassInfo{
	public:
	bb_reflection_R35();
	bb_reflection_R35* g_new();
	virtual int m_Init();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R35**p){return "R35";}
class bb_reflection_R37 : public bb_reflection_ClassInfo{
	public:
	bb_reflection_R37();
	bb_reflection_R37* g_new();
	virtual int m_Init();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R37**p){return "R37";}
class bb_reflection_R39 : public bb_reflection_ClassInfo{
	public:
	bb_reflection_R39();
	bb_reflection_R39* g_new();
	virtual int m_Init();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R39**p){return "R39";}
class bb_reflection_R41 : public bb_reflection_ClassInfo{
	public:
	bb_reflection_R41();
	bb_reflection_R41* g_new();
	virtual int m_Init();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R41**p){return "R41";}
extern bb_reflection_ClassInfo* bb_reflection__boolClass;
class bb_reflection_R47 : public bb_reflection_ClassInfo{
	public:
	bb_reflection_R47();
	bb_reflection_R47* g_new();
	virtual int m_Init();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R47**p){return "R47";}
extern bb_reflection_ClassInfo* bb_reflection__intClass;
class bb_reflection_R57 : public bb_reflection_ClassInfo{
	public:
	bb_reflection_R57();
	bb_reflection_R57* g_new();
	virtual int m_Init();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R57**p){return "R57";}
extern bb_reflection_ClassInfo* bb_reflection__floatClass;
class bb_reflection_R67 : public bb_reflection_ClassInfo{
	public:
	bb_reflection_R67();
	bb_reflection_R67* g_new();
	virtual int m_Init();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R67**p){return "R67";}
extern bb_reflection_ClassInfo* bb_reflection__stringClass;
class bb_reflection_FunctionInfo : public Object{
	public:
	String f__name;
	int f__attrs;
	bb_reflection_ClassInfo* f__retType;
	Array<bb_reflection_ClassInfo* > f__argTypes;
	bb_reflection_FunctionInfo();
	bb_reflection_FunctionInfo* g_new(String,int,bb_reflection_ClassInfo*,Array<bb_reflection_ClassInfo* >);
	bb_reflection_FunctionInfo* g_new2();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_FunctionInfo**p){return "FunctionInfo";}
extern Array<bb_reflection_FunctionInfo* > bb_reflection__functions;
class bb_reflection_R4 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R4();
	bb_reflection_R4* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R4**p){return "R4";}
class bb_reflection_R5 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R5();
	bb_reflection_R5* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R5**p){return "R5";}
class bb_reflection_R6 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R6();
	bb_reflection_R6* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R6**p){return "R6";}
class bb_reflection_R7 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R7();
	bb_reflection_R7* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R7**p){return "R7";}
class bb_reflection_R8 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R8();
	bb_reflection_R8* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R8**p){return "R8";}
class bb_reflection_R9 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R9();
	bb_reflection_R9* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R9**p){return "R9";}
class bb_reflection_R10 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R10();
	bb_reflection_R10* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R10**p){return "R10";}
class bb_reflection_R11 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R11();
	bb_reflection_R11* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R11**p){return "R11";}
class bb_reflection_R12 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R12();
	bb_reflection_R12* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R12**p){return "R12";}
class bb_reflection_R13 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R13();
	bb_reflection_R13* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R13**p){return "R13";}
class bb_reflection_R14 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R14();
	bb_reflection_R14* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R14**p){return "R14";}
class bb_reflection_R15 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R15();
	bb_reflection_R15* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R15**p){return "R15";}
class bb_reflection___GetClass : public bb_reflection__GetClass{
	public:
	bb_reflection___GetClass();
	bb_reflection___GetClass* g_new();
	virtual bb_reflection_ClassInfo* m_GetClass(Object*);
	void mark();
	String debug();
};
String dbg_type(bb_reflection___GetClass**p){return "__GetClass";}
int bb_reflection___init();
extern int bb_reflection__init;
class bb_app_App : public Object{
	public:
	bb_app_App();
	bb_app_App* g_new();
	virtual int m_OnCreate();
	virtual int m_OnUpdate();
	virtual int m_OnSuspend();
	virtual int m_OnResume();
	virtual int m_OnRender();
	virtual int m_OnLoading();
	void mark();
	String debug();
};
String dbg_type(bb_app_App**p){return "App";}
class bb_bapp_BApp : public bb_app_App{
	public:
	bb_challengergui_CHGUI* f_Games;
	bb_challengergui_CHGUI* f_BeaconList;
	bb_challengergui_CHGUI* f_Title;
	bb_challengergui_CHGUI* f_ServerLabel;
	bb_challengergui_CHGUI* f_PwLabel;
	bb_challengergui_CHGUI* f_Pw;
	bb_challengergui_CHGUI* f_On_Off;
	bool f_isOn;
	String f_LastGame;
	bb_bapp_BApp();
	bb_bapp_BApp* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_bapp_BApp**p){return "BApp";}
class bb_Beacon_Beacon : public bb_bapp_BApp{
	public:
	bb_Beacon_Beacon();
	bb_Beacon_Beacon* g_new();
	virtual int m_OnCreate();
	virtual int m_OnRender();
	virtual int m_OnUpdate();
	void mark();
	String debug();
};
String dbg_type(bb_Beacon_Beacon**p){return "Beacon";}
class bb_app_AppDevice : public gxtkApp{
	public:
	bb_app_App* f_app;
	int f_updateRate;
	bb_app_AppDevice();
	bb_app_AppDevice* g_new(bb_app_App*);
	bb_app_AppDevice* g_new2();
	virtual int OnCreate();
	virtual int OnUpdate();
	virtual int OnSuspend();
	virtual int OnResume();
	virtual int OnRender();
	virtual int OnLoading();
	virtual int SetUpdateRate(int);
	void mark();
	String debug();
};
String dbg_type(bb_app_AppDevice**p){return "AppDevice";}
extern gxtkGraphics* bb_graphics_device;
int bb_graphics_SetGraphicsDevice(gxtkGraphics*);
extern gxtkInput* bb_input_device;
int bb_input_SetInputDevice(gxtkInput*);
extern gxtkAudio* bb_audio_device;
int bb_audio_SetAudioDevice(gxtkAudio*);
extern bb_app_AppDevice* bb_app_device;
int bbMain();
class bb_reflection_ConstInfo : public Object{
	public:
	bb_reflection_ConstInfo();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_ConstInfo**p){return "ConstInfo";}
class bb_stack_Stack : public Object{
	public:
	Array<bb_reflection_ConstInfo* > f_data;
	int f_length;
	bb_stack_Stack();
	bb_stack_Stack* g_new();
	bb_stack_Stack* g_new2(Array<bb_reflection_ConstInfo* >);
	virtual int m_Push(bb_reflection_ConstInfo*);
	virtual int m_Push2(Array<bb_reflection_ConstInfo* >,int,int);
	virtual int m_Push3(Array<bb_reflection_ConstInfo* >,int);
	virtual Array<bb_reflection_ConstInfo* > m_ToArray();
	void mark();
	String debug();
};
String dbg_type(bb_stack_Stack**p){return "Stack";}
class bb_reflection_FieldInfo : public Object{
	public:
	String f__name;
	int f__attrs;
	bb_reflection_ClassInfo* f__type;
	bb_reflection_FieldInfo();
	bb_reflection_FieldInfo* g_new(String,int,bb_reflection_ClassInfo*);
	bb_reflection_FieldInfo* g_new2();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_FieldInfo**p){return "FieldInfo";}
class bb_stack_Stack2 : public Object{
	public:
	Array<bb_reflection_FieldInfo* > f_data;
	int f_length;
	bb_stack_Stack2();
	bb_stack_Stack2* g_new();
	bb_stack_Stack2* g_new2(Array<bb_reflection_FieldInfo* >);
	virtual int m_Push4(bb_reflection_FieldInfo*);
	virtual int m_Push5(Array<bb_reflection_FieldInfo* >,int,int);
	virtual int m_Push6(Array<bb_reflection_FieldInfo* >,int);
	virtual Array<bb_reflection_FieldInfo* > m_ToArray();
	void mark();
	String debug();
};
String dbg_type(bb_stack_Stack2**p){return "Stack";}
class bb_reflection_GlobalInfo : public Object{
	public:
	bb_reflection_GlobalInfo();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_GlobalInfo**p){return "GlobalInfo";}
class bb_stack_Stack3 : public Object{
	public:
	Array<bb_reflection_GlobalInfo* > f_data;
	int f_length;
	bb_stack_Stack3();
	bb_stack_Stack3* g_new();
	bb_stack_Stack3* g_new2(Array<bb_reflection_GlobalInfo* >);
	virtual int m_Push7(bb_reflection_GlobalInfo*);
	virtual int m_Push8(Array<bb_reflection_GlobalInfo* >,int,int);
	virtual int m_Push9(Array<bb_reflection_GlobalInfo* >,int);
	virtual Array<bb_reflection_GlobalInfo* > m_ToArray();
	void mark();
	String debug();
};
String dbg_type(bb_stack_Stack3**p){return "Stack";}
class bb_reflection_MethodInfo : public Object{
	public:
	String f__name;
	int f__attrs;
	bb_reflection_ClassInfo* f__retType;
	Array<bb_reflection_ClassInfo* > f__argTypes;
	bb_reflection_MethodInfo();
	bb_reflection_MethodInfo* g_new(String,int,bb_reflection_ClassInfo*,Array<bb_reflection_ClassInfo* >);
	bb_reflection_MethodInfo* g_new2();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_MethodInfo**p){return "MethodInfo";}
class bb_stack_Stack4 : public Object{
	public:
	Array<bb_reflection_MethodInfo* > f_data;
	int f_length;
	bb_stack_Stack4();
	bb_stack_Stack4* g_new();
	bb_stack_Stack4* g_new2(Array<bb_reflection_MethodInfo* >);
	virtual int m_Push10(bb_reflection_MethodInfo*);
	virtual int m_Push11(Array<bb_reflection_MethodInfo* >,int,int);
	virtual int m_Push12(Array<bb_reflection_MethodInfo* >,int);
	virtual Array<bb_reflection_MethodInfo* > m_ToArray();
	void mark();
	String debug();
};
String dbg_type(bb_stack_Stack4**p){return "Stack";}
class bb_stack_Stack5 : public Object{
	public:
	Array<bb_reflection_FunctionInfo* > f_data;
	int f_length;
	bb_stack_Stack5();
	bb_stack_Stack5* g_new();
	bb_stack_Stack5* g_new2(Array<bb_reflection_FunctionInfo* >);
	virtual int m_Push13(bb_reflection_FunctionInfo*);
	virtual int m_Push14(Array<bb_reflection_FunctionInfo* >,int,int);
	virtual int m_Push15(Array<bb_reflection_FunctionInfo* >,int);
	virtual Array<bb_reflection_FunctionInfo* > m_ToArray();
	void mark();
	String debug();
};
String dbg_type(bb_stack_Stack5**p){return "Stack";}
class bb_reflection_R19 : public bb_reflection_FieldInfo{
	public:
	bb_reflection_R19();
	bb_reflection_R19* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R19**p){return "R19";}
class bb_reflection_R20 : public bb_reflection_FieldInfo{
	public:
	bb_reflection_R20();
	bb_reflection_R20* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R20**p){return "R20";}
class bb_reflection_R21 : public bb_reflection_FieldInfo{
	public:
	bb_reflection_R21();
	bb_reflection_R21* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R21**p){return "R21";}
class bb_reflection_R22 : public bb_reflection_FieldInfo{
	public:
	bb_reflection_R22();
	bb_reflection_R22* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R22**p){return "R22";}
class bb_reflection_R23 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R23();
	bb_reflection_R23* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R23**p){return "R23";}
class bb_reflection_R24 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R24();
	bb_reflection_R24* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R24**p){return "R24";}
class bb_reflection_R25 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R25();
	bb_reflection_R25* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R25**p){return "R25";}
class bb_reflection_R26 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R26();
	bb_reflection_R26* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R26**p){return "R26";}
class bb_reflection_R27 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R27();
	bb_reflection_R27* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R27**p){return "R27";}
class bb_reflection_R28 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R28();
	bb_reflection_R28* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R28**p){return "R28";}
class bb_reflection_R30 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R30();
	bb_reflection_R30* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R30**p){return "R30";}
class bb_reflection_R29 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R29();
	bb_reflection_R29* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R29**p){return "R29";}
class bb_reflection_R32 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R32();
	bb_reflection_R32* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R32**p){return "R32";}
class bb_reflection_R34 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R34();
	bb_reflection_R34* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R34**p){return "R34";}
class bb_reflection_R36 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R36();
	bb_reflection_R36* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R36**p){return "R36";}
class bb_reflection_R38 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R38();
	bb_reflection_R38* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R38**p){return "R38";}
class bb_reflection_R40 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R40();
	bb_reflection_R40* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R40**p){return "R40";}
class bb_reflection_R42 : public bb_reflection_FieldInfo{
	public:
	bb_reflection_R42();
	bb_reflection_R42* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R42**p){return "R42";}
class bb_reflection_R44 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R44();
	bb_reflection_R44* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R44**p){return "R44";}
class bb_reflection_R45 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R45();
	bb_reflection_R45* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R45**p){return "R45";}
class bb_reflection_R43 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R43();
	bb_reflection_R43* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R43**p){return "R43";}
class bb_reflection_R46 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R46();
	bb_reflection_R46* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R46**p){return "R46";}
class bb_reflection_R48 : public bb_reflection_FieldInfo{
	public:
	bb_reflection_R48();
	bb_reflection_R48* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R48**p){return "R48";}
class bb_reflection_R51 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R51();
	bb_reflection_R51* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R51**p){return "R51";}
class bb_reflection_R52 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R52();
	bb_reflection_R52* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R52**p){return "R52";}
class bb_reflection_R53 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R53();
	bb_reflection_R53* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R53**p){return "R53";}
class bb_reflection_R54 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R54();
	bb_reflection_R54* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R54**p){return "R54";}
class bb_reflection_R55 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R55();
	bb_reflection_R55* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R55**p){return "R55";}
class bb_reflection_R49 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R49();
	bb_reflection_R49* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R49**p){return "R49";}
class bb_reflection_R50 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R50();
	bb_reflection_R50* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R50**p){return "R50";}
class bb_reflection_R56 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R56();
	bb_reflection_R56* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R56**p){return "R56";}
class bb_reflection_R58 : public bb_reflection_FieldInfo{
	public:
	bb_reflection_R58();
	bb_reflection_R58* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R58**p){return "R58";}
class bb_reflection_R61 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R61();
	bb_reflection_R61* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R61**p){return "R61";}
class bb_reflection_R62 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R62();
	bb_reflection_R62* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R62**p){return "R62";}
class bb_reflection_R63 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R63();
	bb_reflection_R63* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R63**p){return "R63";}
class bb_reflection_R64 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R64();
	bb_reflection_R64* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R64**p){return "R64";}
class bb_reflection_R65 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R65();
	bb_reflection_R65* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R65**p){return "R65";}
class bb_reflection_R59 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R59();
	bb_reflection_R59* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R59**p){return "R59";}
class bb_reflection_R60 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R60();
	bb_reflection_R60* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R60**p){return "R60";}
class bb_reflection_R66 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R66();
	bb_reflection_R66* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R66**p){return "R66";}
class bb_reflection_R68 : public bb_reflection_FieldInfo{
	public:
	bb_reflection_R68();
	bb_reflection_R68* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R68**p){return "R68";}
class bb_reflection_R72 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R72();
	bb_reflection_R72* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R72**p){return "R72";}
class bb_reflection_R73 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R73();
	bb_reflection_R73* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R73**p){return "R73";}
class bb_reflection_R74 : public bb_reflection_MethodInfo{
	public:
	bb_reflection_R74();
	bb_reflection_R74* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R74**p){return "R74";}
class bb_reflection_R69 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R69();
	bb_reflection_R69* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R69**p){return "R69";}
class bb_reflection_R70 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R70();
	bb_reflection_R70* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R70**p){return "R70";}
class bb_reflection_R71 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R71();
	bb_reflection_R71* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R71**p){return "R71";}
class bb_reflection_R75 : public bb_reflection_FunctionInfo{
	public:
	bb_reflection_R75();
	bb_reflection_R75* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_R75**p){return "R75";}
class bb_reflection_UnknownClass : public bb_reflection_ClassInfo{
	public:
	bb_reflection_UnknownClass();
	bb_reflection_UnknownClass* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_reflection_UnknownClass**p){return "UnknownClass";}
extern bb_reflection_ClassInfo* bb_reflection__unknownClass;
class bb_graphics_Image : public Object{
	public:
	gxtkSurface* f_surface;
	int f_width;
	int f_height;
	Array<bb_graphics_Frame* > f_frames;
	int f_flags;
	Float f_tx;
	Float f_ty;
	bb_graphics_Image* f_source;
	bb_graphics_Image();
	static int g_DefaultFlags;
	bb_graphics_Image* g_new();
	virtual int m_SetHandle(Float,Float);
	virtual int m_ApplyFlags(int);
	virtual bb_graphics_Image* m_Init2(gxtkSurface*,int,int);
	virtual bb_graphics_Image* m_Grab(int,int,int,int,int,int,bb_graphics_Image*);
	virtual bb_graphics_Image* m_GrabImage(int,int,int,int,int,int);
	virtual int m_Width();
	virtual int m_Height();
	void mark();
	String debug();
};
String dbg_type(bb_graphics_Image**p){return "Image";}
class bb_graphics_GraphicsContext : public Object{
	public:
	bb_graphics_Image* f_defaultFont;
	bb_graphics_Image* f_font;
	int f_firstChar;
	int f_matrixSp;
	Float f_ix;
	Float f_iy;
	Float f_jx;
	Float f_jy;
	Float f_tx;
	Float f_ty;
	int f_tformed;
	int f_matDirty;
	Float f_color_r;
	Float f_color_g;
	Float f_color_b;
	Float f_alpha;
	int f_blend;
	Float f_scissor_x;
	Float f_scissor_y;
	Float f_scissor_width;
	Float f_scissor_height;
	Array<Float > f_matrixStack;
	bb_graphics_GraphicsContext();
	bb_graphics_GraphicsContext* g_new();
	virtual int m_Validate();
	void mark();
	String debug();
};
String dbg_type(bb_graphics_GraphicsContext**p){return "GraphicsContext";}
extern bb_graphics_GraphicsContext* bb_graphics_context;
String bb_data_FixDataPath(String);
class bb_graphics_Frame : public Object{
	public:
	int f_x;
	int f_y;
	bb_graphics_Frame();
	bb_graphics_Frame* g_new(int,int);
	bb_graphics_Frame* g_new2();
	void mark();
	String debug();
};
String dbg_type(bb_graphics_Frame**p){return "Frame";}
bb_graphics_Image* bb_graphics_LoadImage(String,int,int);
bb_graphics_Image* bb_graphics_LoadImage2(String,int,int,int,int);
int bb_graphics_SetFont(bb_graphics_Image*,int);
extern gxtkGraphics* bb_graphics_renderDevice;
int bb_graphics_SetMatrix(Float,Float,Float,Float,Float,Float);
int bb_graphics_SetMatrix2(Array<Float >);
int bb_graphics_SetColor(Float,Float,Float);
int bb_graphics_SetAlpha(Float);
int bb_graphics_SetBlend(int);
int bb_graphics_DeviceWidth();
int bb_graphics_DeviceHeight();
int bb_graphics_SetScissor(Float,Float,Float,Float);
int bb_graphics_BeginRender();
int bb_graphics_EndRender();
extern int bb_challengergui_CHGUI_MobileMode;
int bb_app_SetUpdateRate(int);
extern String bb_data2_STATUS;
class bb_stream_Stream : public Object{
	public:
	bb_stream_Stream();
	bb_stream_Stream* g_new();
	static bb_databuffer_DataBuffer* g__tmpbuf;
	virtual int m_Write(bb_databuffer_DataBuffer*,int,int)=0;
	virtual void m__Write(int);
	virtual void m_WriteByte(int);
	virtual void m_WriteLine(String);
	virtual int m_Eof()=0;
	virtual int m_Read(bb_databuffer_DataBuffer*,int,int)=0;
	virtual String m_ReadLine();
	virtual void m_Close()=0;
	void mark();
	String debug();
};
String dbg_type(bb_stream_Stream**p){return "Stream";}
class bb_tcpstream_TcpStream : public bb_stream_Stream{
	public:
	BBTcpStream* f__stream;
	bb_tcpstream_TcpStream();
	bb_tcpstream_TcpStream* g_new();
	virtual bool m_Connect(String,int);
	virtual int m_ReadAvail();
	virtual void m_Close();
	virtual int m_Eof();
	virtual int m_Read(bb_databuffer_DataBuffer*,int,int);
	virtual int m_Write(bb_databuffer_DataBuffer*,int,int);
	void mark();
	String debug();
};
String dbg_type(bb_tcpstream_TcpStream**p){return "TcpStream";}
extern bb_tcpstream_TcpStream* bb_data2_Server;
class bb_databuffer_DataBuffer : public BBDataBuffer{
	public:
	bb_databuffer_DataBuffer();
	bb_databuffer_DataBuffer* g_new(int);
	bb_databuffer_DataBuffer* g_new2();
	void mark();
	String debug();
};
String dbg_type(bb_databuffer_DataBuffer**p){return "DataBuffer";}
class bb_stream_StreamError : public ThrowableObject{
	public:
	bb_stream_Stream* f__stream;
	bb_stream_StreamError();
	bb_stream_StreamError* g_new(bb_stream_Stream*);
	bb_stream_StreamError* g_new2();
	void mark();
	String debug();
};
String dbg_type(bb_stream_StreamError**p){return "StreamError";}
class bb_stream_StreamWriteError : public bb_stream_StreamError{
	public:
	bb_stream_StreamWriteError();
	bb_stream_StreamWriteError* g_new(bb_stream_Stream*);
	bb_stream_StreamWriteError* g_new2();
	void mark();
	String debug();
};
String dbg_type(bb_stream_StreamWriteError**p){return "StreamWriteError";}
int bb_protocol_Post(String);
int bb_protocol_RequestGameList();
class bb_stack_Stack6 : public Object{
	public:
	Array<int > f_data;
	int f_length;
	bb_stack_Stack6();
	bb_stack_Stack6* g_new();
	bb_stack_Stack6* g_new2(Array<int >);
	virtual int m_Push16(int);
	virtual int m_Push17(Array<int >,int,int);
	virtual int m_Push18(Array<int >,int);
	virtual Array<int > m_ToArray();
	void mark();
	String debug();
};
String dbg_type(bb_stack_Stack6**p){return "Stack";}
extern bb_bapp_BApp* bb_data2_Game;
class bb_challengergui_CHGUI : public Object{
	public:
	bb_challengergui_CHGUI* f_Parent;
	Float f_Value;
	String f_Text;
	String f_Element;
	Array<bb_challengergui_CHGUI* > f_DropdownItems;
	int f_Visible;
	int f_Minimised;
	Float f_X;
	Float f_Y;
	Float f_W;
	Float f_H;
	int f_Active;
	int f_Shadow;
	int f_Close;
	int f_CloseOver;
	int f_CloseDown;
	int f_Minimise;
	int f_MinimiseOver;
	int f_MinimiseDown;
	int f_HasMenu;
	int f_MenuHeight;
	int f_Tabbed;
	int f_TabHeight;
	Array<bb_challengergui_CHGUI* > f_Buttons;
	int f_Over;
	int f_Down;
	Array<bb_challengergui_CHGUI* > f_ImageButtons;
	bb_graphics_Image* f_Img;
	Array<bb_challengergui_CHGUI* > f_Tickboxes;
	Array<bb_challengergui_CHGUI* > f_Radioboxes;
	Array<bb_challengergui_CHGUI* > f_Listboxes;
	bb_challengergui_CHGUI* f_ListboxSlider;
	int f_ListboxNumber;
	Array<bb_challengergui_CHGUI* > f_ListboxItems;
	int f_ListHeight;
	bb_challengergui_CHGUI* f_SelectedListboxItem;
	Array<bb_challengergui_CHGUI* > f_HSliders;
	int f_MinusOver;
	int f_MinusDown;
	int f_PlusOver;
	int f_PlusDown;
	int f_SliderOver;
	int f_SliderDown;
	Float f_Minimum;
	Float f_Stp;
	Float f_SWidth;
	Array<bb_challengergui_CHGUI* > f_VSliders;
	Array<bb_challengergui_CHGUI* > f_Textfields;
	int f_OnFocus;
	int f_Cursor;
	Array<bb_challengergui_CHGUI* > f_Labels;
	Array<bb_challengergui_CHGUI* > f_Dropdowns;
	int f_DropNumber;
	Array<bb_challengergui_CHGUI* > f_Menus;
	Array<bb_challengergui_CHGUI* > f_Tabs;
	bb_challengergui_CHGUI* f_CurrentTab;
	Array<bb_challengergui_CHGUI* > f_BottomList;
	Array<bb_challengergui_CHGUI* > f_VariList;
	Array<bb_challengergui_CHGUI* > f_TopList;
	Array<bb_challengergui_CHGUI* > f_MenuItems;
	int f_IsMenuParent;
	int f_Tick;
	int f_MenuWidth;
	int f_MenuNumber;
	String f_Tooltip;
	int f_Moveable;
	int f_Mode;
	int f_IsParent;
	int f_SubWindow;
	int f_ReOrdered;
	int f_Clicked;
	int f_DoubleClickMillisecs;
	int f_DoubleClicked;
	int f_StartOvertime;
	int f_OverTime;
	int f_StartDowntime;
	int f_DownTime;
	bb_challengergui_CHGUI* f_TopVari;
	bb_challengergui_CHGUI* f_TopTop;
	bb_challengergui_CHGUI* f_TopBottom;
	bb_challengergui_CHGUI* f_MenuActive;
	bb_challengergui_CHGUI* f_MenuOver;
	int f_FormatText;
	int f_FormatNumber;
	int f_FormatSymbol;
	int f_FormatSpace;
	int f_DKeyMillisecs;
	Float f_Maximum;
	int f_Start;
	int f_Group;
	int f_Moving;
	Float f_MX;
	Float f_MY;
	int f_DClickMillisecs;
	bb_challengergui_CHGUI();
	bb_challengergui_CHGUI* g_new();
	virtual int m_CheckClicked();
	virtual int m_CheckOver();
	virtual int m_CheckDown();
	void mark();
	String debug();
};
String dbg_type(bb_challengergui_CHGUI**p){return "CHGUI";}
bb_challengergui_CHGUI* bb_challengergui_CreateDropdownItem(String,bb_challengergui_CHGUI*,int);
int bb_protocol__readp();
int bb_protocol_ReadProtocol();
int bb_graphics_DebugRenderDevice();
int bb_graphics_Cls(Float,Float,Float);
extern Array<bb_challengergui_CHGUI* > bb_challengergui_CHGUI_BottomList;
extern bb_challengergui_CHGUI* bb_challengergui_CHGUI_Canvas;
int bb_challengergui_CHGUI_RealVisible(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_RealMinimised(bb_challengergui_CHGUI*);
extern Float bb_challengergui_CHGUI_OffsetX;
int bb_challengergui_CHGUI_RealX(bb_challengergui_CHGUI*);
extern Float bb_challengergui_CHGUI_OffsetY;
int bb_challengergui_CHGUI_RealY(bb_challengergui_CHGUI*);
extern Float bb_challengergui_CHGUI_TitleHeight;
extern bb_challengergui_CHGUI* bb_challengergui_CHGUI_LockedWIndow;
int bb_challengergui_CHGUI_RealActive(bb_challengergui_CHGUI*);
extern int bb_challengergui_CHGUI_Shadow;
extern bb_graphics_Image* bb_challengergui_CHGUI_ShadowImg;
int bb_graphics_PushMatrix();
int bb_graphics_Transform(Float,Float,Float,Float,Float,Float);
int bb_graphics_Transform2(Array<Float >);
int bb_graphics_Translate(Float,Float);
int bb_graphics_PopMatrix();
int bb_graphics_DrawImageRect(bb_graphics_Image*,Float,Float,int,int,int,int,int);
int bb_graphics_Rotate(Float);
int bb_graphics_Scale(Float,Float);
int bb_graphics_DrawImageRect2(bb_graphics_Image*,Float,Float,int,int,int,int,Float,Float,Float,int);
extern bb_graphics_Image* bb_challengergui_CHGUI_Style;
class bb_fontinterface_Font : public virtual gc_interface{
	public:
};
class bb_bitmapfont_BitmapFont : public Object,public virtual bb_fontinterface_Font{
	public:
	Array<bb_bitmapchar_BitMapChar* > f_faceChars;
	bool f__drawShadow;
	Array<bb_bitmapchar_BitMapChar* > f_borderChars;
	bb_drawingpoint_DrawingPoint* f__kerning;
	Array<bb_graphics_Image* > f_packedImages;
	Array<bb_bitmapchar_BitMapChar* > f_shadowChars;
	bool f__drawBorder;
	bb_bitmapfont_BitmapFont();
	virtual int m_GetFontHeight();
	virtual bool m_DrawShadow();
	virtual int m_DrawShadow2(bool);
	virtual bb_drawingpoint_DrawingPoint* m_Kerning();
	virtual void m_Kerning2(bb_drawingpoint_DrawingPoint*);
	virtual Float m_GetTxtWidth(String,int,int);
	virtual Float m_GetTxtWidth2(String);
	virtual int m_DrawCharsText(String,Float,Float,Array<bb_bitmapchar_BitMapChar* >,int,int);
	virtual int m_DrawCharsText2(String,Float,Float,int,int);
	virtual bool m_DrawBorder();
	virtual int m_DrawBorder2(bool);
	virtual int m_DrawText(String,Float,Float,int);
	virtual int m_DrawText2(String,Float,Float);
	virtual Float m_GetTxtHeight(String);
	virtual int m_LoadPacked(String,String,bool);
	virtual int m_LoadFontData(String,String,bool);
	bb_bitmapfont_BitmapFont* g_new(String,bool);
	bb_bitmapfont_BitmapFont* g_new2(String);
	bb_bitmapfont_BitmapFont* g_new3();
	static bb_bitmapfont_BitmapFont* g_Load(String,bool);
	void mark();
	String debug();
};
String dbg_type(bb_bitmapfont_BitmapFont**p){return "BitmapFont";}
extern bb_bitmapfont_BitmapFont* bb_challengergui_CHGUI_TitleFont;
class bb_bitmapchar_BitMapChar : public Object{
	public:
	bb_bitmapcharmetrics_BitMapCharMetrics* f_drawingMetrics;
	bb_graphics_Image* f_image;
	String f_imageResourceName;
	String f_imageResourceNameBackup;
	int f_packedFontIndex;
	bb_drawingpoint_DrawingPoint* f_packedPosition;
	bb_drawingpoint_DrawingPoint* f_packedSize;
	bb_bitmapchar_BitMapChar();
	virtual bool m_CharImageLoaded();
	virtual int m_LoadCharImage();
	bb_bitmapchar_BitMapChar* g_new();
	virtual int m_SetImageResourceName(String);
	void mark();
	String debug();
};
String dbg_type(bb_bitmapchar_BitMapChar**p){return "BitMapChar";}
class bb_bitmapcharmetrics_BitMapCharMetrics : public Object{
	public:
	bb_drawingpoint_DrawingPoint* f_drawingSize;
	Float f_drawingWidth;
	bb_drawingpoint_DrawingPoint* f_drawingOffset;
	bb_bitmapcharmetrics_BitMapCharMetrics();
	bb_bitmapcharmetrics_BitMapCharMetrics* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_bitmapcharmetrics_BitMapCharMetrics**p){return "BitMapCharMetrics";}
class bb_drawingpoint_DrawingPoint : public Object{
	public:
	Float f_y;
	Float f_x;
	bb_drawingpoint_DrawingPoint();
	bb_drawingpoint_DrawingPoint* g_new();
	void mark();
	String debug();
};
String dbg_type(bb_drawingpoint_DrawingPoint**p){return "DrawingPoint";}
Float bb_challengergui_CHGUI_TextHeight(bb_bitmapfont_BitmapFont*,String);
class bb_edrawmode_eDrawMode : public Object{
	public:
	bb_edrawmode_eDrawMode();
	void mark();
	String debug();
};
String dbg_type(bb_edrawmode_eDrawMode**p){return "eDrawMode";}
class bb_edrawalign_eDrawAlign : public Object{
	public:
	bb_edrawalign_eDrawAlign();
	void mark();
	String debug();
};
String dbg_type(bb_edrawalign_eDrawAlign**p){return "eDrawAlign";}
int bb_math_Abs(int);
Float bb_math_Abs2(Float);
int bb_graphics_DrawImage(bb_graphics_Image*,Float,Float,int);
int bb_graphics_DrawImage2(bb_graphics_Image*,Float,Float,Float,Float,Float,int);
extern bb_bitmapfont_BitmapFont* bb_challengergui_CHGUI_Font;
int bb_challengergui_CHGUI_DrawWindow(bb_challengergui_CHGUI*);
extern Array<bb_challengergui_CHGUI* > bb_challengergui_CHGUI_KeyboardButtons;
extern int bb_challengergui_CHGUI_ShiftHold;
int bb_challengergui_CHGUI_DrawButton(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_DrawImageButton(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_DrawTickbox(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_DrawRadiobox(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_DrawListbox(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_DrawListboxItem(bb_challengergui_CHGUI*,int);
int bb_challengergui_CHGUI_DrawHSlider(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_DrawVSlider(bb_challengergui_CHGUI*);
extern int bb_challengergui_CHGUI_Cursor;
int bb_graphics_DrawLine(Float,Float,Float,Float);
int bb_challengergui_CHGUI_DrawTextfield(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_DrawLabel(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_DrawDropdown(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_DrawDropdownItem(bb_challengergui_CHGUI*,int);
int bb_challengergui_CHGUI_DrawMenu(bb_challengergui_CHGUI*,int,int);
int bb_challengergui_CHGUI_DrawTab(bb_challengergui_CHGUI*,int);
int bb_challengergui_CHGUI_DrawMenuItem(bb_challengergui_CHGUI*,int);
int bb_challengergui_CHGUI_SubMenu(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_DrawContents(bb_challengergui_CHGUI*);
extern Array<bb_challengergui_CHGUI* > bb_challengergui_CHGUI_VariList;
extern Array<bb_challengergui_CHGUI* > bb_challengergui_CHGUI_TopList;
extern bb_challengergui_CHGUI* bb_challengergui_CHGUI_TooltipFlag;
extern bb_bitmapfont_BitmapFont* bb_challengergui_CHGUI_TooltipFont;
int bb_graphics_DrawRect(Float,Float,Float,Float);
int bb_challengergui_CHGUI_DrawTooltip(bb_challengergui_CHGUI*);
int bb_app_Millisecs();
extern int bb_challengergui_CHGUI_Millisecs;
extern int bb_challengergui_CHGUI_FPSCounter;
extern int bb_challengergui_CHGUI_FPS;
int bb_challengergui_CHGUI_FPSUpdate();
int bb_challengergui_CHGUI_Draw();
extern int bb_challengergui_CHGUI_Width;
extern int bb_challengergui_CHGUI_Height;
extern int bb_challengergui_CHGUI_CanvasFlag;
extern int bb_challengergui_CHGUI_Started;
extern bb_challengergui_CHGUI* bb_challengergui_CHGUI_TopTop;
bb_challengergui_CHGUI* bb_challengergui_CreateWindow(int,int,int,int,String,int,int,int,int,bb_challengergui_CHGUI*);
String bb_app_LoadString(String);
extern bb_challengergui_CHGUI* bb_challengergui_CHGUI_KeyboardWindow;
bb_challengergui_CHGUI* bb_challengergui_CHGUI_CreateKeyButton(int,int,int,int,String,bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_CreateKeyboard();
extern bb_challengergui_CHGUI* bb_challengergui_CHGUI_MsgBoxWindow;
bb_challengergui_CHGUI* bb_challengergui_CreateLabel(int,int,String,bb_challengergui_CHGUI*);
extern bb_challengergui_CHGUI* bb_challengergui_CHGUI_MsgBoxLabel;
bb_challengergui_CHGUI* bb_challengergui_CreateButton(int,int,int,int,String,bb_challengergui_CHGUI*);
extern bb_challengergui_CHGUI* bb_challengergui_CHGUI_MsgBoxButton;
int bb_challengergui_CHGUI_Start();
extern Float bb_data2_SCALE_W;
extern Float bb_data2_SCALE_H;
bb_challengergui_CHGUI* bb_data2_CScale(bb_challengergui_CHGUI*);
bb_challengergui_CHGUI* bb_challengergui_CreateDropdown(int,int,int,int,String,bb_challengergui_CHGUI*);
bb_challengergui_CHGUI* bb_challengergui_CreateTextfield(int,int,int,int,String,bb_challengergui_CHGUI*);
int bb_input_TouchDown(int);
extern int bb_challengergui_CHGUI_MouseBusy;
extern int bb_challengergui_CHGUI_Over;
extern int bb_challengergui_CHGUI_OverFlag;
extern int bb_challengergui_CHGUI_DownFlag;
extern int bb_challengergui_CHGUI_MenuOver;
extern int bb_challengergui_CHGUI_TextBoxOver;
extern int bb_challengergui_CHGUI_TextboxOnFocus;
extern int bb_challengergui_CHGUI_TextBoxDown;
extern int bb_challengergui_CHGUI_DragOver;
extern int bb_challengergui_CHGUI_Moving;
extern Float bb_challengergui_CHGUI_TargetY;
extern Float bb_challengergui_CHGUI_TargetX;
extern int bb_challengergui_CHGUI_IgnoreMouse;
Float bb_input_TouchX(int);
Float bb_input_TouchY(int);
int bb_challengergui_CHGUI_ReorderSubWindows(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_Reorder(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_CloseMenu(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_CloseMenuReverse(bb_challengergui_CHGUI*);
extern int bb_challengergui_CHGUI_Tooltips;
extern int bb_challengergui_CHGUI_TooltipTime;
extern int bb_challengergui_CHGUI_MenuClose;
int bb_challengergui_CHGUI_UpdateMenuItem(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateSubMenu(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateTab(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateMenu(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateDropdownItem(bb_challengergui_CHGUI*,int);
int bb_challengergui_CHGUI_UpdateDropdown(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateLabel(bb_challengergui_CHGUI*);
extern bb_challengergui_CHGUI* bb_challengergui_CHGUI_TextboxFocus;
extern int bb_challengergui_CHGUI_Keyboard;
int bb_input_EnableKeyboard();
extern int bb_challengergui_CHGUI_ShowKeyboard;
extern int bb_challengergui_CHGUI_AutoTextScroll;
int bb_input_GetChar();
int bb_challengergui_CHGUI_GetText(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateKeyboardSizes();
extern int bb_challengergui_CHGUI_KeyboardPage;
extern int bb_challengergui_CHGUI_KeyboardShift;
extern Float bb_challengergui_CHGUI_OldX;
extern Float bb_challengergui_CHGUI_OldY;
int bb_challengergui_CHGUI_UpdateKeyboard(bb_challengergui_CHGUI*);
int bb_input_DisableKeyboard();
int bb_challengergui_CHGUI_UpdateTextfield(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateHSlider(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateVSlider(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateListboxItem(bb_challengergui_CHGUI*,int);
int bb_challengergui_CHGUI_UpdateListbox(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateRadiobox(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateTickbox(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateImageButton(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateButton(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_Locked(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateWindow(bb_challengergui_CHGUI*);
int bb_challengergui_CHGUI_UpdateContents(bb_challengergui_CHGUI*);
extern int bb_challengergui_CHGUI_CursorMillisecs;
extern int bb_challengergui_CHGUI_DragScroll;
extern int bb_challengergui_CHGUI_DragMoving;
extern Float bb_challengergui_CHGUI_OffsetMX;
extern Float bb_challengergui_CHGUI_OffsetMY;
int bb_challengergui_LockFocus(bb_challengergui_CHGUI*);
int bb_challengergui_UnlockFocus();
int bb_challengergui_CHGUI_UpdateMsgBox();
int bb_challengergui_CHGUI_Update();
int bb_challengergui_CHGUI_Delete(bb_challengergui_CHGUI*);
int bb_protocol_RequestBeaconList(String);
bb_exception_DiddyException::bb_exception_DiddyException(){
	f_message=String();
	f_cause=0;
	f_type=String();
	f_fullType=String();
}
String bb_exception_DiddyException::m_Message(){
	DBG_ENTER("DiddyException.Message")
	bb_exception_DiddyException *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<15>");
	return f_message;
}
void bb_exception_DiddyException::m_Message2(String t_message){
	DBG_ENTER("DiddyException.Message")
	bb_exception_DiddyException *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_message,"message")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<19>");
	this->f_message=t_message;
}
ThrowableObject* bb_exception_DiddyException::m_Cause(){
	DBG_ENTER("DiddyException.Cause")
	bb_exception_DiddyException *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<23>");
	return f_cause;
}
void bb_exception_DiddyException::m_Cause2(ThrowableObject* t_cause){
	DBG_ENTER("DiddyException.Cause")
	bb_exception_DiddyException *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_cause,"cause")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<27>");
	if(t_cause==(this)){
		DBG_BLOCK();
		t_cause=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<28>");
	gc_assign(this->f_cause,t_cause);
}
String bb_exception_DiddyException::m_Type(){
	DBG_ENTER("DiddyException.Type")
	bb_exception_DiddyException *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<32>");
	return f_type;
}
String bb_exception_DiddyException::m_FullType(){
	DBG_ENTER("DiddyException.FullType")
	bb_exception_DiddyException *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<36>");
	return f_fullType;
}
String bb_exception_DiddyException::m_ToString(bool t_recurse){
	DBG_ENTER("DiddyException.ToString")
	bb_exception_DiddyException *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_recurse,"recurse")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<56>");
	String t_rv=f_type+String(L": ",2)+f_message;
	DBG_LOCAL(t_rv,"rv")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<57>");
	if(t_recurse){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<58>");
		int t_depth=10;
		DBG_LOCAL(t_depth,"depth")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<59>");
		ThrowableObject* t_current=f_cause;
		DBG_LOCAL(t_current,"current")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<60>");
		while(((t_current)!=0) && t_depth>0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<61>");
			if((dynamic_cast<bb_exception_DiddyException*>(t_current))!=0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<62>");
				t_rv=t_rv+(String(L"\nCaused by ",11)+f_type+String(L": ",2)+dynamic_cast<bb_exception_DiddyException*>(t_current)->f_message);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<63>");
				t_current=dynamic_cast<bb_exception_DiddyException*>(t_current)->f_cause;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<64>");
				t_depth-=1;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<66>");
				t_rv=t_rv+String(L"\nCaused by a non-Diddy exception.",33);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<67>");
				t_current=0;
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<71>");
	return t_rv;
}
bb_exception_DiddyException* bb_exception_DiddyException::g_new(String t_message,ThrowableObject* t_cause){
	DBG_ENTER("DiddyException.new")
	bb_exception_DiddyException *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_message,"message")
	DBG_LOCAL(t_cause,"cause")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<40>");
	this->f_message=t_message;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<41>");
	gc_assign(this->f_cause,t_cause);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<42>");
	bb_reflection_ClassInfo* t_ci=bb_reflection_GetClass2(this);
	DBG_LOCAL(t_ci,"ci")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<43>");
	if((t_ci)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<44>");
		this->f_fullType=t_ci->m_Name();
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<46>");
		this->f_fullType=String(L"diddy.exception.DiddyException",30);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<48>");
	if(this->f_fullType.Contains(String(L".",1))){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<49>");
		this->f_type=this->f_fullType.Slice(this->f_fullType.FindLast(String(L".",1))+1);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<51>");
		this->f_type=this->f_fullType;
	}
	return this;
}
void bb_exception_DiddyException::mark(){
	ThrowableObject::mark();
	gc_mark_q(f_cause);
}
String bb_exception_DiddyException::debug(){
	String t="(DiddyException)\n";
	t+=dbg_decl("message",&f_message);
	t+=dbg_decl("cause",&f_cause);
	t+=dbg_decl("type",&f_type);
	t+=dbg_decl("fullType",&f_fullType);
	return t;
}
bb_reflection_ClassInfo::bb_reflection_ClassInfo(){
	f__name=String();
	f__attrs=0;
	f__sclass=0;
	f__ifaces=Array<bb_reflection_ClassInfo* >();
	f__rconsts=Array<bb_reflection_ConstInfo* >();
	f__consts=Array<bb_reflection_ConstInfo* >();
	f__rfields=Array<bb_reflection_FieldInfo* >();
	f__fields=Array<bb_reflection_FieldInfo* >();
	f__rglobals=Array<bb_reflection_GlobalInfo* >();
	f__globals=Array<bb_reflection_GlobalInfo* >();
	f__rmethods=Array<bb_reflection_MethodInfo* >();
	f__methods=Array<bb_reflection_MethodInfo* >();
	f__rfunctions=Array<bb_reflection_FunctionInfo* >();
	f__functions=Array<bb_reflection_FunctionInfo* >();
	f__ctors=Array<bb_reflection_FunctionInfo* >();
}
String bb_reflection_ClassInfo::m_Name(){
	DBG_ENTER("ClassInfo.Name")
	bb_reflection_ClassInfo *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<222>");
	return f__name;
}
bb_reflection_ClassInfo* bb_reflection_ClassInfo::g_new(String t_name,int t_attrs,bb_reflection_ClassInfo* t_sclass,Array<bb_reflection_ClassInfo* > t_ifaces){
	DBG_ENTER("ClassInfo.new")
	bb_reflection_ClassInfo *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_name,"name")
	DBG_LOCAL(t_attrs,"attrs")
	DBG_LOCAL(t_sclass,"sclass")
	DBG_LOCAL(t_ifaces,"ifaces")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<215>");
	f__name=t_name;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<216>");
	f__attrs=t_attrs;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<217>");
	gc_assign(f__sclass,t_sclass);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<218>");
	gc_assign(f__ifaces,t_ifaces);
	return this;
}
bb_reflection_ClassInfo* bb_reflection_ClassInfo::g_new2(){
	DBG_ENTER("ClassInfo.new")
	bb_reflection_ClassInfo *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<212>");
	return this;
}
int bb_reflection_ClassInfo::m_Init(){
	DBG_ENTER("ClassInfo.Init")
	bb_reflection_ClassInfo *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int bb_reflection_ClassInfo::m_InitR(){
	DBG_ENTER("ClassInfo.InitR")
	bb_reflection_ClassInfo *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<421>");
	if((f__sclass)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<422>");
		bb_stack_Stack* t_consts=(new bb_stack_Stack)->g_new2(f__sclass->f__rconsts);
		DBG_LOCAL(t_consts,"consts")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<423>");
		Array<bb_reflection_ConstInfo* > t_=f__consts;
		int t_2=0;
		while(t_2<t_.Length()){
			DBG_BLOCK();
			bb_reflection_ConstInfo* t_t=t_.At(t_2);
			t_2=t_2+1;
			DBG_LOCAL(t_t,"t")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<424>");
			t_consts->m_Push(t_t);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<426>");
		gc_assign(f__rconsts,t_consts->m_ToArray());
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<427>");
		bb_stack_Stack2* t_fields=(new bb_stack_Stack2)->g_new2(f__sclass->f__rfields);
		DBG_LOCAL(t_fields,"fields")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<428>");
		Array<bb_reflection_FieldInfo* > t_3=f__fields;
		int t_4=0;
		while(t_4<t_3.Length()){
			DBG_BLOCK();
			bb_reflection_FieldInfo* t_t2=t_3.At(t_4);
			t_4=t_4+1;
			DBG_LOCAL(t_t2,"t")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<429>");
			t_fields->m_Push4(t_t2);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<431>");
		gc_assign(f__rfields,t_fields->m_ToArray());
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<432>");
		bb_stack_Stack3* t_globals=(new bb_stack_Stack3)->g_new2(f__sclass->f__rglobals);
		DBG_LOCAL(t_globals,"globals")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<433>");
		Array<bb_reflection_GlobalInfo* > t_5=f__globals;
		int t_6=0;
		while(t_6<t_5.Length()){
			DBG_BLOCK();
			bb_reflection_GlobalInfo* t_t3=t_5.At(t_6);
			t_6=t_6+1;
			DBG_LOCAL(t_t3,"t")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<434>");
			t_globals->m_Push7(t_t3);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<436>");
		gc_assign(f__rglobals,t_globals->m_ToArray());
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<437>");
		bb_stack_Stack4* t_methods=(new bb_stack_Stack4)->g_new2(f__sclass->f__rmethods);
		DBG_LOCAL(t_methods,"methods")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<438>");
		Array<bb_reflection_MethodInfo* > t_7=f__methods;
		int t_8=0;
		while(t_8<t_7.Length()){
			DBG_BLOCK();
			bb_reflection_MethodInfo* t_t4=t_7.At(t_8);
			t_8=t_8+1;
			DBG_LOCAL(t_t4,"t")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<439>");
			t_methods->m_Push10(t_t4);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<441>");
		gc_assign(f__rmethods,t_methods->m_ToArray());
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<442>");
		bb_stack_Stack5* t_functions=(new bb_stack_Stack5)->g_new2(f__sclass->f__rfunctions);
		DBG_LOCAL(t_functions,"functions")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<443>");
		Array<bb_reflection_FunctionInfo* > t_9=f__functions;
		int t_10=0;
		while(t_10<t_9.Length()){
			DBG_BLOCK();
			bb_reflection_FunctionInfo* t_t5=t_9.At(t_10);
			t_10=t_10+1;
			DBG_LOCAL(t_t5,"t")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<444>");
			t_functions->m_Push13(t_t5);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<446>");
		gc_assign(f__rfunctions,t_functions->m_ToArray());
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<448>");
		gc_assign(f__rconsts,f__consts);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<449>");
		gc_assign(f__rfields,f__fields);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<450>");
		gc_assign(f__rglobals,f__globals);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<451>");
		gc_assign(f__rmethods,f__methods);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<452>");
		gc_assign(f__rfunctions,f__functions);
	}
	return 0;
}
void bb_reflection_ClassInfo::mark(){
	Object::mark();
	gc_mark_q(f__sclass);
	gc_mark_q(f__ifaces);
	gc_mark_q(f__rconsts);
	gc_mark_q(f__consts);
	gc_mark_q(f__rfields);
	gc_mark_q(f__fields);
	gc_mark_q(f__rglobals);
	gc_mark_q(f__globals);
	gc_mark_q(f__rmethods);
	gc_mark_q(f__methods);
	gc_mark_q(f__rfunctions);
	gc_mark_q(f__functions);
	gc_mark_q(f__ctors);
}
String bb_reflection_ClassInfo::debug(){
	String t="(ClassInfo)\n";
	t+=dbg_decl("_name",&f__name);
	t+=dbg_decl("_attrs",&f__attrs);
	t+=dbg_decl("_sclass",&f__sclass);
	t+=dbg_decl("_ifaces",&f__ifaces);
	t+=dbg_decl("_consts",&f__consts);
	t+=dbg_decl("_fields",&f__fields);
	t+=dbg_decl("_globals",&f__globals);
	t+=dbg_decl("_methods",&f__methods);
	t+=dbg_decl("_functions",&f__functions);
	t+=dbg_decl("_ctors",&f__ctors);
	t+=dbg_decl("_rconsts",&f__rconsts);
	t+=dbg_decl("_rglobals",&f__rglobals);
	t+=dbg_decl("_rfields",&f__rfields);
	t+=dbg_decl("_rmethods",&f__rmethods);
	t+=dbg_decl("_rfunctions",&f__rfunctions);
	return t;
}
bb_map_Map::bb_map_Map(){
	f_root=0;
}
bb_map_Map* bb_map_Map::g_new(){
	DBG_ENTER("Map.new")
	bb_map_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<7>");
	return this;
}
int bb_map_Map::m_RotateLeft(bb_map_Node* t_node){
	DBG_ENTER("Map.RotateLeft")
	bb_map_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<251>");
	bb_map_Node* t_child=t_node->f_right;
	DBG_LOCAL(t_child,"child")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<252>");
	gc_assign(t_node->f_right,t_child->f_left);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<253>");
	if((t_child->f_left)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<254>");
		gc_assign(t_child->f_left->f_parent,t_node);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<256>");
	gc_assign(t_child->f_parent,t_node->f_parent);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<257>");
	if((t_node->f_parent)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<258>");
		if(t_node==t_node->f_parent->f_left){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<259>");
			gc_assign(t_node->f_parent->f_left,t_child);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<261>");
			gc_assign(t_node->f_parent->f_right,t_child);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<264>");
		gc_assign(f_root,t_child);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<266>");
	gc_assign(t_child->f_left,t_node);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<267>");
	gc_assign(t_node->f_parent,t_child);
	return 0;
}
int bb_map_Map::m_RotateRight(bb_map_Node* t_node){
	DBG_ENTER("Map.RotateRight")
	bb_map_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<271>");
	bb_map_Node* t_child=t_node->f_left;
	DBG_LOCAL(t_child,"child")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<272>");
	gc_assign(t_node->f_left,t_child->f_right);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<273>");
	if((t_child->f_right)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<274>");
		gc_assign(t_child->f_right->f_parent,t_node);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<276>");
	gc_assign(t_child->f_parent,t_node->f_parent);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<277>");
	if((t_node->f_parent)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<278>");
		if(t_node==t_node->f_parent->f_right){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<279>");
			gc_assign(t_node->f_parent->f_right,t_child);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<281>");
			gc_assign(t_node->f_parent->f_left,t_child);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<284>");
		gc_assign(f_root,t_child);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<286>");
	gc_assign(t_child->f_right,t_node);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<287>");
	gc_assign(t_node->f_parent,t_child);
	return 0;
}
int bb_map_Map::m_InsertFixup(bb_map_Node* t_node){
	DBG_ENTER("Map.InsertFixup")
	bb_map_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_node,"node")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<212>");
	while(((t_node->f_parent)!=0) && t_node->f_parent->f_color==-1 && ((t_node->f_parent->f_parent)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<213>");
		if(t_node->f_parent==t_node->f_parent->f_parent->f_left){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<214>");
			bb_map_Node* t_uncle=t_node->f_parent->f_parent->f_right;
			DBG_LOCAL(t_uncle,"uncle")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<215>");
			if(((t_uncle)!=0) && t_uncle->f_color==-1){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<216>");
				t_node->f_parent->f_color=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<217>");
				t_uncle->f_color=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<218>");
				t_uncle->f_parent->f_color=-1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<219>");
				t_node=t_uncle->f_parent;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<221>");
				if(t_node==t_node->f_parent->f_right){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<222>");
					t_node=t_node->f_parent;
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<223>");
					m_RotateLeft(t_node);
				}
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<225>");
				t_node->f_parent->f_color=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<226>");
				t_node->f_parent->f_parent->f_color=-1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<227>");
				m_RotateRight(t_node->f_parent->f_parent);
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<230>");
			bb_map_Node* t_uncle2=t_node->f_parent->f_parent->f_left;
			DBG_LOCAL(t_uncle2,"uncle")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<231>");
			if(((t_uncle2)!=0) && t_uncle2->f_color==-1){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<232>");
				t_node->f_parent->f_color=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<233>");
				t_uncle2->f_color=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<234>");
				t_uncle2->f_parent->f_color=-1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<235>");
				t_node=t_uncle2->f_parent;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<237>");
				if(t_node==t_node->f_parent->f_left){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<238>");
					t_node=t_node->f_parent;
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<239>");
					m_RotateRight(t_node);
				}
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<241>");
				t_node->f_parent->f_color=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<242>");
				t_node->f_parent->f_parent->f_color=-1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<243>");
				m_RotateLeft(t_node->f_parent->f_parent);
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<247>");
	f_root->f_color=1;
	return 0;
}
bool bb_map_Map::m_Set(String t_key,bb_reflection_ClassInfo* t_value){
	DBG_ENTER("Map.Set")
	bb_map_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<29>");
	bb_map_Node* t_node=f_root;
	DBG_LOCAL(t_node,"node")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<30>");
	bb_map_Node* t_parent=0;
	int t_cmp=0;
	DBG_LOCAL(t_parent,"parent")
	DBG_LOCAL(t_cmp,"cmp")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<32>");
	while((t_node)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<33>");
		t_parent=t_node;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<34>");
		t_cmp=m_Compare(t_key,t_node->f_key);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<35>");
		if(t_cmp>0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<36>");
			t_node=t_node->f_right;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<37>");
			if(t_cmp<0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<38>");
				t_node=t_node->f_left;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<40>");
				gc_assign(t_node->f_value,t_value);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<41>");
				return false;
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<45>");
	t_node=(new bb_map_Node)->g_new(t_key,t_value,-1,t_parent);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<47>");
	if((t_parent)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<48>");
		if(t_cmp>0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<49>");
			gc_assign(t_parent->f_right,t_node);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<51>");
			gc_assign(t_parent->f_left,t_node);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<53>");
		m_InsertFixup(t_node);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<55>");
		gc_assign(f_root,t_node);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<57>");
	return true;
}
bb_map_Node* bb_map_Map::m_FindNode(String t_key){
	DBG_ENTER("Map.FindNode")
	bb_map_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<157>");
	bb_map_Node* t_node=f_root;
	DBG_LOCAL(t_node,"node")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<159>");
	while((t_node)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<160>");
		int t_cmp=m_Compare(t_key,t_node->f_key);
		DBG_LOCAL(t_cmp,"cmp")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<161>");
		if(t_cmp>0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<162>");
			t_node=t_node->f_right;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<163>");
			if(t_cmp<0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<164>");
				t_node=t_node->f_left;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<166>");
				return t_node;
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<169>");
	return t_node;
}
bool bb_map_Map::m_Contains(String t_key){
	DBG_ENTER("Map.Contains")
	bb_map_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<25>");
	bool t_=m_FindNode(t_key)!=0;
	return t_;
}
bb_reflection_ClassInfo* bb_map_Map::m_Get(String t_key){
	DBG_ENTER("Map.Get")
	bb_map_Map *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<101>");
	bb_map_Node* t_node=m_FindNode(t_key);
	DBG_LOCAL(t_node,"node")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<102>");
	if((t_node)!=0){
		DBG_BLOCK();
		return t_node->f_value;
	}
	return 0;
}
void bb_map_Map::mark(){
	Object::mark();
	gc_mark_q(f_root);
}
String bb_map_Map::debug(){
	String t="(Map)\n";
	t+=dbg_decl("root",&f_root);
	return t;
}
bb_map_StringMap::bb_map_StringMap(){
}
bb_map_StringMap* bb_map_StringMap::g_new(){
	DBG_ENTER("StringMap.new")
	bb_map_StringMap *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<551>");
	bb_map_Map::g_new();
	return this;
}
int bb_map_StringMap::m_Compare(String t_lhs,String t_rhs){
	DBG_ENTER("StringMap.Compare")
	bb_map_StringMap *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_lhs,"lhs")
	DBG_LOCAL(t_rhs,"rhs")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<554>");
	int t_=t_lhs.Compare(t_rhs);
	return t_;
}
void bb_map_StringMap::mark(){
	bb_map_Map::mark();
}
String bb_map_StringMap::debug(){
	String t="(StringMap)\n";
	t=bb_map_Map::debug()+t;
	return t;
}
bb_map_StringMap* bb_reflection__classesMap;
Array<bb_reflection_ClassInfo* > bb_reflection__classes;
bb_map_Node::bb_map_Node(){
	f_key=String();
	f_right=0;
	f_left=0;
	f_value=0;
	f_color=0;
	f_parent=0;
}
bb_map_Node* bb_map_Node::g_new(String t_key,bb_reflection_ClassInfo* t_value,int t_color,bb_map_Node* t_parent){
	DBG_ENTER("Node.new")
	bb_map_Node *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_key,"key")
	DBG_LOCAL(t_value,"value")
	DBG_LOCAL(t_color,"color")
	DBG_LOCAL(t_parent,"parent")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<364>");
	this->f_key=t_key;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<365>");
	gc_assign(this->f_value,t_value);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<366>");
	this->f_color=t_color;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<367>");
	gc_assign(this->f_parent,t_parent);
	return this;
}
bb_map_Node* bb_map_Node::g_new2(){
	DBG_ENTER("Node.new")
	bb_map_Node *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/map.monkey<361>");
	return this;
}
void bb_map_Node::mark(){
	Object::mark();
	gc_mark_q(f_right);
	gc_mark_q(f_left);
	gc_mark_q(f_value);
	gc_mark_q(f_parent);
}
String bb_map_Node::debug(){
	String t="(Node)\n";
	t+=dbg_decl("key",&f_key);
	t+=dbg_decl("value",&f_value);
	t+=dbg_decl("color",&f_color);
	t+=dbg_decl("parent",&f_parent);
	t+=dbg_decl("left",&f_left);
	t+=dbg_decl("right",&f_right);
	return t;
}
bb_reflection_ClassInfo* bb_reflection_GetClass(String t_name){
	DBG_ENTER("GetClass")
	DBG_LOCAL(t_name,"name")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<463>");
	if(!((bb_reflection__classesMap)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<464>");
		gc_assign(bb_reflection__classesMap,(new bb_map_StringMap)->g_new());
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<465>");
		Array<bb_reflection_ClassInfo* > t_=bb_reflection__classes;
		int t_2=0;
		while(t_2<t_.Length()){
			DBG_BLOCK();
			bb_reflection_ClassInfo* t_c=t_.At(t_2);
			t_2=t_2+1;
			DBG_LOCAL(t_c,"c")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<466>");
			String t_name2=t_c->m_Name();
			DBG_LOCAL(t_name2,"name")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<467>");
			bb_reflection__classesMap->m_Set(t_name2,t_c);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<468>");
			int t_i=t_name2.FindLast(String(L".",1));
			DBG_LOCAL(t_i,"i")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<469>");
			if(t_i==-1){
				DBG_BLOCK();
				continue;
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<470>");
			t_name2=t_name2.Slice(t_i+1);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<471>");
			if(bb_reflection__classesMap->m_Contains(t_name2)){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<472>");
				bb_reflection__classesMap->m_Set(t_name2,0);
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<474>");
				bb_reflection__classesMap->m_Set(t_name2,t_c);
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<478>");
	bb_reflection_ClassInfo* t_3=bb_reflection__classesMap->m_Get(t_name);
	return t_3;
}
bb_reflection__GetClass::bb_reflection__GetClass(){
}
bb_reflection__GetClass* bb_reflection__GetClass::g_new(){
	DBG_ENTER("_GetClass.new")
	bb_reflection__GetClass *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<608>");
	return this;
}
void bb_reflection__GetClass::mark(){
	Object::mark();
}
String bb_reflection__GetClass::debug(){
	String t="(_GetClass)\n";
	return t;
}
bb_reflection__GetClass* bb_reflection__getClass;
bb_reflection_ClassInfo* bb_reflection_GetClass2(Object* t_obj){
	DBG_ENTER("GetClass")
	DBG_LOCAL(t_obj,"obj")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<482>");
	bb_reflection_ClassInfo* t_=bb_reflection__getClass->m_GetClass(t_obj);
	return t_;
}
bb_exception_AssertException::bb_exception_AssertException(){
}
bb_exception_AssertException* bb_exception_AssertException::g_new(String t_message,ThrowableObject* t_cause){
	DBG_ENTER("AssertException.new")
	bb_exception_AssertException *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_message,"message")
	DBG_LOCAL(t_cause,"cause")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<79>");
	bb_exception_DiddyException::g_new(t_message,t_cause);
	return this;
}
void bb_exception_AssertException::mark(){
	bb_exception_DiddyException::mark();
}
String bb_exception_AssertException::debug(){
	String t="(AssertException)\n";
	t=bb_exception_DiddyException::debug()+t;
	return t;
}
bb_exception_ConcurrentModificationException::bb_exception_ConcurrentModificationException(){
}
bb_exception_ConcurrentModificationException* bb_exception_ConcurrentModificationException::g_new(String t_message,ThrowableObject* t_cause){
	DBG_ENTER("ConcurrentModificationException.new")
	bb_exception_ConcurrentModificationException *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_message,"message")
	DBG_LOCAL(t_cause,"cause")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<85>");
	bb_exception_DiddyException::g_new(t_message,t_cause);
	return this;
}
void bb_exception_ConcurrentModificationException::mark(){
	bb_exception_DiddyException::mark();
}
String bb_exception_ConcurrentModificationException::debug(){
	String t="(ConcurrentModificationException)\n";
	t=bb_exception_DiddyException::debug()+t;
	return t;
}
bb_exception_IndexOutOfBoundsException::bb_exception_IndexOutOfBoundsException(){
}
bb_exception_IndexOutOfBoundsException* bb_exception_IndexOutOfBoundsException::g_new(String t_message,ThrowableObject* t_cause){
	DBG_ENTER("IndexOutOfBoundsException.new")
	bb_exception_IndexOutOfBoundsException *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_message,"message")
	DBG_LOCAL(t_cause,"cause")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<91>");
	bb_exception_DiddyException::g_new(t_message,t_cause);
	return this;
}
void bb_exception_IndexOutOfBoundsException::mark(){
	bb_exception_DiddyException::mark();
}
String bb_exception_IndexOutOfBoundsException::debug(){
	String t="(IndexOutOfBoundsException)\n";
	t=bb_exception_DiddyException::debug()+t;
	return t;
}
bb_exception_IllegalArgumentException::bb_exception_IllegalArgumentException(){
}
bb_exception_IllegalArgumentException* bb_exception_IllegalArgumentException::g_new(String t_message,ThrowableObject* t_cause){
	DBG_ENTER("IllegalArgumentException.new")
	bb_exception_IllegalArgumentException *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_message,"message")
	DBG_LOCAL(t_cause,"cause")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<97>");
	bb_exception_DiddyException::g_new(t_message,t_cause);
	return this;
}
void bb_exception_IllegalArgumentException::mark(){
	bb_exception_DiddyException::mark();
}
String bb_exception_IllegalArgumentException::debug(){
	String t="(IllegalArgumentException)\n";
	t=bb_exception_DiddyException::debug()+t;
	return t;
}
bb_exception_XMLParseException::bb_exception_XMLParseException(){
}
bb_exception_XMLParseException* bb_exception_XMLParseException::g_new(String t_message,ThrowableObject* t_cause){
	DBG_ENTER("XMLParseException.new")
	bb_exception_XMLParseException *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_message,"message")
	DBG_LOCAL(t_cause,"cause")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/diddy/exception.monkey<103>");
	bb_exception_DiddyException::g_new(t_message,t_cause);
	return this;
}
void bb_exception_XMLParseException::mark(){
	bb_exception_DiddyException::mark();
}
String bb_exception_XMLParseException::debug(){
	String t="(XMLParseException)\n";
	t=bb_exception_DiddyException::debug()+t;
	return t;
}
bb_boxes_BoolObject::bb_boxes_BoolObject(){
	f_value=false;
}
bb_boxes_BoolObject* bb_boxes_BoolObject::g_new(bool t_value){
	DBG_ENTER("BoolObject.new")
	bb_boxes_BoolObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<11>");
	this->f_value=t_value;
	return this;
}
bool bb_boxes_BoolObject::m_ToBool(){
	DBG_ENTER("BoolObject.ToBool")
	bb_boxes_BoolObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<15>");
	return f_value;
}
bool bb_boxes_BoolObject::m_Equals(bb_boxes_BoolObject* t_box){
	DBG_ENTER("BoolObject.Equals")
	bb_boxes_BoolObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_box,"box")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<19>");
	bool t_=f_value==t_box->f_value;
	return t_;
}
bb_boxes_BoolObject* bb_boxes_BoolObject::g_new2(){
	DBG_ENTER("BoolObject.new")
	bb_boxes_BoolObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<7>");
	return this;
}
void bb_boxes_BoolObject::mark(){
	Object::mark();
}
String bb_boxes_BoolObject::debug(){
	String t="(BoolObject)\n";
	t+=dbg_decl("value",&f_value);
	return t;
}
bb_boxes_IntObject::bb_boxes_IntObject(){
	f_value=0;
}
bb_boxes_IntObject* bb_boxes_IntObject::g_new(int t_value){
	DBG_ENTER("IntObject.new")
	bb_boxes_IntObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<27>");
	this->f_value=t_value;
	return this;
}
bb_boxes_IntObject* bb_boxes_IntObject::g_new2(Float t_value){
	DBG_ENTER("IntObject.new")
	bb_boxes_IntObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<31>");
	this->f_value=int(t_value);
	return this;
}
int bb_boxes_IntObject::m_ToInt(){
	DBG_ENTER("IntObject.ToInt")
	bb_boxes_IntObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<35>");
	return f_value;
}
Float bb_boxes_IntObject::m_ToFloat(){
	DBG_ENTER("IntObject.ToFloat")
	bb_boxes_IntObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<39>");
	Float t_=Float(f_value);
	return t_;
}
String bb_boxes_IntObject::m_ToString2(){
	DBG_ENTER("IntObject.ToString")
	bb_boxes_IntObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<43>");
	String t_=String(f_value);
	return t_;
}
bool bb_boxes_IntObject::m_Equals2(bb_boxes_IntObject* t_box){
	DBG_ENTER("IntObject.Equals")
	bb_boxes_IntObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_box,"box")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<47>");
	bool t_=f_value==t_box->f_value;
	return t_;
}
int bb_boxes_IntObject::m_Compare2(bb_boxes_IntObject* t_box){
	DBG_ENTER("IntObject.Compare")
	bb_boxes_IntObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_box,"box")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<51>");
	int t_=f_value-t_box->f_value;
	return t_;
}
bb_boxes_IntObject* bb_boxes_IntObject::g_new3(){
	DBG_ENTER("IntObject.new")
	bb_boxes_IntObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<23>");
	return this;
}
void bb_boxes_IntObject::mark(){
	Object::mark();
}
String bb_boxes_IntObject::debug(){
	String t="(IntObject)\n";
	t+=dbg_decl("value",&f_value);
	return t;
}
bb_boxes_FloatObject::bb_boxes_FloatObject(){
	f_value=FLOAT(.0);
}
bb_boxes_FloatObject* bb_boxes_FloatObject::g_new(int t_value){
	DBG_ENTER("FloatObject.new")
	bb_boxes_FloatObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<59>");
	this->f_value=Float(t_value);
	return this;
}
bb_boxes_FloatObject* bb_boxes_FloatObject::g_new2(Float t_value){
	DBG_ENTER("FloatObject.new")
	bb_boxes_FloatObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<63>");
	this->f_value=t_value;
	return this;
}
int bb_boxes_FloatObject::m_ToInt(){
	DBG_ENTER("FloatObject.ToInt")
	bb_boxes_FloatObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<67>");
	int t_=int(f_value);
	return t_;
}
Float bb_boxes_FloatObject::m_ToFloat(){
	DBG_ENTER("FloatObject.ToFloat")
	bb_boxes_FloatObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<71>");
	return f_value;
}
String bb_boxes_FloatObject::m_ToString2(){
	DBG_ENTER("FloatObject.ToString")
	bb_boxes_FloatObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<75>");
	String t_=String(f_value);
	return t_;
}
bool bb_boxes_FloatObject::m_Equals3(bb_boxes_FloatObject* t_box){
	DBG_ENTER("FloatObject.Equals")
	bb_boxes_FloatObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_box,"box")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<79>");
	bool t_=f_value==t_box->f_value;
	return t_;
}
int bb_boxes_FloatObject::m_Compare3(bb_boxes_FloatObject* t_box){
	DBG_ENTER("FloatObject.Compare")
	bb_boxes_FloatObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_box,"box")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<83>");
	if(f_value<t_box->f_value){
		DBG_BLOCK();
		return -1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<84>");
	int t_=((f_value>t_box->f_value)?1:0);
	return t_;
}
bb_boxes_FloatObject* bb_boxes_FloatObject::g_new3(){
	DBG_ENTER("FloatObject.new")
	bb_boxes_FloatObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<55>");
	return this;
}
void bb_boxes_FloatObject::mark(){
	Object::mark();
}
String bb_boxes_FloatObject::debug(){
	String t="(FloatObject)\n";
	t+=dbg_decl("value",&f_value);
	return t;
}
bb_boxes_StringObject::bb_boxes_StringObject(){
	f_value=String();
}
bb_boxes_StringObject* bb_boxes_StringObject::g_new(int t_value){
	DBG_ENTER("StringObject.new")
	bb_boxes_StringObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<92>");
	this->f_value=String(t_value);
	return this;
}
bb_boxes_StringObject* bb_boxes_StringObject::g_new2(Float t_value){
	DBG_ENTER("StringObject.new")
	bb_boxes_StringObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<96>");
	this->f_value=String(t_value);
	return this;
}
bb_boxes_StringObject* bb_boxes_StringObject::g_new3(String t_value){
	DBG_ENTER("StringObject.new")
	bb_boxes_StringObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<100>");
	this->f_value=t_value;
	return this;
}
String bb_boxes_StringObject::m_ToString2(){
	DBG_ENTER("StringObject.ToString")
	bb_boxes_StringObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<104>");
	return f_value;
}
bool bb_boxes_StringObject::m_Equals4(bb_boxes_StringObject* t_box){
	DBG_ENTER("StringObject.Equals")
	bb_boxes_StringObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_box,"box")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<108>");
	bool t_=f_value==t_box->f_value;
	return t_;
}
int bb_boxes_StringObject::m_Compare4(bb_boxes_StringObject* t_box){
	DBG_ENTER("StringObject.Compare")
	bb_boxes_StringObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_box,"box")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<112>");
	int t_=f_value.Compare(t_box->f_value);
	return t_;
}
bb_boxes_StringObject* bb_boxes_StringObject::g_new4(){
	DBG_ENTER("StringObject.new")
	bb_boxes_StringObject *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<88>");
	return this;
}
void bb_boxes_StringObject::mark(){
	Object::mark();
}
String bb_boxes_StringObject::debug(){
	String t="(StringObject)\n";
	t+=dbg_decl("value",&f_value);
	return t;
}
Object* bb_boxes_BoxBool(bool t_value){
	DBG_ENTER("BoxBool")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<139>");
	Object* t_=((new bb_boxes_BoolObject)->g_new(t_value));
	return t_;
}
Object* bb_boxes_BoxInt(int t_value){
	DBG_ENTER("BoxInt")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<143>");
	Object* t_=((new bb_boxes_IntObject)->g_new(t_value));
	return t_;
}
Object* bb_boxes_BoxFloat(Float t_value){
	DBG_ENTER("BoxFloat")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<147>");
	Object* t_=((new bb_boxes_FloatObject)->g_new2(t_value));
	return t_;
}
Object* bb_boxes_BoxString(String t_value){
	DBG_ENTER("BoxString")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<151>");
	Object* t_=((new bb_boxes_StringObject)->g_new3(t_value));
	return t_;
}
bool bb_boxes_UnboxBool(Object* t_box){
	DBG_ENTER("UnboxBool")
	DBG_LOCAL(t_box,"box")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<155>");
	bool t_=dynamic_cast<bb_boxes_BoolObject*>(t_box)->f_value;
	return t_;
}
int bb_boxes_UnboxInt(Object* t_box){
	DBG_ENTER("UnboxInt")
	DBG_LOCAL(t_box,"box")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<159>");
	int t_=dynamic_cast<bb_boxes_IntObject*>(t_box)->f_value;
	return t_;
}
Float bb_boxes_UnboxFloat(Object* t_box){
	DBG_ENTER("UnboxFloat")
	DBG_LOCAL(t_box,"box")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<163>");
	Float t_=dynamic_cast<bb_boxes_FloatObject*>(t_box)->f_value;
	return t_;
}
String bb_boxes_UnboxString(Object* t_box){
	DBG_ENTER("UnboxString")
	DBG_LOCAL(t_box,"box")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/boxes.monkey<167>");
	String t_=dynamic_cast<bb_boxes_StringObject*>(t_box)->f_value;
	return t_;
}
bb_reflection_R16::bb_reflection_R16(){
}
bb_reflection_R16* bb_reflection_R16::g_new(){
	bb_reflection_ClassInfo::g_new(String(L"monkey.lang.Object",18),1,0,Array<bb_reflection_ClassInfo* >());
	return this;
}
int bb_reflection_R16::m_Init(){
	m_InitR();
	return 0;
}
void bb_reflection_R16::mark(){
	bb_reflection_ClassInfo::mark();
}
String bb_reflection_R16::debug(){
	String t="(R16)\n";
	t=bb_reflection_ClassInfo::debug()+t;
	return t;
}
bb_reflection_R17::bb_reflection_R17(){
}
bb_reflection_R17* bb_reflection_R17::g_new(){
	bb_reflection_ClassInfo::g_new(String(L"monkey.lang.Throwable",21),33,bb_reflection__classes.At(0),Array<bb_reflection_ClassInfo* >());
	return this;
}
int bb_reflection_R17::m_Init(){
	m_InitR();
	return 0;
}
void bb_reflection_R17::mark(){
	bb_reflection_ClassInfo::mark();
}
String bb_reflection_R17::debug(){
	String t="(R17)\n";
	t=bb_reflection_ClassInfo::debug()+t;
	return t;
}
bb_reflection_R18::bb_reflection_R18(){
}
bb_reflection_R18* bb_reflection_R18::g_new(){
	bb_reflection_ClassInfo::g_new(String(L"diddy.exception.DiddyException",30),32,bb_reflection__classes.At(1),Array<bb_reflection_ClassInfo* >());
	return this;
}
int bb_reflection_R18::m_Init(){
	gc_assign(f__fields,Array<bb_reflection_FieldInfo* >(4));
	gc_assign(f__fields.At(0),((new bb_reflection_R19)->g_new()));
	gc_assign(f__fields.At(1),((new bb_reflection_R20)->g_new()));
	gc_assign(f__fields.At(2),((new bb_reflection_R21)->g_new()));
	gc_assign(f__fields.At(3),((new bb_reflection_R22)->g_new()));
	gc_assign(f__methods,Array<bb_reflection_MethodInfo* >(7));
	gc_assign(f__methods.At(0),((new bb_reflection_R23)->g_new()));
	gc_assign(f__methods.At(1),((new bb_reflection_R24)->g_new()));
	gc_assign(f__methods.At(2),((new bb_reflection_R25)->g_new()));
	gc_assign(f__methods.At(3),((new bb_reflection_R26)->g_new()));
	gc_assign(f__methods.At(4),((new bb_reflection_R27)->g_new()));
	gc_assign(f__methods.At(5),((new bb_reflection_R28)->g_new()));
	gc_assign(f__methods.At(6),((new bb_reflection_R30)->g_new()));
	gc_assign(f__ctors,Array<bb_reflection_FunctionInfo* >(1));
	gc_assign(f__ctors.At(0),((new bb_reflection_R29)->g_new()));
	m_InitR();
	return 0;
}
void bb_reflection_R18::mark(){
	bb_reflection_ClassInfo::mark();
}
String bb_reflection_R18::debug(){
	String t="(R18)\n";
	t=bb_reflection_ClassInfo::debug()+t;
	return t;
}
bb_reflection_R31::bb_reflection_R31(){
}
bb_reflection_R31* bb_reflection_R31::g_new(){
	bb_reflection_ClassInfo::g_new(String(L"diddy.exception.AssertException",31),32,bb_reflection__classes.At(2),Array<bb_reflection_ClassInfo* >());
	return this;
}
int bb_reflection_R31::m_Init(){
	gc_assign(f__ctors,Array<bb_reflection_FunctionInfo* >(1));
	gc_assign(f__ctors.At(0),((new bb_reflection_R32)->g_new()));
	m_InitR();
	return 0;
}
void bb_reflection_R31::mark(){
	bb_reflection_ClassInfo::mark();
}
String bb_reflection_R31::debug(){
	String t="(R31)\n";
	t=bb_reflection_ClassInfo::debug()+t;
	return t;
}
bb_reflection_R33::bb_reflection_R33(){
}
bb_reflection_R33* bb_reflection_R33::g_new(){
	bb_reflection_ClassInfo::g_new(String(L"diddy.exception.ConcurrentModificationException",47),32,bb_reflection__classes.At(2),Array<bb_reflection_ClassInfo* >());
	return this;
}
int bb_reflection_R33::m_Init(){
	gc_assign(f__ctors,Array<bb_reflection_FunctionInfo* >(1));
	gc_assign(f__ctors.At(0),((new bb_reflection_R34)->g_new()));
	m_InitR();
	return 0;
}
void bb_reflection_R33::mark(){
	bb_reflection_ClassInfo::mark();
}
String bb_reflection_R33::debug(){
	String t="(R33)\n";
	t=bb_reflection_ClassInfo::debug()+t;
	return t;
}
bb_reflection_R35::bb_reflection_R35(){
}
bb_reflection_R35* bb_reflection_R35::g_new(){
	bb_reflection_ClassInfo::g_new(String(L"diddy.exception.IndexOutOfBoundsException",41),32,bb_reflection__classes.At(2),Array<bb_reflection_ClassInfo* >());
	return this;
}
int bb_reflection_R35::m_Init(){
	gc_assign(f__ctors,Array<bb_reflection_FunctionInfo* >(1));
	gc_assign(f__ctors.At(0),((new bb_reflection_R36)->g_new()));
	m_InitR();
	return 0;
}
void bb_reflection_R35::mark(){
	bb_reflection_ClassInfo::mark();
}
String bb_reflection_R35::debug(){
	String t="(R35)\n";
	t=bb_reflection_ClassInfo::debug()+t;
	return t;
}
bb_reflection_R37::bb_reflection_R37(){
}
bb_reflection_R37* bb_reflection_R37::g_new(){
	bb_reflection_ClassInfo::g_new(String(L"diddy.exception.IllegalArgumentException",40),32,bb_reflection__classes.At(2),Array<bb_reflection_ClassInfo* >());
	return this;
}
int bb_reflection_R37::m_Init(){
	gc_assign(f__ctors,Array<bb_reflection_FunctionInfo* >(1));
	gc_assign(f__ctors.At(0),((new bb_reflection_R38)->g_new()));
	m_InitR();
	return 0;
}
void bb_reflection_R37::mark(){
	bb_reflection_ClassInfo::mark();
}
String bb_reflection_R37::debug(){
	String t="(R37)\n";
	t=bb_reflection_ClassInfo::debug()+t;
	return t;
}
bb_reflection_R39::bb_reflection_R39(){
}
bb_reflection_R39* bb_reflection_R39::g_new(){
	bb_reflection_ClassInfo::g_new(String(L"diddy.exception.XMLParseException",33),32,bb_reflection__classes.At(2),Array<bb_reflection_ClassInfo* >());
	return this;
}
int bb_reflection_R39::m_Init(){
	gc_assign(f__ctors,Array<bb_reflection_FunctionInfo* >(1));
	gc_assign(f__ctors.At(0),((new bb_reflection_R40)->g_new()));
	m_InitR();
	return 0;
}
void bb_reflection_R39::mark(){
	bb_reflection_ClassInfo::mark();
}
String bb_reflection_R39::debug(){
	String t="(R39)\n";
	t=bb_reflection_ClassInfo::debug()+t;
	return t;
}
bb_reflection_R41::bb_reflection_R41(){
}
bb_reflection_R41* bb_reflection_R41::g_new(){
	bb_reflection_ClassInfo::g_new(String(L"monkey.boxes.BoolObject",23),0,bb_reflection__classes.At(0),Array<bb_reflection_ClassInfo* >());
	gc_assign(bb_reflection__boolClass,(this));
	return this;
}
int bb_reflection_R41::m_Init(){
	gc_assign(f__fields,Array<bb_reflection_FieldInfo* >(1));
	gc_assign(f__fields.At(0),((new bb_reflection_R42)->g_new()));
	gc_assign(f__methods,Array<bb_reflection_MethodInfo* >(2));
	gc_assign(f__methods.At(0),((new bb_reflection_R44)->g_new()));
	gc_assign(f__methods.At(1),((new bb_reflection_R45)->g_new()));
	gc_assign(f__ctors,Array<bb_reflection_FunctionInfo* >(2));
	gc_assign(f__ctors.At(0),((new bb_reflection_R43)->g_new()));
	gc_assign(f__ctors.At(1),((new bb_reflection_R46)->g_new()));
	m_InitR();
	return 0;
}
void bb_reflection_R41::mark(){
	bb_reflection_ClassInfo::mark();
}
String bb_reflection_R41::debug(){
	String t="(R41)\n";
	t=bb_reflection_ClassInfo::debug()+t;
	return t;
}
bb_reflection_ClassInfo* bb_reflection__boolClass;
bb_reflection_R47::bb_reflection_R47(){
}
bb_reflection_R47* bb_reflection_R47::g_new(){
	bb_reflection_ClassInfo::g_new(String(L"monkey.boxes.IntObject",22),0,bb_reflection__classes.At(0),Array<bb_reflection_ClassInfo* >());
	gc_assign(bb_reflection__intClass,(this));
	return this;
}
int bb_reflection_R47::m_Init(){
	gc_assign(f__fields,Array<bb_reflection_FieldInfo* >(1));
	gc_assign(f__fields.At(0),((new bb_reflection_R48)->g_new()));
	gc_assign(f__methods,Array<bb_reflection_MethodInfo* >(5));
	gc_assign(f__methods.At(0),((new bb_reflection_R51)->g_new()));
	gc_assign(f__methods.At(1),((new bb_reflection_R52)->g_new()));
	gc_assign(f__methods.At(2),((new bb_reflection_R53)->g_new()));
	gc_assign(f__methods.At(3),((new bb_reflection_R54)->g_new()));
	gc_assign(f__methods.At(4),((new bb_reflection_R55)->g_new()));
	gc_assign(f__ctors,Array<bb_reflection_FunctionInfo* >(3));
	gc_assign(f__ctors.At(0),((new bb_reflection_R49)->g_new()));
	gc_assign(f__ctors.At(1),((new bb_reflection_R50)->g_new()));
	gc_assign(f__ctors.At(2),((new bb_reflection_R56)->g_new()));
	m_InitR();
	return 0;
}
void bb_reflection_R47::mark(){
	bb_reflection_ClassInfo::mark();
}
String bb_reflection_R47::debug(){
	String t="(R47)\n";
	t=bb_reflection_ClassInfo::debug()+t;
	return t;
}
bb_reflection_ClassInfo* bb_reflection__intClass;
bb_reflection_R57::bb_reflection_R57(){
}
bb_reflection_R57* bb_reflection_R57::g_new(){
	bb_reflection_ClassInfo::g_new(String(L"monkey.boxes.FloatObject",24),0,bb_reflection__classes.At(0),Array<bb_reflection_ClassInfo* >());
	gc_assign(bb_reflection__floatClass,(this));
	return this;
}
int bb_reflection_R57::m_Init(){
	gc_assign(f__fields,Array<bb_reflection_FieldInfo* >(1));
	gc_assign(f__fields.At(0),((new bb_reflection_R58)->g_new()));
	gc_assign(f__methods,Array<bb_reflection_MethodInfo* >(5));
	gc_assign(f__methods.At(0),((new bb_reflection_R61)->g_new()));
	gc_assign(f__methods.At(1),((new bb_reflection_R62)->g_new()));
	gc_assign(f__methods.At(2),((new bb_reflection_R63)->g_new()));
	gc_assign(f__methods.At(3),((new bb_reflection_R64)->g_new()));
	gc_assign(f__methods.At(4),((new bb_reflection_R65)->g_new()));
	gc_assign(f__ctors,Array<bb_reflection_FunctionInfo* >(3));
	gc_assign(f__ctors.At(0),((new bb_reflection_R59)->g_new()));
	gc_assign(f__ctors.At(1),((new bb_reflection_R60)->g_new()));
	gc_assign(f__ctors.At(2),((new bb_reflection_R66)->g_new()));
	m_InitR();
	return 0;
}
void bb_reflection_R57::mark(){
	bb_reflection_ClassInfo::mark();
}
String bb_reflection_R57::debug(){
	String t="(R57)\n";
	t=bb_reflection_ClassInfo::debug()+t;
	return t;
}
bb_reflection_ClassInfo* bb_reflection__floatClass;
bb_reflection_R67::bb_reflection_R67(){
}
bb_reflection_R67* bb_reflection_R67::g_new(){
	bb_reflection_ClassInfo::g_new(String(L"monkey.boxes.StringObject",25),0,bb_reflection__classes.At(0),Array<bb_reflection_ClassInfo* >());
	gc_assign(bb_reflection__stringClass,(this));
	return this;
}
int bb_reflection_R67::m_Init(){
	gc_assign(f__fields,Array<bb_reflection_FieldInfo* >(1));
	gc_assign(f__fields.At(0),((new bb_reflection_R68)->g_new()));
	gc_assign(f__methods,Array<bb_reflection_MethodInfo* >(3));
	gc_assign(f__methods.At(0),((new bb_reflection_R72)->g_new()));
	gc_assign(f__methods.At(1),((new bb_reflection_R73)->g_new()));
	gc_assign(f__methods.At(2),((new bb_reflection_R74)->g_new()));
	gc_assign(f__ctors,Array<bb_reflection_FunctionInfo* >(4));
	gc_assign(f__ctors.At(0),((new bb_reflection_R69)->g_new()));
	gc_assign(f__ctors.At(1),((new bb_reflection_R70)->g_new()));
	gc_assign(f__ctors.At(2),((new bb_reflection_R71)->g_new()));
	gc_assign(f__ctors.At(3),((new bb_reflection_R75)->g_new()));
	m_InitR();
	return 0;
}
void bb_reflection_R67::mark(){
	bb_reflection_ClassInfo::mark();
}
String bb_reflection_R67::debug(){
	String t="(R67)\n";
	t=bb_reflection_ClassInfo::debug()+t;
	return t;
}
bb_reflection_ClassInfo* bb_reflection__stringClass;
bb_reflection_FunctionInfo::bb_reflection_FunctionInfo(){
	f__name=String();
	f__attrs=0;
	f__retType=0;
	f__argTypes=Array<bb_reflection_ClassInfo* >();
}
bb_reflection_FunctionInfo* bb_reflection_FunctionInfo::g_new(String t_name,int t_attrs,bb_reflection_ClassInfo* t_retType,Array<bb_reflection_ClassInfo* > t_argTypes){
	DBG_ENTER("FunctionInfo.new")
	bb_reflection_FunctionInfo *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_name,"name")
	DBG_LOCAL(t_attrs,"attrs")
	DBG_LOCAL(t_retType,"retType")
	DBG_LOCAL(t_argTypes,"argTypes")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<179>");
	f__name=t_name;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<180>");
	f__attrs=t_attrs;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<181>");
	gc_assign(f__retType,t_retType);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<182>");
	gc_assign(f__argTypes,t_argTypes);
	return this;
}
bb_reflection_FunctionInfo* bb_reflection_FunctionInfo::g_new2(){
	DBG_ENTER("FunctionInfo.new")
	bb_reflection_FunctionInfo *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<176>");
	return this;
}
void bb_reflection_FunctionInfo::mark(){
	Object::mark();
	gc_mark_q(f__retType);
	gc_mark_q(f__argTypes);
}
String bb_reflection_FunctionInfo::debug(){
	String t="(FunctionInfo)\n";
	t+=dbg_decl("_name",&f__name);
	t+=dbg_decl("_attrs",&f__attrs);
	t+=dbg_decl("_retType",&f__retType);
	t+=dbg_decl("_argTypes",&f__argTypes);
	return t;
}
Array<bb_reflection_FunctionInfo* > bb_reflection__functions;
bb_reflection_R4::bb_reflection_R4(){
}
bb_reflection_R4* bb_reflection_R4::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__boolClass};
	bb_reflection_FunctionInfo::g_new(String(L"monkey.boxes.BoxBool",20),0,bb_reflection__classes.At(0),Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R4::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R4::debug(){
	String t="(R4)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R5::bb_reflection_R5(){
}
bb_reflection_R5* bb_reflection_R5::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__intClass};
	bb_reflection_FunctionInfo::g_new(String(L"monkey.boxes.BoxInt",19),0,bb_reflection__classes.At(0),Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R5::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R5::debug(){
	String t="(R5)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R6::bb_reflection_R6(){
}
bb_reflection_R6* bb_reflection_R6::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__floatClass};
	bb_reflection_FunctionInfo::g_new(String(L"monkey.boxes.BoxFloat",21),0,bb_reflection__classes.At(0),Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R6::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R6::debug(){
	String t="(R6)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R7::bb_reflection_R7(){
}
bb_reflection_R7* bb_reflection_R7::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__stringClass};
	bb_reflection_FunctionInfo::g_new(String(L"monkey.boxes.BoxString",22),0,bb_reflection__classes.At(0),Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R7::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R7::debug(){
	String t="(R7)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R8::bb_reflection_R8(){
}
bb_reflection_R8* bb_reflection_R8::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__classes.At(0)};
	bb_reflection_FunctionInfo::g_new(String(L"monkey.boxes.UnboxBool",22),0,bb_reflection__boolClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R8::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R8::debug(){
	String t="(R8)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R9::bb_reflection_R9(){
}
bb_reflection_R9* bb_reflection_R9::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__classes.At(0)};
	bb_reflection_FunctionInfo::g_new(String(L"monkey.boxes.UnboxInt",21),0,bb_reflection__intClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R9::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R9::debug(){
	String t="(R9)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R10::bb_reflection_R10(){
}
bb_reflection_R10* bb_reflection_R10::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__classes.At(0)};
	bb_reflection_FunctionInfo::g_new(String(L"monkey.boxes.UnboxFloat",23),0,bb_reflection__floatClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R10::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R10::debug(){
	String t="(R10)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R11::bb_reflection_R11(){
}
bb_reflection_R11* bb_reflection_R11::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__classes.At(0)};
	bb_reflection_FunctionInfo::g_new(String(L"monkey.boxes.UnboxString",24),0,bb_reflection__stringClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R11::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R11::debug(){
	String t="(R11)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R12::bb_reflection_R12(){
}
bb_reflection_R12* bb_reflection_R12::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__stringClass};
	bb_reflection_FunctionInfo::g_new(String(L"monkey.lang.Print",17),1,bb_reflection__intClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R12::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R12::debug(){
	String t="(R12)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R13::bb_reflection_R13(){
}
bb_reflection_R13* bb_reflection_R13::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__stringClass};
	bb_reflection_FunctionInfo::g_new(String(L"monkey.lang.Error",17),1,bb_reflection__intClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R13::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R13::debug(){
	String t="(R13)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R14::bb_reflection_R14(){
}
bb_reflection_R14* bb_reflection_R14::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__stringClass};
	bb_reflection_FunctionInfo::g_new(String(L"monkey.lang.DebugLog",20),1,bb_reflection__intClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R14::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R14::debug(){
	String t="(R14)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R15::bb_reflection_R15(){
}
bb_reflection_R15* bb_reflection_R15::g_new(){
	bb_reflection_FunctionInfo::g_new(String(L"monkey.lang.DebugStop",21),1,bb_reflection__intClass,Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R15::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R15::debug(){
	String t="(R15)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection___GetClass::bb_reflection___GetClass(){
}
bb_reflection___GetClass* bb_reflection___GetClass::g_new(){
	DBG_ENTER("__GetClass.new")
	bb_reflection___GetClass *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("$SOURCE<745>");
	bb_reflection__GetClass::g_new();
	return this;
}
bb_reflection_ClassInfo* bb_reflection___GetClass::m_GetClass(Object* t_o){
	if(dynamic_cast<bb_boxes_StringObject*>(t_o)!=0){
		return bb_reflection__classes.At(11);
	}
	if(dynamic_cast<bb_boxes_FloatObject*>(t_o)!=0){
		return bb_reflection__classes.At(10);
	}
	if(dynamic_cast<bb_boxes_IntObject*>(t_o)!=0){
		return bb_reflection__classes.At(9);
	}
	if(dynamic_cast<bb_boxes_BoolObject*>(t_o)!=0){
		return bb_reflection__classes.At(8);
	}
	if(dynamic_cast<bb_exception_XMLParseException*>(t_o)!=0){
		return bb_reflection__classes.At(7);
	}
	if(dynamic_cast<bb_exception_IllegalArgumentException*>(t_o)!=0){
		return bb_reflection__classes.At(6);
	}
	if(dynamic_cast<bb_exception_IndexOutOfBoundsException*>(t_o)!=0){
		return bb_reflection__classes.At(5);
	}
	if(dynamic_cast<bb_exception_ConcurrentModificationException*>(t_o)!=0){
		return bb_reflection__classes.At(4);
	}
	if(dynamic_cast<bb_exception_AssertException*>(t_o)!=0){
		return bb_reflection__classes.At(3);
	}
	if(dynamic_cast<bb_exception_DiddyException*>(t_o)!=0){
		return bb_reflection__classes.At(2);
	}
	if(dynamic_cast<ThrowableObject*>(t_o)!=0){
		return bb_reflection__classes.At(1);
	}
	if(t_o!=0){
		return bb_reflection__classes.At(0);
	}
	return bb_reflection__unknownClass;
}
void bb_reflection___GetClass::mark(){
	bb_reflection__GetClass::mark();
}
String bb_reflection___GetClass::debug(){
	String t="(__GetClass)\n";
	t=bb_reflection__GetClass::debug()+t;
	return t;
}
int bb_reflection___init(){
	gc_assign(bb_reflection__classes,Array<bb_reflection_ClassInfo* >(12));
	gc_assign(bb_reflection__classes.At(0),((new bb_reflection_R16)->g_new()));
	gc_assign(bb_reflection__classes.At(1),((new bb_reflection_R17)->g_new()));
	gc_assign(bb_reflection__classes.At(2),((new bb_reflection_R18)->g_new()));
	gc_assign(bb_reflection__classes.At(3),((new bb_reflection_R31)->g_new()));
	gc_assign(bb_reflection__classes.At(4),((new bb_reflection_R33)->g_new()));
	gc_assign(bb_reflection__classes.At(5),((new bb_reflection_R35)->g_new()));
	gc_assign(bb_reflection__classes.At(6),((new bb_reflection_R37)->g_new()));
	gc_assign(bb_reflection__classes.At(7),((new bb_reflection_R39)->g_new()));
	gc_assign(bb_reflection__classes.At(8),((new bb_reflection_R41)->g_new()));
	gc_assign(bb_reflection__classes.At(9),((new bb_reflection_R47)->g_new()));
	gc_assign(bb_reflection__classes.At(10),((new bb_reflection_R57)->g_new()));
	gc_assign(bb_reflection__classes.At(11),((new bb_reflection_R67)->g_new()));
	bb_reflection__classes.At(0)->m_Init();
	bb_reflection__classes.At(1)->m_Init();
	bb_reflection__classes.At(2)->m_Init();
	bb_reflection__classes.At(3)->m_Init();
	bb_reflection__classes.At(4)->m_Init();
	bb_reflection__classes.At(5)->m_Init();
	bb_reflection__classes.At(6)->m_Init();
	bb_reflection__classes.At(7)->m_Init();
	bb_reflection__classes.At(8)->m_Init();
	bb_reflection__classes.At(9)->m_Init();
	bb_reflection__classes.At(10)->m_Init();
	bb_reflection__classes.At(11)->m_Init();
	gc_assign(bb_reflection__functions,Array<bb_reflection_FunctionInfo* >(12));
	gc_assign(bb_reflection__functions.At(0),((new bb_reflection_R4)->g_new()));
	gc_assign(bb_reflection__functions.At(1),((new bb_reflection_R5)->g_new()));
	gc_assign(bb_reflection__functions.At(2),((new bb_reflection_R6)->g_new()));
	gc_assign(bb_reflection__functions.At(3),((new bb_reflection_R7)->g_new()));
	gc_assign(bb_reflection__functions.At(4),((new bb_reflection_R8)->g_new()));
	gc_assign(bb_reflection__functions.At(5),((new bb_reflection_R9)->g_new()));
	gc_assign(bb_reflection__functions.At(6),((new bb_reflection_R10)->g_new()));
	gc_assign(bb_reflection__functions.At(7),((new bb_reflection_R11)->g_new()));
	gc_assign(bb_reflection__functions.At(8),((new bb_reflection_R12)->g_new()));
	gc_assign(bb_reflection__functions.At(9),((new bb_reflection_R13)->g_new()));
	gc_assign(bb_reflection__functions.At(10),((new bb_reflection_R14)->g_new()));
	gc_assign(bb_reflection__functions.At(11),((new bb_reflection_R15)->g_new()));
	gc_assign(bb_reflection__getClass,((new bb_reflection___GetClass)->g_new()));
	return 0;
}
int bb_reflection__init;
bb_app_App::bb_app_App(){
}
bb_app_App* bb_app_App::g_new(){
	DBG_ENTER("App.new")
	bb_app_App *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<109>");
	gc_assign(bb_app_device,(new bb_app_AppDevice)->g_new(this));
	return this;
}
int bb_app_App::m_OnCreate(){
	DBG_ENTER("App.OnCreate")
	bb_app_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int bb_app_App::m_OnUpdate(){
	DBG_ENTER("App.OnUpdate")
	bb_app_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int bb_app_App::m_OnSuspend(){
	DBG_ENTER("App.OnSuspend")
	bb_app_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int bb_app_App::m_OnResume(){
	DBG_ENTER("App.OnResume")
	bb_app_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int bb_app_App::m_OnRender(){
	DBG_ENTER("App.OnRender")
	bb_app_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
int bb_app_App::m_OnLoading(){
	DBG_ENTER("App.OnLoading")
	bb_app_App *self=this;
	DBG_LOCAL(self,"Self")
	return 0;
}
void bb_app_App::mark(){
	Object::mark();
}
String bb_app_App::debug(){
	String t="(App)\n";
	return t;
}
bb_bapp_BApp::bb_bapp_BApp(){
	f_Games=0;
	f_BeaconList=0;
	f_Title=0;
	f_ServerLabel=0;
	f_PwLabel=0;
	f_Pw=0;
	f_On_Off=0;
	f_isOn=false;
	f_LastGame=String();
}
bb_bapp_BApp* bb_bapp_BApp::g_new(){
	DBG_ENTER("BApp.new")
	bb_bapp_BApp *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/bapp.monkey<3>");
	bb_app_App::g_new();
	return this;
}
void bb_bapp_BApp::mark(){
	bb_app_App::mark();
	gc_mark_q(f_Games);
	gc_mark_q(f_BeaconList);
	gc_mark_q(f_Title);
	gc_mark_q(f_ServerLabel);
	gc_mark_q(f_PwLabel);
	gc_mark_q(f_Pw);
	gc_mark_q(f_On_Off);
}
String bb_bapp_BApp::debug(){
	String t="(BApp)\n";
	t=bb_app_App::debug()+t;
	t+=dbg_decl("Title",&f_Title);
	t+=dbg_decl("ServerLabel",&f_ServerLabel);
	t+=dbg_decl("LastGame",&f_LastGame);
	t+=dbg_decl("Games",&f_Games);
	t+=dbg_decl("PwLabel",&f_PwLabel);
	t+=dbg_decl("Pw",&f_Pw);
	t+=dbg_decl("BeaconList",&f_BeaconList);
	t+=dbg_decl("On_Off",&f_On_Off);
	t+=dbg_decl("isOn",&f_isOn);
	return t;
}
bb_Beacon_Beacon::bb_Beacon_Beacon(){
}
bb_Beacon_Beacon* bb_Beacon_Beacon::g_new(){
	DBG_ENTER("Beacon.new")
	bb_Beacon_Beacon *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<3>");
	bb_bapp_BApp::g_new();
	return this;
}
int bb_Beacon_Beacon::m_OnCreate(){
	DBG_ENTER("Beacon.OnCreate")
	bb_Beacon_Beacon *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<6>");
	bb_challengergui_CHGUI_MobileMode=1;
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<7>");
	bb_app_SetUpdateRate(30);
	return 0;
}
int bb_Beacon_Beacon::m_OnRender(){
	DBG_ENTER("Beacon.OnRender")
	bb_Beacon_Beacon *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<11>");
	String t_=bb_data2_STATUS;
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<12>");
	if(t_==String(L"connecting",10)){
		DBG_BLOCK();
		DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<13>");
		bb_protocol_RequestGameList();
		DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<14>");
		bb_data2_STATUS=String(L"normal",6);
	}else{
		DBG_BLOCK();
		DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<15>");
		if(t_==String(L"normal",6)){
			DBG_BLOCK();
			DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<16>");
			bb_protocol_ReadProtocol();
			DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<18>");
			bb_graphics_Cls(FLOAT(247.0),FLOAT(247.0),FLOAT(247.0));
			DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<20>");
			bb_challengergui_CHGUI_Draw();
		}else{
			DBG_BLOCK();
			DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<21>");
			if(t_==String(L"start",5)){
				DBG_BLOCK();
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<22>");
				bb_challengergui_CHGUI_Start();
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<24>");
				gc_assign(f_Title,bb_data2_CScale(bb_challengergui_CreateLabel(50,10,String(L"Beacon Config",13),0)));
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<25>");
				gc_assign(f_ServerLabel,bb_data2_CScale(bb_challengergui_CreateLabel(5,60,String(L"Server Type: Static",19),0)));
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<26>");
				gc_assign(f_Games,bb_data2_CScale(bb_challengergui_CreateDropdown(10,110,int(bb_data2_SCALE_W-FLOAT(20.0)),40,String(L"Choose Game",11),0)));
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<27>");
				gc_assign(f_PwLabel,bb_data2_CScale(bb_challengergui_CreateLabel(5,160,String(L"Password:",9),0)));
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<28>");
				gc_assign(f_Pw,bb_data2_CScale(bb_challengergui_CreateTextfield(120,155,170,45,String(),0)));
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<29>");
				gc_assign(f_BeaconList,bb_data2_CScale(bb_challengergui_CreateDropdown(10,210,int(bb_data2_SCALE_W-FLOAT(20.0)),40,String(L"Choose Beacon",13),0)));
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<30>");
				gc_assign(f_On_Off,bb_data2_CScale(bb_challengergui_CreateButton(10,int(bb_data2_SCALE_H-FLOAT(50.0)),int(bb_data2_SCALE_W-FLOAT(20.0)),40,String(L"On/Off",6),0)));
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<32>");
				f_isOn=false;
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<33>");
				gc_assign(bb_data2_Game,(this));
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<34>");
				f_LastGame=String(L"Choose Game",11);
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<35>");
				bb_data2_STATUS=String(L"connecting",10);
			}
		}
	}
	return 0;
}
int bb_Beacon_Beacon::m_OnUpdate(){
	DBG_ENTER("Beacon.OnUpdate")
	bb_Beacon_Beacon *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<40>");
	String t_=bb_data2_STATUS;
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<41>");
	if(t_==String(L"connecting",10)){
		DBG_BLOCK();
		DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<42>");
		bb_challengergui_CHGUI_Update();
	}else{
		DBG_BLOCK();
		DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<43>");
		if(t_==String(L"normal",6)){
			DBG_BLOCK();
			DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<44>");
			bb_challengergui_CHGUI_Update();
			DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<45>");
			if(f_LastGame!=f_Games->f_Text){
				DBG_BLOCK();
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<46>");
				bb_challengergui_CHGUI_Delete(f_BeaconList);
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<47>");
				gc_assign(f_BeaconList,bb_data2_CScale(bb_challengergui_CreateDropdown(10,210,int(bb_data2_SCALE_W-FLOAT(20.0)),40,String(L"Choose Beacon",13),0)));
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<48>");
				f_LastGame=f_Games->f_Text;
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<49>");
				bb_protocol_RequestBeaconList(f_LastGame);
			}
		}
	}
	return 0;
}
void bb_Beacon_Beacon::mark(){
	bb_bapp_BApp::mark();
}
String bb_Beacon_Beacon::debug(){
	String t="(Beacon)\n";
	t=bb_bapp_BApp::debug()+t;
	return t;
}
bb_app_AppDevice::bb_app_AppDevice(){
	f_app=0;
	f_updateRate=0;
}
bb_app_AppDevice* bb_app_AppDevice::g_new(bb_app_App* t_app){
	DBG_ENTER("AppDevice.new")
	bb_app_AppDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_app,"app")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<49>");
	gc_assign(this->f_app,t_app);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<50>");
	bb_graphics_SetGraphicsDevice(GraphicsDevice());
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<51>");
	bb_input_SetInputDevice(InputDevice());
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<52>");
	bb_audio_SetAudioDevice(AudioDevice());
	return this;
}
bb_app_AppDevice* bb_app_AppDevice::g_new2(){
	DBG_ENTER("AppDevice.new")
	bb_app_AppDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<46>");
	return this;
}
int bb_app_AppDevice::OnCreate(){
	DBG_ENTER("AppDevice.OnCreate")
	bb_app_AppDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<56>");
	bb_graphics_SetFont(0,32);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<57>");
	int t_=f_app->m_OnCreate();
	return t_;
}
int bb_app_AppDevice::OnUpdate(){
	DBG_ENTER("AppDevice.OnUpdate")
	bb_app_AppDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<61>");
	int t_=f_app->m_OnUpdate();
	return t_;
}
int bb_app_AppDevice::OnSuspend(){
	DBG_ENTER("AppDevice.OnSuspend")
	bb_app_AppDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<65>");
	int t_=f_app->m_OnSuspend();
	return t_;
}
int bb_app_AppDevice::OnResume(){
	DBG_ENTER("AppDevice.OnResume")
	bb_app_AppDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<69>");
	int t_=f_app->m_OnResume();
	return t_;
}
int bb_app_AppDevice::OnRender(){
	DBG_ENTER("AppDevice.OnRender")
	bb_app_AppDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<73>");
	bb_graphics_BeginRender();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<74>");
	int t_r=f_app->m_OnRender();
	DBG_LOCAL(t_r,"r")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<75>");
	bb_graphics_EndRender();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<76>");
	return t_r;
}
int bb_app_AppDevice::OnLoading(){
	DBG_ENTER("AppDevice.OnLoading")
	bb_app_AppDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<80>");
	bb_graphics_BeginRender();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<81>");
	int t_r=f_app->m_OnLoading();
	DBG_LOCAL(t_r,"r")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<82>");
	bb_graphics_EndRender();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<83>");
	return t_r;
}
int bb_app_AppDevice::SetUpdateRate(int t_hertz){
	DBG_ENTER("AppDevice.SetUpdateRate")
	bb_app_AppDevice *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_hertz,"hertz")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<87>");
	gxtkApp::SetUpdateRate(t_hertz);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<88>");
	f_updateRate=t_hertz;
	return 0;
}
void bb_app_AppDevice::mark(){
	gxtkApp::mark();
	gc_mark_q(f_app);
}
String bb_app_AppDevice::debug(){
	String t="(AppDevice)\n";
	t+=dbg_decl("app",&f_app);
	t+=dbg_decl("updateRate",&f_updateRate);
	return t;
}
gxtkGraphics* bb_graphics_device;
int bb_graphics_SetGraphicsDevice(gxtkGraphics* t_dev){
	DBG_ENTER("SetGraphicsDevice")
	DBG_LOCAL(t_dev,"dev")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<58>");
	gc_assign(bb_graphics_device,t_dev);
	return 0;
}
gxtkInput* bb_input_device;
int bb_input_SetInputDevice(gxtkInput* t_dev){
	DBG_ENTER("SetInputDevice")
	DBG_LOCAL(t_dev,"dev")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<16>");
	gc_assign(bb_input_device,t_dev);
	return 0;
}
gxtkAudio* bb_audio_device;
int bb_audio_SetAudioDevice(gxtkAudio* t_dev){
	DBG_ENTER("SetAudioDevice")
	DBG_LOCAL(t_dev,"dev")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/audio.monkey<17>");
	gc_assign(bb_audio_device,t_dev);
	return 0;
}
bb_app_AppDevice* bb_app_device;
int bbMain(){
	DBG_ENTER("Main")
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/Beacon.monkey<57>");
	(new bb_Beacon_Beacon)->g_new();
	return 0;
}
bb_reflection_ConstInfo::bb_reflection_ConstInfo(){
}
void bb_reflection_ConstInfo::mark(){
	Object::mark();
}
String bb_reflection_ConstInfo::debug(){
	String t="(ConstInfo)\n";
	return t;
}
bb_stack_Stack::bb_stack_Stack(){
	f_data=Array<bb_reflection_ConstInfo* >();
	f_length=0;
}
bb_stack_Stack* bb_stack_Stack::g_new(){
	DBG_ENTER("Stack.new")
	bb_stack_Stack *self=this;
	DBG_LOCAL(self,"Self")
	return this;
}
bb_stack_Stack* bb_stack_Stack::g_new2(Array<bb_reflection_ConstInfo* > t_data){
	DBG_ENTER("Stack.new")
	bb_stack_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<13>");
	gc_assign(this->f_data,t_data.Slice(0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<14>");
	this->f_length=t_data.Length();
	return this;
}
int bb_stack_Stack::m_Push(bb_reflection_ConstInfo* t_value){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<52>");
	if(f_length==f_data.Length()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<53>");
		gc_assign(f_data,f_data.Resize(f_length*2+10));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<55>");
	gc_assign(f_data.At(f_length),t_value);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<56>");
	f_length+=1;
	return 0;
}
int bb_stack_Stack::m_Push2(Array<bb_reflection_ConstInfo* > t_values,int t_offset,int t_count){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<66>");
	for(int t_i=0;t_i<t_count;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<67>");
		m_Push(t_values.At(t_offset+t_i));
	}
	return 0;
}
int bb_stack_Stack::m_Push3(Array<bb_reflection_ConstInfo* > t_values,int t_offset){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<60>");
	for(int t_i=t_offset;t_i<t_values.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<61>");
		m_Push(t_values.At(t_i));
	}
	return 0;
}
Array<bb_reflection_ConstInfo* > bb_stack_Stack::m_ToArray(){
	DBG_ENTER("Stack.ToArray")
	bb_stack_Stack *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<18>");
	Array<bb_reflection_ConstInfo* > t_t=Array<bb_reflection_ConstInfo* >(f_length);
	DBG_LOCAL(t_t,"t")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<19>");
	for(int t_i=0;t_i<f_length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<20>");
		gc_assign(t_t.At(t_i),f_data.At(t_i));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<22>");
	return t_t;
}
void bb_stack_Stack::mark(){
	Object::mark();
	gc_mark_q(f_data);
}
String bb_stack_Stack::debug(){
	String t="(Stack)\n";
	t+=dbg_decl("data",&f_data);
	t+=dbg_decl("length",&f_length);
	return t;
}
bb_reflection_FieldInfo::bb_reflection_FieldInfo(){
	f__name=String();
	f__attrs=0;
	f__type=0;
}
bb_reflection_FieldInfo* bb_reflection_FieldInfo::g_new(String t_name,int t_attrs,bb_reflection_ClassInfo* t_type){
	DBG_ENTER("FieldInfo.new")
	bb_reflection_FieldInfo *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_name,"name")
	DBG_LOCAL(t_attrs,"attrs")
	DBG_LOCAL(t_type,"type")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<111>");
	f__name=t_name;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<112>");
	f__attrs=t_attrs;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<113>");
	gc_assign(f__type,t_type);
	return this;
}
bb_reflection_FieldInfo* bb_reflection_FieldInfo::g_new2(){
	DBG_ENTER("FieldInfo.new")
	bb_reflection_FieldInfo *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<108>");
	return this;
}
void bb_reflection_FieldInfo::mark(){
	Object::mark();
	gc_mark_q(f__type);
}
String bb_reflection_FieldInfo::debug(){
	String t="(FieldInfo)\n";
	t+=dbg_decl("_name",&f__name);
	t+=dbg_decl("_attrs",&f__attrs);
	t+=dbg_decl("_type",&f__type);
	return t;
}
bb_stack_Stack2::bb_stack_Stack2(){
	f_data=Array<bb_reflection_FieldInfo* >();
	f_length=0;
}
bb_stack_Stack2* bb_stack_Stack2::g_new(){
	DBG_ENTER("Stack.new")
	bb_stack_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	return this;
}
bb_stack_Stack2* bb_stack_Stack2::g_new2(Array<bb_reflection_FieldInfo* > t_data){
	DBG_ENTER("Stack.new")
	bb_stack_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<13>");
	gc_assign(this->f_data,t_data.Slice(0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<14>");
	this->f_length=t_data.Length();
	return this;
}
int bb_stack_Stack2::m_Push4(bb_reflection_FieldInfo* t_value){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<52>");
	if(f_length==f_data.Length()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<53>");
		gc_assign(f_data,f_data.Resize(f_length*2+10));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<55>");
	gc_assign(f_data.At(f_length),t_value);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<56>");
	f_length+=1;
	return 0;
}
int bb_stack_Stack2::m_Push5(Array<bb_reflection_FieldInfo* > t_values,int t_offset,int t_count){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<66>");
	for(int t_i=0;t_i<t_count;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<67>");
		m_Push4(t_values.At(t_offset+t_i));
	}
	return 0;
}
int bb_stack_Stack2::m_Push6(Array<bb_reflection_FieldInfo* > t_values,int t_offset){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<60>");
	for(int t_i=t_offset;t_i<t_values.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<61>");
		m_Push4(t_values.At(t_i));
	}
	return 0;
}
Array<bb_reflection_FieldInfo* > bb_stack_Stack2::m_ToArray(){
	DBG_ENTER("Stack.ToArray")
	bb_stack_Stack2 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<18>");
	Array<bb_reflection_FieldInfo* > t_t=Array<bb_reflection_FieldInfo* >(f_length);
	DBG_LOCAL(t_t,"t")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<19>");
	for(int t_i=0;t_i<f_length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<20>");
		gc_assign(t_t.At(t_i),f_data.At(t_i));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<22>");
	return t_t;
}
void bb_stack_Stack2::mark(){
	Object::mark();
	gc_mark_q(f_data);
}
String bb_stack_Stack2::debug(){
	String t="(Stack)\n";
	t+=dbg_decl("data",&f_data);
	t+=dbg_decl("length",&f_length);
	return t;
}
bb_reflection_GlobalInfo::bb_reflection_GlobalInfo(){
}
void bb_reflection_GlobalInfo::mark(){
	Object::mark();
}
String bb_reflection_GlobalInfo::debug(){
	String t="(GlobalInfo)\n";
	return t;
}
bb_stack_Stack3::bb_stack_Stack3(){
	f_data=Array<bb_reflection_GlobalInfo* >();
	f_length=0;
}
bb_stack_Stack3* bb_stack_Stack3::g_new(){
	DBG_ENTER("Stack.new")
	bb_stack_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	return this;
}
bb_stack_Stack3* bb_stack_Stack3::g_new2(Array<bb_reflection_GlobalInfo* > t_data){
	DBG_ENTER("Stack.new")
	bb_stack_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<13>");
	gc_assign(this->f_data,t_data.Slice(0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<14>");
	this->f_length=t_data.Length();
	return this;
}
int bb_stack_Stack3::m_Push7(bb_reflection_GlobalInfo* t_value){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<52>");
	if(f_length==f_data.Length()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<53>");
		gc_assign(f_data,f_data.Resize(f_length*2+10));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<55>");
	gc_assign(f_data.At(f_length),t_value);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<56>");
	f_length+=1;
	return 0;
}
int bb_stack_Stack3::m_Push8(Array<bb_reflection_GlobalInfo* > t_values,int t_offset,int t_count){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<66>");
	for(int t_i=0;t_i<t_count;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<67>");
		m_Push7(t_values.At(t_offset+t_i));
	}
	return 0;
}
int bb_stack_Stack3::m_Push9(Array<bb_reflection_GlobalInfo* > t_values,int t_offset){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<60>");
	for(int t_i=t_offset;t_i<t_values.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<61>");
		m_Push7(t_values.At(t_i));
	}
	return 0;
}
Array<bb_reflection_GlobalInfo* > bb_stack_Stack3::m_ToArray(){
	DBG_ENTER("Stack.ToArray")
	bb_stack_Stack3 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<18>");
	Array<bb_reflection_GlobalInfo* > t_t=Array<bb_reflection_GlobalInfo* >(f_length);
	DBG_LOCAL(t_t,"t")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<19>");
	for(int t_i=0;t_i<f_length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<20>");
		gc_assign(t_t.At(t_i),f_data.At(t_i));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<22>");
	return t_t;
}
void bb_stack_Stack3::mark(){
	Object::mark();
	gc_mark_q(f_data);
}
String bb_stack_Stack3::debug(){
	String t="(Stack)\n";
	t+=dbg_decl("data",&f_data);
	t+=dbg_decl("length",&f_length);
	return t;
}
bb_reflection_MethodInfo::bb_reflection_MethodInfo(){
	f__name=String();
	f__attrs=0;
	f__retType=0;
	f__argTypes=Array<bb_reflection_ClassInfo* >();
}
bb_reflection_MethodInfo* bb_reflection_MethodInfo::g_new(String t_name,int t_attrs,bb_reflection_ClassInfo* t_retType,Array<bb_reflection_ClassInfo* > t_argTypes){
	DBG_ENTER("MethodInfo.new")
	bb_reflection_MethodInfo *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_name,"name")
	DBG_LOCAL(t_attrs,"attrs")
	DBG_LOCAL(t_retType,"retType")
	DBG_LOCAL(t_argTypes,"argTypes")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<143>");
	f__name=t_name;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<144>");
	f__attrs=t_attrs;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<145>");
	gc_assign(f__retType,t_retType);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<146>");
	gc_assign(f__argTypes,t_argTypes);
	return this;
}
bb_reflection_MethodInfo* bb_reflection_MethodInfo::g_new2(){
	DBG_ENTER("MethodInfo.new")
	bb_reflection_MethodInfo *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<140>");
	return this;
}
void bb_reflection_MethodInfo::mark(){
	Object::mark();
	gc_mark_q(f__retType);
	gc_mark_q(f__argTypes);
}
String bb_reflection_MethodInfo::debug(){
	String t="(MethodInfo)\n";
	t+=dbg_decl("_name",&f__name);
	t+=dbg_decl("_attrs",&f__attrs);
	t+=dbg_decl("_retType",&f__retType);
	t+=dbg_decl("_argTypes",&f__argTypes);
	return t;
}
bb_stack_Stack4::bb_stack_Stack4(){
	f_data=Array<bb_reflection_MethodInfo* >();
	f_length=0;
}
bb_stack_Stack4* bb_stack_Stack4::g_new(){
	DBG_ENTER("Stack.new")
	bb_stack_Stack4 *self=this;
	DBG_LOCAL(self,"Self")
	return this;
}
bb_stack_Stack4* bb_stack_Stack4::g_new2(Array<bb_reflection_MethodInfo* > t_data){
	DBG_ENTER("Stack.new")
	bb_stack_Stack4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<13>");
	gc_assign(this->f_data,t_data.Slice(0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<14>");
	this->f_length=t_data.Length();
	return this;
}
int bb_stack_Stack4::m_Push10(bb_reflection_MethodInfo* t_value){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<52>");
	if(f_length==f_data.Length()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<53>");
		gc_assign(f_data,f_data.Resize(f_length*2+10));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<55>");
	gc_assign(f_data.At(f_length),t_value);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<56>");
	f_length+=1;
	return 0;
}
int bb_stack_Stack4::m_Push11(Array<bb_reflection_MethodInfo* > t_values,int t_offset,int t_count){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<66>");
	for(int t_i=0;t_i<t_count;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<67>");
		m_Push10(t_values.At(t_offset+t_i));
	}
	return 0;
}
int bb_stack_Stack4::m_Push12(Array<bb_reflection_MethodInfo* > t_values,int t_offset){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<60>");
	for(int t_i=t_offset;t_i<t_values.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<61>");
		m_Push10(t_values.At(t_i));
	}
	return 0;
}
Array<bb_reflection_MethodInfo* > bb_stack_Stack4::m_ToArray(){
	DBG_ENTER("Stack.ToArray")
	bb_stack_Stack4 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<18>");
	Array<bb_reflection_MethodInfo* > t_t=Array<bb_reflection_MethodInfo* >(f_length);
	DBG_LOCAL(t_t,"t")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<19>");
	for(int t_i=0;t_i<f_length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<20>");
		gc_assign(t_t.At(t_i),f_data.At(t_i));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<22>");
	return t_t;
}
void bb_stack_Stack4::mark(){
	Object::mark();
	gc_mark_q(f_data);
}
String bb_stack_Stack4::debug(){
	String t="(Stack)\n";
	t+=dbg_decl("data",&f_data);
	t+=dbg_decl("length",&f_length);
	return t;
}
bb_stack_Stack5::bb_stack_Stack5(){
	f_data=Array<bb_reflection_FunctionInfo* >();
	f_length=0;
}
bb_stack_Stack5* bb_stack_Stack5::g_new(){
	DBG_ENTER("Stack.new")
	bb_stack_Stack5 *self=this;
	DBG_LOCAL(self,"Self")
	return this;
}
bb_stack_Stack5* bb_stack_Stack5::g_new2(Array<bb_reflection_FunctionInfo* > t_data){
	DBG_ENTER("Stack.new")
	bb_stack_Stack5 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<13>");
	gc_assign(this->f_data,t_data.Slice(0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<14>");
	this->f_length=t_data.Length();
	return this;
}
int bb_stack_Stack5::m_Push13(bb_reflection_FunctionInfo* t_value){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack5 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<52>");
	if(f_length==f_data.Length()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<53>");
		gc_assign(f_data,f_data.Resize(f_length*2+10));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<55>");
	gc_assign(f_data.At(f_length),t_value);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<56>");
	f_length+=1;
	return 0;
}
int bb_stack_Stack5::m_Push14(Array<bb_reflection_FunctionInfo* > t_values,int t_offset,int t_count){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack5 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<66>");
	for(int t_i=0;t_i<t_count;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<67>");
		m_Push13(t_values.At(t_offset+t_i));
	}
	return 0;
}
int bb_stack_Stack5::m_Push15(Array<bb_reflection_FunctionInfo* > t_values,int t_offset){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack5 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<60>");
	for(int t_i=t_offset;t_i<t_values.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<61>");
		m_Push13(t_values.At(t_i));
	}
	return 0;
}
Array<bb_reflection_FunctionInfo* > bb_stack_Stack5::m_ToArray(){
	DBG_ENTER("Stack.ToArray")
	bb_stack_Stack5 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<18>");
	Array<bb_reflection_FunctionInfo* > t_t=Array<bb_reflection_FunctionInfo* >(f_length);
	DBG_LOCAL(t_t,"t")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<19>");
	for(int t_i=0;t_i<f_length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<20>");
		gc_assign(t_t.At(t_i),f_data.At(t_i));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<22>");
	return t_t;
}
void bb_stack_Stack5::mark(){
	Object::mark();
	gc_mark_q(f_data);
}
String bb_stack_Stack5::debug(){
	String t="(Stack)\n";
	t+=dbg_decl("data",&f_data);
	t+=dbg_decl("length",&f_length);
	return t;
}
bb_reflection_R19::bb_reflection_R19(){
}
bb_reflection_R19* bb_reflection_R19::g_new(){
	bb_reflection_FieldInfo::g_new(String(L"message",7),2,bb_reflection__stringClass);
	return this;
}
void bb_reflection_R19::mark(){
	bb_reflection_FieldInfo::mark();
}
String bb_reflection_R19::debug(){
	String t="(R19)\n";
	t=bb_reflection_FieldInfo::debug()+t;
	return t;
}
bb_reflection_R20::bb_reflection_R20(){
}
bb_reflection_R20* bb_reflection_R20::g_new(){
	bb_reflection_FieldInfo::g_new(String(L"cause",5),2,bb_reflection__classes.At(1));
	return this;
}
void bb_reflection_R20::mark(){
	bb_reflection_FieldInfo::mark();
}
String bb_reflection_R20::debug(){
	String t="(R20)\n";
	t=bb_reflection_FieldInfo::debug()+t;
	return t;
}
bb_reflection_R21::bb_reflection_R21(){
}
bb_reflection_R21* bb_reflection_R21::g_new(){
	bb_reflection_FieldInfo::g_new(String(L"type",4),2,bb_reflection__stringClass);
	return this;
}
void bb_reflection_R21::mark(){
	bb_reflection_FieldInfo::mark();
}
String bb_reflection_R21::debug(){
	String t="(R21)\n";
	t=bb_reflection_FieldInfo::debug()+t;
	return t;
}
bb_reflection_R22::bb_reflection_R22(){
}
bb_reflection_R22* bb_reflection_R22::g_new(){
	bb_reflection_FieldInfo::g_new(String(L"fullType",8),2,bb_reflection__stringClass);
	return this;
}
void bb_reflection_R22::mark(){
	bb_reflection_FieldInfo::mark();
}
String bb_reflection_R22::debug(){
	String t="(R22)\n";
	t=bb_reflection_FieldInfo::debug()+t;
	return t;
}
bb_reflection_R23::bb_reflection_R23(){
}
bb_reflection_R23* bb_reflection_R23::g_new(){
	bb_reflection_MethodInfo::g_new(String(L"Message",7),8,bb_reflection__stringClass,Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R23::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R23::debug(){
	String t="(R23)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R24::bb_reflection_R24(){
}
bb_reflection_R24* bb_reflection_R24::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__stringClass};
	bb_reflection_MethodInfo::g_new(String(L"Message",7),8,0,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R24::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R24::debug(){
	String t="(R24)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R25::bb_reflection_R25(){
}
bb_reflection_R25* bb_reflection_R25::g_new(){
	bb_reflection_MethodInfo::g_new(String(L"Cause",5),8,bb_reflection__classes.At(1),Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R25::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R25::debug(){
	String t="(R25)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R26::bb_reflection_R26(){
}
bb_reflection_R26* bb_reflection_R26::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__classes.At(1)};
	bb_reflection_MethodInfo::g_new(String(L"Cause",5),8,0,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R26::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R26::debug(){
	String t="(R26)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R27::bb_reflection_R27(){
}
bb_reflection_R27* bb_reflection_R27::g_new(){
	bb_reflection_MethodInfo::g_new(String(L"Type",4),8,bb_reflection__stringClass,Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R27::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R27::debug(){
	String t="(R27)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R28::bb_reflection_R28(){
}
bb_reflection_R28* bb_reflection_R28::g_new(){
	bb_reflection_MethodInfo::g_new(String(L"FullType",8),8,bb_reflection__stringClass,Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R28::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R28::debug(){
	String t="(R28)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R30::bb_reflection_R30(){
}
bb_reflection_R30* bb_reflection_R30::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__boolClass};
	bb_reflection_MethodInfo::g_new(String(L"ToString",8),0,bb_reflection__stringClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R30::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R30::debug(){
	String t="(R30)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R29::bb_reflection_R29(){
}
bb_reflection_R29* bb_reflection_R29::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__stringClass,bb_reflection__classes.At(1)};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(2),Array<bb_reflection_ClassInfo* >(t_,2));
	return this;
}
void bb_reflection_R29::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R29::debug(){
	String t="(R29)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R32::bb_reflection_R32(){
}
bb_reflection_R32* bb_reflection_R32::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__stringClass,bb_reflection__classes.At(1)};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(3),Array<bb_reflection_ClassInfo* >(t_,2));
	return this;
}
void bb_reflection_R32::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R32::debug(){
	String t="(R32)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R34::bb_reflection_R34(){
}
bb_reflection_R34* bb_reflection_R34::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__stringClass,bb_reflection__classes.At(1)};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(4),Array<bb_reflection_ClassInfo* >(t_,2));
	return this;
}
void bb_reflection_R34::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R34::debug(){
	String t="(R34)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R36::bb_reflection_R36(){
}
bb_reflection_R36* bb_reflection_R36::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__stringClass,bb_reflection__classes.At(1)};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(5),Array<bb_reflection_ClassInfo* >(t_,2));
	return this;
}
void bb_reflection_R36::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R36::debug(){
	String t="(R36)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R38::bb_reflection_R38(){
}
bb_reflection_R38* bb_reflection_R38::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__stringClass,bb_reflection__classes.At(1)};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(6),Array<bb_reflection_ClassInfo* >(t_,2));
	return this;
}
void bb_reflection_R38::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R38::debug(){
	String t="(R38)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R40::bb_reflection_R40(){
}
bb_reflection_R40* bb_reflection_R40::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__stringClass,bb_reflection__classes.At(1)};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(7),Array<bb_reflection_ClassInfo* >(t_,2));
	return this;
}
void bb_reflection_R40::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R40::debug(){
	String t="(R40)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R42::bb_reflection_R42(){
}
bb_reflection_R42* bb_reflection_R42::g_new(){
	bb_reflection_FieldInfo::g_new(String(L"value",5),0,bb_reflection__boolClass);
	return this;
}
void bb_reflection_R42::mark(){
	bb_reflection_FieldInfo::mark();
}
String bb_reflection_R42::debug(){
	String t="(R42)\n";
	t=bb_reflection_FieldInfo::debug()+t;
	return t;
}
bb_reflection_R44::bb_reflection_R44(){
}
bb_reflection_R44* bb_reflection_R44::g_new(){
	bb_reflection_MethodInfo::g_new(String(L"ToBool",6),0,bb_reflection__boolClass,Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R44::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R44::debug(){
	String t="(R44)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R45::bb_reflection_R45(){
}
bb_reflection_R45* bb_reflection_R45::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__classes.At(8)};
	bb_reflection_MethodInfo::g_new(String(L"Equals",6),0,bb_reflection__boolClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R45::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R45::debug(){
	String t="(R45)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R43::bb_reflection_R43(){
}
bb_reflection_R43* bb_reflection_R43::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__boolClass};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(8),Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R43::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R43::debug(){
	String t="(R43)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R46::bb_reflection_R46(){
}
bb_reflection_R46* bb_reflection_R46::g_new(){
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(8),Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R46::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R46::debug(){
	String t="(R46)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R48::bb_reflection_R48(){
}
bb_reflection_R48* bb_reflection_R48::g_new(){
	bb_reflection_FieldInfo::g_new(String(L"value",5),0,bb_reflection__intClass);
	return this;
}
void bb_reflection_R48::mark(){
	bb_reflection_FieldInfo::mark();
}
String bb_reflection_R48::debug(){
	String t="(R48)\n";
	t=bb_reflection_FieldInfo::debug()+t;
	return t;
}
bb_reflection_R51::bb_reflection_R51(){
}
bb_reflection_R51* bb_reflection_R51::g_new(){
	bb_reflection_MethodInfo::g_new(String(L"ToInt",5),0,bb_reflection__intClass,Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R51::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R51::debug(){
	String t="(R51)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R52::bb_reflection_R52(){
}
bb_reflection_R52* bb_reflection_R52::g_new(){
	bb_reflection_MethodInfo::g_new(String(L"ToFloat",7),0,bb_reflection__floatClass,Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R52::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R52::debug(){
	String t="(R52)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R53::bb_reflection_R53(){
}
bb_reflection_R53* bb_reflection_R53::g_new(){
	bb_reflection_MethodInfo::g_new(String(L"ToString",8),0,bb_reflection__stringClass,Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R53::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R53::debug(){
	String t="(R53)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R54::bb_reflection_R54(){
}
bb_reflection_R54* bb_reflection_R54::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__classes.At(9)};
	bb_reflection_MethodInfo::g_new(String(L"Equals",6),0,bb_reflection__boolClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R54::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R54::debug(){
	String t="(R54)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R55::bb_reflection_R55(){
}
bb_reflection_R55* bb_reflection_R55::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__classes.At(9)};
	bb_reflection_MethodInfo::g_new(String(L"Compare",7),0,bb_reflection__intClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R55::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R55::debug(){
	String t="(R55)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R49::bb_reflection_R49(){
}
bb_reflection_R49* bb_reflection_R49::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__intClass};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(9),Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R49::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R49::debug(){
	String t="(R49)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R50::bb_reflection_R50(){
}
bb_reflection_R50* bb_reflection_R50::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__floatClass};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(9),Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R50::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R50::debug(){
	String t="(R50)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R56::bb_reflection_R56(){
}
bb_reflection_R56* bb_reflection_R56::g_new(){
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(9),Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R56::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R56::debug(){
	String t="(R56)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R58::bb_reflection_R58(){
}
bb_reflection_R58* bb_reflection_R58::g_new(){
	bb_reflection_FieldInfo::g_new(String(L"value",5),0,bb_reflection__floatClass);
	return this;
}
void bb_reflection_R58::mark(){
	bb_reflection_FieldInfo::mark();
}
String bb_reflection_R58::debug(){
	String t="(R58)\n";
	t=bb_reflection_FieldInfo::debug()+t;
	return t;
}
bb_reflection_R61::bb_reflection_R61(){
}
bb_reflection_R61* bb_reflection_R61::g_new(){
	bb_reflection_MethodInfo::g_new(String(L"ToInt",5),0,bb_reflection__intClass,Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R61::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R61::debug(){
	String t="(R61)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R62::bb_reflection_R62(){
}
bb_reflection_R62* bb_reflection_R62::g_new(){
	bb_reflection_MethodInfo::g_new(String(L"ToFloat",7),0,bb_reflection__floatClass,Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R62::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R62::debug(){
	String t="(R62)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R63::bb_reflection_R63(){
}
bb_reflection_R63* bb_reflection_R63::g_new(){
	bb_reflection_MethodInfo::g_new(String(L"ToString",8),0,bb_reflection__stringClass,Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R63::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R63::debug(){
	String t="(R63)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R64::bb_reflection_R64(){
}
bb_reflection_R64* bb_reflection_R64::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__classes.At(10)};
	bb_reflection_MethodInfo::g_new(String(L"Equals",6),0,bb_reflection__boolClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R64::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R64::debug(){
	String t="(R64)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R65::bb_reflection_R65(){
}
bb_reflection_R65* bb_reflection_R65::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__classes.At(10)};
	bb_reflection_MethodInfo::g_new(String(L"Compare",7),0,bb_reflection__intClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R65::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R65::debug(){
	String t="(R65)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R59::bb_reflection_R59(){
}
bb_reflection_R59* bb_reflection_R59::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__intClass};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(10),Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R59::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R59::debug(){
	String t="(R59)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R60::bb_reflection_R60(){
}
bb_reflection_R60* bb_reflection_R60::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__floatClass};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(10),Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R60::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R60::debug(){
	String t="(R60)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R66::bb_reflection_R66(){
}
bb_reflection_R66* bb_reflection_R66::g_new(){
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(10),Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R66::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R66::debug(){
	String t="(R66)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R68::bb_reflection_R68(){
}
bb_reflection_R68* bb_reflection_R68::g_new(){
	bb_reflection_FieldInfo::g_new(String(L"value",5),0,bb_reflection__stringClass);
	return this;
}
void bb_reflection_R68::mark(){
	bb_reflection_FieldInfo::mark();
}
String bb_reflection_R68::debug(){
	String t="(R68)\n";
	t=bb_reflection_FieldInfo::debug()+t;
	return t;
}
bb_reflection_R72::bb_reflection_R72(){
}
bb_reflection_R72* bb_reflection_R72::g_new(){
	bb_reflection_MethodInfo::g_new(String(L"ToString",8),0,bb_reflection__stringClass,Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R72::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R72::debug(){
	String t="(R72)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R73::bb_reflection_R73(){
}
bb_reflection_R73* bb_reflection_R73::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__classes.At(11)};
	bb_reflection_MethodInfo::g_new(String(L"Equals",6),0,bb_reflection__boolClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R73::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R73::debug(){
	String t="(R73)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R74::bb_reflection_R74(){
}
bb_reflection_R74* bb_reflection_R74::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__classes.At(11)};
	bb_reflection_MethodInfo::g_new(String(L"Compare",7),0,bb_reflection__intClass,Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R74::mark(){
	bb_reflection_MethodInfo::mark();
}
String bb_reflection_R74::debug(){
	String t="(R74)\n";
	t=bb_reflection_MethodInfo::debug()+t;
	return t;
}
bb_reflection_R69::bb_reflection_R69(){
}
bb_reflection_R69* bb_reflection_R69::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__intClass};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(11),Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R69::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R69::debug(){
	String t="(R69)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R70::bb_reflection_R70(){
}
bb_reflection_R70* bb_reflection_R70::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__floatClass};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(11),Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R70::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R70::debug(){
	String t="(R70)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R71::bb_reflection_R71(){
}
bb_reflection_R71* bb_reflection_R71::g_new(){
	bb_reflection_ClassInfo* t_[]={bb_reflection__stringClass};
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(11),Array<bb_reflection_ClassInfo* >(t_,1));
	return this;
}
void bb_reflection_R71::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R71::debug(){
	String t="(R71)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_R75::bb_reflection_R75(){
}
bb_reflection_R75* bb_reflection_R75::g_new(){
	bb_reflection_FunctionInfo::g_new(String(L"new",3),0,bb_reflection__classes.At(11),Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_R75::mark(){
	bb_reflection_FunctionInfo::mark();
}
String bb_reflection_R75::debug(){
	String t="(R75)\n";
	t=bb_reflection_FunctionInfo::debug()+t;
	return t;
}
bb_reflection_UnknownClass::bb_reflection_UnknownClass(){
}
bb_reflection_UnknownClass* bb_reflection_UnknownClass::g_new(){
	DBG_ENTER("UnknownClass.new")
	bb_reflection_UnknownClass *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/reflection/reflection.monkey<625>");
	bb_reflection_ClassInfo::g_new(String(L"?",1),0,0,Array<bb_reflection_ClassInfo* >());
	return this;
}
void bb_reflection_UnknownClass::mark(){
	bb_reflection_ClassInfo::mark();
}
String bb_reflection_UnknownClass::debug(){
	String t="(UnknownClass)\n";
	t=bb_reflection_ClassInfo::debug()+t;
	return t;
}
bb_reflection_ClassInfo* bb_reflection__unknownClass;
bb_graphics_Image::bb_graphics_Image(){
	f_surface=0;
	f_width=0;
	f_height=0;
	f_frames=Array<bb_graphics_Frame* >();
	f_flags=0;
	f_tx=FLOAT(.0);
	f_ty=FLOAT(.0);
	f_source=0;
}
int bb_graphics_Image::g_DefaultFlags;
bb_graphics_Image* bb_graphics_Image::g_new(){
	DBG_ENTER("Image.new")
	bb_graphics_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<65>");
	return this;
}
int bb_graphics_Image::m_SetHandle(Float t_tx,Float t_ty){
	DBG_ENTER("Image.SetHandle")
	bb_graphics_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<109>");
	this->f_tx=t_tx;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<110>");
	this->f_ty=t_ty;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<111>");
	this->f_flags=this->f_flags&-2;
	return 0;
}
int bb_graphics_Image::m_ApplyFlags(int t_iflags){
	DBG_ENTER("Image.ApplyFlags")
	bb_graphics_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_iflags,"iflags")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<178>");
	f_flags=t_iflags;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<180>");
	if((f_flags&2)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<181>");
		Array<bb_graphics_Frame* > t_=f_frames;
		int t_2=0;
		while(t_2<t_.Length()){
			DBG_BLOCK();
			bb_graphics_Frame* t_f=t_.At(t_2);
			t_2=t_2+1;
			DBG_LOCAL(t_f,"f")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<182>");
			t_f->f_x+=1;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<184>");
		f_width-=2;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<187>");
	if((f_flags&4)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<188>");
		Array<bb_graphics_Frame* > t_3=f_frames;
		int t_4=0;
		while(t_4<t_3.Length()){
			DBG_BLOCK();
			bb_graphics_Frame* t_f2=t_3.At(t_4);
			t_4=t_4+1;
			DBG_LOCAL(t_f2,"f")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<189>");
			t_f2->f_y+=1;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<191>");
		f_height-=2;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<194>");
	if((f_flags&1)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<195>");
		m_SetHandle(Float(f_width)/FLOAT(2.0),Float(f_height)/FLOAT(2.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<198>");
	if(f_frames.Length()==1 && f_frames.At(0)->f_x==0 && f_frames.At(0)->f_y==0 && f_width==f_surface->Width() && f_height==f_surface->Height()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<199>");
		f_flags|=65536;
	}
	return 0;
}
bb_graphics_Image* bb_graphics_Image::m_Init2(gxtkSurface* t_surf,int t_nframes,int t_iflags){
	DBG_ENTER("Image.Init")
	bb_graphics_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_surf,"surf")
	DBG_LOCAL(t_nframes,"nframes")
	DBG_LOCAL(t_iflags,"iflags")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<136>");
	gc_assign(f_surface,t_surf);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<138>");
	f_width=f_surface->Width()/t_nframes;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<139>");
	f_height=f_surface->Height();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<141>");
	gc_assign(f_frames,Array<bb_graphics_Frame* >(t_nframes));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<142>");
	for(int t_i=0;t_i<t_nframes;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<143>");
		gc_assign(f_frames.At(t_i),(new bb_graphics_Frame)->g_new(t_i*f_width,0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<146>");
	m_ApplyFlags(t_iflags);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<147>");
	return this;
}
bb_graphics_Image* bb_graphics_Image::m_Grab(int t_x,int t_y,int t_iwidth,int t_iheight,int t_nframes,int t_iflags,bb_graphics_Image* t_source){
	DBG_ENTER("Image.Grab")
	bb_graphics_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_iwidth,"iwidth")
	DBG_LOCAL(t_iheight,"iheight")
	DBG_LOCAL(t_nframes,"nframes")
	DBG_LOCAL(t_iflags,"iflags")
	DBG_LOCAL(t_source,"source")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<151>");
	gc_assign(this->f_source,t_source);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<152>");
	gc_assign(f_surface,t_source->f_surface);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<154>");
	f_width=t_iwidth;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<155>");
	f_height=t_iheight;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<157>");
	gc_assign(f_frames,Array<bb_graphics_Frame* >(t_nframes));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<159>");
	int t_ix=t_x;
	int t_iy=t_y;
	DBG_LOCAL(t_ix,"ix")
	DBG_LOCAL(t_iy,"iy")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<161>");
	for(int t_i=0;t_i<t_nframes;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<162>");
		if(t_ix+f_width>t_source->f_width){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<163>");
			t_ix=0;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<164>");
			t_iy+=f_height;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<166>");
		if(t_ix+f_width>t_source->f_width || t_iy+f_height>t_source->f_height){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<167>");
			Error(String(L"Image frame outside surface",27));
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<169>");
		gc_assign(f_frames.At(t_i),(new bb_graphics_Frame)->g_new(t_ix+t_source->f_frames.At(0)->f_x,t_iy+t_source->f_frames.At(0)->f_y));
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<170>");
		t_ix+=f_width;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<173>");
	m_ApplyFlags(t_iflags);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<174>");
	return this;
}
bb_graphics_Image* bb_graphics_Image::m_GrabImage(int t_x,int t_y,int t_width,int t_height,int t_frames,int t_flags){
	DBG_ENTER("Image.GrabImage")
	bb_graphics_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_LOCAL(t_frames,"frames")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<104>");
	if(this->f_frames.Length()!=1){
		DBG_BLOCK();
		return 0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<105>");
	bb_graphics_Image* t_=((new bb_graphics_Image)->g_new())->m_Grab(t_x,t_y,t_width,t_height,t_frames,t_flags,this);
	return t_;
}
int bb_graphics_Image::m_Width(){
	DBG_ENTER("Image.Width")
	bb_graphics_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<76>");
	return f_width;
}
int bb_graphics_Image::m_Height(){
	DBG_ENTER("Image.Height")
	bb_graphics_Image *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<80>");
	return f_height;
}
void bb_graphics_Image::mark(){
	Object::mark();
	gc_mark_q(f_surface);
	gc_mark_q(f_frames);
	gc_mark_q(f_source);
}
String bb_graphics_Image::debug(){
	String t="(Image)\n";
	t+=dbg_decl("DefaultFlags",&bb_graphics_Image::g_DefaultFlags);
	t+=dbg_decl("source",&f_source);
	t+=dbg_decl("surface",&f_surface);
	t+=dbg_decl("width",&f_width);
	t+=dbg_decl("height",&f_height);
	t+=dbg_decl("flags",&f_flags);
	t+=dbg_decl("frames",&f_frames);
	t+=dbg_decl("tx",&f_tx);
	t+=dbg_decl("ty",&f_ty);
	return t;
}
bb_graphics_GraphicsContext::bb_graphics_GraphicsContext(){
	f_defaultFont=0;
	f_font=0;
	f_firstChar=0;
	f_matrixSp=0;
	f_ix=FLOAT(1.0);
	f_iy=FLOAT(.0);
	f_jx=FLOAT(.0);
	f_jy=FLOAT(1.0);
	f_tx=FLOAT(.0);
	f_ty=FLOAT(.0);
	f_tformed=0;
	f_matDirty=0;
	f_color_r=FLOAT(.0);
	f_color_g=FLOAT(.0);
	f_color_b=FLOAT(.0);
	f_alpha=FLOAT(.0);
	f_blend=0;
	f_scissor_x=FLOAT(.0);
	f_scissor_y=FLOAT(.0);
	f_scissor_width=FLOAT(.0);
	f_scissor_height=FLOAT(.0);
	f_matrixStack=Array<Float >(192);
}
bb_graphics_GraphicsContext* bb_graphics_GraphicsContext::g_new(){
	DBG_ENTER("GraphicsContext.new")
	bb_graphics_GraphicsContext *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<24>");
	return this;
}
int bb_graphics_GraphicsContext::m_Validate(){
	DBG_ENTER("GraphicsContext.Validate")
	bb_graphics_GraphicsContext *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<35>");
	if((f_matDirty)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<36>");
		bb_graphics_renderDevice->SetMatrix(bb_graphics_context->f_ix,bb_graphics_context->f_iy,bb_graphics_context->f_jx,bb_graphics_context->f_jy,bb_graphics_context->f_tx,bb_graphics_context->f_ty);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<37>");
		f_matDirty=0;
	}
	return 0;
}
void bb_graphics_GraphicsContext::mark(){
	Object::mark();
	gc_mark_q(f_defaultFont);
	gc_mark_q(f_font);
	gc_mark_q(f_matrixStack);
}
String bb_graphics_GraphicsContext::debug(){
	String t="(GraphicsContext)\n";
	t+=dbg_decl("color_r",&f_color_r);
	t+=dbg_decl("color_g",&f_color_g);
	t+=dbg_decl("color_b",&f_color_b);
	t+=dbg_decl("alpha",&f_alpha);
	t+=dbg_decl("blend",&f_blend);
	t+=dbg_decl("ix",&f_ix);
	t+=dbg_decl("iy",&f_iy);
	t+=dbg_decl("jx",&f_jx);
	t+=dbg_decl("jy",&f_jy);
	t+=dbg_decl("tx",&f_tx);
	t+=dbg_decl("ty",&f_ty);
	t+=dbg_decl("tformed",&f_tformed);
	t+=dbg_decl("matDirty",&f_matDirty);
	t+=dbg_decl("scissor_x",&f_scissor_x);
	t+=dbg_decl("scissor_y",&f_scissor_y);
	t+=dbg_decl("scissor_width",&f_scissor_width);
	t+=dbg_decl("scissor_height",&f_scissor_height);
	t+=dbg_decl("matrixStack",&f_matrixStack);
	t+=dbg_decl("matrixSp",&f_matrixSp);
	t+=dbg_decl("font",&f_font);
	t+=dbg_decl("firstChar",&f_firstChar);
	t+=dbg_decl("defaultFont",&f_defaultFont);
	return t;
}
bb_graphics_GraphicsContext* bb_graphics_context;
String bb_data_FixDataPath(String t_path){
	DBG_ENTER("FixDataPath")
	DBG_LOCAL(t_path,"path")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<3>");
	int t_i=t_path.Find(String(L":/",2),0);
	DBG_LOCAL(t_i,"i")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<4>");
	if(t_i!=-1 && t_path.Find(String(L"/",1),0)==t_i+1){
		DBG_BLOCK();
		return t_path;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<5>");
	if(t_path.StartsWith(String(L"./",2)) || t_path.StartsWith(String(L"/",1))){
		DBG_BLOCK();
		return t_path;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/data.monkey<6>");
	String t_=String(L"monkey://data/",14)+t_path;
	return t_;
}
bb_graphics_Frame::bb_graphics_Frame(){
	f_x=0;
	f_y=0;
}
bb_graphics_Frame* bb_graphics_Frame::g_new(int t_x,int t_y){
	DBG_ENTER("Frame.new")
	bb_graphics_Frame *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<18>");
	this->f_x=t_x;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<19>");
	this->f_y=t_y;
	return this;
}
bb_graphics_Frame* bb_graphics_Frame::g_new2(){
	DBG_ENTER("Frame.new")
	bb_graphics_Frame *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<13>");
	return this;
}
void bb_graphics_Frame::mark(){
	Object::mark();
}
String bb_graphics_Frame::debug(){
	String t="(Frame)\n";
	t+=dbg_decl("x",&f_x);
	t+=dbg_decl("y",&f_y);
	return t;
}
bb_graphics_Image* bb_graphics_LoadImage(String t_path,int t_frameCount,int t_flags){
	DBG_ENTER("LoadImage")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<229>");
	gxtkSurface* t_surf=bb_graphics_device->LoadSurface(bb_data_FixDataPath(t_path));
	DBG_LOCAL(t_surf,"surf")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<230>");
	if((t_surf)!=0){
		DBG_BLOCK();
		bb_graphics_Image* t_=((new bb_graphics_Image)->g_new())->m_Init2(t_surf,t_frameCount,t_flags);
		return t_;
	}
	return 0;
}
bb_graphics_Image* bb_graphics_LoadImage2(String t_path,int t_frameWidth,int t_frameHeight,int t_frameCount,int t_flags){
	DBG_ENTER("LoadImage")
	DBG_LOCAL(t_path,"path")
	DBG_LOCAL(t_frameWidth,"frameWidth")
	DBG_LOCAL(t_frameHeight,"frameHeight")
	DBG_LOCAL(t_frameCount,"frameCount")
	DBG_LOCAL(t_flags,"flags")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<234>");
	bb_graphics_Image* t_atlas=bb_graphics_LoadImage(t_path,1,0);
	DBG_LOCAL(t_atlas,"atlas")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<235>");
	if((t_atlas)!=0){
		DBG_BLOCK();
		bb_graphics_Image* t_=t_atlas->m_GrabImage(0,0,t_frameWidth,t_frameHeight,t_frameCount,t_flags);
		return t_;
	}
	return 0;
}
int bb_graphics_SetFont(bb_graphics_Image* t_font,int t_firstChar){
	DBG_ENTER("SetFont")
	DBG_LOCAL(t_font,"font")
	DBG_LOCAL(t_firstChar,"firstChar")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<524>");
	if(!((t_font)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<525>");
		if(!((bb_graphics_context->f_defaultFont)!=0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<526>");
			gc_assign(bb_graphics_context->f_defaultFont,bb_graphics_LoadImage(String(L"mojo_font.png",13),96,2));
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<528>");
		t_font=bb_graphics_context->f_defaultFont;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<529>");
		t_firstChar=32;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<531>");
	gc_assign(bb_graphics_context->f_font,t_font);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<532>");
	bb_graphics_context->f_firstChar=t_firstChar;
	return 0;
}
gxtkGraphics* bb_graphics_renderDevice;
int bb_graphics_SetMatrix(Float t_ix,Float t_iy,Float t_jx,Float t_jy,Float t_tx,Float t_ty){
	DBG_ENTER("SetMatrix")
	DBG_LOCAL(t_ix,"ix")
	DBG_LOCAL(t_iy,"iy")
	DBG_LOCAL(t_jx,"jx")
	DBG_LOCAL(t_jy,"jy")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<289>");
	bb_graphics_context->f_ix=t_ix;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<290>");
	bb_graphics_context->f_iy=t_iy;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<291>");
	bb_graphics_context->f_jx=t_jx;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<292>");
	bb_graphics_context->f_jy=t_jy;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<293>");
	bb_graphics_context->f_tx=t_tx;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<294>");
	bb_graphics_context->f_ty=t_ty;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<295>");
	bb_graphics_context->f_tformed=((t_ix!=FLOAT(1.0) || t_iy!=FLOAT(0.0) || t_jx!=FLOAT(0.0) || t_jy!=FLOAT(1.0) || t_tx!=FLOAT(0.0) || t_ty!=FLOAT(0.0))?1:0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<296>");
	bb_graphics_context->f_matDirty=1;
	return 0;
}
int bb_graphics_SetMatrix2(Array<Float > t_m){
	DBG_ENTER("SetMatrix")
	DBG_LOCAL(t_m,"m")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<285>");
	bb_graphics_SetMatrix(t_m.At(0),t_m.At(1),t_m.At(2),t_m.At(3),t_m.At(4),t_m.At(5));
	return 0;
}
int bb_graphics_SetColor(Float t_r,Float t_g,Float t_b){
	DBG_ENTER("SetColor")
	DBG_LOCAL(t_r,"r")
	DBG_LOCAL(t_g,"g")
	DBG_LOCAL(t_b,"b")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<244>");
	bb_graphics_context->f_color_r=t_r;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<245>");
	bb_graphics_context->f_color_g=t_g;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<246>");
	bb_graphics_context->f_color_b=t_b;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<247>");
	bb_graphics_renderDevice->SetColor(t_r,t_g,t_b);
	return 0;
}
int bb_graphics_SetAlpha(Float t_alpha){
	DBG_ENTER("SetAlpha")
	DBG_LOCAL(t_alpha,"alpha")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<255>");
	bb_graphics_context->f_alpha=t_alpha;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<256>");
	bb_graphics_renderDevice->SetAlpha(t_alpha);
	return 0;
}
int bb_graphics_SetBlend(int t_blend){
	DBG_ENTER("SetBlend")
	DBG_LOCAL(t_blend,"blend")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<264>");
	bb_graphics_context->f_blend=t_blend;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<265>");
	bb_graphics_renderDevice->SetBlend(t_blend);
	return 0;
}
int bb_graphics_DeviceWidth(){
	DBG_ENTER("DeviceWidth")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<221>");
	int t_=bb_graphics_device->Width();
	return t_;
}
int bb_graphics_DeviceHeight(){
	DBG_ENTER("DeviceHeight")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<225>");
	int t_=bb_graphics_device->Height();
	return t_;
}
int bb_graphics_SetScissor(Float t_x,Float t_y,Float t_width,Float t_height){
	DBG_ENTER("SetScissor")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_width,"width")
	DBG_LOCAL(t_height,"height")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<273>");
	bb_graphics_context->f_scissor_x=t_x;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<274>");
	bb_graphics_context->f_scissor_y=t_y;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<275>");
	bb_graphics_context->f_scissor_width=t_width;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<276>");
	bb_graphics_context->f_scissor_height=t_height;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<277>");
	bb_graphics_renderDevice->SetScissor(int(t_x),int(t_y),int(t_width),int(t_height));
	return 0;
}
int bb_graphics_BeginRender(){
	DBG_ENTER("BeginRender")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<206>");
	if(!((bb_graphics_device->Mode())!=0)){
		DBG_BLOCK();
		return 0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<207>");
	gc_assign(bb_graphics_renderDevice,bb_graphics_device);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<208>");
	bb_graphics_context->f_matrixSp=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<209>");
	bb_graphics_SetMatrix(FLOAT(1.0),FLOAT(0.0),FLOAT(0.0),FLOAT(1.0),FLOAT(0.0),FLOAT(0.0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<210>");
	bb_graphics_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<211>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<212>");
	bb_graphics_SetBlend(0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<213>");
	bb_graphics_SetScissor(FLOAT(0.0),FLOAT(0.0),Float(bb_graphics_DeviceWidth()),Float(bb_graphics_DeviceHeight()));
	return 0;
}
int bb_graphics_EndRender(){
	DBG_ENTER("EndRender")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<217>");
	bb_graphics_renderDevice=0;
	return 0;
}
int bb_challengergui_CHGUI_MobileMode;
int bb_app_SetUpdateRate(int t_hertz){
	DBG_ENTER("SetUpdateRate")
	DBG_LOCAL(t_hertz,"hertz")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<145>");
	int t_=bb_app_device->SetUpdateRate(t_hertz);
	return t_;
}
String bb_data2_STATUS;
bb_stream_Stream::bb_stream_Stream(){
}
bb_stream_Stream* bb_stream_Stream::g_new(){
	DBG_ENTER("Stream.new")
	bb_stream_Stream *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<23>");
	return this;
}
bb_databuffer_DataBuffer* bb_stream_Stream::g__tmpbuf;
void bb_stream_Stream::m__Write(int t_n){
	DBG_ENTER("Stream._Write")
	bb_stream_Stream *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_n,"n")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<125>");
	if(m_Write(bb_stream_Stream::g__tmpbuf,0,t_n)!=t_n){
		DBG_BLOCK();
		throw (new bb_stream_StreamWriteError)->g_new(this);
	}
}
void bb_stream_Stream::m_WriteByte(int t_value){
	DBG_ENTER("Stream.WriteByte")
	bb_stream_Stream *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<82>");
	bb_stream_Stream::g__tmpbuf->PokeByte(0,t_value);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<83>");
	m__Write(1);
}
void bb_stream_Stream::m_WriteLine(String t_str){
	DBG_ENTER("Stream.WriteLine")
	bb_stream_Stream *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_str,"str")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<102>");
	String t_=t_str;
	int t_2=0;
	while(t_2<t_.Length()){
		DBG_BLOCK();
		int t_ch=(int)t_[t_2];
		t_2=t_2+1;
		DBG_LOCAL(t_ch,"ch")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<103>");
		m_WriteByte(t_ch);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<105>");
	m_WriteByte(13);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<106>");
	m_WriteByte(10);
}
String bb_stream_Stream::m_ReadLine(){
	DBG_ENTER("Stream.ReadLine")
	bb_stream_Stream *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<70>");
	bb_stack_Stack6* t_buf=(new bb_stack_Stack6)->g_new();
	DBG_LOCAL(t_buf,"buf")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<71>");
	while(!((m_Eof())!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<72>");
		int t_n=m_Read(bb_stream_Stream::g__tmpbuf,0,1);
		DBG_LOCAL(t_n,"n")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<73>");
		if(!((t_n)!=0)){
			DBG_BLOCK();
			break;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<74>");
		int t_ch=bb_stream_Stream::g__tmpbuf->PeekByte(0);
		DBG_LOCAL(t_ch,"ch")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<75>");
		if(!((t_ch)!=0) || t_ch==10){
			DBG_BLOCK();
			break;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<76>");
		if(t_ch!=13){
			DBG_BLOCK();
			t_buf->m_Push16(t_ch);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<78>");
	String t_=String::FromChars(t_buf->m_ToArray());
	return t_;
}
void bb_stream_Stream::mark(){
	Object::mark();
}
String bb_stream_Stream::debug(){
	String t="(Stream)\n";
	t+=dbg_decl("_tmpbuf",&bb_stream_Stream::g__tmpbuf);
	return t;
}
bb_tcpstream_TcpStream::bb_tcpstream_TcpStream(){
	f__stream=0;
}
bb_tcpstream_TcpStream* bb_tcpstream_TcpStream::g_new(){
	DBG_ENTER("TcpStream.new")
	bb_tcpstream_TcpStream *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<24>");
	bb_stream_Stream::g_new();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<25>");
	gc_assign(f__stream,(new BBTcpStream));
	return this;
}
bool bb_tcpstream_TcpStream::m_Connect(String t_host,int t_port){
	DBG_ENTER("TcpStream.Connect")
	bb_tcpstream_TcpStream *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_host,"host")
	DBG_LOCAL(t_port,"port")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<29>");
	bool t_=f__stream->Connect(t_host,t_port);
	return t_;
}
int bb_tcpstream_TcpStream::m_ReadAvail(){
	DBG_ENTER("TcpStream.ReadAvail")
	bb_tcpstream_TcpStream *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<33>");
	int t_=f__stream->ReadAvail();
	return t_;
}
void bb_tcpstream_TcpStream::m_Close(){
	DBG_ENTER("TcpStream.Close")
	bb_tcpstream_TcpStream *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<42>");
	if((f__stream)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<43>");
		f__stream->Close();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<44>");
		f__stream=0;
	}
}
int bb_tcpstream_TcpStream::m_Eof(){
	DBG_ENTER("TcpStream.Eof")
	bb_tcpstream_TcpStream *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<49>");
	int t_=f__stream->Eof();
	return t_;
}
int bb_tcpstream_TcpStream::m_Read(bb_databuffer_DataBuffer* t_buffer,int t_offset,int t_count){
	DBG_ENTER("TcpStream.Read")
	bb_tcpstream_TcpStream *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_buffer,"buffer")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<65>");
	int t_=f__stream->Read(t_buffer,t_offset,t_count);
	return t_;
}
int bb_tcpstream_TcpStream::m_Write(bb_databuffer_DataBuffer* t_buffer,int t_offset,int t_count){
	DBG_ENTER("TcpStream.Write")
	bb_tcpstream_TcpStream *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_buffer,"buffer")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/tcpstream.monkey<69>");
	int t_=f__stream->Write(t_buffer,t_offset,t_count);
	return t_;
}
void bb_tcpstream_TcpStream::mark(){
	bb_stream_Stream::mark();
	gc_mark_q(f__stream);
}
String bb_tcpstream_TcpStream::debug(){
	String t="(TcpStream)\n";
	t=bb_stream_Stream::debug()+t;
	t+=dbg_decl("_stream",&f__stream);
	return t;
}
bb_tcpstream_TcpStream* bb_data2_Server;
bb_databuffer_DataBuffer::bb_databuffer_DataBuffer(){
}
bb_databuffer_DataBuffer* bb_databuffer_DataBuffer::g_new(int t_length){
	DBG_ENTER("DataBuffer.new")
	bb_databuffer_DataBuffer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_length,"length")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/databuffer.monkey<41>");
	if(!_New(t_length)){
		DBG_BLOCK();
		Error(String(L"Allocate DataBuffer failed",26));
	}
	return this;
}
bb_databuffer_DataBuffer* bb_databuffer_DataBuffer::g_new2(){
	DBG_ENTER("DataBuffer.new")
	bb_databuffer_DataBuffer *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/databuffer.monkey<38>");
	return this;
}
void bb_databuffer_DataBuffer::mark(){
	BBDataBuffer::mark();
}
String bb_databuffer_DataBuffer::debug(){
	String t="(DataBuffer)\n";
	return t;
}
bb_stream_StreamError::bb_stream_StreamError(){
	f__stream=0;
}
bb_stream_StreamError* bb_stream_StreamError::g_new(bb_stream_Stream* t_stream){
	DBG_ENTER("StreamError.new")
	bb_stream_StreamError *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_stream,"stream")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<133>");
	gc_assign(f__stream,t_stream);
	return this;
}
bb_stream_StreamError* bb_stream_StreamError::g_new2(){
	DBG_ENTER("StreamError.new")
	bb_stream_StreamError *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<130>");
	return this;
}
void bb_stream_StreamError::mark(){
	ThrowableObject::mark();
	gc_mark_q(f__stream);
}
String bb_stream_StreamError::debug(){
	String t="(StreamError)\n";
	t+=dbg_decl("_stream",&f__stream);
	return t;
}
bb_stream_StreamWriteError::bb_stream_StreamWriteError(){
}
bb_stream_StreamWriteError* bb_stream_StreamWriteError::g_new(bb_stream_Stream* t_stream){
	DBG_ENTER("StreamWriteError.new")
	bb_stream_StreamWriteError *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_stream,"stream")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<162>");
	bb_stream_StreamError::g_new(t_stream);
	return this;
}
bb_stream_StreamWriteError* bb_stream_StreamWriteError::g_new2(){
	DBG_ENTER("StreamWriteError.new")
	bb_stream_StreamWriteError *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/brl/stream.monkey<159>");
	bb_stream_StreamError::g_new2();
	return this;
}
void bb_stream_StreamWriteError::mark(){
	bb_stream_StreamError::mark();
}
String bb_stream_StreamWriteError::debug(){
	String t="(StreamWriteError)\n";
	t=bb_stream_StreamError::debug()+t;
	return t;
}
int bb_protocol_Post(String t_url){
	DBG_ENTER("Post")
	DBG_LOCAL(t_url,"url")
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<4>");
	bb_data2_Server->m_WriteLine(String(L"GET ",4)+t_url+String(L" HTTP/1.0",9));
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<5>");
	bb_data2_Server->m_WriteLine(String((Char)(10),1));
	return 0;
}
int bb_protocol_RequestGameList(){
	DBG_ENTER("RequestGameList")
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<9>");
	gc_assign(bb_data2_Server,(new bb_tcpstream_TcpStream)->g_new());
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<10>");
	do{
		DBG_BLOCK();
	}while(!(bb_data2_Server->m_Connect(String(L"www.fuzzit.us",13),80)));
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<12>");
	bb_protocol_Post(String(L"http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=mobilegetgamelist",69));
	return 0;
}
bb_stack_Stack6::bb_stack_Stack6(){
	f_data=Array<int >();
	f_length=0;
}
bb_stack_Stack6* bb_stack_Stack6::g_new(){
	DBG_ENTER("Stack.new")
	bb_stack_Stack6 *self=this;
	DBG_LOCAL(self,"Self")
	return this;
}
bb_stack_Stack6* bb_stack_Stack6::g_new2(Array<int > t_data){
	DBG_ENTER("Stack.new")
	bb_stack_Stack6 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_data,"data")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<13>");
	gc_assign(this->f_data,t_data.Slice(0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<14>");
	this->f_length=t_data.Length();
	return this;
}
int bb_stack_Stack6::m_Push16(int t_value){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack6 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<52>");
	if(f_length==f_data.Length()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<53>");
		gc_assign(f_data,f_data.Resize(f_length*2+10));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<55>");
	f_data.At(f_length)=t_value;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<56>");
	f_length+=1;
	return 0;
}
int bb_stack_Stack6::m_Push17(Array<int > t_values,int t_offset,int t_count){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack6 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_LOCAL(t_count,"count")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<66>");
	for(int t_i=0;t_i<t_count;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<67>");
		m_Push16(t_values.At(t_offset+t_i));
	}
	return 0;
}
int bb_stack_Stack6::m_Push18(Array<int > t_values,int t_offset){
	DBG_ENTER("Stack.Push")
	bb_stack_Stack6 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_values,"values")
	DBG_LOCAL(t_offset,"offset")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<60>");
	for(int t_i=t_offset;t_i<t_values.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<61>");
		m_Push16(t_values.At(t_i));
	}
	return 0;
}
Array<int > bb_stack_Stack6::m_ToArray(){
	DBG_ENTER("Stack.ToArray")
	bb_stack_Stack6 *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<18>");
	Array<int > t_t=Array<int >(f_length);
	DBG_LOCAL(t_t,"t")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<19>");
	for(int t_i=0;t_i<f_length;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<20>");
		t_t.At(t_i)=f_data.At(t_i);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/stack.monkey<22>");
	return t_t;
}
void bb_stack_Stack6::mark(){
	Object::mark();
	gc_mark_q(f_data);
}
String bb_stack_Stack6::debug(){
	String t="(Stack)\n";
	t+=dbg_decl("data",&f_data);
	t+=dbg_decl("length",&f_length);
	return t;
}
bb_bapp_BApp* bb_data2_Game;
bb_challengergui_CHGUI::bb_challengergui_CHGUI(){
	f_Parent=0;
	f_Value=FLOAT(.0);
	f_Text=String();
	f_Element=String();
	f_DropdownItems=Array<bb_challengergui_CHGUI* >();
	f_Visible=1;
	f_Minimised=0;
	f_X=FLOAT(.0);
	f_Y=FLOAT(.0);
	f_W=FLOAT(.0);
	f_H=FLOAT(.0);
	f_Active=1;
	f_Shadow=0;
	f_Close=0;
	f_CloseOver=0;
	f_CloseDown=0;
	f_Minimise=0;
	f_MinimiseOver=0;
	f_MinimiseDown=0;
	f_HasMenu=0;
	f_MenuHeight=0;
	f_Tabbed=0;
	f_TabHeight=0;
	f_Buttons=Array<bb_challengergui_CHGUI* >();
	f_Over=0;
	f_Down=0;
	f_ImageButtons=Array<bb_challengergui_CHGUI* >();
	f_Img=0;
	f_Tickboxes=Array<bb_challengergui_CHGUI* >();
	f_Radioboxes=Array<bb_challengergui_CHGUI* >();
	f_Listboxes=Array<bb_challengergui_CHGUI* >();
	f_ListboxSlider=0;
	f_ListboxNumber=0;
	f_ListboxItems=Array<bb_challengergui_CHGUI* >();
	f_ListHeight=0;
	f_SelectedListboxItem=0;
	f_HSliders=Array<bb_challengergui_CHGUI* >();
	f_MinusOver=0;
	f_MinusDown=0;
	f_PlusOver=0;
	f_PlusDown=0;
	f_SliderOver=0;
	f_SliderDown=0;
	f_Minimum=FLOAT(.0);
	f_Stp=FLOAT(.0);
	f_SWidth=FLOAT(.0);
	f_VSliders=Array<bb_challengergui_CHGUI* >();
	f_Textfields=Array<bb_challengergui_CHGUI* >();
	f_OnFocus=0;
	f_Cursor=0;
	f_Labels=Array<bb_challengergui_CHGUI* >();
	f_Dropdowns=Array<bb_challengergui_CHGUI* >();
	f_DropNumber=0;
	f_Menus=Array<bb_challengergui_CHGUI* >();
	f_Tabs=Array<bb_challengergui_CHGUI* >();
	f_CurrentTab=0;
	f_BottomList=Array<bb_challengergui_CHGUI* >();
	f_VariList=Array<bb_challengergui_CHGUI* >();
	f_TopList=Array<bb_challengergui_CHGUI* >();
	f_MenuItems=Array<bb_challengergui_CHGUI* >();
	f_IsMenuParent=0;
	f_Tick=0;
	f_MenuWidth=0;
	f_MenuNumber=0;
	f_Tooltip=String();
	f_Moveable=0;
	f_Mode=0;
	f_IsParent=0;
	f_SubWindow=0;
	f_ReOrdered=0;
	f_Clicked=0;
	f_DoubleClickMillisecs=0;
	f_DoubleClicked=0;
	f_StartOvertime=0;
	f_OverTime=0;
	f_StartDowntime=0;
	f_DownTime=0;
	f_TopVari=0;
	f_TopTop=0;
	f_TopBottom=0;
	f_MenuActive=0;
	f_MenuOver=0;
	f_FormatText=1;
	f_FormatNumber=1;
	f_FormatSymbol=1;
	f_FormatSpace=1;
	f_DKeyMillisecs=0;
	f_Maximum=FLOAT(.0);
	f_Start=0;
	f_Group=0;
	f_Moving=0;
	f_MX=FLOAT(.0);
	f_MY=FLOAT(.0);
	f_DClickMillisecs=0;
}
bb_challengergui_CHGUI* bb_challengergui_CHGUI::g_new(){
	DBG_ENTER("CHGUI.new")
	bb_challengergui_CHGUI *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<139>");
	return this;
}
int bb_challengergui_CHGUI::m_CheckClicked(){
	DBG_ENTER("CHGUI.CheckClicked")
	bb_challengergui_CHGUI *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<349>");
	if(bb_challengergui_CHGUI_RealActive(this)==0 || bb_challengergui_CHGUI_RealVisible(this)==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<350>");
		f_Over=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<351>");
		f_Down=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<352>");
		f_Clicked=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<354>");
	if(((f_Over)!=0) && ((f_Down)!=0) && bb_input_TouchDown(0)==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<355>");
		f_Clicked=1;
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<357>");
		f_Clicked=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<361>");
	if((f_Clicked)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<362>");
		if(bb_app_Millisecs()<f_DoubleClickMillisecs){
			DBG_BLOCK();
			f_DoubleClicked=1;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<363>");
		f_DoubleClickMillisecs=bb_app_Millisecs()+275;
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<365>");
		f_DoubleClicked=0;
	}
	return 0;
}
int bb_challengergui_CHGUI::m_CheckOver(){
	DBG_ENTER("CHGUI.CheckOver")
	bb_challengergui_CHGUI *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<276>");
	if(f_Minimised==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<277>");
		if(this!=bb_challengergui_CHGUI_Canvas){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<278>");
			int t_XX=bb_challengergui_CHGUI_RealX(this);
			DBG_LOCAL(t_XX,"XX")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<279>");
			int t_YY=bb_challengergui_CHGUI_RealY(this);
			DBG_LOCAL(t_YY,"YY")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<281>");
			if(bb_challengergui_CHGUI_OverFlag==0 && bb_challengergui_CHGUI_IgnoreMouse==0 && bb_input_TouchX(0)>Float(t_XX) && bb_input_TouchX(0)<Float(t_XX)+f_W && bb_input_TouchY(0)>Float(t_YY) && bb_input_TouchY(0)<Float(t_YY)+f_H){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<282>");
				f_Over=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<283>");
				bb_challengergui_CHGUI_Over=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<284>");
				bb_challengergui_CHGUI_OverFlag=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<285>");
				if(f_Element!=String(L"Window",6)){
					DBG_BLOCK();
					bb_challengergui_CHGUI_DragOver=1;
				}
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<286>");
				f_OverTime=bb_app_Millisecs()-f_StartOvertime;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<288>");
				f_Over=0;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<289>");
				f_OverTime=0;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<290>");
				f_StartOvertime=bb_app_Millisecs();
			}
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<294>");
		if(this!=bb_challengergui_CHGUI_Canvas){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<295>");
			int t_XX2=bb_challengergui_CHGUI_RealX(this);
			DBG_LOCAL(t_XX2,"XX")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<296>");
			int t_YY2=bb_challengergui_CHGUI_RealY(this);
			DBG_LOCAL(t_YY2,"YY")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<297>");
			if(bb_challengergui_CHGUI_OverFlag==0 && bb_challengergui_CHGUI_IgnoreMouse==0 && bb_input_TouchX(0)>Float(t_XX2) && bb_input_TouchX(0)<Float(t_XX2)+f_W && bb_input_TouchY(0)>Float(t_YY2) && bb_input_TouchY(0)<Float(t_YY2)+bb_challengergui_CHGUI_TitleHeight){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<298>");
				f_Over=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<299>");
				bb_challengergui_CHGUI_Over=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<300>");
				bb_challengergui_CHGUI_OverFlag=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<301>");
				f_OverTime=bb_app_Millisecs()-f_StartOvertime;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<303>");
				f_Over=0;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<304>");
				f_OverTime=0;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<305>");
				f_StartOvertime=bb_app_Millisecs();
			}
		}
	}
	return 0;
}
int bb_challengergui_CHGUI::m_CheckDown(){
	DBG_ENTER("CHGUI.CheckDown")
	bb_challengergui_CHGUI *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<314>");
	if(((f_Over)!=0) && ((bb_input_TouchDown(0))!=0) && bb_challengergui_CHGUI_MouseBusy==0 || f_Down==1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<315>");
		bb_challengergui_CHGUI_DownFlag=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<316>");
		bb_challengergui_CHGUI_MouseBusy=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<317>");
		f_Down=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<318>");
		if(f_Element!=String(L"Window",6)){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DragOver=1;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<319>");
		f_DownTime=bb_app_Millisecs()-f_StartDowntime;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<320>");
		f_StartOvertime=bb_app_Millisecs();
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<322>");
	if(bb_input_TouchDown(0)==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<323>");
		f_Down=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<324>");
		f_DownTime=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<325>");
		f_StartDowntime=bb_app_Millisecs();
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<327>");
	if(f_Over==1 && ((bb_input_TouchDown(0))!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<328>");
		if(f_Down==0){
			DBG_BLOCK();
			f_Over=0;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<331>");
	if((f_Down)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<332>");
		if(f_Mode==1){
			DBG_BLOCK();
			bb_challengergui_CHGUI_Reorder(this);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<333>");
		bb_challengergui_CHGUI* t_E=this;
		DBG_LOCAL(t_E,"E")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<334>");
		do{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<335>");
			if(t_E->f_Parent!=0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<336>");
				if(t_E->f_Parent->f_Element==String(L"Window",6) && t_E->f_Parent->f_Mode==1){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<337>");
					bb_challengergui_CHGUI_Reorder(t_E->f_Parent);
				}
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<339>");
				t_E=t_E->f_Parent;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<341>");
				break;
			}
		}while(!(false));
	}
	return 0;
}
void bb_challengergui_CHGUI::mark(){
	Object::mark();
	gc_mark_q(f_Parent);
	gc_mark_q(f_DropdownItems);
	gc_mark_q(f_Buttons);
	gc_mark_q(f_ImageButtons);
	gc_mark_q(f_Img);
	gc_mark_q(f_Tickboxes);
	gc_mark_q(f_Radioboxes);
	gc_mark_q(f_Listboxes);
	gc_mark_q(f_ListboxSlider);
	gc_mark_q(f_ListboxItems);
	gc_mark_q(f_SelectedListboxItem);
	gc_mark_q(f_HSliders);
	gc_mark_q(f_VSliders);
	gc_mark_q(f_Textfields);
	gc_mark_q(f_Labels);
	gc_mark_q(f_Dropdowns);
	gc_mark_q(f_Menus);
	gc_mark_q(f_Tabs);
	gc_mark_q(f_CurrentTab);
	gc_mark_q(f_BottomList);
	gc_mark_q(f_VariList);
	gc_mark_q(f_TopList);
	gc_mark_q(f_MenuItems);
	gc_mark_q(f_TopVari);
	gc_mark_q(f_TopTop);
	gc_mark_q(f_TopBottom);
	gc_mark_q(f_MenuActive);
	gc_mark_q(f_MenuOver);
}
String bb_challengergui_CHGUI::debug(){
	String t="(CHGUI)\n";
	t+=dbg_decl("X",&f_X);
	t+=dbg_decl("Y",&f_Y);
	t+=dbg_decl("W",&f_W);
	t+=dbg_decl("H",&f_H);
	t+=dbg_decl("Visible",&f_Visible);
	t+=dbg_decl("Active",&f_Active);
	t+=dbg_decl("Text",&f_Text);
	t+=dbg_decl("Over",&f_Over);
	t+=dbg_decl("Down",&f_Down);
	t+=dbg_decl("OverTime",&f_OverTime);
	t+=dbg_decl("DownTime",&f_DownTime);
	t+=dbg_decl("Clicked",&f_Clicked);
	t+=dbg_decl("DoubleClicked",&f_DoubleClicked);
	t+=dbg_decl("Value",&f_Value);
	t+=dbg_decl("Close",&f_Close);
	t+=dbg_decl("Minimise",&f_Minimise);
	t+=dbg_decl("Minimised",&f_Minimised);
	t+=dbg_decl("Moveable",&f_Moveable);
	t+=dbg_decl("Moving",&f_Moving);
	t+=dbg_decl("Mode",&f_Mode);
	t+=dbg_decl("Shadow",&f_Shadow);
	t+=dbg_decl("Group",&f_Group);
	t+=dbg_decl("Img",&f_Img);
	t+=dbg_decl("Tick",&f_Tick);
	t+=dbg_decl("Minimum",&f_Minimum);
	t+=dbg_decl("Maximum",&f_Maximum);
	t+=dbg_decl("CurrentTab",&f_CurrentTab);
	t+=dbg_decl("Tooltip",&f_Tooltip);
	t+=dbg_decl("MX",&f_MX);
	t+=dbg_decl("MY",&f_MY);
	t+=dbg_decl("ListboxNumber",&f_ListboxNumber);
	t+=dbg_decl("StartOvertime",&f_StartOvertime);
	t+=dbg_decl("StartDowntime",&f_StartDowntime);
	t+=dbg_decl("SelectedListboxItem",&f_SelectedListboxItem);
	t+=dbg_decl("MinimiseOver",&f_MinimiseOver);
	t+=dbg_decl("MinimiseDown",&f_MinimiseDown);
	t+=dbg_decl("CloseOver",&f_CloseOver);
	t+=dbg_decl("CloseDown",&f_CloseDown);
	t+=dbg_decl("Parent",&f_Parent);
	t+=dbg_decl("IsParent",&f_IsParent);
	t+=dbg_decl("OnFocus",&f_OnFocus);
	t+=dbg_decl("Element",&f_Element);
	t+=dbg_decl("TopTop",&f_TopTop);
	t+=dbg_decl("TopBottom",&f_TopBottom);
	t+=dbg_decl("HasMenu",&f_HasMenu);
	t+=dbg_decl("MenuHeight",&f_MenuHeight);
	t+=dbg_decl("MenuActive",&f_MenuActive);
	t+=dbg_decl("ReOrdered",&f_ReOrdered);
	t+=dbg_decl("DKeyMillisecs",&f_DKeyMillisecs);
	t+=dbg_decl("SubWindow",&f_SubWindow);
	t+=dbg_decl("TopVari",&f_TopVari);
	t+=dbg_decl("DClickMillisecs",&f_DClickMillisecs);
	t+=dbg_decl("IsMenuParent",&f_IsMenuParent);
	t+=dbg_decl("MenuWidth",&f_MenuWidth);
	t+=dbg_decl("MenuOver",&f_MenuOver);
	t+=dbg_decl("MenuNumber",&f_MenuNumber);
	t+=dbg_decl("DropNumber",&f_DropNumber);
	t+=dbg_decl("SWidth",&f_SWidth);
	t+=dbg_decl("Stp",&f_Stp);
	t+=dbg_decl("MinusOver",&f_MinusOver);
	t+=dbg_decl("PlusOver",&f_PlusOver);
	t+=dbg_decl("MinusDown",&f_MinusDown);
	t+=dbg_decl("PlusDown",&f_PlusDown);
	t+=dbg_decl("SliderOver",&f_SliderOver);
	t+=dbg_decl("SliderDown",&f_SliderDown);
	t+=dbg_decl("Start",&f_Start);
	t+=dbg_decl("Cursor",&f_Cursor);
	t+=dbg_decl("Tabbed",&f_Tabbed);
	t+=dbg_decl("TabHeight",&f_TabHeight);
	t+=dbg_decl("ListHeight",&f_ListHeight);
	t+=dbg_decl("ListboxSlider",&f_ListboxSlider);
	t+=dbg_decl("Buttons",&f_Buttons);
	t+=dbg_decl("ImageButtons",&f_ImageButtons);
	t+=dbg_decl("Tickboxes",&f_Tickboxes);
	t+=dbg_decl("Radioboxes",&f_Radioboxes);
	t+=dbg_decl("Dropdowns",&f_Dropdowns);
	t+=dbg_decl("Menus",&f_Menus);
	t+=dbg_decl("TopList",&f_TopList);
	t+=dbg_decl("VariList",&f_VariList);
	t+=dbg_decl("BottomList",&f_BottomList);
	t+=dbg_decl("HSliders",&f_HSliders);
	t+=dbg_decl("VSliders",&f_VSliders);
	t+=dbg_decl("Textfields",&f_Textfields);
	t+=dbg_decl("Tabs",&f_Tabs);
	t+=dbg_decl("Labels",&f_Labels);
	t+=dbg_decl("DropdownItems",&f_DropdownItems);
	t+=dbg_decl("MenuItems",&f_MenuItems);
	t+=dbg_decl("Listboxes",&f_Listboxes);
	t+=dbg_decl("ListboxItems",&f_ListboxItems);
	t+=dbg_decl("DoubleClickMillisecs",&f_DoubleClickMillisecs);
	t+=dbg_decl("FormatNumber",&f_FormatNumber);
	t+=dbg_decl("FormatText",&f_FormatText);
	t+=dbg_decl("FormatSymbol",&f_FormatSymbol);
	t+=dbg_decl("FormatSpace",&f_FormatSpace);
	return t;
}
bb_challengergui_CHGUI* bb_challengergui_CreateDropdownItem(String t_Text,bb_challengergui_CHGUI* t_Dropdown,int t_Value){
	DBG_ENTER("CreateDropdownItem")
	DBG_LOCAL(t_Text,"Text")
	DBG_LOCAL(t_Dropdown,"Dropdown")
	DBG_LOCAL(t_Value,"Value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<515>");
	bb_challengergui_CHGUI* t_N=(new bb_challengergui_CHGUI)->g_new();
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<516>");
	gc_assign(t_N->f_Parent,t_Dropdown);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<518>");
	t_N->f_Value=Float(t_Value);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<519>");
	t_N->f_Text=t_Text;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<520>");
	t_N->f_Element=String(L"DropdownItem",12);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<522>");
	gc_assign(t_N->f_Parent->f_DropdownItems,t_N->f_Parent->f_DropdownItems.Resize(t_N->f_Parent->f_DropdownItems.Length()+1));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<523>");
	gc_assign(t_N->f_Parent->f_DropdownItems.At(t_N->f_Parent->f_DropdownItems.Length()-1),t_N);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<524>");
	return t_N;
}
int bb_protocol__readp(){
	DBG_ENTER("_readp")
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<36>");
	do{
		DBG_BLOCK();
		DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<37>");
		String t_Dat=bb_data2_Server->m_ReadLine();
		DBG_LOCAL(t_Dat,"Dat")
		DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<38>");
		if(t_Dat==String(L"__server__protocol__",20)){
			DBG_BLOCK();
			DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<39>");
			break;
		}
	}while(!(false));
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<42>");
	if((bb_data2_Server->m_ReadAvail())!=0){
		DBG_BLOCK();
		DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<43>");
		int t_Kind=(bb_data2_Server->m_ReadLine()).ToInt();
		DBG_LOCAL(t_Kind,"Kind")
		DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<44>");
		int t_=t_Kind;
		DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<45>");
		if(t_==4){
			DBG_BLOCK();
			DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<46>");
			int t_am=(bb_data2_Server->m_ReadLine()).ToInt();
			DBG_LOCAL(t_am,"am")
			DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<47>");
			for(int t_es=0;t_es<=t_am-1;t_es=t_es+1){
				DBG_BLOCK();
				DBG_LOCAL(t_es,"es")
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<48>");
				bb_challengergui_CreateDropdownItem(bb_data2_Server->m_ReadLine(),bb_data2_Game->f_Games,0);
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<50>");
			if(t_==5){
				DBG_BLOCK();
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<51>");
				int t_am2=(bb_data2_Server->m_ReadLine()).ToInt();
				DBG_LOCAL(t_am2,"am")
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<52>");
				for(int t_es2=0;t_es2<=t_am2-1;t_es2=t_es2+1){
					DBG_BLOCK();
					DBG_LOCAL(t_es2,"es")
					DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<53>");
					bb_challengergui_CreateDropdownItem(bb_data2_Server->m_ReadLine(),bb_data2_Game->f_BeaconList,0);
				}
			}
		}
	}
	return 0;
}
int bb_protocol_ReadProtocol(){
	DBG_ENTER("ReadProtocol")
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<23>");
	if(bb_data2_Server!=0){
		DBG_BLOCK();
		DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<24>");
		if((bb_data2_Server->m_ReadAvail())!=0){
			DBG_BLOCK();
			DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<25>");
			while((bb_data2_Server->m_ReadAvail())!=0){
				DBG_BLOCK();
				DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<26>");
				bb_protocol__readp();
			}
			DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<28>");
			bb_data2_Server->m_Close();
			DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<29>");
			bb_data2_Server=0;
		}
	}
	return 0;
}
int bb_graphics_DebugRenderDevice(){
	DBG_ENTER("DebugRenderDevice")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<48>");
	if(!((bb_graphics_renderDevice)!=0)){
		DBG_BLOCK();
		Error(String(L"Rendering operations can only be performed inside OnRender",58));
	}
	return 0;
}
int bb_graphics_Cls(Float t_r,Float t_g,Float t_b){
	DBG_ENTER("Cls")
	DBG_LOCAL(t_r,"r")
	DBG_LOCAL(t_g,"g")
	DBG_LOCAL(t_b,"b")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<354>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<356>");
	bb_graphics_renderDevice->Cls(t_r,t_g,t_b);
	return 0;
}
Array<bb_challengergui_CHGUI* > bb_challengergui_CHGUI_BottomList;
bb_challengergui_CHGUI* bb_challengergui_CHGUI_Canvas;
int bb_challengergui_CHGUI_RealVisible(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_RealVisible")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3921>");
	bb_challengergui_CHGUI* t_E=0;
	DBG_LOCAL(t_E,"E")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3922>");
	t_E=t_N;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3923>");
	int t_V=t_N->f_Visible;
	DBG_LOCAL(t_V,"V")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3924>");
	if(t_V==0){
		DBG_BLOCK();
		return t_V;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3925>");
	do{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3926>");
		if(t_E->f_Parent!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3927>");
			t_V=t_E->f_Parent->f_Visible;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3928>");
			if(t_V==0){
				DBG_BLOCK();
				break;
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3929>");
			t_E=t_E->f_Parent;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3931>");
			break;
		}
	}while(!(false));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3934>");
	if(bb_challengergui_CHGUI_Canvas->f_Visible==0){
		DBG_BLOCK();
		t_V=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3935>");
	return t_V;
}
int bb_challengergui_CHGUI_RealMinimised(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_RealMinimised")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3962>");
	bb_challengergui_CHGUI* t_E=0;
	DBG_LOCAL(t_E,"E")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3963>");
	t_E=t_N;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3964>");
	int t_M=t_E->f_Minimised;
	DBG_LOCAL(t_M,"M")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3965>");
	if(t_M==1){
		DBG_BLOCK();
		return t_M;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3966>");
	do{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3967>");
		if(t_E->f_Parent!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3968>");
			t_M=t_E->f_Parent->f_Minimised;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3969>");
			if(t_M==1){
				DBG_BLOCK();
				break;
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3970>");
			t_E=t_E->f_Parent;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3972>");
			break;
		}
	}while(!(false));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3975>");
	return t_M;
}
Float bb_challengergui_CHGUI_OffsetX;
int bb_challengergui_CHGUI_RealX(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_RealX")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3881>");
	bb_challengergui_CHGUI* t_E=0;
	DBG_LOCAL(t_E,"E")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3882>");
	t_E=t_N;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3883>");
	int t_X=int(t_N->f_X);
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3884>");
	do{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3885>");
		if(t_E->f_Parent!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3886>");
			if(t_E->f_Parent->f_Element!=String(L"Tab",3)){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3887>");
				t_X=int(Float(t_X)+t_E->f_Parent->f_X);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3888>");
				t_E=t_E->f_Parent;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3890>");
				t_E=t_E->f_Parent;
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3893>");
			break;
		}
	}while(!(false));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3896>");
	t_X=int(Float(t_X)+bb_challengergui_CHGUI_OffsetX);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3897>");
	return t_X;
}
Float bb_challengergui_CHGUI_OffsetY;
int bb_challengergui_CHGUI_RealY(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_RealY")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3901>");
	bb_challengergui_CHGUI* t_E=0;
	DBG_LOCAL(t_E,"E")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3902>");
	t_E=t_N;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3903>");
	int t_Y=int(t_N->f_Y);
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3904>");
	do{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3905>");
		if(t_E->f_Parent!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3906>");
			if(t_E->f_Parent->f_Element!=String(L"Tab",3)){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3907>");
				t_Y=int(Float(t_Y)+t_E->f_Parent->f_Y);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3908>");
				t_E=t_E->f_Parent;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3910>");
				t_E=t_E->f_Parent;
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3913>");
			break;
		}
	}while(!(false));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3916>");
	t_Y=int(Float(t_Y)+bb_challengergui_CHGUI_OffsetY);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3917>");
	return t_Y;
}
Float bb_challengergui_CHGUI_TitleHeight;
bb_challengergui_CHGUI* bb_challengergui_CHGUI_LockedWIndow;
int bb_challengergui_CHGUI_RealActive(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_RealActive")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3939>");
	bb_challengergui_CHGUI* t_E=0;
	DBG_LOCAL(t_E,"E")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3940>");
	t_E=t_N;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3941>");
	int t_A=t_N->f_Active;
	DBG_LOCAL(t_A,"A")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3942>");
	if(t_A==0){
		DBG_BLOCK();
		return t_A;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3943>");
	do{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3944>");
		if(t_E==bb_challengergui_CHGUI_LockedWIndow){
			DBG_BLOCK();
			return 1;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3945>");
		if(t_E->f_Parent!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3946>");
			if(t_E->f_Parent==bb_challengergui_CHGUI_LockedWIndow){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3947>");
				return 1;
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3949>");
			t_A=t_E->f_Parent->f_Active;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3950>");
			if(t_A==0){
				DBG_BLOCK();
				break;
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3952>");
			t_E=t_E->f_Parent;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3954>");
			break;
		}
	}while(!(false));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3958>");
	return t_A;
}
int bb_challengergui_CHGUI_Shadow;
bb_graphics_Image* bb_challengergui_CHGUI_ShadowImg;
int bb_graphics_PushMatrix(){
	DBG_ENTER("PushMatrix")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<310>");
	int t_sp=bb_graphics_context->f_matrixSp;
	DBG_LOCAL(t_sp,"sp")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<311>");
	bb_graphics_context->f_matrixStack.At(t_sp+0)=bb_graphics_context->f_ix;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<312>");
	bb_graphics_context->f_matrixStack.At(t_sp+1)=bb_graphics_context->f_iy;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<313>");
	bb_graphics_context->f_matrixStack.At(t_sp+2)=bb_graphics_context->f_jx;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<314>");
	bb_graphics_context->f_matrixStack.At(t_sp+3)=bb_graphics_context->f_jy;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<315>");
	bb_graphics_context->f_matrixStack.At(t_sp+4)=bb_graphics_context->f_tx;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<316>");
	bb_graphics_context->f_matrixStack.At(t_sp+5)=bb_graphics_context->f_ty;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<317>");
	bb_graphics_context->f_matrixSp=t_sp+6;
	return 0;
}
int bb_graphics_Transform(Float t_ix,Float t_iy,Float t_jx,Float t_jy,Float t_tx,Float t_ty){
	DBG_ENTER("Transform")
	DBG_LOCAL(t_ix,"ix")
	DBG_LOCAL(t_iy,"iy")
	DBG_LOCAL(t_jx,"jx")
	DBG_LOCAL(t_jy,"jy")
	DBG_LOCAL(t_tx,"tx")
	DBG_LOCAL(t_ty,"ty")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<331>");
	Float t_ix2=t_ix*bb_graphics_context->f_ix+t_iy*bb_graphics_context->f_jx;
	DBG_LOCAL(t_ix2,"ix2")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<332>");
	Float t_iy2=t_ix*bb_graphics_context->f_iy+t_iy*bb_graphics_context->f_jy;
	DBG_LOCAL(t_iy2,"iy2")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<333>");
	Float t_jx2=t_jx*bb_graphics_context->f_ix+t_jy*bb_graphics_context->f_jx;
	DBG_LOCAL(t_jx2,"jx2")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<334>");
	Float t_jy2=t_jx*bb_graphics_context->f_iy+t_jy*bb_graphics_context->f_jy;
	DBG_LOCAL(t_jy2,"jy2")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<335>");
	Float t_tx2=t_tx*bb_graphics_context->f_ix+t_ty*bb_graphics_context->f_jx+bb_graphics_context->f_tx;
	DBG_LOCAL(t_tx2,"tx2")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<336>");
	Float t_ty2=t_tx*bb_graphics_context->f_iy+t_ty*bb_graphics_context->f_jy+bb_graphics_context->f_ty;
	DBG_LOCAL(t_ty2,"ty2")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<337>");
	bb_graphics_SetMatrix(t_ix2,t_iy2,t_jx2,t_jy2,t_tx2,t_ty2);
	return 0;
}
int bb_graphics_Transform2(Array<Float > t_m){
	DBG_ENTER("Transform")
	DBG_LOCAL(t_m,"m")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<327>");
	bb_graphics_Transform(t_m.At(0),t_m.At(1),t_m.At(2),t_m.At(3),t_m.At(4),t_m.At(5));
	return 0;
}
int bb_graphics_Translate(Float t_x,Float t_y){
	DBG_ENTER("Translate")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<341>");
	bb_graphics_Transform(FLOAT(1.0),FLOAT(0.0),FLOAT(0.0),FLOAT(1.0),t_x,t_y);
	return 0;
}
int bb_graphics_PopMatrix(){
	DBG_ENTER("PopMatrix")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<321>");
	int t_sp=bb_graphics_context->f_matrixSp-6;
	DBG_LOCAL(t_sp,"sp")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<322>");
	bb_graphics_SetMatrix(bb_graphics_context->f_matrixStack.At(t_sp+0),bb_graphics_context->f_matrixStack.At(t_sp+1),bb_graphics_context->f_matrixStack.At(t_sp+2),bb_graphics_context->f_matrixStack.At(t_sp+3),bb_graphics_context->f_matrixStack.At(t_sp+4),bb_graphics_context->f_matrixStack.At(t_sp+5));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<323>");
	bb_graphics_context->f_matrixSp=t_sp;
	return 0;
}
int bb_graphics_DrawImageRect(bb_graphics_Image* t_image,Float t_x,Float t_y,int t_srcX,int t_srcY,int t_srcWidth,int t_srcHeight,int t_frame){
	DBG_ENTER("DrawImageRect")
	DBG_LOCAL(t_image,"image")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_srcX,"srcX")
	DBG_LOCAL(t_srcY,"srcY")
	DBG_LOCAL(t_srcWidth,"srcWidth")
	DBG_LOCAL(t_srcHeight,"srcHeight")
	DBG_LOCAL(t_frame,"frame")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<473>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<475>");
	bb_graphics_Frame* t_f=t_image->f_frames.At(t_frame);
	DBG_LOCAL(t_f,"f")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<477>");
	if((bb_graphics_context->f_tformed)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<478>");
		bb_graphics_PushMatrix();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<481>");
		bb_graphics_Translate(-t_image->f_tx+t_x,-t_image->f_ty+t_y);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<483>");
		bb_graphics_context->m_Validate();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<485>");
		bb_graphics_renderDevice->DrawSurface2(t_image->f_surface,FLOAT(0.0),FLOAT(0.0),t_srcX+t_f->f_x,t_srcY+t_f->f_y,t_srcWidth,t_srcHeight);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<487>");
		bb_graphics_PopMatrix();
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<489>");
		bb_graphics_context->m_Validate();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<491>");
		bb_graphics_renderDevice->DrawSurface2(t_image->f_surface,-t_image->f_tx+t_x,-t_image->f_ty+t_y,t_srcX+t_f->f_x,t_srcY+t_f->f_y,t_srcWidth,t_srcHeight);
	}
	return 0;
}
int bb_graphics_Rotate(Float t_angle){
	DBG_ENTER("Rotate")
	DBG_LOCAL(t_angle,"angle")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<349>");
	bb_graphics_Transform((Float)cos((t_angle)*D2R),-(Float)sin((t_angle)*D2R),(Float)sin((t_angle)*D2R),(Float)cos((t_angle)*D2R),FLOAT(0.0),FLOAT(0.0));
	return 0;
}
int bb_graphics_Scale(Float t_x,Float t_y){
	DBG_ENTER("Scale")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<345>");
	bb_graphics_Transform(t_x,FLOAT(0.0),FLOAT(0.0),t_y,FLOAT(0.0),FLOAT(0.0));
	return 0;
}
int bb_graphics_DrawImageRect2(bb_graphics_Image* t_image,Float t_x,Float t_y,int t_srcX,int t_srcY,int t_srcWidth,int t_srcHeight,Float t_rotation,Float t_scaleX,Float t_scaleY,int t_frame){
	DBG_ENTER("DrawImageRect")
	DBG_LOCAL(t_image,"image")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_srcX,"srcX")
	DBG_LOCAL(t_srcY,"srcY")
	DBG_LOCAL(t_srcWidth,"srcWidth")
	DBG_LOCAL(t_srcHeight,"srcHeight")
	DBG_LOCAL(t_rotation,"rotation")
	DBG_LOCAL(t_scaleX,"scaleX")
	DBG_LOCAL(t_scaleY,"scaleY")
	DBG_LOCAL(t_frame,"frame")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<497>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<499>");
	bb_graphics_Frame* t_f=t_image->f_frames.At(t_frame);
	DBG_LOCAL(t_f,"f")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<501>");
	bb_graphics_PushMatrix();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<503>");
	bb_graphics_Translate(t_x,t_y);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<504>");
	bb_graphics_Rotate(t_rotation);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<505>");
	bb_graphics_Scale(t_scaleX,t_scaleY);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<506>");
	bb_graphics_Translate(-t_image->f_tx,-t_image->f_ty);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<508>");
	bb_graphics_context->m_Validate();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<510>");
	bb_graphics_renderDevice->DrawSurface2(t_image->f_surface,FLOAT(0.0),FLOAT(0.0),t_srcX+t_f->f_x,t_srcY+t_f->f_y,t_srcWidth,t_srcHeight);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<512>");
	bb_graphics_PopMatrix();
	return 0;
}
bb_graphics_Image* bb_challengergui_CHGUI_Style;
bb_bitmapfont_BitmapFont::bb_bitmapfont_BitmapFont(){
	f_faceChars=Array<bb_bitmapchar_BitMapChar* >();
	f__drawShadow=true;
	f_borderChars=Array<bb_bitmapchar_BitMapChar* >();
	f__kerning=0;
	f_packedImages=Array<bb_graphics_Image* >();
	f_shadowChars=Array<bb_bitmapchar_BitMapChar* >();
	f__drawBorder=true;
}
int bb_bitmapfont_BitmapFont::m_GetFontHeight(){
	DBG_ENTER("BitmapFont.GetFontHeight")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<263>");
	if(f_faceChars.At(32)==0){
		DBG_BLOCK();
		return 0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<264>");
	int t_=int(f_faceChars.At(32)->f_drawingMetrics->f_drawingSize->f_y);
	return t_;
}
bool bb_bitmapfont_BitmapFont::m_DrawShadow(){
	DBG_ENTER("BitmapFont.DrawShadow")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<66>");
	return f__drawShadow;
}
int bb_bitmapfont_BitmapFont::m_DrawShadow2(bool t_value){
	DBG_ENTER("BitmapFont.DrawShadow")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<70>");
	f__drawShadow=t_value;
	return 0;
}
bb_drawingpoint_DrawingPoint* bb_bitmapfont_BitmapFont::m_Kerning(){
	DBG_ENTER("BitmapFont.Kerning")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<667>");
	if(f__kerning==0){
		DBG_BLOCK();
		gc_assign(f__kerning,(new bb_drawingpoint_DrawingPoint)->g_new());
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<668>");
	return f__kerning;
}
void bb_bitmapfont_BitmapFont::m_Kerning2(bb_drawingpoint_DrawingPoint* t_value){
	DBG_ENTER("BitmapFont.Kerning")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<672>");
	gc_assign(f__kerning,t_value);
}
Float bb_bitmapfont_BitmapFont::m_GetTxtWidth(String t_text,int t_fromChar,int t_toChar){
	DBG_ENTER("BitmapFont.GetTxtWidth")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_text,"text")
	DBG_LOCAL(t_fromChar,"fromChar")
	DBG_LOCAL(t_toChar,"toChar")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<214>");
	Float t_twidth=FLOAT(.0);
	DBG_LOCAL(t_twidth,"twidth")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<215>");
	Float t_MaxWidth=FLOAT(0.0);
	DBG_LOCAL(t_MaxWidth,"MaxWidth")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<216>");
	int t_char=0;
	DBG_LOCAL(t_char,"char")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<217>");
	int t_lastchar=0;
	DBG_LOCAL(t_lastchar,"lastchar")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<219>");
	for(int t_i=t_fromChar;t_i<=t_toChar;t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<220>");
		t_char=(int)t_text[t_i-1];
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<221>");
		if(t_char>=0 && t_char<f_faceChars.Length() && t_char!=10 && t_char!=13){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<222>");
			if(f_faceChars.At(t_char)!=0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<223>");
				t_lastchar=t_char;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<224>");
				t_twidth=t_twidth+f_faceChars.At(t_char)->f_drawingMetrics->f_drawingWidth+m_Kerning()->f_x;
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<226>");
			if(t_char==10){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<227>");
				if(bb_math_Abs2(t_MaxWidth)<bb_math_Abs2(t_twidth)){
					DBG_BLOCK();
					t_MaxWidth=t_twidth-m_Kerning()->f_x-f_faceChars.At(t_lastchar)->f_drawingMetrics->f_drawingWidth+f_faceChars.At(t_lastchar)->f_drawingMetrics->f_drawingSize->f_x;
				}
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<228>");
				t_twidth=FLOAT(0.0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<229>");
				t_lastchar=t_char;
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<232>");
	if(t_lastchar>=0 && t_lastchar<f_faceChars.Length()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<233>");
		if(f_faceChars.At(t_lastchar)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<234>");
			t_twidth=t_twidth-f_faceChars.At(t_lastchar)->f_drawingMetrics->f_drawingWidth;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<235>");
			t_twidth=t_twidth+f_faceChars.At(t_lastchar)->f_drawingMetrics->f_drawingSize->f_x;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<238>");
	if(bb_math_Abs2(t_MaxWidth)<bb_math_Abs2(t_twidth)){
		DBG_BLOCK();
		t_MaxWidth=t_twidth-m_Kerning()->f_x;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<239>");
	return t_MaxWidth;
}
Float bb_bitmapfont_BitmapFont::m_GetTxtWidth2(String t_text){
	DBG_ENTER("BitmapFont.GetTxtWidth")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_text,"text")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<205>");
	Float t_=m_GetTxtWidth(t_text,1,t_text.Length());
	return t_;
}
int bb_bitmapfont_BitmapFont::m_DrawCharsText(String t_text,Float t_x,Float t_y,Array<bb_bitmapchar_BitMapChar* > t_target,int t_align,int t_startPos){
	DBG_ENTER("BitmapFont.DrawCharsText")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_text,"text")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_target,"target")
	DBG_LOCAL(t_align,"align")
	DBG_LOCAL(t_startPos,"startPos")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<619>");
	Float t_drx=t_x;
	Float t_dry=t_y;
	DBG_LOCAL(t_drx,"drx")
	DBG_LOCAL(t_dry,"dry")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<620>");
	Float t_oldX=t_x;
	DBG_LOCAL(t_oldX,"oldX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<621>");
	int t_xOffset=0;
	DBG_LOCAL(t_xOffset,"xOffset")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<624>");
	if(t_align!=1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<625>");
		int t_lineSepPos=t_text.Find(String(L"\n",1),t_startPos);
		DBG_LOCAL(t_lineSepPos,"lineSepPos")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<626>");
		if(t_lineSepPos<0){
			DBG_BLOCK();
			t_lineSepPos=t_text.Length();
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<627>");
		int t_=t_align;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<628>");
		if(t_==2){
			DBG_BLOCK();
			t_xOffset=int(this->m_GetTxtWidth(t_text,t_startPos,t_lineSepPos)/FLOAT(2.0));
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<629>");
			if(t_==3){
				DBG_BLOCK();
				t_xOffset=int(this->m_GetTxtWidth(t_text,t_startPos,t_lineSepPos));
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<633>");
	for(int t_i=t_startPos;t_i<=t_text.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<634>");
		int t_char=(int)t_text[t_i-1];
		DBG_LOCAL(t_char,"char")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<635>");
		if(t_char>=0 && t_char<=t_target.Length()){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<636>");
			if(t_char==10){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<637>");
				t_dry+=f_faceChars.At(32)->f_drawingMetrics->f_drawingSize->f_y+m_Kerning()->f_y;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<638>");
				this->m_DrawCharsText(t_text,t_oldX,t_dry,t_target,t_align,t_i+1);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<639>");
				return 0;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<640>");
				if(t_target.At(t_char)!=0){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<641>");
					if(t_target.At(t_char)->m_CharImageLoaded()==false){
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<642>");
						t_target.At(t_char)->m_LoadCharImage();
					}
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<644>");
					if(t_target.At(t_char)->f_image!=0){
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<645>");
						bb_graphics_DrawImage(t_target.At(t_char)->f_image,t_drx-Float(t_xOffset),t_dry,0);
					}else{
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<646>");
						if(t_target.At(t_char)->f_packedFontIndex>0){
							DBG_BLOCK();
							DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<647>");
							bb_graphics_DrawImageRect(f_packedImages.At(t_target.At(t_char)->f_packedFontIndex),Float(-t_xOffset)+t_drx+t_target.At(t_char)->f_drawingMetrics->f_drawingOffset->f_x,t_dry+t_target.At(t_char)->f_drawingMetrics->f_drawingOffset->f_y,int(t_target.At(t_char)->f_packedPosition->f_x),int(t_target.At(t_char)->f_packedPosition->f_y),int(t_target.At(t_char)->f_packedSize->f_x),int(t_target.At(t_char)->f_packedSize->f_y),0);
						}
					}
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<649>");
					t_drx+=f_faceChars.At(t_char)->f_drawingMetrics->f_drawingWidth+m_Kerning()->f_x;
				}
			}
		}
	}
	return 0;
}
int bb_bitmapfont_BitmapFont::m_DrawCharsText2(String t_text,Float t_x,Float t_y,int t_mode,int t_align){
	DBG_ENTER("BitmapFont.DrawCharsText")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_text,"text")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_mode,"mode")
	DBG_LOCAL(t_align,"align")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<606>");
	if(t_mode==1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<607>");
		m_DrawCharsText(t_text,t_x,t_y,f_borderChars,t_align,1);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<608>");
		if(t_mode==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<609>");
			m_DrawCharsText(t_text,t_x,t_y,f_faceChars,t_align,1);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<611>");
			m_DrawCharsText(t_text,t_x,t_y,f_shadowChars,t_align,1);
		}
	}
	return 0;
}
bool bb_bitmapfont_BitmapFont::m_DrawBorder(){
	DBG_ENTER("BitmapFont.DrawBorder")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<79>");
	return f__drawBorder;
}
int bb_bitmapfont_BitmapFont::m_DrawBorder2(bool t_value){
	DBG_ENTER("BitmapFont.DrawBorder")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<83>");
	f__drawBorder=t_value;
	return 0;
}
int bb_bitmapfont_BitmapFont::m_DrawText(String t_text,Float t_x,Float t_y,int t_align){
	DBG_ENTER("BitmapFont.DrawText")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_text,"text")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_align,"align")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<187>");
	if(m_DrawShadow()){
		DBG_BLOCK();
		m_DrawCharsText2(t_text,t_x,t_y,2,t_align);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<188>");
	if(m_DrawBorder()){
		DBG_BLOCK();
		m_DrawCharsText2(t_text,t_x,t_y,1,t_align);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<189>");
	m_DrawCharsText2(t_text,t_x,t_y,0,t_align);
	return 0;
}
int bb_bitmapfont_BitmapFont::m_DrawText2(String t_text,Float t_x,Float t_y){
	DBG_ENTER("BitmapFont.DrawText")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_text,"text")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<197>");
	this->m_DrawText(t_text,t_x,t_y,1);
	return 0;
}
Float bb_bitmapfont_BitmapFont::m_GetTxtHeight(String t_Text){
	DBG_ENTER("BitmapFont.GetTxtHeight")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_Text,"Text")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<250>");
	int t_count=0;
	DBG_LOCAL(t_count,"count")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<251>");
	for(int t_i=0;t_i<t_Text.Length();t_i=t_i+1){
		DBG_BLOCK();
		DBG_LOCAL(t_i,"i")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<252>");
		if((int)t_Text[t_i]==10){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<253>");
			t_count+=1;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<256>");
	Float t_=Float(t_count)*(f_faceChars.At(32)->f_drawingMetrics->f_drawingSize->f_y+m_Kerning()->f_y)+Float(m_GetFontHeight());
	return t_;
}
int bb_bitmapfont_BitmapFont::m_LoadPacked(String t_info,String t_fontName,bool t_dynamicLoad){
	DBG_ENTER("BitmapFont.LoadPacked")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_info,"info")
	DBG_LOCAL(t_fontName,"fontName")
	DBG_LOCAL(t_dynamicLoad,"dynamicLoad")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<542>");
	String t_header=t_info.Slice(0,t_info.Find(String(L",",1),0));
	DBG_LOCAL(t_header,"header")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<544>");
	String t_separator=String();
	DBG_LOCAL(t_separator,"separator")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<545>");
	String t_=t_header;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<546>");
	if(t_==String(L"P1",2)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<547>");
		t_separator=String(L".",1);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<548>");
		if(t_==String(L"P1.01",5)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<549>");
			t_separator=String(L"_P_",3);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<551>");
	t_info=t_info.Slice(t_info.Find(String(L",",1),0)+1);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<552>");
	gc_assign(f_borderChars,Array<bb_bitmapchar_BitMapChar* >(65536));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<553>");
	gc_assign(f_faceChars,Array<bb_bitmapchar_BitMapChar* >(65536));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<554>");
	gc_assign(f_shadowChars,Array<bb_bitmapchar_BitMapChar* >(65536));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<555>");
	gc_assign(f_packedImages,Array<bb_graphics_Image* >(256));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<556>");
	int t_maxPacked=0;
	DBG_LOCAL(t_maxPacked,"maxPacked")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<557>");
	int t_maxChar=0;
	DBG_LOCAL(t_maxChar,"maxChar")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<559>");
	String t_prefixName=t_fontName;
	DBG_LOCAL(t_prefixName,"prefixName")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<560>");
	if(t_prefixName.ToLower().EndsWith(String(L".txt",4))){
		DBG_BLOCK();
		t_prefixName=t_prefixName.Slice(0,-4);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<562>");
	Array<String > t_charList=t_info.Split(String(L";",1));
	DBG_LOCAL(t_charList,"charList")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<563>");
	Array<String > t_2=t_charList;
	int t_3=0;
	while(t_3<t_2.Length()){
		DBG_BLOCK();
		String t_chr=t_2.At(t_3);
		t_3=t_3+1;
		DBG_LOCAL(t_chr,"chr")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<565>");
		Array<String > t_chrdata=t_chr.Split(String(L",",1));
		DBG_LOCAL(t_chrdata,"chrdata")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<566>");
		if(t_chrdata.Length()<2){
			DBG_BLOCK();
			break;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<567>");
		bb_bitmapchar_BitMapChar* t_char=0;
		DBG_LOCAL(t_char,"char")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<568>");
		int t_charIndex=(t_chrdata.At(0)).ToInt();
		DBG_LOCAL(t_charIndex,"charIndex")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<569>");
		if(t_maxChar<t_charIndex){
			DBG_BLOCK();
			t_maxChar=t_charIndex;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<571>");
		String t_4=t_chrdata.At(1);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<572>");
		if(t_4==String(L"B",1)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<573>");
			gc_assign(f_borderChars.At(t_charIndex),(new bb_bitmapchar_BitMapChar)->g_new());
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<574>");
			t_char=f_borderChars.At(t_charIndex);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<575>");
			if(t_4==String(L"F",1)){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<576>");
				gc_assign(f_faceChars.At(t_charIndex),(new bb_bitmapchar_BitMapChar)->g_new());
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<577>");
				t_char=f_faceChars.At(t_charIndex);
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<578>");
				if(t_4==String(L"S",1)){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<579>");
					gc_assign(f_shadowChars.At(t_charIndex),(new bb_bitmapchar_BitMapChar)->g_new());
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<580>");
					t_char=f_shadowChars.At(t_charIndex);
				}
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<582>");
		t_char->f_packedFontIndex=(t_chrdata.At(2)).ToInt();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<583>");
		if(f_packedImages.At(t_char->f_packedFontIndex)==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<584>");
			gc_assign(f_packedImages.At(t_char->f_packedFontIndex),bb_graphics_LoadImage(t_prefixName+t_separator+String(t_char->f_packedFontIndex)+String(L".png",4),1,bb_graphics_Image::g_DefaultFlags));
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<585>");
			if(t_maxPacked<t_char->f_packedFontIndex){
				DBG_BLOCK();
				t_maxPacked=t_char->f_packedFontIndex;
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<587>");
		t_char->f_packedPosition->f_x=Float((t_chrdata.At(3)).ToInt());
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<588>");
		t_char->f_packedPosition->f_y=Float((t_chrdata.At(4)).ToInt());
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<589>");
		t_char->f_packedSize->f_x=Float((t_chrdata.At(5)).ToInt());
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<590>");
		t_char->f_packedSize->f_y=Float((t_chrdata.At(6)).ToInt());
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<591>");
		t_char->f_drawingMetrics->f_drawingOffset->f_x=Float((t_chrdata.At(8)).ToInt());
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<592>");
		t_char->f_drawingMetrics->f_drawingOffset->f_y=Float((t_chrdata.At(9)).ToInt());
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<593>");
		t_char->f_drawingMetrics->f_drawingSize->f_x=Float((t_chrdata.At(10)).ToInt());
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<594>");
		t_char->f_drawingMetrics->f_drawingSize->f_y=Float((t_chrdata.At(11)).ToInt());
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<595>");
		t_char->f_drawingMetrics->f_drawingWidth=Float((t_chrdata.At(12)).ToInt());
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<598>");
	gc_assign(f_borderChars,f_borderChars.Slice(0,t_maxChar+1));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<599>");
	gc_assign(f_faceChars,f_faceChars.Slice(0,t_maxChar+1));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<600>");
	gc_assign(f_shadowChars,f_shadowChars.Slice(0,t_maxChar+1));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<601>");
	gc_assign(f_packedImages,f_packedImages.Slice(0,t_maxPacked+1));
	return 0;
}
int bb_bitmapfont_BitmapFont::m_LoadFontData(String t_Info,String t_fontName,bool t_dynamicLoad){
	DBG_ENTER("BitmapFont.LoadFontData")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_Info,"Info")
	DBG_LOCAL(t_fontName,"fontName")
	DBG_LOCAL(t_dynamicLoad,"dynamicLoad")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<440>");
	if(t_Info.StartsWith(String(L"P1",2))){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<441>");
		m_LoadPacked(t_Info,t_fontName,t_dynamicLoad);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<442>");
		return 0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<444>");
	Array<String > t_tokenStream=t_Info.Split(String(L",",1));
	DBG_LOCAL(t_tokenStream,"tokenStream")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<445>");
	int t_index=0;
	DBG_LOCAL(t_index,"index")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<446>");
	gc_assign(f_borderChars,Array<bb_bitmapchar_BitMapChar* >(65536));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<447>");
	gc_assign(f_faceChars,Array<bb_bitmapchar_BitMapChar* >(65536));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<448>");
	gc_assign(f_shadowChars,Array<bb_bitmapchar_BitMapChar* >(65536));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<450>");
	String t_prefixName=t_fontName;
	DBG_LOCAL(t_prefixName,"prefixName")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<451>");
	if(t_prefixName.ToLower().EndsWith(String(L".txt",4))){
		DBG_BLOCK();
		t_prefixName=t_prefixName.Slice(0,-4);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<453>");
	int t_char=0;
	DBG_LOCAL(t_char,"char")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<454>");
	while(t_index<t_tokenStream.Length()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<456>");
		String t_strChar=t_tokenStream.At(t_index);
		DBG_LOCAL(t_strChar,"strChar")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<457>");
		if(t_strChar.Trim()==String()){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<459>");
			t_index+=1;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<460>");
			break;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<462>");
		t_char=(t_strChar).ToInt();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<464>");
		t_index+=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<466>");
		String t_kind=t_tokenStream.At(t_index);
		DBG_LOCAL(t_kind,"kind")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<468>");
		t_index+=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<470>");
		String t_=t_kind;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<471>");
		if(t_==String(L"{BR",3)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<472>");
			t_index+=3;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<473>");
			gc_assign(f_borderChars.At(t_char),(new bb_bitmapchar_BitMapChar)->g_new());
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<474>");
			f_borderChars.At(t_char)->f_drawingMetrics->f_drawingOffset->f_x=Float((t_tokenStream.At(t_index)).ToInt());
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<475>");
			f_borderChars.At(t_char)->f_drawingMetrics->f_drawingOffset->f_y=Float((t_tokenStream.At(t_index+1)).ToInt());
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<476>");
			f_borderChars.At(t_char)->f_drawingMetrics->f_drawingSize->f_x=Float((t_tokenStream.At(t_index+2)).ToInt());
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<477>");
			f_borderChars.At(t_char)->f_drawingMetrics->f_drawingSize->f_y=Float((t_tokenStream.At(t_index+3)).ToInt());
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<478>");
			f_borderChars.At(t_char)->f_drawingMetrics->f_drawingWidth=Float((t_tokenStream.At(t_index+4)).ToInt());
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<479>");
			if(t_dynamicLoad==false){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<480>");
				gc_assign(f_borderChars.At(t_char)->f_image,bb_graphics_LoadImage(t_prefixName+String(L"_BORDER_",8)+String(t_char)+String(L".png",4),1,bb_graphics_Image::g_DefaultFlags));
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<481>");
				f_borderChars.At(t_char)->f_image->m_SetHandle(-f_borderChars.At(t_char)->f_drawingMetrics->f_drawingOffset->f_x,-f_borderChars.At(t_char)->f_drawingMetrics->f_drawingOffset->f_y);
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<483>");
				f_borderChars.At(t_char)->m_SetImageResourceName(t_prefixName+String(L"_BORDER_",8)+String(t_char)+String(L".png",4));
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<485>");
			t_index+=5;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<486>");
			t_index+=1;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<488>");
			if(t_==String(L"{SH",3)){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<489>");
				t_index+=3;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<490>");
				gc_assign(f_shadowChars.At(t_char),(new bb_bitmapchar_BitMapChar)->g_new());
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<491>");
				f_shadowChars.At(t_char)->f_drawingMetrics->f_drawingOffset->f_x=Float((t_tokenStream.At(t_index)).ToInt());
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<492>");
				f_shadowChars.At(t_char)->f_drawingMetrics->f_drawingOffset->f_y=Float((t_tokenStream.At(t_index+1)).ToInt());
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<493>");
				f_shadowChars.At(t_char)->f_drawingMetrics->f_drawingSize->f_x=Float((t_tokenStream.At(t_index+2)).ToInt());
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<494>");
				f_shadowChars.At(t_char)->f_drawingMetrics->f_drawingSize->f_y=Float((t_tokenStream.At(t_index+3)).ToInt());
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<495>");
				f_shadowChars.At(t_char)->f_drawingMetrics->f_drawingWidth=Float((t_tokenStream.At(t_index+4)).ToInt());
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<496>");
				String t_filename=t_prefixName+String(L"_SHADOW_",8)+String(t_char)+String(L".png",4);
				DBG_LOCAL(t_filename,"filename")
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<497>");
				if(t_dynamicLoad==false){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<498>");
					gc_assign(f_shadowChars.At(t_char)->f_image,bb_graphics_LoadImage(t_filename,1,bb_graphics_Image::g_DefaultFlags));
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<499>");
					f_shadowChars.At(t_char)->f_image->m_SetHandle(-f_shadowChars.At(t_char)->f_drawingMetrics->f_drawingOffset->f_x,-f_shadowChars.At(t_char)->f_drawingMetrics->f_drawingOffset->f_y);
				}else{
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<501>");
					f_shadowChars.At(t_char)->m_SetImageResourceName(t_filename);
				}
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<508>");
				t_index+=5;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<509>");
				t_index+=1;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<511>");
				if(t_==String(L"{FC",3)){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<512>");
					t_index+=3;
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<513>");
					gc_assign(f_faceChars.At(t_char),(new bb_bitmapchar_BitMapChar)->g_new());
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<514>");
					f_faceChars.At(t_char)->f_drawingMetrics->f_drawingOffset->f_x=Float((t_tokenStream.At(t_index)).ToInt());
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<515>");
					f_faceChars.At(t_char)->f_drawingMetrics->f_drawingOffset->f_y=Float((t_tokenStream.At(t_index+1)).ToInt());
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<516>");
					f_faceChars.At(t_char)->f_drawingMetrics->f_drawingSize->f_x=Float((t_tokenStream.At(t_index+2)).ToInt());
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<517>");
					f_faceChars.At(t_char)->f_drawingMetrics->f_drawingSize->f_y=Float((t_tokenStream.At(t_index+3)).ToInt());
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<518>");
					f_faceChars.At(t_char)->f_drawingMetrics->f_drawingWidth=Float((t_tokenStream.At(t_index+4)).ToInt());
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<519>");
					if(t_dynamicLoad==false){
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<520>");
						gc_assign(f_faceChars.At(t_char)->f_image,bb_graphics_LoadImage(t_prefixName+String(L"_",1)+String(t_char)+String(L".png",4),1,bb_graphics_Image::g_DefaultFlags));
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<521>");
						f_faceChars.At(t_char)->f_image->m_SetHandle(-f_faceChars.At(t_char)->f_drawingMetrics->f_drawingOffset->f_x,-f_faceChars.At(t_char)->f_drawingMetrics->f_drawingOffset->f_y);
					}else{
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<523>");
						f_faceChars.At(t_char)->m_SetImageResourceName(t_prefixName+String(L"_",1)+String(t_char)+String(L".png",4));
					}
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<525>");
					t_index+=5;
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<526>");
					t_index+=1;
				}else{
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<529>");
					Print(String(L"Error loading font! Char = ",27)+String(t_char));
				}
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<533>");
	gc_assign(f_borderChars,f_borderChars.Slice(0,t_char+1));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<534>");
	gc_assign(f_faceChars,f_faceChars.Slice(0,t_char+1));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<535>");
	gc_assign(f_shadowChars,f_shadowChars.Slice(0,t_char+1));
	return 0;
}
bb_bitmapfont_BitmapFont* bb_bitmapfont_BitmapFont::g_new(String t_fontDescriptionFilePath,bool t_dynamicLoad){
	DBG_ENTER("BitmapFont.new")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_fontDescriptionFilePath,"fontDescriptionFilePath")
	DBG_LOCAL(t_dynamicLoad,"dynamicLoad")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<45>");
	String t_text=bb_app_LoadString(t_fontDescriptionFilePath);
	DBG_LOCAL(t_text,"text")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<46>");
	if(t_text==String()){
		DBG_BLOCK();
		Print(String(L"FONT ",5)+t_fontDescriptionFilePath+String(L" WAS NOT FOUND!!!",17));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<47>");
	m_LoadFontData(t_text,t_fontDescriptionFilePath,t_dynamicLoad);
	return this;
}
bb_bitmapfont_BitmapFont* bb_bitmapfont_BitmapFont::g_new2(String t_fontDescriptionFilePath){
	DBG_ENTER("BitmapFont.new")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_fontDescriptionFilePath,"fontDescriptionFilePath")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<55>");
	String t_text=bb_app_LoadString(t_fontDescriptionFilePath);
	DBG_LOCAL(t_text,"text")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<56>");
	if(t_text==String()){
		DBG_BLOCK();
		Print(String(L"FONT ",5)+t_fontDescriptionFilePath+String(L" WAS NOT FOUND!!!",17));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<57>");
	m_LoadFontData(t_text,t_fontDescriptionFilePath,true);
	return this;
}
bb_bitmapfont_BitmapFont* bb_bitmapfont_BitmapFont::g_new3(){
	DBG_ENTER("BitmapFont.new")
	bb_bitmapfont_BitmapFont *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<28>");
	return this;
}
bb_bitmapfont_BitmapFont* bb_bitmapfont_BitmapFont::g_Load(String t_fontName,bool t_dynamicLoad){
	DBG_ENTER("BitmapFont.Load")
	DBG_LOCAL(t_fontName,"fontName")
	DBG_LOCAL(t_dynamicLoad,"dynamicLoad")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<35>");
	bb_bitmapfont_BitmapFont* t_font=(new bb_bitmapfont_BitmapFont)->g_new(t_fontName,t_dynamicLoad);
	DBG_LOCAL(t_font,"font")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapfont.monkey<36>");
	return t_font;
}
void bb_bitmapfont_BitmapFont::mark(){
	Object::mark();
	gc_mark_q(f_faceChars);
	gc_mark_q(f_borderChars);
	gc_mark_q(f__kerning);
	gc_mark_q(f_packedImages);
	gc_mark_q(f_shadowChars);
}
String bb_bitmapfont_BitmapFont::debug(){
	String t="(BitmapFont)\n";
	t+=dbg_decl("_drawShadow",&f__drawShadow);
	t+=dbg_decl("_drawBorder",&f__drawBorder);
	t+=dbg_decl("borderChars",&f_borderChars);
	t+=dbg_decl("faceChars",&f_faceChars);
	t+=dbg_decl("shadowChars",&f_shadowChars);
	t+=dbg_decl("packedImages",&f_packedImages);
	t+=dbg_decl("_kerning",&f__kerning);
	return t;
}
bb_bitmapfont_BitmapFont* bb_challengergui_CHGUI_TitleFont;
bb_bitmapchar_BitMapChar::bb_bitmapchar_BitMapChar(){
	f_drawingMetrics=(new bb_bitmapcharmetrics_BitMapCharMetrics)->g_new();
	f_image=0;
	f_imageResourceName=String();
	f_imageResourceNameBackup=String();
	f_packedFontIndex=0;
	f_packedPosition=(new bb_drawingpoint_DrawingPoint)->g_new();
	f_packedSize=(new bb_drawingpoint_DrawingPoint)->g_new();
}
bool bb_bitmapchar_BitMapChar::m_CharImageLoaded(){
	DBG_ENTER("BitMapChar.CharImageLoaded")
	bb_bitmapchar_BitMapChar *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<42>");
	if(f_image==0 && f_imageResourceName!=String()){
		DBG_BLOCK();
		return false;
	}else{
		DBG_BLOCK();
		return true;
	}
}
int bb_bitmapchar_BitMapChar::m_LoadCharImage(){
	DBG_ENTER("BitMapChar.LoadCharImage")
	bb_bitmapchar_BitMapChar *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<30>");
	if(m_CharImageLoaded()==false){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<31>");
		gc_assign(f_image,bb_graphics_LoadImage(f_imageResourceName,1,bb_graphics_Image::g_DefaultFlags));
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<32>");
		f_image->m_SetHandle(-this->f_drawingMetrics->f_drawingOffset->f_x,-this->f_drawingMetrics->f_drawingOffset->f_y);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<33>");
		f_imageResourceNameBackup=f_imageResourceName;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<34>");
		f_imageResourceName=String();
	}
	return 0;
}
bb_bitmapchar_BitMapChar* bb_bitmapchar_BitMapChar::g_new(){
	DBG_ENTER("BitMapChar.new")
	bb_bitmapchar_BitMapChar *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<15>");
	return this;
}
int bb_bitmapchar_BitMapChar::m_SetImageResourceName(String t_value){
	DBG_ENTER("BitMapChar.SetImageResourceName")
	bb_bitmapchar_BitMapChar *self=this;
	DBG_LOCAL(self,"Self")
	DBG_LOCAL(t_value,"value")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapchar.monkey<46>");
	f_imageResourceName=t_value;
	return 0;
}
void bb_bitmapchar_BitMapChar::mark(){
	Object::mark();
	gc_mark_q(f_drawingMetrics);
	gc_mark_q(f_image);
	gc_mark_q(f_packedPosition);
	gc_mark_q(f_packedSize);
}
String bb_bitmapchar_BitMapChar::debug(){
	String t="(BitMapChar)\n";
	t+=dbg_decl("drawingMetrics",&f_drawingMetrics);
	t+=dbg_decl("image",&f_image);
	t+=dbg_decl("packedFontIndex",&f_packedFontIndex);
	t+=dbg_decl("packedPosition",&f_packedPosition);
	t+=dbg_decl("packedSize",&f_packedSize);
	t+=dbg_decl("imageResourceNameBackup",&f_imageResourceNameBackup);
	t+=dbg_decl("imageResourceName",&f_imageResourceName);
	return t;
}
bb_bitmapcharmetrics_BitMapCharMetrics::bb_bitmapcharmetrics_BitMapCharMetrics(){
	f_drawingSize=(new bb_drawingpoint_DrawingPoint)->g_new();
	f_drawingWidth=FLOAT(.0);
	f_drawingOffset=(new bb_drawingpoint_DrawingPoint)->g_new();
}
bb_bitmapcharmetrics_BitMapCharMetrics* bb_bitmapcharmetrics_BitMapCharMetrics::g_new(){
	DBG_ENTER("BitMapCharMetrics.new")
	bb_bitmapcharmetrics_BitMapCharMetrics *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/bitmapcharmetrics.monkey<12>");
	return this;
}
void bb_bitmapcharmetrics_BitMapCharMetrics::mark(){
	Object::mark();
	gc_mark_q(f_drawingSize);
	gc_mark_q(f_drawingOffset);
}
String bb_bitmapcharmetrics_BitMapCharMetrics::debug(){
	String t="(BitMapCharMetrics)\n";
	t+=dbg_decl("drawingOffset",&f_drawingOffset);
	t+=dbg_decl("drawingSize",&f_drawingSize);
	t+=dbg_decl("drawingWidth",&f_drawingWidth);
	return t;
}
bb_drawingpoint_DrawingPoint::bb_drawingpoint_DrawingPoint(){
	f_y=FLOAT(.0);
	f_x=FLOAT(.0);
}
bb_drawingpoint_DrawingPoint* bb_drawingpoint_DrawingPoint::g_new(){
	DBG_ENTER("DrawingPoint.new")
	bb_drawingpoint_DrawingPoint *self=this;
	DBG_LOCAL(self,"Self")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/fontmachine/drawingpoint.monkey<8>");
	return this;
}
void bb_drawingpoint_DrawingPoint::mark(){
	Object::mark();
}
String bb_drawingpoint_DrawingPoint::debug(){
	String t="(DrawingPoint)\n";
	t+=dbg_decl("x",&f_x);
	t+=dbg_decl("y",&f_y);
	return t;
}
Float bb_challengergui_CHGUI_TextHeight(bb_bitmapfont_BitmapFont* t_Fnt,String t_Text){
	DBG_ENTER("CHGUI_TextHeight")
	DBG_LOCAL(t_Fnt,"Fnt")
	DBG_LOCAL(t_Text,"Text")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3871>");
	Array<String > t_Split=t_Text.Split(String(L"\n",1));
	DBG_LOCAL(t_Split,"Split")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3872>");
	Float t_H=Float(t_Fnt->m_GetFontHeight());
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3873>");
	Float t_Height=FLOAT(0.0);
	DBG_LOCAL(t_Height,"Height")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3874>");
	for(int t_N=0;t_N<=t_Split.Length()-1;t_N=t_N+1){
		DBG_BLOCK();
		DBG_LOCAL(t_N,"N")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3875>");
		t_Height=t_Height+t_H;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3877>");
	return t_Height;
}
bb_edrawmode_eDrawMode::bb_edrawmode_eDrawMode(){
}
void bb_edrawmode_eDrawMode::mark(){
	Object::mark();
}
String bb_edrawmode_eDrawMode::debug(){
	String t="(eDrawMode)\n";
	return t;
}
bb_edrawalign_eDrawAlign::bb_edrawalign_eDrawAlign(){
}
void bb_edrawalign_eDrawAlign::mark(){
	Object::mark();
}
String bb_edrawalign_eDrawAlign::debug(){
	String t="(eDrawAlign)\n";
	return t;
}
int bb_math_Abs(int t_x){
	DBG_ENTER("Abs")
	DBG_LOCAL(t_x,"x")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<46>");
	if(t_x>=0){
		DBG_BLOCK();
		return t_x;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<47>");
	int t_=-t_x;
	return t_;
}
Float bb_math_Abs2(Float t_x){
	DBG_ENTER("Abs")
	DBG_LOCAL(t_x,"x")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<73>");
	if(t_x>=FLOAT(0.0)){
		DBG_BLOCK();
		return t_x;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/monkey/math.monkey<74>");
	Float t_=-t_x;
	return t_;
}
int bb_graphics_DrawImage(bb_graphics_Image* t_image,Float t_x,Float t_y,int t_frame){
	DBG_ENTER("DrawImage")
	DBG_LOCAL(t_image,"image")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_frame,"frame")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<417>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<419>");
	bb_graphics_Frame* t_f=t_image->f_frames.At(t_frame);
	DBG_LOCAL(t_f,"f")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<421>");
	if((bb_graphics_context->f_tformed)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<422>");
		bb_graphics_PushMatrix();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<424>");
		bb_graphics_Translate(t_x-t_image->f_tx,t_y-t_image->f_ty);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<426>");
		bb_graphics_context->m_Validate();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<428>");
		if((t_image->f_flags&65536)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<429>");
			bb_graphics_renderDevice->DrawSurface(t_image->f_surface,FLOAT(0.0),FLOAT(0.0));
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<431>");
			bb_graphics_renderDevice->DrawSurface2(t_image->f_surface,FLOAT(0.0),FLOAT(0.0),t_f->f_x,t_f->f_y,t_image->f_width,t_image->f_height);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<434>");
		bb_graphics_PopMatrix();
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<436>");
		bb_graphics_context->m_Validate();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<438>");
		if((t_image->f_flags&65536)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<439>");
			bb_graphics_renderDevice->DrawSurface(t_image->f_surface,t_x-t_image->f_tx,t_y-t_image->f_ty);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<441>");
			bb_graphics_renderDevice->DrawSurface2(t_image->f_surface,t_x-t_image->f_tx,t_y-t_image->f_ty,t_f->f_x,t_f->f_y,t_image->f_width,t_image->f_height);
		}
	}
	return 0;
}
int bb_graphics_DrawImage2(bb_graphics_Image* t_image,Float t_x,Float t_y,Float t_rotation,Float t_scaleX,Float t_scaleY,int t_frame){
	DBG_ENTER("DrawImage")
	DBG_LOCAL(t_image,"image")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_rotation,"rotation")
	DBG_LOCAL(t_scaleX,"scaleX")
	DBG_LOCAL(t_scaleY,"scaleY")
	DBG_LOCAL(t_frame,"frame")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<448>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<450>");
	bb_graphics_Frame* t_f=t_image->f_frames.At(t_frame);
	DBG_LOCAL(t_f,"f")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<452>");
	bb_graphics_PushMatrix();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<454>");
	bb_graphics_Translate(t_x,t_y);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<455>");
	bb_graphics_Rotate(t_rotation);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<456>");
	bb_graphics_Scale(t_scaleX,t_scaleY);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<458>");
	bb_graphics_Translate(-t_image->f_tx,-t_image->f_ty);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<460>");
	bb_graphics_context->m_Validate();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<462>");
	if((t_image->f_flags&65536)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<463>");
		bb_graphics_renderDevice->DrawSurface(t_image->f_surface,FLOAT(0.0),FLOAT(0.0));
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<465>");
		bb_graphics_renderDevice->DrawSurface2(t_image->f_surface,FLOAT(0.0),FLOAT(0.0),t_f->f_x,t_f->f_y,t_image->f_width,t_image->f_height);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<468>");
	bb_graphics_PopMatrix();
	return 0;
}
bb_bitmapfont_BitmapFont* bb_challengergui_CHGUI_Font;
int bb_challengergui_CHGUI_DrawWindow(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_DrawWindow")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2228>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2229>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2230>");
	Float t_W=t_N->f_W;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2231>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2232>");
	Float t_TH=bb_challengergui_CHGUI_TitleHeight;
	DBG_LOCAL(t_TH,"TH")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2233>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2234>");
	if(bb_challengergui_CHGUI_LockedWIndow==t_N){
		DBG_BLOCK();
		t_Active=1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2236>");
	if(t_N!=bb_challengergui_CHGUI_Canvas){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2238>");
		if(((t_N->f_Shadow)!=0) && ((bb_challengergui_CHGUI_Shadow)!=0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2239>");
			if((t_N->f_Minimised)!=0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2241>");
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y-FLOAT(10.0),0,0,20,20,0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2243>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+FLOAT(10.0),t_Y-FLOAT(10.0),20,0,10,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(10.0),FLOAT(1.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2245>");
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W-FLOAT(10.0),t_Y-FLOAT(10.0),30,0,20,20,0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2247>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y+FLOAT(10.0),0,20,10,10,FLOAT(0.0),FLOAT(1.0),(t_TH-FLOAT(20.0))/FLOAT(10.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2249>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y+FLOAT(10.0),40,20,10,10,FLOAT(0.0),FLOAT(1.0),(t_TH-FLOAT(20.0))/FLOAT(10.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2251>");
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y+t_TH-FLOAT(10.0),0,30,20,20,0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2253>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+FLOAT(10.0),t_Y+t_TH,20,40,10,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(10.0),FLOAT(1.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2255>");
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W-FLOAT(10.0),t_Y+t_TH-FLOAT(10.0),30,30,20,20,0);
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2258>");
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y-FLOAT(10.0),0,0,20,20,0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2260>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+FLOAT(10.0),t_Y-FLOAT(10.0),20,0,10,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(10.0),FLOAT(1.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2262>");
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W-FLOAT(10.0),t_Y-FLOAT(10.0),30,0,20,20,0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2264>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y+FLOAT(10.0),0,20,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(10.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2266>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y+FLOAT(10.0),40,20,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(10.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2268>");
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),0,30,20,20,0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2270>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+FLOAT(10.0),t_Y+t_H,20,40,10,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(10.0),FLOAT(1.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2272>");
				bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),30,30,20,20,0);
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2276>");
		Float t_XOf=FLOAT(10.0);
		DBG_LOCAL(t_XOf,"XOf")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2277>");
		Float t_YOf=FLOAT(10.0);
		DBG_LOCAL(t_YOf,"YOf")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2278>");
		if(bb_challengergui_CHGUI_RealActive(t_N)==0 && bb_challengergui_CHGUI_LockedWIndow!=t_N){
			DBG_BLOCK();
			t_YOf=t_YOf+FLOAT(30.0);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2281>");
		if(t_N->f_Text!=String()){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2283>");
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y,int(t_XOf),int(t_YOf),10,10,0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2285>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y,int(t_XOf+FLOAT(10.0)),int(t_YOf),50,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(50.0),FLOAT(1.0),0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2287>");
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y,int(t_XOf+FLOAT(60.0)),int(t_YOf),10,10,0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2289>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+FLOAT(10.0),int(t_XOf),int(t_YOf+FLOAT(10.0)),10,10,FLOAT(0.0),FLOAT(1.0),(t_TH-FLOAT(20.0))/FLOAT(10.0),0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2291>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+FLOAT(10.0),int(t_XOf+FLOAT(10.0)),int(t_YOf+FLOAT(10.0)),50,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(50.0),(t_TH-FLOAT(20.0))/FLOAT(10.0),0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2293>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+FLOAT(10.0),int(t_XOf+FLOAT(60.0)),int(t_YOf+FLOAT(10.0)),10,10,FLOAT(0.0),FLOAT(1.0),(t_TH-FLOAT(20.0))/FLOAT(10.0),0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2295>");
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y+t_TH-FLOAT(10.0),int(t_XOf),int(t_YOf+FLOAT(20.0)),10,10,0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2297>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+t_TH-FLOAT(10.0),int(t_XOf+FLOAT(10.0)),int(t_YOf+FLOAT(20.0)),50,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(50.0),FLOAT(1.0),0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2299>");
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+t_TH-FLOAT(10.0),int(t_XOf+FLOAT(60.0)),int(t_YOf+FLOAT(20.0)),10,10,0);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2302>");
		if(t_N->f_Minimised==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2303>");
			if(t_N->f_Text==String()){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2305>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,10,70,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2307>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y,20,70,50,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(50.0),FLOAT(1.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2309>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y,70,70,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2311>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+FLOAT(10.0),10,80,10,40,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(40.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2313>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+FLOAT(10.0),70,80,10,40,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(40.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2315>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+FLOAT(10.0),20,80,50,40,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(50.0),(t_H-FLOAT(20.0))/FLOAT(40.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2317>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-FLOAT(10.0),10,120,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2319>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+t_H-FLOAT(10.0),20,120,50,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(50.0),FLOAT(1.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2321>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),70,120,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2324>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_TH,10,80,10,40,FLOAT(0.0),FLOAT(1.0),(t_H-t_TH-FLOAT(10.0))/FLOAT(40.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2326>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+t_TH,70,80,10,40,FLOAT(0.0),FLOAT(1.0),(t_H-t_TH-FLOAT(10.0))/FLOAT(40.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2328>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+t_TH,20,80,50,40,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(50.0),(t_H-t_TH-FLOAT(10.0))/FLOAT(40.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2330>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-FLOAT(10.0),10,120,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2332>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+t_H-FLOAT(10.0),20,120,50,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(50.0),FLOAT(1.0),0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2334>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),70,120,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2339>");
		if(t_N->f_Close==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2340>");
			if(((t_N->f_CloseOver)!=0) && t_N->f_CloseDown==0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2341>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_TH/FLOAT(2.5)-FLOAT(10.0),t_Y+(t_TH-t_TH/FLOAT(2.5))/FLOAT(2.0),105,10,15,15,FLOAT(0.0),t_TH/FLOAT(2.5)/FLOAT(15.0),t_TH/FLOAT(2.5)/FLOAT(15.0),0);
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2342>");
				if((t_N->f_CloseDown)!=0){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2343>");
					bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_TH/FLOAT(2.5)-FLOAT(10.0),t_Y+(t_TH-t_TH/FLOAT(2.5))/FLOAT(2.0),120,10,15,15,FLOAT(0.0),t_TH/FLOAT(2.5)/FLOAT(15.0),t_TH/FLOAT(2.5)/FLOAT(15.0),0);
				}else{
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2344>");
					if((t_Active)!=0){
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2345>");
						bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_TH/FLOAT(2.5)-FLOAT(10.0),t_Y+(t_TH-t_TH/FLOAT(2.5))/FLOAT(2.0),90,10,15,15,FLOAT(0.0),t_TH/FLOAT(2.5)/FLOAT(15.0),t_TH/FLOAT(2.5)/FLOAT(15.0),0);
					}else{
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2347>");
						bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_TH/FLOAT(2.5)-FLOAT(10.0),t_Y+(t_TH-t_TH/FLOAT(2.5))/FLOAT(2.0),135,10,15,15,FLOAT(0.0),t_TH/FLOAT(2.5)/FLOAT(15.0),t_TH/FLOAT(2.5)/FLOAT(15.0),0);
					}
				}
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2352>");
		if(t_N->f_Minimise==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2353>");
			if(((t_N->f_MinimiseOver)!=0) && t_N->f_MinimiseDown==0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2354>");
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-(t_TH/FLOAT(2.5)+t_TH/FLOAT(2.5))-t_TH/FLOAT(1.5),t_Y+(t_TH-t_TH/FLOAT(2.5))/FLOAT(2.0),105,25,15,15,FLOAT(0.0),t_TH/FLOAT(2.5)/FLOAT(15.0),t_TH/FLOAT(2.5)/FLOAT(15.0),0);
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2355>");
				if((t_N->f_MinimiseDown)!=0){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2356>");
					bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-(t_TH/FLOAT(2.5)+t_TH/FLOAT(2.5))-t_TH/FLOAT(1.5),t_Y+(t_TH-t_TH/FLOAT(2.5))/FLOAT(2.0),120,25,15,15,FLOAT(0.0),t_TH/FLOAT(2.5)/FLOAT(15.0),t_TH/FLOAT(2.5)/FLOAT(15.0),0);
				}else{
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2357>");
					if((t_Active)!=0){
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2358>");
						bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-(t_TH/FLOAT(2.5)+t_TH/FLOAT(2.5))-t_TH/FLOAT(1.5),t_Y+(t_TH-t_TH/FLOAT(2.5))/FLOAT(2.0),90,25,15,15,FLOAT(0.0),t_TH/FLOAT(2.5)/FLOAT(15.0),t_TH/FLOAT(2.5)/FLOAT(15.0),0);
					}else{
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2360>");
						bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-(t_TH/FLOAT(2.5)+t_TH/FLOAT(2.5))-t_TH/FLOAT(1.5),t_Y+(t_TH-t_TH/FLOAT(2.5))/FLOAT(2.0),135,25,15,15,FLOAT(0.0),t_TH/FLOAT(2.5)/FLOAT(15.0),t_TH/FLOAT(2.5)/FLOAT(15.0),0);
					}
				}
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2369>");
		Float t_XOff=(t_TH-t_TH/FLOAT(2.0))/FLOAT(2.0);
		DBG_LOCAL(t_XOff,"XOff")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2370>");
		Float t_YOff=t_TH-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_TitleFont,t_N->f_Text);
		DBG_LOCAL(t_YOff,"YOff")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2371>");
		bb_graphics_SetAlpha(FLOAT(0.25));
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2372>");
		if((t_Active)!=0){
			DBG_BLOCK();
			bb_graphics_SetAlpha(FLOAT(1.0));
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2373>");
		bb_challengergui_CHGUI_TitleFont->m_DrawText2(t_N->f_Text,t_X+t_XOff,t_Y+t_YOff/FLOAT(2.0));
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2374>");
		bb_graphics_SetAlpha(FLOAT(1.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2377>");
	if((t_N->f_HasMenu)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2378>");
		if(t_N!=bb_challengergui_CHGUI_Canvas){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2379>");
			if((bb_challengergui_CHGUI_Shadow)!=0){
				DBG_BLOCK();
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X,t_Y+t_TH+Float(t_N->f_MenuHeight),20,40,10,10,FLOAT(0.0),t_W/FLOAT(10.0),FLOAT(1.0),0);
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2380>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(1.0),t_Y+bb_challengergui_CHGUI_TitleHeight,100,90,40,10,FLOAT(0.0),(t_W-FLOAT(2.0))/FLOAT(40.0),(FLOAT(10.0)+bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text)-FLOAT(10.0))/FLOAT(10.0),0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2381>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(1.0),t_Y+bb_challengergui_CHGUI_TitleHeight+Float(t_N->f_MenuHeight-10),100,100,40,10,FLOAT(0.0),(t_W-FLOAT(2.0))/FLOAT(40.0),FLOAT(1.0),0);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2383>");
			if((bb_challengergui_CHGUI_Shadow)!=0){
				DBG_BLOCK();
				bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X,t_Y+Float(t_N->f_MenuHeight),20,40,10,10,FLOAT(0.0),t_W/FLOAT(10.0),FLOAT(1.0),0);
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2384>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(1.0),t_Y,100,90,40,10,FLOAT(0.0),(t_W-FLOAT(2.0))/FLOAT(40.0),(FLOAT(10.0)+bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text)-FLOAT(10.0))/FLOAT(10.0),0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2385>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(1.0),t_Y+Float(t_N->f_MenuHeight-10),100,100,40,10,FLOAT(0.0),(t_W-FLOAT(2.0))/FLOAT(40.0),FLOAT(1.0),0);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2390>");
	if(((t_N->f_Tabbed)!=0) && t_N->f_Minimised==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2391>");
		int t_YY=int(t_Y+Float(t_N->f_MenuHeight));
		DBG_LOCAL(t_YY,"YY")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2392>");
		if(t_N->f_Text!=String()){
			DBG_BLOCK();
			t_YY=int(Float(t_YY)+bb_challengergui_CHGUI_TitleHeight);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2393>");
		Float t_Height=Float(t_N->f_TabHeight+5);
		DBG_LOCAL(t_Height,"Height")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2396>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+FLOAT(1.0),Float(t_YY),10,140,10,10,0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2398>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(11.0),Float(t_YY),60,140,10,10,0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2400>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(11.0),Float(t_YY),20,140,40,10,FLOAT(0.0),(t_W-FLOAT(22.0))/FLOAT(40.0),FLOAT(1.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2402>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(1.0),Float(t_YY+10),10,150,10,10,FLOAT(0.0),FLOAT(1.0),(t_Height-FLOAT(10.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2404>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(11.0),Float(t_YY+10),60,150,10,10,FLOAT(0.0),FLOAT(1.0),(t_Height-FLOAT(10.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2406>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(11.0),Float(t_YY+10),20,150,40,10,FLOAT(0.0),(t_N->f_W-FLOAT(22.0))/FLOAT(40.0),(t_Height-FLOAT(10.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2408>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+FLOAT(1.0),Float(t_YY)+t_Height-FLOAT(10.0),10,160,10,10,0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2410>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(11.0),Float(t_YY)+t_Height-FLOAT(10.0),60,160,10,10,0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2412>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(11.0),Float(t_YY)+t_Height-FLOAT(10.0),20,160,40,10,FLOAT(0.0),(t_W-FLOAT(22.0))/FLOAT(40.0),FLOAT(1.0),0);
	}
	return 0;
}
Array<bb_challengergui_CHGUI* > bb_challengergui_CHGUI_KeyboardButtons;
int bb_challengergui_CHGUI_ShiftHold;
int bb_challengergui_CHGUI_DrawButton(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_DrawButton")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2418>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2419>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2420>");
	Float t_W=t_N->f_W;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2421>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2422>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2423>");
	int t_State=0;
	DBG_LOCAL(t_State,"State")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2424>");
	if((t_N->f_Over)!=0){
		DBG_BLOCK();
		t_State=40;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2425>");
	if((t_N->f_Down)!=0){
		DBG_BLOCK();
		t_State=80;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2426>");
	if(t_N==bb_challengergui_CHGUI_KeyboardButtons.At(104) && ((bb_challengergui_CHGUI_ShiftHold)!=0)){
		DBG_BLOCK();
		t_State=80;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2427>");
	if(t_Active==0){
		DBG_BLOCK();
		t_State=120;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2432>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y,160,10+t_State,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2434>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y,170,10+t_State,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2436>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y,210,10+t_State,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2438>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+FLOAT(10.0),160,20+t_State,10,20,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(20.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2440>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+FLOAT(10.0),210,20+t_State,10,20,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(20.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2442>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+FLOAT(10.0),170,20+t_State,40,20,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),(t_H-FLOAT(20.0))/FLOAT(20.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2444>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-FLOAT(10.0),160,40+t_State,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2446>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+t_H-FLOAT(10.0),170,40+t_State,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2448>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),210,40+t_State,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2451>");
	Float t_XOff=(t_W-bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text))/FLOAT(2.0);
	DBG_LOCAL(t_XOff,"XOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2452>");
	Float t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text))/FLOAT(2.0);
	DBG_LOCAL(t_YOff,"YOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2453>");
	bb_graphics_SetAlpha(FLOAT(0.25));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2454>");
	if((t_Active)!=0){
		DBG_BLOCK();
		bb_graphics_SetAlpha(FLOAT(1.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2455>");
	bb_challengergui_CHGUI_Font->m_DrawText2(t_N->f_Text,t_X+t_XOff,t_Y+t_YOff);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2456>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	return 0;
}
int bb_challengergui_CHGUI_DrawImageButton(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_DrawImageButton")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2479>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2480>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2481>");
	Float t_W=t_N->f_W;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2482>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2483>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2484>");
	int t_State=0;
	DBG_LOCAL(t_State,"State")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2485>");
	if((t_N->f_Over)!=0){
		DBG_BLOCK();
		t_State=int(t_W);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2486>");
	if((t_N->f_Down)!=0){
		DBG_BLOCK();
		t_State=int(t_W*FLOAT(2.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2487>");
	if(t_Active==0){
		DBG_BLOCK();
		t_State=int(t_W*FLOAT(3.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2489>");
	bb_graphics_DrawImageRect(t_N->f_Img,t_X,t_Y,t_State,0,int(t_W),int(t_H),0);
	return 0;
}
int bb_challengergui_CHGUI_DrawTickbox(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_DrawTickbox")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2495>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2496>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2497>");
	Float t_W=t_N->f_H;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2498>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2499>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2500>");
	int t_OffX=230;
	DBG_LOCAL(t_OffX,"OffX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2501>");
	int t_OffY=10;
	DBG_LOCAL(t_OffY,"OffY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2502>");
	int t_OffW=20;
	DBG_LOCAL(t_OffW,"OffW")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2503>");
	int t_OffH=20;
	DBG_LOCAL(t_OffH,"OffH")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2504>");
	if((t_N->f_Over)!=0){
		DBG_BLOCK();
		t_OffY=30;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2505>");
	if((t_N->f_Down)!=0){
		DBG_BLOCK();
		t_OffY=50;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2506>");
	if(t_Active==0){
		DBG_BLOCK();
		t_OffY=70;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2507>");
	if(t_N->f_Value>FLOAT(0.0)){
		DBG_BLOCK();
		t_OffX=250;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2510>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,t_OffW,t_OffH,FLOAT(0.0),t_W/Float(t_OffW),t_H/Float(t_OffH),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2512>");
	Float t_XOff=t_W/FLOAT(4.0);
	DBG_LOCAL(t_XOff,"XOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2513>");
	Float t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text))/FLOAT(2.0);
	DBG_LOCAL(t_YOff,"YOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2514>");
	bb_graphics_SetAlpha(FLOAT(0.25));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2515>");
	if((t_Active)!=0){
		DBG_BLOCK();
		bb_graphics_SetAlpha(FLOAT(1.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2516>");
	bb_challengergui_CHGUI_Font->m_DrawText2(t_N->f_Text,t_X+t_W+t_XOff,t_Y+t_YOff);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2517>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	return 0;
}
int bb_challengergui_CHGUI_DrawRadiobox(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_DrawRadiobox")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2523>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2524>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2525>");
	Float t_W=t_N->f_H;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2526>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2527>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2528>");
	int t_OffX=230;
	DBG_LOCAL(t_OffX,"OffX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2529>");
	int t_OffY=100;
	DBG_LOCAL(t_OffY,"OffY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2530>");
	int t_OffW=20;
	DBG_LOCAL(t_OffW,"OffW")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2531>");
	int t_OffH=20;
	DBG_LOCAL(t_OffH,"OffH")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2532>");
	if((t_N->f_Over)!=0){
		DBG_BLOCK();
		t_OffY=120;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2533>");
	if((t_N->f_Down)!=0){
		DBG_BLOCK();
		t_OffY=140;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2534>");
	if(t_Active==0){
		DBG_BLOCK();
		t_OffY=160;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2535>");
	if(t_N->f_Value>FLOAT(0.0)){
		DBG_BLOCK();
		t_OffX=250;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2538>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,t_OffW,t_OffH,FLOAT(0.0),t_W/Float(t_OffW),t_H/Float(t_OffH),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2540>");
	Float t_XOff=t_W/FLOAT(4.0);
	DBG_LOCAL(t_XOff,"XOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2541>");
	Float t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text))/FLOAT(2.0);
	DBG_LOCAL(t_YOff,"YOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2542>");
	bb_graphics_SetAlpha(FLOAT(0.25));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2543>");
	if((t_Active)!=0){
		DBG_BLOCK();
		bb_graphics_SetAlpha(FLOAT(1.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2544>");
	bb_challengergui_CHGUI_Font->m_DrawText2(t_N->f_Text,t_X+t_W+t_XOff,t_Y+t_YOff);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2545>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	return 0;
}
int bb_challengergui_CHGUI_DrawListbox(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_DrawListbox")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3074>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3075>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3076>");
	Float t_W=t_N->f_W;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3077>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3078>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3079>");
	t_N->f_ListboxSlider->f_X=t_N->f_X+t_W-FLOAT(17.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3080>");
	t_N->f_ListboxSlider->f_Y=t_N->f_ListboxSlider->f_Parent->f_Y+t_N->f_Y-t_N->f_Parent->f_Y;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3081>");
	t_N->f_ListboxSlider->f_H=t_N->f_H;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3082>");
	int t_OffX=90;
	DBG_LOCAL(t_OffX,"OffX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3083>");
	int t_OffY=80;
	DBG_LOCAL(t_OffY,"OffY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3087>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3089>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y,t_OffX+10,t_OffY,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3091>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y,t_OffX+50,t_OffY,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3093>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+FLOAT(10.0),t_OffX,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(10.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3095>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+FLOAT(10.0),t_OffX+50,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(10.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3097>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+FLOAT(10.0),t_OffX+10,t_OffY+10,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),(t_H-FLOAT(20.0))/FLOAT(10.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3099>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-FLOAT(10.0),t_OffX,t_OffY+20,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3101>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+t_H-FLOAT(10.0),t_OffX+10,t_OffY+20,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3103>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),t_OffX+50,t_OffY+20,10,10,0);
	return 0;
}
int bb_challengergui_CHGUI_DrawListboxItem(bb_challengergui_CHGUI* t_N,int t_C){
	DBG_ENTER("CHGUI_DrawListboxItem")
	DBG_LOCAL(t_N,"N")
	DBG_LOCAL(t_C,"C")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3109>");
	t_N->f_X=FLOAT(0.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3110>");
	t_N->f_Y=Float(t_C*t_N->f_Parent->f_ListHeight);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3111>");
	t_N->f_W=t_N->f_Parent->f_W;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3112>");
	t_N->f_H=Float(t_N->f_Parent->f_ListHeight);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3114>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3115>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3116>");
	Float t_W=t_N->f_W;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3117>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3118>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3120>");
	if(t_N->f_Over==1 || ((t_N->f_Down)!=0) || t_N->f_Parent->f_SelectedListboxItem==t_N){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3122>");
		int t_OffX=90;
		DBG_LOCAL(t_OffX,"OffX")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3123>");
		int t_OffY=110;
		DBG_LOCAL(t_OffY,"OffY")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3124>");
		if(((t_N->f_Down)!=0) || t_N->f_Parent->f_SelectedListboxItem==t_N){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3125>");
			t_OffX=90;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3126>");
			t_OffY=140;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3130>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3132>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y,t_OffX+10,t_OffY,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),FLOAT(1.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3134>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y,t_OffX+50,t_OffY,10,10,0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3136>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+FLOAT(10.0),t_OffX,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3138>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+FLOAT(10.0),t_OffX+50,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3140>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+FLOAT(10.0),t_OffX+10,t_OffY+10,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),(t_H-FLOAT(20.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3142>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-FLOAT(10.0),t_OffX,t_OffY+20,10,10,0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3144>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+t_H-FLOAT(10.0),t_OffX+10,t_OffY+20,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),FLOAT(1.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3146>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),t_OffX+50,t_OffY+20,10,10,0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3151>");
	Float t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text))/FLOAT(2.0);
	DBG_LOCAL(t_YOff,"YOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3152>");
	bb_graphics_SetAlpha(FLOAT(0.25));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3153>");
	if((t_Active)!=0){
		DBG_BLOCK();
		bb_graphics_SetAlpha(FLOAT(1.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3154>");
	bb_challengergui_CHGUI_Font->m_DrawText2(t_N->f_Text,t_X+FLOAT(10.0),t_Y+t_YOff);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3155>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	return 0;
}
int bb_challengergui_CHGUI_DrawHSlider(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_DrawHSlider")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2857>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2858>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2859>");
	Float t_W=t_N->f_W;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2860>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2861>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2863>");
	int t_OffX=460;
	DBG_LOCAL(t_OffX,"OffX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2864>");
	int t_OffY=10;
	DBG_LOCAL(t_OffY,"OffY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2866>");
	if(t_Active==0){
		DBG_BLOCK();
		t_OffY=70;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2869>");
	if(((t_N->f_MinusOver)!=0) && t_N->f_MinusDown==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2870>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY+20,20,20,FLOAT(0.0),t_H/FLOAT(20.0),t_H/FLOAT(20.0),0);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2871>");
		if((t_N->f_MinusDown)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2872>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY+40,20,20,FLOAT(0.0),t_H/FLOAT(20.0),t_H/FLOAT(20.0),0);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2874>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,20,20,FLOAT(0.0),t_H/FLOAT(20.0),t_H/FLOAT(20.0),0);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2878>");
	if(((t_N->f_PlusOver)!=0) && t_N->f_PlusDown==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2879>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_H,t_Y,t_OffX+60,t_OffY+20,20,20,FLOAT(0.0),t_H/FLOAT(20.0),t_H/FLOAT(20.0),0);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2880>");
		if((t_N->f_PlusDown)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2881>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_H,t_Y,t_OffX+60,t_OffY+40,20,20,FLOAT(0.0),t_H/FLOAT(20.0),t_H/FLOAT(20.0),0);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2883>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_H,t_Y,t_OffX+60,t_OffY,20,20,FLOAT(0.0),t_H/FLOAT(20.0),t_H/FLOAT(20.0),0);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2888>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_H,t_Y,t_OffX+20,t_OffY,40,20,FLOAT(0.0),(t_W-t_H-t_H)/FLOAT(40.0),t_H/FLOAT(20.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2891>");
	int t_XOF=475;
	DBG_LOCAL(t_XOF,"XOF")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2892>");
	int t_YOF=100;
	DBG_LOCAL(t_YOF,"YOF")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2893>");
	if(((t_N->f_SliderOver)!=0) && t_N->f_SliderDown==0){
		DBG_BLOCK();
		t_YOF=t_YOF+20;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2894>");
	if((t_N->f_SliderDown)!=0){
		DBG_BLOCK();
		t_YOF=t_YOF+40;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2895>");
	if(t_Active==0){
		DBG_BLOCK();
		t_YOF=t_YOF+60;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2896>");
	Float t_XPOS=t_X+t_H-FLOAT(5.0)+(t_N->f_Value-t_N->f_Minimum)*t_N->f_Stp;
	DBG_LOCAL(t_XPOS,"XPOS")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2898>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_XPOS,t_Y,t_XOF,t_YOF,5,20,FLOAT(0.0),FLOAT(1.0),t_H/FLOAT(20.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2899>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_XPOS+FLOAT(5.0),t_Y,t_XOF+5,t_YOF,40,20,FLOAT(0.0),(t_N->f_SWidth-FLOAT(10.0))/FLOAT(40.0),t_H/FLOAT(20.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2900>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_XPOS+t_N->f_SWidth-FLOAT(5.0),t_Y,t_XOF+45,t_YOF,5,20,FLOAT(0.0),FLOAT(1.0),t_H/FLOAT(20.0),0);
	return 0;
}
int bb_challengergui_CHGUI_DrawVSlider(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_DrawVSlider")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2905>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2906>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2907>");
	Float t_W=t_N->f_W;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2908>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2909>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2911>");
	int t_OffX=370;
	DBG_LOCAL(t_OffX,"OffX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2912>");
	int t_OffY=10;
	DBG_LOCAL(t_OffY,"OffY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2914>");
	if(t_Active==0){
		DBG_BLOCK();
		t_OffX=430;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2917>");
	if(((t_N->f_MinusOver)!=0) && t_N->f_MinusDown==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2918>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX+20,t_OffY,20,20,FLOAT(0.0),t_W/FLOAT(20.0),t_W/FLOAT(20.0),0);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2919>");
		if((t_N->f_MinusDown)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2920>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX+40,t_OffY,20,20,FLOAT(0.0),t_W/FLOAT(20.0),t_W/FLOAT(20.0),0);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2922>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,20,20,FLOAT(0.0),t_W/FLOAT(20.0),t_W/FLOAT(20.0),0);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2926>");
	if(((t_N->f_PlusOver)!=0) && t_N->f_PlusDown==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2927>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-t_W,t_OffX+20,t_OffY+60,20,20,FLOAT(0.0),t_W/FLOAT(20.0),t_W/FLOAT(20.0),0);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2928>");
		if((t_N->f_PlusDown)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2929>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-t_W,t_OffX+40,t_OffY+60,20,20,FLOAT(0.0),t_W/FLOAT(20.0),t_W/FLOAT(20.0),0);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2931>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-t_W,t_OffX,t_OffY+60,20,20,FLOAT(0.0),t_W/FLOAT(20.0),t_W/FLOAT(20.0),0);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2936>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_W,t_OffX,t_OffY+20,20,40,FLOAT(0.0),t_W/FLOAT(20.0),(t_H-t_W-t_W)/FLOAT(40.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2939>");
	int t_XOF=370;
	DBG_LOCAL(t_XOF,"XOF")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2940>");
	int t_YOF=100;
	DBG_LOCAL(t_YOF,"YOF")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2941>");
	if(((t_N->f_SliderOver)!=0) && t_N->f_SliderDown==0){
		DBG_BLOCK();
		t_XOF=t_XOF+20;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2942>");
	if((t_N->f_SliderDown)!=0){
		DBG_BLOCK();
		t_XOF=t_XOF+40;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2943>");
	if(t_Active==0){
		DBG_BLOCK();
		t_XOF=t_XOF+60;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2944>");
	Float t_YPOS=t_Y+t_W-FLOAT(5.0)+(t_N->f_Value-t_N->f_Minimum)*t_N->f_Stp;
	DBG_LOCAL(t_YPOS,"YPOS")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2946>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_YPOS,t_XOF,t_YOF,20,5,FLOAT(0.0),t_W/FLOAT(20.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2947>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_YPOS+FLOAT(5.0),t_XOF,t_YOF+5,20,40,FLOAT(0.0),t_W/FLOAT(20.0),(t_N->f_SWidth-FLOAT(10.0))/FLOAT(40.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2948>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_YPOS+t_N->f_SWidth-FLOAT(5.0),t_XOF,t_YOF+45,20,5,FLOAT(0.0),t_W/FLOAT(20.0),FLOAT(1.0),0);
	return 0;
}
int bb_challengergui_CHGUI_Cursor;
int bb_graphics_DrawLine(Float t_x1,Float t_y1,Float t_x2,Float t_y2){
	DBG_ENTER("DrawLine")
	DBG_LOCAL(t_x1,"x1")
	DBG_LOCAL(t_y1,"y1")
	DBG_LOCAL(t_x2,"x2")
	DBG_LOCAL(t_y2,"y2")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<377>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<379>");
	bb_graphics_context->m_Validate();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<380>");
	bb_graphics_renderDevice->DrawLine(t_x1,t_y1,t_x2,t_y2);
	return 0;
}
int bb_challengergui_CHGUI_DrawTextfield(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_DrawTextfield")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2953>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2954>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2955>");
	Float t_W=t_N->f_W;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2956>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2957>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2959>");
	int t_OffX=280;
	DBG_LOCAL(t_OffX,"OffX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2960>");
	int t_OffY=10;
	DBG_LOCAL(t_OffY,"OffY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2961>");
	int t_OffH=40;
	DBG_LOCAL(t_OffH,"OffH")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2963>");
	if((t_N->f_Over)!=0){
		DBG_BLOCK();
		t_OffY=50;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2964>");
	if(((t_N->f_Down)!=0) && t_N->f_OnFocus==0){
		DBG_BLOCK();
		t_OffY=90;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2965>");
	if(bb_challengergui_CHGUI_RealActive(t_N)==0){
		DBG_BLOCK();
		t_OffY=130;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2967>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2969>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y,t_OffX+10,t_OffY,20,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(20.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2971>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y,t_OffX+30,t_OffY,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2973>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+FLOAT(10.0),t_OffX,t_OffY+10,10,20,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(20.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2975>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+FLOAT(10.0),t_OffX+30,t_OffY+10,10,20,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(20.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2977>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+FLOAT(10.0),t_OffX+10,t_OffY+10,20,20,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(20.0),(t_H-FLOAT(20.0))/FLOAT(20.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2979>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-FLOAT(10.0),t_OffX,t_OffY+30,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2981>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+t_H-FLOAT(10.0),t_OffX+10,t_OffY+30,20,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(20.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2983>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),t_OffX+30,t_OffY+30,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2986>");
	Float t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text))/FLOAT(2.0);
	DBG_LOCAL(t_YOff,"YOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2987>");
	bb_graphics_SetAlpha(FLOAT(0.25));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2988>");
	if((t_Active)!=0){
		DBG_BLOCK();
		bb_graphics_SetAlpha(FLOAT(1.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2989>");
	bb_challengergui_CHGUI_Font->m_DrawText2(t_N->f_Text,t_X+FLOAT(5.0),t_Y+t_YOff);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2990>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2993>");
	if((t_N->f_OnFocus)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2994>");
		String t_Before=t_N->f_Text.Slice(0,t_N->f_Cursor);
		DBG_LOCAL(t_Before,"Before")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2995>");
		int t_Length=int(bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_Before+String(L"NOT",3))-bb_challengergui_CHGUI_Font->m_GetTxtWidth2(String(L"NOT",3)));
		DBG_LOCAL(t_Length,"Length")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2996>");
		if((bb_challengergui_CHGUI_Cursor)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2997>");
			bb_graphics_SetColor(FLOAT(0.0),FLOAT(0.0),FLOAT(0.0));
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2998>");
			bb_graphics_DrawLine(t_X+Float(t_Length)+FLOAT(8.0),t_Y+t_YOff,t_X+Float(t_Length)+FLOAT(8.0),t_Y+t_YOff+bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text));
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2999>");
			bb_graphics_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_DrawLabel(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_DrawLabel")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2462>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2463>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2464>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2467>");
	Float t_XOff=FLOAT(.0);
	DBG_LOCAL(t_XOff,"XOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2468>");
	Float t_YOff=FLOAT(.0);
	DBG_LOCAL(t_YOff,"YOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2470>");
	bb_graphics_SetAlpha(FLOAT(0.25));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2471>");
	if((t_Active)!=0){
		DBG_BLOCK();
		bb_graphics_SetAlpha(FLOAT(1.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2472>");
	bb_challengergui_CHGUI_Font->m_DrawText2(t_N->f_Text,t_X,t_Y);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2473>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	return 0;
}
int bb_challengergui_CHGUI_DrawDropdown(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_DrawDropdown")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2551>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2552>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2553>");
	Float t_W=t_N->f_W;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2554>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2555>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2557>");
	int t_OffX=280;
	DBG_LOCAL(t_OffX,"OffX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2558>");
	int t_OffY=10;
	DBG_LOCAL(t_OffY,"OffY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2559>");
	if((t_N->f_Over)!=0){
		DBG_BLOCK();
		t_OffY=50;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2560>");
	if(((t_N->f_Down)!=0) || ((t_N->f_OnFocus)!=0)){
		DBG_BLOCK();
		t_OffY=90;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2561>");
	if(t_Active==0){
		DBG_BLOCK();
		t_OffY=130;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2564>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2566>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y,t_OffX+10,t_OffY,10,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(10.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2568>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y,t_OffX+30,t_OffY,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2570>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+FLOAT(10.0),t_OffX,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(10.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2572>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+FLOAT(10.0),t_OffX+30,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(20.0))/FLOAT(10.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2574>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+FLOAT(10.0),t_OffX+10,t_OffY+10,20,20,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(20.0),(t_H-FLOAT(20.0))/FLOAT(20.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2576>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-FLOAT(10.0),t_OffX,t_OffY+30,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2578>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+t_H-FLOAT(10.0),t_OffX+10,t_OffY+30,10,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(10.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2580>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),t_OffX+30,t_OffY+30,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2583>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_H,t_Y,t_OffX+40,t_OffY,40,40,FLOAT(0.0),t_H/FLOAT(40.0),t_H/FLOAT(40.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2586>");
	Float t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text))/FLOAT(2.0);
	DBG_LOCAL(t_YOff,"YOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2587>");
	bb_graphics_SetAlpha(FLOAT(0.25));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2588>");
	if((t_Active)!=0){
		DBG_BLOCK();
		bb_graphics_SetAlpha(FLOAT(1.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2589>");
	bb_challengergui_CHGUI_Font->m_DrawText2(t_N->f_Text,t_X+FLOAT(5.0),t_Y+t_YOff);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2590>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	return 0;
}
int bb_challengergui_CHGUI_DrawDropdownItem(bb_challengergui_CHGUI* t_N,int t_C){
	DBG_ENTER("CHGUI_DrawDropdownItem")
	DBG_LOCAL(t_N,"N")
	DBG_LOCAL(t_C,"C")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2597>");
	t_N->f_X=FLOAT(0.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2598>");
	t_N->f_Y=Float(t_C+1)*t_N->f_Parent->f_H;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2599>");
	t_N->f_W=t_N->f_Parent->f_W;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2600>");
	t_N->f_H=t_N->f_Parent->f_H;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2602>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2603>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2604>");
	Float t_W=t_N->f_Parent->f_W;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2605>");
	Float t_H=t_N->f_Parent->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2606>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2608>");
	int t_OffX=90;
	DBG_LOCAL(t_OffX,"OffX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2609>");
	int t_OffY=80;
	DBG_LOCAL(t_OffY,"OffY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2610>");
	int t_OffH=30;
	DBG_LOCAL(t_OffH,"OffH")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2612>");
	if((t_N->f_Over)!=0){
		DBG_BLOCK();
		t_OffY=110;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2613>");
	if(((t_N->f_Down)!=0) && ((bb_challengergui_CHGUI_RealActive(t_N))!=0)){
		DBG_BLOCK();
		t_OffY=140;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2615>");
	if(t_C!=0 && t_C!=t_N->f_Parent->f_DropNumber){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2617>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),t_H/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2619>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y,t_OffX+50,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),t_H/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2621>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y,t_OffX+10,t_OffY+10,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),t_H/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2623>");
		if((bb_challengergui_CHGUI_Shadow)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2624>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y,40,20,10,10,FLOAT(0.0),FLOAT(1.0),t_H/FLOAT(10.0),0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2625>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y,0,20,10,10,FLOAT(0.0),FLOAT(1.0),t_H/FLOAT(10.0),0);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2629>");
	if(t_C==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2631>");
		if((bb_challengergui_CHGUI_Shadow)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2632>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y+FLOAT(10.0),40,20,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(10.0))/FLOAT(10.0),0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2633>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y+FLOAT(10.0),0,20,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(10.0))/FLOAT(10.0),0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2635>");
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y-FLOAT(10.0),0,0,10,20,0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2637>");
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y-FLOAT(10.0),40,0,10,20,0);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2641>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2643>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y,t_OffX+10,t_OffY,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),FLOAT(1.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2645>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y,t_OffX+50,t_OffY,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2647>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+FLOAT(10.0),t_OffX,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(10.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2649>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+FLOAT(10.0),t_OffX+50,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(10.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2651>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+FLOAT(10.0),t_OffX+10,t_OffY+10,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),(t_H-FLOAT(10.0))/FLOAT(10.0),0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2654>");
	if(t_C==t_N->f_Parent->f_DropNumber){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2656>");
		if((bb_challengergui_CHGUI_Shadow)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2657>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y,40,20,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(10.0))/FLOAT(10.0),0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2658>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y,0,20,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(10.0))/FLOAT(10.0),0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2660>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+FLOAT(10.0),t_Y+t_H,20,40,10,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(10.0),FLOAT(1.0),0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2662>");
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),30,30,20,20,0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2664>");
			bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),0,30,20,20,0);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2667>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(10.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2669>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y,t_OffX+50,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(10.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2671>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y,t_OffX+10,t_OffY+10,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),(t_H-FLOAT(10.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2673>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+t_H-FLOAT(10.0),t_OffX,t_OffY+20,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2675>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+t_H-FLOAT(10.0),t_OffX+10,t_OffY+20,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),FLOAT(1.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2677>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),t_OffX+50,t_OffY+20,10,10,FLOAT(0.0),FLOAT(1.0),FLOAT(1.0),0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2681>");
	Float t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text))/FLOAT(2.0);
	DBG_LOCAL(t_YOff,"YOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2682>");
	bb_graphics_SetAlpha(FLOAT(0.25));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2683>");
	if((t_Active)!=0){
		DBG_BLOCK();
		bb_graphics_SetAlpha(FLOAT(1.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2684>");
	bb_challengergui_CHGUI_Font->m_DrawText2(t_N->f_Text,t_X+FLOAT(5.0),t_Y+t_YOff);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2685>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	return 0;
}
int bb_challengergui_CHGUI_DrawMenu(bb_challengergui_CHGUI* t_N,int t_XOffset,int t_C){
	DBG_ENTER("CHGUI_DrawMenu")
	DBG_LOCAL(t_N,"N")
	DBG_LOCAL(t_XOffset,"XOffset")
	DBG_LOCAL(t_C,"C")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2689>");
	if(t_C==0){
		DBG_BLOCK();
		t_XOffset=t_XOffset+1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2690>");
	t_N->f_X=Float(t_XOffset-t_C);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2691>");
	if(t_N->f_Parent!=bb_challengergui_CHGUI_Canvas && t_N->f_Parent->f_Text!=String()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2692>");
		t_N->f_Y=bb_challengergui_CHGUI_TitleHeight;
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2694>");
		t_N->f_Y=FLOAT(0.0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2696>");
	t_N->f_W=FLOAT(20.0)+bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2697>");
	t_N->f_H=FLOAT(10.0)+bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2698>");
	t_N->f_Parent->f_HasMenu=1;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2699>");
	t_N->f_Parent->f_MenuHeight=int(t_N->f_H);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2700>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2701>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2702>");
	Float t_W=t_N->f_W;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2703>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2704>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2705>");
	int t_OffX=100;
	DBG_LOCAL(t_OffX,"OffX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2706>");
	int t_OffY=90;
	DBG_LOCAL(t_OffY,"OffY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2707>");
	if((t_N->f_Over)!=0){
		DBG_BLOCK();
		t_OffY=120;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2708>");
	if(((t_N->f_Down)!=0) || ((t_N->f_OnFocus)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2709>");
		if(t_Active==1){
			DBG_BLOCK();
			t_OffY=150;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2713>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX,t_OffY,40,10,FLOAT(0.0),t_W/FLOAT(40.0),(t_H-FLOAT(10.0))/FLOAT(10.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2714>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y+(t_H-FLOAT(10.0)),t_OffX,t_OffY+10,40,10,FLOAT(0.0),t_W/FLOAT(40.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2717>");
	Float t_XOff=(t_W-bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text))/FLOAT(2.0);
	DBG_LOCAL(t_XOff,"XOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2718>");
	Float t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text))/FLOAT(2.0);
	DBG_LOCAL(t_YOff,"YOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2719>");
	bb_graphics_SetAlpha(FLOAT(0.25));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2720>");
	if((t_Active)!=0){
		DBG_BLOCK();
		bb_graphics_SetAlpha(FLOAT(1.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2721>");
	bb_challengergui_CHGUI_Font->m_DrawText2(t_N->f_Text,t_X+t_XOff,t_Y+t_YOff);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2722>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	return 0;
}
int bb_challengergui_CHGUI_DrawTab(bb_challengergui_CHGUI* t_N,int t_Offset){
	DBG_ENTER("CHGUI_DrawTab")
	DBG_LOCAL(t_N,"N")
	DBG_LOCAL(t_Offset,"Offset")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3005>");
	t_N->f_X=Float(t_Offset);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3006>");
	t_N->f_W=FLOAT(20.0)+bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3007>");
	t_N->f_H=FLOAT(10.0)+bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3008>");
	t_N->f_Parent->f_Tabbed=1;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3009>");
	t_N->f_Parent->f_TabHeight=int(t_N->f_H);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3010>");
	t_N->f_Y=FLOAT(5.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3011>");
	if(t_N->f_Parent->f_Text!=String()){
		DBG_BLOCK();
		t_N->f_Y=t_N->f_Y+bb_challengergui_CHGUI_TitleHeight;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3012>");
	if((t_N->f_Parent->f_HasMenu)!=0){
		DBG_BLOCK();
		t_N->f_Y=t_N->f_Y+Float(t_N->f_Parent->f_MenuHeight);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3014>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3015>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3016>");
	Float t_W=t_N->f_W;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3017>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3018>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3020>");
	int t_OffX=10;
	DBG_LOCAL(t_OffX,"OffX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3021>");
	int t_OffY=180;
	DBG_LOCAL(t_OffY,"OffY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3022>");
	if(t_N->f_Parent->f_CurrentTab==t_N){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3023>");
		t_OffX=70;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3024>");
		t_OffY=180;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3026>");
	if((t_N->f_Down)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3027>");
		t_OffX=10;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3028>");
		t_OffY=210;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3030>");
	if(((t_N->f_Over)!=0) && t_N->f_Down==0 && t_N->f_Parent->f_CurrentTab!=t_N){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3031>");
		t_OffX=40;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3032>");
		t_OffY=180;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3035>");
	if(t_Active==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3036>");
		t_OffX=40;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3037>");
		t_OffY=210;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3042>");
	int t_YY=int(t_Y);
	DBG_LOCAL(t_YY,"YY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3045>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,Float(t_YY),t_OffX,t_OffY,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3047>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),Float(t_YY),t_OffX+20,t_OffY,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3049>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),Float(t_YY),t_OffX+10,t_OffY,10,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(10.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3051>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,Float(t_YY+10),t_OffX,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),Float((t_N->f_Parent->f_TabHeight-10)/10),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3053>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),Float(t_YY+10),t_OffX+20,t_OffY+10,10,10,FLOAT(0.0),FLOAT(1.0),Float((t_N->f_Parent->f_TabHeight-10)/10),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3055>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),Float(t_YY+10),t_OffX+10,t_OffY+10,10,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(10.0),Float((t_N->f_Parent->f_TabHeight-10)/10),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3058>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,Float(t_YY+t_N->f_Parent->f_TabHeight-10),t_OffX,t_OffY+20,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3060>");
	bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),Float(t_YY+t_N->f_Parent->f_TabHeight-10),t_OffX+20,t_OffY+20,10,10,0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3062>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),Float(t_YY+t_N->f_Parent->f_TabHeight-10),t_OffX+10,t_OffY+20,10,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(10.0),FLOAT(1.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3065>");
	Float t_XOff=(t_W-bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text))/FLOAT(2.0);
	DBG_LOCAL(t_XOff,"XOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3066>");
	Float t_YOff=(Float(t_N->f_Parent->f_TabHeight)-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text))/FLOAT(2.0);
	DBG_LOCAL(t_YOff,"YOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3067>");
	bb_graphics_SetAlpha(FLOAT(0.25));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3068>");
	if((t_Active)!=0){
		DBG_BLOCK();
		bb_graphics_SetAlpha(FLOAT(1.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3069>");
	bb_challengergui_CHGUI_Font->m_DrawText2(t_N->f_Text,t_X+t_XOff,Float(t_YY)+t_YOff);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3070>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	return 0;
}
int bb_challengergui_CHGUI_DrawMenuItem(bb_challengergui_CHGUI* t_N,int t_C){
	DBG_ENTER("CHGUI_DrawMenuItem")
	DBG_LOCAL(t_N,"N")
	DBG_LOCAL(t_C,"C")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2728>");
	if(t_N->f_Parent->f_Element!=String(L"MenuItem",8)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2729>");
		t_N->f_X=FLOAT(0.0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2730>");
		t_N->f_Y=t_N->f_Parent->f_H+Float(t_C)*t_N->f_Parent->f_H-Float(t_C);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2731>");
		t_N->f_Parent->f_HasMenu=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2732>");
		t_N->f_Parent->f_MenuHeight=int(t_N->f_H);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2734>");
	if(t_N->f_Parent->f_Element==String(L"MenuItem",8)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2735>");
		t_N->f_X=t_N->f_Parent->f_W-FLOAT(1.0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2736>");
		t_N->f_Y=Float(t_C)*t_N->f_Parent->f_H-Float(t_C);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2739>");
	t_N->f_H=FLOAT(10.0)+bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2740>");
	t_N->f_W=FLOAT(20.0)+bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2742>");
	if((t_N->f_IsMenuParent)!=0){
		DBG_BLOCK();
		t_N->f_W=t_N->f_W+t_N->f_H/FLOAT(3.0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2743>");
	if((t_N->f_Tick)!=0){
		DBG_BLOCK();
		t_N->f_W=t_N->f_W+t_N->f_H/FLOAT(3.0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2745>");
	if(t_N->f_W>Float(t_N->f_Parent->f_MenuWidth)){
		DBG_BLOCK();
		t_N->f_Parent->f_MenuWidth=int(t_N->f_W);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2746>");
	if(Float(t_N->f_Parent->f_MenuWidth)>t_N->f_W){
		DBG_BLOCK();
		t_N->f_W=Float(t_N->f_Parent->f_MenuWidth);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2748>");
	if((t_N->f_Tick)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2749>");
		if(Float(t_N->f_Parent->f_MenuWidth)+t_N->f_H<t_N->f_W){
			DBG_BLOCK();
			t_N->f_Parent->f_MenuWidth=int(t_N->f_W+t_N->f_H);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2751>");
		if(Float(t_N->f_Parent->f_MenuWidth)<t_N->f_W){
			DBG_BLOCK();
			t_N->f_Parent->f_MenuWidth=int(t_N->f_W);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2754>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2755>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2756>");
	Float t_W=t_N->f_W;
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2757>");
	Float t_H=t_N->f_H;
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2758>");
	int t_Active=bb_challengergui_CHGUI_RealActive(t_N);
	DBG_LOCAL(t_Active,"Active")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2760>");
	int t_OffX=100;
	DBG_LOCAL(t_OffX,"OffX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2761>");
	int t_OffY=90;
	DBG_LOCAL(t_OffY,"OffY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2762>");
	if(((t_N->f_Over)!=0) || ((t_N->f_OnFocus)!=0)){
		DBG_BLOCK();
		t_OffY=120;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2763>");
	if(((t_N->f_Down)!=0) && ((bb_challengergui_CHGUI_RealActive(t_N))!=0)){
		DBG_BLOCK();
		t_OffY=150;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2766>");
	if(t_C==t_N->f_Parent->f_MenuNumber && ((bb_challengergui_CHGUI_Shadow)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2767>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y,40,20,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(10.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2768>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y,0,20,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(10.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2770>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+FLOAT(10.0),t_Y+t_H,20,40,10,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(10.0),FLOAT(1.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2772>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X+t_W-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),30,30,20,20,0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2774>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y+t_H-FLOAT(10.0),0,30,20,20,0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2778>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y,t_OffX,t_OffY,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),t_H/FLOAT(10.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2779>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX-10,t_OffY,10,10,FLOAT(0.0),FLOAT(1.0),t_H/FLOAT(10.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2780>");
	bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y,t_OffX+40,t_OffY,10,10,FLOAT(0.0),FLOAT(1.0),t_H/FLOAT(10.0),0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2783>");
	if(((bb_challengergui_CHGUI_Shadow)!=0) && t_C!=t_N->f_Parent->f_MenuNumber){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2785>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X+t_W,t_Y,40,20,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(1.0))/FLOAT(10.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2787>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_ShadowImg,t_X-FLOAT(10.0),t_Y,0,20,10,10,FLOAT(0.0),FLOAT(1.0),(t_H-FLOAT(1.0))/FLOAT(10.0),0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2791>");
	if(t_C==t_N->f_Parent->f_MenuNumber){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2792>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y+(t_H-FLOAT(10.0)),t_OffX-10,t_OffY+10,10,10,0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2793>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y+(t_H-FLOAT(10.0)),t_OffX,t_OffY+10,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),FLOAT(1.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2794>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y+(t_H-FLOAT(10.0)),t_OffX+40,t_OffY+10,10,10,0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2797>");
	if(t_C==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2798>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X,t_Y,t_OffX-10,t_OffY-10,10,10,0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2799>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(10.0),t_Y,t_OffX,t_OffY-10,40,10,FLOAT(0.0),(t_W-FLOAT(20.0))/FLOAT(40.0),FLOAT(1.0),0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2800>");
		bb_graphics_DrawImageRect(bb_challengergui_CHGUI_Style,t_X+t_W-FLOAT(10.0),t_Y,t_OffX+40,t_OffY-10,10,10,0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2804>");
	if((t_N->f_Tick)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2805>");
		int t_XOF=230;
		DBG_LOCAL(t_XOF,"XOF")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2806>");
		int t_YOF=10;
		DBG_LOCAL(t_YOF,"YOF")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2807>");
		if((t_N->f_Over)!=0){
			DBG_BLOCK();
			t_YOF=30;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2808>");
		if((t_N->f_Down)!=0){
			DBG_BLOCK();
			t_YOF=50;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2809>");
		if(bb_challengergui_CHGUI_RealActive(t_N)==0){
			DBG_BLOCK();
			t_XOF=70;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2810>");
		if(t_N->f_Value>FLOAT(0.0)){
			DBG_BLOCK();
			t_XOF=250;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2812>");
		bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+FLOAT(5.0),t_Y+(t_H-t_H/FLOAT(2.6))/FLOAT(2.0),t_XOF,t_YOF,20,20,FLOAT(0.0),t_H/FLOAT(2.6)/FLOAT(20.0),t_H/FLOAT(2.6)/FLOAT(20.0),0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2819>");
	Float t_YOff=(t_H-bb_challengergui_CHGUI_TextHeight(bb_challengergui_CHGUI_Font,t_N->f_Text))/FLOAT(2.0);
	DBG_LOCAL(t_YOff,"YOff")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2820>");
	bb_graphics_SetAlpha(FLOAT(0.25));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2821>");
	if((t_Active)!=0){
		DBG_BLOCK();
		bb_graphics_SetAlpha(FLOAT(1.0));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2822>");
	if(t_N->f_Tick==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2823>");
		bb_challengergui_CHGUI_Font->m_DrawText2(t_N->f_Text,t_X+FLOAT(10.0),t_Y+t_YOff);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2825>");
		bb_challengergui_CHGUI_Font->m_DrawText2(t_N->f_Text,t_X+FLOAT(10.0)+t_H/FLOAT(2.0),t_Y+t_YOff);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2827>");
	bb_graphics_SetAlpha(FLOAT(1.0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2830>");
	if((t_N->f_IsMenuParent)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2831>");
		if(bb_challengergui_CHGUI_RealActive(t_N)==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2832>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_H/FLOAT(4.0)-FLOAT(8.0),t_Y+(t_H-t_H/FLOAT(4.0))/FLOAT(2.0),130,180,10,10,FLOAT(0.0),t_H/FLOAT(4.0)/FLOAT(10.0),t_H/FLOAT(4.0)/FLOAT(10.0),0);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2834>");
			bb_graphics_DrawImageRect2(bb_challengergui_CHGUI_Style,t_X+t_W-t_H/FLOAT(4.0)-FLOAT(8.0),t_Y+(t_H-t_H/FLOAT(4.0))/FLOAT(2.0),140,180,10,10,FLOAT(0.0),t_H/FLOAT(4.0)/FLOAT(10.0),t_H/FLOAT(4.0)/FLOAT(10.0),0);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_SubMenu(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_SubMenu")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2840>");
	int t_C=0;
	DBG_LOCAL(t_C,"C")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2841>");
	int t_XX=0;
	DBG_LOCAL(t_XX,"XX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2842>");
	if((t_N->f_OnFocus)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2843>");
		t_C=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2844>");
		t_N->f_HasMenu=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2845>");
		for(t_XX=0;t_XX<=t_N->f_MenuItems.Length()-1;t_XX=t_XX+1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2846>");
			if(bb_challengergui_CHGUI_RealMinimised(t_N->f_MenuItems.At(t_XX))==0 && ((bb_challengergui_CHGUI_RealVisible(t_N->f_MenuItems.At(t_XX)))!=0)){
				DBG_BLOCK();
				bb_challengergui_CHGUI_DrawMenuItem(t_N->f_MenuItems.At(t_XX),t_C);
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2847>");
			if(bb_challengergui_CHGUI_RealMinimised(t_N->f_MenuItems.At(t_XX))==0 && ((bb_challengergui_CHGUI_RealVisible(t_N->f_MenuItems.At(t_XX)))!=0)){
				DBG_BLOCK();
				t_C=t_C+1;
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2849>");
		if(t_N->f_MenuItems.Length()>0){
			DBG_BLOCK();
			t_N->f_MenuNumber=t_C-1;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2850>");
		for(t_XX=0;t_XX<=t_N->f_MenuItems.Length()-1;t_XX=t_XX+1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2851>");
			if((t_N->f_MenuItems.At(t_XX)->f_IsMenuParent)!=0){
				DBG_BLOCK();
				bb_challengergui_CHGUI_SubMenu(t_N->f_MenuItems.At(t_XX));
			}
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_DrawContents(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_DrawContents")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2092>");
	int t_X=0;
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2093>");
	int t_XX=0;
	DBG_LOCAL(t_XX,"XX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2094>");
	int t_XOffset=0;
	DBG_LOCAL(t_XOffset,"XOffset")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2095>");
	int t_C=0;
	DBG_LOCAL(t_C,"C")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2098>");
	if(t_N->f_Element!=String(L"Tab",3)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2099>");
		if(t_N->f_Parent!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2100>");
			if(bb_challengergui_CHGUI_RealVisible(t_N)==1 && bb_challengergui_CHGUI_RealMinimised(t_N->f_Parent)==0){
				DBG_BLOCK();
				bb_challengergui_CHGUI_DrawWindow(t_N);
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2102>");
			bb_challengergui_CHGUI_DrawWindow(t_N);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2107>");
	for(t_X=0;t_X<=t_N->f_Buttons.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2108>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Buttons.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Buttons.At(t_X))==0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawButton(t_N->f_Buttons.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2112>");
	for(t_X=0;t_X<=t_N->f_ImageButtons.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2113>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_ImageButtons.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_ImageButtons.At(t_X))==0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawImageButton(t_N->f_ImageButtons.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2117>");
	for(t_X=0;t_X<=t_N->f_Tickboxes.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2118>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Tickboxes.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Tickboxes.At(t_X))==0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawTickbox(t_N->f_Tickboxes.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2122>");
	for(t_X=0;t_X<=t_N->f_Radioboxes.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2123>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Radioboxes.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Radioboxes.At(t_X))==0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawRadiobox(t_N->f_Radioboxes.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2127>");
	for(t_X=0;t_X<=t_N->f_Listboxes.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2128>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Listboxes.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Listboxes.At(t_X))==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2129>");
			bb_challengergui_CHGUI_DrawListbox(t_N->f_Listboxes.At(t_X));
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2131>");
			int t_C2=0;
			DBG_LOCAL(t_C2,"C")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2133>");
			for(t_XX=int(t_N->f_Listboxes.At(t_X)->f_ListboxSlider->f_Value);Float(t_XX)<=t_N->f_Listboxes.At(t_X)->f_ListboxSlider->f_Value+Float(t_N->f_Listboxes.At(t_X)->f_ListboxNumber);t_XX=t_XX+1){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2134>");
				if(t_XX<t_N->f_Listboxes.At(t_X)->f_ListboxItems.Length() && t_XX>-1){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2135>");
					if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Listboxes.At(t_X)->f_ListboxItems.At(t_XX)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Listboxes.At(t_X)->f_ListboxItems.At(t_XX))==0){
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2136>");
						bb_challengergui_CHGUI_DrawListboxItem(t_N->f_Listboxes.At(t_X)->f_ListboxItems.At(t_XX),t_C2);
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2137>");
						t_C2=t_C2+1;
					}
				}
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2145>");
	for(t_X=0;t_X<=t_N->f_HSliders.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2146>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_HSliders.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_HSliders.At(t_X))==0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawHSlider(t_N->f_HSliders.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2150>");
	for(t_X=0;t_X<=t_N->f_VSliders.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2151>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_VSliders.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_VSliders.At(t_X))==0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawVSlider(t_N->f_VSliders.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2155>");
	for(t_X=0;t_X<=t_N->f_Textfields.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2156>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Textfields.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Textfields.At(t_X))==0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawTextfield(t_N->f_Textfields.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2161>");
	for(t_X=0;t_X<=t_N->f_Labels.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2162>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Labels.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Labels.At(t_X))==0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawLabel(t_N->f_Labels.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2166>");
	for(t_X=0;t_X<=t_N->f_Dropdowns.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2167>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Dropdowns.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Dropdowns.At(t_X))==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2168>");
			bb_challengergui_CHGUI_DrawDropdown(t_N->f_Dropdowns.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2173>");
	for(t_X=0;t_X<=t_N->f_Dropdowns.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2174>");
		if(t_N->f_Dropdowns.At(t_X)->f_OnFocus==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2175>");
			t_C=0;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2176>");
			for(t_XX=0;t_XX<=t_N->f_Dropdowns.At(t_X)->f_DropdownItems.Length()-1;t_XX=t_XX+1){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2177>");
				if(bb_challengergui_CHGUI_RealMinimised(t_N->f_Dropdowns.At(t_X))==0 && ((bb_challengergui_CHGUI_RealVisible(t_N->f_Dropdowns.At(t_X)->f_DropdownItems.At(t_XX)))!=0)){
					DBG_BLOCK();
					bb_challengergui_CHGUI_DrawDropdownItem(t_N->f_Dropdowns.At(t_X)->f_DropdownItems.At(t_XX),t_C);
				}
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2178>");
				if(bb_challengergui_CHGUI_RealMinimised(t_N->f_Dropdowns.At(t_X))==0 && ((bb_challengergui_CHGUI_RealVisible(t_N->f_Dropdowns.At(t_X)->f_DropdownItems.At(t_XX)))!=0)){
					DBG_BLOCK();
					t_C=t_C+1;
				}
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2180>");
			if(t_N->f_Dropdowns.Length()>0){
				DBG_BLOCK();
				t_N->f_Dropdowns.At(t_X)->f_DropNumber=t_C-1;
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2185>");
	t_XOffset=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2186>");
	t_N->f_HasMenu=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2187>");
	t_C=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2188>");
	for(t_X=0;t_X<=t_N->f_Menus.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2189>");
		if(bb_challengergui_CHGUI_RealMinimised(t_N->f_Menus.At(t_X))==0 && ((bb_challengergui_CHGUI_RealVisible(t_N->f_Menus.At(t_X)))!=0)){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawMenu(t_N->f_Menus.At(t_X),t_XOffset,t_C);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2190>");
		if(bb_challengergui_CHGUI_RealMinimised(t_N->f_Menus.At(t_X))==0 && ((bb_challengergui_CHGUI_RealVisible(t_N->f_Menus.At(t_X)))!=0)){
			DBG_BLOCK();
			t_XOffset=int(Float(t_XOffset)+t_N->f_Menus.At(t_X)->f_W);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2191>");
		if(bb_challengergui_CHGUI_RealMinimised(t_N->f_Menus.At(t_X))==0 && ((bb_challengergui_CHGUI_RealVisible(t_N->f_Menus.At(t_X)))!=0)){
			DBG_BLOCK();
			t_C=t_C+1;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2195>");
	t_C=5;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2196>");
	t_N->f_Tabbed=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2197>");
	for(t_X=0;t_X<=t_N->f_Tabs.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2198>");
		if(bb_challengergui_CHGUI_RealMinimised(t_N->f_Tabs.At(t_X))==0 && ((bb_challengergui_CHGUI_RealVisible(t_N->f_Tabs.At(t_X)))!=0)){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawTab(t_N->f_Tabs.At(t_X),t_C);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2199>");
		if(bb_challengergui_CHGUI_RealMinimised(t_N->f_Tabs.At(t_X))==0 && ((bb_challengergui_CHGUI_RealVisible(t_N->f_Tabs.At(t_X)))!=0)){
			DBG_BLOCK();
			t_C=int(Float(t_C)+t_N->f_Tabs.At(t_X)->f_W);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2203>");
	for(int t_NN=0;t_NN<=t_N->f_BottomList.Length()-1;t_NN=t_NN+1){
		DBG_BLOCK();
		DBG_LOCAL(t_NN,"NN")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2204>");
		if((t_N->f_BottomList.At(t_NN)->f_Visible)!=0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawContents(t_N->f_BottomList.At(t_NN));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2206>");
	for(int t_NN2=0;t_NN2<=t_N->f_VariList.Length()-1;t_NN2=t_NN2+1){
		DBG_BLOCK();
		DBG_LOCAL(t_NN2,"NN")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2207>");
		if((t_N->f_VariList.At(t_NN2)->f_Visible)!=0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawContents(t_N->f_VariList.At(t_NN2));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2209>");
	for(int t_NN3=0;t_NN3<=t_N->f_TopList.Length()-1;t_NN3=t_NN3+1){
		DBG_BLOCK();
		DBG_LOCAL(t_NN3,"NN")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2210>");
		if((t_N->f_TopList.At(t_NN3)->f_Visible)!=0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawContents(t_N->f_TopList.At(t_NN3));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2213>");
	if((t_N->f_Tabbed)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2214>");
		bb_challengergui_CHGUI_DrawContents(t_N->f_CurrentTab);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2218>");
	for(t_X=0;t_X<=t_N->f_Menus.Length()-1;t_X=t_X+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2219>");
		bb_challengergui_CHGUI_SubMenu(t_N->f_Menus.At(t_X));
	}
	return 0;
}
Array<bb_challengergui_CHGUI* > bb_challengergui_CHGUI_VariList;
Array<bb_challengergui_CHGUI* > bb_challengergui_CHGUI_TopList;
bb_challengergui_CHGUI* bb_challengergui_CHGUI_TooltipFlag;
bb_bitmapfont_BitmapFont* bb_challengergui_CHGUI_TooltipFont;
int bb_graphics_DrawRect(Float t_x,Float t_y,Float t_w,Float t_h){
	DBG_ENTER("DrawRect")
	DBG_LOCAL(t_x,"x")
	DBG_LOCAL(t_y,"y")
	DBG_LOCAL(t_w,"w")
	DBG_LOCAL(t_h,"h")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<369>");
	bb_graphics_DebugRenderDevice();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<371>");
	bb_graphics_context->m_Validate();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/graphics.monkey<372>");
	bb_graphics_renderDevice->DrawRect(t_x,t_y,t_w,t_h);
	return 0;
}
int bb_challengergui_CHGUI_DrawTooltip(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_DrawTooltip")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3161>");
	int t_X=int(Float(bb_challengergui_CHGUI_RealX(t_N))+t_N->f_W);
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3162>");
	int t_Y=int(Float(bb_challengergui_CHGUI_RealY(t_N))+t_N->f_H);
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3163>");
	int t_W=int(bb_challengergui_CHGUI_TooltipFont->m_GetTxtWidth2(t_N->f_Tooltip)+FLOAT(10.0));
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3164>");
	int t_H=int(bb_challengergui_CHGUI_TooltipFont->m_GetTxtHeight(t_N->f_Tooltip)+FLOAT(10.0));
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3165>");
	if(t_X+t_W>bb_graphics_DeviceWidth()){
		DBG_BLOCK();
		t_X=bb_graphics_DeviceWidth()-t_W;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3166>");
	if(t_Y+t_H>bb_graphics_DeviceHeight()){
		DBG_BLOCK();
		t_Y=bb_challengergui_CHGUI_RealY(t_N)-t_H;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3167>");
	bb_graphics_SetColor(FLOAT(100.0),FLOAT(100.0),FLOAT(100.0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3168>");
	bb_graphics_DrawRect(Float(t_X),Float(t_Y),Float(t_W),Float(t_H));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3169>");
	bb_graphics_SetColor(FLOAT(250.0),FLOAT(250.0),FLOAT(210.0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3170>");
	bb_graphics_DrawRect(Float(t_X+1),Float(t_Y+1),Float(t_W-2),Float(t_H-2));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3171>");
	bb_graphics_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3172>");
	bb_challengergui_CHGUI_TooltipFont->m_DrawText2(t_N->f_Tooltip,Float(t_X+5),Float(t_Y+5));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3173>");
	bb_graphics_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
	return 0;
}
int bb_app_Millisecs(){
	DBG_ENTER("Millisecs")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<153>");
	int t_=bb_app_device->MilliSecs();
	return t_;
}
int bb_challengergui_CHGUI_Millisecs;
int bb_challengergui_CHGUI_FPSCounter;
int bb_challengergui_CHGUI_FPS;
int bb_challengergui_CHGUI_FPSUpdate(){
	DBG_ENTER("CHGUI_FPSUpdate")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4023>");
	if(bb_app_Millisecs()>bb_challengergui_CHGUI_Millisecs){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4024>");
		bb_challengergui_CHGUI_Millisecs=bb_app_Millisecs()+1000;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4025>");
		bb_challengergui_CHGUI_FPS=bb_challengergui_CHGUI_FPSCounter;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4026>");
		bb_challengergui_CHGUI_FPSCounter=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4028>");
	bb_challengergui_CHGUI_FPSCounter=bb_challengergui_CHGUI_FPSCounter+1;
	return 0;
}
int bb_challengergui_CHGUI_Draw(){
	DBG_ENTER("CHGUI_Draw")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<850>");
	int t_N=0;
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<851>");
	int t_NN=0;
	DBG_LOCAL(t_NN,"NN")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<853>");
	bb_graphics_SetBlend(0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<854>");
	bb_graphics_SetColor(FLOAT(255.0),FLOAT(255.0),FLOAT(255.0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<856>");
	for(t_N=0;t_N<=bb_challengergui_CHGUI_BottomList.Length()-1;t_N=t_N+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<857>");
		if((bb_challengergui_CHGUI_BottomList.At(t_N)->f_Visible)!=0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawContents(bb_challengergui_CHGUI_BottomList.At(t_N));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<859>");
	for(t_N=0;t_N<=bb_challengergui_CHGUI_VariList.Length()-1;t_N=t_N+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<860>");
		if((bb_challengergui_CHGUI_VariList.At(t_N)->f_Visible)!=0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawContents(bb_challengergui_CHGUI_VariList.At(t_N));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<862>");
	for(t_N=0;t_N<=bb_challengergui_CHGUI_TopList.Length()-1;t_N=t_N+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<863>");
		if((bb_challengergui_CHGUI_TopList.At(t_N)->f_Visible)!=0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DrawContents(bb_challengergui_CHGUI_TopList.At(t_N));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<866>");
	if(bb_challengergui_CHGUI_TooltipFlag!=0){
		DBG_BLOCK();
		bb_challengergui_CHGUI_DrawTooltip(bb_challengergui_CHGUI_TooltipFlag);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<868>");
	bb_challengergui_CHGUI_FPSUpdate();
	return 0;
}
int bb_challengergui_CHGUI_Width;
int bb_challengergui_CHGUI_Height;
int bb_challengergui_CHGUI_CanvasFlag;
int bb_challengergui_CHGUI_Started;
bb_challengergui_CHGUI* bb_challengergui_CHGUI_TopTop;
bb_challengergui_CHGUI* bb_challengergui_CreateWindow(int t_X,int t_Y,int t_W,int t_H,String t_Title,int t_Moveable,int t_CloseButton,int t_MinimiseButton,int t_Mode,bb_challengergui_CHGUI* t_Parent){
	DBG_ENTER("CreateWindow")
	DBG_LOCAL(t_X,"X")
	DBG_LOCAL(t_Y,"Y")
	DBG_LOCAL(t_W,"W")
	DBG_LOCAL(t_H,"H")
	DBG_LOCAL(t_Title,"Title")
	DBG_LOCAL(t_Moveable,"Moveable")
	DBG_LOCAL(t_CloseButton,"CloseButton")
	DBG_LOCAL(t_MinimiseButton,"MinimiseButton")
	DBG_LOCAL(t_Mode,"Mode")
	DBG_LOCAL(t_Parent,"Parent")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<377>");
	if(bb_challengergui_CHGUI_Started==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<378>");
		bb_challengergui_CHGUI_Started=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<379>");
		bb_challengergui_CHGUI_Start();
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<381>");
	bb_challengergui_CHGUI* t_N=(new bb_challengergui_CHGUI)->g_new();
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<382>");
	t_N->f_X=Float(t_X);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<383>");
	t_N->f_Y=Float(t_Y);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<384>");
	t_N->f_W=Float(t_W);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<385>");
	t_N->f_H=Float(t_H);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<386>");
	t_N->f_Text=t_Title;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<387>");
	t_N->f_Shadow=bb_challengergui_CHGUI_Shadow;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<388>");
	t_N->f_Close=t_CloseButton;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<389>");
	t_N->f_Minimise=t_MinimiseButton;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<390>");
	t_N->f_Moveable=t_Moveable;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<391>");
	t_N->f_Mode=t_Mode;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<392>");
	gc_assign(t_N->f_Parent,t_Parent);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<393>");
	if(t_N->f_Parent==0){
		DBG_BLOCK();
		gc_assign(t_N->f_Parent,bb_challengergui_CHGUI_Canvas);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<394>");
	if(t_N->f_Parent!=0){
		DBG_BLOCK();
		t_N->f_Parent->f_IsParent=1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<395>");
	t_N->f_Element=String(L"Window",6);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<396>");
	if(bb_challengergui_CHGUI_CanvasFlag==1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<397>");
		if(t_Mode==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<398>");
			gc_assign(bb_challengergui_CHGUI_BottomList,bb_challengergui_CHGUI_BottomList.Resize(bb_challengergui_CHGUI_BottomList.Length()+1));
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<399>");
			gc_assign(bb_challengergui_CHGUI_BottomList.At(bb_challengergui_CHGUI_BottomList.Length()-1),t_N);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<401>");
		if(t_Mode==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<402>");
			gc_assign(bb_challengergui_CHGUI_VariList,bb_challengergui_CHGUI_VariList.Resize(bb_challengergui_CHGUI_VariList.Length()+1));
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<403>");
			gc_assign(bb_challengergui_CHGUI_VariList.At(bb_challengergui_CHGUI_VariList.Length()-1),t_N);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<405>");
		if(t_Mode==2){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<406>");
			gc_assign(bb_challengergui_CHGUI_TopList,bb_challengergui_CHGUI_TopList.Resize(bb_challengergui_CHGUI_TopList.Length()+1));
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<407>");
			gc_assign(bb_challengergui_CHGUI_TopList.At(bb_challengergui_CHGUI_TopList.Length()-1),t_N);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<408>");
			gc_assign(bb_challengergui_CHGUI_TopTop,t_N);
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<411>");
		t_N->f_SubWindow=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<412>");
		if(t_Mode==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<413>");
			gc_assign(t_N->f_Parent->f_BottomList,t_N->f_Parent->f_BottomList.Resize(t_N->f_Parent->f_BottomList.Length()+1));
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<414>");
			gc_assign(t_N->f_Parent->f_BottomList.At(t_N->f_Parent->f_BottomList.Length()-1),t_N);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<416>");
		if(t_Mode==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<417>");
			gc_assign(t_N->f_Parent->f_VariList,t_N->f_Parent->f_VariList.Resize(t_N->f_Parent->f_VariList.Length()+1));
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<418>");
			gc_assign(t_N->f_Parent->f_VariList.At(t_N->f_Parent->f_VariList.Length()-1),t_N);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<420>");
		if(t_Mode==2){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<421>");
			gc_assign(t_N->f_Parent->f_TopList,t_N->f_Parent->f_TopList.Resize(t_N->f_Parent->f_TopList.Length()+1));
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<422>");
			gc_assign(t_N->f_Parent->f_TopList.At(t_N->f_Parent->f_TopList.Length()-1),t_N);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<426>");
	return t_N;
}
String bb_app_LoadString(String t_path){
	DBG_ENTER("LoadString")
	DBG_LOCAL(t_path,"path")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/app.monkey<141>");
	String t_=bb_app_device->LoadString(bb_data_FixDataPath(t_path));
	return t_;
}
bb_challengergui_CHGUI* bb_challengergui_CHGUI_KeyboardWindow;
bb_challengergui_CHGUI* bb_challengergui_CHGUI_CreateKeyButton(int t_X,int t_Y,int t_W,int t_H,String t_Text,bb_challengergui_CHGUI* t_Parent){
	DBG_ENTER("CHGUI_CreateKeyButton")
	DBG_LOCAL(t_X,"X")
	DBG_LOCAL(t_Y,"Y")
	DBG_LOCAL(t_W,"W")
	DBG_LOCAL(t_H,"H")
	DBG_LOCAL(t_Text,"Text")
	DBG_LOCAL(t_Parent,"Parent")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3209>");
	bb_challengergui_CHGUI* t_N=(new bb_challengergui_CHGUI)->g_new();
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3210>");
	gc_assign(t_N->f_Parent,t_Parent);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3211>");
	if(t_Parent==0){
		DBG_BLOCK();
		gc_assign(t_N->f_Parent,bb_challengergui_CHGUI_Canvas);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3212>");
	t_N->f_X=Float(t_X);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3213>");
	t_N->f_Y=Float(t_Y);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3214>");
	t_N->f_W=Float(t_W);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3215>");
	t_N->f_H=Float(t_H);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3216>");
	t_N->f_Text=t_Text;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3218>");
	gc_assign(t_N->f_Parent->f_Buttons,t_N->f_Parent->f_Buttons.Resize(t_N->f_Parent->f_Buttons.Length()+1));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3219>");
	gc_assign(t_N->f_Parent->f_Buttons.At(t_N->f_Parent->f_Buttons.Length()-1),t_N);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3220>");
	return t_N;
}
int bb_challengergui_CHGUI_CreateKeyboard(){
	DBG_ENTER("CHGUI_CreateKeyboard")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3224>");
	gc_assign(bb_challengergui_CHGUI_KeyboardWindow,bb_challengergui_CreateWindow(0,0,bb_graphics_DeviceWidth(),100,String(),0,0,0,2,0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3225>");
	Float t_KeyWidth=Float(bb_graphics_DeviceWidth())/FLOAT(12.5);
	DBG_LOCAL(t_KeyWidth,"KeyWidth")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3226>");
	Float t_KeyHeight=Float(bb_graphics_DeviceWidth())/FLOAT(12.5);
	DBG_LOCAL(t_KeyHeight,"KeyHeight")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3228>");
	Float t_GapX=t_KeyWidth*FLOAT(2.0)/FLOAT(9.0);
	DBG_LOCAL(t_GapX,"GapX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3229>");
	Float t_GapY=t_KeyWidth*FLOAT(2.0)/FLOAT(9.0);
	DBG_LOCAL(t_GapY,"GapY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3231>");
	if(bb_graphics_DeviceWidth()>bb_graphics_DeviceHeight()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3232>");
		t_KeyHeight=t_KeyHeight/FLOAT(1.7);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3233>");
		t_GapY=t_GapY/FLOAT(1.2);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3235>");
		t_KeyHeight=t_KeyHeight*FLOAT(1.5);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3236>");
		t_GapY=t_GapY*FLOAT(1.2);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3240>");
	Float t_EndGap=(Float(bb_graphics_DeviceWidth())-t_KeyWidth*FLOAT(10.0)-t_GapX*FLOAT(9.0))/FLOAT(2.0);
	DBG_LOCAL(t_EndGap,"EndGap")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3241>");
	bb_challengergui_CHGUI_KeyboardWindow->f_H=t_EndGap*FLOAT(2.0)+t_GapY*FLOAT(3.0)+t_KeyHeight*FLOAT(4.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3242>");
	bb_challengergui_CHGUI_KeyboardWindow->f_Y=Float(bb_graphics_DeviceHeight())-bb_challengergui_CHGUI_KeyboardWindow->f_H;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3245>");
	Float t_SX=t_EndGap;
	DBG_LOCAL(t_SX,"SX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3246>");
	Float t_SY=t_EndGap;
	DBG_LOCAL(t_SY,"SY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3249>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(0),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"q",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3250>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3251>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(1),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"w",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3252>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3253>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(2),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"e",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3254>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3255>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(3),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"r",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3256>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3257>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(4),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"t",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3258>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3259>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(5),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"y",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3260>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3261>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(6),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"u",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3262>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3263>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(7),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"i",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3264>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3265>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(8),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"o",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3266>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3267>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(9),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"p",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3268>");
	t_SX=t_EndGap+t_KeyWidth/FLOAT(2.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3269>");
	t_SY=t_SY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3270>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(10),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"a",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3271>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3272>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(11),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"s",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3273>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3274>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(12),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"d",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3275>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3276>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(13),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"f",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3277>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3278>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(14),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"g",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3279>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3280>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(15),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"h",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3281>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3282>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(16),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"j",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3283>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3284>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(17),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"k",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3285>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3286>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(18),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"l",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3287>");
	t_SX=t_EndGap+t_KeyWidth/FLOAT(2.0)+t_GapX+t_KeyWidth;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3288>");
	t_SY=t_SY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3289>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(19),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"z",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3290>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3291>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(20),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"x",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3292>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3293>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(21),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"c",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3294>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3295>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(22),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"v",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3296>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3297>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(23),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"b",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3298>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3299>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(24),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"n",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3300>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3301>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(25),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"m",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3303>");
	t_SX=t_EndGap;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3304>");
	t_SY=t_EndGap;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3307>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(26),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"Q",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3308>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3309>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(27),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"W",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3310>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3311>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(28),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"E",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3312>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3313>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(29),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"R",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3314>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3315>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(30),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"T",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3316>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3317>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(31),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"Y",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3318>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3319>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(32),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"U",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3320>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3321>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(33),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"I",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3322>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3323>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(34),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"O",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3324>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3325>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(35),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"P",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3326>");
	t_SX=t_EndGap+t_KeyWidth/FLOAT(2.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3327>");
	t_SY=t_SY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3328>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(36),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"A",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3329>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3330>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(37),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"S",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3331>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3332>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(38),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"D",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3333>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3334>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(39),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"F",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3335>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3336>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(40),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"G",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3337>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3338>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(41),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"H",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3339>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3340>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(42),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"J",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3341>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3342>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(43),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"K",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3343>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3344>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(44),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"L",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3345>");
	t_SX=t_EndGap+t_KeyWidth/FLOAT(2.0)+t_GapX+t_KeyWidth;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3346>");
	t_SY=t_SY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3347>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(45),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"Z",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3348>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3349>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(46),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"X",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3350>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3351>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(47),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"C",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3352>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3353>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(48),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"V",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3354>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3355>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(49),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"B",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3356>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3357>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(50),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"N",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3358>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3359>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(51),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"M",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3362>");
	t_SX=t_EndGap;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3363>");
	t_SY=t_EndGap;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3366>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(52),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"1",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3367>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3368>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(53),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"2",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3369>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3370>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(54),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"3",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3371>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3372>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(55),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"4",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3373>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3374>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(56),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"5",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3375>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3376>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(57),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"6",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3377>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3378>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(58),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"7",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3379>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3380>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(59),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"8",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3381>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3382>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(60),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"9",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3383>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3384>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(61),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"0",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3385>");
	t_SX=t_EndGap+t_KeyWidth/FLOAT(2.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3386>");
	t_SY=t_SY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3387>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(62),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"-",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3388>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3389>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(63),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"/",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3390>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3391>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(64),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"\\",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3392>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3393>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(65),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L":",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3394>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3395>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(66),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L";",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3396>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3397>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(67),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"(",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3398>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3399>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(68),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L")",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3400>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3401>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(69),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L""L"\xa3"L"",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3402>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3403>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(70),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"&",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3404>");
	t_SX=t_EndGap+t_KeyWidth/FLOAT(2.0)+t_GapX+t_KeyWidth;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3405>");
	t_SY=t_SY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3406>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(71),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"@",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3407>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3408>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(72),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L".",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3409>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3410>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(73),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L",",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3411>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3412>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(74),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"?",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3413>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3414>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(75),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"!",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3415>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3416>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(76),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"'",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3417>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3418>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(77),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String((Char)(34),1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3421>");
	t_SX=t_EndGap;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3422>");
	t_SY=t_EndGap;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3425>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(78),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"[",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3426>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3427>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(79),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"]",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3428>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3429>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(80),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"{",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3430>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3431>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(81),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"}",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3432>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3433>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(82),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"#",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3434>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3435>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(83),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"%",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3436>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3437>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(84),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"^",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3438>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3439>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(85),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"*",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3440>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3441>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(86),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"+",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3442>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3443>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(87),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"=",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3444>");
	t_SX=t_EndGap+t_KeyWidth/FLOAT(2.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3445>");
	t_SY=t_SY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3446>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(88),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"_",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3447>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3448>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(89),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"|",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3449>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3450>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(90),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"~",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3451>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3452>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(91),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"<",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3453>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3454>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(92),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L">",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3455>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3456>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(93),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"$",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3457>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3458>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(94),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L""L"\x20ac"L"",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3459>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3460>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(95),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L""L"\xe9"L"",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3461>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3462>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(96),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L""L"\xac"L"",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3463>");
	t_SX=t_EndGap+t_KeyWidth/FLOAT(2.0)+t_GapX+t_KeyWidth;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3464>");
	t_SY=t_SY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3465>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(97),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"@",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3466>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3467>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(98),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L".",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3468>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3469>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(99),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L",",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3470>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3471>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(100),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"?",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3472>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3473>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(101),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"!",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3474>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3475>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(102),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String(L"'",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3476>");
	t_SX=t_SX+t_KeyWidth+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3477>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(103),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth),int(t_KeyHeight),String((Char)(34),1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3480>");
	t_SX=t_EndGap;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3481>");
	t_SY=t_EndGap+t_KeyHeight+t_GapY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3482>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(104),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth+t_KeyWidth/FLOAT(2.0)),int(t_KeyHeight),String(L"Shft",4),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3483>");
	t_SX=t_EndGap+t_KeyWidth*FLOAT(9.0)+t_GapX*FLOAT(9.0)-t_KeyWidth/FLOAT(2.0)-t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3484>");
	t_SY=t_EndGap+t_KeyHeight+t_GapY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3485>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(105),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth+t_KeyWidth/FLOAT(2.0)),int(t_KeyHeight),String(L"<--",3),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3486>");
	t_SX=t_EndGap;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3487>");
	t_SY=t_SY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3488>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(106),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth*FLOAT(2.0)+t_KeyWidth/FLOAT(2.0)+t_GapX),int(t_KeyHeight),String(L"123",3),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3489>");
	t_SX=t_SX+t_KeyWidth*FLOAT(2.0)+t_KeyWidth/FLOAT(2.0)+t_GapX+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3490>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(107),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth*FLOAT(5.0)+t_GapX*FLOAT(4.0)),int(t_KeyHeight),String(L" ",1),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3491>");
	t_SX=t_SX+t_KeyWidth*FLOAT(5.0)+t_GapX*FLOAT(5.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3492>");
	gc_assign(bb_challengergui_CHGUI_KeyboardButtons.At(108),bb_challengergui_CHGUI_CreateKeyButton(int(t_SX),int(t_SY),int(t_KeyWidth*FLOAT(2.0)+t_KeyWidth/FLOAT(2.0)+t_GapX),int(t_KeyHeight),String(L"Enter",5),bb_challengergui_CHGUI_KeyboardWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3494>");
	for(int t_C=0;t_C<=108;t_C=t_C+1){
		DBG_BLOCK();
		DBG_LOCAL(t_C,"C")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3495>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C)->f_Visible=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3497>");
	bb_challengergui_CHGUI_KeyboardWindow->f_Visible=0;
	return 0;
}
bb_challengergui_CHGUI* bb_challengergui_CHGUI_MsgBoxWindow;
bb_challengergui_CHGUI* bb_challengergui_CreateLabel(int t_X,int t_Y,String t_Text,bb_challengergui_CHGUI* t_Parent){
	DBG_ENTER("CreateLabel")
	DBG_LOCAL(t_X,"X")
	DBG_LOCAL(t_Y,"Y")
	DBG_LOCAL(t_Text,"Text")
	DBG_LOCAL(t_Parent,"Parent")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<678>");
	if(bb_challengergui_CHGUI_Started==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<679>");
		bb_challengergui_CHGUI_Started=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<680>");
		bb_challengergui_CHGUI_Start();
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<682>");
	bb_challengergui_CHGUI* t_N=(new bb_challengergui_CHGUI)->g_new();
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<683>");
	gc_assign(t_N->f_Parent,t_Parent);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<684>");
	if(t_Parent==0){
		DBG_BLOCK();
		gc_assign(t_N->f_Parent,bb_challengergui_CHGUI_Canvas);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<685>");
	t_N->f_X=Float(t_X);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<686>");
	t_N->f_Y=Float(t_Y);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<687>");
	t_N->f_Text=t_Text;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<689>");
	t_N->f_Element=String(L"Label",5);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<691>");
	gc_assign(t_N->f_Parent->f_Labels,t_N->f_Parent->f_Labels.Resize(t_N->f_Parent->f_Labels.Length()+1));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<692>");
	gc_assign(t_N->f_Parent->f_Labels.At(t_N->f_Parent->f_Labels.Length()-1),t_N);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<693>");
	return t_N;
}
bb_challengergui_CHGUI* bb_challengergui_CHGUI_MsgBoxLabel;
bb_challengergui_CHGUI* bb_challengergui_CreateButton(int t_X,int t_Y,int t_W,int t_H,String t_Text,bb_challengergui_CHGUI* t_Parent){
	DBG_ENTER("CreateButton")
	DBG_LOCAL(t_X,"X")
	DBG_LOCAL(t_Y,"Y")
	DBG_LOCAL(t_W,"W")
	DBG_LOCAL(t_H,"H")
	DBG_LOCAL(t_Text,"Text")
	DBG_LOCAL(t_Parent,"Parent")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<431>");
	if(bb_challengergui_CHGUI_Started==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<432>");
		bb_challengergui_CHGUI_Started=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<433>");
		bb_challengergui_CHGUI_Start();
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<435>");
	bb_challengergui_CHGUI* t_N=(new bb_challengergui_CHGUI)->g_new();
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<436>");
	gc_assign(t_N->f_Parent,t_Parent);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<437>");
	if(t_Parent==0){
		DBG_BLOCK();
		gc_assign(t_N->f_Parent,bb_challengergui_CHGUI_Canvas);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<438>");
	t_N->f_X=Float(t_X);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<439>");
	t_N->f_Y=Float(t_Y);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<440>");
	t_N->f_W=Float(t_W);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<441>");
	t_N->f_H=Float(t_H);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<442>");
	t_N->f_Text=t_Text;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<443>");
	t_N->f_Element=String(L"Button",6);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<445>");
	gc_assign(t_N->f_Parent->f_Buttons,t_N->f_Parent->f_Buttons.Resize(t_N->f_Parent->f_Buttons.Length()+1));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<446>");
	gc_assign(t_N->f_Parent->f_Buttons.At(t_N->f_Parent->f_Buttons.Length()-1),t_N);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<447>");
	return t_N;
}
bb_challengergui_CHGUI* bb_challengergui_CHGUI_MsgBoxButton;
int bb_challengergui_CHGUI_Start(){
	DBG_ENTER("CHGUI_Start")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3180>");
	if(bb_challengergui_CHGUI_Width==0){
		DBG_BLOCK();
		bb_challengergui_CHGUI_Width=bb_graphics_DeviceWidth();
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3181>");
	if(bb_challengergui_CHGUI_Height==0){
		DBG_BLOCK();
		bb_challengergui_CHGUI_Height=bb_graphics_DeviceHeight();
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3182>");
	bb_challengergui_CHGUI_CanvasFlag=1;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3183>");
	gc_assign(bb_challengergui_CHGUI_Canvas,bb_challengergui_CreateWindow(0,0,bb_challengergui_CHGUI_Width,bb_challengergui_CHGUI_Height,String(),0,0,0,0,0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3184>");
	bb_challengergui_CHGUI_CanvasFlag=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3185>");
	if(bb_challengergui_CHGUI_Style==0){
		DBG_BLOCK();
		gc_assign(bb_challengergui_CHGUI_Style,bb_graphics_LoadImage(String(L"GUI_mac.png",11),1,bb_graphics_Image::g_DefaultFlags));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3186>");
	gc_assign(bb_challengergui_CHGUI_ShadowImg,bb_graphics_LoadImage(String(L"Shadow.png",10),1,bb_graphics_Image::g_DefaultFlags));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3187>");
	if(bb_challengergui_CHGUI_MobileMode==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3188>");
		if(bb_challengergui_CHGUI_TitleFont==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TitleFont,bb_bitmapfont_BitmapFont::g_Load(String(L"Arial10B.txt",12),true));
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3189>");
		if(bb_challengergui_CHGUI_Font==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_Font,bb_bitmapfont_BitmapFont::g_Load(String(L"Arial12.txt",11),true));
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3191>");
		if(bb_challengergui_CHGUI_TitleFont==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TitleFont,bb_bitmapfont_BitmapFont::g_Load(String(L"Arial20B.txt",12),true));
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3192>");
		if(bb_challengergui_CHGUI_Font==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_Font,bb_bitmapfont_BitmapFont::g_Load(String(L"Arial22.txt",11),true));
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3193>");
		bb_challengergui_CHGUI_TitleHeight=FLOAT(50.0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3195>");
	bb_challengergui_CHGUI_CreateKeyboard();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3196>");
	gc_assign(bb_challengergui_CHGUI_MsgBoxWindow,bb_challengergui_CreateWindow(100,100,200,100,String(L"Message box",11),0,0,0,2,0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3197>");
	gc_assign(bb_challengergui_CHGUI_MsgBoxLabel,bb_challengergui_CreateLabel(100,50,String(L"Message text",12),bb_challengergui_CHGUI_MsgBoxWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3198>");
	gc_assign(bb_challengergui_CHGUI_MsgBoxButton,bb_challengergui_CreateButton(150,70,100,25,String(L"Ok",2),bb_challengergui_CHGUI_MsgBoxWindow));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3199>");
	bb_challengergui_CHGUI_MsgBoxWindow->f_Visible=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3200>");
	gc_assign(bb_challengergui_CHGUI_TooltipFont,bb_bitmapfont_BitmapFont::g_Load(String(L"Arial10.txt",11),true));
	return 0;
}
Float bb_data2_SCALE_W;
Float bb_data2_SCALE_H;
bb_challengergui_CHGUI* bb_data2_CScale(bb_challengergui_CHGUI* t_c){
	DBG_ENTER("CScale")
	DBG_LOCAL(t_c,"c")
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<23>");
	t_c->f_X*=Float(bb_graphics_DeviceWidth())/bb_data2_SCALE_W;
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<24>");
	t_c->f_W*=Float(bb_graphics_DeviceWidth())/bb_data2_SCALE_W;
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<25>");
	t_c->f_Y*=Float(bb_graphics_DeviceHeight())/bb_data2_SCALE_H;
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<26>");
	t_c->f_H*=Float(bb_graphics_DeviceHeight())/bb_data2_SCALE_H;
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/data.monkey<27>");
	return t_c;
}
bb_challengergui_CHGUI* bb_challengergui_CreateDropdown(int t_X,int t_Y,int t_W,int t_H,String t_Text,bb_challengergui_CHGUI* t_Parent){
	DBG_ENTER("CreateDropdown")
	DBG_LOCAL(t_X,"X")
	DBG_LOCAL(t_Y,"Y")
	DBG_LOCAL(t_W,"W")
	DBG_LOCAL(t_H,"H")
	DBG_LOCAL(t_Text,"Text")
	DBG_LOCAL(t_Parent,"Parent")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<494>");
	if(bb_challengergui_CHGUI_Started==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<495>");
		bb_challengergui_CHGUI_Started=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<496>");
		bb_challengergui_CHGUI_Start();
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<498>");
	bb_challengergui_CHGUI* t_N=(new bb_challengergui_CHGUI)->g_new();
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<499>");
	gc_assign(t_N->f_Parent,t_Parent);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<500>");
	if(t_N->f_Parent==0){
		DBG_BLOCK();
		gc_assign(t_N->f_Parent,bb_challengergui_CHGUI_Canvas);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<501>");
	t_N->f_X=Float(t_X);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<502>");
	t_N->f_Y=Float(t_Y);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<503>");
	t_N->f_H=Float(t_H);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<504>");
	t_N->f_W=Float(t_W);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<505>");
	t_N->f_Text=t_Text;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<506>");
	t_N->f_Element=String(L"Dropdown",8);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<508>");
	gc_assign(t_N->f_Parent->f_Dropdowns,t_N->f_Parent->f_Dropdowns.Resize(t_N->f_Parent->f_Dropdowns.Length()+1));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<509>");
	gc_assign(t_N->f_Parent->f_Dropdowns.At(t_N->f_Parent->f_Dropdowns.Length()-1),t_N);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<510>");
	return t_N;
}
bb_challengergui_CHGUI* bb_challengergui_CreateTextfield(int t_X,int t_Y,int t_W,int t_H,String t_Text,bb_challengergui_CHGUI* t_Parent){
	DBG_ENTER("CreateTextfield")
	DBG_LOCAL(t_X,"X")
	DBG_LOCAL(t_Y,"Y")
	DBG_LOCAL(t_W,"W")
	DBG_LOCAL(t_H,"H")
	DBG_LOCAL(t_Text,"Text")
	DBG_LOCAL(t_Parent,"Parent")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<634>");
	if(bb_challengergui_CHGUI_Started==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<635>");
		bb_challengergui_CHGUI_Started=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<636>");
		bb_challengergui_CHGUI_Start();
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<638>");
	bb_challengergui_CHGUI* t_N=(new bb_challengergui_CHGUI)->g_new();
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<639>");
	gc_assign(t_N->f_Parent,t_Parent);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<640>");
	if(t_Parent==0){
		DBG_BLOCK();
		gc_assign(t_N->f_Parent,bb_challengergui_CHGUI_Canvas);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<641>");
	t_N->f_X=Float(t_X);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<642>");
	t_N->f_Y=Float(t_Y);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<643>");
	t_N->f_W=Float(t_W);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<644>");
	t_N->f_H=Float(t_H);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<645>");
	t_N->f_Text=t_Text;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<646>");
	t_N->f_Element=String(L"Textfield",9);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<648>");
	gc_assign(t_N->f_Parent->f_Textfields,t_N->f_Parent->f_Textfields.Resize(t_N->f_Parent->f_Textfields.Length()+1));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<649>");
	gc_assign(t_N->f_Parent->f_Textfields.At(t_N->f_Parent->f_Textfields.Length()-1),t_N);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<650>");
	return t_N;
}
int bb_input_TouchDown(int t_index){
	DBG_ENTER("TouchDown")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<93>");
	int t_=bb_input_device->KeyDown(384+t_index);
	return t_;
}
int bb_challengergui_CHGUI_MouseBusy;
int bb_challengergui_CHGUI_Over;
int bb_challengergui_CHGUI_OverFlag;
int bb_challengergui_CHGUI_DownFlag;
int bb_challengergui_CHGUI_MenuOver;
int bb_challengergui_CHGUI_TextBoxOver;
int bb_challengergui_CHGUI_TextboxOnFocus;
int bb_challengergui_CHGUI_TextBoxDown;
int bb_challengergui_CHGUI_DragOver;
int bb_challengergui_CHGUI_Moving;
Float bb_challengergui_CHGUI_TargetY;
Float bb_challengergui_CHGUI_TargetX;
int bb_challengergui_CHGUI_IgnoreMouse;
Float bb_input_TouchX(int t_index){
	DBG_ENTER("TouchX")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<85>");
	Float t_=bb_input_device->TouchX(t_index);
	return t_;
}
Float bb_input_TouchY(int t_index){
	DBG_ENTER("TouchY")
	DBG_LOCAL(t_index,"index")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<89>");
	Float t_=bb_input_device->TouchY(t_index);
	return t_;
}
int bb_challengergui_CHGUI_ReorderSubWindows(bb_challengergui_CHGUI* t_Top){
	DBG_ENTER("CHGUI_ReorderSubWindows")
	DBG_LOCAL(t_Top,"Top")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3984>");
	if(t_Top->f_Parent->f_TopVari!=t_Top && t_Top->f_Mode==1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3985>");
		int t_N=0;
		DBG_LOCAL(t_N,"N")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3986>");
		for(t_N=0;t_N<=t_Top->f_Parent->f_VariList.Length()-1;t_N=t_N+1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3987>");
			if(t_Top->f_Parent->f_VariList.At(t_N)==t_Top){
				DBG_BLOCK();
				break;
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3989>");
		for(int t_NN=t_N;t_NN<=t_Top->f_Parent->f_VariList.Length()-2;t_NN=t_NN+1){
			DBG_BLOCK();
			DBG_LOCAL(t_NN,"NN")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3990>");
			gc_assign(t_Top->f_Parent->f_VariList.At(t_NN),t_Top->f_Parent->f_VariList.At(t_NN+1));
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3992>");
		gc_assign(t_Top->f_Parent->f_VariList.At(t_Top->f_Parent->f_VariList.Length()-1),t_Top);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3993>");
		gc_assign(t_Top->f_Parent->f_TopVari,t_Top);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3996>");
	if(t_Top->f_Parent->f_TopTop!=t_Top && t_Top->f_Mode==2){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3997>");
		int t_N2=0;
		DBG_LOCAL(t_N2,"N")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3998>");
		for(t_N2=0;t_N2<=t_Top->f_Parent->f_TopList.Length()-1;t_N2=t_N2+1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3999>");
			if(t_Top->f_Parent->f_TopList.At(t_N2)==t_Top){
				DBG_BLOCK();
				break;
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4001>");
		for(int t_NN2=t_N2;t_NN2<=t_Top->f_Parent->f_TopList.Length()-2;t_NN2=t_NN2+1){
			DBG_BLOCK();
			DBG_LOCAL(t_NN2,"NN")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4002>");
			gc_assign(t_Top->f_Parent->f_TopList.At(t_NN2),t_Top->f_Parent->f_TopList.At(t_NN2+1));
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4004>");
		gc_assign(t_Top->f_Parent->f_TopList.At(t_Top->f_Parent->f_TopList.Length()-1),t_Top);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4005>");
		gc_assign(t_Top->f_Parent->f_TopTop,t_Top);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4008>");
	if(t_Top->f_Parent->f_TopBottom!=t_Top && t_Top->f_Mode==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4009>");
		int t_N3=0;
		DBG_LOCAL(t_N3,"N")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4010>");
		for(t_N3=0;t_N3<=t_Top->f_Parent->f_BottomList.Length()-1;t_N3=t_N3+1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4011>");
			if(t_Top->f_Parent->f_BottomList.At(t_N3)==t_Top){
				DBG_BLOCK();
				break;
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4013>");
		for(int t_NN3=t_N3;t_NN3<=t_Top->f_Parent->f_BottomList.Length()-2;t_NN3=t_NN3+1){
			DBG_BLOCK();
			DBG_LOCAL(t_NN3,"NN")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4014>");
			gc_assign(t_Top->f_Parent->f_BottomList.At(t_NN3),t_Top->f_Parent->f_BottomList.At(t_NN3+1));
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4016>");
		gc_assign(t_Top->f_Parent->f_BottomList.At(t_Top->f_Parent->f_BottomList.Length()-1),t_Top);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4017>");
		gc_assign(t_Top->f_Parent->f_TopBottom,t_Top);
	}
	return 0;
}
int bb_challengergui_CHGUI_Reorder(bb_challengergui_CHGUI* t_Top){
	DBG_ENTER("CHGUI_Reorder")
	DBG_LOCAL(t_Top,"Top")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3979>");
	if(t_Top->f_Mode==1){
		DBG_BLOCK();
		bb_challengergui_CHGUI_ReorderSubWindows(t_Top);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3980>");
	if(t_Top->f_Mode==2){
		DBG_BLOCK();
		bb_challengergui_CHGUI_ReorderSubWindows(t_Top);
	}
	return 0;
}
int bb_challengergui_CHGUI_CloseMenu(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_CloseMenu")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1614>");
	t_N->f_OnFocus=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1615>");
	bb_challengergui_CHGUI* t_E=t_N;
	DBG_LOCAL(t_E,"E")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1616>");
	do{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1617>");
		if(t_E->f_Parent->f_Element==String(L"Window",6)){
			DBG_BLOCK();
			t_E->f_Parent->f_MenuActive=0;
			break;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1618>");
		if(t_E->f_Parent->f_Element==String(L"MenuItem",8)){
			DBG_BLOCK();
			t_E->f_Parent->f_OnFocus=0;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1619>");
		if(t_E->f_Parent->f_Element==String(L"Menu",4)){
			DBG_BLOCK();
			t_E->f_Parent->f_OnFocus=0;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1621>");
		t_E=t_E->f_Parent;
	}while(!(false));
	return 0;
}
int bb_challengergui_CHGUI_CloseMenuReverse(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_CloseMenuReverse")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1626>");
	t_N->f_OnFocus=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1627>");
	bb_challengergui_CHGUI* t_E=t_N;
	DBG_LOCAL(t_E,"E")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1628>");
	int t_C=0;
	DBG_LOCAL(t_C,"C")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1629>");
	for(t_C=0;t_C<=t_E->f_MenuItems.Length()-1;t_C=t_C+1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1630>");
		if(t_E->f_MenuItems.Length()>0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1631>");
			bb_challengergui_CHGUI_CloseMenuReverse(t_E->f_MenuItems.At(t_C));
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1632>");
			t_E->f_MenuItems.At(t_C)->f_OnFocus=0;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1634>");
			break;
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_Tooltips;
int bb_challengergui_CHGUI_TooltipTime;
int bb_challengergui_CHGUI_MenuClose;
int bb_challengergui_CHGUI_UpdateMenuItem(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateMenuItem")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1566>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1567>");
	if((bb_challengergui_CHGUI_RealActive(t_N))!=0){
		DBG_BLOCK();
		t_N->m_CheckDown();
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1568>");
	if(bb_challengergui_CHGUI_RealVisible(t_N)==0){
		DBG_BLOCK();
		bb_challengergui_CHGUI_CloseMenu(t_N);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1570>");
	if((t_N->f_Over)!=0){
		DBG_BLOCK();
		bb_challengergui_CHGUI_MenuOver=1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1572>");
	if(((t_N->f_IsMenuParent)!=0) && ((t_N->f_Over)!=0) && ((bb_challengergui_CHGUI_RealActive(t_N))!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1573>");
		t_N->f_OnFocus=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1574>");
		if(t_N->f_Parent->f_MenuOver!=0 && t_N!=t_N->f_Parent->f_MenuOver){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1575>");
			t_N->f_Parent->f_MenuOver->f_OnFocus=0;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1576>");
			bb_challengergui_CHGUI_CloseMenuReverse(t_N->f_Parent->f_MenuOver);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1578>");
		gc_assign(t_N->f_Parent->f_MenuOver,t_N);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1581>");
	if(t_N->f_Parent->f_MenuOver!=0 && ((t_N->f_Over)!=0) && t_N!=t_N->f_Parent->f_MenuOver){
		DBG_BLOCK();
		t_N->f_Parent->f_MenuOver->f_OnFocus=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1583>");
	if((t_N->f_Clicked)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1584>");
		if(t_N->f_IsMenuParent==0 && t_N->f_Tick==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1585>");
			bb_challengergui_CHGUI_CloseMenu(t_N);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1587>");
		if((t_N->f_Tick)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1588>");
			if(t_N->f_Value==FLOAT(0.0)){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1589>");
				t_N->f_Value=FLOAT(1.0);
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1591>");
				t_N->f_Value=FLOAT(0.0);
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1596>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1597>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1599>");
	if((bb_challengergui_CHGUI_MenuClose)!=0){
		DBG_BLOCK();
		bb_challengergui_CHGUI_CloseMenu(t_N);
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateSubMenu(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateSubMenu")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1603>");
	int t_XX=0;
	DBG_LOCAL(t_XX,"XX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1604>");
	for(t_XX=t_N->f_MenuItems.Length()-1;t_XX>=0;t_XX=t_XX+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1605>");
		t_N->f_MenuItems.At(t_XX)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1606>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_MenuItems.At(t_XX)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_MenuItems.At(t_XX))==0 && ((t_N->f_OnFocus)!=0)){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateMenuItem(t_N->f_MenuItems.At(t_XX));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1608>");
	for(t_XX=t_N->f_MenuItems.Length()-1;t_XX>=0;t_XX=t_XX+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1609>");
		if((t_N->f_MenuItems.At(t_XX)->f_IsMenuParent)!=0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateSubMenu(t_N->f_MenuItems.At(t_XX));
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateTab(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateTab")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2028>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2029>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2031>");
	if((t_N->f_Clicked)!=0){
		DBG_BLOCK();
		gc_assign(t_N->f_Parent->f_CurrentTab,t_N);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2033>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2034>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateMenu(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateMenu")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1534>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1535>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1536>");
	if((t_N->f_Over)!=0){
		DBG_BLOCK();
		bb_challengergui_CHGUI_MenuOver=1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1538>");
	if((t_N->f_Clicked)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1539>");
		if(t_N->f_Parent->f_MenuActive!=t_N){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1540>");
			if(t_N->f_MenuItems.Length()>0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1541>");
				t_N->f_OnFocus=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1542>");
				gc_assign(t_N->f_Parent->f_MenuActive,t_N);
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1545>");
			t_N->f_OnFocus=0;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1546>");
			bb_challengergui_CHGUI_CloseMenuReverse(t_N->f_Parent->f_MenuActive);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1547>");
			t_N->f_Parent->f_MenuActive=0;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1552>");
	if(((t_N->f_OnFocus)!=0) && t_N->f_MenuItems.Length()<1){
		DBG_BLOCK();
		t_N->f_OnFocus=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1553>");
	if(((t_N->f_Over)!=0) && t_N->f_Parent->f_MenuActive!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1554>");
		t_N->f_Parent->f_MenuActive->f_OnFocus=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1555>");
		bb_challengergui_CHGUI_CloseMenuReverse(t_N->f_Parent->f_MenuActive);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1556>");
		gc_assign(t_N->f_Parent->f_MenuActive,t_N);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1557>");
		t_N->f_OnFocus=1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1560>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1561>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateDropdownItem(bb_challengergui_CHGUI* t_N,int t_C){
	DBG_ENTER("CHGUI_UpdateDropdownItem")
	DBG_LOCAL(t_N,"N")
	DBG_LOCAL(t_C,"C")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1506>");
	t_N->f_H=t_N->f_Parent->f_H;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1507>");
	t_N->f_W=t_N->f_Parent->f_W;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1508>");
	t_N->f_X=FLOAT(0.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1509>");
	t_N->f_Y=t_N->f_Parent->f_H+Float(t_C)*t_N->f_H-Float(t_C)-FLOAT(1.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1511>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1512>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1514>");
	if((t_N->f_Clicked)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1515>");
		t_N->f_Parent->f_Text=t_N->f_Text;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1516>");
		t_N->f_Parent->f_Value=t_N->f_Value;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1517>");
		t_N->f_Parent->f_OnFocus=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1520>");
	if((bb_input_TouchDown(0))!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1521>");
		if(bb_input_TouchX(0)<Float(bb_challengergui_CHGUI_RealX(t_N->f_Parent)) || bb_input_TouchX(0)>Float(bb_challengergui_CHGUI_RealX(t_N->f_Parent))+t_N->f_W || bb_input_TouchY(0)<Float(bb_challengergui_CHGUI_RealY(t_N->f_Parent)) || bb_input_TouchY(0)>Float(bb_challengergui_CHGUI_RealY(t_N->f_Parent))+t_N->f_H*Float(t_N->f_Parent->f_DropdownItems.Length()+1)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1522>");
			t_N->f_Parent->f_OnFocus=0;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1523>");
			t_N->f_Parent->f_Over=0;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1524>");
			t_N->f_Parent->f_Down=0;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1528>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1529>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateDropdown(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateDropdown")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1489>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1490>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1492>");
	if((t_N->f_Clicked)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1493>");
		if(t_N->f_OnFocus==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1494>");
			t_N->f_OnFocus=0;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1496>");
			t_N->f_OnFocus=1;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1500>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1501>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateLabel(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateLabel")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2081>");
	t_N->f_W=bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2082>");
	t_N->f_H=bb_challengergui_CHGUI_Font->m_GetTxtHeight(t_N->f_Text);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2084>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2085>");
	t_N->m_CheckDown();
	return 0;
}
bb_challengergui_CHGUI* bb_challengergui_CHGUI_TextboxFocus;
int bb_challengergui_CHGUI_Keyboard;
int bb_input_EnableKeyboard(){
	DBG_ENTER("EnableKeyboard")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<41>");
	int t_=bb_input_device->SetKeyboardEnabled(1);
	return t_;
}
int bb_challengergui_CHGUI_ShowKeyboard;
int bb_challengergui_CHGUI_AutoTextScroll;
int bb_input_GetChar(){
	DBG_ENTER("GetChar")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<57>");
	int t_=bb_input_device->GetChar();
	return t_;
}
int bb_challengergui_CHGUI_GetText(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_GetText")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1957>");
	String t_Before=t_N->f_Text.Slice(0,t_N->f_Cursor);
	DBG_LOCAL(t_Before,"Before")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1958>");
	String t_After=t_N->f_Text.Slice(t_N->f_Cursor);
	DBG_LOCAL(t_After,"After")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1959>");
	int t_In=bb_input_GetChar();
	DBG_LOCAL(t_In,"In")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1962>");
	if(t_In>96 && t_In<123 && t_N->f_FormatText==1 && bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text)<t_N->f_W-FLOAT(12.0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1963>");
		t_Before=t_Before+String((Char)(t_In),1);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1964>");
		t_N->f_Cursor=t_N->f_Cursor+1;
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1967>");
		if(t_In>64 && t_In<91 && t_N->f_FormatText==1 && bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text)<t_N->f_W-FLOAT(12.0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1968>");
			t_Before=t_Before+String((Char)(t_In),1);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1969>");
			t_N->f_Cursor=t_N->f_Cursor+1;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1972>");
			if(t_In>45 && t_In<58 && t_N->f_FormatNumber==1 && bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text)<t_N->f_W-FLOAT(12.0)){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1973>");
				if(t_In!=47 || t_N->f_FormatSymbol==1){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1974>");
					t_Before=t_Before+String((Char)(t_In),1);
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1975>");
					t_N->f_Cursor=t_N->f_Cursor+1;
				}
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1978>");
				if(t_In>32 && t_In<48 && t_N->f_FormatSymbol==1 && bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text)<t_N->f_W-FLOAT(12.0)){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1979>");
					t_Before=t_Before+String((Char)(t_In),1);
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1980>");
					t_N->f_Cursor=t_N->f_Cursor+1;
				}else{
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1983>");
					if(t_In>57 && t_In<65 && t_N->f_FormatSymbol==1 && bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text)<t_N->f_W-FLOAT(12.0)){
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1984>");
						t_Before=t_Before+String((Char)(t_In),1);
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1985>");
						t_N->f_Cursor=t_N->f_Cursor+1;
					}else{
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1988>");
						if(t_In>90 && t_In<97 && t_N->f_FormatSymbol==1 && bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text)<t_N->f_W-FLOAT(12.0)){
							DBG_BLOCK();
							DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1989>");
							t_Before=t_Before+String((Char)(t_In),1);
							DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1990>");
							t_N->f_Cursor=t_N->f_Cursor+1;
						}else{
							DBG_BLOCK();
							DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1993>");
							if(t_In>122 && t_In<127 && t_N->f_FormatSymbol==1 && bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text)<t_N->f_W-FLOAT(12.0)){
								DBG_BLOCK();
								DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1994>");
								t_Before=t_Before+String((Char)(t_In),1);
								DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1995>");
								t_N->f_Cursor=t_N->f_Cursor+1;
							}else{
								DBG_BLOCK();
								DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1998>");
								if(t_In==32 && t_N->f_FormatSpace==1 && bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text)<t_N->f_W-FLOAT(12.0)){
									DBG_BLOCK();
									DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1999>");
									t_Before=t_Before+String((Char)(t_In),1);
									DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2000>");
									t_N->f_Cursor=t_N->f_Cursor+1;
								}else{
									DBG_BLOCK();
									DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2002>");
									if(t_In==8){
										DBG_BLOCK();
										DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2004>");
										t_Before=t_Before.Slice(0,t_Before.Length()-1);
										DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2005>");
										if(t_N->f_Cursor>0){
											DBG_BLOCK();
											t_N->f_Cursor=t_N->f_Cursor-1;
										}
									}else{
										DBG_BLOCK();
										DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2007>");
										if(t_In==127){
											DBG_BLOCK();
											DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2009>");
											t_After=t_After.Slice(1);
										}else{
											DBG_BLOCK();
											DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2011>");
											if(t_In==65575){
												DBG_BLOCK();
												DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2013>");
												if(t_N->f_Cursor<t_N->f_Text.Length()){
													DBG_BLOCK();
													t_N->f_Cursor=t_N->f_Cursor+1;
												}
											}else{
												DBG_BLOCK();
												DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2015>");
												if(t_In==65573){
													DBG_BLOCK();
													DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2017>");
													if(t_N->f_Cursor>0){
														DBG_BLOCK();
														t_N->f_Cursor=t_N->f_Cursor-1;
													}
												}else{
													DBG_BLOCK();
													DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2019>");
													if(t_In==13){
														DBG_BLOCK();
														DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2021>");
														t_N->f_OnFocus=0;
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
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2024>");
	t_N->f_Text=t_Before+t_After;
	return 0;
}
int bb_challengergui_CHGUI_UpdateKeyboardSizes(){
	DBG_ENTER("CHGUI_UpdateKeyboardSizes")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3725>");
	Float t_KeyWidth=Float(bb_graphics_DeviceWidth())/FLOAT(12.5);
	DBG_LOCAL(t_KeyWidth,"KeyWidth")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3726>");
	Float t_KeyHeight=Float(bb_graphics_DeviceWidth())/FLOAT(12.5);
	DBG_LOCAL(t_KeyHeight,"KeyHeight")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3728>");
	Float t_GapX=t_KeyWidth*FLOAT(2.0)/FLOAT(9.0);
	DBG_LOCAL(t_GapX,"GapX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3729>");
	Float t_GapY=t_KeyWidth*FLOAT(2.0)/FLOAT(9.0);
	DBG_LOCAL(t_GapY,"GapY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3731>");
	if(bb_graphics_DeviceWidth()>bb_graphics_DeviceHeight()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3732>");
		t_KeyHeight=t_KeyHeight/FLOAT(1.7);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3733>");
		t_GapY=t_GapY/FLOAT(1.2);
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3735>");
		t_KeyHeight=t_KeyHeight*FLOAT(1.5);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3736>");
		t_GapY=t_GapY*FLOAT(1.2);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3740>");
	Float t_EndGap=(Float(bb_graphics_DeviceWidth())-t_KeyWidth*FLOAT(10.0)-t_GapX*FLOAT(9.0))/FLOAT(2.0);
	DBG_LOCAL(t_EndGap,"EndGap")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3742>");
	bb_challengergui_CHGUI_KeyboardWindow->f_H=t_EndGap*FLOAT(2.0)+t_GapY*FLOAT(3.0)+t_KeyHeight*FLOAT(4.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3743>");
	bb_challengergui_CHGUI_KeyboardWindow->f_Y=Float(bb_graphics_DeviceHeight())-bb_challengergui_CHGUI_KeyboardWindow->f_H;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3744>");
	bb_challengergui_CHGUI_KeyboardWindow->f_W=Float(bb_graphics_DeviceWidth());
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3746>");
	Float t_SX=t_EndGap;
	DBG_LOCAL(t_SX,"SX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3747>");
	Float t_SY=t_EndGap;
	DBG_LOCAL(t_SY,"SY")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3749>");
	for(int t_C=0;t_C<=9;t_C=t_C+1){
		DBG_BLOCK();
		DBG_LOCAL(t_C,"C")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3750>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C)->f_X=t_SX;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3751>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C)->f_Y=t_SY;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3752>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C)->f_W=t_KeyWidth;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3753>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C)->f_H=t_KeyHeight;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3754>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C+26)->f_X=t_SX;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3755>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C+26)->f_Y=t_SY;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3756>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C+26)->f_W=t_KeyWidth;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3757>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C+26)->f_H=t_KeyHeight;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3758>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C+52)->f_X=t_SX;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3759>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C+52)->f_Y=t_SY;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3760>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C+52)->f_W=t_KeyWidth;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3761>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C+52)->f_H=t_KeyHeight;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3762>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C+78)->f_X=t_SX;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3763>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C+78)->f_Y=t_SY;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3764>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C+78)->f_W=t_KeyWidth;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3765>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C+78)->f_H=t_KeyHeight;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3766>");
		t_SX=t_SX+t_GapX+t_KeyWidth;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3769>");
	t_SX=t_EndGap+t_KeyWidth/FLOAT(2.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3770>");
	t_SY=t_SY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3772>");
	for(int t_C2=10;t_C2<=18;t_C2=t_C2+1){
		DBG_BLOCK();
		DBG_LOCAL(t_C2,"C")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3773>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2)->f_X=t_SX;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3774>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2)->f_Y=t_SY;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3775>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2)->f_W=t_KeyWidth;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3776>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2)->f_H=t_KeyHeight;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3777>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2+26)->f_X=t_SX;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3778>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2+26)->f_Y=t_SY;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3779>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2+26)->f_W=t_KeyWidth;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3780>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2+26)->f_H=t_KeyHeight;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3781>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2+52)->f_X=t_SX;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3782>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2+52)->f_Y=t_SY;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3783>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2+52)->f_W=t_KeyWidth;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3784>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2+52)->f_H=t_KeyHeight;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3785>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2+78)->f_X=t_SX;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3786>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2+78)->f_Y=t_SY;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3787>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2+78)->f_W=t_KeyWidth;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3788>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C2+78)->f_H=t_KeyHeight;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3789>");
		t_SX=t_SX+t_GapX+t_KeyWidth;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3792>");
	t_SX=t_EndGap+t_KeyWidth/FLOAT(2.0)+t_GapX+t_KeyWidth;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3793>");
	t_SY=t_SY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3795>");
	for(int t_C3=19;t_C3<=25;t_C3=t_C3+1){
		DBG_BLOCK();
		DBG_LOCAL(t_C3,"C")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3796>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3)->f_X=t_SX;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3797>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3)->f_Y=t_SY;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3798>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3)->f_W=t_KeyWidth;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3799>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3)->f_H=t_KeyHeight;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3800>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3+26)->f_X=t_SX;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3801>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3+26)->f_Y=t_SY;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3802>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3+26)->f_W=t_KeyWidth;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3803>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3+26)->f_H=t_KeyHeight;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3804>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3+52)->f_X=t_SX;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3805>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3+52)->f_Y=t_SY;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3806>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3+52)->f_W=t_KeyWidth;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3807>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3+52)->f_H=t_KeyHeight;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3808>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3+78)->f_X=t_SX;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3809>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3+78)->f_Y=t_SY;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3810>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3+78)->f_W=t_KeyWidth;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3811>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C3+78)->f_H=t_KeyHeight;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3812>");
		t_SX=t_SX+t_GapX+t_KeyWidth;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3815>");
	t_SX=t_EndGap;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3816>");
	t_SY=t_EndGap+t_KeyHeight+t_GapY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3817>");
	bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_X=t_SX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3818>");
	bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_Y=t_SY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3819>");
	bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_W=t_KeyWidth+t_KeyWidth/FLOAT(2.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3820>");
	bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_H=t_KeyHeight;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3823>");
	t_SX=t_EndGap+t_KeyWidth*FLOAT(9.0)+t_GapX*FLOAT(9.0)-t_KeyWidth/FLOAT(2.0)-t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3824>");
	t_SY=t_EndGap+t_KeyHeight+t_GapY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3825>");
	bb_challengergui_CHGUI_KeyboardButtons.At(105)->f_X=t_SX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3826>");
	bb_challengergui_CHGUI_KeyboardButtons.At(105)->f_Y=t_SY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3827>");
	bb_challengergui_CHGUI_KeyboardButtons.At(105)->f_W=t_KeyWidth+t_KeyWidth/FLOAT(2.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3828>");
	bb_challengergui_CHGUI_KeyboardButtons.At(105)->f_H=t_KeyHeight;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3830>");
	t_SX=t_EndGap;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3831>");
	t_SY=t_SY+t_KeyHeight+t_GapY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3832>");
	bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_X=t_SX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3833>");
	bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_Y=t_SY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3834>");
	bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_W=t_KeyWidth*FLOAT(2.0)+t_KeyWidth/FLOAT(2.0)+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3835>");
	bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_H=t_KeyHeight;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3837>");
	t_SX=t_SX+t_KeyWidth*FLOAT(2.0)+t_KeyWidth/FLOAT(2.0)+t_GapX+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3838>");
	bb_challengergui_CHGUI_KeyboardButtons.At(107)->f_X=t_SX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3839>");
	bb_challengergui_CHGUI_KeyboardButtons.At(107)->f_Y=t_SY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3840>");
	bb_challengergui_CHGUI_KeyboardButtons.At(107)->f_W=t_KeyWidth*FLOAT(5.0)+t_GapX*FLOAT(4.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3841>");
	bb_challengergui_CHGUI_KeyboardButtons.At(107)->f_H=t_KeyHeight;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3843>");
	t_SX=t_SX+t_KeyWidth*FLOAT(5.0)+t_GapX*FLOAT(5.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3844>");
	bb_challengergui_CHGUI_KeyboardButtons.At(108)->f_X=t_SX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3845>");
	bb_challengergui_CHGUI_KeyboardButtons.At(108)->f_Y=t_SY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3846>");
	bb_challengergui_CHGUI_KeyboardButtons.At(108)->f_W=t_KeyWidth*FLOAT(2.0)+t_KeyWidth/FLOAT(2.0)+t_GapX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3847>");
	bb_challengergui_CHGUI_KeyboardButtons.At(108)->f_H=t_KeyHeight;
	return 0;
}
int bb_challengergui_CHGUI_KeyboardPage;
int bb_challengergui_CHGUI_KeyboardShift;
Float bb_challengergui_CHGUI_OldX;
Float bb_challengergui_CHGUI_OldY;
int bb_challengergui_CHGUI_UpdateKeyboard(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateKeyboard")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3502>");
	bb_challengergui_CHGUI_Reorder(bb_challengergui_CHGUI_KeyboardWindow);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3504>");
	bb_challengergui_CHGUI_UpdateKeyboardSizes();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3507>");
	bb_challengergui_CHGUI_KeyboardWindow->f_X=FLOAT(0.0)-bb_challengergui_CHGUI_OffsetX;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3508>");
	bb_challengergui_CHGUI_KeyboardWindow->f_Y=Float(bb_graphics_DeviceHeight())-bb_challengergui_CHGUI_KeyboardWindow->f_H-bb_challengergui_CHGUI_OffsetY;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3511>");
	if(bb_challengergui_CHGUI_KeyboardPage>1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3512>");
		bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_Active=0;
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3514>");
		bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_Active=1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3519>");
	for(int t_C=0;t_C<=108;t_C=t_C+1){
		DBG_BLOCK();
		DBG_LOCAL(t_C,"C")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3520>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C)->f_Active=1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3523>");
	bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_Active=1;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3525>");
	bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_Active=1;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3527>");
	bb_challengergui_CHGUI_KeyboardButtons.At(107)->f_Active=1;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3530>");
	if(t_N->f_FormatSpace==0){
		DBG_BLOCK();
		bb_challengergui_CHGUI_KeyboardButtons.At(107)->f_Active=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3534>");
	if(t_N->f_FormatText==1 && t_N->f_FormatNumber==0 && t_N->f_FormatSymbol==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3535>");
		if(bb_challengergui_CHGUI_KeyboardPage>1){
			DBG_BLOCK();
			bb_challengergui_CHGUI_KeyboardPage=0;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3536>");
		bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_Active=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3539>");
	if(t_N->f_FormatNumber==1 && t_N->f_FormatText==0 && t_N->f_FormatSymbol==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3540>");
		bb_challengergui_CHGUI_KeyboardPage=2;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3541>");
		bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_Active=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3542>");
		bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_Active=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3543>");
		for(int t_C2=62;t_C2<=77;t_C2=t_C2+1){
			DBG_BLOCK();
			DBG_LOCAL(t_C2,"C")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3544>");
			bb_challengergui_CHGUI_KeyboardButtons.At(t_C2)->f_Active=0;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3548>");
	if(t_N->f_FormatNumber==0 && t_N->f_FormatText==0 && t_N->f_FormatSymbol==1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3549>");
		if(bb_challengergui_CHGUI_KeyboardPage<2){
			DBG_BLOCK();
			bb_challengergui_CHGUI_KeyboardPage=2;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3550>");
		bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_Active=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3551>");
		for(int t_C3=52;t_C3<=61;t_C3=t_C3+1){
			DBG_BLOCK();
			DBG_LOCAL(t_C3,"C")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3552>");
			bb_challengergui_CHGUI_KeyboardButtons.At(t_C3)->f_Active=0;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3556>");
	if(t_N->f_FormatText==1 && t_N->f_FormatNumber==1 && t_N->f_FormatSymbol==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3557>");
		if(bb_challengergui_CHGUI_KeyboardPage>2){
			DBG_BLOCK();
			bb_challengergui_CHGUI_KeyboardPage=0;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3558>");
		for(int t_C4=62;t_C4<=77;t_C4=t_C4+1){
			DBG_BLOCK();
			DBG_LOCAL(t_C4,"C")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3559>");
			bb_challengergui_CHGUI_KeyboardButtons.At(t_C4)->f_Active=0;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3563>");
	if(t_N->f_FormatText==1 && t_N->f_FormatNumber==0 && t_N->f_FormatSymbol==1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3564>");
		for(int t_C5=52;t_C5<=61;t_C5=t_C5+1){
			DBG_BLOCK();
			DBG_LOCAL(t_C5,"C")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3565>");
			bb_challengergui_CHGUI_KeyboardButtons.At(t_C5)->f_Active=0;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3569>");
	if(t_N->f_FormatText==0 && t_N->f_FormatNumber==1 && t_N->f_FormatSymbol==1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3570>");
		if(bb_challengergui_CHGUI_KeyboardPage<2){
			DBG_BLOCK();
			bb_challengergui_CHGUI_KeyboardPage=2;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3571>");
		bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_Active=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3576>");
	if(bb_challengergui_CHGUI_KeyboardPage>1){
		DBG_BLOCK();
		bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_Active=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3583>");
	if(bb_challengergui_CHGUI_KeyboardPage<2){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3584>");
		if((bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_Clicked)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3585>");
			if(bb_challengergui_CHGUI_KeyboardShift==0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3586>");
				bb_challengergui_CHGUI_KeyboardShift=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3587>");
				bb_challengergui_CHGUI_KeyboardPage=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3588>");
				bb_challengergui_CHGUI_ShiftHold=0;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3590>");
				bb_challengergui_CHGUI_KeyboardShift=0;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3591>");
				bb_challengergui_CHGUI_KeyboardPage=0;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3592>");
				bb_challengergui_CHGUI_ShiftHold=0;
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3596>");
		if((bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_DoubleClicked)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3597>");
			bb_challengergui_CHGUI_ShiftHold=1;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3598>");
			bb_challengergui_CHGUI_KeyboardShift=1;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3599>");
			bb_challengergui_CHGUI_KeyboardPage=1;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3604>");
	if((bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_Clicked)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3605>");
		if(bb_challengergui_CHGUI_KeyboardPage==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3606>");
			bb_challengergui_CHGUI_KeyboardPage=2;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3607>");
			bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_Active=0;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3608>");
			if(bb_challengergui_CHGUI_KeyboardPage==1){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3609>");
				bb_challengergui_CHGUI_KeyboardPage=2;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3610>");
				bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_Active=0;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3611>");
				if(bb_challengergui_CHGUI_KeyboardPage==2){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3612>");
					bb_challengergui_CHGUI_KeyboardPage=3;
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3613>");
					bb_challengergui_CHGUI_KeyboardButtons.At(104)->f_Active=0;
				}else{
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3614>");
					if(bb_challengergui_CHGUI_KeyboardPage==3){
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3615>");
						bb_challengergui_CHGUI_KeyboardPage=0;
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3616>");
						bb_challengergui_CHGUI_KeyboardShift=0;
					}
				}
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3622>");
	if(bb_challengergui_CHGUI_KeyboardPage==0 || bb_challengergui_CHGUI_KeyboardPage==1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3623>");
		if(t_N->f_FormatNumber==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3624>");
			bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_Text=String(L"123",3);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3626>");
			bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_Text=String(L"#+=",3);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3630>");
	if(bb_challengergui_CHGUI_KeyboardPage==2){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3631>");
		if(t_N->f_FormatSymbol==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3632>");
			bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_Text=String(L"#+=",3);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3634>");
			bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_Text=String(L"Abc",3);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3638>");
	if(bb_challengergui_CHGUI_KeyboardPage==3){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3639>");
		if(t_N->f_FormatText==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3640>");
			bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_Text=String(L"#+=",3);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3641>");
			if(t_N->f_FormatNumber==1){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3642>");
				bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_Text=String(L"123",3);
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3644>");
				bb_challengergui_CHGUI_KeyboardButtons.At(106)->f_Text=String(L"#+=",3);
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3651>");
	for(int t_C6=0;t_C6<=108;t_C6=t_C6+1){
		DBG_BLOCK();
		DBG_LOCAL(t_C6,"C")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3652>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C6)->f_Visible=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3655>");
	if(bb_challengergui_CHGUI_KeyboardPage==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3656>");
		for(int t_C7=0;t_C7<=25;t_C7=t_C7+1){
			DBG_BLOCK();
			DBG_LOCAL(t_C7,"C")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3657>");
			bb_challengergui_CHGUI_KeyboardButtons.At(t_C7)->f_Visible=1;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3660>");
	if(bb_challengergui_CHGUI_KeyboardPage==1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3661>");
		for(int t_C8=26;t_C8<=51;t_C8=t_C8+1){
			DBG_BLOCK();
			DBG_LOCAL(t_C8,"C")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3662>");
			bb_challengergui_CHGUI_KeyboardButtons.At(t_C8)->f_Visible=1;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3665>");
	if(bb_challengergui_CHGUI_KeyboardPage==2){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3666>");
		for(int t_C9=52;t_C9<=77;t_C9=t_C9+1){
			DBG_BLOCK();
			DBG_LOCAL(t_C9,"C")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3667>");
			bb_challengergui_CHGUI_KeyboardButtons.At(t_C9)->f_Visible=1;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3670>");
	if(bb_challengergui_CHGUI_KeyboardPage==3){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3671>");
		for(int t_C10=78;t_C10<=103;t_C10=t_C10+1){
			DBG_BLOCK();
			DBG_LOCAL(t_C10,"C")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3672>");
			bb_challengergui_CHGUI_KeyboardButtons.At(t_C10)->f_Visible=1;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3675>");
	for(int t_C11=104;t_C11<=108;t_C11=t_C11+1){
		DBG_BLOCK();
		DBG_LOCAL(t_C11,"C")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3676>");
		bb_challengergui_CHGUI_KeyboardButtons.At(t_C11)->f_Visible=1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3681>");
	String t_Before=t_N->f_Text.Slice(0,t_N->f_Cursor);
	DBG_LOCAL(t_Before,"Before")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3682>");
	String t_After=t_N->f_Text.Slice(t_N->f_Cursor);
	DBG_LOCAL(t_After,"After")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3685>");
	for(int t_C1=0;t_C1<=103;t_C1=t_C1+1){
		DBG_BLOCK();
		DBG_LOCAL(t_C1,"C1")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3686>");
		if(((bb_challengergui_CHGUI_KeyboardButtons.At(t_C1)->f_Clicked)!=0) && bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text)<t_N->f_W-FLOAT(12.0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3687>");
			t_Before=t_Before+bb_challengergui_CHGUI_KeyboardButtons.At(t_C1)->f_Text;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3688>");
			t_N->f_Cursor=t_N->f_Cursor+1;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3689>");
			if(bb_challengergui_CHGUI_ShiftHold==0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3690>");
				if(bb_challengergui_CHGUI_KeyboardPage==0 || bb_challengergui_CHGUI_KeyboardPage==1){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3691>");
					bb_challengergui_CHGUI_KeyboardShift=0;
					bb_challengergui_CHGUI_KeyboardPage=0;
				}
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3697>");
	if((bb_challengergui_CHGUI_KeyboardButtons.At(105)->f_Down)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3698>");
		if(bb_challengergui_CHGUI_KeyboardButtons.At(105)->f_DKeyMillisecs<bb_app_Millisecs()){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3699>");
			bb_challengergui_CHGUI_KeyboardButtons.At(105)->f_DKeyMillisecs=bb_app_Millisecs()+150;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3700>");
			t_Before=t_Before.Slice(0,t_Before.Length()-1);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3701>");
			if(t_N->f_Cursor>0){
				DBG_BLOCK();
				t_N->f_Cursor=t_N->f_Cursor-1;
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3705>");
	if(((bb_challengergui_CHGUI_KeyboardButtons.At(107)->f_Clicked)!=0) && bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text)<t_N->f_W-FLOAT(12.0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3706>");
		t_Before=t_Before+String(L" ",1);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3707>");
		t_N->f_Cursor=t_N->f_Cursor+1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3710>");
	if((bb_challengergui_CHGUI_KeyboardButtons.At(108)->f_Clicked)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3711>");
		t_N->f_OnFocus=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3712>");
		bb_challengergui_CHGUI_KeyboardWindow->f_Visible=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3713>");
		bb_challengergui_CHGUI_KeyboardPage=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3714>");
		if(bb_challengergui_CHGUI_AutoTextScroll==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3715>");
			bb_challengergui_CHGUI_TargetX=bb_challengergui_CHGUI_OldX;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3716>");
			bb_challengergui_CHGUI_TargetY=bb_challengergui_CHGUI_OldY;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3717>");
			bb_challengergui_CHGUI_Moving=1;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3721>");
	t_N->f_Text=t_Before+t_After;
	return 0;
}
int bb_input_DisableKeyboard(){
	DBG_ENTER("DisableKeyboard")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/mojo/input.monkey<45>");
	int t_=bb_input_device->SetKeyboardEnabled(0);
	return t_;
}
int bb_challengergui_CHGUI_UpdateTextfield(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateTextfield")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1873>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1874>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1876>");
	if((t_N->f_Over)!=0){
		DBG_BLOCK();
		bb_challengergui_CHGUI_TextBoxOver=1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1877>");
	if((t_N->f_Down)!=0){
		DBG_BLOCK();
		bb_challengergui_CHGUI_TextBoxDown=1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1878>");
	if((t_N->f_OnFocus)!=0){
		DBG_BLOCK();
		bb_challengergui_CHGUI_TextboxOnFocus=1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1881>");
	if((t_N->f_Clicked)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1882>");
		gc_assign(bb_challengergui_CHGUI_TextboxFocus,t_N);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1883>");
		if(t_N->f_OnFocus==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1884>");
			t_N->f_Cursor=t_N->f_Text.Length();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1885>");
			t_N->f_OnFocus=1;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1886>");
			if(bb_challengergui_CHGUI_Keyboard==1){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1887>");
				bb_input_EnableKeyboard();
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1888>");
				if(bb_challengergui_CHGUI_Keyboard==2){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1889>");
					bb_challengergui_CHGUI_ShowKeyboard=1;
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1890>");
					bb_challengergui_CHGUI_KeyboardWindow->f_Visible=1;
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1891>");
					bb_challengergui_CHGUI_ShiftHold=0;
				}
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1894>");
			if((bb_challengergui_CHGUI_AutoTextScroll)!=0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1895>");
				bb_challengergui_CHGUI_TargetX=Float(-bb_challengergui_CHGUI_RealX(t_N))+bb_challengergui_CHGUI_OffsetX+FLOAT(100.0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1896>");
				bb_challengergui_CHGUI_TargetY=Float(-bb_challengergui_CHGUI_RealY(t_N))+bb_challengergui_CHGUI_OffsetY+FLOAT(100.0);
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1897>");
				bb_challengergui_CHGUI_Moving=1;
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1903>");
			int t_C=0;
			DBG_LOCAL(t_C,"C")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1904>");
			for(t_C=0;t_C<=t_N->f_Text.Length()-1;t_C=t_C+1){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1905>");
				if(bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text.Slice(0,t_C)+String(L"NON",3))-bb_challengergui_CHGUI_Font->m_GetTxtWidth2(String(L"NON",3))>bb_input_TouchX(0)-Float(bb_challengergui_CHGUI_RealX(t_N))-FLOAT(10.0)){
					DBG_BLOCK();
					break;
				}
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1907>");
			t_N->f_Cursor=t_C;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1912>");
	if((t_N->f_OnFocus)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1913>");
		if(bb_challengergui_CHGUI_Keyboard==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1914>");
			bb_challengergui_CHGUI_GetText(t_N);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1915>");
			if(bb_challengergui_CHGUI_Keyboard==2){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1916>");
				bb_challengergui_CHGUI_UpdateKeyboard(t_N);
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1918>");
				bb_challengergui_CHGUI_GetText(t_N);
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1927>");
	if(((bb_input_TouchDown(0))!=0) && t_N->f_Over==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1928>");
		if(bb_challengergui_CHGUI_Keyboard==2){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1929>");
			if(bb_input_TouchY(0)<Float(bb_graphics_DeviceHeight())-bb_challengergui_CHGUI_KeyboardWindow->f_H){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1930>");
				t_N->f_OnFocus=0;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1931>");
				if(bb_challengergui_CHGUI_Keyboard==1){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1932>");
					bb_input_DisableKeyboard();
				}else{
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1933>");
					if(bb_challengergui_CHGUI_Keyboard==2){
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1934>");
						bb_challengergui_CHGUI_ShowKeyboard=0;
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1935>");
						bb_challengergui_CHGUI_KeyboardWindow->f_Visible=0;
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1936>");
						bb_challengergui_CHGUI_KeyboardPage=0;
					}
				}
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1940>");
			t_N->f_OnFocus=0;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1941>");
			if(bb_challengergui_CHGUI_Keyboard==1){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1942>");
				bb_input_DisableKeyboard();
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1943>");
				if(bb_challengergui_CHGUI_Keyboard==2){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1944>");
					bb_challengergui_CHGUI_ShowKeyboard=0;
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1945>");
					bb_challengergui_CHGUI_KeyboardWindow->f_Visible=0;
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1946>");
					bb_challengergui_CHGUI_KeyboardPage=0;
				}
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1951>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1952>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateHSlider(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateHSlider")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1640>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1641>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1642>");
	if(t_N->f_Value<t_N->f_Minimum){
		DBG_BLOCK();
		t_N->f_Value=t_N->f_Minimum;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1643>");
	if(t_N->f_Value>t_N->f_Maximum){
		DBG_BLOCK();
		t_N->f_Value=t_N->f_Maximum;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1644>");
	int t_X=bb_challengergui_CHGUI_RealX(t_N);
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1645>");
	int t_Y=bb_challengergui_CHGUI_RealY(t_N);
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1646>");
	int t_W=int(t_N->f_W);
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1647>");
	int t_H=int(t_N->f_H);
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1650>");
	t_N->f_Stp=(t_N->f_W-FLOAT(2.0)*t_N->f_H)/(t_N->f_Maximum-t_N->f_Minimum);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1651>");
	t_N->f_SWidth=t_N->f_Stp;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1652>");
	if(t_N->f_SWidth<t_N->f_H){
		DBG_BLOCK();
		t_N->f_SWidth=t_N->f_H;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1653>");
	if(t_N->f_SWidth>t_N->f_W-t_N->f_H-t_N->f_H-FLOAT(10.0)){
		DBG_BLOCK();
		t_N->f_SWidth=t_N->f_W-t_N->f_H-t_N->f_H-FLOAT(10.0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1654>");
	t_N->f_Stp=(t_N->f_W-t_N->f_SWidth-t_N->f_H-t_N->f_H)/(t_N->f_Maximum-t_N->f_Minimum);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1655>");
	t_N->f_SWidth=t_N->f_SWidth+FLOAT(10.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1658>");
	if(((t_N->f_MinusOver)!=0) && ((t_N->f_MinusDown)!=0) && bb_input_TouchDown(0)==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1659>");
		t_N->f_MinusOver=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1660>");
		t_N->f_MinusDown=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1661>");
		t_N->f_Value=t_N->f_Value-FLOAT(1.0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1662>");
		if(t_N->f_Value<t_N->f_Minimum){
			DBG_BLOCK();
			t_N->f_Value=t_N->f_Minimum;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1666>");
	if(((t_N->f_Over)!=0) && bb_input_TouchX(0)>Float(t_X) && bb_input_TouchX(0)<Float(t_X+t_H) && bb_input_TouchY(0)>Float(t_Y) && bb_input_TouchY(0)<Float(t_Y+t_H)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1667>");
		if(bb_challengergui_CHGUI_MouseBusy==0 || ((t_N->f_MinusDown)!=0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1668>");
			t_N->f_MinusOver=1;
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1671>");
		t_N->f_MinusOver=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1675>");
	if(((t_N->f_MinusOver)!=0) || ((t_N->f_MinusDown)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1676>");
		if((bb_input_TouchDown(0))!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1677>");
			t_N->f_MinusDown=1;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1679>");
			t_N->f_MinusDown=0;
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1682>");
		t_N->f_MinusDown=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1686>");
	if(((t_N->f_PlusOver)!=0) && ((t_N->f_PlusDown)!=0) && bb_input_TouchDown(0)==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1687>");
		t_N->f_PlusOver=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1688>");
		t_N->f_PlusDown=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1689>");
		t_N->f_Value=t_N->f_Value+FLOAT(1.0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1690>");
		if(t_N->f_Value>t_N->f_Maximum){
			DBG_BLOCK();
			t_N->f_Value=t_N->f_Maximum;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1694>");
	if(bb_input_TouchX(0)>Float(t_X+t_W-t_H) && bb_input_TouchX(0)<Float(t_X+t_W) && bb_input_TouchY(0)>Float(t_Y) && bb_input_TouchY(0)<Float(t_Y+t_H)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1695>");
		if(bb_challengergui_CHGUI_MouseBusy==0 || ((t_N->f_PlusDown)!=0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1696>");
			t_N->f_PlusOver=1;
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1699>");
		t_N->f_PlusOver=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1703>");
	if(((t_N->f_PlusOver)!=0) || ((t_N->f_PlusDown)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1704>");
		if((bb_input_TouchDown(0))!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1705>");
			t_N->f_PlusDown=1;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1707>");
			t_N->f_PlusDown=0;
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1710>");
		t_N->f_PlusDown=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1714>");
	Float t_XPOS=Float(t_X+t_H-5)+(t_N->f_Value-t_N->f_Minimum)*t_N->f_Stp;
	DBG_LOCAL(t_XPOS,"XPOS")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1716>");
	if(((t_N->f_Over)!=0) && bb_input_TouchX(0)>t_XPOS && bb_input_TouchX(0)<t_XPOS+t_N->f_SWidth && t_N->f_PlusOver==0 && t_N->f_MinusOver==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1717>");
		if(bb_challengergui_CHGUI_MouseBusy==0 || ((t_N->f_SliderDown)!=0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1718>");
			t_N->f_SliderOver=1;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1719>");
			if(t_N->f_SliderDown==0){
				DBG_BLOCK();
				t_N->f_Start=int(bb_input_TouchX(0));
			}
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1722>");
		t_N->f_SliderOver=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1725>");
	if(((t_N->f_SliderOver)!=0) || ((t_N->f_SliderDown)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1726>");
		if((bb_input_TouchDown(0))!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1727>");
			t_N->f_SliderDown=1;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1729>");
			t_N->f_SliderDown=0;
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1732>");
		t_N->f_SliderDown=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1735>");
	if((t_N->f_SliderDown)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1736>");
		Float t_Change=bb_input_TouchX(0)-Float(t_N->f_Start);
		DBG_LOCAL(t_Change,"Change")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1737>");
		t_N->f_Value=t_N->f_Value+t_Change/t_N->f_Stp;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1738>");
		t_N->f_Start=int(bb_input_TouchX(0));
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1739>");
		if(t_N->f_Value<t_N->f_Minimum){
			DBG_BLOCK();
			t_N->f_Value=t_N->f_Minimum;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1740>");
		if(t_N->f_Value>t_N->f_Maximum){
			DBG_BLOCK();
			t_N->f_Value=t_N->f_Maximum;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1743>");
	if(t_N->f_SliderDown==0 && t_N->f_MinusDown==0 && t_N->f_PlusDown==0 && ((t_N->f_Down)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1744>");
		t_N->f_Value=(bb_input_TouchX(0)-Float(t_X)-Float(t_H)-Float(t_H)+FLOAT(10.0))/t_N->f_Stp+t_N->f_Minimum;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1745>");
		if(t_N->f_Value<t_N->f_Minimum){
			DBG_BLOCK();
			t_N->f_Value=t_N->f_Minimum;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1746>");
		if(t_N->f_Value>t_N->f_Maximum){
			DBG_BLOCK();
			t_N->f_Value=t_N->f_Maximum;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1749>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1750>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateVSlider(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateVSlider")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1756>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1757>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1758>");
	if(t_N->f_Value<t_N->f_Minimum){
		DBG_BLOCK();
		t_N->f_Value=t_N->f_Minimum;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1759>");
	if(t_N->f_Value>t_N->f_Maximum){
		DBG_BLOCK();
		t_N->f_Value=t_N->f_Maximum;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1760>");
	int t_X=bb_challengergui_CHGUI_RealX(t_N);
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1761>");
	int t_Y=bb_challengergui_CHGUI_RealY(t_N);
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1762>");
	int t_W=int(t_N->f_W);
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1763>");
	int t_H=int(t_N->f_H);
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1766>");
	t_N->f_Stp=(t_N->f_H-FLOAT(2.0)*t_N->f_W)/(t_N->f_Maximum-t_N->f_Minimum);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1767>");
	t_N->f_SWidth=t_N->f_Stp;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1768>");
	if(t_N->f_SWidth<t_N->f_W){
		DBG_BLOCK();
		t_N->f_SWidth=t_N->f_W;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1769>");
	if(t_N->f_SWidth>t_N->f_H-t_N->f_W-t_N->f_W-FLOAT(10.0)){
		DBG_BLOCK();
		t_N->f_SWidth=t_N->f_H-t_N->f_W-t_N->f_W-FLOAT(10.0);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1770>");
	t_N->f_Stp=(t_N->f_H-t_N->f_SWidth-t_N->f_W-t_N->f_W)/(t_N->f_Maximum-t_N->f_Minimum);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1771>");
	t_N->f_SWidth=t_N->f_SWidth+FLOAT(10.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1775>");
	if(((t_N->f_MinusOver)!=0) && ((t_N->f_MinusDown)!=0) && bb_input_TouchDown(0)==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1776>");
		t_N->f_MinusOver=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1777>");
		t_N->f_MinusDown=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1778>");
		t_N->f_Value=t_N->f_Value-FLOAT(1.0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1779>");
		if(t_N->f_Value<t_N->f_Minimum){
			DBG_BLOCK();
			t_N->f_Value=t_N->f_Minimum;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1783>");
	if(bb_input_TouchX(0)>Float(t_X) && bb_input_TouchX(0)<Float(t_X+t_W) && bb_input_TouchY(0)>Float(t_Y) && bb_input_TouchY(0)<Float(t_Y+t_W)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1784>");
		if(bb_challengergui_CHGUI_MouseBusy==0 || ((t_N->f_MinusDown)!=0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1785>");
			t_N->f_MinusOver=1;
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1788>");
		t_N->f_MinusOver=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1792>");
	if(((t_N->f_MinusOver)!=0) || ((t_N->f_MinusDown)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1793>");
		if((bb_input_TouchDown(0))!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1794>");
			t_N->f_MinusDown=1;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1796>");
			t_N->f_MinusDown=0;
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1799>");
		t_N->f_MinusDown=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1803>");
	if(((t_N->f_PlusOver)!=0) && ((t_N->f_PlusDown)!=0) && bb_input_TouchDown(0)==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1804>");
		t_N->f_PlusOver=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1805>");
		t_N->f_PlusDown=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1806>");
		t_N->f_Value=t_N->f_Value+FLOAT(1.0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1807>");
		if(t_N->f_Value>t_N->f_Maximum){
			DBG_BLOCK();
			t_N->f_Value=t_N->f_Maximum;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1811>");
	if(((t_N->f_Over)!=0) && bb_input_TouchX(0)>Float(t_X) && bb_input_TouchX(0)<Float(t_X+t_W) && bb_input_TouchY(0)>Float(t_Y+t_H-t_W) && bb_input_TouchY(0)<Float(t_Y+t_H)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1812>");
		if(bb_challengergui_CHGUI_MouseBusy==0 || ((t_N->f_PlusDown)!=0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1813>");
			t_N->f_PlusOver=1;
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1816>");
		t_N->f_PlusOver=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1820>");
	if(((t_N->f_PlusOver)!=0) || ((t_N->f_PlusDown)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1821>");
		if((bb_input_TouchDown(0))!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1822>");
			t_N->f_PlusDown=1;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1824>");
			t_N->f_PlusDown=0;
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1827>");
		t_N->f_PlusDown=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1831>");
	Float t_YPOS=Float(t_Y+t_W-5)+(t_N->f_Value-t_N->f_Minimum)*t_N->f_Stp;
	DBG_LOCAL(t_YPOS,"YPOS")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1833>");
	if(((t_N->f_Over)!=0) && bb_input_TouchY(0)>t_YPOS && bb_input_TouchY(0)<t_YPOS+t_N->f_SWidth && t_N->f_PlusOver==0 && t_N->f_MinusOver==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1834>");
		if(bb_challengergui_CHGUI_MouseBusy==0 || ((t_N->f_SliderDown)!=0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1835>");
			t_N->f_SliderOver=1;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1836>");
			if(t_N->f_SliderDown==0){
				DBG_BLOCK();
				t_N->f_Start=int(bb_input_TouchY(0));
			}
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1839>");
		t_N->f_SliderOver=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1842>");
	if(((t_N->f_SliderOver)!=0) || ((t_N->f_SliderDown)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1843>");
		if((bb_input_TouchDown(0))!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1844>");
			t_N->f_SliderDown=1;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1846>");
			t_N->f_SliderDown=0;
		}
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1849>");
		t_N->f_SliderDown=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1852>");
	if((t_N->f_SliderDown)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1853>");
		Float t_Change=bb_input_TouchY(0)-Float(t_N->f_Start);
		DBG_LOCAL(t_Change,"Change")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1854>");
		t_N->f_Value=t_N->f_Value+t_Change/t_N->f_Stp;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1855>");
		t_N->f_Start=int(bb_input_TouchY(0));
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1856>");
		if(t_N->f_Value<t_N->f_Minimum){
			DBG_BLOCK();
			t_N->f_Value=t_N->f_Minimum;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1857>");
		if(t_N->f_Value>t_N->f_Maximum){
			DBG_BLOCK();
			t_N->f_Value=t_N->f_Maximum;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1860>");
	if(t_N->f_SliderDown==0 && t_N->f_MinusDown==0 && t_N->f_PlusDown==0 && ((t_N->f_Down)!=0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1861>");
		t_N->f_Value=(bb_input_TouchY(0)-Float(t_Y)-Float(t_W)-Float(t_W)+FLOAT(10.0))/t_N->f_Stp+t_N->f_Minimum;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1862>");
		if(t_N->f_Value<t_N->f_Minimum){
			DBG_BLOCK();
			t_N->f_Value=t_N->f_Minimum;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1863>");
		if(t_N->f_Value>t_N->f_Maximum){
			DBG_BLOCK();
			t_N->f_Value=t_N->f_Maximum;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1866>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1867>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateListboxItem(bb_challengergui_CHGUI* t_N,int t_C){
	DBG_ENTER("CHGUI_UpdateListboxItem")
	DBG_LOCAL(t_N,"N")
	DBG_LOCAL(t_C,"C")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2060>");
	t_N->f_X=FLOAT(0.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2061>");
	t_N->f_Y=Float(t_C*t_N->f_Parent->f_ListHeight);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2062>");
	t_N->f_W=t_N->f_Parent->f_W;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2063>");
	t_N->f_H=Float(t_N->f_Parent->f_ListHeight);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2065>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2066>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2068>");
	if((t_N->f_Clicked)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2069>");
		t_N->f_Parent->f_Text=t_N->f_Text;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2070>");
		gc_assign(t_N->f_Parent->f_SelectedListboxItem,t_N);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2071>");
		t_N->f_Parent->f_Value=t_N->f_Value;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2074>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2075>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateListbox(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateListbox")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2039>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2040>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2042>");
	t_N->f_ListHeight=bb_challengergui_CHGUI_Font->m_GetFontHeight()+10;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2043>");
	t_N->f_ListboxNumber=int(t_N->f_H/Float(t_N->f_ListHeight)-FLOAT(1.0));
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2045>");
	t_N->f_ListboxSlider->f_Minimum=FLOAT(0.0);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2046>");
	t_N->f_ListboxSlider->f_Maximum=Float(t_N->f_ListboxItems.Length()-t_N->f_ListboxNumber-1);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2048>");
	if(t_N->f_ListboxSlider->f_Maximum<FLOAT(1.0)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2049>");
		t_N->f_ListboxSlider->f_Visible=0;
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2051>");
		t_N->f_ListboxSlider->f_Visible=1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2054>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<2055>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateRadiobox(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateRadiobox")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1471>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1472>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1473>");
	t_N->f_W=t_N->f_H+t_N->f_H/FLOAT(4.0)+bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1474>");
	if((t_N->f_Clicked)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1475>");
		t_N->f_Value=FLOAT(1.0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1476>");
		for(int t_X=0;t_X<=t_N->f_Parent->f_Radioboxes.Length()-1;t_X=t_X+1){
			DBG_BLOCK();
			DBG_LOCAL(t_X,"X")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1477>");
			if(t_N->f_Parent->f_Radioboxes.At(t_X)->f_Group==t_N->f_Group && t_N->f_Parent->f_Radioboxes.At(t_X)!=t_N){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1478>");
				t_N->f_Parent->f_Radioboxes.At(t_X)->f_Value=FLOAT(0.0);
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1483>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1484>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateTickbox(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateTickbox")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1452>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1453>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1454>");
	t_N->f_W=t_N->f_H+t_N->f_H/FLOAT(4.0)+bb_challengergui_CHGUI_Font->m_GetTxtWidth2(t_N->f_Text);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1455>");
	if((t_N->f_Clicked)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1456>");
		if(t_N->f_Value==FLOAT(0.0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1457>");
			t_N->f_Value=FLOAT(1.0);
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1459>");
			t_N->f_Value=FLOAT(0.0);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1464>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1465>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateImageButton(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateImageButton")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1441>");
	t_N->f_W=Float(t_N->f_Img->m_Width()/4);
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1442>");
	t_N->f_H=Float(t_N->f_Img->m_Height());
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1443>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1444>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1446>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1447>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateButton(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateButton")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1432>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1433>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1435>");
	if(bb_challengergui_CHGUI_Tooltips==1 && t_N->f_Tooltip!=String() && t_N->f_OverTime>bb_challengergui_CHGUI_TooltipTime){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1436>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			gc_assign(bb_challengergui_CHGUI_TooltipFlag,t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_Locked(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_Locked")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4032>");
	bb_challengergui_CHGUI* t_E=0;
	DBG_LOCAL(t_E,"E")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4033>");
	t_E=t_N;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4034>");
	if(t_E==bb_challengergui_CHGUI_LockedWIndow){
		DBG_BLOCK();
		return 1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4035>");
	do{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4036>");
		if(t_E->f_Parent!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4037>");
			if(t_E->f_Parent==bb_challengergui_CHGUI_LockedWIndow){
				DBG_BLOCK();
				return 1;
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4038>");
			t_E=t_E->f_Parent;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<4040>");
			return 0;
		}
	}while(!(false));
}
int bb_challengergui_CHGUI_UpdateWindow(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateWindow")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1278>");
	Float t_X=Float(bb_challengergui_CHGUI_RealX(t_N));
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1279>");
	Float t_Y=Float(bb_challengergui_CHGUI_RealY(t_N));
	DBG_LOCAL(t_Y,"Y")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1280>");
	int t_W=int(t_N->f_W);
	DBG_LOCAL(t_W,"W")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1281>");
	int t_H=int(t_N->f_H);
	DBG_LOCAL(t_H,"H")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1283>");
	if((t_N->f_Minimised)!=0){
		DBG_BLOCK();
		t_H=int(bb_challengergui_CHGUI_TitleHeight);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1284>");
	int t_TH=int(bb_challengergui_CHGUI_TitleHeight);
	DBG_LOCAL(t_TH,"TH")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1286>");
	if(t_N->f_Text==String()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1287>");
		t_N->f_Close=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1288>");
		t_N->f_Minimise=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1291>");
	t_N->m_CheckOver();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1292>");
	t_N->m_CheckDown();
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1295>");
	if((t_N->f_Over)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1296>");
		if(bb_input_TouchY(0)>t_Y && bb_input_TouchY(0)<t_Y+bb_challengergui_CHGUI_TitleHeight){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1297>");
			bb_challengergui_CHGUI_DragOver=1;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1304>");
	if((t_N->f_Close)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1305>");
		Float t_TH2=bb_challengergui_CHGUI_TitleHeight;
		DBG_LOCAL(t_TH2,"TH")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1306>");
		if(((t_N->f_CloseOver)!=0) && ((t_N->f_CloseDown)!=0) && bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1307>");
			t_N->f_CloseOver=0;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1308>");
			t_N->f_CloseDown=0;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1309>");
			t_N->f_Visible=0;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1311>");
		if(((t_N->f_Over)!=0) && bb_input_TouchX(0)>t_X+Float(t_W)-t_TH2/FLOAT(2.5)-FLOAT(10.0) && bb_input_TouchX(0)<t_X+Float(t_W)-t_TH2/FLOAT(2.5)-FLOAT(10.0)+t_TH2/FLOAT(2.5) && bb_input_TouchY(0)>t_Y+(t_TH2-t_TH2/FLOAT(2.5))/FLOAT(2.0) && bb_input_TouchY(0)<t_Y+(t_TH2-t_TH2/FLOAT(2.5))/FLOAT(2.0)+t_TH2/FLOAT(2.5)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1312>");
			if(bb_challengergui_CHGUI_MouseBusy==0 || ((t_N->f_CloseDown)!=0)){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1313>");
				t_N->f_CloseOver=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1314>");
				bb_challengergui_CHGUI_DragOver=1;
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1317>");
			t_N->f_CloseOver=0;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1319>");
		if(((t_N->f_CloseOver)!=0) || ((t_N->f_CloseDown)!=0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1320>");
			if((bb_input_TouchDown(0))!=0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1321>");
				t_N->f_CloseDown=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1322>");
				bb_challengergui_CHGUI_DragOver=1;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1324>");
				t_N->f_CloseDown=0;
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1327>");
			t_N->f_CloseDown=0;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1332>");
	if((t_N->f_Minimise)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1333>");
		Float t_TH1=bb_challengergui_CHGUI_TitleHeight;
		DBG_LOCAL(t_TH1,"TH1")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1334>");
		int t_Off2=int((t_TH1-t_TH1/FLOAT(2.0))/FLOAT(2.0));
		DBG_LOCAL(t_Off2,"Off2")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1336>");
		if(((t_N->f_MinimiseOver)!=0) && ((t_N->f_MinimiseDown)!=0) && bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1337>");
			t_N->f_CloseOver=0;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1338>");
			t_N->f_CloseDown=0;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1339>");
			if(t_N->f_Minimised==0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1340>");
				t_N->f_Minimised=1;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1342>");
				t_N->f_Minimised=0;
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1346>");
		if(((t_N->f_Over)!=0) && bb_input_TouchX(0)>t_X+Float(t_W)-(Float(t_TH)/FLOAT(2.5)+Float(t_TH)/FLOAT(2.5))-Float(t_TH)/FLOAT(1.5) && bb_input_TouchX(0)<t_X+Float(t_W)-(Float(t_TH)/FLOAT(2.5)+Float(t_TH)/FLOAT(2.5))-Float(t_TH)/FLOAT(1.5)+Float(t_TH)/FLOAT(2.5) && bb_input_TouchY(0)>t_Y+(Float(t_TH)-Float(t_TH)/FLOAT(2.5))/FLOAT(2.0) && bb_input_TouchY(0)<t_Y+(Float(t_TH)-Float(t_TH)/FLOAT(2.5))/FLOAT(2.0)+Float(t_TH)/FLOAT(2.5)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1347>");
			if(bb_challengergui_CHGUI_MouseBusy==0 || ((t_N->f_MinimiseDown)!=0)){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1348>");
				t_N->f_MinimiseOver=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1349>");
				bb_challengergui_CHGUI_DragOver=1;
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1352>");
			t_N->f_MinimiseOver=0;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1354>");
		if(((t_N->f_MinimiseOver)!=0) || ((t_N->f_MinimiseDown)!=0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1355>");
			if((bb_input_TouchDown(0))!=0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1356>");
				t_N->f_MinimiseDown=1;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1357>");
				bb_challengergui_CHGUI_DragOver=1;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1359>");
				t_N->f_MinimiseDown=0;
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1362>");
			t_N->f_MinimiseDown=0;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1367>");
	if(t_N->f_Moveable==1 && ((t_N->f_Over)!=0) && t_N->f_Moving==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1368>");
		if(bb_input_TouchY(0)>t_Y && bb_input_TouchY(0)<t_Y+bb_challengergui_CHGUI_TitleHeight && ((bb_input_TouchDown(0))!=0) && t_N->f_Text!=String()){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1369>");
			if(t_N->f_CloseOver==0 && t_N->f_MinimiseOver==0 && t_N->f_CloseDown==0 && t_N->f_MinimiseDown==0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1370>");
				if(bb_challengergui_CHGUI_Moving==0){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1371>");
					t_N->f_Moving=1;
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1372>");
					t_N->f_MX=bb_input_TouchX(0);
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1373>");
					t_N->f_MY=bb_input_TouchY(0);
				}
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1378>");
	if(bb_input_TouchDown(0)==0){
		DBG_BLOCK();
		t_N->f_Moving=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1380>");
	if(t_N->f_Moving==1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1381>");
		t_N->f_X=t_N->f_X+(bb_input_TouchX(0)-t_N->f_MX);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1382>");
		t_N->f_Y=t_N->f_Y+(bb_input_TouchY(0)-t_N->f_MY);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1383>");
		t_N->f_MX=bb_input_TouchX(0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1384>");
		t_N->f_MY=bb_input_TouchY(0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1385>");
		bb_challengergui_CHGUI_DragOver=1;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1387>");
		bb_challengergui_CHGUI* t_RP=t_N->f_Parent;
		DBG_LOCAL(t_RP,"RP")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1389>");
		do{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1390>");
			if(t_RP->f_Element!=String(L"Tab",3)){
				DBG_BLOCK();
				break;
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1391>");
			if(t_RP->f_Parent!=0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1392>");
				t_RP=t_RP->f_Parent;
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1394>");
				break;
			}
		}while(!(false));
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1399>");
		if(t_N->f_X<FLOAT(0.0)){
			DBG_BLOCK();
			t_N->f_X=FLOAT(0.0);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1401>");
		if(t_N->f_X>t_RP->f_W-t_N->f_W){
			DBG_BLOCK();
			t_N->f_X=t_RP->f_W-t_N->f_W;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1403>");
		if(t_N->f_Y>t_RP->f_H-Float(t_H)){
			DBG_BLOCK();
			t_N->f_Y=t_RP->f_H-Float(t_H);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1405>");
		int t_YVal=t_RP->f_MenuHeight;
		DBG_LOCAL(t_YVal,"YVal")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1406>");
		if(t_RP->f_Text!=String()){
			DBG_BLOCK();
			t_YVal=int(Float(t_YVal)+bb_challengergui_CHGUI_TitleHeight);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1407>");
		if((t_RP->f_Tabbed)!=0){
			DBG_BLOCK();
			t_YVal=t_YVal+t_RP->f_TabHeight+5;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1409>");
		if(t_N->f_Y<Float(t_YVal)){
			DBG_BLOCK();
			t_N->f_Y=Float(t_YVal);
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1414>");
	if(((t_N->f_Clicked)!=0) && bb_input_TouchY(0)>t_Y && bb_input_TouchY(0)<t_Y+bb_challengergui_CHGUI_TitleHeight && t_N->f_Text!=String()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1415>");
		if(((t_N->f_Minimise)!=0) && t_N->f_CloseOver==0 && t_N->f_MinimiseOver==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1416>");
			if(t_N->f_DClickMillisecs>bb_app_Millisecs()){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1417>");
				if(t_N->f_Minimised==1){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1418>");
					t_N->f_Minimised=0;
				}else{
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1420>");
					t_N->f_Minimised=1;
				}
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1423>");
				t_N->f_DClickMillisecs=bb_app_Millisecs()+275;
			}
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateContents(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_UpdateContents")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1145>");
	int t_X=0;
	DBG_LOCAL(t_X,"X")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1146>");
	int t_XX=0;
	DBG_LOCAL(t_XX,"XX")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1147>");
	int t_C=0;
	DBG_LOCAL(t_C,"C")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1148>");
	t_N->f_ReOrdered=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1150>");
	for(t_X=t_N->f_Menus.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1151>");
		t_N->f_Menus.At(t_X)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1152>");
		bb_challengergui_CHGUI_UpdateSubMenu(t_N->f_Menus.At(t_X));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1155>");
	if((t_N->f_Tabbed)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1156>");
		bb_challengergui_CHGUI_UpdateContents(t_N->f_CurrentTab);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1160>");
	for(int t_NN=t_N->f_TopList.Length()-1;t_NN>=0;t_NN=t_NN+-1){
		DBG_BLOCK();
		DBG_LOCAL(t_NN,"NN")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1161>");
		bb_challengergui_CHGUI_UpdateContents(t_N->f_TopList.At(t_NN));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1164>");
	for(int t_NN2=t_N->f_VariList.Length()-1;t_NN2>=0;t_NN2=t_NN2+-1){
		DBG_BLOCK();
		DBG_LOCAL(t_NN2,"NN")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1165>");
		bb_challengergui_CHGUI_UpdateContents(t_N->f_VariList.At(t_NN2));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1168>");
	for(int t_NN3=t_N->f_BottomList.Length()-1;t_NN3>=0;t_NN3=t_NN3+-1){
		DBG_BLOCK();
		DBG_LOCAL(t_NN3,"NN")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1169>");
		bb_challengergui_CHGUI_UpdateContents(t_N->f_BottomList.At(t_NN3));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1173>");
	for(t_X=t_N->f_Tabs.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1174>");
		t_N->f_Tabs.At(t_X)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1175>");
		if(((bb_challengergui_CHGUI_RealActive(t_N->f_Tabs.At(t_X)))!=0) && ((bb_challengergui_CHGUI_RealVisible(t_N->f_Tabs.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Tabs.At(t_X))==0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateTab(t_N->f_Tabs.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1180>");
	if((bb_challengergui_CHGUI_MenuClose)!=0){
		DBG_BLOCK();
		t_N->f_MenuActive=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1181>");
	for(t_X=t_N->f_Menus.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1182>");
		t_N->f_Menus.At(t_X)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1183>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Menus.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Menus.At(t_X))==0 && ((bb_challengergui_CHGUI_RealActive(t_N->f_Menus.At(t_X)))!=0)){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateMenu(t_N->f_Menus.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1187>");
	for(t_X=t_N->f_Dropdowns.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1188>");
		t_C=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1189>");
		for(t_XX=0;t_XX<=t_N->f_Dropdowns.At(t_X)->f_DropdownItems.Length()-1;t_XX=t_XX+1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1190>");
			t_N->f_Dropdowns.At(t_X)->f_DropdownItems.At(t_XX)->m_CheckClicked();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1191>");
			if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Dropdowns.At(t_X)->f_DropdownItems.At(t_XX)))!=0) && ((t_N->f_Dropdowns.At(t_X)->f_OnFocus)!=0) && ((bb_challengergui_CHGUI_RealVisible(t_N->f_Dropdowns.At(t_X)))!=0)){
				DBG_BLOCK();
				bb_challengergui_CHGUI_UpdateDropdownItem(t_N->f_Dropdowns.At(t_X)->f_DropdownItems.At(t_XX),t_C);
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1192>");
			if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Dropdowns.At(t_X)->f_DropdownItems.At(t_XX)))!=0) && ((t_N->f_Dropdowns.At(t_X)->f_OnFocus)!=0) && ((bb_challengergui_CHGUI_RealVisible(t_N->f_Dropdowns.At(t_X)))!=0)){
				DBG_BLOCK();
				t_C=t_C+1;
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1197>");
	for(t_X=t_N->f_Dropdowns.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1198>");
		t_N->f_Dropdowns.At(t_X)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1199>");
		if(((bb_challengergui_CHGUI_RealActive(t_N->f_Dropdowns.At(t_X)))!=0) && ((bb_challengergui_CHGUI_RealVisible(t_N->f_Dropdowns.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Dropdowns.At(t_X))==0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateDropdown(t_N->f_Dropdowns.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1203>");
	for(t_X=t_N->f_Labels.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1204>");
		t_N->f_Labels.At(t_X)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1205>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Labels.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Labels.At(t_X))==0 && ((bb_challengergui_CHGUI_RealActive(t_N->f_Labels.At(t_X)))!=0)){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateLabel(t_N->f_Labels.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1209>");
	for(t_X=t_N->f_Textfields.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1210>");
		t_N->f_Textfields.At(t_X)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1211>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Textfields.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Textfields.At(t_X))==0 && ((bb_challengergui_CHGUI_RealActive(t_N->f_Textfields.At(t_X)))!=0)){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateTextfield(t_N->f_Textfields.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1215>");
	for(t_X=t_N->f_HSliders.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1216>");
		t_N->f_HSliders.At(t_X)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1217>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_HSliders.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_HSliders.At(t_X))==0 && ((bb_challengergui_CHGUI_RealActive(t_N->f_HSliders.At(t_X)))!=0)){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateHSlider(t_N->f_HSliders.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1221>");
	for(t_X=t_N->f_VSliders.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1222>");
		t_N->f_VSliders.At(t_X)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1223>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_VSliders.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_VSliders.At(t_X))==0 && ((bb_challengergui_CHGUI_RealActive(t_N->f_VSliders.At(t_X)))!=0)){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateVSlider(t_N->f_VSliders.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1227>");
	for(t_X=t_N->f_Listboxes.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1228>");
		t_N->f_Listboxes.At(t_X)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1229>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Listboxes.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Listboxes.At(t_X))==0 && ((bb_challengergui_CHGUI_RealActive(t_N->f_Listboxes.At(t_X)))!=0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1230>");
			int t_C2=0;
			DBG_LOCAL(t_C2,"C")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1232>");
			for(t_XX=int(t_N->f_Listboxes.At(t_X)->f_ListboxSlider->f_Value);Float(t_XX)<=t_N->f_Listboxes.At(t_X)->f_ListboxSlider->f_Value+Float(t_N->f_Listboxes.At(t_X)->f_ListboxNumber);t_XX=t_XX+1){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1233>");
				if(t_XX<t_N->f_Listboxes.At(t_X)->f_ListboxItems.Length() && t_XX>-1){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1234>");
					if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Listboxes.At(t_X)->f_ListboxItems.At(t_XX)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Listboxes.At(t_X)->f_ListboxItems.At(t_XX))==0 && ((bb_challengergui_CHGUI_RealActive(t_N->f_Listboxes.At(t_X)->f_ListboxItems.At(t_XX)))!=0)){
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1235>");
						t_N->f_Listboxes.At(t_X)->f_ListboxItems.At(t_XX)->m_CheckClicked();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1236>");
						bb_challengergui_CHGUI_UpdateListboxItem(t_N->f_Listboxes.At(t_X)->f_ListboxItems.At(t_XX),t_C2);
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1237>");
						t_C2=t_C2+1;
					}
				}
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1241>");
			bb_challengergui_CHGUI_UpdateListbox(t_N->f_Listboxes.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1246>");
	for(t_X=t_N->f_Radioboxes.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1247>");
		t_N->f_Radioboxes.At(t_X)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1248>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Radioboxes.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Radioboxes.At(t_X))==0 && ((bb_challengergui_CHGUI_RealActive(t_N->f_Radioboxes.At(t_X)))!=0)){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateRadiobox(t_N->f_Radioboxes.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1252>");
	for(t_X=t_N->f_Tickboxes.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1253>");
		t_N->f_Tickboxes.At(t_X)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1254>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Tickboxes.At(t_X)))!=0) && ((bb_challengergui_CHGUI_RealActive(t_N->f_Tickboxes.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Tickboxes.At(t_X))==0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateTickbox(t_N->f_Tickboxes.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1258>");
	for(t_X=t_N->f_ImageButtons.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1259>");
		t_N->f_ImageButtons.At(t_X)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1260>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_ImageButtons.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_ImageButtons.At(t_X))==0 && ((bb_challengergui_CHGUI_RealActive(t_N->f_ImageButtons.At(t_X)))!=0)){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateImageButton(t_N->f_ImageButtons.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1264>");
	for(t_X=t_N->f_Buttons.Length()-1;t_X>=0;t_X=t_X+-1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1265>");
		t_N->f_Buttons.At(t_X)->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1266>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N->f_Buttons.At(t_X)))!=0) && bb_challengergui_CHGUI_RealMinimised(t_N->f_Buttons.At(t_X))==0 && ((bb_challengergui_CHGUI_RealActive(t_N->f_Buttons.At(t_X)))!=0)){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateButton(t_N->f_Buttons.At(t_X));
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1269>");
	if(t_N->f_Element!=String(L"Tab",3)){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1271>");
		t_N->m_CheckClicked();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1272>");
		if(((bb_challengergui_CHGUI_RealVisible(t_N))!=0) && ((bb_challengergui_CHGUI_RealActive(t_N))!=0) || ((bb_challengergui_CHGUI_Locked(t_N))!=0)){
			DBG_BLOCK();
			bb_challengergui_CHGUI_UpdateWindow(t_N);
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_CursorMillisecs;
int bb_challengergui_CHGUI_DragScroll;
int bb_challengergui_CHGUI_DragMoving;
Float bb_challengergui_CHGUI_OffsetMX;
Float bb_challengergui_CHGUI_OffsetMY;
int bb_challengergui_LockFocus(bb_challengergui_CHGUI* t_Window){
	DBG_ENTER("LockFocus")
	DBG_LOCAL(t_Window,"Window")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<873>");
	if(bb_challengergui_CHGUI_MsgBoxWindow->f_Visible==0 || t_Window==bb_challengergui_CHGUI_MsgBoxWindow){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<874>");
		gc_assign(bb_challengergui_CHGUI_LockedWIndow,t_Window);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<875>");
		bb_challengergui_CHGUI_Reorder(t_Window);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<876>");
		bb_challengergui_CHGUI_Canvas->f_Active=0;
	}
	return 0;
}
int bb_challengergui_UnlockFocus(){
	DBG_ENTER("UnlockFocus")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<882>");
	if(bb_challengergui_CHGUI_MsgBoxWindow->f_Visible==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<883>");
		bb_challengergui_CHGUI_LockedWIndow=0;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<884>");
		bb_challengergui_CHGUI_Canvas->f_Active=1;
	}
	return 0;
}
int bb_challengergui_CHGUI_UpdateMsgBox(){
	DBG_ENTER("CHGUI_UpdateMsgBox")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3855>");
	if((bb_challengergui_CHGUI_MsgBoxWindow->f_Visible)!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3856>");
		bb_challengergui_LockFocus(bb_challengergui_CHGUI_MsgBoxWindow);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3858>");
		bb_challengergui_CHGUI_MsgBoxWindow->f_X=Float(bb_graphics_DeviceWidth()/2)-bb_challengergui_CHGUI_MsgBoxWindow->f_W/FLOAT(2.0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3859>");
		bb_challengergui_CHGUI_MsgBoxWindow->f_Y=Float(bb_graphics_DeviceHeight()/2)-bb_challengergui_CHGUI_MsgBoxWindow->f_H/FLOAT(2.0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3860>");
		bb_challengergui_CHGUI_MsgBoxLabel->f_X=(bb_challengergui_CHGUI_MsgBoxWindow->f_W-bb_challengergui_CHGUI_Font->m_GetTxtWidth2(bb_challengergui_CHGUI_MsgBoxLabel->f_Text))/FLOAT(2.0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3862>");
		if((bb_challengergui_CHGUI_MsgBoxButton->f_Clicked)!=0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3863>");
			bb_challengergui_CHGUI_MsgBoxWindow->f_Visible=0;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<3864>");
			bb_challengergui_UnlockFocus();
		}
	}
	return 0;
}
int bb_challengergui_CHGUI_Update(){
	DBG_ENTER("CHGUI_Update")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<743>");
	if(bb_input_TouchDown(0)==0){
		DBG_BLOCK();
		bb_challengergui_CHGUI_MouseBusy=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<744>");
	bb_challengergui_CHGUI_Over=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<745>");
	bb_challengergui_CHGUI_OverFlag=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<746>");
	bb_challengergui_CHGUI_DownFlag=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<747>");
	bb_challengergui_CHGUI_MenuOver=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<748>");
	bb_challengergui_CHGUI_TextBoxOver=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<749>");
	bb_challengergui_CHGUI_TextboxOnFocus=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<750>");
	bb_challengergui_CHGUI_TextBoxDown=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<751>");
	bb_challengergui_CHGUI_DragOver=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<752>");
	bb_challengergui_CHGUI_TooltipFlag=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<753>");
	if(bb_challengergui_CHGUI_Canvas!=0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<754>");
		bb_challengergui_CHGUI_Canvas->f_W=Float(bb_challengergui_CHGUI_Width);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<755>");
		bb_challengergui_CHGUI_Canvas->f_H=Float(bb_challengergui_CHGUI_Height);
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<759>");
	if(bb_challengergui_CHGUI_Moving==1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<760>");
		bb_challengergui_CHGUI_OffsetY=bb_challengergui_CHGUI_OffsetY-(bb_challengergui_CHGUI_OffsetY-bb_challengergui_CHGUI_TargetY)/FLOAT(8.0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<761>");
		bb_challengergui_CHGUI_OffsetX=bb_challengergui_CHGUI_OffsetX-(bb_challengergui_CHGUI_OffsetX-bb_challengergui_CHGUI_TargetX)/FLOAT(8.0);
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<762>");
		if(bb_challengergui_CHGUI_OffsetY-bb_challengergui_CHGUI_TargetY>FLOAT(-1.0) && bb_challengergui_CHGUI_OffsetY-bb_challengergui_CHGUI_TargetY<FLOAT(1.0) && bb_challengergui_CHGUI_OffsetX-bb_challengergui_CHGUI_TargetX>FLOAT(-1.0) && bb_challengergui_CHGUI_OffsetX-bb_challengergui_CHGUI_TargetX<FLOAT(1.0)){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<763>");
			bb_challengergui_CHGUI_OffsetY=bb_challengergui_CHGUI_TargetY;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<764>");
			bb_challengergui_CHGUI_OffsetX=bb_challengergui_CHGUI_TargetX;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<765>");
			bb_challengergui_CHGUI_Moving=0;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<769>");
	for(int t_N=bb_challengergui_CHGUI_TopList.Length()-1;t_N>=0;t_N=t_N+-1){
		DBG_BLOCK();
		DBG_LOCAL(t_N,"N")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<770>");
		bb_challengergui_CHGUI_UpdateContents(bb_challengergui_CHGUI_TopList.At(t_N));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<772>");
	for(int t_N2=bb_challengergui_CHGUI_VariList.Length()-1;t_N2>=0;t_N2=t_N2+-1){
		DBG_BLOCK();
		DBG_LOCAL(t_N2,"N")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<773>");
		bb_challengergui_CHGUI_UpdateContents(bb_challengergui_CHGUI_VariList.At(t_N2));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<775>");
	for(int t_N3=bb_challengergui_CHGUI_BottomList.Length()-1;t_N3>=0;t_N3=t_N3+-1){
		DBG_BLOCK();
		DBG_LOCAL(t_N3,"N")
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<776>");
		bb_challengergui_CHGUI_UpdateContents(bb_challengergui_CHGUI_BottomList.At(t_N3));
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<781>");
	if(((bb_input_TouchDown(0))!=0) && bb_challengergui_CHGUI_DownFlag==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<782>");
		bb_challengergui_CHGUI_IgnoreMouse=1;
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<784>");
		bb_challengergui_CHGUI_IgnoreMouse=0;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<786>");
	bb_challengergui_CHGUI_MenuClose=0;
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<787>");
	if(bb_challengergui_CHGUI_MenuOver==0 && ((bb_input_TouchDown(0))!=0)){
		DBG_BLOCK();
		bb_challengergui_CHGUI_MenuClose=1;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<788>");
	if(bb_challengergui_CHGUI_CursorMillisecs<bb_app_Millisecs()){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<789>");
		bb_challengergui_CHGUI_CursorMillisecs=bb_app_Millisecs()+300;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<790>");
		if(bb_challengergui_CHGUI_Cursor==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<791>");
			bb_challengergui_CHGUI_Cursor=1;
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<793>");
			bb_challengergui_CHGUI_Cursor=0;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<797>");
	if(bb_challengergui_CHGUI_DragScroll==1){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<798>");
		if(bb_challengergui_CHGUI_DragOver==0 && ((bb_input_TouchDown(0))!=0) && bb_challengergui_CHGUI_DragMoving==0 && bb_challengergui_CHGUI_TextboxOnFocus==0){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<799>");
			bb_challengergui_CHGUI_OffsetMX=bb_input_TouchX(0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<800>");
			bb_challengergui_CHGUI_OffsetMY=bb_input_TouchY(0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<801>");
			bb_challengergui_CHGUI_DragMoving=1;
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<804>");
		if(bb_challengergui_CHGUI_DragMoving==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<805>");
			bb_challengergui_CHGUI_OffsetX=bb_challengergui_CHGUI_OffsetX+(bb_input_TouchX(0)-bb_challengergui_CHGUI_OffsetMX);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<806>");
			bb_challengergui_CHGUI_OffsetY=bb_challengergui_CHGUI_OffsetY+(bb_input_TouchY(0)-bb_challengergui_CHGUI_OffsetMY);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<807>");
			bb_challengergui_CHGUI_OffsetMX=bb_input_TouchX(0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<808>");
			bb_challengergui_CHGUI_OffsetMY=bb_input_TouchY(0);
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<811>");
			if(bb_challengergui_CHGUI_OffsetX<Float(-1*(bb_challengergui_CHGUI_Width-bb_graphics_DeviceWidth()))){
				DBG_BLOCK();
				bb_challengergui_CHGUI_OffsetX=Float(-1*(bb_challengergui_CHGUI_Width-bb_graphics_DeviceWidth()));
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<812>");
			if(bb_challengergui_CHGUI_OffsetX>FLOAT(0.0)){
				DBG_BLOCK();
				bb_challengergui_CHGUI_OffsetX=FLOAT(0.0);
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<814>");
			if(bb_challengergui_CHGUI_OffsetY<Float(bb_graphics_DeviceHeight()-bb_challengergui_CHGUI_Height)){
				DBG_BLOCK();
				bb_challengergui_CHGUI_OffsetY=Float(bb_graphics_DeviceHeight()-bb_challengergui_CHGUI_Height);
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<815>");
			if(bb_challengergui_CHGUI_OffsetY>FLOAT(0.0)){
				DBG_BLOCK();
				bb_challengergui_CHGUI_OffsetY=FLOAT(0.0);
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<818>");
		if(bb_challengergui_CHGUI_Width<bb_graphics_DeviceWidth()){
			DBG_BLOCK();
			bb_challengergui_CHGUI_OffsetX=Float(bb_graphics_DeviceWidth()/2-bb_challengergui_CHGUI_Width/2);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<819>");
		if(bb_challengergui_CHGUI_Height<bb_graphics_DeviceHeight()){
			DBG_BLOCK();
			bb_challengergui_CHGUI_OffsetY=Float(bb_graphics_DeviceHeight()/2-bb_challengergui_CHGUI_Height/2);
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<822>");
		if(bb_input_TouchDown(0)==0){
			DBG_BLOCK();
			bb_challengergui_CHGUI_DragMoving=0;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<825>");
	if(((bb_input_TouchDown(0))!=0) && bb_challengergui_CHGUI_TextBoxDown==0 && bb_challengergui_CHGUI_DragMoving==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<826>");
		if(bb_challengergui_CHGUI_Keyboard==2){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<827>");
			if(bb_input_TouchY(0)<Float(bb_graphics_DeviceHeight())-bb_challengergui_CHGUI_KeyboardWindow->f_H){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<828>");
				bb_challengergui_CHGUI_TargetX=bb_challengergui_CHGUI_OldX;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<829>");
				bb_challengergui_CHGUI_TargetY=bb_challengergui_CHGUI_OldY;
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<830>");
				bb_challengergui_CHGUI_Moving=1;
			}
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<833>");
			bb_challengergui_CHGUI_TargetX=bb_challengergui_CHGUI_OldX;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<834>");
			bb_challengergui_CHGUI_TargetY=bb_challengergui_CHGUI_OldY;
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<835>");
			bb_challengergui_CHGUI_Moving=1;
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<839>");
	if(bb_challengergui_CHGUI_TextBoxOver==0 && bb_challengergui_CHGUI_TextboxOnFocus==0 && bb_challengergui_CHGUI_Moving==0 && bb_challengergui_CHGUI_DragMoving==0){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<840>");
		bb_challengergui_CHGUI_OldX=bb_challengergui_CHGUI_OffsetX;
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<841>");
		bb_challengergui_CHGUI_OldY=bb_challengergui_CHGUI_OffsetY;
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<844>");
	bb_challengergui_CHGUI_UpdateMsgBox();
	return 0;
}
int bb_challengergui_CHGUI_Delete(bb_challengergui_CHGUI* t_N){
	DBG_ENTER("CHGUI_Delete")
	DBG_LOCAL(t_N,"N")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<933>");
	int t_C=0;
	DBG_LOCAL(t_C,"C")
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<936>");
	if(t_N->f_Element==String(L"Window",6) && t_N->f_Mode==2){
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<937>");
		for(t_C=0;t_C<=t_N->f_Parent->f_TopList.Length()-1;t_C=t_C+1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<938>");
			if(t_N->f_Parent->f_TopList.At(t_C)==t_N){
				DBG_BLOCK();
				break;
			}
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<941>");
		for(int t_NN=t_C;t_NN<=t_N->f_Parent->f_TopList.Length()-2;t_NN=t_NN+1){
			DBG_BLOCK();
			DBG_LOCAL(t_NN,"NN")
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<942>");
			gc_assign(t_N->f_Parent->f_TopList.At(t_NN),t_N->f_Parent->f_TopList.At(t_NN+1));
		}
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<944>");
		gc_assign(t_N->f_Parent->f_TopList,t_N->f_Parent->f_TopList.Slice(0,t_N->f_Parent->f_TopList.Length()-1));
	}else{
		DBG_BLOCK();
		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<947>");
		if(t_N->f_Element==String(L"Window",6) && t_N->f_Mode==1){
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<948>");
			for(t_C=0;t_C<=t_N->f_Parent->f_VariList.Length()-1;t_C=t_C+1){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<949>");
				if(t_N->f_Parent->f_VariList.At(t_C)==t_N){
					DBG_BLOCK();
					break;
				}
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<952>");
			for(int t_NN2=t_C;t_NN2<=t_N->f_Parent->f_VariList.Length()-2;t_NN2=t_NN2+1){
				DBG_BLOCK();
				DBG_LOCAL(t_NN2,"NN")
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<953>");
				gc_assign(t_N->f_Parent->f_VariList.At(t_NN2),t_N->f_Parent->f_VariList.At(t_NN2+1));
			}
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<955>");
			gc_assign(t_N->f_Parent->f_VariList,t_N->f_Parent->f_VariList.Slice(0,t_N->f_Parent->f_VariList.Length()-1));
		}else{
			DBG_BLOCK();
			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<958>");
			if(t_N->f_Element==String(L"Window",6) && t_N->f_Mode==0){
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<959>");
				for(t_C=0;t_C<=t_N->f_Parent->f_BottomList.Length()-1;t_C=t_C+1){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<960>");
					if(t_N->f_Parent->f_BottomList.At(t_C)==t_N){
						DBG_BLOCK();
						break;
					}
				}
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<963>");
				for(int t_NN3=t_C;t_NN3<=t_N->f_Parent->f_BottomList.Length()-2;t_NN3=t_NN3+1){
					DBG_BLOCK();
					DBG_LOCAL(t_NN3,"NN")
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<964>");
					gc_assign(t_N->f_Parent->f_BottomList.At(t_NN3),t_N->f_Parent->f_BottomList.At(t_NN3+1));
				}
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<966>");
				gc_assign(t_N->f_Parent->f_BottomList,t_N->f_Parent->f_BottomList.Slice(0,t_N->f_Parent->f_BottomList.Length()-1));
			}else{
				DBG_BLOCK();
				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<969>");
				if(t_N->f_Element==String(L"Menu",4)){
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<970>");
					for(t_C=0;t_C<=t_N->f_Parent->f_Menus.Length()-1;t_C=t_C+1){
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<971>");
						if(t_N->f_Parent->f_Menus.At(t_C)==t_N){
							DBG_BLOCK();
							break;
						}
					}
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<974>");
					for(int t_NN4=t_C;t_NN4<=t_N->f_Parent->f_Menus.Length()-2;t_NN4=t_NN4+1){
						DBG_BLOCK();
						DBG_LOCAL(t_NN4,"NN")
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<975>");
						gc_assign(t_N->f_Parent->f_Menus.At(t_NN4),t_N->f_Parent->f_Menus.At(t_NN4+1));
					}
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<977>");
					gc_assign(t_N->f_Parent->f_Menus,t_N->f_Parent->f_Menus.Slice(0,t_N->f_Parent->f_Menus.Length()-1));
				}else{
					DBG_BLOCK();
					DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<980>");
					if(t_N->f_Element==String(L"MenuItem",8)){
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<981>");
						for(t_C=0;t_C<=t_N->f_Parent->f_MenuItems.Length()-1;t_C=t_C+1){
							DBG_BLOCK();
							DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<982>");
							if(t_N->f_Parent->f_MenuItems.At(t_C)==t_N){
								DBG_BLOCK();
								break;
							}
						}
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<985>");
						for(int t_NN5=t_C;t_NN5<=t_N->f_Parent->f_MenuItems.Length()-2;t_NN5=t_NN5+1){
							DBG_BLOCK();
							DBG_LOCAL(t_NN5,"NN")
							DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<986>");
							gc_assign(t_N->f_Parent->f_MenuItems.At(t_NN5),t_N->f_Parent->f_MenuItems.At(t_NN5+1));
						}
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<988>");
						gc_assign(t_N->f_Parent->f_MenuItems,t_N->f_Parent->f_MenuItems.Slice(0,t_N->f_Parent->f_MenuItems.Length()-1));
					}else{
						DBG_BLOCK();
						DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<991>");
						if(t_N->f_Element==String(L"Tab",3)){
							DBG_BLOCK();
							DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<992>");
							for(t_C=0;t_C<=t_N->f_Parent->f_Tabs.Length()-1;t_C=t_C+1){
								DBG_BLOCK();
								DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<993>");
								if(t_N->f_Parent->f_Tabs.At(t_C)==t_N){
									DBG_BLOCK();
									break;
								}
							}
							DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<996>");
							for(int t_NN6=t_C;t_NN6<=t_N->f_Parent->f_Tabs.Length()-2;t_NN6=t_NN6+1){
								DBG_BLOCK();
								DBG_LOCAL(t_NN6,"NN")
								DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<997>");
								gc_assign(t_N->f_Parent->f_Tabs.At(t_NN6),t_N->f_Parent->f_Tabs.At(t_NN6+1));
							}
							DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<999>");
							gc_assign(t_N->f_Parent->f_Tabs,t_N->f_Parent->f_Tabs.Slice(0,t_N->f_Parent->f_Tabs.Length()-1));
						}else{
							DBG_BLOCK();
							DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1002>");
							if(t_N->f_Element==String(L"Button",6)){
								DBG_BLOCK();
								DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1003>");
								for(t_C=0;t_C<=t_N->f_Parent->f_Buttons.Length()-1;t_C=t_C+1){
									DBG_BLOCK();
									DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1004>");
									if(t_N->f_Parent->f_Buttons.At(t_C)==t_N){
										DBG_BLOCK();
										break;
									}
								}
								DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1007>");
								for(int t_NN7=t_C;t_NN7<=t_N->f_Parent->f_Buttons.Length()-2;t_NN7=t_NN7+1){
									DBG_BLOCK();
									DBG_LOCAL(t_NN7,"NN")
									DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1008>");
									gc_assign(t_N->f_Parent->f_Buttons.At(t_NN7),t_N->f_Parent->f_Buttons.At(t_NN7+1));
								}
								DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1010>");
								gc_assign(t_N->f_Parent->f_Buttons,t_N->f_Parent->f_Buttons.Slice(0,t_N->f_Parent->f_Buttons.Length()-1));
							}else{
								DBG_BLOCK();
								DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1013>");
								if(t_N->f_Element==String(L"ImageButton",11)){
									DBG_BLOCK();
									DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1014>");
									for(t_C=0;t_C<=t_N->f_Parent->f_ImageButtons.Length()-1;t_C=t_C+1){
										DBG_BLOCK();
										DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1015>");
										if(t_N->f_Parent->f_ImageButtons.At(t_C)==t_N){
											DBG_BLOCK();
											break;
										}
									}
									DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1018>");
									for(int t_NN8=t_C;t_NN8<=t_N->f_Parent->f_ImageButtons.Length()-2;t_NN8=t_NN8+1){
										DBG_BLOCK();
										DBG_LOCAL(t_NN8,"NN")
										DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1019>");
										gc_assign(t_N->f_Parent->f_ImageButtons.At(t_NN8),t_N->f_Parent->f_ImageButtons.At(t_NN8+1));
									}
									DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1021>");
									gc_assign(t_N->f_Parent->f_ImageButtons,t_N->f_Parent->f_ImageButtons.Slice(0,t_N->f_Parent->f_ImageButtons.Length()-1));
								}else{
									DBG_BLOCK();
									DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1024>");
									if(t_N->f_Element==String(L"Tickbox",7)){
										DBG_BLOCK();
										DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1025>");
										for(t_C=0;t_C<=t_N->f_Parent->f_Tickboxes.Length()-1;t_C=t_C+1){
											DBG_BLOCK();
											DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1026>");
											if(t_N->f_Parent->f_Tickboxes.At(t_C)==t_N){
												DBG_BLOCK();
												break;
											}
										}
										DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1029>");
										for(int t_NN9=t_C;t_NN9<=t_N->f_Parent->f_Tickboxes.Length()-2;t_NN9=t_NN9+1){
											DBG_BLOCK();
											DBG_LOCAL(t_NN9,"NN")
											DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1030>");
											gc_assign(t_N->f_Parent->f_Tickboxes.At(t_NN9),t_N->f_Parent->f_Tickboxes.At(t_NN9+1));
										}
										DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1032>");
										gc_assign(t_N->f_Parent->f_Tickboxes,t_N->f_Parent->f_Tickboxes.Slice(0,t_N->f_Parent->f_Tickboxes.Length()-1));
									}else{
										DBG_BLOCK();
										DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1035>");
										if(t_N->f_Element==String(L"Radio",5)){
											DBG_BLOCK();
											DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1036>");
											for(t_C=0;t_C<=t_N->f_Parent->f_Radioboxes.Length()-1;t_C=t_C+1){
												DBG_BLOCK();
												DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1037>");
												if(t_N->f_Parent->f_Radioboxes.At(t_C)==t_N){
													DBG_BLOCK();
													break;
												}
											}
											DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1040>");
											for(int t_NN10=t_C;t_NN10<=t_N->f_Parent->f_Radioboxes.Length()-2;t_NN10=t_NN10+1){
												DBG_BLOCK();
												DBG_LOCAL(t_NN10,"NN")
												DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1041>");
												gc_assign(t_N->f_Parent->f_Radioboxes.At(t_NN10),t_N->f_Parent->f_Radioboxes.At(t_NN10+1));
											}
											DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1043>");
											gc_assign(t_N->f_Parent->f_Radioboxes,t_N->f_Parent->f_Radioboxes.Slice(0,t_N->f_Parent->f_Radioboxes.Length()-1));
										}else{
											DBG_BLOCK();
											DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1046>");
											if(t_N->f_Element==String(L"Listbox",7)){
												DBG_BLOCK();
												DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1047>");
												for(t_C=0;t_C<=t_N->f_Parent->f_Listboxes.Length()-1;t_C=t_C+1){
													DBG_BLOCK();
													DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1048>");
													if(t_N->f_Parent->f_Listboxes.At(t_C)==t_N){
														DBG_BLOCK();
														break;
													}
												}
												DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1051>");
												for(int t_NN11=t_C;t_NN11<=t_N->f_Parent->f_Listboxes.Length()-2;t_NN11=t_NN11+1){
													DBG_BLOCK();
													DBG_LOCAL(t_NN11,"NN")
													DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1052>");
													gc_assign(t_N->f_Parent->f_Listboxes.At(t_NN11),t_N->f_Parent->f_Listboxes.At(t_NN11+1));
												}
												DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1054>");
												gc_assign(t_N->f_Parent->f_Listboxes,t_N->f_Parent->f_Listboxes.Slice(0,t_N->f_Parent->f_Listboxes.Length()-1));
											}else{
												DBG_BLOCK();
												DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1057>");
												if(t_N->f_Element==String(L"ListboxItem",11)){
													DBG_BLOCK();
													DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1058>");
													for(t_C=0;t_C<=t_N->f_Parent->f_ListboxItems.Length()-1;t_C=t_C+1){
														DBG_BLOCK();
														DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1059>");
														if(t_N->f_Parent->f_ListboxItems.At(t_C)==t_N){
															DBG_BLOCK();
															break;
														}
													}
													DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1062>");
													for(int t_NN12=t_C;t_NN12<=t_N->f_Parent->f_ListboxItems.Length()-2;t_NN12=t_NN12+1){
														DBG_BLOCK();
														DBG_LOCAL(t_NN12,"NN")
														DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1063>");
														gc_assign(t_N->f_Parent->f_ListboxItems.At(t_NN12),t_N->f_Parent->f_ListboxItems.At(t_NN12+1));
													}
													DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1065>");
													gc_assign(t_N->f_Parent->f_ListboxItems,t_N->f_Parent->f_ListboxItems.Slice(0,t_N->f_Parent->f_ListboxItems.Length()-1));
												}else{
													DBG_BLOCK();
													DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1068>");
													if(t_N->f_Element==String(L"Dropdown",8)){
														DBG_BLOCK();
														DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1069>");
														for(t_C=0;t_C<=t_N->f_Parent->f_Dropdowns.Length()-1;t_C=t_C+1){
															DBG_BLOCK();
															DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1070>");
															if(t_N->f_Parent->f_Dropdowns.At(t_C)==t_N){
																DBG_BLOCK();
																break;
															}
														}
														DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1073>");
														for(int t_NN13=t_C;t_NN13<=t_N->f_Parent->f_Dropdowns.Length()-2;t_NN13=t_NN13+1){
															DBG_BLOCK();
															DBG_LOCAL(t_NN13,"NN")
															DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1074>");
															gc_assign(t_N->f_Parent->f_Dropdowns.At(t_NN13),t_N->f_Parent->f_Dropdowns.At(t_NN13+1));
														}
														DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1076>");
														gc_assign(t_N->f_Parent->f_Dropdowns,t_N->f_Parent->f_Dropdowns.Slice(0,t_N->f_Parent->f_Dropdowns.Length()-1));
													}else{
														DBG_BLOCK();
														DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1079>");
														if(t_N->f_Element==String(L"DropdownItem",12)){
															DBG_BLOCK();
															DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1080>");
															for(t_C=0;t_C<=t_N->f_Parent->f_DropdownItems.Length()-1;t_C=t_C+1){
																DBG_BLOCK();
																DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1081>");
																if(t_N->f_Parent->f_DropdownItems.At(t_C)==t_N){
																	DBG_BLOCK();
																	break;
																}
															}
															DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1084>");
															for(int t_NN14=t_C;t_NN14<=t_N->f_Parent->f_DropdownItems.Length()-2;t_NN14=t_NN14+1){
																DBG_BLOCK();
																DBG_LOCAL(t_NN14,"NN")
																DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1085>");
																gc_assign(t_N->f_Parent->f_DropdownItems.At(t_NN14),t_N->f_Parent->f_DropdownItems.At(t_NN14+1));
															}
															DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1087>");
															gc_assign(t_N->f_Parent->f_DropdownItems,t_N->f_Parent->f_DropdownItems.Slice(0,t_N->f_Parent->f_DropdownItems.Length()-1));
														}else{
															DBG_BLOCK();
															DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1090>");
															if(t_N->f_Element==String(L"Textfield",9)){
																DBG_BLOCK();
																DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1091>");
																for(t_C=0;t_C<=t_N->f_Parent->f_Textfields.Length()-1;t_C=t_C+1){
																	DBG_BLOCK();
																	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1092>");
																	if(t_N->f_Parent->f_Textfields.At(t_C)==t_N){
																		DBG_BLOCK();
																		break;
																	}
																}
																DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1095>");
																for(int t_NN15=t_C;t_NN15<=t_N->f_Parent->f_Textfields.Length()-2;t_NN15=t_NN15+1){
																	DBG_BLOCK();
																	DBG_LOCAL(t_NN15,"NN")
																	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1096>");
																	gc_assign(t_N->f_Parent->f_Textfields.At(t_NN15),t_N->f_Parent->f_Textfields.At(t_NN15+1));
																}
																DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1098>");
																gc_assign(t_N->f_Parent->f_Textfields,t_N->f_Parent->f_Textfields.Slice(0,t_N->f_Parent->f_Textfields.Length()-1));
															}else{
																DBG_BLOCK();
																DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1101>");
																if(t_N->f_Element==String(L"Label",5)){
																	DBG_BLOCK();
																	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1102>");
																	for(t_C=0;t_C<=t_N->f_Parent->f_Labels.Length()-1;t_C=t_C+1){
																		DBG_BLOCK();
																		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1103>");
																		if(t_N->f_Parent->f_Labels.At(t_C)==t_N){
																			DBG_BLOCK();
																			break;
																		}
																	}
																	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1106>");
																	for(int t_NN16=t_C;t_NN16<=t_N->f_Parent->f_Labels.Length()-2;t_NN16=t_NN16+1){
																		DBG_BLOCK();
																		DBG_LOCAL(t_NN16,"NN")
																		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1107>");
																		gc_assign(t_N->f_Parent->f_Labels.At(t_NN16),t_N->f_Parent->f_Labels.At(t_NN16+1));
																	}
																	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1109>");
																	gc_assign(t_N->f_Parent->f_Labels,t_N->f_Parent->f_Labels.Slice(0,t_N->f_Parent->f_Labels.Length()-1));
																}else{
																	DBG_BLOCK();
																	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1112>");
																	if(t_N->f_Element==String(L"VSlider",7)){
																		DBG_BLOCK();
																		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1113>");
																		for(t_C=0;t_C<=t_N->f_Parent->f_VSliders.Length()-1;t_C=t_C+1){
																			DBG_BLOCK();
																			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1114>");
																			if(t_N->f_Parent->f_VSliders.At(t_C)==t_N){
																				DBG_BLOCK();
																				break;
																			}
																		}
																		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1117>");
																		for(int t_NN17=t_C;t_NN17<=t_N->f_Parent->f_VSliders.Length()-2;t_NN17=t_NN17+1){
																			DBG_BLOCK();
																			DBG_LOCAL(t_NN17,"NN")
																			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1118>");
																			gc_assign(t_N->f_Parent->f_VSliders.At(t_NN17),t_N->f_Parent->f_VSliders.At(t_NN17+1));
																		}
																		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1120>");
																		gc_assign(t_N->f_Parent->f_VSliders,t_N->f_Parent->f_VSliders.Slice(0,t_N->f_Parent->f_VSliders.Length()-1));
																	}else{
																		DBG_BLOCK();
																		DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1123>");
																		if(t_N->f_Element==String(L"HSlider",7)){
																			DBG_BLOCK();
																			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1124>");
																			for(t_C=0;t_C<=t_N->f_Parent->f_HSliders.Length()-1;t_C=t_C+1){
																				DBG_BLOCK();
																				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1125>");
																				if(t_N->f_Parent->f_HSliders.At(t_C)==t_N){
																					DBG_BLOCK();
																					break;
																				}
																			}
																			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1128>");
																			for(int t_NN18=t_C;t_NN18<=t_N->f_Parent->f_HSliders.Length()-2;t_NN18=t_NN18+1){
																				DBG_BLOCK();
																				DBG_LOCAL(t_NN18,"NN")
																				DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1129>");
																				gc_assign(t_N->f_Parent->f_HSliders.At(t_NN18),t_N->f_Parent->f_HSliders.At(t_NN18+1));
																			}
																			DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1131>");
																			gc_assign(t_N->f_Parent->f_HSliders,t_N->f_Parent->f_HSliders.Slice(0,t_N->f_Parent->f_HSliders.Length()-1));
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
					}
				}
			}
		}
	}
	DBG_INFO("C:/Program Files (x86)/Monkey/modules/challengergui/challengergui.monkey<1137>");
	t_N=0;
	return 0;
}
int bb_protocol_RequestBeaconList(String t_game){
	DBG_ENTER("RequestBeaconList")
	DBG_LOCAL(t_game,"game")
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<16>");
	gc_assign(bb_data2_Server,(new bb_tcpstream_TcpStream)->g_new());
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<17>");
	do{
		DBG_BLOCK();
	}while(!(bb_data2_Server->m_Connect(String(L"www.fuzzit.us",13),80)));
	DBG_INFO("J:/WORK/Fuzzit/iOS Beacon Demo/repo/wurtland/iOS Apps/protocol.monkey<19>");
	bb_protocol_Post(String(L"http://www.fuzzit.us/cgi-bin/GlobalServer.py?action=mobilegetbeaconlist&game=",77)+t_game);
	return 0;
}
int bbInit(){
	bb_reflection__classesMap=0;
	DBG_GLOBAL("_classesMap",&bb_reflection__classesMap);
	bb_reflection__classes=Array<bb_reflection_ClassInfo* >();
	DBG_GLOBAL("_classes",&bb_reflection__classes);
	bb_reflection__getClass=0;
	DBG_GLOBAL("_getClass",&bb_reflection__getClass);
	bb_reflection__boolClass=0;
	DBG_GLOBAL("_boolClass",&bb_reflection__boolClass);
	bb_reflection__intClass=0;
	DBG_GLOBAL("_intClass",&bb_reflection__intClass);
	bb_reflection__floatClass=0;
	DBG_GLOBAL("_floatClass",&bb_reflection__floatClass);
	bb_reflection__stringClass=0;
	DBG_GLOBAL("_stringClass",&bb_reflection__stringClass);
	bb_reflection__functions=Array<bb_reflection_FunctionInfo* >();
	DBG_GLOBAL("_functions",&bb_reflection__functions);
	bb_reflection__init=bb_reflection___init();
	DBG_GLOBAL("_init",&bb_reflection__init);
	bb_graphics_device=0;
	DBG_GLOBAL("device",&bb_graphics_device);
	bb_input_device=0;
	DBG_GLOBAL("device",&bb_input_device);
	bb_audio_device=0;
	DBG_GLOBAL("device",&bb_audio_device);
	bb_app_device=0;
	DBG_GLOBAL("device",&bb_app_device);
	bb_reflection__unknownClass=((new bb_reflection_UnknownClass)->g_new());
	DBG_GLOBAL("_unknownClass",&bb_reflection__unknownClass);
	bb_graphics_context=(new bb_graphics_GraphicsContext)->g_new();
	DBG_GLOBAL("context",&bb_graphics_context);
	bb_graphics_Image::g_DefaultFlags=0;
	DBG_GLOBAL("DefaultFlags",&bb_graphics_Image::g_DefaultFlags);
	bb_graphics_renderDevice=0;
	DBG_GLOBAL("renderDevice",&bb_graphics_renderDevice);
	bb_challengergui_CHGUI_MobileMode=0;
	DBG_GLOBAL("CHGUI_MobileMode",&bb_challengergui_CHGUI_MobileMode);
	bb_data2_STATUS=String(L"start",5);
	DBG_GLOBAL("STATUS",&bb_data2_STATUS);
	bb_data2_Server=0;
	DBG_GLOBAL("Server",&bb_data2_Server);
	bb_stream_Stream::g__tmpbuf=(new bb_databuffer_DataBuffer)->g_new(4096);
	DBG_GLOBAL("_tmpbuf",&bb_stream_Stream::g__tmpbuf);
	bb_data2_Game=0;
	DBG_GLOBAL("Game",&bb_data2_Game);
	bb_challengergui_CHGUI_BottomList=Array<bb_challengergui_CHGUI* >();
	DBG_GLOBAL("CHGUI_BottomList",&bb_challengergui_CHGUI_BottomList);
	bb_challengergui_CHGUI_Canvas=0;
	DBG_GLOBAL("CHGUI_Canvas",&bb_challengergui_CHGUI_Canvas);
	bb_challengergui_CHGUI_OffsetX=FLOAT(.0);
	DBG_GLOBAL("CHGUI_OffsetX",&bb_challengergui_CHGUI_OffsetX);
	bb_challengergui_CHGUI_OffsetY=FLOAT(.0);
	DBG_GLOBAL("CHGUI_OffsetY",&bb_challengergui_CHGUI_OffsetY);
	bb_challengergui_CHGUI_TitleHeight=FLOAT(25.0);
	DBG_GLOBAL("CHGUI_TitleHeight",&bb_challengergui_CHGUI_TitleHeight);
	bb_challengergui_CHGUI_LockedWIndow=0;
	DBG_GLOBAL("CHGUI_LockedWIndow",&bb_challengergui_CHGUI_LockedWIndow);
	bb_challengergui_CHGUI_Shadow=1;
	DBG_GLOBAL("CHGUI_Shadow",&bb_challengergui_CHGUI_Shadow);
	bb_challengergui_CHGUI_ShadowImg=0;
	DBG_GLOBAL("CHGUI_ShadowImg",&bb_challengergui_CHGUI_ShadowImg);
	bb_challengergui_CHGUI_Style=0;
	DBG_GLOBAL("CHGUI_Style",&bb_challengergui_CHGUI_Style);
	bb_challengergui_CHGUI_TitleFont=0;
	DBG_GLOBAL("CHGUI_TitleFont",&bb_challengergui_CHGUI_TitleFont);
	bb_challengergui_CHGUI_Font=0;
	DBG_GLOBAL("CHGUI_Font",&bb_challengergui_CHGUI_Font);
	bb_challengergui_CHGUI_KeyboardButtons=Array<bb_challengergui_CHGUI* >(109);
	DBG_GLOBAL("CHGUI_KeyboardButtons",&bb_challengergui_CHGUI_KeyboardButtons);
	bb_challengergui_CHGUI_ShiftHold=0;
	DBG_GLOBAL("CHGUI_ShiftHold",&bb_challengergui_CHGUI_ShiftHold);
	bb_challengergui_CHGUI_Cursor=0;
	DBG_GLOBAL("CHGUI_Cursor",&bb_challengergui_CHGUI_Cursor);
	bb_challengergui_CHGUI_VariList=Array<bb_challengergui_CHGUI* >();
	DBG_GLOBAL("CHGUI_VariList",&bb_challengergui_CHGUI_VariList);
	bb_challengergui_CHGUI_TopList=Array<bb_challengergui_CHGUI* >();
	DBG_GLOBAL("CHGUI_TopList",&bb_challengergui_CHGUI_TopList);
	bb_challengergui_CHGUI_TooltipFlag=0;
	DBG_GLOBAL("CHGUI_TooltipFlag",&bb_challengergui_CHGUI_TooltipFlag);
	bb_challengergui_CHGUI_TooltipFont=0;
	DBG_GLOBAL("CHGUI_TooltipFont",&bb_challengergui_CHGUI_TooltipFont);
	bb_challengergui_CHGUI_Millisecs=0;
	DBG_GLOBAL("CHGUI_Millisecs",&bb_challengergui_CHGUI_Millisecs);
	bb_challengergui_CHGUI_FPSCounter=0;
	DBG_GLOBAL("CHGUI_FPSCounter",&bb_challengergui_CHGUI_FPSCounter);
	bb_challengergui_CHGUI_FPS=0;
	DBG_GLOBAL("CHGUI_FPS",&bb_challengergui_CHGUI_FPS);
	bb_challengergui_CHGUI_Width=0;
	DBG_GLOBAL("CHGUI_Width",&bb_challengergui_CHGUI_Width);
	bb_challengergui_CHGUI_Height=0;
	DBG_GLOBAL("CHGUI_Height",&bb_challengergui_CHGUI_Height);
	bb_challengergui_CHGUI_CanvasFlag=0;
	DBG_GLOBAL("CHGUI_CanvasFlag",&bb_challengergui_CHGUI_CanvasFlag);
	bb_challengergui_CHGUI_Started=0;
	DBG_GLOBAL("CHGUI_Started",&bb_challengergui_CHGUI_Started);
	bb_challengergui_CHGUI_TopTop=0;
	DBG_GLOBAL("CHGUI_TopTop",&bb_challengergui_CHGUI_TopTop);
	bb_challengergui_CHGUI_KeyboardWindow=0;
	DBG_GLOBAL("CHGUI_KeyboardWindow",&bb_challengergui_CHGUI_KeyboardWindow);
	bb_challengergui_CHGUI_MsgBoxWindow=0;
	DBG_GLOBAL("CHGUI_MsgBoxWindow",&bb_challengergui_CHGUI_MsgBoxWindow);
	bb_challengergui_CHGUI_MsgBoxLabel=0;
	DBG_GLOBAL("CHGUI_MsgBoxLabel",&bb_challengergui_CHGUI_MsgBoxLabel);
	bb_challengergui_CHGUI_MsgBoxButton=0;
	DBG_GLOBAL("CHGUI_MsgBoxButton",&bb_challengergui_CHGUI_MsgBoxButton);
	bb_data2_SCALE_W=FLOAT(300.0);
	DBG_GLOBAL("SCALE_W",&bb_data2_SCALE_W);
	bb_data2_SCALE_H=FLOAT(480.0);
	DBG_GLOBAL("SCALE_H",&bb_data2_SCALE_H);
	bb_challengergui_CHGUI_MouseBusy=0;
	DBG_GLOBAL("CHGUI_MouseBusy",&bb_challengergui_CHGUI_MouseBusy);
	bb_challengergui_CHGUI_Over=0;
	DBG_GLOBAL("CHGUI_Over",&bb_challengergui_CHGUI_Over);
	bb_challengergui_CHGUI_OverFlag=0;
	DBG_GLOBAL("CHGUI_OverFlag",&bb_challengergui_CHGUI_OverFlag);
	bb_challengergui_CHGUI_DownFlag=0;
	DBG_GLOBAL("CHGUI_DownFlag",&bb_challengergui_CHGUI_DownFlag);
	bb_challengergui_CHGUI_MenuOver=0;
	DBG_GLOBAL("CHGUI_MenuOver",&bb_challengergui_CHGUI_MenuOver);
	bb_challengergui_CHGUI_TextBoxOver=0;
	DBG_GLOBAL("CHGUI_TextBoxOver",&bb_challengergui_CHGUI_TextBoxOver);
	bb_challengergui_CHGUI_TextboxOnFocus=0;
	DBG_GLOBAL("CHGUI_TextboxOnFocus",&bb_challengergui_CHGUI_TextboxOnFocus);
	bb_challengergui_CHGUI_TextBoxDown=0;
	DBG_GLOBAL("CHGUI_TextBoxDown",&bb_challengergui_CHGUI_TextBoxDown);
	bb_challengergui_CHGUI_DragOver=0;
	DBG_GLOBAL("CHGUI_DragOver",&bb_challengergui_CHGUI_DragOver);
	bb_challengergui_CHGUI_Moving=0;
	DBG_GLOBAL("CHGUI_Moving",&bb_challengergui_CHGUI_Moving);
	bb_challengergui_CHGUI_TargetY=FLOAT(.0);
	DBG_GLOBAL("CHGUI_TargetY",&bb_challengergui_CHGUI_TargetY);
	bb_challengergui_CHGUI_TargetX=FLOAT(.0);
	DBG_GLOBAL("CHGUI_TargetX",&bb_challengergui_CHGUI_TargetX);
	bb_challengergui_CHGUI_IgnoreMouse=0;
	DBG_GLOBAL("CHGUI_IgnoreMouse",&bb_challengergui_CHGUI_IgnoreMouse);
	bb_challengergui_CHGUI_Tooltips=1;
	DBG_GLOBAL("CHGUI_Tooltips",&bb_challengergui_CHGUI_Tooltips);
	bb_challengergui_CHGUI_TooltipTime=1500;
	DBG_GLOBAL("CHGUI_TooltipTime",&bb_challengergui_CHGUI_TooltipTime);
	bb_challengergui_CHGUI_MenuClose=0;
	DBG_GLOBAL("CHGUI_MenuClose",&bb_challengergui_CHGUI_MenuClose);
	bb_challengergui_CHGUI_TextboxFocus=0;
	DBG_GLOBAL("CHGUI_TextboxFocus",&bb_challengergui_CHGUI_TextboxFocus);
	bb_challengergui_CHGUI_Keyboard=1;
	DBG_GLOBAL("CHGUI_Keyboard",&bb_challengergui_CHGUI_Keyboard);
	bb_challengergui_CHGUI_ShowKeyboard=0;
	DBG_GLOBAL("CHGUI_ShowKeyboard",&bb_challengergui_CHGUI_ShowKeyboard);
	bb_challengergui_CHGUI_AutoTextScroll=0;
	DBG_GLOBAL("CHGUI_AutoTextScroll",&bb_challengergui_CHGUI_AutoTextScroll);
	bb_challengergui_CHGUI_KeyboardPage=0;
	DBG_GLOBAL("CHGUI_KeyboardPage",&bb_challengergui_CHGUI_KeyboardPage);
	bb_challengergui_CHGUI_KeyboardShift=0;
	DBG_GLOBAL("CHGUI_KeyboardShift",&bb_challengergui_CHGUI_KeyboardShift);
	bb_challengergui_CHGUI_OldX=FLOAT(.0);
	DBG_GLOBAL("CHGUI_OldX",&bb_challengergui_CHGUI_OldX);
	bb_challengergui_CHGUI_OldY=FLOAT(.0);
	DBG_GLOBAL("CHGUI_OldY",&bb_challengergui_CHGUI_OldY);
	bb_challengergui_CHGUI_CursorMillisecs=0;
	DBG_GLOBAL("CHGUI_CursorMillisecs",&bb_challengergui_CHGUI_CursorMillisecs);
	bb_challengergui_CHGUI_DragScroll=0;
	DBG_GLOBAL("CHGUI_DragScroll",&bb_challengergui_CHGUI_DragScroll);
	bb_challengergui_CHGUI_DragMoving=0;
	DBG_GLOBAL("CHGUI_DragMoving",&bb_challengergui_CHGUI_DragMoving);
	bb_challengergui_CHGUI_OffsetMX=FLOAT(.0);
	DBG_GLOBAL("CHGUI_OffsetMX",&bb_challengergui_CHGUI_OffsetMX);
	bb_challengergui_CHGUI_OffsetMY=FLOAT(.0);
	DBG_GLOBAL("CHGUI_OffsetMY",&bb_challengergui_CHGUI_OffsetMY);
	return 0;
}
void gc_mark(){
	gc_mark_q(bb_reflection__classesMap);
	gc_mark_q(bb_reflection__classes);
	gc_mark_q(bb_reflection__getClass);
	gc_mark_q(bb_reflection__boolClass);
	gc_mark_q(bb_reflection__intClass);
	gc_mark_q(bb_reflection__floatClass);
	gc_mark_q(bb_reflection__stringClass);
	gc_mark_q(bb_reflection__functions);
	gc_mark_q(bb_graphics_device);
	gc_mark_q(bb_input_device);
	gc_mark_q(bb_audio_device);
	gc_mark_q(bb_app_device);
	gc_mark_q(bb_reflection__unknownClass);
	gc_mark_q(bb_graphics_context);
	gc_mark_q(bb_graphics_renderDevice);
	gc_mark_q(bb_data2_Server);
	gc_mark_q(bb_stream_Stream::g__tmpbuf);
	gc_mark_q(bb_data2_Game);
	gc_mark_q(bb_challengergui_CHGUI_BottomList);
	gc_mark_q(bb_challengergui_CHGUI_Canvas);
	gc_mark_q(bb_challengergui_CHGUI_LockedWIndow);
	gc_mark_q(bb_challengergui_CHGUI_ShadowImg);
	gc_mark_q(bb_challengergui_CHGUI_Style);
	gc_mark_q(bb_challengergui_CHGUI_TitleFont);
	gc_mark_q(bb_challengergui_CHGUI_Font);
	gc_mark_q(bb_challengergui_CHGUI_KeyboardButtons);
	gc_mark_q(bb_challengergui_CHGUI_VariList);
	gc_mark_q(bb_challengergui_CHGUI_TopList);
	gc_mark_q(bb_challengergui_CHGUI_TooltipFlag);
	gc_mark_q(bb_challengergui_CHGUI_TooltipFont);
	gc_mark_q(bb_challengergui_CHGUI_TopTop);
	gc_mark_q(bb_challengergui_CHGUI_KeyboardWindow);
	gc_mark_q(bb_challengergui_CHGUI_MsgBoxWindow);
	gc_mark_q(bb_challengergui_CHGUI_MsgBoxLabel);
	gc_mark_q(bb_challengergui_CHGUI_MsgBoxButton);
	gc_mark_q(bb_challengergui_CHGUI_TextboxFocus);
}
//${TRANSCODE_END}

FILE *fopenFile( String path,String mode ){

	if( !path.StartsWith( "monkey://" ) ){
		path=path;
	}else if( path.StartsWith( "monkey://data/" ) ){
		path=String("./data/")+path.Slice(14);
	}else if( path.StartsWith( "monkey://internal/" ) ){
		path=String("./internal/")+path.Slice(18);
	}else if( path.StartsWith( "monkey://external/" ) ){
		path=String("./external/")+path.Slice(18);
	}else{
		return 0;
	}

#if _WIN32
	return _wfopen( path.ToCString<wchar_t>(),mode.ToCString<wchar_t>() );
#else
	return fopen( path.ToCString<char>(),mode.ToCString<char>() );
#endif
}

static String extractExt( String path ){
	int i=path.FindLast( "." )+1;
	if( i && path.Find( "/",i )==-1 && path.Find( "\\",i )==-1 ) return path.Slice( i );
	return "";
}

unsigned char *loadImage( String path,int *width,int *height,int *depth ){
	FILE *f=fopenFile( path,"rb" );
	if( !f ) return 0;
	unsigned char *data=stbi_load_from_file( f,width,height,depth,0 );
	fclose( f );
	return data;
}

unsigned char *loadImage( unsigned char *data,int length,int *width,int *height,int *depth ){
	return stbi_load_from_memory( data,length,width,height,depth,0 );
}

void unloadImage( unsigned char *data ){
	stbi_image_free( data );
}

//for reading WAV file...
static const char *readTag( FILE *f ){
	static char buf[8];
	if( fread( buf,4,1,f )!=1 ) return "";
	buf[4]=0;
	return buf;
}

static int readInt( FILE *f ){
	unsigned char buf[4];
	if( fread( buf,4,1,f )!=1 ) return -1;
	return (buf[3]<<24) | (buf[2]<<16) | (buf[1]<<8) | buf[0];
}

static int readShort( FILE *f ){
	unsigned char buf[2];
	if( fread( buf,2,1,f )!=1 ) return -1;
	return (buf[1]<<8) | buf[0];
}

static void skipBytes( int n,FILE *f ){
	char *p=(char*)malloc( n );
	fread( p,n,1,f );
	free(p);
}

static unsigned char *loadSound_wav( String path,int *plength,int *pchannels,int *pformat,int *phertz ){

	if( FILE *f=fopenFile( path,"rb" ) ){
		if( !strcmp( readTag( f ),"RIFF" ) ){
			int len=readInt( f )-8;len=len;
			if( !strcmp( readTag( f ),"WAVE" ) ){
				if( !strcmp( readTag( f ),"fmt " ) ){
					int len2=readInt( f );
					int comp=readShort( f );
					if( comp==1 ){
						int chans=readShort( f );
						int hertz=readInt( f );
						int bytespersec=readInt( f );bytespersec=bytespersec;
						int pad=readShort( f );pad=pad;
						int bits=readShort( f );
						int format=bits/8;
						if( len2>16 ) skipBytes( len2-16,f );
						for(;;){
							const char *p=readTag( f );
							if( feof( f ) ) break;
							int size=readInt( f );
							if( strcmp( p,"data" ) ){
								skipBytes( size,f );
								continue;
							}
							unsigned char *data=(unsigned char*)malloc( size );
							if( fread( data,size,1,f )==1 ){
								*plength=size/(chans*format);
								*pchannels=chans;
								*pformat=format;
								*phertz=hertz;
								fclose( f );
								return data;
							}
							free( data );
						}
					}
				}
			}
		}
		fclose( f );
	}
	return 0;
}

static unsigned char *loadSound_ogg( String path,int *length,int *channels,int *format,int *hertz ){

	FILE *f=fopenFile( path,"rb" );
	if( !f ) return 0;
	
	int error;
	stb_vorbis *v=stb_vorbis_open_file( f,0,&error,0 );
	if( !v ){
		fclose( f );
		return 0;
	}
	
	stb_vorbis_info info=stb_vorbis_get_info( v );
	
	int limit=info.channels*4096;
	int offset=0,data_len=0,total=limit;

	short *data=(short*)malloc( total*sizeof(short) );
	
	for(;;){
		int n=stb_vorbis_get_frame_short_interleaved( v,info.channels,data+offset,total-offset );
		if( !n ) break;
	
		data_len+=n;
		offset+=n*info.channels;
		
		if( offset+limit>total ){
			total*=2;
			data=(short*)realloc( data,total*sizeof(short) );
		}
	}
	
	*length=data_len;
	*channels=info.channels;
	*format=2;
	*hertz=info.sample_rate;
	
	stb_vorbis_close(v);
	fclose( f );

	return (unsigned char*)data;
}

unsigned char *loadSound( String path,int *length,int *channels,int *format,int *hertz ){

	String ext=extractExt( path ).ToLower();
	
	if( ext=="wav" ) return loadSound_wav( path,length,channels,format,hertz );

	if( ext=="ogg" ) return loadSound_ogg( path,length,channels,format,hertz );
	
	return 0;
}

void unloadSound( unsigned char *data ){
	free( data );
}

ALCdevice *alcDevice;

ALCcontext *alcContext;

void warn( const char *p ){
	puts( p );
}

void fail( const char *p ){
	puts( p );
	exit( -1 );
}

int main( int argc,const char *argv[] ){

	if( !glfwInit() ){
		puts( "glfwInit failed" );
		exit( -1 );
	}

	GLFWvidmode desktopMode;
	glfwGetDesktopMode( &desktopMode );
	
	int w=CFG_GLFW_WINDOW_WIDTH;
	if( !w ) w=desktopMode.Width;
	
	int h=CFG_GLFW_WINDOW_HEIGHT;
	if( !h ) h=desktopMode.Height;
	
	glfwOpenWindowHint( GLFW_WINDOW_NO_RESIZE,CFG_GLFW_WINDOW_RESIZABLE ? GL_FALSE : GL_TRUE );
	
	if( !glfwOpenWindow( w,h, 0,0,0,0,CFG_OPENGL_DEPTH_BUFFER_ENABLED ? 32 : 0,0,CFG_GLFW_WINDOW_FULLSCREEN ? GLFW_FULLSCREEN : GLFW_WINDOW  ) ){
		fail( "glfwOpenWindow failed" );
	}

	glfwSetWindowPos( (desktopMode.Width-w)/2,(desktopMode.Height-h)/2 );	

	glfwSetWindowTitle( _STRINGIZE(CFG_GLFW_WINDOW_TITLE) );
	
	if( (alcDevice=alcOpenDevice( 0 )) ){
		if( (alcContext=alcCreateContext( alcDevice,0 )) ){
			if( alcMakeContextCurrent( alcContext ) ){
				//alc all go!
			}else{
				warn( "alcMakeContextCurrent failed" );
			}
		}else{
			warn( "alcCreateContext failed" );
		}
	}else{
		warn( "alcOpenDevice failed" );
	}
	
#if INIT_GL_EXTS
	Init_GL_Exts();
#endif

	try{
	
		bb_std_main( argc,argv );

		if( runner ) runner();
		
	}catch( ThrowableObject *ex ){
	
		Print( "Monkey Runtime Error : Uncaught Monkey Exception" );

	}catch( const char *err ){

	}
	
	if( alcContext ) alcDestroyContext( alcContext );

	if( alcDevice ) alcCloseDevice( alcDevice );

	glfwTerminate();

	return 0;
}
