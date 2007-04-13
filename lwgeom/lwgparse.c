/*
 * Written by Ralph Mason ralph.mason<at>telogis.com
 *
 * Copyright Telogis 2004
 * www.telogis.com
 *
 */
#include <string.h>
#include <stdio.h>
/* Solaris9 does not provide stdint.h */
/* #include <stdint.h> */
#include <inttypes.h>

#include "liblwgeom.h"
#include "wktparse.h"

/*
 * To get byte order
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
*/

void set_zm(char z, char m);
void close_parser(void);

typedef uint32_t int4;

typedef struct tag_tuple tuple;

struct tag_outputstate{
	uchar*	pos;
};

typedef struct tag_outputstate output_state;
typedef void (*output_func)(tuple* this, output_state* out);
typedef void (*read_col_func)(const char **f);



/* Globals */

int srid=-1;

static int ferror_occured;
static allocator local_malloc;
static report_error error_func;

struct tag_tuple{
	output_func   of;
	union union_tag {
		double points[4];
		int4   pointsi[4];

		struct struct_tag {
			tuple* 	stack_next;
			int	type;
			int	num;
			int	size_here;
		} nn;
	} uu;
	struct tag_tuple *next;
};

struct {
	int	type;
	int	flags;
	int	srid;
	int	ndims;
	int	hasZ;
	int	hasM;
	/* create integer version */
	int lwgi;
	/* input is integer (wkb only)*/
	int from_lwgi;

	int4 alloc_size;

	/*
		linked list of all tuples
	*/
	tuple*	first;
	tuple*  last;

	/*
		stack of open geometries
	*/
	tuple*  stack;

} the_geom;

tuple* free_list=0;
int minpoints;
int checkclosed;

/*
 * This inicates if the number of points in the geometry is required to
 * be odd (one) or even (zero, currently not enforced) or whatever (-one)
 */
int isodd;
double *first_point=NULL;
double *last_point=NULL;

/* External functions */
extern void init_parser(const char *);

/* Prototypes */
tuple* alloc_tuple(output_func of,size_t size);
static void error(const char* err);
void free_tuple(tuple* to_free);
void inc_num(void);
void alloc_stack_tuple(int type,output_func of,size_t size);
void check_dims(int num);
void WRITE_DOUBLES(output_state* out,double* points, int cnt);
#ifdef SHRINK_INTS
void WRITE_INT4(output_state * out,int4 val);
#endif
void write_size(tuple* this,output_state* out);
void alloc_lwgeom(int srid);
void write_point_2(tuple* this,output_state* out);
void write_point_3(tuple* this,output_state* out);
void write_point_4(tuple* this,output_state* out);
void write_point_2i(tuple* this,output_state* out);
void write_point_3i(tuple* this,output_state* out);
void write_point_4i(tuple* this,output_state* out);
void alloc_point_2d(double x,double y);
void alloc_point_3d(double x,double y,double z);
void alloc_point_4d(double x,double y,double z,double m);
void write_type(tuple* this,output_state* out);
void write_count(tuple* this,output_state* out);
void write_type_count(tuple* this,output_state* out);
void alloc_point(void);
void alloc_linestring(void);
void alloc_linestring_closed(void);
void alloc_circularstring(void);
void alloc_circularstring_closed(void);
void alloc_polygon(void);
void alloc_multipoint(void);
void alloc_multilinestring(void);
void alloc_multicurve(void);
void alloc_multipolygon(void);
void alloc_multisurface(void);
void alloc_geomertycollection(void);
void alloc_counter(void);
void alloc_empty(void);
uchar* make_lwgeom(void);
uchar strhex_readbyte(const char *in);
uchar read_wkb_byte(const char **in);
void read_wkb_bytes(const char **in, uchar* out, int cnt);
int4 read_wkb_int(const char **in);
double read_wkb_double(const char **in, int convert_from_int);
void read_wkb_point(const char **b);
void read_collection(const char **b, read_col_func f);
void read_collection2(const char **b);
void parse_wkb(const char **b);
void alloc_wkb(const char *parser);
uchar* parse_it(const char* geometry, allocator allocfunc, report_error errfunc);
uchar* parse_lwg(const char* geometry, allocator allocfunc, report_error errfunc);
uchar* parse_lwgi(const char* geometry, allocator allocfunc, report_error errfunc);

