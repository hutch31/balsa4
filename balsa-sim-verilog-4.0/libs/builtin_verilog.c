/*
  	The Balsa Asynchronous Hardware Synthesis System
  	Copyright (C) 1995-2003 Department of Computer Science
  	The University of Manchester, Oxford Road, Manchester, UK, M13 9PL
  	
  	This program is free software; you can redistribute it and/or modify
  	it under the terms of the GNU General Public License as published by
  	the Free Software Foundation; either version 2 of the License, or
  	(at your option) any later version.
  	
  	This program is distributed in the hope that it will be useful,
  	but WITHOUT ANY WARRANTY; without even the implied warranty of
  	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  	GNU General Public License for more details.
  	
  	You should have received a copy of the GNU General Public License
  	along with this program; if not, write to the Free Software
  	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
  
  	`builtin.c'
  	Verilog PLI wrapper infrastructure to allow plugin functions
  
  	$Id: builtin_verilog.c,v 1.25 2004/04/18 14:15:18 janinl Exp $
*/

#if ! defined(VPI) && ! defined(ACC)
#error
#endif

#include "config.h"

#include <balsasim/builtin.h>
#include <balsasim/bstring.h>
#include <balsasim/object.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#undef bool

#include "veriuser.h"
#include "acc_user.h"

/*
    Always use PLI-1 with the Cver simulator because 
    vpi-{get|put}userdata are not implemented (yet).
*/
#ifdef CVER
#undef VPI
#define ACC
#endif

/*
    This is specific to Cadence simulators, so only include it if
    compiling for one of those. It is only needed for the PLI-1
    registration function, so if registering by VPI then it is not
    needed.
*/
#if defined(NCV) || ( defined(VXL) && defined(ACC) )
#include "vxl_veriuser.h"
#elif ( defined(CVER) && defined(ACC) )
#include "cv_veriuser.h"
#endif

#if defined(VPI)
#include "vpi_user.h"
#endif

/*
    Most of the access routines have the same structure whether using
    acc_* or vpi_* calls. A small number (2) of vpi functions were
    needed to enable this. These macros are used to reduce all the
    other functions to a single implementation.
*/
#if defined(VPI)

#define TFARG_HANDLE vpiHandle
#define GET_TFARG_HANDLE_FROM_IDX vpi_handle_tfarg
#define GET_TFINST_HANDLE vpi_handle(vpiSysTfCall, NULL)
#define GET_TFARG_SIZE_FROM_HANDLE(h) vpi_get(vpiSize,(h))
#define GET_TYPESTRING_FROM_HANDLE(h) vpi_get_str(vpiType,(h))
#define VALUE_STRUCTURE struct t_vpi_value
#define VECTOR_TYPE vpiVectorVal
#define INT_TYPE vpiIntVal
#define ALLOC_VECTOR_SPACE_GET(d,c)
#define ALLOC_VECTOR_SPACE_PUT(d,c) (d).value.vector=malloc((c)* sizeof (struct t_vpi_vecval))
#define GET_VALUE_FROM_HANDLE(h,d) vpi_get_value((h),&(d))
#define SET_VALUE_FROM_HANDLE(h,d,y) vpi_put_value ((h),&(d),NULL,(y))
#define STATIC_NODELAY static unsigned delay = vpiNoDelay
#define FREE_VECTOR_SPACE_GET(d)
#define GET_TYPE_FROM_HANDLE(h) vpi_get(vpiType,(h))
#define NET_TYPE vpiNet
#define REG_TYPE vpiReg
#define CONST_TYPE vpiConstant
#define STRING_TYPE vpiStringVal
#define GET_ARG_COUNT vpi_get_arg_count
#define SCAN_ARGS vpi_scan_plusargs

#define GET_TIME_64(fd) \
	{ \
		struct t_vpi_time s_time; \
		s_time.type = vpiSimTime; \
		vpi_get_time(NULL, &(s_time)); \
		(fd)->words[0] = s_time.low; \
		(fd)->words[1] = s_time.high; \
	}

#else

#define TFARG_HANDLE handle
#define GET_TFARG_HANDLE_FROM_IDX acc_handle_tfarg
#define GET_TFINST_HANDLE acc_handle_tfinst()
#define GET_TFARG_SIZE_FROM_HANDLE(h) acc_fetch_size(h)
#define GET_TYPESTRING_FROM_HANDLE(h) acc_fetch_type_str(acc_fetch_type(argh))
#define VALUE_STRUCTURE s_acc_value
#define VECTOR_TYPE accVectorVal
#define INT_TYPE accIntVal
#define ALLOC_VECTOR_SPACE_GET(d,c) (d).value.vector=malloc((c)* sizeof (s_acc_vecval))
#define ALLOC_VECTOR_SPACE_PUT(d,c) (d).value.vector=malloc((c)* sizeof (s_acc_vecval))
#define GET_VALUE_FROM_HANDLE(h,d) acc_fetch_value ((h),"%%",&(d))
#define SET_VALUE_FROM_HANDLE(h,d,y) acc_set_value((h), &(d), &(y))
#define STATIC_NODELAY static s_setval_delay delay = { {accNoDelay}, accNoDelay };
#define FREE_VECTOR_SPACE_GET(d) free((d).value.vector)
#define GET_TYPE_FROM_HANDLE(h) acc_fetch_type((h))
#define NET_TYPE accNet
#define REG_TYPE accReg
#define CONST_TYPE accConstant
#define STRING_TYPE accStringVal
#define GET_ARG_COUNT tf_nump
#define SCAN_ARGS mc_scan_plusargs

