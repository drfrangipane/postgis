/***********************************************************
 *
 * $Id: lwgeom_btree.c 2273 2006-01-09 15:55:55Z strk $
 *
 * Comparision function for use in Binary Tree searches
 * (ORDER BY, GROUP BY, DISTINCT)
 *
 ***********************************************************/

#include <math.h>
#include <float.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "postgres.h"
#include "fmgr.h"
#include "utils/geo_decls.h"

#include "liblwgeom.h"
#include "lwgeom_pg.h"

Datum lwgeom_lt(PG_FUNCTION_ARGS);
Datum lwgeom_le(PG_FUNCTION_ARGS);
Datum lwgeom_eq(PG_FUNCTION_ARGS);
Datum lwgeom_ge(PG_FUNCTION_ARGS);
Datum lwgeom_gt(PG_FUNCTION_ARGS);
Datum lwgeom_cmp(PG_FUNCTION_ARGS);

/* #define PGIS_DEBUG */

#if USE_VERSION == 72
#define BTREE_SRID_MISMATCH_SEVERITY NOTICE
#else
#if USE_VERSION < 80
#define BTREE_SRID_MISMATCH_SEVERITY WARNING
#else
#define BTREE_SRID_MISMATCH_SEVERITY ERROR
#endif
#endif

PG_FUNCTION_INFO_V1(lwgeom_lt);
Datum lwgeom_lt(PG_FUNCTION_ARGS)
{
	PG_LWGEOM *geom1 = (PG_LWGEOM *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	PG_LWGEOM *geom2 = (PG_LWGEOM *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
	BOX2DFLOAT4 box1;
	BOX2DFLOAT4 box2;

#ifdef PGIS_DEBUG
	elog(NOTICE, "lwgeom_lt called");
#endif

	if (pglwgeom_getSRID(geom1) != pglwgeom_getSRID(geom2))
	{
		elog(BTREE_SRID_MISMATCH_SEVERITY,
			"Operation on two GEOMETRIES with different SRIDs\n");
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 )
			pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 )
			pfree(geom2);
		PG_RETURN_NULL();
	}

#ifdef PGIS_DEBUG
	elog(NOTICE, "lwgeom_lt passed getSRID test");
#endif

	getbox2d_p(SERIALIZED_FORM(geom1), &box1);
	getbox2d_p(SERIALIZED_FORM(geom2), &box2);

#ifdef PGIS_DEBUG
	elog(NOTICE, "lwgeom_lt getbox2d_p passed");
#endif

	if  ( ! FPeq(box1.xmin , box2.xmin) ) {
		if  (box1.xmin < box2.xmin)
			PG_RETURN_BOOL(TRUE);
	}

	if  ( ! FPeq(box1.ymin , box2.ymin) ) {
		if  (box1.ymin < box2.ymin)
			PG_RETURN_BOOL(TRUE);
	}

	if  ( ! FPeq(box1.xmax , box2.xmax) ) {
		if  (box1.xmax < box2.xmax)
			PG_RETURN_BOOL(TRUE);
	}

	if  ( ! FPeq(box1.ymax , box2.ymax) ) {
		if  (box1.ymax < box2.ymax)
			PG_RETURN_BOOL(TRUE);
	}

	PG_RETURN_BOOL(FALSE);
}

PG_FUNCTION_INFO_V1(lwgeom_le);
Datum lwgeom_le(PG_FUNCTION_ARGS)
{
	PG_LWGEOM *geom1 = (PG_LWGEOM *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	PG_LWGEOM *geom2 = (PG_LWGEOM *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
	BOX2DFLOAT4 box1;
	BOX2DFLOAT4 box2;

#ifdef PGIS_DEBUG
	elog(NOTICE, "lwgeom_le called");
#endif

	if (pglwgeom_getSRID(geom1) != pglwgeom_getSRID(geom2))
	{
		elog(BTREE_SRID_MISMATCH_SEVERITY,
			"Operation on two GEOMETRIES with different SRIDs\n");
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 )
			pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 )
			pfree(geom2);
		PG_RETURN_NULL();
	}

	getbox2d_p(SERIALIZED_FORM(geom1), &box1);
	getbox2d_p(SERIALIZED_FORM(geom2), &box2);

	if  ( ! FPeq(box1.xmin , box2.xmin) ) {
		if  (box1.xmin < box2.xmin)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_BOOL(TRUE);
		}
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
		PG_RETURN_BOOL(FALSE);
	}

	if  ( ! FPeq(box1.ymin , box2.ymin) ) {
		if  (box1.ymin < box2.ymin)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_BOOL(TRUE);
		}
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
		PG_RETURN_BOOL(FALSE);
	}

	if  ( ! FPeq(box1.xmax , box2.xmax) ) {
		if  (box1.xmax < box2.xmax)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_BOOL(TRUE);
		}
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
		PG_RETURN_BOOL(FALSE);
	}

	if  ( ! FPeq(box1.ymax , box2.ymax) ) {
		if  (box1.ymax < box2.ymax)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_BOOL(TRUE);
		}
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
		PG_RETURN_BOOL(FALSE);
	}

	if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
	if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);

	PG_RETURN_BOOL(TRUE);
}