void
set_srid(double d_srid)
{
	if ( d_srid >= 0 )
		d_srid+=0.1;
	else
		d_srid-=0.1;


	srid=(int)(d_srid+0.1);
}

tuple *
alloc_tuple(output_func of,size_t size)
{
	tuple* ret = free_list;

	if ( ! ret ){
		int toalloc = (ALLOC_CHUNKS /sizeof(tuple));
		ret = malloc( toalloc *sizeof(tuple) );

		free_list = ret;

		while(--toalloc){
			ret->next = ret+1;
			ret++;
		}

		ret->next = NULL;

		return alloc_tuple(of,size);
	}

	free_list = ret->next;
	ret->of = of;
	ret->next = NULL;

	if ( the_geom.last ) {
		the_geom.last->next = ret;
		the_geom.last = ret;
	}
	else {
		the_geom.first = the_geom.last = ret;
	}

	the_geom.alloc_size += size;
	return ret;
}

static void
error(const char* err)
{
	error_func(err);
	ferror_occured=1;
}

void
free_tuple(tuple* to_free)
{

	tuple* list_end = to_free;

	if( !to_free)
		return;

	while(list_end->next){
		list_end=list_end->next;
	}

	list_end->next = free_list;
	free_list = to_free;
}

void
inc_num(void)
{
	the_geom.stack->uu.nn.num++;
}

/*
	Allocate a 'counting' tuple
*/
void
alloc_stack_tuple(int type,output_func of,size_t size)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_stack_tuple %d, %d", type, size);
#endif

	tuple*	p;
	inc_num();

	p = alloc_tuple(of,size);
	p->uu.nn.stack_next = the_geom.stack;
	p->uu.nn.type = type;
	p->uu.nn.size_here = the_geom.alloc_size;
	p->uu.nn.num = 0;
	the_geom.stack = p;
}

void
pop(void)
{
	the_geom.stack = the_geom.stack->uu.nn.stack_next;
}

void
popc(void)
{
	if ( the_geom.stack->uu.nn.num < minpoints){
		error("geometry requires more points");
	}
        if(isodd != -1 && the_geom.stack->uu.nn.num % 2 != isodd) {
                error("geometry must have an odd number of points");
        }
	if ( checkclosed && first_point && last_point) {
		if ( memcmp(first_point, last_point,
			sizeof(double)*the_geom.ndims) )
		{
			error("geometry contains non-closed rings");
		}
	}
	the_geom.stack = the_geom.stack->uu.nn.stack_next;
}


void
check_dims(int num)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("check_dims the_geom.ndims = %d, num = %d", the_geom.ndims, num);
#endif

	if( the_geom.ndims != num){
		if (the_geom.ndims) {
			error("Can not mix dimensionality in a geometry");
		} else {

#ifdef PGIS_DEBUG
                        lwnotice("check_dims: setting dim %d", num);
#endif

			the_geom.ndims = num;
			if ( num > 2 ) the_geom.hasZ = 1;
			if ( num > 3 ) the_geom.hasM = 1;
		}
	}
}

#define WRITE_INT4_REAL(x,y) { memcpy(x->pos,&y,4); x->pos+=4;}
#define WRITE_INT4_REAL_MULTIPLE(x,y,z) { memcpy(x->pos,&y,z*4); x->pos+=(z*4);}

/*
	we can shrink ints to one byte if they are less than 127
	by setting the extra bit.  Because if the different byte
	ordering possibilities we need to set the lsb on little
	endian to show a packed one and the msb on a big endian
	machine
*/
#ifdef SHRINK_INTS
void
WRITE_INT4(output_state * out,int4 val)
{
	if ( val <= 0x7f ){
		if ( getMachineEndian() == NDR ){
			val = (val<<1) | 1;
		}
		else{
			val |=0x80;
		}

		*out->pos++ = (uchar)val;
		the_geom.alloc_size-=3;
	}
	else{
		if ( getMachineEndian() == NDR ){
			val <<=1;
		}
		WRITE_INT4_REAL(out,val);
	}
}
#else
#define WRITE_INT4 WRITE_INT4_REAL
#endif