#define GET_TIME_64(fd) (fd)->words[0] = tf_getlongsimtime(&((fd)->words[1]))

#endif /* #if defined(VPI) */

#define FREE_VECTOR_SPACE_PUT(d) free((d).value.vector)

#if defined(VXL) || defined(NCV) || ! defined(VPI)
#define PRINT_FUNCTION io_printf
#else
#define PRINT_FUNCTION vpi_printf
#endif

/*
    This stuff really stinks. Verilog-XL has the functions
    tf_i{get,set}workarea, but they cause a segfault. NC-Verilog
    doesn't have them. Similarly for the vpi_{get,put}_userdata. VCS
    doesn't have the tf_i* variants, but the extra qualification is
    there because I have no access to a VCS with VPI support to know
    what is implemented there.
    
    The initialisation functions may or may not be needed for acc_*
    based systems.
*/
#if defined(VXL) || defined(NCV) || ( defined(VCS) && defined(ACC) ) || ( defined(CVER) && defined(ACC) )
#define GET_USER_DATA_FROM_HANDLE(h) tf_getworkarea()
#define SET_USER_DATA_FROM_HANDLE(h,d) tf_setworkarea((void *)(d))
#define TF_FUNC_INITIALIZE acc_initialize()
#elif defined(ACC)
#define GET_USER_DATA_FROM_HANDLE(h) tf_igetworkarea(h)
#define SET_USER_DATA_FROM_HANDLE(h,d) tf_isetworkarea((h),(d))
#define TF_FUNC_INITIALIZE acc_initialize()
#else
#define GET_USER_DATA_FROM_HANDLE(h) vpi_get_userdata(h)
#define SET_USER_DATA_FROM_HANDLE(h,d) vpi_put_userdata((h),(d))
#define TF_FUNC_INITIALIZE
#endif

#define DEBUG_CMDLINE_ARG_STRING "BalsaDebug"
#define COMMON_CMDLINE_ARG_STRING "BalsaSim+"

/* BalsaSimRegisterAllFunctions: defined by the wrapper generated by balsa-verilog-make-builtin-lib */
extern void BalsaSimRegisterAllFunctions (void);
static void BalsaSimInit (void);

#if defined(VPI)

/*
    This is useful for vpi_* based systems - there is no vpi equivalent
    of acc_handle_tfarg() so this does the same thing in vpi-land
*/
inline static vpiHandle vpi_handle_tfarg (int idx)
{
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);
    vpiHandle hTf, hArg, hArgItr;
    s_vpi_error_info sError;
    int i;

    hTf = vpi_handle (vpiSysTfCall, NULL);
    if (vpi_chk_error (&(sError)))
    {
        if (BalsaDebug >= 2)
            fprintf (stderr, "%s Exit\n", __FUNCTION__);
        return 0;
    }

    if (idx < 1)
    {
        if (BalsaDebug >= 2)
            fprintf (stderr, "%s Exit\n", __FUNCTION__);
        return hTf;
    }

    hArgItr = vpi_iterate (vpiArgument, hTf);

    if (vpi_chk_error (&(sError)))
    {
        if (BalsaDebug >= 2)
            fprintf (stderr, "%s Exit\n", __FUNCTION__);
        return 0;
    }

    for (i = 0; i < idx; i++)
    {
        hArg = vpi_scan (hArgItr);
        if (vpi_chk_error (&(sError)))
        {
            if (BalsaDebug >= 2)
                fprintf (stderr, "%s Exit\n", __FUNCTION__);
            return 0;
        }
    }

    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return hArg;
}

/*
    There is no trivial way to get the number of arguments in vpi-land
    - this is a functional replacement for tf_nump().
*/
inline static unsigned vpi_get_arg_count (void)
{
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);
    vpiHandle hTf, hArg, hArgItr;
    s_vpi_error_info sError;
    unsigned i = 0;

    hTf = vpi_handle (vpiSysTfCall, NULL);
    if (vpi_chk_error (&(sError)))
        return 0;

    hArgItr = vpi_iterate (vpiArgument, hTf);
    if (vpi_chk_error (&(sError)))
        return 0;

    while ((hArg = vpi_scan (hArgItr)))
    {
        if (vpi_chk_error (&(sError)))
            return 0;
        i++;
    }
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return i;
}

