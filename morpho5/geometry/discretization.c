/** @file discretization.c
 *  @author T J Atherton
 *
 *  @brief Different finite element discretizations
 */

#include "morpho.h"
#include "field.h"
#include "mesh.h"
#include "discretization.h"

/* -------------------------------------------------------
 * Discretizations
 * ------------------------------------------------------- */

typedef struct {
    char *label; // Label for the discretization
    int order;
    grade g;
    int *shape; // Number of degrees of freedom per grade [g+1 elements]
} discretization;

/** Initialize a discretization structure */
bool discretization_init(discretization *d, char *label, int order, grade g) {
    d->label = label;
    d->order = order;
    d->g = g;
    d->shape = MORPHO_MALLOC(sizeof(int)*(g+1));
    return d->shape;
}

/** Free a discretization structure */
void discretization_clear(discretization *d) {
    if (d->shape) MORPHO_FREE(d->shape);
}

/** */
bool discretization_assemblefieldref(discretization *d, objectmesh *mesh, objectfield *f, objectsparse *out) {
    
    /*elementid n = mesh_nelementsforgrade(mesh, d->g);
    for (elementid i=0; i<n; i++) {
        
    }*/
    
    return false;
}

/*
void discretization_value(objectfield *fld, discretization *disc, objectsparse *elmatrix) {
    
}

void discretization_gradient(void) {
    
}
*/

/* -------------------------------------------------------
 * Discretization veneer class
 * ------------------------------------------------------- */

objecttype objectdiscretizationtype;
#define OBJECT_DISCRETIZATION objectdiscretizationtype

/** Gets the object as a discretization */
#define MORPHO_GETDISCRETIZATION(val)   ((objectdiscretization *) MORPHO_GETOBJECT(val))

typedef struct {
    object obj;
    discretization d;
} objectdiscretization;

/** Discretization object definitions */
void objectdiscretization_printfn(object *obj) {
    objectdiscretization *d = (objectdiscretization *) obj;
    printf("<%s %i>", d->d.label, d->d.order);
}

void objectdiscretization_markfn(object *obj, void *v) {
}

void objectdiscretization_freefn(object *obj) {
    objectdiscretization *d = (objectdiscretization *) obj;
    discretization_clear(&d->d);
}

size_t objectdiscretization_sizefn(object *obj) {
    return sizeof(objectdiscretization);
}

objecttypedefn objectdiscretizationdefn = {
    .printfn=objectdiscretization_printfn,
    .markfn=objectdiscretization_markfn,
    .freefn=objectdiscretization_freefn,
    .sizefn=objectdiscretization_sizefn
};

/** Creates a new discretization object */
objectdiscretization *object_newdiscretization(char *label, int order, int grade, int *shape) {
    objectdiscretization *new = (objectdiscretization *) object_new(sizeof(objectdiscretization), OBJECT_DISCRETIZATION);
    
    if (new) {
        discretization_init(&new->d, label, order, grade);
        for (int i=0; i<=grade; i++) new->d.shape[i]=shape[i];
    }

    return new;
}

/* -------------------------------------------------------
 * Discretization objects
 * ------------------------------------------------------- */

/** Gets the array element with given indices */
value Discretization_order(vm *v, int nargs, value *args) {
    objectdiscretization *d=MORPHO_GETDISCRETIZATION(MORPHO_SELF(args));
    
    return MORPHO_INTEGER(d->d.order);
}

MORPHO_BEGINCLASS(Discretization)
MORPHO_METHOD(DISCRETIZATION_ORDERMETHOD, Discretization_order, BUILTIN_FLAGSEMPTY)
MORPHO_ENDCLASS

/* -------------------------------------------------------
 * 1D Lagrange elements
 * ------------------------------------------------------- */

/** Returns the number of nodes per element */
int cgn_nodecount(discretization *d) {
    return d->order+1; // 1 additional node per order
}

/** Returns the location of the nodes for a given element
 @note Coordinates are in the reference element */
bool cgn_nodepositions(discretization *d, objectmatrix *out) {
    int n = cgn_nodecount(d);
    
    for (int i=0; i<n; i++) {
        matrix_setelement(out, 0, i, ((double) i)/(n-1));
    }
    
    return true;
}

/*bool cgn_value(discretization *d, objectfield *f, int indx[], double *out) {
    
}*/

typedef struct {
    grade g;      // Grade of entry
    elementid id; // Element id
    int k;        // Value number
} fieldentry;

DECLARE_VARRAY(fieldentry, fieldentry);
DEFINE_VARRAY(fieldentry, fieldentry);

/** Determines hoe */
bool cgn_assemble(discretization *d, objectmesh *m, elementid id, int nv, elementid vid[], varray_fieldentry *out) {
    
    fieldentry v0 = { .g = MESH_GRADE_VERTEX, .id = vid[0], .k = 0 };
    varray_fieldentrywrite(out, v0);
    
    
    
    fieldentry v1 = { .g = MESH_GRADE_VERTEX, .id = vid[1], .k = 0 };
    varray_fieldentrywrite(out, v1);
    
    return true;
}


/** Constructor for lagrange cfn */
value lagrange_constructor(vm *v, int nargs, value *args) {
    value out=MORPHO_NIL;
    int order=1;
    
    if (nargs==1) {
        morpho_valuetoint(MORPHO_GETARG(args, 0), &order);
    }
    
    int shape[2] = { 1, order-1 };
    objectdiscretization *new = object_newdiscretization(LAGRANGE_CONSTRUCTORNAME, order, MESH_GRADE_LINE, shape);
    
    if (new) out = MORPHO_OBJECT(new);
    if (MORPHO_ISOBJECT(out)) morpho_bindobjects(v, 1, &out);
    
    return out;
}

/* -------------------------------------------------------
 * Initialization
 * ------------------------------------------------------- */

void discretization_initialize(void) {
    objectdiscretizationtype=object_addtype(&objectdiscretizationdefn);
    
    objectstring objclassname = MORPHO_STATICSTRING(OBJECT_CLASSNAME);
    value objclass = builtin_findclass(MORPHO_OBJECT(&objclassname));
    
    value discretizationclass=builtin_addclass(DISCRETIZATION_CLASSNAME, MORPHO_GETCLASSDEFINITION(Discretization), objclass);
    object_setveneerclass(OBJECT_DISCRETIZATION, discretizationclass);
    
    builtin_addfunction(LAGRANGE_CONSTRUCTORNAME, lagrange_constructor, BUILTIN_FLAGSEMPTY);
}