void
WRITE_DOUBLES(output_state* out,double* points, int cnt)
{
	if ( the_geom.lwgi){
		int4 vals[4];
		int i;

		for(i=0; i<cnt;i++){
			vals[i] = (int4)(((points[i]+180.0)*0xB60B60)+.5);
		}
		memcpy(out->pos,vals,sizeof(int4)*cnt);
		out->pos+=sizeof(int4)*cnt;
	}
	else{
		memcpy(out->pos,points,sizeof(double)*cnt);
		out->pos+=sizeof(double)*cnt;
	}

}

void
write_size(tuple* this,output_state* out)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("write_size");
#endif

	WRITE_INT4_REAL(out,the_geom.alloc_size);
}

void
alloc_lwgeom(int srid)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_lwgeom %d", srid);
#endif

	the_geom.srid=srid;
	the_geom.alloc_size=0;
	the_geom.stack=NULL;
	the_geom.ndims=0;
	the_geom.hasZ=0;
	the_geom.hasM=0;

	/* Free if used already */
	if ( the_geom.first ){
		free_tuple(the_geom.first);
		the_geom.first=the_geom.last=NULL;
	}

	if ( srid != -1 ){
		the_geom.alloc_size+=4;
	}

	the_geom.stack = alloc_tuple(write_size,4);
}

void
write_point_2(tuple* this,output_state* out)
{
	WRITE_DOUBLES(out,this->uu.points,2);
}

void
write_point_3(tuple* this,output_state* out)
{
	WRITE_DOUBLES(out,this->uu.points,3);
}

void
write_point_4(tuple* this,output_state* out)
{
	WRITE_DOUBLES(out,this->uu.points,4);
}

void
write_point_2i(tuple* this,output_state* out)
{
	WRITE_INT4_REAL_MULTIPLE(out,this->uu.points,2);
}

void
write_point_3i(tuple* this,output_state* out)
{
	WRITE_INT4_REAL_MULTIPLE(out,this->uu.points,3);
}

void
write_point_4i(tuple* this,output_state* out)
{
	WRITE_INT4_REAL_MULTIPLE(out,this->uu.points,4);
}

void
alloc_point_2d(double x,double y)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_point_2d %f,%f", x, y);
#endif

	tuple* p = alloc_tuple(write_point_2,the_geom.lwgi?8:16);
	p->uu.points[0] = x;
	p->uu.points[1] = y;

	/* keep track of point */
	if ( checkclosed ) {
		if ( ! the_geom.stack->uu.nn.num )
			first_point = p->uu.points;
		last_point = p->uu.points;
	}

	inc_num();
	check_dims(2);
}

void
alloc_point_3d(double x,double y,double z)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_point_3d %f, %f, %f", x, y, z);
#endif

	tuple* p = alloc_tuple(write_point_3,the_geom.lwgi?12:24);
	p->uu.points[0] = x;
	p->uu.points[1] = y;
	p->uu.points[2] = z;

	/* keep track of point */
	if ( checkclosed ) {
		if ( ! the_geom.stack->uu.nn.num )
			first_point = p->uu.points;
		last_point = p->uu.points;
	}

	inc_num();
	check_dims(3);
}

void
alloc_point_4d(double x,double y,double z,double m)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_point_4d %f, %f, %f, %f", x, y, z, m);
#endif

	tuple* p = alloc_tuple(write_point_4,the_geom.lwgi?16:32);
	p->uu.points[0] = x;
	p->uu.points[1] = y;
	p->uu.points[2] = z;
	p->uu.points[3] = m;

	/* keep track of point */
	if ( checkclosed ) {
		if ( ! the_geom.stack->uu.nn.num )
			first_point = p->uu.points;
		last_point = p->uu.points;
	}

	inc_num();
	check_dims(4);
}