inline static char *vpi_scan_plusargs (char *key)
{
    return mc_scan_plusargs (key);
}

#endif /* #if defined(VPI) */

/*
    We need a bootstrap function to do the registration of the system
    task. On cadence simulators this function is explicitly named. VCS
    needs no registration function because it uses an extra
    file. Icarus and Modelsim automagically look for a variable called
    vlog_startup_routines and run the functions whose pointers it finds
    in that array - thus this references the bootstrap function.
    
    What a bloody mess.
    
    NC-Verilog uses the old (PLI-1) based registration because it seems
    that functions registered by the VPI method cannot get/set their
    userdata, but if the function is registered with the old method
    they can...
    
    Did I mention about this being a mess?
*/

/* these can't, alas, be static in all cases, as VCS finds them by name and so cannot find them if they are */
#if defined(VCS)
#define MAYBE_STATIC
#else
#define MAYBE_STATIC static
#endif
MAYBE_STATIC int BalsaBuiltinFunction_check (void);
MAYBE_STATIC int BalsaBuiltinFunction_call (void);
MAYBE_STATIC int BalsaInternType_check (void);
MAYBE_STATIC int BalsaInternType_call (void);
MAYBE_STATIC int BalsaUnref_check (void);
MAYBE_STATIC int BalsaUnref_call (void);
MAYBE_STATIC int BalsaRef_check (void);
MAYBE_STATIC int BalsaRef_call (void);

#if defined(VPI) && ! ( defined(NCV) || defined(VCS) )

void bootstrap (void)
{
    s_vpi_systf_data builtin;
    s_vpi_systf_data intern;
    s_vpi_systf_data unref;
    s_vpi_systf_data ref;

    char *debug_remainder = SCAN_ARGS (DEBUG_CMDLINE_ARG_STRING);

    if (debug_remainder)
    {
        fprintf (stderr, "## %s\n", debug_remainder);

        BalsaDebug = 1;
        if (*debug_remainder == '=')
        {
            fprintf (stderr, "## %s\n", debug_remainder);

            BalsaDebug = 1;
            if (*debug_remainder == '=')
                sscanf (debug_remainder + 1, "%d", &BalsaDebug);
            fprintf (stderr, "BalsaDebug level set to %d\n", BalsaDebug);
        }

        BalsaSimInit ();

        builtin.type = vpiSysTask;
        builtin.tfname = "$BalsaBuiltin";
        builtin.calltf = (PLI_INT32 (*)(char *)) BalsaBuiltinFunction_call;
        builtin.compiletf = (PLI_INT32 (*)(char *)) BalsaBuiltinFunction_check;
        builtin.sizetf = 0;

        vpi_register_systf (&builtin);

        sscanf (debug_remainder + 1, "%d", &BalsaDebug);
        fprintf (stderr, "BalsaDebug level set to %d\n", BalsaDebug);
    }

    BalsaSimInit ();

    builtin.type = vpiSysTask;
    builtin.tfname = "$BalsaBuiltin";
    builtin.calltf = (PLI_INT32 (*)(char *)) BalsaBuiltinFunction_call;
    builtin.compiletf = (PLI_INT32 (*)(char *)) BalsaBuiltinFunction_check;
    builtin.sizetf = 0;

    vpi_register_systf (&builtin);

    intern.type = vpiSysTask;
    intern.tfname = "$BalsaInternType";
    intern.calltf = (PLI_INT32 (*)(char *)) BalsaInternType_call;
    intern.compiletf = (PLI_INT32 (*)(char *)) BalsaInternType_check;
    intern.sizetf = 0;

    vpi_register_systf (&intern);

    unref.type = vpiSysTask;
    unref.tfname = "$BalsaUnref";
    unref.calltf = (PLI_INT32 (*)(char *)) BalsaUnref_call;
    unref.compiletf = (PLI_INT32 (*)(char *)) BalsaUnref_check;
    unref.sizetf = 0;

    vpi_register_systf (&unref);

    ref.type = vpiSysTask;
    ref.tfname = "$BalsaRef";
    ref.calltf = (PLI_INT32 (*)(char *)) BalsaRef_call;
    ref.compiletf = (PLI_INT32 (*)(char *)) BalsaRef_check;
    ref.sizetf = 0;

    vpi_register_systf (&ref);
}

/* AB 2008-08-04 void (*vlog_startup_routines[]) (void) =
{
(void (*)()) bootstrap, 0}; */

#ifdef CVER
/* dummy +loadvpi= boostrap routine - mimics old style exec all routines */
/* in standard PLI vlog_startup_routines for the Cver simulator*/
void vpi_compat_bootstrap (void)
{
    int i;

    for (i = 0;; i++)
    {
        if (vlog_startup_routines[i] == NULL)
            break;
        vlog_startup_routines[i] ();
    }
}