PG_FUNCTION_INFO_V1(lwgeom_eq);
Datum lwgeom_eq(PG_FUNCTION_ARGS)
{
	PG_LWGEOM *geom1 = (PG_LWGEOM *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	PG_LWGEOM *geom2 = (PG_LWGEOM *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
	BOX2DFLOAT4 box1;
	BOX2DFLOAT4 box2;

#ifdef PGIS_DEBUG
	elog(NOTICE, "lwgeom_eq called");
#endif

	if (pglwgeom_getSRID(geom1) != pglwgeom_getSRID(geom2))
	{
		elog(BTREE_SRID_MISMATCH_SEVERITY,
			"Operation on two GEOMETRIES with different SRIDs\n");
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 )
			pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 )
			pfree(geom2);
		PG_RETURN_NULL();
	}

	getbox2d_p(SERIALIZED_FORM(geom1), &box1);
	getbox2d_p(SERIALIZED_FORM(geom2), &box2);

	if  ( ! FPeq(box1.xmin , box2.xmin) ) 
	{
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
		PG_RETURN_BOOL(FALSE);
	}

	if  ( ! FPeq(box1.ymin , box2.ymin) ) 
	{
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
		PG_RETURN_BOOL(FALSE);
	}

	if  ( ! FPeq(box1.xmax , box2.xmax) ) 
	{
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
		PG_RETURN_BOOL(FALSE);
	}

	if  ( ! FPeq(box1.ymax , box2.ymax) )
	{
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
		PG_RETURN_BOOL(FALSE);
	}

	if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
	if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);

	PG_RETURN_BOOL(TRUE);
}

PG_FUNCTION_INFO_V1(lwgeom_ge);
Datum lwgeom_ge(PG_FUNCTION_ARGS)
{
	PG_LWGEOM *geom1 = (PG_LWGEOM *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	PG_LWGEOM *geom2 = (PG_LWGEOM *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
	BOX2DFLOAT4 box1;
	BOX2DFLOAT4 box2;

#ifdef PGIS_DEBUG
	elog(NOTICE, "lwgeom_ge called");
#endif

	if (pglwgeom_getSRID(geom1) != pglwgeom_getSRID(geom2))
	{
		elog(BTREE_SRID_MISMATCH_SEVERITY,
			"Operation on two GEOMETRIES with different SRIDs\n");
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 )
			pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 )
			pfree(geom2);
		PG_RETURN_NULL();
	}

	getbox2d_p(SERIALIZED_FORM(geom1), &box1);
	getbox2d_p(SERIALIZED_FORM(geom2), &box2);

	if  ( ! FPeq(box1.xmin , box2.xmin) ) {
		if  (box1.xmin > box2.xmin)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_BOOL(TRUE);
		}
		PG_RETURN_BOOL(FALSE);
	}

	if  ( ! FPeq(box1.ymin , box2.ymin) ) {
		if  (box1.ymin > box2.ymin)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_BOOL(TRUE);
		}
		PG_RETURN_BOOL(FALSE);
	}

	if  ( ! FPeq(box1.xmax , box2.xmax) ) {
		if  (box1.xmax > box2.xmax)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_BOOL(TRUE);
		}
		PG_RETURN_BOOL(FALSE);
	}

	if  ( ! FPeq(box1.ymax , box2.ymax) ) {
		if  (box1.ymax > box2.ymax)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_BOOL(TRUE);
		}
		PG_RETURN_BOOL(FALSE);
	}

	if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
	if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);

	PG_RETURN_BOOL(TRUE);
}

PG_FUNCTION_INFO_V1(lwgeom_gt);
Datum lwgeom_gt(PG_FUNCTION_ARGS)
{
	PG_LWGEOM *geom1 = (PG_LWGEOM *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	PG_LWGEOM *geom2 = (PG_LWGEOM *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
	BOX2DFLOAT4 box1;
	BOX2DFLOAT4 box2;

#ifdef PGIS_DEBUG
	elog(NOTICE, "lwgeom_gt called");
#endif

	if (pglwgeom_getSRID(geom1) != pglwgeom_getSRID(geom2))
	{
		elog(BTREE_SRID_MISMATCH_SEVERITY,
			"Operation on two GEOMETRIES with different SRIDs\n");
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 )
			pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 )
			pfree(geom2);
		PG_RETURN_NULL();
	}

	getbox2d_p(SERIALIZED_FORM(geom1), &box1);
	getbox2d_p(SERIALIZED_FORM(geom2), &box2);

	if  ( ! FPeq(box1.xmin , box2.xmin) ) {
		if  (box1.xmin > box2.xmin)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_BOOL(TRUE);
		}
	}

	if  ( ! FPeq(box1.ymin , box2.ymin) ) {
		if  (box1.ymin > box2.ymin)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_BOOL(TRUE);
		}
	}

	if  ( ! FPeq(box1.xmax , box2.xmax) ) {
		if  (box1.xmax > box2.xmax)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_BOOL(TRUE);
		}
	}

	if  ( ! FPeq(box1.ymax , box2.ymax) ) {
		if  (box1.ymax > box2.ymax)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_BOOL(TRUE);
		}
	}

	if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
	if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);

	PG_RETURN_BOOL(FALSE);
}