void
write_type(tuple* this,output_state* out)
{
	uchar type=0;

	/* Empty handler - switch back */
	if ( this->uu.nn.type == 0xff )
		this->uu.nn.type = COLLECTIONTYPE;

	type |= this->uu.nn.type;

	if (the_geom.ndims) /* Support empty */
	{
		TYPE_SETZM(type, the_geom.hasZ, the_geom.hasM);
	}

	if ( the_geom.srid != -1 ){
		type |= 0x40;
	}

	*(out->pos)=type;
	out->pos++;

	if ( the_geom.srid != -1 ){
		/* Only the first geometry will have a srid attached */
		WRITE_INT4(out,the_geom.srid);
		the_geom.srid = -1;
	}
}

void
write_count(tuple* this,output_state* out)
{
	int num = this->uu.nn.num;
	WRITE_INT4(out,num);
}

void
write_type_count(tuple* this,output_state* out)
{
	write_type(this,out);
	write_count(this,out);
}

void
alloc_point(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_point");
#endif

	if( the_geom.lwgi)
		alloc_stack_tuple(POINTTYPEI,write_type,1);
	else
		alloc_stack_tuple(POINTTYPE,write_type,1);

	minpoints=1;
	checkclosed=0;
        isodd=-1;
}

void
alloc_linestring(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_linestring");
#endif

	if( the_geom.lwgi)
		alloc_stack_tuple(LINETYPEI,write_type,1);
	else
		alloc_stack_tuple(LINETYPE,write_type,1);

	minpoints=2;
	checkclosed=0;
        isodd=-1;
}

void alloc_linestring_closed(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_linestring_closed called.");
#endif

        alloc_linestring();
        checkclosed=1;
}

void
alloc_circularstring(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_circularstring");
#endif

        alloc_stack_tuple(CURVETYPE,write_type,1);
        minpoints=3;
        checkclosed=0;
        isodd=1;
}

void alloc_circularstring_closed(void)
{
      
#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_circularstring_closed");
#endif

        alloc_circularstring();
        checkclosed=1;
}

void
alloc_polygon(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_polygon");
#endif

	if( the_geom.lwgi)
		alloc_stack_tuple(POLYGONTYPEI, write_type,1);
	else
		alloc_stack_tuple(POLYGONTYPE, write_type,1);

	minpoints=3;
	checkclosed=1;
        isodd=-1;

}

void
alloc_curvepolygon(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_curvepolygon called.");
#endif

        alloc_stack_tuple(CURVEPOLYTYPE, write_type, 1);
        minpoints=3;
        checkclosed=1;
        isodd=-1;
}

void
alloc_compoundcurve(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_compoundcurve called.");
#endif

        alloc_stack_tuple(COMPOUNDTYPE, write_type, 1);
}

void
alloc_multipoint(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_multipoint");
#endif 

	alloc_stack_tuple(MULTIPOINTTYPE,write_type,1);
}

void
alloc_multilinestring(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_multilinestring");
#endif

	alloc_stack_tuple(MULTILINETYPE,write_type,1);
}

void
alloc_multicurve(void)
{
       
#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_multicurve");
#endif

        alloc_stack_tuple(MULTICURVETYPE,write_type,1);
}

void
alloc_multipolygon(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_multipolygon");
#endif

	alloc_stack_tuple(MULTIPOLYGONTYPE,write_type,1);
}

void
alloc_multisurface(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_multisurface called");
#endif

        alloc_stack_tuple(MULTISURFACETYPE,write_type,1);
}

void
alloc_geomertycollection(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_geometrycollection");
#endif

	alloc_stack_tuple(COLLECTIONTYPE,write_type,1);
}

void
alloc_counter(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_counter");
#endif

	alloc_stack_tuple(0,write_count,4);
}

void
alloc_empty(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_empty");
#endif

	tuple* st = the_geom.stack;
	/* Find the last geometry */
	while(st->uu.nn.type == 0){
		st =st->uu.nn.stack_next;
	}

	/* Reclaim memory */
	free_tuple(st->next);

	/* Put an empty geometry collection on the top of the stack */
	st->next=NULL;
	the_geom.stack=st;
	the_geom.alloc_size=st->uu.nn.size_here;

	/* Mark as a empty stop */
	if (st->uu.nn.type != 0xFF){
		st->uu.nn.type=0xFF;
		st->of = write_type_count;
		the_geom.alloc_size+=4;
		st->uu.nn.size_here=the_geom.alloc_size;
	}

	st->uu.nn.num=0;
}