/* Defined for compatibility with Mac OS X */
void _vpi_compat_bootstrap (void)
{
    vpi_compat_bootstrap ();
}
#endif

#elif defined(NCV) || ( defined(VXL) && defined(ACC) ) || ( defined(CVER) && defined(ACC) )

p_tfcell bootstrap (void)
{
    char *debug_remainder = SCAN_ARGS (DEBUG_CMDLINE_ARG_STRING);

    if (debug_remainder)
    {
        fprintf (stderr, "## %s\n", debug_remainder);

        BalsaDebug = 1;
        if (*debug_remainder == '=')
            sscanf (debug_remainder + 1, "%d", &BalsaDebug);
        fprintf (stderr, "BalsaDebug level set to %d\n", BalsaDebug);
    }

    BalsaSimInit ();

    static s_tfcell retVal[] = {
        {usertask, 0, BalsaBuiltinFunction_check,
          0, BalsaBuiltinFunction_call, 0, "$BalsaBuiltin", 1},
        {usertask, 0, BalsaInternType_check,
          0, BalsaInternType_call, 0, "$BalsaInternType", 1},
        {usertask, 0, BalsaUnref_check,
          0, BalsaUnref_call, 0, "$BalsaUnref", 1},
        {usertask, 0, BalsaRef_check,
          0, BalsaRef_call, 0, "$BalsaRef", 1},
        {0}
    };
    return retVal;
}

#ifdef CVER
/* Defined for compatibility with Mac OS X */
p_tfcell _bootstrap (void)
{
    return bootstrap ();
}
#endif

#elif defined(VCS) && defined(ACC)

extern int VCS_main (int argc, char *argv[]);

int main (int argc, char *argv[])
{
    int i = 0;

    for (; i < argc; i++)
    {
        if (*argv[i] == '+' && !strncmp (argv[i] + 1, DEBUG_CMDLINE_ARG_STRING, strlen (DEBUG_CMDLINE_ARG_STRING)))
        {
            BalsaDebug = 1;
            if (*(argv[i] + strlen (DEBUG_CMDLINE_ARG_STRING) + 1) == '=')
                sscanf (argv[i] + strlen (DEBUG_CMDLINE_ARG_STRING) + 2, "+BalsaDebug=%d", &BalsaDebug);
            fprintf (stderr, "BalsaDebug level set to %d\n", BalsaDebug);
            break;
        }
    }

    BalsaSimInit ();

    VCS_main (argc, argv);      /* Run simv */
    return 0;                   /* Never gets here */
}

#else
/* nothing if neither VPI or a Cadence simulator */
#endif

/*
    Nothing below here is conditionally compiled.
*/

bool err_intercept (int level, char *facility, char *code)
{
    return (true);
}

int (*endofcompile_routines[]) (void) =
{
0};

/*
    Report the simulation time as a 64-bit value.
*/
long long BalsaSimulationTimeValue (void)
{
    long long simTime = 0;
    char time[32];              /* enough for 64 bits of unsigned */

    FormatData *fd = NewFormatData (3); /* 3 incase the top bit of [1] is set */

    GET_TIME_64 (fd);
    FormatDataFormatAsUInt (time, fd, 10, 0);
    sscanf (time, "%lld", &simTime);
    DeleteFormatData (fd);

    return simTime;
}

/*
    Report the simulation time as a stringed 64-bit value.
*/
static void Builtin_BalsaSimulationTime (BuiltinFunction * function, BuiltinFunctionInstanceData * instance)
{
    if (BalsaDebug)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);
    if (!instance->portWidthsAreResolved)
    {
    } else
    {
        char time[32];          /* enough for 64 bits of unsigned */
        FormatData *fd = NewFormatData (3); /* 3 incase the top bit of [1] is set */

        GET_TIME_64 (fd);
        FormatDataFormatAsUInt (time, fd, 10, 0);
        BalsaString *string = NewBalsaString (time, strlen (time));

        SetBalsaObject (instance->objects[0], string, (BalsaDestructor) BalsaStringUnref);
        FormatDataSetBalsaObject (instance->result, instance->objects[0], 0);
    }
    if (BalsaDebug)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
}

/*
	Terminate simulation
*/
static void Builtin_BalsaSimulationStop (BuiltinFunction * function, BuiltinFunctionInstanceData * instance)
{
	fprintf (stdout, "BalsaSimulationStop called\n");
	tf_dofinish ();
}