PG_FUNCTION_INFO_V1(lwgeom_cmp);
Datum lwgeom_cmp(PG_FUNCTION_ARGS)
{
	PG_LWGEOM *geom1 = (PG_LWGEOM *) PG_DETOAST_DATUM(PG_GETARG_DATUM(0));
	PG_LWGEOM *geom2 = (PG_LWGEOM *) PG_DETOAST_DATUM(PG_GETARG_DATUM(1));
	BOX2DFLOAT4 box1;
	BOX2DFLOAT4 box2;

#ifdef PGIS_DEBUG
	elog(NOTICE, "lwgeom_cmp called");
#endif

	if (pglwgeom_getSRID(geom1) != pglwgeom_getSRID(geom2))
	{
		elog(BTREE_SRID_MISMATCH_SEVERITY,
			"Operation on two GEOMETRIES with different SRIDs\n");
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 )
			pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 )
			pfree(geom2);
		PG_RETURN_NULL();
	}

	getbox2d_p(SERIALIZED_FORM(geom1), &box1);
	getbox2d_p(SERIALIZED_FORM(geom2), &box2);

	if  ( ! FPeq(box1.xmin , box2.xmin) ) {
		if  (box1.xmin < box2.xmin)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_INT32(-1);
		}
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
		PG_RETURN_INT32(1);
	}

	if  ( ! FPeq(box1.ymin , box2.ymin) ) {
		if  (box1.ymin < box2.ymin)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_INT32(-1);
		}
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
		PG_RETURN_INT32(1);
	}

	if  ( ! FPeq(box1.xmax , box2.xmax) ) {
		if  (box1.xmax < box2.xmax)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_INT32(-1);
		}
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
		PG_RETURN_INT32(1);
	}

	if  ( ! FPeq(box1.ymax , box2.ymax) ) {
		if  (box1.ymax < box2.ymax)
		{
			if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
			if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
			PG_RETURN_INT32(-1);
		}
		if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
		if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);
		PG_RETURN_INT32(1);
	}

	if ( (Pointer *)PG_GETARG_DATUM(0) != (Pointer *)geom1 ) pfree(geom1);
	if ( (Pointer *)PG_GETARG_DATUM(1) != (Pointer *)geom2 ) pfree(geom2);

	PG_RETURN_INT32(0);
}

/***********************************************************
 *
 * $Log$
 * Revision 1.9  2006/01/09 15:55:55  strk
 * ISO C90 comments (finished in lwgeom/)
 *
 * Revision 1.8  2005/06/15 16:04:11  strk
 * fault tolerant btree ops
 *
 * Revision 1.7  2005/02/07 13:21:10  strk
 * Replaced DEBUG* macros with PGIS_DEBUG*, to avoid clashes with postgresql DEBUG
 *
 * Revision 1.6  2005/01/05 12:44:47  strk
 * Added is_worth_caching_serialized_bbox(). Renamed lwgeom_setSRID() to
 * pglwgeom_setSRID(). Fixed a bug in PG_LWGEOM_construct support for
 * AUTOCACHE_BBOX.
 *
 * Revision 1.5  2004/09/29 10:50:30  strk
 * Big layout change.
 * lwgeom.h is public API
 * liblwgeom.h is private header
 * lwgeom_pg.h is for PG-links
 * lw<type>.c contains type-specific functions
 *
 * Revision 1.4  2004/09/29 06:31:42  strk
 * Changed LWGEOM to PG_LWGEOM.
 * Changed LWGEOM_construct to PG_LWGEOM_construct.
 *
 * Revision 1.3  2004/08/20 14:08:41  strk
 * Added Geom{etry,}FromWkb(<geometry>,[<int4>]) funx.
 * Added LWGEOM typedef and SERIALIZED_FORM(LWGEOM) macro.
 * Made lwgeom_setSRID an API function.
 * Added LWGEOM_setAllocator().
 *
 * Revision 1.2  2004/08/19 13:10:13  strk
 * fixed typos
 *
 * (ORDER BY, GROUP BY, DISTINCT)
 *
 ***********************************************************/