uchar *
make_lwgeom(void)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("make_lwgeom");
#endif

	uchar* out_c;
	output_state out;
	tuple* cur;
	out_c = (uchar*)local_malloc(the_geom.alloc_size);
	out.pos = out_c;
	cur = the_geom.first ;

	while(cur){
		cur->of(cur,&out);
		cur=cur->next;
	}

	/* if ints shrink then we need to rewrite the size smaller */
	out.pos = out_c;
	write_size(NULL,&out);

	return out_c;
}

void
lwg_parse_yynotice(char* s)
{
        lwnotice(s);
}

int
lwg_parse_yyerror(char* s)
{
	error("parse error - invalid geometry");
	/* error_func("parse error - invalid geometry"); */
	return 1;
}

/*
 Table below generated using this ruby.

 a=(0..0xff).to_a.collect{|x|0xff};('0'..'9').each{|x|a[x[0]]=x[0]-'0'[0]}
 ('a'..'f').each{|x|v=x[0]-'a'[0]+10;a[x[0]]=a[x.upcase[0]]=v}
 puts '{'+a.join(",")+'}'

 */
static const uchar to_hex[]  = {
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	0,1,2,3,4,5,6,7,8,9,255,255,255,255,255,255,255,10,11,12,13,14,
	15,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,10,11,12,13,14,15,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255};

uchar
strhex_readbyte(const char* in)
{
	if ( *in == 0 ){
		if ( ! ferror_occured){
			error("invalid wkb");
		}
		return 0;
	}
	return to_hex[(int)*in]<<4 | to_hex[(int)*(in+1)];
}

uchar
read_wkb_byte(const char** in)
{
	uchar ret = strhex_readbyte(*in);
	(*in)+=2;
	return ret;
}

int swap_order;

void
read_wkb_bytes(const char** in, uchar* out, int cnt)
{
	if ( ! swap_order ){
		while(cnt--) *out++ = read_wkb_byte(in);
	}
	else{
		out += (cnt-1);
		while(cnt--) *out-- = read_wkb_byte(in);
	}
}

int4
read_wkb_int(const char** in)
{
	int4 ret=0;
	read_wkb_bytes(in,(uchar*)&ret,4);
	return ret;
}

double
read_wkb_double(const char** in, int convert_from_int)
{
	double ret=0;

	if ( ! convert_from_int){
		read_wkb_bytes(in,(uchar*)&ret,8);
		return ret;
	}else{
		ret  = read_wkb_int(in);
		ret /= 0xb60b60;
		return ret-180.0;
 	}
}

void
read_wkb_point(const char **b)
{
	int i;
	tuple* p = NULL;


	if(the_geom.lwgi && the_geom.from_lwgi ){
		/*
		 * Special case - reading from lwgi to lwgi
		 * we don't want to go via doubles in the middle.
		 */
		switch(the_geom.ndims){
			case 2: p=alloc_tuple(write_point_2i,8); break;
			case 3: p=alloc_tuple(write_point_3i,12); break;
			case 4: p=alloc_tuple(write_point_4i,16); break;
		}

		for(i=0;i<the_geom.ndims;i++){
			p->uu.pointsi[i]=read_wkb_int(b);
		}
	}
	else{
		int mul = the_geom.lwgi ? 1 : 2;

		switch(the_geom.ndims){
			case 2: p=alloc_tuple(write_point_2,8*mul); break;
			case 3: p=alloc_tuple(write_point_3,12*mul); break;
			case 4: p=alloc_tuple(write_point_4,16*mul); break;
		}

		for(i=0;i<the_geom.ndims;i++){
			p->uu.points[i]=read_wkb_double(b,the_geom.from_lwgi);
		}
	}

	inc_num();
	check_dims(the_geom.ndims);
}

void
read_collection(const char **b, read_col_func f)
{
	int4 cnt=read_wkb_int(b);
	alloc_counter();

	while(cnt--){
		if ( ferror_occured )	return;
		f(b);
	}

	pop();
}

void
read_collection2(const char **b)
{
	read_collection(b, read_wkb_point);
}