static void Builtin_BalsaGetCommandLineArg (BuiltinFunction * function, BuiltinFunctionInstanceData * instance)
{
    if (BalsaDebug)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);
    if (!instance->portWidthsAreResolved)
    {
    } else
    {
        char *arg_name, *arg_value;
        BalsaString *string;
        BalsaString *sourceStr = BALSA_STRING (FormatDataGetBalsaObject (instance->arguments[0], 0)->data);

        /* get enough space for "BalsaSim+<whatever>=\0" */
        arg_name = malloc (strlen (COMMON_CMDLINE_ARG_STRING) + sourceStr->length + 2);
        strcpy (arg_name, COMMON_CMDLINE_ARG_STRING);
        strncpy (arg_name + strlen (COMMON_CMDLINE_ARG_STRING), sourceStr->string, sourceStr->length);
        arg_name[strlen (COMMON_CMDLINE_ARG_STRING) + sourceStr->length] = '=';
        arg_name[strlen (COMMON_CMDLINE_ARG_STRING) + sourceStr->length + 1] = '\0';

        arg_value = SCAN_ARGS (arg_name);
        free (arg_name);
        if (arg_value)
            string = NewBalsaString (arg_value, strlen (arg_value));
        else
            string = NewBalsaString ("", 0);

        SetBalsaObject (instance->objects[0], string, (BalsaDestructor) BalsaStringUnref);
        FormatDataSetBalsaObject (instance->result, instance->objects[0], 0);
    }
    if (BalsaDebug)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
}

static char *message_facility = "BalsaBuiltin";

static void BalsaSimInit (void)
{
    if (BalsaDebug)
    {
        fprintf (stderr, "initialising ...\n");

        BalsaSim_PrintF = (int (*)(const char *format,...)) PRINT_FUNCTION;
        fprintf (stderr, "%s builtin functions infrastructure support loaded\n", message_facility);
    }

    BalsaSimRegisterAllFunctions ();
    BalsaSim_RegisterBuiltinFunction ("BalsaSimulationTime", 0, 0, Builtin_BalsaSimulationTime, BALSA_OBJECT_BITS, NULL, 1);
    BalsaSim_RegisterBuiltinFunction ("BalsaSimulationStop", 0, 0, Builtin_BalsaSimulationStop, 1, NULL, 0);
    BalsaSim_RegisterBuiltinFunction ("BalsaGetCommandLineArg", 0, 1, Builtin_BalsaGetCommandLineArg, BALSA_OBJECT_BITS, (unsigned[])
      {
      BALSA_OBJECT_BITS}, 1);

    BalsaSim_PrintF = (int (*)(const char *format,...)) PRINT_FUNCTION;
}

typedef struct WorkArea_
{
    BuiltinFunction *func;
    BuiltinFunctionInstanceData *instData;
    unsigned returnValIndex;
} WorkArea;

inline static WorkArea *GetWorkArea (void)
{
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);
    WorkArea *wa;
    TFARG_HANDLE tfh;

    tfh = GET_TFINST_HANDLE;
    wa = (WorkArea *) GET_USER_DATA_FROM_HANDLE (tfh);
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return wa;
}

inline static void SetWorkArea (WorkArea * wa)
{
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);
    TFARG_HANDLE tfh;

    tfh = GET_TFINST_HANDLE;
    SET_USER_DATA_FROM_HANDLE (tfh, wa);
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
}

inline static char *GetString (unsigned idx)
{
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);
    char *str;
	/* AB 2008-08-25.  acc_get_value shouldn't be used for strings, CVER problem. */

#ifdef VPI
    TFARG_HANDLE argh;
    VALUE_STRUCTURE data;

    data.format = STRING_TYPE;
    argh = GET_TFARG_HANDLE_FROM_IDX (idx);
    GET_VALUE_FROM_HANDLE (argh, data);
    str = data.value.str;
#endif
#ifdef ACC
    str = acc_fetch_tfarg_str (idx);
#endif
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return str;
}

inline static unsigned GetSize (unsigned idx)
{
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);
    unsigned size;
    TFARG_HANDLE argh = GET_TFARG_HANDLE_FROM_IDX (idx);

    size = GET_TFARG_SIZE_FROM_HANDLE (argh);
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return size;
}

inline static char *TypeString (unsigned idx)
{
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);
    char *typeStr;
    TFARG_HANDLE argh = GET_TFARG_HANDLE_FROM_IDX (idx);

    typeStr = GET_TYPESTRING_FROM_HANDLE (argh);
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return typeStr;
}

inline static void GetVec (FormatData * bd, unsigned idx)
{
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);
    TFARG_HANDLE argh;
    unsigned i;
    VALUE_STRUCTURE data;

    data.format = VECTOR_TYPE;

    argh = GET_TFARG_HANDLE_FROM_IDX (idx);
    if (!argh)
    {
        fprintf (stderr, "FATAL: GetVec(%u) : unable to get a handle\n", idx);
        exit (EXIT_FAILURE);
    }

    ALLOC_VECTOR_SPACE_GET (data, bd->wordCount);
    GET_VALUE_FROM_HANDLE (argh, data);
    for (i = 0; i < bd->wordCount; i++)
        bd->words[i] = data.value.vector[i].aval;
    FREE_VECTOR_SPACE_GET (data);
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
}

inline static void SetVec (FormatData * bd, unsigned idx)
{
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);
    TFARG_HANDLE argh;
    unsigned i;
    VALUE_STRUCTURE data;

    STATIC_NODELAY;
    data.format = VECTOR_TYPE;

    argh = GET_TFARG_HANDLE_FROM_IDX (idx);
    if (!argh)
    {
        fprintf (stderr, "FATAL: GetVec(%u) : unable to get a handle\n", idx);
        exit (EXIT_FAILURE);
    }

    ALLOC_VECTOR_SPACE_PUT (data, bd->wordCount);
    for (i = 0; i < bd->wordCount; i++)
    {
        data.value.vector[i].aval = bd->words[i];
        data.value.vector[i].bval = 0;
    }
    SET_VALUE_FROM_HANDLE (argh, data, delay);
    FREE_VECTOR_SPACE_PUT (data);
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
}

inline static unsigned CheckType (unsigned idx, bool allowConstant)
{
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);
    unsigned retVal;
    TFARG_HANDLE argh = GET_TFARG_HANDLE_FROM_IDX (idx);
    int type = GET_TYPE_FROM_HANDLE (argh);

    switch (type)
    {
    case NET_TYPE:
    case REG_TYPE:
        retVal = 0;
        break;
    case CONST_TYPE:
        if (allowConstant)
            retVal = 0;
        else
            retVal = 1;
        break;
    default:
        retVal = 1;
    }
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return retVal;
}

inline static int GetInt (unsigned idx)
{
    int i;

    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);
    TFARG_HANDLE argh = GET_TFARG_HANDLE_FROM_IDX (idx);
    VALUE_STRUCTURE data;

    data.format = INT_TYPE;
    GET_VALUE_FROM_HANDLE (argh, data);
    i = data.value.integer;
    if (BalsaDebug >= 2)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return i;
}

#define MIN_ARGS 4
#define ACK_STATE 1
#define NAME_IDX 2
#define PARAM_COUNT_IDX 3
#define RETURN_ARG_OFFSET 4

/* The task will be called with (acknowledge_state(0 or 1), task_name, num_params, [param ,]* result [, arg]*) */
MAYBE_STATIC int BalsaBuiltinFunction_check (void)
{
    unsigned ack_state, total_args, i, j;
    char *name = 0;
    BuiltinFunction *func;

    if (BalsaDebug)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);

    TF_FUNC_INITIALIZE;

    total_args = GET_ARG_COUNT ();
    if (total_args < MIN_ARGS)
    {
        tf_error ("%s: requires at least %d arguments, but only had %d\n", message_facility, MIN_ARGS, total_args);
        return -1;
    }

    ack_state = GetInt (ACK_STATE);
    if (ack_state != 0 && ack_state != 1)
    {
        tf_error ("%s: first argument must be interpretable as a signal state (0 or 1)\n", message_facility);
        return -1;
    }

    name = GetString (NAME_IDX);
    if (!name)
    {
        tf_error ("%s: second argument must be interpretable as a builtin function name\n", message_facility);
        return -1;
    }

    func = BalsaSim_FindBuiltinFunction (name);
    if (!(func))
    {
        tf_error ("%s: builtin function \"%s\" not registered\n", message_facility, name);
        return -1;
    }

    if (GetInt (PARAM_COUNT_IDX) != func->parameterCount)
    {
        tf_error ("%s: parameter arity of instance of function \"%s\" (%d) does not match that registered (%d)\n",
          message_facility, (char *) func->name, GetInt (PARAM_COUNT_IDX), func->parameterCount);
        return -1;
    }

    if ((total_args - (func->parameterCount + RETURN_ARG_OFFSET)) != func->arity)
    {
        tf_error ("%s: argument arity of instance of function \"%s\" (%d) does not match that registered (%d)\n",
          message_facility, (char *) func->name, (total_args - (func->parameterCount + RETURN_ARG_OFFSET)), func->arity);
        return -1;
    }

    if (func->resultWidth > 0)  /* if == 0, we don't know the size - check it on first use */
    {
        if (CheckType ((func->parameterCount + RETURN_ARG_OFFSET), false))
        {
            tf_error ("%s: return value of function \"%s\" must connect to a net, reg or const type, but it was a %s",
              message_facility, (char *) func->name, TypeString (func->parameterCount + RETURN_ARG_OFFSET));
            return -1;
        }
        if (GetSize (func->parameterCount + RETURN_ARG_OFFSET) != func->resultWidth)
        {
            tf_error ("%s: return value of function \"%s\" is of size %d bits, but function was expecting one of size %d",
              message_facility, (char *) func->name, GetSize (func->parameterCount + RETURN_ARG_OFFSET), func->resultWidth);
            return -1;
        }
    }

    for (i = ((func->parameterCount + RETURN_ARG_OFFSET) + 1), j = 0; i <= total_args; i++, j++)
    {
        if (func->argumentWidths[j] > 0) /* if == 0, we don't know the size - check it on first use */
        {
            if (CheckType (i, true))
            {
                tf_error ("%s: argument %d of function \"%s\" must connect to a net, reg or const type, but it was a %s",
                  message_facility, i, (char *) func->name, TypeString (i));
                return -1;
            }
            if (GetSize (i) != func->argumentWidths[j])
            {
                tf_error
                  ("%s: argument %d of function \"%s\" is of size %d bits, but function was expecting one of size %d",
                  message_facility, i, (char *) func->name, GetSize (i), func->argumentWidths[j]);
                return -1;
            }
        }
    }

    if (BalsaDebug)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return 0;
}