void
parse_wkb(const char **b)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("parse_wkb");
#endif

	int4 type;
	uchar xdr = read_wkb_byte(b);
	int4 localsrid;

	swap_order=0;

	if ( xdr != getMachineEndian() )
	{
		swap_order=1;
	}

	type = read_wkb_int(b);

	/* quick exit on error */
	if ( ferror_occured ) return;

	the_geom.ndims=2;
	if (type & WKBZOFFSET)
	{
		the_geom.hasZ = 1;
		the_geom.ndims++;
	}
	else the_geom.hasZ = 0;
	if (type & WKBMOFFSET)
	{
		the_geom.hasM = 1;
		the_geom.ndims++;
	}
	else the_geom.hasM = 0;

	if (type & WKBSRIDFLAG )
	{
		/* local (in-EWKB) srid spec overrides SRID=#; */
		localsrid = read_wkb_int(b);
		if ( localsrid != -1 )
		{
			if ( the_geom.srid == -1 ) the_geom.alloc_size += 4;
			the_geom.srid = localsrid;
		}
	}

	type &=0x0f;

	if ( the_geom.lwgi  ){

		if ( type<= POLYGONTYPE )
			alloc_stack_tuple(type +9,write_type,1);
		else
			alloc_stack_tuple(type,write_type,1);
	}
	else{
		/* If we are writing lwg and are reading wbki */
		int4 towrite=type;
		if (towrite >= POINTTYPEI && towrite <= POLYGONTYPEI){
			towrite-=9;
		}
		alloc_stack_tuple(towrite,write_type,1);
	}

	switch(type ){
		case	POINTTYPE:
			read_wkb_point(b);
			break;

		case	LINETYPE:
			read_collection(b,read_wkb_point);
			break;

                case    CURVETYPE:
                        read_collection(b,read_wkb_point);
                        break;

		case	POLYGONTYPE:
			read_collection(b,read_collection2);
			break;

                case    COMPOUNDTYPE:
                        read_collection(b,parse_wkb);
                        break;

                case    CURVEPOLYTYPE:
                        read_collection(b,parse_wkb);
                        break;

		case	MULTIPOINTTYPE:
		case	MULTILINETYPE:
                case    MULTICURVETYPE:
		case	MULTIPOLYGONTYPE:
                case    MULTISURFACETYPE:
		case	COLLECTIONTYPE:
			read_collection(b,parse_wkb);
			break;

		case	POINTTYPEI:
			the_geom.from_lwgi=1;
			read_wkb_point(b);
			break;

		case	LINETYPEI:
			the_geom.from_lwgi=1;
			read_collection(b,read_wkb_point);
			break;

		case	POLYGONTYPEI:
			the_geom.from_lwgi=1;
			read_collection(b,read_collection2);
			break;

		default:
			error("Invalid type in wbk");
	}

	the_geom.from_lwgi=0;

	pop();
}


void
alloc_wkb(const char *parser)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("alloc_wkb");
#endif

	parse_wkb(&parser);
}

/*
	Parse a string and return a LW_GEOM
*/
uchar *
parse_it(const char *geometry, allocator allocfunc, report_error errfunc)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("parse_it: %s", geometry);
#endif

	local_malloc = allocfunc;
	error_func=errfunc;

	ferror_occured = 0;

	init_parser(geometry);

	lwg_parse_yyparse();

	close_parser();

	if (ferror_occured)
		return NULL;

	return make_lwgeom();
}

uchar *
parse_lwg(const char* geometry,allocator allocfunc,report_error errfunc)
{
	the_geom.lwgi=0;
	return parse_it(geometry,allocfunc,errfunc);
}

uchar *
parse_lwgi(const char* geometry,allocator allocfunc,report_error errfunc)
{
	the_geom.lwgi=1;
	return parse_it(geometry,allocfunc,errfunc);
}

void
set_zm(char z, char m)
{

#ifdef PGIS_DEBUG_CALLS
        lwnotice("set_zm %d, %d", z, m);
#endif

	the_geom.hasZ = z;
	the_geom.hasM = m;
	the_geom.ndims = 2+z+m;
}