MAYBE_STATIC int BalsaBuiltinFunction_call (void)
{
    WorkArea *pWa;
    unsigned i, j;
    BalsaParameter **parameters;
    char *name;
    int ack_state;

    if (BalsaDebug)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);

    TF_FUNC_INITIALIZE;
    pWa = GetWorkArea ();

    if (!pWa)
    {
        pWa = malloc (sizeof (WorkArea));
        SetWorkArea (pWa);
        pWa = GetWorkArea ();
        if (!pWa)
        {
            fprintf (stderr, "FATAL: %s: pWa not set!\n", __FUNCTION__);
            return -1;
            exit (EXIT_FAILURE);
        }

        name = GetString (NAME_IDX);
        pWa->func = BalsaSim_FindBuiltinFunction (name);

        if (!pWa->func)
        {
            tf_error ("FATAL: %s: Balsa function \"%s\" not registered\n", message_facility, name);
            exit (EXIT_FAILURE);
        }
        pWa->returnValIndex = (pWa->func->parameterCount + RETURN_ARG_OFFSET);

        if (pWa->func->parameterCount > 0)
        {
            parameters = calloc (sizeof (BalsaParameter *), pWa->func->parameterCount);
            for (i = RETURN_ARG_OFFSET, j = 0; j < pWa->func->parameterCount; i++, j++)
            {
                char *parameterString = GetString (i);

#if defined (ICARUS) || defined (CVER)
                {               /* Need un-escape the parameter string */
                    char *unescapedString = BalsaSim_ConvertBackslashCharsToASCII (strdup (parameterString));

                    parameters[j] = BalsaParameterParseFromString (unescapedString);
                    free (unescapedString);
                }
#else
                parameters[j] = BalsaParameterParseFromString (parameterString);
#endif
            }
        } else
            parameters = NULL;

        pWa->instData = NewBuiltinFunctionInstanceData (pWa->func, parameters);

        /* if we can't have checked it at elaboration time because the size was unknown */
        if (pWa->func->resultWidth == 0)
        {
            if (CheckType (pWa->returnValIndex, false))
            {
                tf_error ("%s: return value of function \"%s\" must connect to a net, reg or const type, but it was a %s",
                  message_facility, (char *) pWa->func->name, TypeString (pWa->returnValIndex));
                return -1;
            }
            if (GetSize (pWa->returnValIndex) != pWa->instData->resultWidth)
            {
                tf_error ("%s: return value of function \"%s\" is of size %d bits, but function was expecting one of size %d",
                  message_facility, (char *) pWa->func->name, GetSize (pWa->returnValIndex), pWa->instData->resultWidth);
                return -1;
            }
        }

        for (i = (pWa->returnValIndex + 1), j = 0; j < pWa->func->arity; i++, j++)
        {
            /* if we can't have checked it at elaboration time because the size was unknown */
            if (pWa->func->argumentWidths[j] == 0)
            {
                if (CheckType (i, true))
                {
                    tf_error ("%s: argument %d of function \"%s\" must connect to a net, reg or const type, but it was a %s",
                      message_facility, i, (char *) pWa->func->name, TypeString (i));
                    return -1;
                }
                if (GetSize (i) != pWa->instData->argumentWidths[j])
                {
                    tf_error
                      ("%s: argument %d of function \"%s\" is of size %d bits, but function was expecting one of size %d",
                      message_facility, i, (char *) pWa->func->name, GetSize (i), pWa->instData->argumentWidths[j]);
                    return -1;
                }
            }
        }
    }

    ack_state = GetInt (ACK_STATE);

    if (BalsaDebug)
        fprintf (stderr, "Called function %s with ack_state=%d\n", pWa->func->name, ack_state);

    if (ack_state != 0 && pWa->func->function_ackdown == NULL)
        return 0;

    BalsaSim_BuiltinFunctionRenewObjects (pWa->func, pWa->instData);

    for (i = (pWa->returnValIndex + 1), j = 0; j < pWa->func->arity; i++, j++)
        GetVec (pWa->instData->arguments[j], i);

    if (BalsaDebug)
        fprintf (stderr, "Called function: %s\n", pWa->func->name);

    if (ack_state == 0)
        pWa->func->function (pWa->func, pWa->instData);
    else
        pWa->func->function_ackdown (pWa->func, pWa->instData);

    if (BalsaDebug)
        fprintf (stderr, "Exited function\n");

    SetVec (pWa->instData->result, pWa->returnValIndex);

    if (BalsaDebug)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return 0;
}

/* The task will be called with (type_name, type_def) */
MAYBE_STATIC int BalsaInternType_check (void)
{
    unsigned i;

    if (BalsaDebug)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);

    TF_FUNC_INITIALIZE;

    if (GET_ARG_COUNT () != 2)
    {
        tf_error ("BalsaInternType: requires 2 arguments, but only had %d", GET_ARG_COUNT ());
        return -1;
    }

    for (i = 1; i <= 2; i++)
    {
        if (!GetString (i))
        {
            tf_error ("BalsaInternType: argument %d of \"BalsaInternType\" was not available as a string", i);
            return -1;
        }
    }

    if (BalsaDebug)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return 0;
}

MAYBE_STATIC int BalsaInternType_call (void)
{
    char *args[2];
    unsigned i, j;
    BalsaType *type = NULL;

    if (BalsaDebug)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);

    TF_FUNC_INITIALIZE;

    for (i = 0, j = 1; i < 2; i++, j++)
    {
        args[i] = GetString (j);
#if defined (ICARUS) || defined (CVER)
        args[i] = BalsaSim_ConvertBackslashCharsToASCII (strdup (args[i]));
#else
        args[i] = strdup (args[i]);
#endif
    }

    if (BalsaDebug)
        fprintf (stderr, "interning type %s\n", args[0]);

    if (BalsaTypeParseFromString (args[1], 0, &type) && type)
    {
        type->name = strdup (args[0]);
        BalsaInternType (type);
    } else
        tf_error ("BalsaInternType: problem interning type `%s'as %s", args[1], args[0]);

    for (i = 0; i < 2; i++)
        free (args[i]);

    if (BalsaDebug)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return 0;
}

MAYBE_STATIC int BalsaUnref_check (void)
{
    if (BalsaDebug)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);

    TF_FUNC_INITIALIZE;

    if (GET_ARG_COUNT () != 1)
    {
        tf_error ("BalsaUnref: requires 1 arguments, but had %d", GET_ARG_COUNT ());
        return -1;
    }

    if (CheckType (1, true))
    {
        tf_error ("BalsaUnref: argument 1 must connect to a net, reg or const type, but it was a %s", TypeString (1));
        return -1;
    }

    if (GetSize (1) != BALSA_OBJECT_BITS)
    {
        tf_error ("BalsaUnref: argument 1 is of size %d bits, but should be %d", GetSize (1), BALSA_OBJECT_BITS);
        return -1;
    }

    if (BalsaDebug)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return 0;
}

MAYBE_STATIC int BalsaRef_check (void)
{
    if (BalsaDebug)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);

    TF_FUNC_INITIALIZE;

    if (GET_ARG_COUNT () != 1)
    {
        tf_error ("BalsaRef: requires 1 arguments, but had %d", GET_ARG_COUNT ());
        return -1;
    }

    if (CheckType (1, true))
    {
        tf_error ("BalsaRef: argument 1 must connect to a net, reg or const type, but it was a %s", TypeString (1));
        return -1;
    }

    if (GetSize (1) != BALSA_OBJECT_BITS)
    {
        tf_error ("BalsaRef: argument 1 is of size %d bits, but should be %d", GetSize (1), BALSA_OBJECT_BITS);
        return -1;
    }

    if (BalsaDebug)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return 0;
}

MAYBE_STATIC int BalsaUnref_call (void)
{
    void *value;

    FormatData *fd;

    if (BalsaDebug)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);

    TF_FUNC_INITIALIZE;
    fd = NewFormatData (BALSA_OBJECT_BITS / (8 * sizeof (unsigned)));
    GetVec (fd, 1);
    value = FormatDataExtractPointer (fd, 0, BALSA_OBJECT_BITS);

    if (value && value != (void *) -1)
        BalsaObjectUnref (value);

    DeleteFormatData (fd);

    if (BalsaDebug)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return 0;
}

MAYBE_STATIC int BalsaRef_call (void)
{
    void *value;

    FormatData *fd;

    if (BalsaDebug)
        fprintf (stderr, "%s Entry\n", __FUNCTION__);

    TF_FUNC_INITIALIZE;
    fd = NewFormatData (BALSA_OBJECT_BITS / (8 * sizeof (unsigned)));
    GetVec (fd, 1);
    value = FormatDataExtractPointer (fd, 0, BALSA_OBJECT_BITS);

    if (value && value != (void *) -1)
        BalsaObjectRef (value);

    DeleteFormatData (fd);

    if (BalsaDebug)
        fprintf (stderr, "%s Exit\n", __FUNCTION__);
    return 0;
}
